# KRE 启动恢复、观测器减负与问题复盘

## 1. 目标、边界与结论口径

本任务的最终目标是 KRE 可靠取得角度与速度所有权并持续闭环运行。启动提供两条路由：定位后直接交接 KRE，或定位后先经过 I/f 拉升再交接 KRE。I/f 只是可选过渡段，不是最终所有者或故障回退；仅证明 I/f 能拖动电机不算恢复完成。生产固件只允许 KRE 取得运行时估算器所有权；Flux 只作为单独的编译期诊断基准，不作为自动回退。

本阶段不包含 HFI 初始位置接管，不修改主 Simulink 模型或 KRE 生成代码。A2L 只删除已经从固件移除的估算器选择器对象，其他地址必须按最终 ELF/map 重新同步。旧符号型死区补偿只能在电流链路恢复后作为受限 A/B 项，不能直接恢复到生产路径。

调查初期的首要假设是：KRE 及若干默认关闭功能的关闭路径占用了过多 20 kHz 快环时间，使 2 kHz 速度中断或前台服务不能持续推进，因此对齐电流指令没有被第二次及后续速度回调建立。后续修复同时涉及参数启动门、命令消费上下文、中断仲裁、KRE 参数应用和估算器所有权，当前没有单变量实机 A/B，不能把唯一根因归结为某一行代码或仅归结为快环超时。

已经确认的代码关系如下：

- 生产构建没有运行时估算器选择器，快环固定执行 KRE；诊断构建固定执行 Flux。问题不是 Flux 与 KRE 在快环中同时完整运行。需要裁剪的是 KRE 参数准备、VAFID 旁路以及 HFI、RRC-DOB、APSFSM 等关闭路径的额外调用。
- 对齐 d-q 电流由 2 kHz 速度回调更新。若速度回调停滞，`dqCommand.q` 可以一直保持为零，这与“启动后没有电流上升”的现象一致。
- 对齐目标默认值为 `D=0`、`Q=0x2B85=11141 Q15`，从零开始在 666 个速度周期内线性上升；2 kHz 下完整对齐约为 333 ms。
- KRE 的快环周期为 50 us。超过该预算或接近预算运行，都可能破坏低优先级任务的确定性。
- RRC-DOB、APSFSM 和 VAFID 的输出资格受闭环条件限制，正常情况下不能直接抵消 I/f 对齐电流；但关闭状态下的重复复位、参数扫描或函数调用仍可能增加负载。

2026-08-31 用户已确认当前板卡与标定配置下问题修复，启动后控制命令能够生效。该确认关闭本次现场症状，不自动代表低速、加载、反转、所有功能组合或生产发布矩阵全部通过；相应结论仍需绑定同版本固件哈希、标定快照和原始采集数据。

## 2. 冻结诊断基线

第一次上电前应在功率关闭状态确认 ELF、map 和手工维护的 A2L 来自同一构建，并记录完整 SHA-256。计划中给出的 `51BC…26A` 只是待核对的缩写标识，不能替代完整哈希。

诊断基线至少记录：

- 源码提交 ID、IAR 工程配置和所有编译宏；
- ELF、HEX、map 的完整哈希及生成时间；
- `.xcp_cal_m4` 的实际占用；
- 本次标定快照和功率级状态；
- 是否为 `FOC_DIAG_FLUX_REFERENCE=1` 诊断构建；
- 负载 profiler 是否启用及其测量开销说明。

诊断量使用现有 XCP 段中的以下信号；除删除失效选择器对象外，本任务不手工同步这些 A2L 地址，台架前必须从最终 ELF/map 刷新：

- `Meas_Foc_FastLoopCount_u32`
- `Meas_Foc_SpeedLoopCount_u32`
- `Meas_Foc_IdleSvcCount_u32`
- `Meas_Foc_AlignRemain_tick_u16`
- `Meas_Foc_FastLoopLastCycles_u32`
- `Meas_Foc_FastLoopMaxCycles_u32`
- `Meas_Foc_FastLoopOverrunCount_u32`
- `Meas_Foc_FastLoopBudgetCycles_u32`
- `Meas_KRE_RstCount_u32`
- `Meas_KRE_RstReason_u8`
- `Meas_KRE_ParamValid_u8`
- `Meas_KRE_ParamApplySeq_u32`
- `Meas_KRE_ParamPending_u8`

