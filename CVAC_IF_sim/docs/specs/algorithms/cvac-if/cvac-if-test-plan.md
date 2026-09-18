# CVAC I-f MIL Test Plan

## Status: Nominal complete; robustness incomplete

**Last Updated:** 2026-08-24

## Component tests

- Reset/disabled output and deterministic re-enable.
- Alignment and vector-rotation current profiles.
- HPF DC rejection and finite response to a power step.
- Zero-speed and invalid-parameter division guards.
- Acceleration/current PI upper and lower saturation and recovery.
- `[0,2*pi)` angle wrap and observer-angle error wrap.
- Every state transition and every Abort code.

## Integrated tests

- Official modes 0 and 1 with the Teknic profile are compared against the
  source snapshot using the same non-persistent SimulationInput overrides.
- CVAC mode is run with paper IPMSM loads 0, 0.5, and 1.0 rated torque and an
  acceleration-phase load step.
- Handoff qualification is tested at 0, 5, and 15 electrical-degree observer
  angle offsets.
- Estimated flux is swept over 50/100/150%; estimated Lq over 70/100/130%,
  while plant parameters remain nominal.
- Invalid flux/Lq, negative command, overload, voltage saturation, frozen
  speed, timeout, and repeated enable are tested.

## Acceptance

- No NaN or Inf.
- Full load reaches 400 rpm within 1 s after Align.
- Nominal estimated-vs-true angle-error RMS below 5 degrees after validity.
- During synchronous startup the true angle error remains between
  approximately -16.78 and +19.89 electrical degrees.
- Handoff angle jump below 5 degrees, Iq jump below 5% rated peak current, and
  speed dip below 2%.
- A failed acceptance criterion remains a reported failure; tests and limits
  are not relaxed to make the run pass.

## Execution

Component checks use deterministic MATLAB assertions. Full-system runs use
finite-stop-time `SimulationInput` arrays and `logsout`; caches and generated
test artifacts are excluded from Git.

Executable entry points are `verify_cvac_if_controller`, `run_cvac_if_mil`,
`run_cvac_if_robustness`, and `smoke_official_if_modes`.
