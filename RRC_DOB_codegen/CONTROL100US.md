# RRC-DOB 100 us integration record

This is the historical **Apply-locked** integration record. The subsequent
user-requested Apply-enabled configuration is recorded in
[APPLY100US.md](APPLY100US.md); the locked artifact hashes below are retained
as baseline evidence, not the current Apply-enabled image.

## Baseline before integration

- Source baseline: `41a59a06a52681d7f9dc9eae4e9da552fb0415e0`.
- User reported the motor running with `Meas_MCU_TotalLoad_pct_f32 = 53%`.
  This is user-reported evidence, not an automated bench measurement.
  Operating point, raw trace, live calibration, ISR Last/Max/Overrun, and
  board firmware readback were not supplied; these gates remain NOT_RUN.
- Local baseline CM4 HEX SHA-256:
  `FB3A61037C292FAAC21B094D2215DDCD240A0303BD2036CF99098C7FFC288342`.
- PWM 20 kHz, control/KRE 10 kHz, speed/FWC 2 kHz. KRE remains the estimator.
- Main Simulink model, A2L, command-source/startup defaults and PWM hardware
  configuration are outside this change.

## Algorithm contract and intended retiming

Source of truth: existing `README.md` equations and the parameterized
`rrcDobDiscreteReferenceStep.m`, checked against `rrc_dob_compensator.c`.
This is a d/q voltage compensation algorithm after the current PI, not a new
angle/speed estimator. The motor is IPMSM, with separate Ld/Lq. Keep the
existing off/monitor/apply selector values and default off.

The prewarped Tustin observer uses h=Ts/2, q=tan(6*pi*fElectrical*Ts),
r=R*h/L, kv=h*Vbase/(L*Ibase), lambda=L*Ibase/(Vbase*Ts), rho=R*Ibase/Vbase.
At 100 us, r/kv double, lambda halves and rho is unchanged. State/output
order and Q formats remain the existing contract. Refresh every 0.5 ms and
preserve ramp time in milliseconds. Capture modulator-limited voltage after
each control calculation for consumption on the following control sample;
hold the command through the intervening PWM cycle.

The existing fixed-point core squares q in signed Q30. Its represented LUT
domain is q <= tan(0.225*pi), with maximum electrical angle increment
0.0375 cycle/sample. Keep that domain: maximum electrical frequency becomes
375 Hz at 100 us (750 Hz at 50 us). Default 800--4000 rpm with four pole
pairs remains inside the domain; unsupported parameter ranges must reject
explicitly. Do not reuse the 750 Hz physical range at 100 us.

Predeclared numerical gates: finite state/output, discrete poles inside the
unit circle in the admitted parameter range, C-versus-MATLAB disturbance
error <= 2 Q15 LSB RMS and <= 8 Q15 LSB peak on deterministic replay.
Include reset, ramp, limits, out-of-range/reverse speed, stale applied voltage,
frequency changes, OFF bypass and parameter handoff. Bench timing remains
50 us deadline / 40 us WCET target; 53% average load does not establish WCET.

## Delivered configuration (2026-09-16)

Status: software integration/replay/build PASS; powered Monitor acceptance
NOT_RUN; Apply voltage ownership LOCKED. The repository integration contract
requires Monitor before Apply. With no answer yet to the weak-field policy
question, this version preserves FWC and does not enable combined compensation.

- Independent `FOC_RRCDOB_ENABLE=1`, `FOC_RRCDOB_APPLY_ENABLE=0`.
- `FOC_AUX_ALGORITHMS_ENABLE=0`: HFIPD, HFI, APSFSM, VAFID and the separate
  dead-time compensation remain excluded. RRC-DOB is not an estimator;
  fixed KRE ownership and the existing startup/command defaults are unchanged.
- OFF bypasses algorithm execution, capture, numeric calibration scans and
  repeated resets. Monitor leaves the raw PI voltage output unchanged.
- Coefficients refresh every five control samples; a 10 ms ramp takes 100
  control samples. Reverse speed remains outside the existing RRC contract.
- PWM submission is checked before capturing the modulator-limited voltage
  and its Park angle. The next control sample consumes this previous capture;
  the intervening PWM cycle holds duty/trigger values. A missing ADC sample
  clears RRC history once, preventing a two-sample angle delta from being
  interpreted as one sample. This software command timing is not a measured
  inverter voltage or proof of hardware latch timing.