`IdleSvcCount` 只用于证明前台服务持续推进，不要求固定频率或与中断计数形成固定比例。

## 3. 首次裸 KRE 测试的固定条件

首次测试必须在使能前写入并回读以下条件，运行期间禁止下载、刷新或应用参数：

| 项目 | 固定值或要求 |
| --- | --- |
| 启动方式 | 首先验证 `Cal_FocStartupMode_u8=0`：定位后直接交接 KRE；再以 `1` 验证“定位 → I/f → KRE”过渡路由。其他值按路由 `1` 处理 |
| 估算器 | 生产构建固定 KRE，无运行时选择值 |
| HFI | OFF，`Cal_Hfi_Enable_u8=0` |
| APSFSM | OFF，选择值为 `0` |
| RRC-DOB | OFF，选择值为 `0` |
| VAFID | OFF，不执行 probe、eligibility 或 observer step |
| Id 参考 | Zero；MTPA 输出、IdMap 和测试覆盖关闭 |
| 速度控制器 | PI，ADRC 选择值为 `0` |
| 直接接口 | 关闭 |
| 强制占空比 | 关闭 |
| ADC/PWM 采样偏移 | `Cal_MAS_SampleOffset_tick_u16=120` |
| 相电阻 | source/applied 均回读为 500 mOhm |
| d/q 电感 | source/applied 分别回读为 1300/1380 uH |
| 极对数 | source/applied 均回读为 4 |
| 永磁磁链 | source/applied 均回读为 46 mWb |
| 参数状态 | Motor 与 MTPA 状态成功；KRE `ParamValid=1`、`ParamPending=0` |
| 对齐参考 | D 起点/目标为 0；Q 起点为 0、目标为 11141 Q15；时长 666 tick |

源标定的默认 PM 磁链为 46 mWb，而 applied 变量在静态初始化时可能为零。因此不能只检查源变量；裸 KRE 台架开始前必须回读初始化阶段已形成 applied 值和 KRE 有效快照。这是测试配置检查，不得实现为 `enableControl`、`enablePowerStage`、定位或 I/f 的运行时门；参数或 KRE 输出无效时只拒绝 KRE handoff。

## 4. “无电流上升”诊断链

从使能前开始连续采集至少 100 ms，并保留同一时间轴。100 ms 内理论增量约为 2000 个快环和 200 个速度环；窗口边界、抢占和读取不同步会产生少量误差。所有 `uint32` 计数差值应按无符号回绕计算。

### 4.1 调度是否推进

首先比较：

```text
fast_rate  = delta(Meas_Foc_FastLoopCount_u32) / delta_time
speed_rate = delta(Meas_Foc_SpeedLoopCount_u32) / delta_time
ratio      = delta(fast_count) / delta(speed_count)
```

正常目标为快环约 20 kHz、速度环约 2 kHz，计数斜率比约为 10:1。`IdleSvcCount` 应持续增加；它没有固定目标斜率，但在电机运行期间长期不变表示前台服务被饿死或主循环被阻塞。

同时采集 Last、Max、Budget 和 Overrun。`Budget` 对应 50 us，任何 `Overrun` 增量都判为失败。若速度环计数停滞且 Overrun 增长，快环超时假设成立，应先完成关闭路径裁剪，不能通过增大对齐电流掩盖问题。

### 4.2 状态机与对齐回调

若速度环正常，则检查控制使能、功率使能、FOC state/substate、actual control mode 和 `Meas_Foc_AlignRemain_tick_u16`。

