# RRC-DOB calibration range-check removal

For the subsequent output-sign calibration and latest firmware, see
[SIGN_TEST.md](SIGN_TEST.md). The bounds-removal behavior below is retained.

## Baseline and scope

- Source baseline: `fec20d6bac27fe07da172819ffe0bb123d23dede`.
- User requests removal of parameter-invalid blocking logic.
- Remove engineering calibration bounds for R/L, pole pairs, cutoff ratio,
  output magnitude, speed window and ramp duration. Do not change defaults,
  stopped-only snapshots, estimator ownership or compensation equations.
- Retain existing interface/numeric requirements: non-null pointers, defined
  selector, nonnegative bandwidth/output magnitude, computable ramp division
  and representable fixed-point coefficients. These are not tunable bench
  range limits. Rejected numeric/interface inputs can still report Stat=5.
- Preserve runtime speed qualification (Stat=6), previous-voltage freshness
  (Stat=7), arithmetic overflow checks (Stat=8), output limits and OFF bypass.
- Accepting a broader calibration snapshot does not validate its dynamics or
  extend the LUT's 375 Hz electrical runtime domain at 100 us.
- Baseline Apply HEX SHA-256:
  `724701C908BE80ECCFAC66C0050A16F67A3C90D60A57D5BBEEC13C670C5AA8DA`.
- No uncommitted changes in the affected algorithm files; unrelated user
  work remains untouched. No A2L change or board flashing is requested.

## Removed and retained behavior

Removed calibration-time gates: R 10--2000 mOhm, Ld/Lq 200--5000 uH,
pole pairs 1--16, cutoff ratio 1638--6554 Q15, output limit 1--3277 Q15,
nonzero/ordered speed bounds and the 10000 rpm upper limit, ramp 1--1000 ms,
and the 10--375 Hz electrical equivalent speed-window check at 100 us.
Out-of-range snapshots are now stored without clamping to those limits.

The numeric checks are retained, not bypassed: L=0 cannot prepare coefficients;
ramp=0 cannot produce a defined ramp step; negative cutoff/output magnitude,
undefined selector, null pointer and unrepresentable coefficients still fail.
Current Q formats and the existing reciprocal implementation also retain their
coefficient-domain constraints. Thus Stat=5 is not removed entirely. Motor
calibration upstream has not been changed by this RRC-local modification.

Actual angle increments are still qualified against the stored speed window
and the LUT maximum. A window above 375 Hz can be accepted, but operation
above that LUT domain still reports Stat=6. Inverted/zero-speed windows may
similarly leave the observer inactive; snapshot acceptance is not validity
of a physical operating point. No runtime eligibility policy is added.

## Software evidence

- 100 us Apply arithmetic and period regression: PASS (IAR 9.40.1).
- 17 independent acceptance cases cross the former engineering bounds;
  all are accepted unchanged. Numeric/interface rejection cases also pass.
- A 5626 rpm / four-pole-pair window is accepted; actual operation beyond
  375 Hz still reports Stat=6 rather than indexing outside the LUT.
- Existing 3200-row MATLAB-vector replay: PASS; disturbance RMS 0.286775639,
  peak 0.5021515, applied-command peak 0.943298 Q15 LSB. MATLAB vector
  generation was not rerun. This covers the previous valid operating domain,
  not stability over newly admitted calibration values.
- OFF/Monitor/Apply, stopped snapshots, FWC eligibility, voltage capture and
  ADC-gap production-function regression: PASS. Both test builds have zero
  compiler/linker errors and warnings. Test doubles and pending bench scope
  remain as described in APPLY100US.md.
- Raw outputs: Build/RRCDOB100usTests/rrc_100_apply1_result.txt and
  Build/RRCDOB100usTests/integration_result.txt (not committed).

## CM4 build and provenance

- IAR CM4 `Multi Motor Evalkit V1.0`: PASS, 0 errors and 15 warnings.
- HEX `Build/CM4_FOC/MMEk_Demo_CM4_FOC.hex` SHA-256:
  `BF5E162CAFE93FFFF59B030EE5B72A9D5E6EAACE534D543B17F762C45DA93269`.
- ELF `Build/CM4_FOC/MMEk_Demo_CM4_FOC.elf` SHA-256:
  `BF1CB273394A6F3B51584476294BCC7A06ED1BC3CF2158667F95FB791ADA3A5A`.
- Baseline record commit: `fbab698`. Unrelated working-tree changes were
  included by the build but are not staged in the scoped algorithm commit.
- Apply remains compiled in; runtime selector remains OFF by default.
  All source calibration defaults are unchanged from APPLY100US.md.
- No board test, flash, DWT measurement or new operating-domain validation
  is claimed. No CM0+ rebuild or A2L edit was performed.
