# FWC voltage-aware field weakening

`fwc_q15_adapter.c` is the production 2 kHz field-weakening controller. It
does not own the estimator, startup route, CAN command source, or power stage.
It is eligible only for internal-PI, closed-loop bare FOC. HFI, RRC-DOB,
VAFID, APSFSM, direct-current control, an external speed controller, or forced
duty leave the existing control path unchanged and report FWC as ineligible.

## Discrete controller

The 20 kHz PWM path publishes a coherent post-modulator snapshot. At the 2 kHz
boundary the controller uses

```
VutilReq = sqrt(3) * Vreq / Vdc
VutilAct = sqrt(3) * Vact / Vdc
alpha = 0.5 / (LpfTau_ms + 0.5)
VreqFlt += alpha * (VutilReq - VreqFlt)
VactFlt += alpha * (VutilAct - VactFlt)
Hi = VutilTgt + Hyst
Lo = VutilTgt - Hyst
e = clamp(VreqFlt - Hi, 0, 1) + clamp(VreqFlt - Lo, -1, 0)
u = clamp(Kp * e + xi, 0, -IdLo)
xi(k+1) = xi(k) + Ki * 500 us * e
```

Integration is conditionally held when `u` is already at the boundary and the
error would drive it farther into that boundary. `u` is rate limited at
`IdDnRate` while adding negative Id and `IdUpRate` while releasing it. The
absolute FWC result is combined with IdMap using the more-negative value and
then limited by the 1 PU current circle with d-axis priority.

`Cal_FWC_IdLo_Q15_s16` is constrained to `[-13107, 0]`, which is `[-20 A,
0 A]` at the configured 50 A base current. The default target is 0.95 PU,
with `Kp=0.5`, `Ki=40`, a 4 PU/s weakening rate, and a 1 PU/s release rate.

The default LPF time constant is 5 ms. The first eligible valid sample seeds
both filter states directly; 0 ms bypasses the filter. The original
`Meas_FWC_VutilReq_PU_f32` and `Meas_FWC_VutilAct_PU_f32` remain raw values.
The control uses its own private filter states, not the RTE observation LPF.

`Cal_FWC_Hyst_PU_f32=0.02` gives a 0.93--0.97 PU deadband at the default
target. An inactive weakening latch requires a continuous `VreqFlt > Hi`
for `Cal_FWC_Enter_ms_u16=2` ms (four 2 kHz samples). The PI runs only while
this latch is active and holds its integrator inside the deadband. Exit
requires `VreqFlt < Lo`, FWC-only Id magnitude <=1 Q15, and no recovery
governor for `Cal_FWC_Exit_ms_u16=20` ms (40 samples). Exit clears the FWC
integrator and Id, but never clears a negative IdMap command.

The new XCP calibrations are `Cal_FWC_LpfTau_ms_f32` (0--100 ms),
`Cal_FWC_Hyst_PU_f32` (0--min(0.05,target,1-target)),
`Cal_FWC_Enter_ms_u16` (0--100 ms), and `Cal_FWC_Exit_ms_u16` (0--1000 ms).
Nonfinite LPF/hysteresis inputs use 5 ms/0.02 PU before clamping. Durations
are integer milliseconds, exactly two ticks/ms; zero confirms immediately.
Effective threshold/duration changes restart the affected confirmation count;
LPF time-constant changes preserve filter/PI history. Disable, ineligibility,
invalid input, and reset clear all new state at the speed boundary.

New observations are `Meas_FWC_VreqFlt_PU_f32`, `Meas_FWC_VactFlt_PU_f32`,
`Meas_FWC_WeakAct_u8`, and `Meas_FWC_IdFw_Q15_s16`. `WeakAct` denotes the
weakening latch, whereas existing `Meas_FWC_Act_u8` still denotes ownership
of the reference path. Existing status codes 0--5 retain their meanings.

## Saturation recovery