- Parameter/mode/reset writes become pending during run/ramp/control enable.
  Foreground service applies a coherent snapshot only while stopped, with
  an interrupt-masked recheck. Preparation is inside that stopped critical
  section; its interrupt latency still needs bench measurement.
- Stable OFF defers numeric-parameter scanning until mode enable or reset is
  requested. `Meas_RRCDOB_Pend_u8=0` in OFF does not mean every numeric XCP
  value has already been copied. Invalid snapshots publish parameter-invalid
  status and an applied selector of OFF; they do not disable the FOC startup.
- Main SLX, generated 50 us wrapper assets and A2L were not edited. The
  handwritten C adapter was compared with the existing parameterized MATLAB
  reference. No updated wrapper-code-generation claim is made.

## Calibration and operation

Source-default snapshot (not a board readback):

| Symbol | Value |
| --- | --- |
| `Cal_RRCDOB_Sel_u8` | 0 (OFF) |
| `Cal_RRCDOB_Rst_u8` | 0 |
| `Cal_RRCDOB_WcRatio_Q15_s16` | 3277 |
| `Cal_RRCDOB_OutHi_Q15_s16` | 3277 |
| `Cal_RRCDOB_SpdLo_rpm_u16` | 800 |
| `Cal_RRCDOB_SpdHi_rpm_u16` | 4000 |
| `Cal_RRCDOB_Ramp_ms_u16` | 10 |

Motor snapshot uses applied motor calibration: default R=500 mOhm,
Ld=1300 uH, Lq=1380 uH, four pole pairs; voltage/current bases 1000 V/50 A.

For Monitor, stop the motor and disable control, set `Cal_RRCDOB_Sel_u8=1`,
then wait for `Meas_RRCDOB_Sel_u8=1` and `Meas_RRCDOB_Pend_u8=0` before
starting. At valid positive speed, expect `Stat=2`, `Valid=1`, `OutAct=0`.
Observe `Meas_RRCDOB_VdHat_Q15_s16` and `Meas_RRCDOB_VqHat_Q15_s16`.
Set selector 0 while stopped to return to OFF. Selector 2 in this build
reports `Stat=4` when otherwise valid and always `OutAct=0`: it does not
inject compensation. Apply remains a separate bench-gated follow-up.

## Software evidence

| Check | Result and scope |
| --- | --- |
| MATLAB full-state and Q27-matrix poles | PASS, 144 cases; maximum pole magnitude about 0.9998; sampled grid, not proof over every calibration |
| 100 us MATLAB/C replay | PASS, 3200 rows, disturbance RMS 0.286776 / peak 0.502152 Q15 LSB (limits 2 / 8) |
| Apply-enabled standalone arithmetic | PASS, same replay; voltage command peak error 0.943298 LSB; not a production Apply or bench test |
| Period/ramp/limits/reset/stale/reverse/invalid/OFF | PASS at 100 us locked and test-only Apply; 50 us Apply unit regression PASS |
| Production-function integration with dependency doubles | PASS: OFF bypass, Monitor unchanged voltage, FWC eligibility, delayed capture, ADC-gap reset, stopped snapshots and race/PRIMASK checks |
| KRE 100 us / current PI | PASS, 1800-row replay, hash `bb309250`; PI scaling/anti-windup PASS |
| FWC speed recovery integration | PASS, 12037 checks |
| Bare-KRE startup dispatch / stop current | PASS; startup fixture explicitly compiles RRC out; separate RRC integration suite above covers its call sites; stop suite 30 checks |
| PWM divider / held duty / ADC / ISR accounting | PASS; four timing compile-guard cases |
| IAR 9.40.1 CM4 build | PASS, 0 errors, 15 warnings (unused symbols/variables and enum mixing; see build log) |

Arithmetic replay uses a coordinate-transform test double for preconstructed
d/q inputs. Integration tests double PI, PWM hardware and observer math.
Neither proves target execution time, physical phase accuracy, current-ripple
reduction, motor stability or FWC/Apply coexistence. Link map contains RRC
execute/capture symbols; excluded auxiliary execute symbols are absent.

Reproduce from the repository root:

