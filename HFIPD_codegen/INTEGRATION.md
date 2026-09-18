# HFIPD pre-alignment integration

## Baseline, 2026-09-15

Repository D:\A_PRJ\infineon3in1\code_xcp, starting HEAD
fdb07e34e6b30311ff47f9f37eb354be41a232d9. Existing dirty control, KRE,
FWC, profiler, models and A2L changes are unrelated and remain unstaged.
This note establishes the local baseline before algorithm/control edits.

Source: task 01a09f0c-1d1b-7772-be5c-da70d5554674, hfipd_step.m and
hfipd_config.m in hfipd_pmsm_reproduction; Jiang / Cheng, Defence Technology
33 (2024), 19-29. This is standstill injection/demodulation before alignment,
not a replacement for the compiled KRE/Flux estimator.

Requested: calibratable enable, default OFF. Identification precedes alignment.
A valid electrical rotor angle seeds the existing I/f accumulator/alignment
frame for either startup mode. Direct closed-loop mode retains nonzero
alignment and valid-observer handoff. OFF bypasses identification and never
writes an initial angle. Stop/fault invalidate a session; restart identifies anew.

The source falsely asserts validity at ideal +/-90 degree errors. The port
adds a second magnetic-axis track with a pi/4-offset initial guess and requires
axis agreement before polarity detection. This is an integration extension.
The saturated locked-rotor plant is an explicit assumption, not motor data.
Target defaults Ld=1300 uH, Lq=1380 uH are weakly salient; actual construction
and saturation need bench confirmation. The algorithm needs measured SI
alpha/beta current, not Rs/Ld/Lq/flux/pole-pair inputs.

Scheduling: 50 us, 1 kHz injection, N=20 versus original 100 us/N=10.
Fixed-size single state. Freeze the calibration snapshot while stopped at
the 2 kHz boundary. Runtime writes take effect on the next startup session.
Leave interrupt priorities and command-source/startup-mode defaults unchanged.

Before control integration: MATLAB single replay, C equivalence, reset,
nonfinite/invalid/no-saturation/degenerate cases. Before bench acceptance:
CM4 build, actual VI delay, startup/stop/reversal/restarts, ELF and calibration
hashes, raw traces, Fast:Speed about 10:1, no overruns, foreground progress,
and >=20% measured fast-loop WCET margin.

Initial status: implementation, replay, build and bench NOT_RUN.
No A2L or main SLX edit is requested.

## Completion record

Baseline commit: 89ea7028c755f4c3d362a0fc30a7102cbda00a66.
Software implementation, replay and both ownership builds completed.
Bench timing/accuracy and target-motor tuning remain NOT_RUN.
See README.md and reports/VALIDATION.md for the final scope and measured results.