The fast path compares the requested/actual gap in voltage-utilization units,
not raw voltage units. Once FWC has commanded the Id floor, it counts 40
consecutive saturated 20 kHz samples. At recovery entry the 2 kHz integration
latches `min(abs(effective reference), abs(estimated speed))` as a fixed speed
cap. The normal input-limited command is clamped to +/- this cap BEFORE the
existing acceleration/deceleration limiter. The ramp and PI are not reseeded;
feedback noise cannot move the cap, and lower commands remain effective.
Recovery release preserves the ramp/integrator and restores the raw command.
Release requires 40 consecutive nonsaturated fast samples
AND `(VreqFlt - VactFlt) <= 0.5 * SatEps` continuously for `Exit_ms` (default
20 ms) at the 2 kHz boundary. Any interruption restarts confirmation. The raw
application command then resumes through the existing limiter and ramp.

The speed PI continues to regulate the real speed error. While recovering,
`fwc_speed_recovery.h` filters measured Iq with a fixed 20 ms time constant
(`alpha=0.5/20.5`), seeded from the pre-entry Iq command. When error and
`Iq_request-Iq_filtered` have the same sign, outward integral accumulation is
held and the next integral state is corrected by existing speed `Kaw*Ts`,
clamped to [0,1]. The Q24 integral is bounded by the PI current limits.
Opposing error remains free to unwind saturation. The current-cycle Iq
output is never assigned from measured current. Disable, invalid input,
ineligibility and stop clear recovery state at the speed boundary.
See [RECOVERY_FIX.md](RECOVERY_FIX.md) for evidence and verification scope.

The fast path also feeds the actual/requested radial voltage scale to the Id
and Iq PI integrators using their existing `Kaw*Ts` values and
`Cal_FWC_CurAwGain_PU_f32`. The new global PI configuration selects Back
Calculation (`IFX_MATH_CFG_PI_ANTI_WIND = 2`).
This actuator back calculation remains unfiltered on the 20 kHz path; the
new filter and debounce logic add no fast-path operations.

## Validation artifacts

`fwc_reference_step.m` is the explicit single-precision 500 us reference.
`fwc_reference_wrapper.slx` is its independent fixed-step ERT wrapper; its
generated C sources are validation-only and are deliberately not linked into
the production IAR project. `run_fwc_reference_replay.m` and the two scripts
under `tests/` cover the fixed vector, FWC floor, enable/invalid paths, current
circle, recovery entry/exit, and generated-C replay. `FwcReferenceTest` uses
eight MATLAB unit tests, including a shared eight-sample golden vector.
The generated-C runner additionally compares the production Q15 adapter to
ERT every cycle over 1400 quantized-input samples, including online tuning,
IdMap, invalid input, disable, and interrupted recovery. Allowed differences
are 1 Q15 count for Id and 1e-6 PU for the two filtered voltage signals;
activity/recovery/status values must match exactly.

## RAM and validation record (2026-09-09)

Baseline: `5ccb01d86428495d2e775a33da7f73db77ffd771`; the FWC subtree had
no uncommitted changes. The workspace also contains unrelated user changes;
the firmware below was built with those changes present.

The CM4 XCP region is expanded from 0x400 to 0x500 bytes at the same
0x0800D000 base because the link requires 0x413 (1043) bytes. Its reserved
range is 0x0800D000--0x0800D4FF, leaving 237 bytes. The linked 8 KiB stack
remains 0x0800E000--0x0800FFFF. New symbols remain in `.xcp_cal_m4`, with
`NO_OPT volatile`. A2L was not changed; resolve symbol addresses from the new
ELF/map when manually updating calibration tooling, including existing symbols.

IAR 9.40.1 CM4 FOC incremental build/link passed. Compilation of existing MS
code emitted three unused-variable warnings; the final link had no warnings.
ELF SHA256: `A3ACB36A946321A9E0DB4827ED0A6318F2995942019CF3DB6A48EDA9A8E7F44F`.
No new bench trace was supplied and no motor was driven. Startup, low-Vdc
acceleration/deceleration, stop/reversal and DWT Last/Max/Overrun still need
bench validation (Fast:Speed about 10:1, zero overrun, >=20% WCET margin).

The Config Wizard UI persistence file must be opened with its matching tool and
saved with PI anti-windup set to **Back Calculation**. The generated header is
already set to `2`; the binary `.icwp` file is intentionally not edited by this
change.
