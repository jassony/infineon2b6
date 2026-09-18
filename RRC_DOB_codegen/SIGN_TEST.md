# RRC-DOB output sign calibration

## Baseline and requested change

- Baseline source: `f283629d4260154f8ea523d299da348ea3485b92`.
- User reports less sinusoidal current with Apply enabled and requests a
  calibration to reverse compensation for physical comparison. This report
  is not proof that the original sign is incorrect.
- Baseline HEX SHA-256:
  `BF5E162CAFE93FFFF59B030EE5B72A9D5E6EAACE534D543B17F762C45DA93269`.
- Add only `Cal_RRCDOB_Rev_u8` (uint8, NO_OPT volatile, .xcp_cal_m4).
  Default 0 retains correction=-ramp*estimate; nonzero selects
  correction=+ramp*estimate. Both d/q axes reverse together before the
  existing symmetric output limit. No independent per-axis gain is added.
- This changes only correction generation, not the observer equations,
  measured-current sign, voltage-capture sign or coordinate transforms.
  In closed loop the changed command naturally affects subsequent inputs.
- Include the setting in the existing stopped-only coherent parameter
  snapshot, equality check and interrupt-masked recheck. Changing it resets
  the observer through the existing parameter-change path. Default OFF and
  current Apply/Monitor selectors remain unchanged. No new protection logic.
- No A2L, main-model edit or flashing. Existing unrelated changes preserved.

## Planned verification

Check default-sign 3200-row replay, same-input estimate/state identity with
opposite corrections, both axes, ramp/limits/zero limit, Monitor unchanged
voltage, Apply corrected voltage, stopped snapshot and concurrent-write
recheck. Run IAR CM4 build and record artifact hashes. Physical current-ripple,
FWC interaction and DWT timing comparison remain bench work, not software PASS.

## Calibration procedure

Stop and disable control before each direction change. Use Sel=2 for Apply,
set `Cal_RRCDOB_Rev_u8=0` for the original sign or 1 for reversed sign, then
wait for `Meas_RRCDOB_Sel_u8=2` and `Meas_RRCDOB_Pend_u8=0` before starting.
The full snapshot is applied by foreground service while stopped. No new
applied-sign measurement is added: existing Pend and VdOut/VqOut suffice for
the comparison. Stable OFF still defers numeric/sign scanning until enable
or reset is requested. Values other than zero also select reversed sign;
use only 0/1 for clear test records.

For eligible operation, Stat=3 and OutAct=1 indicate Apply ownership. In
Monitor (Sel=1), the candidate correction changes sign but voltage injection
remains disabled. Before symmetric limit clipping:

| Rev | Candidate correction | Apply voltage |
| --- | --- | --- |
| 0 (default) | -ramp * Hat | PI - ramp * Hat |
| 1 | +ramp * Hat | PI + ramp * Hat |

Keep speed/load/bus voltage and all other calibrations identical when comparing
phase-current waveforms. Record Stat/Valid/OutAct/Sat and Hat/Out on the same
timebase. Reverse-sign improvement alone does not distinguish a sign mistake
from phase/timing/model error. Source defaults remain Sel=0, Rev=0, Rst=0,
WcRatio=3277, OutHi=3277, SpdLo=800 rpm, SpdHi=4000 rpm, Ramp=10 ms.

The new symbol must be added manually to calibration tooling against the new
ELF if needed. No A2L file is edited automatically.

## Executed software checks

- 100 us Apply period/calibration regression: PASS, including the prior 17
  range-removal cases and retained interface/numeric checks.
- Sign comparison: PASS, 1200 same-input/state pairs across Monitor/Apply and
  limits 3277/1/0 Q15. Both d/q corrections are exact negatives, including
  ramp and symmetric limit behavior; observer estimates and next axis states
  are identical. Nonzero values 1 and 255 both select reverse. Parameter
  sign changes reset the existing observer/ramp state.
- Original-sign MATLAB-vector replay: PASS, 3200 rows. Disturbance RMS
  0.286775639 / peak 0.5021515 Q15 LSB; command peak 0.943298 Q15 LSB.
  Existing reference vectors were reused, not regenerated.
- Production-function integration: PASS, including sign-write pending while
  running, stopped application, interrupt-masked sign-change race recheck,
  stable-snapshot no-op and previous OFF/Monitor/Apply/FWC/capture/ADC tests.
- Both IAR test compiles/links: zero errors and warnings. Test doubles remain
  in use; these are not motor-loop stability or timing measurements.
- Raw outputs (not committed): Build/RRCDOB100usTests/rrc_100_apply1_result.txt
  and Build/RRCDOB100usTests/integration_result.txt.

## Firmware delivery

- IAR 9.40.1 CM4 `Multi Motor Evalkit V1.0`: PASS, 0 errors, 15 warnings.
- HEX: Build/CM4_FOC/MMEk_Demo_CM4_FOC.hex, SHA-256
  `7F4725C086F30BBB055483B049929F206266775E24AACD3CAD88293B1FF78120`.
- ELF: Build/CM4_FOC/MMEk_Demo_CM4_FOC.elf, SHA-256
  `985B5215CB0D927BBB45D236EF1BBE2CB4A0FA01D19DCE843ABBA5566E798201`.
- Link map confirms Cal_RRCDOB_Rev_u8 is retained as a one-byte symbol.
  Use this ELF to resolve calibration addresses; A2L has not been modified.
- Baseline record: `9f37fca`. Build includes pre-existing unrelated worktree
  edits; only RRC-related source, tests and records are committed here.
- No CM0+ build, device connection, flashing or automatic calibration write.
  Physical forward/reverse-sign comparison, FWC interaction and DWT
  Last/Max/Overrun remain NOT_RUN. No claim of improved motor performance.
