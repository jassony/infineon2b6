# KRE coefficient initialization and online calibration

## Scope

The active CM4 production build (`FOC_DIAG_FLUX_REFERENCE=0`) keeps the
existing discrete equations and update order. This is a local generated-C
customization, not a model regeneration. Re-generating the wrapper requires
reapplying the cached-coefficient interface and repeating replay validation.
Neither SLX models nor A2L files were changed by this work.

Five derived values are prepared at initialization and whenever a captured
parameter set changes:

| Value | Expression |
| --- | --- |
| H2 gain | `1 - expf(-alpha * Ts)` |
| Speed filter gain | `1 - expf(-2*pi*speedFilterHz*Ts)` |
| PLL proportional gain | `2*damping*(2*pi*bandwidthHz)` |
| PLL integral gain per step | `(2*pi*bandwidthHz)^2 * Ts` |
| Inductance difference | `Ld - Lq` |

Use the same single-precision constants and expression ordering as the
previous generated step. No Euler/Tustin substitutions, inverse approximations,
Q-matrix reductions, or new parameter-range checks were added to the fast step.

## Online inputs

All seven existing KRE calibration symbols are live while running:

- `Cal_Kre_Alpha_radps_f32`
- `Cal_Kre_A_radps_f32`
- `Cal_Kre_Gamma_f32`
- `Cal_Kre_SigmaEpsilon_Wb_f32`
- `Cal_Kre_SpeedFilter_Hz_f32`
- `Cal_Kre_PllBandwidth_Hz_f32`
- `Cal_Kre_PllDamping_f32`

The five shared motor requests also apply while running, through the project
motor-parameter service (write the request, not the `Applied` mirror):

- `Cal_MotorPhaseResistance_mOhm_u16`
- `Cal_MotorDirectInductance_uH_u16`
- `Cal_MotorQuadratureInductance_uH_u16`
- `Cal_MTPA_PermanentMagnetFlux_mWb_u16`
- `Cal_MotorPolePairs_u8`

Ts and PU base voltage/current/speed are build configuration, not new runtime
calibration variables. No new XCP variables or parameter selector was added.

## Preparation and publication

1. Foreground captures the calibration group twice. A mismatch defers the
   update. Matching reads do not prove completion of several separate XCP
   writes; an intermediate but stable group can be applied. Use small changes
   or stop the motor for a multi-parameter operating-point transition.
2. Changed snapshots calculate coefficients with IRQs enabled. Unchanged
   snapshots do not repeat exp calls.
3. A short caller-owned PRIMASK critical section publishes the raw parameters
   and coefficients between complete control ISR executions. No exp calls are
   made while publishing. Fast/speed priorities and command routing are unchanged.
4. For a shared motor request, fixed-point motor conversions are prepared
   outside the lock. The motor fields/mirrors and corresponding KRE snapshot
   are committed under the same lock, so a fast ISR cannot see new controller
   inductances with an old KRE motor snapshot.
5. Publication does not reset KRE, clear validity, or discard its delayed VI
   sample. Initialize/start/stop/manual and numerical-fault resets remain.

The update becomes effective on the next control execution after foreground
publication, without requiring a stop/restart. This is not a guaranteed update
latency: if foreground is starved, preparation is delayed. Expensive coefficient
work must not be moved back into the fast ISR to mask foreground starvation.

## Retained checks and related consumers

KRE fast-step physical/range validation remains removed. The following have
actual downstream dependencies and were intentionally retained:

- Input/output finite-value detection and fault-reset edge latching.
- Snapshot consistency and readiness (not physical parameter validation).
- KRE active-flux epsilon threshold for normalization/PLL initialization.
- Shared motor nonzero and Q15/angle-increment representability checks, plus
  nonzero PM flux. These protect fixed-point controller/I-f/V-f consumers and
  avoid a zero denominator. Failed preparation leaves the active set intact;
  `Cal_MotorParameterStatus_u8=1` reports rejection.
- Hardware/current/voltage fault protection.
- RRC-DOB and VAFID retain their own parameter checks and update contracts.
  They are serviced after the joint FOC/KRE publication, not in the same
  transaction. RRC-DOB restarts its dynamic state on a changed algorithm
  parameter set; VAFID may restart collection on configuration changes.
  Enabling these sidecars therefore requires separate online-tuning bench tests.

Preserving states does not guarantee a bumpless estimate when changing motor
parameters, pole pairs, or PLL gains. An invalid KRE setting can still cause
invalid output and a numerical-fault reset. No claim of stable arbitrary online
parameter jumps is made. The Flux diagnostic path is not bench-validated here.