进入 `run` 后 1～2 个速度周期内，对齐剩余计数应开始递减。Q 电流参考应从零变为正值并向 11141 Q15 上升。按默认线性斜坡估算，100 ms 后约为 3340 Q15；该数值只用于波形合理性判断，严格判据是计数持续递减、Q 指令单调向目标移动。

### 4.3 指令到功率级的顺序定位

必须按以下顺序定位首个断点，不能跨级调参：

| 观测结果 | 结论 | 下一步 |
| --- | --- | --- |
| 速度环停滞，Overrun 增长 | 快环超时或抢占异常 | 裁剪关闭路径，复测计数与预算 |
| 速度环正常，`dqCommand.q=0` | 对齐回调、run 状态或控制/功率使能链中断 | 检查 state/substate、AlignRemain 和实际使能；KRE 参数状态不得钳制此阶段 |
| `dqCommand.q!=0`，FOC d-q/alpha-beta 电压为零 | 电流 PI、限幅或电压输出门控中断 | 检查 PI 状态、限幅、直连接口和控制模式 |
| FOC 电压非零，调制器实际电压为零 | 调制器、故障、功率使能或强制占空比路径异常 | 检查 modulator state、fault、PWM enable 与 force-duty |
| PWM 非零，采样电流仍为零 | 软件已把命令送到功率级 | 转入栅极、母线、ADC 触发、采样偏移和电流重构检查 |
| 电流已建立，KRE 仍不能交接 | 启动电流问题已解决，剩余为估算器输出问题 | 检查端电压模型、valid/status、raw omega、复位和角度连续性；活动磁链仅作诊断，不再额外钳制交接 |

每个阶段至少同步采集以下信号组：

- 快环/速度环/前台计数，Last/Max/Budget/Overrun；
- FOC state、substate、actual control mode、控制与功率使能、对齐剩余计数；
- d-q 指令与反馈、alpha-beta/UVW 电流；
- d-q/alpha-beta 电压、调制器状态、实际电压、PWM 占空比；
- KRE valid、status、角度、速度、`Meas_KRE_ActFlux_Wb_f32`、`Meas_KRE_RawOmega_radps_f32`、复位计数/原因；
- Motor/MTPA/KRE 参数状态与快照序列。

若某结构成员尚未在 A2L 中暴露，可用同版本调试器或现有测量通道采集；不得为了本次诊断自动生成或覆盖 A2L。

## 5. KRE 参数快照与 pending 语义

KRE 参数准备必须从 20 kHz 快环移到停机/standby 的前台服务。一个完整快照由 14 个 KRE/电机/基值参数和 2 个 PLL 参数组成，SI 换算和整组一致性读取都在前台完成；快环只读取已发布的缓存。

状态语义如下：

| 信号 | 语义 |
| --- | --- |
| `Meas_KRE_ParamValid_u8` | 已发布一组完整快照；它不承担物理范围校验，也不表示源标定当前没有新改动 |
| `Meas_KRE_ParamPending_u8` | KRE 实际读取的 applied/KRE 标定快照与当前捕获快照不同，或整组读取不一致；Motor/MTPA 的 source 请求由各自状态独立表示 |
| `Meas_KRE_ParamApplySeq_u32` | 每次成功发布一组新快照后增加一次；未变化或失败不得增加 |
| `Meas_KRE_RstCount_u32` | 统计真正送入生成观测器的复位边沿，不统计同一待处理边沿的重复请求 |
| `Meas_KRE_RstReason_u8` | 最近一次复位边沿的原因，用于区分初始化、外部请求、参数应用和输入/输出异常 |

约束为：

