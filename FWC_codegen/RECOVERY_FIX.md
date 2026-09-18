# Speed recovery feedback-copy fix

## Baseline and contract (2026-09-10)

Source baseline: `bb10aa6d0afaaa174aeb8e8e3d9efa00cbcac6f0`.
Git MCP status and affected unstaged diffs were inspected before integration.
The working tree contains pre-existing KRE parameter/profiling, requested-stop,
PI calibration, model and A2L changes. These are not part of this repair.
This baseline note intentionally does not stage those mixed integration files.

Evidence: `power-fit-230v8600_2026-09-07_18-11-44.MF4`, 57.582--60.171 s.
The effective speed reference changes from a constant 8600.5 rpm to
8159--8932 rpm. Recovery count increments once, not repeatedly. Iq reference
equals measured Iq in 250 of 259 recorded samples after the transition.
The former recovery pass copies estimated speed to the reference and measured
Iq to the speed integrator every 500 us. Voltage LPF/debounce alone cannot
remove this feedback-copy path. The recording is not a 2 kHz closed-loop replay:
speed/current are recorded at 10 ms and FWC state at 100 ms.

This is a reference/current-control integration repair, not a new estimator.
Motor magnetic topology, Ld/Lq, PM flux and pole pairs are not inputs to this
governor; the existing motor/estimator configuration remains unchanged.

Intended contract:

- Run only at the 2 kHz speed boundary under existing FWC eligibility.
- On recovery entry, latch the smaller magnitude of the current effective
  reference and estimated speed as a fixed reference cap. Do not reseed the
  ramp or PI from noisy feedback. Preserve the existing ramp state/integrator.
- While active, clamp the application's requested speed to that symmetric cap
  after the normal input speed limits, then use the existing acceleration and
  deceleration limiter. Lower-speed commands remain effective. Feedback noise
  cannot move the cap; release restores the raw command through the same ramp.
- Preserve real speed error and proportional regulation. Filter measured Iq
  at 2 kHz with a 20 ms first-order time constant, initialized from the previous
  Iq command, then use bounded existing speed-PI Kaw*Ts for actuator tracking.
  Track only when the speed error asks for more unavailable current; suppress
  integration farther in that direction, retain integration that unwinds it.
- Disable/invalid/ineligible/stop clears governor state without changing the
  normal ramp or PI; repeated calls while inactive bypass recovery math.
- Do not change FWC Id control, -20 A floor, current circle, saturation timing,
  voltage filtering, 20 kHz current-PI tracking, CAN, KRE, A2L or PI tunings.

Validation will cover fixed vectors, both rotation signs, entry/hold/release,
lower commands, anti-windup direction and limits, noisy current, disabled paths,
existing FWC reference/generated-C replay, production-function integration and
CM4 IAR build. Hardware startup/stop/reversal and DWT remain bench gates.

## Implemented equations

All new operations are 500 us, hand-written integration code. The existing
FWC ERT wrapper is unchanged; no new generated block is added to the main FOC
model. The independent reference is `fwc_speed_recovery_reference.m`.

```
entry: cap = min(abs(previous effective reference), abs(estimated speed), 32767)
       IqF = previous Iq command; preserve PI integral and Q30 ramp history
target = clamp(normal input-limited command, -cap, cap)
effective reference = existing acceleration limiter(target)
e = existing Q14 half-difference(effective reference, estimated speed)
IqRequest, Iafter = existing speed PI(e)
IqF += (0.5 / 20.5) * (measured Iq - IqF)       [Q15 counts, single]
if sign(e) == sign(IqRequest - IqF), both nonzero:
    Itrack = Iafter
    if Iafter - Ibefore has the sign of e: Itrack = Ibefore
    Itrack -= clamp(KawTs / 2^KawQFormat, 0, 1) * (IqRequest - IqF) * 512
    Inext = clamp(Itrack, PI lower limit * 512, PI upper limit * 512) [Q24]
else: Inext = Iafter
```

The current-cycle IqRequest is unchanged by actuator tracking; the final
existing Id-priority circle still limits it. Negative Kaw and unsupported
Q formats disable the added correction, but outward integration is still held.
All algorithm inputs are bounded integer signals; the single filter is a convex
combination of bounded Q15 counts and has no float calibration input.

## Verification result

- Local baseline note commit: `8ca96960cf6b293c0c74dc0ab393e5bc318920bd`.
- Independent C kernel: 6024 checks passed, IAR C-SPY Cortex-M4 simulation.
- 160-cycle MATLAB/C comparison: activity, target and Q24 integral identical;
  filtered-current difference <= 5e-7 Q15 count (decimal print rounding).
- Production MS function bodies plus real Infineon PI/acceleration limiter:
  12037 checks passed. Includes positive/negative speed, noise, lower command,
  reversal request, smooth release, no instantaneous Iq feedthrough, persistent
  current shortfall, opposite speed error, and inactive-path equivalence.
- Existing MATLAB FWC suite: 8/8 passed. Existing adapter tests and generated
  ERT replay (including 1400 adapter/ERT cycles) passed.
- Existing requested-stop tests: 30 checks passed.
- IAR 9.40.1 CM4 FOC build: 0 errors, 15 warnings in existing code locations.
  Configuration: `Multi Motor Evalkit V1.0`.
- Artifact: `Build/CM4_FOC/MMEk_Demo_CM4_FOC.elf`.
  SHA256: `9C51A5C793DE39E141D5EA50CAE79311D12C4AB3E91D0F6550CDF1BAE68F7419`.
- Current linked XCP section: 0x41b / 0x500 bytes (1051 / 1280), 229 bytes free.
  Stack remains 0x0800e000--0x0800ffff (8 KiB). No new XCP symbols were added.
  The FOC structure layout has changed: resolve ALL measurement addresses
  against the new ELF before recording. A2L is deliberately untouched.

The ELF includes the user's other dirty-worktree changes; its hash, not the
baseline commit alone, identifies this build. The repair is left uncommitted:
the available Git MCP supports whole-file staging only, and main_cm4/MS contain
unrelated changes. Do not stage those whole files as if they were only this fix.

Reproduce the new software checks using the two `run_fwc_speed_recovery*_tests.ps1`
scripts. For the independent reference comparison after the kernel runner:

```matlab
addpath('FWC_codegen');
lines = splitlines(string(fileread('Build/FWC_SpeedRecoveryTests/result.txt')));
parts = split(lines(startsWith(lines,'VECTOR,')), ',');
actual = str2double(parts(:,2:end));
max(abs(actual - fwc_speed_recovery_reference_vectors()))
% Expected columns <= [0, 0, 0, 0.01, 4] (last column is Q24 counts).
```

No motor has been driven and no fresh bench/DWT evidence is claimed. Repeat the
230 V/high-speed case with original command, effective reference, estimated
speed, Id/Iq reference/feedback, speed PI integral, FWC governor/count and voltage
utilizations. Verify hold/release and commanded deceleration, stop, reversal,
repeated startup, FWC disable/invalid paths; capture Fast:Speed ~10:1, zero
overrun, continuing foreground progress and >=20% fast-loop WCET margin.
The 10/100 ms MF4 cannot establish 2 kHz dynamics or prove the physical speed
response of the repaired loop. It identified the feedback-copy defect only.
