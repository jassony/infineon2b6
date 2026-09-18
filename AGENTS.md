# PMSM Sensorless Algorithm Integration Guide

This repository contains a production FOC project and several in-progress
sensorless algorithms. Treat algorithm work as a gated integration workflow,
not as a direct edit to the active current-control path.

## Scope And Safety

- Preserve unrelated work in a dirty worktree. Before modifying files, run
  `mcp__git__git_status` and inspect the affected diff with Git MCP.
- Make a local baseline commit before a material algorithm integration or a
  bench-tested behavior change. Stage only files belonging to that algorithm.
- Do not modify `afo/SensorlessFocFOSMOExample/mcb_pmsm_foc_sensorless_f28379d.slx`
  unless the user explicitly requests a main-model change.
- Do not create or modify A2L files unless explicitly requested. XCP variables
  must still use the naming rules below so they can be added manually later.
- Never silently replace the existing Flux estimator. A new estimator is first
  an observer-only diagnostic path, then an explicitly declared FOC owner;
  ownership may be runtime-selectable or fixed by separate build configuration.

## FOC Control-Path Regression Guardrails

- Motor motion during alignment or I/f is not proof of successful startup.
  Verify the complete chain from requested command through effective speed,
  `Iq_ref`, voltage, PWM, measured current, KRE handoff, and closed-loop speed
  response. Include speed changes, stop, reversal, and repeated starts.
- Do not change defaults such as `Cal_FocCommandSource_u8` or
  `Cal_FocStartupMode_u8` to hide a routing, scheduling, or handoff defect.
  Defaults are interface contracts and require a separate reviewed change and
  power-cycle regression.
- Start, stop, speed, and control-enable commands must be consumed at a
  deterministic control boundary. For this FOC application, latch them at the
  2 kHz speed-state-machine boundary; foreground service must not be required
  for a command to take effect.
- Treat 20 kHz, 2 kHz, and foreground ownership plus interrupt priority as a
  control contract. Before moving work or changing priority, prove that shared
  FOC state cannot be accessed by nested callbacks, then capture fast/speed/
  idle count slopes and the fast-loop overrun counter.
- Integrate dead-time compensation, APSFSM, RRC-DOB, and VAFID one at a time.
  The stable OFF path must bypass algorithm steps, parameter scans, and
  repeated resets at the call site. Pass the bare-KRE bench gate before adding
  the next feature; use Shadow/Monitor before Apply.
- Any generated-code or library-math change on the 20 kHz path requires replay
  plus measured DWT Last/Max/Overrun timing. Require `Fast:Speed` near 10:1,
  zero overruns, continuing foreground progress, and at least 20% WCET margin.
- Stage parameter sets as a coherent snapshot and apply them atomically only
  while stopped. Runtime writes become pending and must not reset KRE every
  cycle. Parameter readiness and estimator validity may restrict KRE ownership
  only; they must not suppress power, control, alignment current, or I/f.
- Do not retain unused selectors, calibration symbols, ABI placeholders, or
  fallback logic unless an explicit compatibility requirement justifies them.
  Before declaring a bench fix, archive the exact firmware hash, calibration
  snapshot, startup route, command source, raw trace, and tested coverage.

## Git MCP Workflow

All Git operations for this repository must use the `mcp__git__*` tools. Do
not invoke `git` through PowerShell, `exec_command`, scripts, IDE actions, or
another wrapper. If Git MCP is unavailable or rejects the repository path,
report the blocker instead of using a shell fallback.

- Pass `D:\A_PRJ\infineon3in1\code_xcp` as `repo_path` and confirm the tool
  response names the intended repository before a mutating operation.
- Before editing, call `mcp__git__git_status` and inspect relevant existing
  changes with `mcp__git__git_diff_unstaged`, `mcp__git__git_diff_staged`,
  `mcp__git__git_diff`, or `mcp__git__git_show` as appropriate.
- Preserve unrelated user changes. Do not discard, clean, overwrite, or stage
  them silently.
- Use explicit file paths with `mcp__git__git_add`; never stage a broad set
  equivalent to `.` or `-A`.
- Before committing, inspect `mcp__git__git_diff_staged`, run proportionate
  tests/builds, and recheck `mcp__git__git_status`. Commit only the intended
  logical change.
- After committing, verify the result with `mcp__git__git_status` and
  `mcp__git__git_log` or `mcp__git__git_show`. Report the commit id, subject,
  verification performed, and any remaining worktree changes.
- Use `mcp__git__git_create_branch` for agent-created branches. Follow an
  explicit user-provided name; otherwise use the `codex/` prefix.
- Do not reset, restore, clean, amend, rebase, force-update, delete branches,
  or change checkout state unless the user explicitly requests the exact
  operation and its scope has been verified.

