# RRC-DOB dead-time voltage compensation

Output-only sixth-harmonic time advance is available through
`Cal_RRCDOB_Lead_us_u16` (default 0 = bypass). The applied stopped-only snapshot
is `Meas_RRCDOB_Lead_us_u16`. See [LEAD_TEST.md](LEAD_TEST.md) for the current
firmware hashes, test evidence and bench procedure. This does not change KRE's
VI_fb delay or the main FOC angle; powered validation remains NOT_RUN.

Output-sign comparison is available through `Cal_RRCDOB_Rev_u8`: 0 retains
the original direction, nonzero reverses both axes. See [SIGN_TEST.md](SIGN_TEST.md)
for stopped-only operation and the historical sign-only test artifacts.

Current production configuration: **100 us / 10 kHz, default OFF, Monitor
available, Apply enabled by explicit user request**. See
[APPLY100US.md](APPLY100US.md) for current operation and acceptance status,
and [CONTROL100US.md](CONTROL100US.md) for the original locked integration. The
20 kHz timing and Apply instructions below describe the earlier implementation;
use the current 100 us calibration procedure instead. Default selector is OFF.

Engineering calibration bounds have subsequently been removed at user request;
see [PARAMETER_CHECKS.md](PARAMETER_CHECKS.md) for retained numeric/interface
requirements and current build evidence. Historical bounds below are not gates.

This directory contains the staged integration of the revised-resonant
controller disturbance observer (RRC-DOB) for inverter dead-time voltage
compensation. The production FOC model is intentionally not modified.

## Source of truth

The mathematical source of truth is Lang et al., *Decoupled Dead-Time
Compensation Method Using Revised-Resonant Control-Based Disturbance Observer
in PMSM Drives*, specifically Fig. 8 and (17)--(19), (31). The source-task
MATLAB artifact is used as a comparison input, but its raw-PI input wiring is
not copied because the observer plant input in (18) is the applied voltage
reference minus the cross-coupling voltage.

For each d/q axis:

```text
deltaUHat = Grrc(s) * (i - iHat)
iHat      = (uApplied + deltaUHat) / (LHat*s + RHat)
uCommand  = uPI - deltaUHat

Grrc(s) = (LHat*s + RHat) * 2*wc*s / (s^2 + w0^2)
w0      = 6*abs(omegaElectrical)
wc      = cutoffRatio*w0, default cutoffRatio = 0.10
```

The current project has d/q decoupling disabled. This first integration has a
compile-time guard against enabling it, because a future decoupling-enabled
build must pass `uApplied - ucc` explicitly rather than silently changing the
observer equation.

## Discrete contract

- Fast-loop sample time: `Ts = 50 us` (20 kHz).
- The current-loop factor must remain one, so the compensator executes on every
  50 us sample.
- Motor: IPMSM; d and q observers use explicit `Ld` and `Lq` respectively.
- Inputs per sample: previous modulator-limited applied d/q voltage in Q15,
  present measured d/q current in Q15, present raw PI d/q voltage in Q15, and
  electrical angle as one unsigned Q0.32 cycle.
- Output order follows the source algorithm: publish the disturbance estimate
  from the current state, then update the observer state.
- Applied command is `uPI - ramp*deltaUHat`, independently saturated by the
  configured compensation limit and then by the existing voltage path.
- The independent wrapper and replay use fixed-size scalar/vector ports only.

The independent model is
`rrc_dob_voltage_compensation_wrapper.slx`. It contains eight typed inputs,
one MATLAB Function step, one eight-element `int32` Unit Delay, and four
outputs; it contains no plant, inverter, PI controller, or main FOC model.
Its configuration is fixed-step discrete at `5e-5` seconds, `ert.tlc`, C,
code-generation only, and ARM Cortex production hardware.

The bilinear transform uses `h = Ts/2`, `q = tan(w0*Ts/2)`,
`c = wc/w0`, `r = R*h/L`, `kv = h*Vbase/(L*Ibase)`, and
`d = 1 + 2*c*q + q^2`. The closed-form matrices are:

```text
A11 = ((1-r)*(1+q^2)-2*c*q*(1+r))/((1+r)*d)
A12 = -4*c*q/d
A13 = -4*c*(q^2-r)/((1+r)*d)
A21 = -2*q^2/((1+r)*d)
A22 = (1+2*c*q-q^2)/d
A23 = 2*q*(1+r+2*c*q)/((1+r)*d)
A31 = -2*q/((1+r)*d)
A32 = -2*q/d
A33 = ((1+r)*(1-q^2)+2*c*q*(1-r))/((1+r)*d)

B11 = 2*kv*(1+q^2)/((1+r)*d)
B12 = 4*c*q/d
B21 = -2*kv*q^2/((1+r)*d)
B22 = 2*q^2/d
B31 = -2*kv*q/((1+r)*d)
B32 = 2*q/d
```

