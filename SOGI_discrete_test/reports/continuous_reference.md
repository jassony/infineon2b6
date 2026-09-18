# Continuous-reference harness

Status: **PASS**. 2026-09-10 18:25:53 *; 26.1.0.3346908 (R2026a) Update 5.

A 50 Hz sine is sampled and held at the common Ts before the continuous SOGI. The DUT sees its single-precision version. Outputs are compared at identical sample instants without interpolation or phase shifting.

Solver convergence: MaxStep = Ts/8, RelTol = 1e-9, AbsTol = 1e-11 versus MaxStep = Ts/16, RelTol = 1e-10, AbsTol = 1e-12. Maximum sampled continuous difference must be <= 1e-6 PU.

ZOH sample error uses abs(error) <= 5e-5 + 5e-5 abs(reference). FE, BE and Tustin report their actual discretization error; MEASURED is not an exact-match pass.

| Check | Method | Metric | Limit | Status |
|---|---|---:|---:|---|
| continuous_convergence | Both | 4.3298698e-15 | 1e-06 | PASS |
| dut_excitation_response | FE | 1.01466227 | 0.5 | PASS |
| continuous_comparison | FE | 0.0138063194 | NaN | MEASURED |
| error_output_wiring | FE | 0 | 2e-06 | PASS |
| dut_excitation_response | BE | 0.992867887 | 0.5 | PASS |
| continuous_comparison | BE | 0.0191057301 | NaN | MEASURED |
| error_output_wiring | BE | 0 | 2e-06 | PASS |
| dut_excitation_response | Tustin | 1.0036521 | 0.5 | PASS |
| continuous_comparison | Tustin | 0.00785960476 | NaN | MEASURED |
| error_output_wiring | Tustin | 0 | 2e-06 | PASS |
| dut_excitation_response | ZOH | 1.0036521 | 0.5 | PASS |
| continuous_comparison | ZOH | 0.0473297203 | 1 | PASS |
| error_output_wiring | ZOH | 0 | 2e-06 | PASS |

[CSV metrics](continuous_reference.csv) · [Raw data](continuous_reference_raw.mat)

![Continuous comparison](continuous_reference.png)