1. 适配器不再增加 Rs/Ld/Lq、磁链、增益或阈值的正值/范围资格门；参数服务只保证一次发布来自同一组稳定读取。
2. 运行中写入只置 `Pending=1`，KRE 继续使用上一组有效 applied 快照，不逐周期换参或复位。
3. 到达 off/standby 后才整组应用。成功应用使 `ApplySeq` 增加一次，并只产生一个参数应用复位。
4. 整组读取不一致时上一组 applied 快照不被覆盖；KRE 是否能交接只由其输出 valid/status 及角度、速度有限值决定。
5. `ParamValid/Pending` 只描述 KRE 参数服务，不得关闭功率级、控制使能或对齐。KRE 交接只使用适配器已经发布的 valid/status 及角度、速度有限值，不叠加活动磁链阈值门。
6. 输入或输出连续无效时，故障复位请求应锁存为一个边沿，禁止每个快环重复增加复位计数。
7. 停机前台以连续双读拒绝读取过程中变化的组合；一致的候选快照立即应用，不依赖软定时器。运行期间仍禁止下载或应用整组参数。

曾加入的 20 ms 稳定门使用 `timer_get_ticks()`，但启用 load profiler 后 CM4 SysTick 没有推进该 soft-timer 计数。结果首次快照永久 pending，再经旧的启动门把 control/power enable 同时清零，造成状态机无法进入 run、对齐计数反复重装以及 dq/PWM/电流全为零。该定时门和启动使能门均已删除。

裸 KRE 台架首次使能前应回读到 `ParamValid=1`、`ParamPending=0` 且 `ApplySeq` 已反映初始化快照。这只是测试配置检查，不得作为 `enableControl`、`enablePowerStage`、定位或 I/f 的软件门；若未满足，只拒绝 KRE handoff 并发布诊断。运行期间三者除故意写入测试外应保持稳定，`RstCount` 不应按快环斜率增长。

## 6. `FOC_DIAG_FLUX_REFERENCE` 构建契约

估算器所有权完全由编译宏确定；`Cal_Foc_EstimatorSelector_u8`、`Meas_Foc_ActiveEstimator_u8`、选择器枚举、set/get API 及定位阶段的选择器快照均已删除，不保留符号地址。

### 6.1 诊断构建：`FOC_DIAG_FLUX_REFERENCE=1`

- 固件固定执行内部 Flux 参考路径；
- Flux/KRE A/B 使用两次独立构建，不能在同一固件中切换；
- 不提供运行时估算器切换或自动回退；
- 每次重新启动都要重新对齐并按相同条件采集。

### 6.2 生产构建：`FOC_DIAG_FLUX_REFERENCE=0`

- 运行路径固定为 KRE，不再读取或归一化任何估算器选择值；
- `Cal_FocStartupMode_u8=0` 为“定位 → KRE”，`1` 为“定位 → I/f → KRE”；其他值按后一路由处理；
- 共享 FOC 结构当前仍包含 `fluxEstimator`，用于 Flux 诊断构建和公共电机参数存储；它不是运行时选择器或必须保留的 XCP 符号地址，生产快环不得调用 Flux；
- map 必须证明没有 Flux 执行或调度符号进入生产运行路径，同时 KRE 与保留的 VAFID 功能正常链接；
- KRE 估算结果在交接前无效时不得取得闭环所有权；不得关闭定位或所选 I/f 过渡路由。交接成功后保持 KRE closedLoop，瞬时无效样本同时保持最后有效角度和速度，不自动退回 I/f 或 Flux。

以上差异必须通过两个宏配置的构建和 map 验证，不能仅凭宏定义存在即判为通过。

## 7. 关闭路径减负要求

| 模块 | OFF 时的要求 | 状态改变时的要求 |
| --- | --- | --- |
| HFI | 调用点直接绕过 `applyPolar`，不做注入计算 | 仅在 enable/active 边沿执行一次复位 |
| APSFSM | 2 kHz 路径不扫描完整标定、不执行补偿 | 模式、控制/速度窗口资格或显式 reset 边沿清状态一次 |
| RRC-DOB | 不执行 compensator、getOutput 或重复清状态 | 选择器、速度/电压新鲜度/参数资格边沿复位一次；Monitor/Apply 功能保留 |
| VAFID | 不调用 probe、fast eligibility、voltage capture 或 observer step；前台不周期扫描 | OFF/Shadow、闭环结构资格或观测器工作点资格边沿才清理；显式 Reset/Apply/Revert 命令仍服务，参数反馈掩码继续锁零 |
| KRE 参数 | 快环不读 XCP 参数、不做 SI 换算和整组校验 | 只在前台 stage，并在停止状态 apply |