With observer states normalized by `Ibase`, the disturbance estimate is:

```text
lambda = L*Ibase/(Vbase*Ts)
rho    = R*Ibase/Vbase
deltaUHatPU = [-4*c*q*lambda, -4*c*q*lambda, 2*c*rho] * x
              + 4*c*q*lambda*iPU
```

When the tracked frequency changes, state 1 is retained, state 2 is multiplied
by `(qNew/qOld)^2`, and state 3 by `qNew/qOld`. A first activation, a frequency
ratio outside `[0.5, 2.0]`, or an out-of-range frequency resets the axis states.

## Fixed-point representation

The firmware hot path contains no floating-point arithmetic, transcendental
function, dynamic allocation, or integer division.

| Quantity | Representation |
| --- | --- |
| Voltage/current input and output | signed Q1.15 (`int16`) |
| Electrical angle | unsigned Q0.32 cycle (`uint32`) |
| Prewarped `q` | signed Q2.29 (`int32`) |
| Matrix/output coefficients | signed Q4.27 (`int32`) |
| Observer states | signed Q5.26 (`int32`) |
| Apply ramp | unsigned Q1.31 (`uint32`) |
| Multiply-accumulate intermediate | signed 64 bit |

The prewarp is supplied by a 257-point lookup table over 10--750 Hz electrical
frequency with linear interpolation. Coefficients are cached and refreshed at
most every ten fast-loop samples. Narrowing operations use symmetric rounding;
the configured correction and final command use saturation, while a state,
intermediate, or Q15 disturbance-estimate overflow invalidates the sample,
clears the observer states, and publishes zero compensation.

## Calibration and validity

The initial software-only operating envelope is:

- positive mechanical speed 800--4000 rpm;
- `RHat = 0.5 ohm`, `LdHat = 1.30 mH`, `LqHat = 1.38 mH`;
- four pole pairs, `Vbase = 1000 V`, `Ibase = 50 A`;
- cutoff ratio 0.10 and maximum compensation 0.10 PU;
- ramp time 10 ms.

Invalid selector, motor parameters, speed range, sample time, angle-derived
frequency, stale applied voltage, arithmetic overflow, or unavailable initial
state produces `Valid = 0`, zero compensation, and an explicit status.

The adapter API is:

```c
void RrcDobCompensator_initialize(void);
void RrcDobCompensator_reset(void);
uint8_t RrcDobCompensator_setParameters(const RRCDOB_Parameters *parameters);
void RrcDobCompensator_captureAppliedVoltage(
    Ifx_Math_CmpFract16 voltageAlphaBetaQ15, uint32 electricalAngleQ32);
uint8_t RrcDobCompensator_execute(
    Ifx_Math_CmpFract16 rawVoltageDQ_Q15,
    Ifx_Math_CmpFract16 currentDQ_Q15,
    uint32 electricalAngleQ32,
    Ifx_Math_CmpFract16 *compensatedVoltageDQ_Q15);
void RrcDobCompensator_getOutput(RRCDOB_Output *output);
```

Selector semantics are explicit:

- `0`: off; clear state; no dead-time voltage compensation is applied.
- `1`: monitor; RRC-DOB runs and publishes measurements without changing the
  voltage command.
- `2`: apply; RRC-DOB owns dead-time compensation and writes the compensated
  d/q command into the PWM voltage path.

The CM4 firmware no longer links or executes the legacy sign-based
`Ifx_MAS_DeadTimeCompensatorF16` path. The library source is retained only as
historical/reference code. Consequently, an invalid or inactive RRC-DOB output
does not fall back to a second compensator.

The configured lower-speed limit must correspond to at least 10 Hz electrical.
Forced-duty and HFI operation revoke RRC-DOB eligibility and reset its dynamic
state because neither injected voltage is represented by the applied-voltage
feedback contract.

The slow-loop parameter handoff updates the complete RRC-DOB parameter/state
snapshot with interrupts disabled. The fast ISR therefore cannot observe a
partial selector or parameter update. Within one fast sample, regulation writes
the output state and voltage generation consumes the same state before the next
parameter handoff can run.

## Firmware activation