### Commit Message Format

Use Conventional Commits:

```
<type>(<scope>): <imperative summary>
```

- Allowed types: `feat`, `fix`, `refactor`, `perf`, `docs`, `test`, `build`,
  `ci`, `chore`, and `revert`.
- Use a scope when it identifies the affected subsystem, for example `foc`,
  `sensorless`, `xcp`, `cm4`, or `build`.
- Keep the subject specific, imperative, and no longer than 72 characters;
  do not end it with a period.
- When useful, add a body after one blank line that explains the reason for
  the change and any behavior or compatibility impact. Add footers only for
  meaningful metadata such as `Refs: #123` or `BREAKING CHANGE: ...`.
- One commit contains one logical change and its message describes only the
  staged files. Split unrelated work into separate commits.

Examples:

```
feat(sensorless): add observer-only KRE adapter
fix(foc): preserve Flux ownership on invalid estimate
docs(xcp): document calibration variable naming
build(cm4): update IAR project sources
```

## Toolchain And CM4 Build

- The installed IAR Embedded Workbench for ARM is version 9.40.1 at
  `D:\APP\iar9401`. The ARM compiler tools are under
  `D:\APP\iar9401\arm\bin`; the command-line build executable is
  `D:\APP\iar9401\common\bin\iarbuild.exe`, not the ARM `bin` directory.
- The CM4 FOC project is `IAR/cm4_mc/MMEk_Demo_CM4_FOC.ewp` and its active
  configuration is `Multi Motor Evalkit V1.0`. Build this project for a
  CM4-only FOC source change unless the task explicitly requires the complete
  multicore image.
- The CM4 FOC artifact is written under `Build/CM4_FOC/`. Do not commit build
  output, IAR caches, browse information, or local IDE settings.
- Do not treat the older IAR path in local VS Code settings as authoritative.
  Use the installation path above for command-line verification.

## Choose The Algorithm Track

Classify the algorithm before writing implementation code.

| Track | Examples | FOC integration point |
| --- | --- | --- |
| External observer | FADO, KRE, HFO, EKF, MRAS, SMO variants | `ExternalObserverManager` returns electrical angle and mechanical speed |
| Injection or demodulation | HFI and carrier injection variants | Modulator/PWM voltage command path; it does not replace the observer-manager contract |
| Current, torque, or reference algorithm | MTPA, field weakening, torque estimation | Reference-current path; do not add it to the estimator selector |

Do not force an injection algorithm into the external-observer interface or an
observer into the PWM injection interface.

## Required Workflow

Complete every gate in order. Do not skip from a paper or MATLAB function
directly to the active FOC control loop.

### 0. Baseline And Requirements

1. State the source of truth: paper, equations, code, and intended operating
   range.
2. Identify every state, parameter, input, output, unit, sample time, reset
   condition, and validity condition.
3. Record whether the target motor is SPMSM or IPMSM. Keep `Ld`, `Lq`, PM flux,
   and pole-pair use explicit; pass only parameters required by the algorithm.
4. Create or verify a local Git baseline before integration work.

Exit gate: the algorithm contract is unambiguous and no active FOC file has
been changed.

### 1. MATLAB Discrete Reference

1. Implement the algorithm as explicit discrete difference equations at the
   intended fast-loop sample time, normally `Ts = 50e-6` seconds.
2. Use fixed-size `single` states where code generation is planned.
3. Include reset behavior, parameter validation, finite-value handling, angle
   wrapping, saturation, and any low-speed mode explicitly.
4. Verify fixed-vector and recorded-data replay before opening the FOC path.

Exit gate: all states and outputs remain finite; behavior is explainable from
the source equations.

### 2. Independent Wrapper Model

For a generated external observer, create a separate model under
`<ALGO>_codegen/` named `<algo>_external_observer_wrapper.slx`.

- The model contains only algorithm inputs, a resettable or Unit Delay state,
  one discrete algorithm step, and outputs.
- Do not add Plant, inverter, current PI, speed PI, or the main FOC model.
- Use fixed-step discrete solver, `FixedStep = 5e-5`, `ert.tlc`, C language,
  and code-generation-only configuration.
- Use MATLAB MCP and Simulink model tools for model inspection and structural
  edits. Run `model_check` after structural edits.

Reference patterns:

- `FADO_codegen/fado_external_observer_wrapper.slx`: scalar SI inputs plus
  explicit Unit Delay state.
- `KRE_codegen/kre_external_observer_wrapper.slx`: resettable observer
  subsystem with parameter vectors and status output.

Exit gate: wrapper topology is complete, structurally clean, and contains no
unconnected error-level ports.

### 3. ERT Generation And Replay

1. Generate the wrapper C code into its own `<algo>_external_observer_wrapper_ert_rtw/`
   directory.