裁剪后若完整快环峰值仍不能保留至少 20% 裕量，则停止生产集成。不得用 KRE 降采样或手改生成 C 绕过预算；应另立 MATLAB 离散参考、Wrapper 和 ERT 优化任务，单独处理 `expf`、`sqrtf`、`atan2f`、`floorf` 等核心成本。

## 8. 分阶段实机采集矩阵

每一阶段都沿用第 3 节的基础条件，只打开本阶段明确列出的功能。任一阶段失败即停止叠加功能，返回上一个已通过基线。

| 阶段 | 构建/配置 | 主要采集 | 通过门槛 | 是否必须台架 |
| --- | --- | --- | --- | --- |
| 0. 功率关闭基线 | 核对同一 ELF/map/A2L、完整哈希和标定回读 | build ID、哈希、参数状态、section 占用 | 文件一致、无意外参数写入 | 是 |
| 1. Flux/KRE A/B | 分别构建 Flux 诊断固件和 KRE 生产固件；全部新增功能 OFF | 20k/2k/idle、FOC 状态、dq、电压、PWM、电流、KRE 全部诊断 | 两次测试条件相同；KRE Overrun=0；对齐链完整 | 是 |
| 2. 裸 KRE 交接 | 分别验证“定位 → KRE”和“定位 → I/f → KRE”；全部附加功能 OFF | 角度、速度、valid/status、ActFlux、rawOmega、reset、所有权事件 | 两条路由最终均由 KRE closedLoop 接管；交接前无复位风暴、NaN/Inf 或不可解释角跳 | 是 |
| 3. APSFSM Shadow | 仅 APSFSM Shadow | 基线信号加 torque estimate、补偿输出/活动状态 | 不改变基线启动；输出受资格门控 | 是 |
| 4. APSFSM Apply | 单独 Apply | dq 基值/补偿值、电流和负载 | 启动与交接不回退，限幅有效 | 是 |
| 5. RRC-DOB Monitor | APSFSM/HFI/VAFID OFF | RRC 状态、残差、输出资格、快环负载 | Monitor 不取得电压所有权，负载仍满足预算 | 是 |
| 6. RRC-DOB Apply | 初始限幅 0.01 PU，100 ms 渐入 | 电压基值/补偿值、dq 电流、KRE 角度与复位 | 渐入连续，无启动破坏或角跳 | 是 |
| 7. VAFID Shadow | APSFSM/HFI/RRC OFF，禁止参数反馈 | probe、eligibility/reject、采样偏移、参数估计、快环负载 | OFF 到 Shadow 边沿确定；不反馈电机参数 | 是 |
| 8. 组合 | 先两两组合，再全组合 | 前述全集与 profiler | 每次只增加一个变量，重复启动/低速/反转/加载均通过 | 是 |
| 9. 异常参数输入 | 置零、非有限值、运行中下载 | ParamValid/Pending/ApplySeq、KRE valid/status、reset、使能、所有权 | 集成层不增加范围门；若 KRE 输出无效则不得交接；定位/I/f 路由不被封锁且无 Flux 回退 | 是 |

### 8.1 每阶段通用控制效果门

每个阶段必须先上电回读命令源、启动路由、使能和速度默认值，不得靠修改默认值获得“通过”。除本阶段特有信号外，统一验证两条链路：

1. 定位链：`AlignRemain` 递减，`Iq_ref -> Vdq -> PWM -> 实际电流` 依次建立；KRE 无效只能阻止交接，不能使定位 `Iq_ref` 归零。
2. 运行链：`命令源 -> 请求速度 -> 速度边界锁存值 -> 有效速度 -> 速度 PI/Iq_ref -> Vdq -> PWM -> 电流/实际转速` 连续有效。

