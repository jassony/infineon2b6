# New SOGI FE model validation

Generated: 2026-09-10 23:34:29.

Status: **PASS**, 132 PASS, 0 FAIL.

Scope: New FE basic-block models, actual five-port inputs, output equivalence and concurrent reuse.

Independent reference: reset-segmented input/output filter recurrence, using actual single port parameters cast to double. Original sample indices are retained. Tolerance: abs(error) <= 5e-05 + 5e-05*abs(reference).

Raw case CSV/MAT files contain time, actual input/reset/parameter snapshots, model outputs, independent reference and error. model_checks.csv records each executed check; model_frequency_fits.csv separates own-method agreement from continuous approximation.

Remaining or separately scoped checks:

- Compiled types/sample times and actual signal-object binding are checked separately.
- Visual layout acceptance is separate.
- FE 5000 us time replay is intentionally NOT_RUN because poles are unstable.
- No BE/Tustin/ZOH Simulink implementation is claimed.
- No unrestricted input-amplitude range or runtime parameter tuning is accepted.