2. Compare generated-code replay to the MATLAB discrete reference using the
   same vectors and tolerances.
3. Verify angle wrap, speed conversion, reset, saturation, and invalid-input
   behavior separately.
4. Do not copy generated cache files, `slprj`, `.slxc`, `buildInfo.mat`, or
   `tmwinternal` into a baseline commit unless the user specifically requests
   them.

Exit gate: generated outputs match the reference within the documented
tolerance and no dynamic-size behavior is introduced.

### 4. C Adapter And XCP Contract

Implement an algorithm-local adapter in `<ALGO>_codegen/`.

External-observer adapters must expose this logical contract:

```c
void <Algo>ExternalObserver_initialize(void);
void <Algo>ExternalObserver_reset(void);
void <Algo>ExternalObserver_execute(float voltageAlpha_V,
                                    float voltageBeta_V,
                                    float currentAlpha_A,
                                    float currentBeta_A);
uint8_t <Algo>ExternalObserver_getFocEstimate(float *electricalAngle_rad,
                                               float *mechanicalSpeed_rpm);
```

- Convert Q15/PU values to SI once at the adapter boundary. If the generated
  algorithm uses PU internally, keep that conversion local and documented.
- Validate parameters and inputs before publishing an estimate.
- `getFocEstimate` returns false for invalid, stale, non-finite, or not-yet-
  initialized estimates.
- Reset observer state when stopping, changing estimator, or receiving an
  explicit reset. Do not reset merely because a valid online calibration was
  applied.

Naming:

- Calibration: `Cal_<Algo>_<Meaning>_<Unit>_<Type>`
- Measurement: `Meas_<Algo>_<Meaning>_<Unit>_<Type>`
- C API: `<Algo>ExternalObserver_*` or `<Algo>Injection_*`
- All XCP-visible variables use `NO_OPT volatile` and the established XCP
  section. Do not edit the A2L automatically.

### XCP Variable Name Length And Abbreviations

Use short, stable XCP variable names that remain readable in calibration and
trace tools. Apply this rule to newly introduced variables only; do not rename
an existing variable or modify its A2L entry without an explicit migration
task.

- Keep the required prefix and algorithm name: `Cal_<Algo>_...` for tunables
  and `Meas_<Algo>_...` for runtime values. Keep recognized algorithm names
  uppercase, for example `ADRC`, `MTPA`, `KRE`, and `HFI`.
- Use one compact, unambiguous signal token followed by unit and C type:
  `<Kind>_<Algo>_<Signal>_<Unit>_<Type>`. Omit a repeated scope word when the
  algorithm already makes it clear; use `Spd` only when it distinguishes a
  speed-loop selector, reset, status, or activity signal.
- Use the following standard tokens rather than spelling out long words:
  `Spd` speed, `Sel` selector, `Rst` reset, `Act` active, `Stat` status,
  `Valid` valid, `Ref` reference, `Flt` filtered, `Out` output, `Hi` upper
  limit, `Lo` lower limit, `Acc` acceleration, `Dist` total disturbance,
  `B0` ADRC critical gain, `Wc` control bandwidth, `Wo` observer bandwidth,
  and `ESO` extended-state observer. Do not abbreviate `Iq`, `Id`, `PU`, or
  established physical units.
- Keep unit and representation suffixes explicit: `_PU`, `_Q15`, `_radps`,
  `_PUps` (PU/s), `_PUps2` (PU/s^2), followed by `_u8`, `_s16`, `_u16`,
  `_s32`, or `_f32`. For a compound unit that cannot be represented clearly
  by the standard tokens, favor a short documented form over an opaque
  abbreviation.
- Examples for future ADRC speed-loop additions are
  `Cal_ADRC_SpdSel_u8`, `Cal_ADRC_SpdRst_u8`, `Cal_ADRC_B0_PUperPUps2_f32`,
  `Cal_ADRC_Wc_radps_f32`, `Cal_ADRC_Wo_radps_f32`,
  `Cal_ADRC_IqHi_Q15_s16`, `Meas_ADRC_ESO_Spd_PU_f32`,
  `Meas_ADRC_ESO_Acc_PUps_f32`, and `Meas_ADRC_ESO_Dist_PUps2_f32`.

Exit gate: adapter has explicit validity status and is testable without FOC
ownership.

### 5. Observer Manager Integration

Only after the adapter passes replay and static checks:

1. Declare whether estimator ownership is runtime-selectable or fixed at
   compile time. Add a `FocEstimatorSelector` value only when runtime switching
   is an explicit requirement; do not retain a non-functional selector or
   compatibility symbol for a fixed-owner production build.
2. Add the required `initialize`, `reset`, `execute`, and `getFocEstimate`
   dispatch in `ExternalObserverManager`, or bind those operations directly in
   the compile-time owner branch.
