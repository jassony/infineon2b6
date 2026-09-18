# Requested-stop current shortcut

`Cal_FOC_StopIsHi_A_f32` defaults to **25 A**, in `.xcp_cal_m4`.
Zero disables the shortcut; negative and non-finite values also preserve the
original ramp-down stop. Thresholds above the representable feedback magnitude
cannot trigger it. The A2L is not updated by this change.

At the existing 2 kHz state-machine boundary:

- In `run`, a control-disable or power-disable request enters `standBy` directly
  when feedback `Is > threshold`; otherwise it enters `rampDown` as before.
- In `rampDown`, either high current or the original ramped-speed condition
  enters `standBy`. Thus a current increase during deceleration also ends drive.
- Faults retain priority. High current alone never stops an otherwise enabled run.
- Exact equality follows the original speed-based stop.

`Is = sqrt(Ialpha^2 + Ibeta^2)` uses the latest completed reconstructed current
sample, converted from Q15 with the configured 50 A base. This is current-vector
amplitude, equivalent to d-q magnitude, **not phase RMS**. No filtering, dwell
time, or IdMap eligibility is applied. The implementation compares squared
amplitudes at the speed boundary and adds no work to the 20 kHz callback.

Direct stop means using the existing standby path without waiting for the speed
ramp: the fast loop stops regulation and commands zero voltage. It does not
mean the mechanical shaft reaches zero speed immediately. Existing standby
reset and power-stage disable sequencing remain responsible for shutdown.

Run `MS/tests/run_foc_stop_current_tests.ps1` with IAR 9.40.1. The script extracts
the production current predicate and run/ramp-down function bodies, stubs speed
limiting and substate execution, and checks 30 boundary/transition assertions
with the Cortex-M4 C-SPY simulator. It covers both current signs, vector sum,
25 A equality, calibration changes/disable/invalid values, command gating,
fault priority, and both signs of the original speed exit threshold.

Build `IAR/cm4_mc/MMEk_Demo_CM4_FOC.ewp`, configuration
`Multi Motor Evalkit V1.0`, for the full CM4 integration check.
Simulation does not validate hardware shutdown, current transients, or ISR
timing. Bench validation remains required for high/low-load stops, deceleration
current crossing, reversal and repeated starts. Record command, state, Is,
speed, Iq reference/feedback, voltage/PWM and fast/speed/idle timing alongside
the exact firmware hash and calibration snapshot.

## Local verification, 2026-09-08

- C-SPY: all 30 checks passed.
- CM4 IAR build: passed, 0 errors and 3 existing unused-variable warnings.
- HEX SHA-256:
  `8cf8429c00e1de75e70f172d7e91844bc53953ccd40b8c4cccba4dae1da6a998`.
- Default new calibration: `Cal_FOC_StopIsHi_A_f32 = 25.0F`.
- No firmware flashing or bench run was performed. This build includes the
  pre-existing working-tree changes; no unrelated files were staged/committed.
