# HFIPD software validation — 2026-09-15

## Scope and gates

This is a hand-written injection/demodulation port with an independent single
MATLAB reference. It is not an ERT observer or a Simulink structural change;
wrapper-model / ERT gates are not applicable to this implementation choice.
No main SLX or A2L was changed by this task. Physical bench gates are NOT_RUN.

| Check | Result |
|---|---|
| MATLAB single reference / independent RK4 assumed saturation plant | PASS for declared acceptance/rejection cases |
| MATLAB vs compiled C, identical measured-current replay | PASS: 19 cases, maximum angular difference 2.8610e-6 rad |
| Accepted estimate error on paper-parameter plant | Maximum 0.60356 degrees (135-degree case) |
| Exact 90/270-degree source degeneracy | Both rejected with status 13; no initial angle accepted |
| No-saturation case | Rejected with status 14 |
| NaN parameter, NaN current, overcurrent | Rejected with expected statuses |
| IAR Cortex-M4 C-SPY kernel/adapter | PASS: 30,031 checks; maximum replay angle difference 1.07288361e-6 rad |
| Production startup/fast/run-state function regions in C-SPY fixture | PASS: 10,824 checks |
| Existing OFF-path requested-stop regression | PASS: 30 checks |
| MATLAB Code Analyzer, two new m files | No reported issues |
| IAR CM4 fixed KRE production | PASS, 0 errors, 3 existing unused-variable warnings in incremental final build |
| Separate Flux diagnostic image | PASS; full build 0 errors/15 existing warnings, final incremental 0 errors/3 warnings |
| Actual motor, rotating rotor, load, reversal, repeatability | NOT_RUN |
| DWT Last/Max/Overrun, Fast:Speed, foreground progress | NOT_RUN |
| Recorded bench-current replay / measured saturation curve | NOT_RUN |

The accepted-error gate was <4 degrees. C/reference tolerances were <0.1
degree, <1e-4 V, <1e-4 A*s, and identical stage/valid flags, set before running.
The second-track 135/315-degree cases were added after the initial 17 cases to
cover its offset-guess geometry; the final 19-case result supersedes the initial
<0.2-degree accepted maximum. No phase shifting was applied to comparisons.

The startup fixture extracts the actual production branch, fast-loop function,
and run-state stop/fault logic. It verifies both routes, fixed alignment count
during injection, exactly one valid angle write, no post-write zero reset,
observer-valid gating of direct handoff, invalid-result continuation without
angle write, cancel/restart, V/f exclusion, and forced-duty result rejection.
Unrelated controllers, ADC/PWM hardware, standby reset dependencies and the
observer acceptance response are stubbed. It does not prove plant dynamics or
hardware current regulation. The standalone adapter test also covers snapshot
freezing, partial calibration rejection, finite handling and state isolation.

## Target-motor limitation

The supplied reproduction uses Rs=.4457 ohm, Ld=1.28 mH, Lq=2.09 mH and an
assumed positive differential saturation curve (0.25 strength, 3 A scale).
These are not identified parameters of this project's motor.

The extra sensitivity study uses this project's saved Rs=.5 ohm,
Ld=1.3 mH, Lq=1.38 mH, retaining the same hypothetical saturation curve:

| True electrical angle | Final diagnostic error | Accepted |
|---|---:|---|
| 30 degrees | about +0.036 degrees | yes |
| 120 degrees | about -9.433 degrees | no |
| 240 degrees | about +177.456 degrees | no |

These are three exploratory tests, not a passed full-angle target-motor gate.
Default enable is OFF. Target-specific gains, tracking time, voltage/current
limits, pulse/tail times and actual delay require bench calibration. The two
axis check fixes the demonstrated false-valid cases; it is not a proof against
all indistinguishable states or disturbances.

## Final artifacts and snapshot

Production ELF: `Build/CM4_FOC/MMEk_Demo_CM4_FOC.elf`

SHA256 `9D35ACE609A9BFBACCB7B449B75AF38CEB1D2721F5BF34FCD4BBF8EDF925924D`

Diagnostic ELF: `Build/HFIPD_Tests/FluxDiagnostic/MMEk_Demo_CM4_FOC.elf`

SHA256 `A0E51E473E365C35511F8989AD057D206D5ECF4EA8EA7E7202BCB5B34DDBA501`

Compiler IAR 9.40.1, configuration `Multi Motor Evalkit V1.0`. The diagnostic
project copy changes only owner define and output locations; active project
remains fixed KRE. Logs, raw 5000-sample current/reference/C matrices, generated
test vectors and simulator logs are in `Build/HFIPD_Tests` and excluded from Git.
The parameter snapshot and status interpretation are listed in ../README.md.
Default HFIPD Enable=0, Hf=20 V, Pulse=10 V, Kp=2000, Ki=0, Init=0 rad,
Delay=2 ticks, Contrast=.01, AxisHi=8 deg, IsHi=45 A,
Track/Settle/Pulse/Tail/Gap=300/400/800/800/300 fast ticks.
Command source and startup-mode defaults are unchanged by this task.

XCP section uses 0x477 / 0x500 bytes, leaving 137 bytes. No linker-region growth.
All addresses must be resolved from the new ELF before measurement; the user's
pre-existing A2L modifications remain untouched. No flashing or bench command
was issued. These ELF hashes include unrelated pre-existing dirty-worktree work.

## Git / integration boundary

Local baseline: `89ea7028c755f4c3d362a0fc30a7102cbda00a66`,
`docs(sensorless): establish HFIPD integration baseline`.

The working project includes the HFIPD hooks in
`Example/CM4_FOC/main_cm4.c` and `MS/src/Ifx_MS_FocSolutionF16.c`.
Both files already contained unrelated changes. Git MCP supports whole-file
staging, so these mixed files remain uncommitted, rather than staging the user's
other work. The existing untracked MS stop test runner only gains an OFF-path
HFIPD stub and also stays uncommitted. New HFIPD files and the previously clean
IAR project are included in the independent module commit. A checkout of that commit
alone does not include the dirty production startup hooks.
`control_integration.patch` archives only this task's two production-file
changes against the exact dirty baseline below, for review and later staging.
It is already applied in this workspace and must not be applied a second time.

Exact pre-edit control-file copies are in `Build/HFIPD_Tests/baseline`:

- main_cm4.c SHA256 C4686C35371E0EBCC0583DCDB3BF8BCCA7B6817D2551F8A0EAF8743D06636E3B
- Ifx_MS_FocSolutionF16.c SHA256 E63FEDDE0B6F14FA4C5943F38428C1E30616572A216307B538799585530D887D

Bench handoff must record the flashed ELF hash, actual calibration snapshot,
startup route and command source, raw alpha/beta and d/q currents, Iq reference,
injected/applied voltage, PWM, initial/observer angles, speed, stage/validity,
fast/speed timing and overrun counts. Verify actual startup, speed changes,
stop, both directions and repeated starts with >=20% measured WCET margin.