3. Execute only the declared owner. For a fixed KRE production build, keep
   Flux in a separate diagnostic build rather than as a runtime fallback.
4. Keep algorithm-specific voltage/current timing documented. Do not assume
   FADO delayed-voltage timing applies to KRE, HFO, or a new algorithm.
5. Keep a new algorithm observer-only until its published measurements are
   valid on the bench.

Exit gate: ownership, reset, and invalid-output behavior are deterministic;
each required production or diagnostic build preserves its declared owner.

### 6. FOC Ownership And Startup

An observer may take over Park angle and speed feedback only when all are true:

- the declared estimator owner is the requested algorithm;
- FOC is in `run` and FOC control mode;
- rotor alignment has completed with a non-zero alignment duration;
- the observer has produced a finite, valid estimate during alignment;
- every additional handoff condition is part of the algorithm contract and has
  independent replay and bench evidence. Do not add an integration-layer flux
  or signal-magnitude threshold merely as a precaution.

Estimator validity may deny angle and speed ownership, but it must not disable
the power stage, control enable, alignment-current generation, or the selected
I/f route.

For direct closed-loop startup, preserve rotor alignment, warm the observer
during alignment, consume the handoff event once, initialize the speed PI and
rate limiter deliberately, and retain closed loop through the intended low-
speed behavior. Do not add blind angle takeover or automatic fallback to Flux
unless explicitly required and validated.

Exit gate: the handoff is reproducible; invalid estimates do not drive Park
angle or PWM control.

### 7. Build And Bench Validation

Run in this order:

1. MATLAB reference/replay.
2. Generated C static check and replay.
3. IAR build for every declared ownership configuration. Link both branches
   only for a runtime-selectable design; otherwise build Flux diagnostic and
   fixed-owner production images separately.
4. Observer-only bench run when that stage is part of the declared integration
   workflow; compare `Meas_<Algo>_*` against expected behavior without changing
   FOC ownership.
5. No-load selected-estimator startup.
6. Low-speed, zero-speed if applicable, reversal, load, and parameter-error
   tests.
7. Force invalid input or invalid parameters and verify the algorithm-contract
   response: deny estimator ownership while preserving alignment and the
   selected startup route, or perform a safe stop only when an independently
   specified fault policy requires it. Never switch estimators implicitly.

During every bench test, capture angle, speed, `Iq_ref`, `Iq_fb`, key algorithm
states, validity/status, current-loop timing, and speed-loop timing. Diagnose
startup stutter by separating angle discontinuity, speed-PI/ramp transient,
current-loop tracking, voltage timing, and ISR overrun before changing gains.

Exit gate: no NaN/Inf, no ISR timeout, no unexplained angle jump, and Flux
regression remains clean.

### 8. Commit And Handoff

1. Stage only source, intentional generated C/H/data files, IAR-project edits,
   adapter code, and deterministic test scripts that belong to the algorithm.
2. Exclude caches and unrelated user work.
3. Commit a concise baseline after each bench-validated integration milestone.
4. Report what was tested, what was not tested, selected calibration values,
   firmware artifact hash, and remaining risks.

## Standard Deliverables For Every Algorithm

- Equation and parameter mapping note.
- MATLAB discrete reference and replay script.
- Independent wrapper model and ERT configuration record.
- Generated-code comparison result.
- C adapter with calibration, measurement, reset, and validity behavior.
- Manager/integration diff only after observer-only validation.
- IAR build result and bench evidence.
- Local Git commit that excludes unrelated work.

## New Simulink Model Readability

Apply these rules to new models. Do not migrate, rename or rearrange existing
models unless explicitly requested. Read `SOGI_discrete_test/MODELING_RULES.md`
and its record template before modeling; README distinguishes existing trial
assets from target interfaces. The user has now authorized the FE interface
and basic-block structure change described below, including its functional
subsystem name. FE structure is implemented; structural/compiled-interface
checks recorded 213 PASS and 0 FAIL. See
`SOGI_discrete_test/reports/fe_physical_inputs/change_record.md`.
Actual FE input validation and numerical simulations are NOT_RUN; structural
results do not imply input-guard or numerical acceptance. BE/Tustin/ZOH remain legacy dictionary-based
implementations pending separate migration; existing helpers and reports do
not prove formal parameter inputs or instance isolation.

### Required modeling workflow

- Follow source TF or state equations -> parameterized MATLAB discretization
  -> script quick-simulation gate -> signal-definition/interface contract -> Simulink model
  -> later structural, visual, numerical and reuse acceptance. These gates
  describe future implementation; do not claim an unexecuted gate passed.