```matlab
addpath('RRC_DOB_codegen/tests');
generate_rrc_control100us_vectors();
```

```powershell
& RRC_DOB_codegen/tests/run_rrc_control_period_tests.ps1 -ControlPeriodUs 100 -ApplyEnable 0
& RRC_DOB_codegen/tests/run_rrc_control_period_tests.ps1 -ControlPeriodUs 100 -ApplyEnable 1
& RRC_DOB_codegen/tests/run_rrc_control_period_tests.ps1 -ControlPeriodUs 50 -ApplyEnable 1
& RRC_DOB_codegen/tests/run_rrc_foc_integration_tests.ps1
& KRE_codegen/tests/run_kre_external_observer_adapter_tests.ps1 -Optimization '-Om' -ControlPeriodUs 100
& FWC_codegen/tests/run_fwc_speed_recovery_integration_tests.ps1
& MS/tests/run_foc_control10k_startup_tests.ps1
& MS/tests/run_foc_stop_current_tests.ps1
& MS/tests/run_foc_control_rate_tests.ps1
& D:/APP/iar9401/common/bin/iarbuild.exe IAR/cm4_mc/MMEk_Demo_CM4_FOC.ewp -build 'Multi Motor Evalkit V1.0' -log all
```

Local raw outputs: `Build/RRCDOB100usTests/`, `Build/rrc_*.log`.
Build output/caches are intentionally excluded from commits.

## Firmware and provenance

Local pre-integration record commit:
`4c40b78b4eed21567fa60a47fc6b1ab3ecc65637`.
The bare 10 kHz rollback image is archived under `Build/RRCDOB_Baseline10k/`.
The new image was built from this working tree, including existing unrelated
M4 task/watchdog modifications; those modifications are preserved, not staged
in this algorithm commit. The commit alone is not a complete clean-tree image
reproduction snapshot.

- New CM4 HEX: `Build/CM4_FOC/MMEk_Demo_CM4_FOC.hex`, SHA-256
  `E331D100D17EE2E0570082BE85A69B071EFB5CBB58FBCA4B7CF2FBF65E1A6B6B`.
- New CM4 ELF: `Build/CM4_FOC/MMEk_Demo_CM4_FOC.elf`, SHA-256
  `F5C8FBA3126D3270E22359C6E93CFAE4A40828A6F59212100C260DED70CE6CC6`.
- CM0+ companion artifact hash retained from the bare-10k delivery:
  `9D94C21A3709EDA1D1AB44CFF47E821030CAEF82ECC65501D30FA1D7571FF78D`.
  CM0+ was not rebuilt or flashed; board version/readback remains unverified.

## Manual acceptance template

Date/operator/board/motor: NOT_RUN. CM4/CM0+ readback hashes: NOT_RUN.
Record live calibration, command source, startup route, DC bus, speed, load,
temperature and raw trace location before comparing the same operating point.

| Item | Bare 10 kHz | RRC OFF | RRC Monitor |
| --- | --- | --- | --- |
| Total load | 53%, user report; operating point unknown | NOT_RUN | NOT_RUN |
| Fast-loop load | NOT_RUN | NOT_RUN | NOT_RUN |
| Complete ISR DWT Last/Max | NOT_RUN | NOT_RUN | NOT_RUN |
| Control-step DWT Last/Max | NOT_RUN | NOT_RUN | NOT_RUN |
| ISR overruns (must be zero), Max <=40 us, deadline 50 us | NOT_RUN | NOT_RUN | NOT_RUN |
| PWM/control/speed slopes 20k/10k/2k; foreground progress | NOT_RUN | NOT_RUN | NOT_RUN |
| Start, speed change, stop, reverse, repeated start | NOT_RUN | NOT_RUN | NOT_RUN |
| FWC entry/exit and speed recovery | NOT_RUN | NOT_RUN | NOT_RUN |
| RRC estimates/validity, angle, Iq reference/feedback, current response | NOT_RUN | NOT_RUN | NOT_RUN |
| ADC misses, capture/hold phase, parameter-change interrupt latency | NOT_RUN | NOT_RUN | NOT_RUN |

Apply is not enabled by this record. It requires powered Monitor evidence and
an explicit FWC interaction policy before its own voltage-ownership validation.
