# CVAC I-f MIL Validation Record

**Date:** 2026-08-24  
**MATLAB:** R2026a  
**Scope:** MIL only; no code generation or hardware execution

## Structural and component checks

- `model_check` reports both models healthy for unconnected ports, dangling
  lines, and Stateflow lint.
- The direct controller test visits states 1 through 7, stays finite, verifies
  disable/re-enable, and checks zero-flux and reverse-command Abort.
- Official modes 0 and 1 run from `SimulationInput` with finite outputs.

## Nominal paper-IPMSM matrix

All rows pass `run_cvac_if_mil`.

| Load | Final rpm | 98% time | Handoff | Angle jump | Iq jump | Speed dip | Est. RMS | Slope |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 0.0 | 399.58 | 1.1887 s | 1.9360 s | 0.795 deg | 0.455% | 0.000% | 0.944 deg | 0.919 |
| 0.5 | 399.97 | 1.2055 s | 1.3995 s | 0.316 deg | 2.030% | 1.120% | 2.193 deg | 0.850 |
| 1.0 | 399.85 | 1.2291 s | 1.5140 s | 0.648 deg | 0.177% | 0.000% | 1.868 deg | 0.863 |

Full load reaches 98% speed 0.7291 s after alignment. All rows have matching
estimate/true-error sign and no NaN/Inf.

## Estimated-parameter robustness

Plant parameters stay nominal; only controller estimates change.

| Case | Final state | Abort | Handoff | Result |
|---|---:|---:|---:|---|
| Flux 50% | 8 | 3 | none | Fail: sustained raw angle error |
| Flux 100% | 7 | 0 | 1.5140 s | Pass |
| Flux 150% | 7 | 0 | 1.4515 s | Pass |
| Lq 70% | 8 | 6 | none | Fail: 5 s startup timeout |
| Lq 100% | 7 | 0 | 1.5140 s | Pass |
| Lq 130% | 8 | 6 | none | Fail: 5 s startup timeout |

Every case remains finite. Failed cases retain the 60-degree/50 ms loss-of-
synchronism gate and the 5-degree handoff gate; neither limit was relaxed.

## Explicit implementation deviations

- Requested `BetaSeed = 61.9707 rad/s2`; executable MIL calibration is
  `150 rad/s2` to meet the rated-load timing gate.
- Requested acceleration limit is `1454.1986 rad/s2`; executable MIL
  calibration caps applied acceleration at `200 rad/s2` for this Plant.
- Handoff also rejects falling observer speed and, near minimum current,
  underspeed greater than 0.005 PU. The 0.02 PU absolute limit remains.

## Gate decision

Nominal MIL and structural gates pass. The requested robustness gate does not,
so the feature commit is intentionally withheld.