- One source representation is sufficient. When independent TF and SS sources
  both exist, cross-check them. An SS derived from a TF, or vice versa, is a
  derived reference, not a second independent source. Record source/version,
  equations, units, initial conditions, input hold and state/output timing.
- Choose applicable discretization methods for the algorithm and record the
  reasons. Do not require four methods or SOGI fields for general models.
  MATLAB scripts must accept physical parameters, Ts and method and regenerate
  coefficients and responses without loading SLX/SLDD or relying on hidden
  base-workspace values. Constants such as 0 and 1 are allowed; configurable
  parameters and derived coefficients must not be hard-coded.
- Define tolerances before running the script gate. Check poles for linear
  systems and suitable bounds/trajectory criteria for nonlinear systems.
  Record failures, unexecuted checks and unstable exploratory methods; skips
  are not passes. Normally only an actually passed, stable selected baseline
  permits modeling. The current user-authorized FE structural change is a
  recorded exception to running new numerical gates now, not a preflight
  PASS; numerical acceptance remains pending. SOGI may compare FE/BE/Tustin/ZOH;
  if all four form its baseline,
  all four must be stable. Skip and explain unstable FE exploration without
  masking divergence. Repeat the gate after equation/method/baseline changes.

### 信号定义合同

- 接线前先定义信号，区分外部输入、输出、中间量、状态、控制/复位、
  物理参数信号和派生系数。正式接口、共享、状态及记录信号必须有定义；
  纯局部连线可在公式映射中定义，不强制每根线创建对象。
- 每项记录实际物理/数学含义、完整名称与简图名称映射、单位、参考方向/
  坐标和 PU 基准、类型/维度/范围、采样周期与速率、当前/下一拍和延迟
  语义、生产者/消费者、初值及复位优先级、valid/失效/NaN 处理政策、
  记录与 XCP 暴露及实例角色。范围元数据不等于限幅或运行断言；
  异常处理由算法合同决定，不任意添加限幅、替代值或 valid 端口。
  SOGI 的 1 PU 是试例输入幅值，不是通用量程；u、D/Q 和状态范围须由
  工况确定，不能机械限定 [-1,1]，未确定填“待定”且不宣称该门禁 PASS。
- 需要统一类型、单位、范围、记录或代码接口的信号，在试例独立 SLDD 中
  定义 `Simulink.Signal`，记录实际线名与对象同名映射并显式绑定/解析。
  对象存在不代表已绑定。采样时序等在端口/块配置和合同中定义，不能
  承诺全部合同属性均在 Signal 对象中。核心仍从端口取得数值，字典
  类型和元数据不能成为隐藏参数来源。同作用域每个命名信号只有一个生产者，
  多实例状态及实参保持独立，不能因同名全局对象而共享状态。
  区分 Parameter 外部标定值与 Signal 信号属性，不得覆盖现有 Cal
  Parameter。端口显示名、实际线名、对象名及类别分别记录；显式解析
  的线名匹配 Signal 对象名。同名被占用时另取唯一线名/Signal 对象名，
  保留原 Cal 端口和 XCP 标识，不能强制绑定错误对象类型。
- SOGI 的 Input_PU/u 为 PU 标量输入，Reset_States 为非零有效的 uint8；
  f0/k/Ts 的 Cal 全名端口仍是 single 参数信号输入。D/Q 是当前物理
  同相/正交输出，Dnext/Qnext 是下一拍状态候选；不得混用拍次或把 Q
  更新接到 Dnext。D/Q 的既有 XCP 名称保留，简图名须记录其映射。
  FE 的 Dnext/Qnext 由复位选择后的当前输入/状态计算并写入延时；复位
  有效且参数有限时为零。零初值属于延时状态，不要求任意首拍候选为零。
- 顺序为定义合同 -> 按定义接线/绑定 -> 编译核对传播后的类型、维度、
  采样周期及实际对象解析 -> 记录结果。新增信号定义规则本轮仅补文档，
  不要求改动模型或实际对象；未执行核查记 NOT_RUN，不能把元数据或
  既有结构 PASS 当成信号绑定、取值约束或异常策略已验收。

### Parameter input and instance contract

- Parameterization must be part of the model's formal inputs. Choose separate
  parameter ports or a typed bus according to reuse needs. A dictionary or
  configuration panel alone does not constitute a parameter input interface.
  Specify signal/reset/parameter/output names, order, types, dimensions, units,
  valid ranges, initial/reset behavior and sample timing before modeling.
- Core parameter values must come through those inputs. Do not use evalin,
  slResolve, direct dictionary queries or globally bound Gain coefficients
  to obtain numeric parameters. Dictionary-based type definitions are allowed.
  Implement the selected difference equation directly from physical parameter
  inputs, or unpack externally derived coefficients, using basic operations.
  Mathematical constants such as 0, 1 and 2*pi may use Constant/Gain blocks.
