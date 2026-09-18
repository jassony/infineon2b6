# APSFSM torque compensation

This directory contains the independent 2 kHz APSFSM torque-compensation reference, wrapper, generated code, Q15 adapter, and deterministic verification artifacts. It does not contain a plant, estimator, parameter-identification path, or production FOC model.

The governing equations, interfaces, defaults, mode behavior, safety limits, and test matrix are frozen in:

- `docs/specs/algorithms/apsfsm/apsfsm-system-architecture.md`
- `docs/specs/algorithms/apsfsm/apsfsm-implementation-test.md`

## Equation mapping

The source S-function emits the current output from the old state and updates state afterward:

```text
iq[k]      = B[k] sin(theta[k]) + C[k] cos(theta[k])
theta[k+1] = wrap(theta[k] + Ts omegaBase speedFb[k])
c[k+1]     = max(lambda c[k] + KHat^2/2, KHat^2/2)
B[k+1]     = B[k] + KHat sin(theta[k+1] + rho) error[k] / c[k+1]
C[k+1]     = C[k] + KHat cos(theta[k+1] + rho) error[k] / c[k+1]
```

The B/C vector is projected to the configured compensation-current bound. Production integration additionally limits total q and dq magnitude; a clipped applied sample holds B/C/c but continues the mechanical-angle integration.

The code-generation-ready MATLAB core has the fixed interface:

```matlab
[stateNext, diagnostics] = apsfsmTorqueCompensationStep( ...
    state, speedReferencePU, speedFeedbackPU, parameters, control)
```

- `state` is `single(4,1)` in the order `[BHat; CHat; ThetaMech; Covariance]`.
- `parameters` is `single(5,1)` in the order `[OmegaBase_radps; KHat; Rho_rad; Lambda; CoeffLimit_PU]`.
- `control` is `single(3,1)` in the order `[LearnEnable; FreezeAdaptation; Reset]`; every element is exactly zero or one.
- `diagnostics` is `single(4,1)` in the order `[IqRaw_PU; Valid; Status; SpeedError_PU]`.

`apsfsmTorqueCompensationReference` is a separate executable policy oracle for modes, eligibility, settle/ramp timing, and total q/current-circle clipping. Firmware remains the owner of those policies and all Q15 conversion.

## Defaults

Firmware defaults are mode off, `KHat=0.25`, `rho=-pi/2`, `lambda=0.98`, `IqComp=+-0.05 PU`, positive `800--4000 rpm`, 500 ms settle, and 500 ms apply ramp. Source-replay-only settings are `KHat=3.18853405461`, `lambda=0.999`, and `IqLimit=+-0.35 PU`.

## Deterministic verification

Run `verify_apsfsm_torque_compensation` from MATLAB. The class-based suite covers T01--T09 timing, safety, projection, clipping-freeze, and mode/ramp cases, plus recovery from the wrapper's parameter-independent zero covariance. T10 replays the checked-in source-task extraction in `testdata/apsfsm_source_replay_vectors.mat` (7801 samples, 3.9 s) and checks terminal B/C and the terminal harmonic peak against its recorded golden values with a `1e-4 PU` tolerance. The source S-function logs B/C during its Outputs phase, so the terminal golden is compared with the state immediately before the final Update; the post-final-update state is reported separately. The reported maximum transient `IqRaw` is informational and is not substituted for the recorded terminal harmonic peak.

Run `verify_apsfsm_ert_replay(pathToTcc)` after ERT generation to compile the generated wrapper source and replay the same 7801 source samples plus non-finite parameter/input cases. The test accepts a TinyCC executable argument or `APSFSM_TCC_EXE`, builds only in a temporary directory, and requires state and diagnostic errors no greater than `2e-5 PU`. `apsfsm_ert_nonfinite_host.c` and `apsfsm_ert_math_host.c` are host-only portability shims for TinyCC; the generated model source is compiled unchanged and the firmware does not link these replay files.

## Integration boundary

The adapter is called after `Ifx_MS_FocSolutionF16_executeSpeedControl()` and before the final IdMap current-magnitude measurement. Selector 0 resets/off, selector 1 learns in shadow, and selector 2 applies the bounded q compensation. No A2L entry is generated automatically.
