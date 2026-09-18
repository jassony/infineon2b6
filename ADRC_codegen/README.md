# ADRC Speed Controller Codegen Mapping

## Scope

This directory ports the speed-loop ADRC from MathWorks R2026a
`PMSMSpeedUsingADRCExample`, specifically `Speed Control / ADRC_Controller_Speed`
in `mcb_pmsm_adrc_qep_f28379d.slx`.

The ADRC is a speed-to-q-current-reference algorithm. It does not participate
in the external-observer manager, rotor angle selection, current PI, or the
main sensorless model.

The target speed loop runs at 2 kHz (`Ts = 500 us`). All wrapper math and
runtime adapter states are `single` precision.

## Discrete Contract

The source example uses a second-order ADRC with a third-order extended state
observer (ESO). The source zero-cancellation filter has
`0.01 * (Ts_speed / Ts_current) = 0.1` at the target sample rates.

For each step, with `r` as speed reference, `y` as measured speed, and `u`
as q-current reference:

```text
r_f[k] = r_f[k-1] + 0.1 * (r[k] - r_f[k-1])
p      = exp(-wo * Ts)
beta0  = 1 - p^3
beta1  = 1.5 * (1-p)^2 * (1+p) / Ts
beta2  = (1-p)^3 / Ts^2

e      = y - z0
zc     = [z0 + beta0*e, z1 + beta1*e, z2 + beta2*e]
u      = sat(((r_f-zc0)*wc^2 - 2*wc*zc1 - zc2) / b0, umin, umax)
z0+    = zc0 + Ts*zc1 + 0.5*Ts^2*zc2 + 0.5*Ts^2*b0*u
z1+    = zc1 + Ts*zc2 + Ts*b0*u
z2+    = zc2
```

The fixed source-example defaults are:

| Parameter | Value |
| --- | --- |
| `b0` | `19817.677368` PU speed / (PU Iq * s^2) |
| `wc` | `104.719755` rad/s |
| `wo` | `837.758041` rad/s |
| Output limits | `[-0.8, 0.8]` PU |
| Initial states | zero |

These are replay baselines only. The target motor must identify `b0` before
ADRC is selected for a bench closed-loop run.

## Wrapper Model

`adrc_speed_controller_wrapper.slx` is an independent, fixed-step discrete
ERT wrapper. It has no plant, PWM, current PI, speed PI, or main FOC model.

Inputs:

| Input | Type |
| --- | --- |
| `Speed_Reference_PU`, `Speed_Feedback_PU` | `single` PU |
| `Critical_Gain_PU_per_PU_s2`, `Control_Bandwidth_radps`, `Observer_Bandwidth_radps` | `single` |
| `Iq_Upper_Limit_PU`, `Iq_Lower_Limit_PU` | `single` PU |
| `Reset_u8` | `uint8` |

Outputs are `Iq_Reference_PU`, `Valid_u8`, `Status_u8`, filtered reference,
and the three ESO states. Status values are `0=idle/reset`, `1=valid`,
`2=parameter invalid`, and `3=numerical invalid`.

The model uses an explicit Stateflow implementation of the generated discrete
equations because the source ADRC block mask exposes `b0`, `wc`, `wo`, and
limits as non-tunable code-generation parameters. The source block remains the
behavioral reference; the wrapper's root inputs make the required runtime XCP
calibration contract explicit.

ERT configuration is fixed-step discrete, `FixedStep = 5e-4`, `ert.tlc`, C
language, and code-generation-only mode. Generate it with:

```matlab
cd('D:/A_PRJ/infineon3in1/code_xcp/ADRC_codegen')
slbuild('adrc_speed_controller_wrapper')
verify_adrc_speed_controller_wrapper
```

The local generated C carries a MathWorks Academic License banner. It is kept
only as a local code-generation/replay artifact and is not linked into or
committed for the IAR firmware. A commercially licensed generated artifact
would be required before replacing the adapter with generated C in a product
build.

## Firmware Adapter

`adrc_speed_controller_adapter.c` is the production integration boundary:

```c
void ADRC_SpeedController_initialize(void);
void ADRC_SpeedController_reset(void);
uint8_t ADRC_SpeedController_execute(referenceSpeedQ15, measuredSpeedQ15,
                                     calibration, iqReferenceQ15, diagnostics);
```

It converts Q15 to PU once at entry using `PU = Q15 / 32768`, and converts
back with rounded, saturated `Q15 = round(PU * 32768)` limited to
`[-32768, 32767]`. The speed reference passed by FOC is already rate-limited;
the measured speed is the selected estimator's Q15 output. Invalid parameters,
non-finite math, or an invalid output reset the state and publish zero
q-current. The FOC core does not fall back to PI automatically.

For a bumpless open-to-closed-loop handoff, the FOC core preloads the callback
output with the q-current that was actually applied immediately before speed
control took ownership. In direct startup this is the final positioning
current; in ordinary startup it is the outgoing I/f current. On the first
valid execution after reset, the adapter initializes the ESO speed state from
the measured speed and its disturbance state from this current, so the first
ADRC output equals the outgoing current subject to the configured Iq limits.
The preload is consumed once. A normal online gain change does not reapply it.

Live calibrations are `b0`, `wc`, `wo`, and positive/negative Q15 Iq
limits. They are valid only when all floating-point values are finite, `b0 > 0`,
`wc > 0`, `wo > 0`, and lower limit is not greater than upper limit. Sample
time, ADRC order, and filter coefficient are intentionally fixed rather than
exposed as XCP calibrations. Reset states are zero; the one-shot closed-loop
handoff initialization described above is fixed integration behavior.

## Firmware Integration

`main_cm4.c` exposes these XCP-visible variables in the existing M4 XCP
section, without editing an A2L file:

- `Cal_ADRC_Speed_Selector_u8` (0=PI, 1=ADRC), latched only in `off` or
  `standBy`.
- `Cal_ADRC_Speed_Reset_u8`, edge-triggered reset command.
- `Cal_ADRC_Speed_CriticalGain_PU_per_PU_s2_f32`, bandwidths, and Q15 Iq
  limits, all live while ADRC is selected.
- `Meas_ADRC_Speed_*` active, valid, status, q-current, PU output, and ESO
  diagnostics.

The FOC callback is called only by the normal closed-loop speed-reference
path. Direct-current mode remains higher priority. MTPA receives the resulting
q-current reference afterward and continues to calculate d/q references.

The ADRC state resets on selector change, explicit reset edge, stop, fault,
and direct-current mode entry. A valid online numerical calibration change
does not reset state.

## Verification

`verify_adrc_speed_controller_wrapper.m` compares the wrapper simulation to
an independent single-precision discrete reference. It covers fixed vectors,
speed steps, reversal, noise, reset, live valid tuning, zero/negative gains,
invalid limits, and non-finite input.

Before bench selection of ADRC, perform observer-output comparison, target
motor `b0` identification, no-load startup, low-speed/reversal/load tests,
and invalid-parameter safety checks. Capture angle, speed, Iq reference and
feedback, ADRC states/status, and both loop periods. Do not select ADRC for a
bench closed-loop run using the source example `b0`.