- Record whether the method uses a direct physical-parameter difference
  equation or an offline coefficient snapshot. Offline MATLAB derivation and
  reference remain required; offline Ad/Bd/Cd/Dd inputs are not mandatory for
  every method. If supplied, derived coefficients are read-only and generated
  together externally. Freeze all parameter inputs for the run. The external
  wrapper must check actual inputs, not merely dictionary values, for types,
  finite values, ranges, stability, Ts consistency and any physical/derived
  coherence. FE's external input validation is pending, not a runtime guard
  proven by the current structural change. Checks and reset must not write
  parameters, models or dictionaries.
- Each instance owns its state, reset and parameter input independently.
  Shared mutable globals must not couple instances. Reset clears that
  instance's states without recomputing parameters or affecting another
  instance. Different instances may receive different valid external snapshots.
- Prefer inherited instance sample time, or configure it explicitly in an
  external wrapper. A Ts parameter port or field describes the design period and must
  match compiled instance scheduling; it does not control runtime scheduling.
  To change Ts, stop, rediscretize and recompile. Do not promise runtime Ts
  writes change the sample rate.
- The authorized SOGI FE subsystem is SOGI_DualOutput_Filter_FE. Its inputs
  are Input_PU, Reset_States, Cal_SOGI_F0_Hz_f32, Cal_SOGI_K_f32 and
  Cal_SOGI_Ts_s_f32; outputs are InPhase_D_PU and Quadrature_Q_PU.
  Signal, parameter and state arithmetic is single; reset is scalar uint8,
  nonzero active. Use real connected parameter inputs in the equations:
  Dnext = D + Ts*2*pi*f0*(k*(u-D)-Q);
  Qnext = Q + Ts*2*pi*f0*D.
  Expand with Product, Sum, constant 2*pi Gain and Unit Delay blocks.
  Output current physical D/Q, then update; reset takes priority and makes
  outputs and stored next states zero. Unit Delays inherit instance scheduling.
  Ts participates in arithmetic and must match the external sample period;
  it must not be advertised as a runtime sample-rate control.
- The comparison model receives the three physical parameter inputs and
  routes them to FE. The validation wrapper supplies dictionary-backed
  Constants externally. FE must not read dictionary coefficients or carry
  dummy parameter ports. BE/Tustin/ZOH are still dictionary-based, with their
  formal parameter interfaces deferred. The experimental 51-field Params bus
  and its helpers are not used or accepted in this FE change.
- SLDD may hold type definitions and external baseline configurations. Keep
  the SOGI dictionary as the unique saved baseline source, while allowing
  independent reuse from valid external snapshots. No hidden base workspace
  or duplicate same-name numeric sources. The blue parameter panel is only
  external configuration assistance, not a substitute for real parameter input ports.
  Show full symbols, units and actual dictionary entries, distinguish saved
  from pending edits, and never display temporary test inputs as saved data.
  The SOGI panel uses us for Ts; scripts/storage use seconds. Configuration
  writes must reject running/Fast Restart/pending-edit conditions.

### Legacy dictionary trial before migration

Retain the existing read-only gates for the unmigrated dictionary trial,
including BE/Tustin/ZOH:
InitFcn uses `sogi_check_parameter_binding(model,false)` for the effective
51 values and coherence via `Simulink.data.resolveInGlobal`; StartFcn uses
`sogi_check_parameter_binding(model,true,true)` and `CompiledIsActive` to
reject active unstable methods. Saved Commented values do not prove temporary
SimulationInput overrides took effect. Default static calls inspect the
model's own Commented configuration and poles. No callback writes the model
or dictionary. These legacy global checks do not validate FE's physical
parameter ports or a future bus. External input validation is pending; do not
lock it to these callbacks or claim that the real input interface is checked.

### Functional naming

- Names plus interfaces must identify function and role without opening core
  internals. Use `<algorithm>_<function>_<role>.slx`, for example
  `sogi_dual_output_filter_discrete.slx` or
  `sogi_dual_output_filter_validation.slx`. Method subsystems should express
  algorithm/function/method, such as SOGI_DualOutput_Filter_FE or SOGI_Filter_Tustin.
- Basic blocks need meaningful roles, such as Weight_State1_For_D,
  Sum_D_Output, Update_State1 or Delay_State1. Names with adjacent annotations
  may jointly explain details; do not require unnecessarily long block names.
  Default Gain1/Sum1/Subsystem2 or isolated A11 names are not final deliverables.
  Names must be unique within a scope. Keep matrix symbols in parameters and
  equation mapping annotations; distinguish auxiliary from physical states.
