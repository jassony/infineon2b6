# VAFID shadow parameter-identification integration

This folder contains the MATLAB reference, independent Simulink/ERT wrapper,
production C sidecar, and deterministic replay tests for the v5 full-parameter
VAFID algorithm. The firmware path is diagnostic-only: it can add a bounded
dual-frequency current probe and publish estimates, but it cannot take FOC
ownership or write KRE, motor, current-controller, or MTPA parameters. No A2L
file is generated or modified.

## Source of truth and target

The equations were transcribed from the source reviewed in Codex task
`01a03788-792d-7940-bc0a-7483980dbfbe`:

- `kre_ipmsm_fullparam_sfun_v5.m`, SHA-256
  `500141673CED6DB4CD8FAD31E9F89AA7712DB5A41FB6B673F1094D597F00BAF8`;
- `kre_fullparam_probe_sfun_v1.m`, SHA-256
  `2D1365FB5DBE25262A3E735E5646F6D26D4DD354354AAAA4B11FFB4B7999D077`;
- protected KRE step `kre_ipmsm_onlineid_step.m`, SHA-256
  `B56AF7CD9235BC48DBA43C3809039026DFB3FA83DE93504C9ECD57A5A110F9E0`.

The target is the project's IPMSM at a 50 us current-loop sample time. Inputs
are motor-terminal voltage in volts, phase-current alpha/beta in amperes, KRE
electrical angle in radians, raw electrical speed in rad/s, and active flux in
webers. Outputs are d/q probe commands in current PU plus Rs in ohms, Ld/Lq in
henries, PM flux in webers, fit/conditioning diagnostics, and validity state.
Reset or any invalid/gating sample clears oscillator, tracking, settling, and
window state; firmware also invalidates stale results after 300 ms.

## Contract

`vafid_discrete_reference_step` runs at 50 us and accepts applied alpha/beta
voltage, measured alpha/beta current, KRE electrical angle/raw electrical
speed, and KRE active flux. The internal dq frame uses a 25 Hz one-pole speed
filter plus an 8 Hz wrapped-angle tracker.

Two current-reference probes are produced while mode is enabled:

- d axis: 150 Hz, default 0.005 PU, hard capped at 0.015 PU;
- q axis: 220 Hz, default 0.005 PU, hard capped at 0.010 PU.

After 250 ms settling, the function accumulates exactly 2000 consecutive
valid samples (100 ms). Synchronous detection creates complex Vd, Vq, Id, and
Iq phasors at both frequencies. The two dq voltage equations form a fixed real
8-by-3 least-squares problem for `[Rs, Ld, Lq]`:

```text
Vd = Rs*Id + j*wp*Ld*Id - we*Lq*Iq
Vq = Rs*Iq + we*Ld*Id + j*wp*Lq*Iq
```

The columns are normalized before the fixed 3-by-3 solve. A window is rejected
for a singular solve, normalized condition number >= 1e4, relative residual
>= 0.65, a non-finite value, or a non-positive/out-of-range parameter. Accepted
Rs/Ld/Lq windows are fused with gain 0.2.

The PM flux estimate removes IPMSM saliency from low-pass-filtered active flux:

```text
FluxPM = LPF(activeFlux) - (Ld - Lq)*mean(Id)
```

It is not published until three consecutive windows are accepted, then is
fused with gain 0.35. Invalid samples abort the current window and require a
fresh settling interval and three new consecutive accepted windows. Mode off
produces zero probes and never runs a backup RLS estimator.

## Production integration constraints

The production sidecar is disabled by default and has no FOC ownership or
parameter-write path. A collection window is eligible only while KRE owns a
closed-loop FOC run and all of the following remain true for every fast-loop
sample:

- APSFSM torque compensation is fully off;
- HFI and RRC-DOB outputs are inactive;
- the modulator is on, is not forcing duty, and is not overmodulating;
- the no-dead-time-model signature and ADC trigger timing token are unchanged.

VAFID independently estimates the motor-terminal voltage as
`VmodulatorActual - VdeadTimeModel`. The current CM4 build excludes the legacy
dead-time model to reduce fast-loop load and image size, so
`VdeadTimeModel = 0` and a fixed no-model signature is supplied. This preserves
the shadow-only interface but can bias Rs/Ld/Lq estimates, especially at low
speed or high current; those estimates require fresh replay and bench
qualification. The KRE voltage input is deliberately unchanged, so this
diagnostic path cannot alter angle or speed behavior. Changing a gate or ADC
trigger timing rejects the whole window and starts a new settling interval.

The fast-loop pairing is explicit. At ADC sample `k`, VAFID consumes current
`i[k]` with the effective voltage and probe record captured from the preceding
PWM interval, `u[k-1]`. KRE's delayed electrical angle is advanced by one
sample using its raw electrical speed before the low-bandwidth dq-frame
tracker. This avoids adding another 50 us phase delay to the 150/220 Hz
lock-in signals.

