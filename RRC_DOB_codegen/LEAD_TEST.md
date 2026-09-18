# RRC-DOB output lead experiment

## Baseline before implementation

- Algorithm source: `45353565e9aae957877cc0c4a5f9312544a94399`.
- Existing CM4 HEX SHA256: `7F4725C086F30BBB055483B049929F206266775E24AACD3CAD88293B1FF78120`.
- Existing map: code 42468 B, readonly data 17175 B, readwrite data 14300 B;
  RRC state 300 B. These are the pre-change artifacts, not a clean-tree build.
- Preserve unrelated worktree changes, including the user-edited A2L.
- Control period 100 us, PWM period 50 us. KRE VI_fb delay, main FOC angle,
  PI, PWM scheduling and existing fault policy are outside this experiment.

## Approved experiment contract

For each axis independently, with raw disturbance estimate x[k],
Omega = 6 * electrical angle increment (radians), phi = Omega * Lead_us / Ts_us:

    y[k] = sin(Omega + phi)/sin(Omega) * x[k]
         - sin(phi)/sin(Omega) * x[k-1]

Coefficients are Q16.16; multiply/accumulate uses int64. Reuse the existing
Q15 sine table with interpolation. Refresh at the existing RRC coefficient
boundary. The zero-frequency limit is A=1+Lead/Ts, B=Lead/Ts.
Order: raw estimate -> lead -> existing ramp -> Rev -> existing limit -> PI sum.
No prediction is fed back into the DOB states. First valid sample passes
through; existing resets clear both independent histories. Lead=0 bypasses.

Add default-zero Cal_RRCDOB_Lead_us_u16 and applied-snapshot
Meas_RRCDOB_Lead_us_u16, in the existing XCP section. Use stopped-only parameter
handoff and existing Pend semantics. Do not edit the A2L or main SLX.

Acceptance: mathematical reference then fixed-point replay; <=1% amplitude
and <=1 degree phase error for steady sixth harmonic in the default speed
window at 0/25/50/75/100/150/200 us; zero-lead bit equality; reset, frequency
change, both signs, saturation, Monitor/Apply and parameter races; CM4 build.
This FIR is narrowband and does not preserve arbitrary DC/noise harmonics.

Hardware acceptance is NOT_RUN: phase-current FFT/raw traces, independent
speed evidence, repeated starts/stops/speed changes, DWT Last/Max/Overrun and
>=20% WCET margin. Keep an identical calibration/load and test 0,25,50,75,100 us
with Rev=0, stopping and confirming Pend=0 between changes. Do not interpret
the trial lead as a measured hardware delay.

## Implemented interface and calibration

| Symbol | Meaning | Default / application |
| --- | --- | --- |
| `Cal_RRCDOB_Lead_us_u16` | Sixth-harmonic output advance in microseconds | 0; stopped-only snapshot |
| `Meas_RRCDOB_Lead_us_u16` | Last successfully applied advance, not the pending request | 0; retained across dynamic resets |

Both are `NO_OPT volatile uint16_t` in `.xcp_cal_m4`. The internal parameter
member is `RRCDOB_Parameters.leadTime_us`; existing C APIs and selector meanings
are unchanged. All parameter constructors/comparisons and the masked recheck
include it. No engineering range gate was added. Numerical software acceptance
covers 0..200 us at the listed points; uint16 representation is NOT a validated
bench operating range. The existing arithmetic/status behavior is retained.

- `Lead=0`: no lead coefficient calculation, prediction, or history update;
  the original output calculation remains in effect.
- Nonzero: shared coefficient refresh every 500 us, independent raw-Hat history
  per d/q axis. Reset, stale voltage, speed-window exit, and existing large
  frequency-change resets clear both histories. The first valid output uses
  raw Hat before the existing ramp/sign/limit.
- `VdHat/VqHat` remain the original estimates. `VdOut/VqOut` are the final
  correction candidates (also visible in Monitor). For nonzero Lead, do NOT
  expect instantaneous `Out = +/-Hat`; that sign-only relation assumes Lead=0
  and completed ramp without saturation.
- Running writes remain pending, including during rampDown. Stop AND disable
  control; confirm `Pend=0` and applied Lead before restarting. Stable OFF
  deliberately skips numeric scans: a Lead write while both selectors are OFF
  is incorporated when the selector is next enabled while stopped.
- No change to KRE, Park/FOC angle, current PI, FWC, PWM timing, A2L or SLX.
  KRE continues to receive the previous saved modulator-limited voltage;
  its existing internal complete-VI delay remains intact.

### 台架操作

1. 使用下述新固件及匹配 ELF 更新标定工具地址；本任务没有修改 A2L。
   不能沿用旧固件的变量地址，包括原有 RRC 标定/观测量。
2. 保留已能运行的启动电流、负载、电机参数、WcRatio、Ramp 和 OutHi，
   记录完整标定快照、启动路由和命令源。固定 Rev=0，Lead=0。
3. 先测 Sel=1（Monitor）；再测 Sel=2、OutHi=0（输出应为零）；
   然后恢复原试验 OutHi，Sel=2、Lead=0 建立补偿基线。每次停机修改。