KRE 交接后至少执行非零速度阶跃、变速、停止、反转和重复启动，同时确认 20 kHz/2 kHz 计数斜率约为 10:1、前台持续推进且 Overrun 不增加。每增加一项补偿或辨识功能，都必须重复以上门槛；“电机能转”或“I/f 能拖动”不能替代控制效果验证。

每次台架记录必须包含：测试日期、操作者、板卡/电机/母线配置、固件完整哈希、标定快照、采样率、触发条件、原始数据文件和结论。截图不能替代可重放的原始数据。

## 9. 验收清单

### 9.1 调度与启动

- 快环与速度环计数斜率约为 10:1，前台计数持续增加；
- 完整快环峰值不超过 50 us 预算的 80%，即至少保留 20% 裕量；
- `Meas_Foc_FastLoopOverrunCount_u32` 在所有验收窗口内不增加；
- 进入 run 后 1～2 个速度周期内 AlignRemain 开始递减；
- 对齐阶段按顺序出现 Q 电流指令、FOC 电压、调制器/PWM 和实际电流。

### 9.2 KRE 与参数

- 启动前应确认参数快照有效且无 pending；异常参数不得覆盖上一组有效快照；
- 运行中写参不改变 applied 快照、不逐周期复位；停机后整组应用一次；
- I/f 只作为可选启动过渡段；两条启动路由最终都必须由 KRE 接管。KRE 交接前 valid/status 和角度连续性正常，活动磁链仅作诊断，交接后 substate 持续为 closedLoop；
- 无 NaN/Inf、复位风暴或不可解释的角度跳变；
- 异常参数不得关闭定位或 I/f 路由；交接前 KRE 输出无效时拒绝其所有权。交接后的瞬时无效不得改变路由或制造估算速度清零阶跃。

### 9.3 构建与交付

- IAR CM4 `Multi Motor Evalkit V1.0` 构建为 0 错误且无新增警告；
- `.xcp_cal_m4 <= 0x400`；
- 生产 map 中无 Flux 运行/调度符号，KRE 与 VAFID 正常链接；
- 记录最终 ELF/HEX 完整哈希、标定值、负载数据、台架覆盖范围和未验证风险；
- 主 Simulink 模型和 KRE 生成 C 没有被本任务修改；A2L 已删除两个失效选择器对象，其余地址须由最终产物刷新。

## 10. 软件可验证项与必须上台架项

### 10.1 已完成的软件证据（2026-08-31）

- 两个 `FOC_DIAG_FLUX_REFERENCE` 分支的编译期所有权语义；
- KRE 参数 double-read/stage/apply、无额外物理范围门、pending、ApplySeq 和单次复位语义；
- HFI、APSFSM、RRC-DOB、VAFID 的 OFF 路径调用关系和边沿复位；
- KRE 生成代码和主 Simulink 模型未被修改；A2L 仅删除两个失效选择器对象；
- IAR 编译、链接、map 符号、section 大小和产物哈希；
- 可独立执行的 host/unit/replay 测试结果。

本次已形成以下软件证据：