The compile-time Apply lock has been removed, while the startup selector
remains safely defaulted to `0`. To enable voltage output through XCP, set
`Cal_RRCDOB_Sel_u8 = 2`. A selector transition automatically resets the
observer and starts the configured output ramp; `Cal_RRCDOB_Rst_u8 = 1` is an
additional one-shot manual reset.

Actual output requires all runtime gates to remain valid:

- FOC is running in closed loop;
- forced-duty and HFI injection are off;
- positive mechanical speed is within `Cal_RRCDOB_SpdLo_rpm_u16` and
  `Cal_RRCDOB_SpdHi_rpm_u16` (defaults 800--4000 rpm);
- the previous applied-voltage sample is fresh and all arithmetic is valid.

`Meas_RRCDOB_Stat_u8 = 3`, `Meas_RRCDOB_Valid_u8 = 1`, and
`Meas_RRCDOB_OutAct_u8 = 1` jointly confirm that compensation is affecting the
PWM voltage command. HFI and VAFID identification must remain off during an
RRC-DOB-only validation run. For a first powered test, use a conservative
`Cal_RRCDOB_OutHi_Q15_s16 = 328` (0.01 PU) and
`Cal_RRCDOB_Ramp_ms_u16 = 100`, then increase only from captured evidence.

New XCP-visible symbols use `Cal_RRCDOB_*` and `Meas_RRCDOB_*`, are declared
`NO_OPT volatile`, and are not added to an A2L automatically.

## APSFSM torque-compensation interaction

APSFSM runs at 2 kHz after speed control and changes the d/q current reference.
RRC-DOB runs at 20 kHz after the current PI and changes the d/q voltage command.
They therefore have separate ownership boundaries and can coexist. RRC-DOB
will see APSFSM-induced reference dynamics through the ordinary current loop;
it must not read or modify APSFSM state.

Combined bench validation must nevertheless separate three cases (APSFSM only,
RRC-DOB only, both enabled) because both algorithms may act near the sixth
electrical harmonic and their combined CPU load must satisfy the existing DWT
fast-loop budget. RRC-DOB still defaults off and requires an explicit selector
change before it can alter either APSFSM output or the voltage command.

## Verification gates

1. Double/single discrete reference remains finite for fixed-frequency,
   sweep, reversal, reset, saturation, and invalid-input vectors.
2. Integer reference differs from the single reference by at most 2 Q15 LSB
   RMS and 8 Q15 LSB peak in the validated range.
3. Generated C replays bit-for-bit against the integer reference.
4. Static scan finds no `float`, `double`, `tanf`, dynamic allocation, or
   division helper in the firmware core.
5. CM4 IAR builds with the no-compensation, monitor, and Apply branches; the
   legacy compensator object is absent from the link.
6. Apply is source-enabled but still requires powered bench validation of
   finite state, expected sixth-order tracking, ISR timing, and the controlled
   no-compensation-to-RRC handoff before production release.

## Current verification record

- `rrcDobDiscreteStepTest`: 14/14 tests pass.
- `rrcDobFixedPointStepTest`: 10/10 tests pass. Across 800 rpm, 4000 rpm,
  frequency sweep, and APSFSM-like 10:1 steps, the worst error is 0.094 Q15
  LSB RMS and 1 Q15 LSB peak against the single-precision reference.
- `verify_rrc_dob_wrapper`: 161 normal-simulation samples match the integer
  step bit-for-bit for command, estimate, state, and status.
- ERT source generation completes in
  `rrc_dob_voltage_compensation_wrapper_ert_rtw/`.
- The CM4 IAR build excludes the legacy dead-time compensator and compiles the
  RRC-DOB Apply output path. The startup selector remains off. DWT timing,
  combined APSFSM/RRC-DOB operation, a controlled no-compensation-to-RRC
  handoff, and powered bench validation remain open release gates.
- A clean `Multi Motor Evalkit V1.0` build completes with 0 errors and the 16
  pre-existing warnings. The image uses 51,438 bytes of readonly code, 17,175
  bytes of readonly data, and 15,214 bytes of readwrite data. Enabling Apply
  adds 80 bytes of code and no data versus the locked build; relative to the
  VAFID-integrated legacy-compensator baseline, the firmware still saves 960
  bytes of code, 9 bytes of readonly data, and 50 bytes of readwrite data. The
  final HEX SHA-256 is
  `F964CF3EACE8FBAC1EFE2944538162A6979D6085F45D6447EB7A68B22733A5CE`.