The first firmware integration exports KRE active flux for diagnostics but
does not yet qualify it as an unbiased PM-flux source because KRE still uses
the uncorrected PWM-command voltage. Therefore Rs/Ld/Lq can become valid in
shadow mode, while the `VAFID_VALID_FLUX_PM` bit remains clear until a
dead-time-corrected active-flux path is independently replayed and bench
validated. Feedback is compile-time locked with
`VAFID_FEEDBACK_ALLOWED_MASK == 0`.

## Verification

### Closed-loop MIL operating-point matrix

`vafid_closed_loop_validation.slx` is an independent 50 us discrete MIL
model. It closes a 2 kHz speed loop and 20 kHz d/q current loops around a
dynamic IPMSM plant; the VAFID probes are added to the current references,
pass through the current regulators and plant, and only then return as
terminal voltage/current measurements. Identifier estimates remain shadow
outputs and are never fed back into the controller or plant parameters.

Run the strict matrix with:

```matlab
results = run_vafid_closed_loop_validation;
```

The default call throws when any strict gate fails. During diagnosis, retain
the complete table with `run_vafid_closed_loop_validation("", false)`.

The matrix contains 13 cases: the 3-by-3 combination of 500/3000/7000 rpm
and 0/3/7 N.m, steady -3000 rpm at -3 N.m, a 1->7->2 N.m variable-load
case, a 1000->5000->2000 rpm variable-speed case, and a +3000->-3000 rpm
reversal with signed load reversal. Every case uses `Simulink.SimulationInput`
and checks finite signals, qualification timing and accepted-window count,
5 percent Rs/Ld/Lq/PM-flux accuracy, final-estimate drift, fit residual,
conditioning, speed recovery, current tracking, voltage headroom, and probe
amplitude/phase transfer. The matching Gherkin specification is
`validation/vafid_closed_loop_validation.feature` and is executable with the
Simulink model-test workflow.

The plant truth is deliberately different from the identifier seed:
0.600 ohm versus 0.500 ohm Rs, 1.105 mH versus 1.300 mH Ld, 1.587 mH versus
1.380 mH Lq, and 0.0506 Wb versus 0.046 Wb PM flux. Mechanical inertia and
viscous friction are explicit validation-model assumptions, not production
calibrations. Ideal rotor angle, raw electrical speed, and unbiased active
flux isolate parameter-identification behavior from KRE observer error.

This model is intentionally an average-value plant. It does not claim PWM,
ADC quantization/noise, inverter dead time, DC-link ripple, saturation,
temperature drift, KRE angle error, ISR WCET, or motor-bench coverage. Torque
compensation is held off because production eligibility rejects a VAFID
window while APSFSM torque compensation is active. Dead-time compensation is
not applied in this isolated MIL model; the production no-dead-time-model
signature and its possible Rs bias remain separate replay and bench gates.

MATLAB R2026a results on 2026-09-07 kept every signal finite. All 13 cases
met the non-Rs gates, including Ld/Lq/PM-flux accuracy, qualification,
fit/conditioning, speed recovery, current tracking, voltage headroom, and
probe transfer. None met the 5 percent Rs gate: error was 8.79..8.81 percent
at 500 rpm, 26.27..26.73 percent at 3000 rpm, 105.04..106.42 percent at
7000 rpm, 26.61 percent at -3000 rpm, and 16.59 percent at the final 2000 rpm
point of the variable-speed case. A separate executable Gherkin run at
500 rpm passed 10 of 11 assertions and failed only `RsAccurate`.

The speed-dependent Rs error is consistent with an unresolved voltage/current
sample-pair and rotating-frame phase bias. It is not a convergence-time issue:
extending the 500 rpm dwell increased accepted windows from 14 to 24 while Rs
remained outside the gate. Rs feedback must therefore remain prohibited until
the production PWM/ADC timing is reproduced and any phase compensation is
derived, replayed, and bench validated. The variable definitions and safe
calibration sequence are in `CALIBRATION_GUIDE.md`.

Run the class-based deterministic suite with:

```matlab
results = verify_vafid_discrete_reference();
```

The suite covers default-off behavior, reset, exact settle/window timing,
continuous-window restart after invalid data, noiseless synthetic recovery,
ill-conditioned and NaN rejection, and saliency-corrected PM flux.

`verify_vafid_external_observer_wrapper` generates ERT C and replays 24 fixed
samples through the MATLAB reference, MATLAB Coder MEX, and direct generated C
harness. The expected maximum single-precision difference is zero. The KRE
diagnostic bridge is checked separately by
`verify_kre_external_observer_diagnostics` so the original position/speed
outputs remain within single-precision tolerance while active flux and raw
electrical speed are exported.

The production host test in `tests/vafid_parameter_identifier_host_test.c`
covers default-off pass-through, feedback hard lock, synthetic Rs/Ld/Lq
recovery, unqualified active-flux isolation, gate/configuration changes,
probe clipping, current-sample KRE invalidation, and runtime calibration
recovery. Hardware ISR timing and motor-bench accuracy remain separate release
gates; passing host replay and the CM4 IAR build does not authorize feedback.