- Distinguish reusable instances by actual roles such as Voltage, Current,
  Alpha or Beta, not only a number. Add units/method/role only when useful
  for identification. Preserve full existing XCP identifiers; do not rename
  them to match new block labels. The FE subsystem and its internal roles may
  be renamed within the authorized structural change, updating dependencies.
  Other model-file and method renames remain future work.
- Do not embed configurable frequency, gain or sample-period values in a
  reusable model name. Record them in instance configurations and test data
  so the functional name remains accurate after parameter changes.

### Layout and later acceptance

The user subsequently authorized applying the sizing rules. The two SOGI
trial models have been resized and rerouted across seven scopes; see
SOGI_discrete_test/reports/port_aware_layout/layout_review.md for actual evidence.
This is a geometry-only change, with compilation and numerical simulation
NOT_RUN. See SOGI_discrete_test/MODELING_RULES.md sections 4.1-4.7 for the
detailed policy and section 4.2 for block-category dimensions.

- After applying geometry, reread actual block Position: Simulink may
  normalize requested coordinates. Require the final geometry, not just the
  requested values, to meet the grid and port-spacing policy. For wire ends,
  use actual PortConnectivity coordinates and endpoint identity; do not
  hard-code the input-arrow offset observed in PortHandles.Position.
- Check the adopted clearance between unrelated parallel segments whose
  projections overlap. SOGI uses 20 model units in this layout acceptance;
  one-unit separation must not pass merely because lines do not intersect.
  Reserve measured signal-label width plus 20 units before a bend, or a
  separately reviewed label segment. No subsequent segment may cross text.
  Missing signal-label bounds remain automatic NOT_RUN; review every named
  line in the final export separately. Clear temporary find highlights before
  exporting, preserving intentional functional colors.
- Include these checks in the sizing workflow after routing and before
  archival; record actual grid/connection positions, channel clearance and
  line-label manual evidence. Old fixed-coordinate SOGI helpers are historical
  and can overwrite the new layout; follow the current report's scoped
  model_edit and routing procedure when reproducing it.

- Arrange ordinary paths in fixed left-to-right columns: input, preprocessing,
  algorithm, postprocessing/error, output. Omit unused stages without reversing
  order. Put parallel branches and signal/reset/parameter inputs on separate
  rows. Forward consumers must be right of producers even across local tags.
- Use orthogonal lines with zero diagonal segments and zero crossings through
  unrelated block interiors. Do not overlap unrelated signals or obscure
  names/labels/annotations. Use local Goto/From for long fan-out where helpful;
  one visible producer per tag, no global/scoped hiding of dependencies.
  Explicitly inspect different-signal intersections, collinear overlaps and
  T-shaped false connections as well as diagonal lines and block collisions.
  Route reset, zero-value and other distinct signals through separate channels.
  Unify sizes only within groups sharing function, port signature and display
  form; review each branch's full names separately. Avoid meaningless detours that disguise
  reversed semantic order.
- When the operation is commutative, choose addition/multiplication input ports
  according to the sources' top-to-bottom order so an upper parameter does not
  cross the main signal. Preserve subtraction, division and matrix operand
  order; never change their semantics for appearance. Check sign/operand
  mapping after any permitted port swap.
- Keep state updates/returns in a separate lower area with state and current/
  next-sample meaning. Continuous-state readout for ordinary output packing
  is forward postprocessing and stays right of its producer, not a feedback
  exception. Published effective states must obey first-sample reset behavior.
- Use Arial 14 pt by default, never below 12 pt; SOGI targets 14 pt
  throughout. Rectangular calculation blocks, including coefficient Products,
  Gains and conversions, must be at least 120 px wide; standard symbols such
  as Sum at least 40 px; Unit Delay at least 70 px; SOGI method subsystems at least
  300 by 170 px (rounded up to the 10-unit grid). SOGI's old SampleHold is a 110 px sampling interface.
- Use a 10-unit model-coordinate grid. Initial port pitch is 20 for short-symbol
  basic blocks and 40 for hierarchical blocks showing full named ports;
  text clearance is 10 and hierarchical internal margins are 20.
  Keep 14 pt text: points are not pixels; measure text at the same coordinate
  scale. Count visible physical ports, so one vector/bus port counts as one.
  Treat top/bottom control ports separately by actual port kind/location;
  an ordinary reset Inport is not a top control port because of its name.
- Fonts, category minimum sizes, text clearance and the applicable minimum
  port pitch are project acceptance requirements; estimation formulas and
  routing/column gaps are adjustable design starting points. Record and
  separately review special-shape deviations. Snap block geometry to the grid,
  but route to actual port coordinates rather than forcing ports onto it.
  Global alignment must preserve actual reset placement, operand order and
  current/next-sample relationships; column labels do not define execution order.