- profiler 基线提交：`142172315794deec34936e079e4fe5d73bcd17dd`（`feat(cm4): add real-time load profiling`）。
- IAR 9.40.1、`Multi Motor Evalkit V1.0`：诊断构建 `FOC_DIAG_FLUX_REFERENCE=1` 为 0 error；Flux 参考分支可编译。该诊断产物随后被最终生产构建覆盖。
- 最终生产构建 `FOC_DIAG_FLUX_REFERENCE=0`：0 error / 14 条既有 warning；map 中无链接后的 `Ifx_MDA_FluxEstimatorF16_*` 运行符号和估算器选择器符号，KRE execute/get-estimate 与 VAFID 功能正常链接。
- 最终 `.xcp_cal_m4` 为 `0x36e`，小于 `0x400`。
- KRE adapter IAR/C-SPY host test：四组编译和链接均 0 error / 0 warning，运行输出 `KRE adapter tests passed`；覆盖快照待应用语义、无额外物理范围门、单次复位、VAFID OFF 旁路及仅按 observer valid/status 交接。
- APSFSM adapter IAR/C-SPY host test：编译、链接均 0 error / 0 warning，运行输出 `APSFSM adapter tests passed`；覆盖参数 valid/invalid 恢复边沿及持续无效不重复清状态。
- VAFID 既有 IAR/C-SPY host test运行输出 `PASS: VAFID host tests`；验证 OFF/Shadow 相关边沿修改后测试仍通过。
- 最终 ELF SHA-256：`6F5B10925FBDF740E438EF2EAA2747A73A7C52359E3ABA3FF6CAA08E422DE3BD`。
- 最终 HEX SHA-256：`DB5F3EEC17EA9C532E737B7AFF3D69C39DC2B13D06B6D6D07A403BE2B3ABD77D`。
- 最终 map SHA-256：`4095173428F828EB6ADCE9F744E75B78018C16768317272EDBDB208F375AF7B9`。

软件证据不能替代第 10.2 节的实机证据。用户已确认当前配置下“无电流上升、启动后命令无控制效果”的现场症状恢复；实际快环峰值、20% 裕量以及完整覆盖范围仍须用同版本原始记录验收。

### 10.2 必须上台架，软件检查不能替代

- 20 kHz/2 kHz/idle 在真实中断优先级下是否持续推进；
- 完整 ISR 的 Last/Max/Overrun 和至少 20% 实时裕量；
- d-q 指令到 FOC 电压、PWM、相电流的完整物理链路；
- ADC 触发、120 tick 采样偏移与电流重构是否正确；
- KRE 在定位期间的端电压模型、活动磁链诊断、角度连续性和直接交接；
- 重复启动、低速、反转、加载、非法参数以及功能组合；
- Flux/KRE 两个编译配置 A/B 的实机差异和无自动回退行为。

当前可报告“用户确认当前配置下 KRE 启动和控制效果已恢复”。在同版本 ELF 哈希、标定快照、原始波形和测试覆盖补齐前，不得扩展为“所有路由/组合均通过”或“生产发布已验收”。

## 11. 证据记录模板

```text
源码提交：
IAR 配置：
FOC_DIAG_FLUX_REFERENCE：
ELF SHA-256：
HEX SHA-256：
map SHA-256：
.xcp_cal_m4 占用：
编译错误/新增警告：

Rs/Ld/Lq/polePairs/FluxPM applied：
Motor/MTPA/KRE 参数状态：
KRE ParamValid/Pending/ApplySeq：
附加功能开关：
命令源及上电回读值：
启动路由及上电回读值：
请求速度/锁存速度/有效速度：

采集窗口：
delta fast/speed/idle：
fast Last/Max/Budget/Overrun：
FOC 状态/子状态/KRE handoff：
对齐计数与 Iq 指令：
电压/PWM/电流链路：
KRE valid/status/ActFlux/rawOmega/reset：

已通过范围：
未验证范围：
原始数据路径：
结论：
```

## 12. 已修复问题复盘与后续硬规则

### 12.1 症状与已确认的修复机制

本次问题不是单一的“KRE 算法不收敛”。现场症状依次表现为：定位阶段 `Iq` 为零、无电流上升且无法启动；随后能够转动，但标定转速、启动和停止命令没有控制效果。把 `Cal_FocCommandSource_u8` 默认值改为 `1` 只改变命令数据源，不能修复调度、状态机或 KRE 交接问题，最终默认值保持为 `0`。

代码与软件证据确认了以下相互叠加的机制和修复：

