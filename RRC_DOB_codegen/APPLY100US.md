# RRC-DOB 100 us Apply enablement

This records the initial Apply-enabled build. See [PARAMETER_CHECKS.md](PARAMETER_CHECKS.md)
for the subsequent calibration range-check removal and current artifact hashes.

## Baseline and authorization

- Baseline commit: `c0a7ad9636c6cf20008fa85f9074dec063da2b56`.
- User explicitly requested `FOC_RRCDOB_APPLY_ENABLE=1` after being informed
  that the 100 us Monitor bench and FWC/Apply coexistence gates remain pending.
- Existing RRC-DOB, timing configuration and FOC integration sources have no
  unstaged changes before this change. Unrelated worktree edits are preserved.
- Locked baseline CM4 HEX SHA-256:
  `E331D100D17EE2E0570082BE85A69B071EFB5CBB58FBCA4B7CF2FBF65E1A6B6B`.
- Scope: enable the existing Apply branch; preserve default selector OFF,
  equations, calibration values, stopped-only snapshots, KRE ownership,
  FWC behavior, startup routing and timing. No new protection logic, A2L
  edits, main-model edits or flashing are requested.
- FWC remains eligible with RRC-DOB in this configuration. Enabling Apply
  does not establish that their combined powered behavior is validated.

## Verification status before change

The locked baseline arithmetic replay and OFF/Monitor integration regression
passed in the preceding inspection. Powered Monitor/Apply, FWC coexistence,
ISR WCET/overrun and board readback remain NOT_RUN. This document records
authorization and provenance, not completion of those bench gates.

## Delivered behavior

`FOC_RRCDOB_ENABLE=1`, `FOC_RRCDOB_APPLY_ENABLE=1`, control 100 us,
PWM 50 us. The only production behavior change is enabling the existing
Apply voltage branch. No observer arithmetic or FOC ownership logic changed.

Source calibration defaults are unchanged: selector 0, reset 0, cutoff ratio
3277 Q15, per-axis output limit 3277 Q15, speed range 800--4000 rpm, ramp 10 ms.
Motor estimates continue to use the applied common motor snapshot.

After loading the new CM4 image, stop and disable control, then write
`Cal_RRCDOB_Sel_u8=2`. Wait for `Meas_RRCDOB_Sel_u8=2` and
`Meas_RRCDOB_Pend_u8=0` before starting. In eligible closed-loop operation
with valid speed/input, expect `Stat=3`, `Act=1`, `Valid=1`, `OutAct=1`.
Selector 1 remains Monitor (Stat=2, OutAct=0); selector 0 remains OFF.
Mode/parameter/reset changes, including a switch to OFF, are still applied
only while stopped. Output-active confirms voltage-path application, not
measured current-ripple improvement.

## Software verification

- IAR 9.40.1 100 us Apply arithmetic regression: PASS, compile/link zero
  errors and warnings. Reused the existing 3200-row MATLAB reference vectors;
  MATLAB generation was not rerun. Disturbance error RMS 0.286775639 / peak
  0.5021515 Q15 LSB; applied command peak error 0.943298 Q15 LSB.
- Production-function integration regression: PASS, compile/link zero errors
  and warnings. Added Apply checks for stopped-only selection, transition
  reset, corrected d/q input to Cartesian-to-polar conversion, angle addition,
  and raw PI output when outputActive=0. Existing OFF/Monitor/capture/ADC-gap
  and FWC eligibility checks continue to pass.
- Replay uses a Park-transform double; integration uses PI/PWM/observer/math
  doubles. These tests do not establish physical phase alignment, ISR WCET,
  motor stability or FWC/Apply coexistence.
- C-SPY and IAR build startup require execution outside this session's sandbox.
  No hardware connection or flashing is performed by these software tests.
- Raw test outputs: `Build/RRCDOB100usTests/rrc_100_apply1_result.txt` and
  `Build/RRCDOB100usTests/integration_result.txt` (not committed).

## CM4 artifact and remaining acceptance

- IAR CM4 `Multi Motor Evalkit V1.0` full build: PASS, 0 errors, 15 warnings.
- HEX: `Build/CM4_FOC/MMEk_Demo_CM4_FOC.hex`, SHA-256
  `724701C908BE80ECCFAC66C0050A16F67A3C90D60A57D5BBEEC13C670C5AA8DA`.
- ELF: `Build/CM4_FOC/MMEk_Demo_CM4_FOC.elf`, SHA-256
  `BBEA35CE53BDFC7D585123C2DC744242311B66D35B70EAE36D5E4517E60FFA6F`.
- Built from the working tree including pre-existing unrelated M4 changes;
  the scoped source commit alone is not a clean-tree reproduction snapshot.
  The preceding authorization/baseline record is commit `2478141`.
- Firmware output is regenerated; no CM0+ build, device flash or A2L change.
- Powered Monitor/Apply, current harmonic improvement, start/stop/reversal,
  FWC coexistence, DWT Last/Max/Overrun and board readback remain NOT_RUN.
  Apply is enabled by explicit request, not by claiming those gates passed.