- Set N=max(nL,nR). For N=0 use Hport=0; for N>=1 use
  Hport=mt+mb+(N-1)*p. Initial estimates are
  H = ceil10(max(Hmin, Hport, Hinside));
  W = ceil10(max(Wmin, 2*mx+L+C+R, Wtop, Wbottom)).
  L/R measure only internally displayed side-port labels; C reserves the
  central symbol/space (80 by default for hierarchy). External block names
  have separate occupancy boxes and must not be counted twice in body width.
  These formulas are starting estimates: read actual port coordinates and
  iterate clearances. Handle zero/one-port cases without negative spacing
  or division by zero; preserve category-specific shapes.
- Apply group maximum W/H only to identical function/port/display signatures.
  Different port counts may use separate groups sharing column baselines and,
  when necessary, width; an eight-port block must not enlarge every one-port
  block. Column widths and row heights include bodies, all external text and
  control-interface bounds; reserve routing channels separately. Initial
  horizontal gaps are 60 for basic blocks and 100 for hierarchy, turn channels
  20, and vertical group gaps 60. Adjust for actual collisions: these are
  project starting values, not MathWorks hard limits.
  Compute column widths and row heights from the projection union of occupancy
  boxes after port/baseline alignment, not just the maximum individual width
  or height; account for offsets, long labels and top/bottom control ports.
- Size Model Reference blocks for complete labels and central whitespace;
  the old SOGI 620 by 480 px shape is only a prior-interface reference and
  must be reconsidered for the three new physical parameter inputs. Preserve standard Inport/Outport shapes,
  but inspect full name spacing separately. Widen or space long functional
  names instead of reducing font size.
- Treat 120 px rectangular blocks, 40 px standard symbols and 70 px Unit
  Delays as readable minimum widths, not reasons to scale the whole canvas.
  Keep SOGI Arial 14 pt. First remove excessive whitespace and line length;
  size blocks to their actual content, group each equation by function in rows,
  keep related operations close, and place explanations beside their region.
  Preserve left-to-right flow; serpentine returns must not disguise order.
  Aim to trace one equation group within a normal review window. If it is
  still too wide, consider functional hierarchy with explicit interfaces;
  hierarchy is not an automatic topology change; this update changes rules only.
- Default editor zoom is 100 percent with scrolling. Fit-to-window is only
  navigation. Scrolling does not make an excessively large canvas acceptable.
  Review both whole-diagram scanability and full local text readability; a
  geometric PASS cannot prove convenient human review. New coordinates are
  instance-specific layout choices, not universal fixed targets.
  Disable new hierarchical content previews with
  `ContentPreviewEnabled='off'`; navigate inside for equations and ensure
  thumbnails do not obscure port names. Inspect editor and exported views.
- After adding ports, renaming, changing fonts or expanding bus ports,
  recalculate block -> group -> column/row -> routing -> structural/visual
  checks. In later implementation, use model_edit then model_read/model_check
  for structure. Re-route from actual port coordinates after moving blocks and
  inspect every scope separately; identical topology does not excuse different
  labels. Check functional names, ports, mathematical mapping, semantic flow,
  geometry, fonts, sizes, text space and preview interference manually.
  Review overall compactness and local legibility together. Record before/after
  drawing bounds, width/height reduction and screenshots at the same review
  scale; compare the actual tracing effort, not only automatic geometry.
  Verify propagated signal types, dimensions and sample times against the
  signal contract, and confirm actual object binding rather than object existence.
  Check intersections/overlaps/false T connections between different signals,
  separate reset/zero channels, and operand order after port choices.
  An automatic block-obstacle PASS does not cover line-to-line checks or
  replace manual review; automatic PASS does not certify visual acceptance.
- After implementation, compare model output with the passed script using
  identical input/parameter snapshots, initial state and timing; do not shift
  traces to conceal delay errors. Test two simultaneous instances with
  different parameters and independent resets, proving state/reset/parameter
  isolation. Sequentially retuning one instance does not prove reuse.
  Verify frozen parameters and rejection of mismatched Ts/input snapshots.
- Maintain the record template: source/assumptions; method choice and script
  gate; signal definitions (meaning, full/diagram names, units/frame/PU base,
  type/shape/range, timing, producer/consumer, initial/reset, validity/NaN and
  logging/XCP policy); typed interface, object binding and derivations;
  instance roles and scheduling;
  functional-name/equation mapping; layout before/after bounds and reduction,
  screenshots, overall scanability and local readability; port counts N,
  measured L/R, estimated and actual W/H, minimum actual port spacing,
  group ID, occupancy bounds and reasons for deviations;
  structural/manual/numerical evidence;
  actual PASS/FAIL/NOT_RUN and skipped reasons. Each report proves only its
  executed scope. Existing trial data are not evidence that new goals passed.
  Record every user-requested rule improvement in both rules and workflow.
