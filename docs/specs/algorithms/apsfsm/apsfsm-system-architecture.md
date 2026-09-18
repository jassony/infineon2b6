# APSFSM Torque Compensation System and Architecture Specification

**Status:** Approved for implementation  
**Last updated:** 2026-08-27  
**Target:** CM4 FOC firmware, 2 kHz speed loop

## 1. Purpose and scope

APSFSM suppresses the first mechanical-order speed ripple by learning a sine/cosine q-axis current compensation from speed error. It is a current-reference algorithm, not an estimator. It shall run after the selected PI or ADRC speed controller and before the existing Id reference policy.

Version 1 deliberately excludes motor-parameter identification, excitation probes, automatic parameter feedback, estimator selection changes, A2L edits, and changes to the protected production Simulink model.

The algorithm is based on Zhang et al., *Adaptive Periodic Speed Fluctuation Suppression for Permanent Magnet Compressor Drives*, Sensors 2025, 25(7), 2074, DOI: https://doi.org/10.3390/s25072074, and on the source reproduction identified in Codex task `01a03788-792d-7940-bc0a-7483980dbfbe`.

## 2. External interface

All runtime values are scalar, real, fixed-size, and sampled at 500 us.

| Signal | Direction | Type | Unit / scaling | Meaning |
| --- | --- | --- | --- | --- |
| `referenceSpeedQ15` | input | `Ifx_Math_Fract16` | PU, Q15 | Rate-limited positive mechanical-speed reference |
| `measuredSpeedQ15` | input | `Ifx_Math_Fract16` | PU, Q15 | FOC-selected estimated mechanical speed |
| `baseDqQ15` | input | `Ifx_Math_CmpFract16` | PU, Q15 | Current command after speed control |
| `controlEligible` | input | `uint8_t` | boolean | Run + FOC + closed-loop + non-direct-interface gate |
| `calibration` | input | fixed struct | mixed | Mode, RGN gains, limits, speed window, settle/ramp times |
| `compensatedDqQ15` | output | `Ifx_Math_CmpFract16` | PU, Q15 | Unchanged command in off/shadow; bounded q compensation in apply |
| `diagnostics` | output | fixed struct | mixed | Raw/applied compensation, learned state and status |

Public C functions are `APSFSM_TorqueComp_initialize`, `APSFSM_TorqueComp_reset`, and `APSFSM_TorqueComp_execute`. The API allocates no dynamic memory and performs Q15/PU conversion only at the adapter boundary.

## 3. Modes and eligibility

| Selector | Mode | Behavior |
| ---: | --- | --- |
| 0 | Off | Output zero, pass through dq command, reset all learning and ramp state |
| 1 | Shadow | Learn and publish diagnostics; dq command remains bit-for-bit unchanged |
| 2 | Apply | Learn and add bounded compensation to q; ramp application gain from zero to one |

Mode 0 is the power-on default. A live transition from shadow to apply retains `BHat` and `CHat` and ramps the applied amplitude over the calibrated 500 ms. Any invalid selector is treated as off.

Learning is eligible only when all conditions hold continuously for the calibrated settle time:

- the FOC state is `run`, actual control mode is FOC, substate is closed loop, and the direct-current interface is disabled;
- both reference and measured speed are positive and inclusive of the calibrated 800--4000 rpm window;
- all calibration and numeric inputs are valid and finite.

Stop, fault, ramp-down, leaving closed loop, zero/reverse speed, speed-window exit, explicit reset, or invalid data immediately produces zero compensation and resets `BHat`, `CHat`, `ThetaMech`, `Covariance`, settle counter, and apply ramp.

## 4. Discrete equations and timing

Let `Ts = 5e-4 s`, `omegaBase = 2*pi*10000/60 rad/s`, and `e[k] = speedRefPU[k] - speedFbPU[k]`. State initial conditions are `B[0]=0`, `C[0]=0`, `theta[0]=0`, and `c[0]=0.5*KHat^2`.

The source S-function has explicit output-before-update semantics:

