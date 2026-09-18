# SOGI discrete validation

Numerical validation: **PASS**. Generated 2026-09-10 18:25:27 * with 26.1.0.3346908 (R2026a) Update 5.

Baseline: f0 = 50 Hz, Ts = 4.99999987369e-05 s, k = 1.41421353817. Parameters and model inputs are quantized to single once; references use double.

| Check | PASS | FAIL | Skipped unstable |
|---|---:|---:|---:|
| Sample replay / repeated runs / scan | 79 | 0 | 1 |
| Frequency response | 40 | 0 | 0 |
| Invalid parameters | 10 | 0 | 0 |
| DC response | 8 | 0 | 0 |

Sample comparison: abs(error) <= 5e-5 + 5e-5 abs(reference), with no time alignment correction.

Frequency fitting uses the final 0.2 s. At 50 Hz, the continuous amplitude error must be <= 2% and phase error <= 1 degree. At all five frequencies, simulation versus its own discrete theory must meet amplitude error <= 2e-4 + 2e-4 abs(H) and phase error <= 0.05 degree.

## Center-frequency results

| Method | Output | Amplitude | Phase (deg) | Continuous amplitude error (%) | Continuous phase error (deg) | Status |
|---|---|---:|---:|---:|---:|---|
| FE | D | 1.01123190 | 0.000840 | 1.123190 | 0.000840 | PASS |
| FE | Q | 1.01124223 | -90.449166 | 1.124223 | 0.449166 | PASS |
| BE | D | 0.98901524 | 0.000825 | 1.098476 | 0.000825 | PASS |
| BE | Q | 0.98902556 | -89.549099 | 1.097444 | 0.450901 | PASS |
| Tustin | D | 1.00000069 | -0.001670 | 0.000069 | 0.001670 | PASS |
| Tustin | Q | 0.99998017 | -90.001661 | 0.001983 | 0.001661 | PASS |
| ZOH | D | 0.99998902 | -0.451659 | 0.001098 | 0.451659 | PASS |
| ZOH | Q | 0.99998903 | -90.449884 | 0.001097 | 0.449884 | PASS |

## Sample-period scan

| Ts (us) | Method | Pole radius, double | Pole radius, single | Status |
|---:|---|---:|---:|---|
| 50 | FE | 0.98895517 | 0.98895517 | PASS |
| 50 | BE | 0.98895514 | 0.98895514 | PASS |
| 50 | Tustin | 0.98895448 | 0.98895448 | PASS |
| 50 | ZOH | 0.98895425 | 0.98895424 | PASS |
| 100 | FE | 0.97803790 | 0.97803790 | PASS |
| 100 | BE | 0.97803742 | 0.97803743 | PASS |
| 100 | Tustin | 0.97803230 | 0.97803230 | PASS |
| 100 | ZOH | 0.97803051 | 0.97803053 | PASS |
| 500 | FE | 0.89584031 | 0.89584032 | PASS |
| 500 | BE | 0.89556774 | 0.89556775 | PASS |
| 500 | Tustin | 0.89507940 | 0.89507937 | PASS |
| 500 | ZOH | 0.89487425 | 0.89487425 | PASS |
| 1000 | FE | 0.80895472 | 0.80895472 | PASS |
| 1000 | BE | 0.80504330 | 0.80504331 | PASS |
| 1000 | Tustin | 0.80228568 | 0.80228567 | PASS |
| 1000 | ZOH | 0.80079992 | 0.80079992 | PASS |
| 5000 | FE | 1.11622561 | 1.11622555 | NOT_RUN_UNSTABLE |
| 5000 | BE | 0.41926446 | 0.41926445 | PASS |
| 5000 | Tustin | 0.43076739 | 0.43076739 | PASS |
| 5000 | ZOH | 0.32932154 | 0.32932153 | PASS |

## Artifacts and interpretation

- [implementation.csv](implementation.csv): per-case replay, finite-value and reset checks.
- [frequency.csv](frequency.csv), [poles.csv](poles.csv), [parameters.csv](parameters.csv), [dc.csv](dc.csv): measured numeric evidence.
- [validation_raw.mat](validation_raw.mat): exact input, reset, sample times, model outputs, independent references and configuration for every run.
- Q has DC gain k; it is not a DC-blocking output.
- NOT_RUN_UNSTABLE means a mathematically unstable candidate was deliberately disabled in that simulation only; it is not a passing simulation.
- This report scores numerical simulation only. Model structural checks, compiled type/sample-time inspection and continuous-harness evidence are recorded separately. No code generation, FOC integration or hardware behavior is claimed.

![Startup](startup_waveforms.png)

![Frequency response](frequency_response.png)

![Disturbances](disturbance_waveforms.png)