- 曾加入的 20 ms 参数稳定门依赖未推进的 soft-timer，使首次参数快照永久 pending；旧启动门又据此关闭 control/power。修复删除该时间门和参数对启动使能的钳制。
- 启停和速度命令原先依赖前台消费；KRE 快环负载高时前台可能不能按时服务。修复后命令在确定性的 2 kHz 速度状态机边界锁存，前台不再是命令生效的必要条件。
- 20 kHz 快环和 2 kHz 速度环访问同一 FOC 对象。修复后的中断配置使二者采用相同抢占优先级，避免通过速度环嵌套快环来换取表面响应；仍须用计数斜率和 Overrun 证明调度裕量。
- KRE 参数改为前台整组 stage、停机一次 apply；运行中只置 pending 并继续使用上一组快照，不再逐周期换参或复位，也不增加重复的物理范围启动门。
- 生产固件使用编译期 KRE 所有权，删除失效 selector 和符号兼容逻辑。KRE 交接后的瞬时无效样本保持最后有效角度与速度，不清零速度，也不自动退回 I/f 或 Flux。
- HFI、APSFSM、RRC-DOB 和 VAFID 的稳定 OFF 路径以调用点旁路和边沿复位为原则，避免“功能已关闭但快环仍持续执行”。

由于当前工作树包含多项同时修改，且没有修复前后同工况 DWT/XCP 单变量 A/B，本复盘不声称其中某一项是唯一根因，也不量化各项节省的周期。

### 12.2 本次无效或高风险做法

- 通过修改命令源或启动模式默认值碰运气；
- 为保险而增加没有算法依据的参数范围、磁链阈值或 `ParamValid/Pending` 启动门；
- 恢复运行时估算器 selector、无用符号地址或 Flux 自动回退；
- 只看到电机在定位/I/f 阶段转动，就认定 KRE 已闭环且速度命令有效；
- 在裸 KRE 未完成启动与控制效果回归前，同时叠加死区补偿、APSFSM、RRC-DOB 和 VAFID；
- 用编译成功、host test、map 代码尺寸代替实测 WCET 和实机控制链证据。

### 12.3 后续修改的不可破坏约束

| 层级 | 不可破坏约束 | 最小证据 |
| --- | --- | --- |
| 命令 | 启停、速度和使能在 2 kHz 状态机边界生效，不依赖前台 | 请求值、锁存值、有效值及阶跃响应 |
| 定位 | 参数或估算器无效不得关闭功率、控制、定位 `Iq` 或所选 I/f 路由 | `AlignRemain -> Iq -> Vdq -> PWM -> 电流` |
| 调度 | 20 kHz/2 kHz 不得嵌套访问共享 FOC 对象，前台持续推进 | 计数约 10:1、idle 增长、Overrun=0、WCET 裕量至少 20% |
| KRE 所有权 | 只按已定义的 valid/status 和有限值交接；生产不自动回退 Flux | handoff 事件、角度/速度连续性、复位计数 |
| 参数 | 整组 stage，停机原子 apply；运行中 pending 使用旧快照 | ParamValid/Pending/ApplySeq 和单次 reset |
| 功能 OFF | 调用点旁路算法 step/扫描，资格边沿只复位一次 | OFF 前后调用计数和 DWT 负载对比 |
| 交付 | 修复结论必须绑定实际刷写产物和可重放原始数据 | ELF/map/A2L 身份、标定快照、数据路径、覆盖范围 |

### 12.4 强制集成顺序

1. 冻结并刷写裸 KRE 基线，所有附加功能 OFF；先验证两种启动路由和完整控制效果门。
2. 完成 replay/host test、IAR 构建、map 检查和 DWT 负载采集；20 kHz 路径变化不得只看代码尺寸。
3. 每次只加入一个功能和一个行为变量，先 Shadow/Monitor，后 Apply；单独提交、单独刷写、单独留证据。
4. 每项都重复定位、KRE 交接、速度阶跃、变速、停止、反转和重复启动。任一门失败立即回到上一个已通过基线，不继续叠加。
5. 单项全部通过后才进行两两组合和全组合，并记录尚未覆盖的低速、负载、温度和异常参数风险。

本次已确认修复的实际提交、修复后实机 ELF 哈希、标定快照和原始波形路径仍应按第 11 节补齐；不得用本文件中早期软件构建的哈希代替实际刷写产物。