## Verification (2026-09-08)

Baseline commit: `5ccb01d86428495d2e775a33da7f73db77ffd771`
(`perf(kre): remove fast-step parameter validation`). Baseline firmware SHA256:
`7E0CFA8263CAB9ABAB079E7CE690ABF3B24915E1D4475485EF70C8B59306FB76`.

`run_kre_external_observer_adapter_tests.ps1` passed with `-Oh` and with
`-Optimization -Om` plus `--no_inline --no_unroll --no_tbaa --no_scheduling`.
Both retain the 6000-step valid-input replay digest `7b81d650` from the
uncached baseline. This is bounded regression evidence, not exhaustive proof.
Additional tests cover each of the 12 calibration inputs, unchanged/staged
snapshots, coefficient formulas, no parameter-triggered state reset, joint
shared-motor publication, invalid motor conversion rejection, and existing
input/output numerical-fault behavior. The simulator compiles the real motor
prepare/apply functions rather than copies of their implementation.

Initial incremental CM4 `Multi Motor Evalkit V1.0` build passed: 0 errors,
3 existing unused-local warnings in `Ifx_MS_FocSolutionF16.c`. The following
table records this task's isolated delta before a concurrent stop-threshold
change was added to the shared workspace.

| Map metric | Before | After | Delta |
| --- | ---: | ---: | ---: |
| Readonly code | 55154 B | 55426 B | +272 B |
| Readonly data | 17213 B | 17213 B | 0 B |
| Readwrite data | 15219 B | 15279 B | +60 B |
| `.xcp_cal_m4` | 0x3FB | 0x3FB | 0 B |
| Generated step size | 0x53A | 0x4F0 | -74 B |

The 60 B is three 20-byte coefficient copies (staged/applied/generated).
Foreground local snapshots and prepared motor values also use stack space;
the map delta is not a dynamic stack-peak measurement.

Thumb disassembly confirms the step has no exponential call, while
`KreExternalObserver_calculateCoefficients` contains two `__iar_exp32` calls.
The production motor publication helper contains no division or exp call.
At an achieved 20 kHz this eliminates 40000 repeated exp evaluations/second.

Initial validated ELF SHA256:
`FD0155AB658DD77E9D2B61A7BA045B8A204D2B9904C20E45EA019ECE97DF1E2D`.

During final verification, a concurrent edit added `Cal_FOC_StopIsHi_A_f32`
and stop-path code in the shared `Ifx_MS_FocSolutionF16.c`. It was preserved,
not authored or behavior-validated by this task. A subsequent full CM4 rebuild
(`-build`, not just `-make`) succeeded with 0 errors and 15 warnings across
the project. The `-Om` adapter/motor replay was also rerun successfully against
the merged source. The final ELF/map/HEX were copied to a separate artifact
directory to avoid confusing later background rebuilds with this result:

`Build/KRE_OnlineValidated_20260908_1756/MMEk_Demo_CM4_FOC.elf`

SHA256: `E4217262D86DBEBFBED461C70FDC0550DE2D763CA2A7F86BE57F2A628199B8C8`.

The final merged map reports code 55570 B, readonly data 17218 B, readwrite
data 15283 B, and XCP section 0x3FF/0x400 (only one byte remains). The extra
144 B code, 5 B readonly data and 4 B RAM relative to the isolated table are
from the concurrent change, not coefficient caching. Final snapshot Thumb
disassembly again confirms no exp call in the step and two in preparation.

## Required bench follow-up

No board execution or new MF4 was performed for this change. Verify:

- DWT fast-loop Last/Max/Overrun, total load, FastObs share, WindowCycles,
  UpdateSeq, idle progress, Fast:Speed near 10:1 and at least 20% WCET margin.
- Small live changes to each input: Applied mirrors and ParamApplySeq advance;
  ParamPending clears; KRE RstCount does not advance for a valid update alone.
- Observe angle, speed, validity/status, Iq reference/feedback and PWM through
  start, speed changes, stop, reversal and repeated starts.
- Test invalid parameter behavior at a safe bench stage; do not disable
  hardware protection. Repeat separately with any enabled RRC-DOB/VAFID path.

Use symbols from the exact ELF. This task adds no XCP variables, but ordinary
RAM symbols moved and the concurrent XCP insertion shifted XCP addresses too;
existing A2L addresses must not be assumed current.
This work does not edit or regenerate the A2L automatically.