1. `iqRaw[k] = B[k]*sin(theta[k]) + C[k]*cos(theta[k])`.
2. `theta[k+1] = wrap_0_2pi(theta[k] + Ts*omegaBase*speedFbPU[k])`.
3. `cCandidate = max(lambda*c[k] + 0.5*KHat^2, 0.5*KHat^2)`.
4. `Bcandidate = B[k] + KHat*sin(theta[k+1] + rho)*e[k]/cCandidate`.
5. `Ccandidate = C[k] + KHat*cos(theta[k+1] + rho)*e[k]/cCandidate`.
6. Project `[Bcandidate,Ccandidate]` onto the compensation-current circle defined by the calibrated Iq limits.

When external q/current-circle clipping occurs in apply mode, the clipped compensation is used, `ThetaMech` still advances, and the B/C/Covariance update is held for that sample. Shadow mode never alters the dq command.

## 5. Calibration and numerical safety

| Calibration | Firmware default | Validation |
| --- | ---: | --- |
| Selector | 0 | 0, 1, or 2 |
| `KHat` | 0.25 | finite, positive, and `0.5*KHat^2` finite |
| `Rho` | `-pi/2 rad` | finite; normalized before use |
| `Lambda` | 0.98 | finite, `0 < lambda < 1`, and convergence inequality passes |
| Iq low/high | -1638 / +1638 Q15 | low <= 0 <= high and within system q limits |
| Speed low/high | 800 / 4000 rpm | `0 < low < high <= 10000` |
| Settle time | 500 ms | nonzero, representable as 2 kHz ticks |
| Apply ramp | 500 ms | nonzero, representable as 2 kHz ticks |

The convergence guard is `sqrt((2*lambda-1)^2 + (omegaHigh*Ts)^2) < 1`, where `omegaHigh` is the calibrated upper mechanical speed in rad/s. The firmware default `lambda=0.98` passes at 4000 rpm. Source-replay-only values are `KHat=3.18853405461`, `rho=-pi/2`, `lambda=0.999`, and symmetric `IqLimit=0.35 PU` at the source motor base speed.

All algorithm states are `single`. Non-finite values are rejected before publication; no previous compensation is retained after an invalid sample.

## 6. Integration architecture

```text
rate-limited speed ref ----+
estimated speed -----------+--> APSFSM RGN state/update --> raw Iq compensation
base dq command -----------+--> mode/ramp/limit/current-circle --> compensated dq
                                                               |
executeSpeedControl --> APSFSM --> IdMap Is observation --> current controller
```

The placement lets both PI and ADRC use the same compensation. IdMap remains the sole automatic d-axis reference source, while APSFSM changes only the q-axis command.

The independent Simulink wrapper contains only fixed scalar ports, the algorithm step, resettable state, and diagnostics. It uses fixed-step discrete scheduling at 500 us, `ert.tlc`, C output, and code-generation-only configuration. No algebraic feedback path or rate transition is introduced.

## 7. Status and diagnostics

Status values are `IDLE=0`, `WAIT_SPEED=1`, `SHADOW_VALID=2`, `APPLY_VALID=3`, `PARAMETER_INVALID=4`, and `NUMERICAL_INVALID=5`. `Clipped` is a separate flag so a valid but constrained sample remains distinguishable.

XCP-visible names follow `Cal_APSFSM_*` and `Meas_APSFSM_*`, use `NO_OPT volatile`, and reside in `.xcp_cal_m4`. The A2L is intentionally unchanged.

## 8. Known limitations

- Only the first mechanical harmonic is learned.
- Mechanical angle is reconstructed by integrating estimated speed; it is reset between eligible runs and therefore relearns phase.
- Reverse rotation and operation outside 800--4000 rpm are unsupported in version 1.
- Automatic feedback into speed-controller anti-windup is not added; APSFSM freezes its own adaptation when the combined command clips.
- Bench performance and calibration remain a manual validation activity.

## 9. API verification notes

The Q15 types, speed-loop timing, FOC state fields, external speed-controller interface, MTPA/IdMap handoff, XCP section pragmas, and ERT/IAR source patterns were verified against neighboring `ADRC_codegen`, `MTPA_codegen`, `IdMap_codegen`, `ConfigWizard/Ifx_MS_FocSolutionF16_Cfg.h`, and `Example/CM4_FOC/main_cm4.c` implementations.