4. 停机且控制关闭时依次标定 Lead=25、50、75、100 us，确认 Pend=0、
   Meas_Lead 等于目标值。运行时确认 Sel=2、Stat=3、Valid=1、OutAct=1。
   首轮无改善，不自动继续增加超前时间或补偿幅度；回到 Lead=0 对比。
5. 200 Hz 电频率下六次谐波为 1200 Hz，上述超前对应
   10.8/21.6/32.4/43.2 度。它们是试验值，不是实测系统延迟。
6. 同步记录相电流原始采样（注明采样率/窗口）、FFT、5/7 次谐波、THD、
   转速指令/反馈、Iq_ref/Iq_fb、Hat/Out、Stat/Valid/OutAct/Sat。
   用独立测速验证实际速度；若没有，明确记录实际转速未独立确认，
   不把 KRE 3000 rpm 和 RRC 200 Hz 当作两项独立证据。
7. 比较 FastLoop/PwmIrq Last/Max、Overrun 和 IdleSvcCount。
   当前 FastLoop:SpeedLoop 应约 5:1，PwmIrq:SpeedLoop 约 10:1。
   IRQ 的既有截止时间是 50 us，不是控制步长 100 us；要求 IRQ Max
   不超过既有周期预算的 80%、Overrun 增量为 0、前台持续推进。
   不满足则本次台架验收不通过，不修改保护逻辑来掩盖结果。
8. 通过重复启停、变速及所需运行方向测试后再确认台架有效。

## Software evidence (2026-09-17)

- MATLAB MCP: `rrcDobLeadReferenceTest`, 39 PASS / 0 FAIL / 0 incomplete.
  Independent double-precision reference, harmonic identity, first/reset sample,
  zero-frequency extension, zero lead and non-unity DC behavior.
- IAR 9.40.1 C-SPY: actual production sine table, 903 coefficient cases per
  period (800..4000 rpm, 4 pole pairs, step 25 rpm; seven Lead values).
  At 100 us: maximum fractional gain error 0.000126654249 (0.0126654%), phase
  0.00261181254 degrees, quantized waveform error 2.64980487 Q15 LSB at amplitude
  4096. At 50 us: 0.0295654%, 0.00633910376 degrees, 4.73632789 LSB.
  Both satisfy the predefined 1% / 1 degree thresholds. C uses its own analytic
  double oracle; these lead results are not a plant simulation or ERT replay.
- 2400 paired lead/zero-lead steps per tested build: identical raw estimates
  and original DOB states, independent axes, both signs, Monitor/Apply, first
  sample/ramp, OutHi=0/1/3277, stale/reset/re-entry/frequency reset and refresh
  cadence. Final Apply testing also checks Q15 saturation after PI summation.
- Existing 3200-row MATLAB-derived DOB replay at 100 us retained: RMS
  0.286775639, peak 0.5021515, Apply command peak 0.943298 Q15 LSB. The pre-change
  output/state fingerprints remain `4762d8cb` (Apply compiled in) and
  `51054257` (Apply compiled out). Default-zero behavior matches this baseline.
- Production-function integration fixture: pending writes while running and
  rampDown, stopped snapshot, lead/sign recheck races, stable-OFF bypass,
  Monitor/Apply and existing capture/ADC/FWC paths. Static source assertions
  retain the modulator voltage -> KRE route and call order. PI/PWM/observer
  plant timing is not simulated by this fixture.
- Test development found two harness issues: the original 10 s simulator
  limit was too short for the new sweep (now 60 s), and C-SPY can print failed
  assertions yet exit zero. Both runners now reject failed-assertion text,
  not just process exit / final success text. A reverse-step recovery test
  initially advanced two samples outside the existing angle domain; its input
  sequence was corrected and rerun. These are test changes, not firmware guards.
- Full CM4 IAR rebuild, configuration `Multi Motor Evalkit V1.0`: 0 errors,
  15 existing warnings outside the new algorithm. No generated model changes.

| Memory (map bytes) | Before | After | Delta |
| --- | ---: | ---: | ---: |
| Readonly code | 42468 | 42996 | +528 |
| Readonly data | 17175 | 17175 | 0 |
| Readwrite data | 14300 | 14320 | +20 |
| RRC internal state (included above) | 300 | 316 | +16 |

Artifacts (full current worktree build, including existing unrelated user changes):

- `Build/CM4_FOC/MMEk_Demo_CM4_FOC.hex` SHA256:
  `C77D5AF217324EA78A845882AC65DB7685565F2BB3E604CECFC4F3E00D9DD5BC`
- `Build/CM4_FOC/MMEk_Demo_CM4_FOC.elf` SHA256:
  `AB2DC8042B8AA89CA8BE942619D9F417D9AC30FFE293CAD83375343D67A5DD79`
- Map for this exact artifact only: Cal Lead `0x0800D378`, Meas Lead
  `0x0800D394`, 2 bytes each. Prefer importing symbols from the matching ELF.

Hardware flashing, powered current/THD results, independent speed verification,
DWT/WCET acceptance, startup/stop/reversal regression: **NOT_RUN**.
Nonzero Lead remains an experimental calibration, not a proven fix for 160 Hz.
