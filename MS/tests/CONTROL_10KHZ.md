# PWM 20 kHz / FOC 10 kHz 软件交付记录

本文件记录原始裸 KRE 降频基线。后续 RRC-DOB Monitor 接入、用户报告的
53% 基线负载和新固件哈希见
[RRC-DOB 100 us 记录](../../RRC_DOB_codegen/CONTROL100US.md)。

状态：**软件测试与 CM4 构建通过；实机待验证（NOT_RUN）**。日期：2026-09-16。
本次没有下载固件、启动电机或测量实际负载。人工验收表见
[control10k_acceptance/bench_record.md](control10k_acceptance/bench_record.md)。

## 配置和调度

| 项目 | 20 kHz 控制基线 | 本次配置 |
| --- | --- | --- |
| PWM / 中断 | 20 kHz / 50 us | 保持 |
| 电流控制、KRE、调制器提交 | 20 kHz / 50 us | 10 kHz / 100 us |
| 速度、命令锁存、弱磁、速度恢复 | 2 kHz / 500 us | 保持 |
| d/q PI Ki·Ts，Q15 | 40 | 80 |
| d/q PI Kaw·Ts，Q15 | 81 | 162 |
| d/q PI Kp，Q15 | 6997 | 保持 |
| 弱磁 Id 下限持续饱和恢复判据 | 40 个控制周期 / 2 ms | 20 个控制周期 / 2 ms |
| PWM_PERIOD / 调制器半周期 | 4000 / 1999 ticks | 保持 |
| 死区 / 默认采样偏移 | 120 ticks / 250 ticks | 保持 |

唯一控制周期定义在 `ConfigWizard/FocTiming_Cfg.h`。MS、MHA、电流 PI、I/f、V/f
配置从它派生；不支持运行时改频。仅允许 50/100 us，辅助算法启用时仍要求 50 us。
主程序检查 MHA/MS 分频、控制周期和速度周期一致；100 us 不能用于旧 Flux 诊断构建。
ConfigWizard XML/ICWP 仍是原有向导资产；重新导出头文件会覆盖本次手工配置，需要重新审查。

PWM ISR 相位上电为 0；第一次中断保持，第二次执行，之后连续交替。
启停、故障、对齐均不重置分频相位。比较值及 ADC 触发值只在控制调用内提交一次，
保持周期不重复提交。已移除 PendSV 调度和 PatternGen 延迟提交计数路径。
硬件 shadow/capture 更新 API 沿用原实现。

快环与速度回调有效 NVIC 抢占优先级均为 1；未移动计算到前台或改变优先级。
两者不能相互嵌套访问 FOC 对象；前台参数快照发布仍使用原有临界区。
命令源默认 `0`（CAN），启动模式默认 `0`（对齐后直接 KRE 闭环），原 I/f 路线保留。
启动模式 1、对齐、停止及高电流停止逻辑均保留。

## 算法和采样合同

生产所有者仍固定为 KRE。`FOC_AUX_ALGORITHMS_ENABLE=0` 在源文件和调用处排除
HFIPD、HFI、RRC-DOB、APSFSM、VAFID；原死区补偿未链接，仍不执行。
链接符号中已确认 KRE 存在、上述辅助算法入口/标定符号不存在。
FWC、速度 PI、速度恢复和既有 ADRC/IdMap 接口保留，未修改选择器默认值。

KRE 参数准备从新 Ts 重新生成离散系数，生成 C 算法方程未改动。
I/f、V/f 角度积分同步为 100 us；电机参数发布也使用此周期重新计算角度增量。
KRE 速度滤波和 PLL 系数根据 Ts 重新计算；FWC 的 2 kHz 滤波、进入/退出计时不变。
ADC 零偏校准仍平均 128 次采样，以保留平均样本数：约 6.4 ms 变为约 12.8 ms，
它不是运行超时阈值，启动仍等待校准状态完成。

控制调用顺序为：读取 ADC 完整组 → 电流重构 → KRE → 电流 PI → 调制器 → 提交比较值。
100 us 下用**最近一次提交且已保持的 PWM 扇区**重构电流，避免继续使用旧 20 kHz
管线中更早一拍的扇区。读取 Vdc 和两路电流的状态后，只有三路均 valid 才整组发布；
任一缺失保留上个完整输出且 `sampleValid=false`，不混合新旧相电流。
运行/减速期间缺组时计数、使 KRE 按现有无效输入策略失效并保持占空比，不推进电流 PI。
2 kHz 命令/状态机继续运行，进入 standby/fault 等状态后照常执行零电压/关断路径。
未新增 ADC 超时自动停机策略；持续缺组和恢复行为需要在台架确认。

送给 KRE 的 `self->voltageAlphaBeta` 是本次生成新命令**之前**保存的上次调制器
`actualVoltage`，对应已保持的占空比；不把刚计算的新命令当成已施加电压。
KRE 适配器内部另外保留一拍完整 VI_fb 输入延迟（现为 100 us）。
调制器电压是软件模型，未补偿实际死区误差。硬件 ADC valid 位不能单独证明三路来自同一
PWM 窗口；触发完成到读取的间隔、buffer/capture 生效时刻及电压/电流拍次仍为实机验收项。

## 统计含义

| 变量 | 本次含义 |
| --- | --- |
| `Meas_Foc_PwmIrqCount_u32` | 每个 PWM 回调计数，预期 20,000/s |
| `Meas_Foc_FastLoopCount_u32` | 实际进入控制函数次数，预期 10,000/s；ADC 缺组提前返回也计入 |
| `Meas_Foc_SpeedLoopCount_u32` | 速度回调次数，预期 2,000/s |
| `Meas_Foc_PwmIrqLastCycles_u32` / `MaxCycles` | 完整 PWM 回调测量范围，含控制及负载统计收尾 |
| `Meas_Foc_FastLoopLastCycles_u32` / `MaxCycles` | 控制函数调用耗时，不含保持周期 |
| `Meas_Foc_FastLoopBudgetCycles_u32` | 保持为 50 us × SystemCoreClock |
| `Meas_Foc_FastLoopOverrunCount_u32` | PWM 回调耗时达到或超过 50 us 时累计 |
| `Meas_Foc_AdcMissCount_u32` | 运行/减速中未取得完整 ADC 组而跳过控制的次数 |

`Meas_MCU_FocFastLoad_pct_f32` 每个 PWM 回调只结算一次，覆盖保持和控制两类回调。
控制调用没有第二次加入总负载。分段统计只抽取实际控制调用，默认分频 16 时约 625 次/s；
分段百分比不能直接乘总快环负载来推算精确模块负载。
见 `M4_BSW/SOURCE/ld_task/tests/FAST_PROFILE.md`。

验收比例 PWM:控制:速度 = **10:5:1**，要求零 overrun、前台计数继续增长。
按 50 us 中断截止时间保留 20% 裕量，即最坏时间不超过 **40 us**。
DWT 范围从 C 回调入口附近到末尾统计之前，不包含异常入口/出口、调用包装及最后几次写入；
需要台架结合硬件时序确认真实完整 ISR 的 WCET 和速度中断造成的响应延迟。
160 MHz 下 50/40 us 分别对应 8000/6400 cycles；以实际 SystemCoreClock 为准。
本次没有修改 A2L，新符号可用对应 ELF 的 Live Watch 读取，旧 A2L 地址不能视为已重新匹配。

## 已执行的软件验证

| 检查 | 结果与范围 |
| --- | --- |
| 分频/PWM/ADC | PASS：20,000 IRQ → 10,000 控制；保持周期不提交；连续启停相位；三个缺 ADC 通道逐一测试；扇区选择；DWT 回绕和 50 us 超时阈值 |
| 统计开/关 | PASS：两种配置分别验证一次/IRQ 结算及一次/控制启动分段统计 |
| 编译配置约束 | PASS：50/100 us 接受；75 us 和 100 us+辅助算法拒绝，共 4 个用例 |
| 实际定点电流 PI | PASS：生产初始化和 Math PI；相同物理时长积分累积、双轴正负误差、抗饱和缩放、复位 |
| KRE MATLAB/C 回放 | PASS：1,800 行；20 kHz 双子步保持电压、10 kHz 采样、一拍 VI 延迟；正/负转速场景、停转段、重复复位；生产 150 Hz 滤波/100 Hz PLL；IAR -Om |
| KRE 50 us 回归 | PASS：原回放 hash `7b81d650` 保持；100 us 旧向量回放 hash 为 `bb309250` |
| 启动调度 | PASS：两条启动路线各重复 3 次、对齐、观测器预热调用、有效性阻止/允许交接、一次性交接、变更速度符号、缺 ADC 与停机/故障路径、实际 regulationLoop 的 d/q 给定传递 |
| 高电流停止 | PASS：30 checks |
| FWC 适配器 | PASS：精确 2 ms 恢复阈值及既有用例 |
| FWC 速度恢复 | PASS：6,024 checks；生产 limitSpeed/calcCurrentQRef 与真实 PI/斜坡集成 12,037 checks |
| CM4 IAR 9.40.1 | PASS：最终增量构建 0 errors / 3 warnings（已有未使用状态变量）；20 kHz 基线构建 0 errors / 15 warnings |

新测试提取生产 ISR、PatternGen、ADC、控制/启动函数，外设及部分状态/算法依赖用测试替身。
启动调度测试不证明真实电机闭环启动成功；计数测试中的 2 kHz 事件为模拟调度。
KRE 回放使用独立 MATLAB 离散参考和合成 RL/反电势输入，证明离散实现一致性，
不代表实际 IPMSM 全工况闭环稳定性。电机来源默认 Ld=1.30 mH、Lq=1.38 mH。

回放预设容差：角度 0.002 rad、速度 2 rpm、磁链 5e-5 Wb、原始电角速度 1 rad/s。
实测最大误差依次为 `9.53674316e-7`、`8.54492188e-4`、`2.23517418e-8`、`7.01904297e-4`。

复现顺序（从仓库根目录；各 KRE 测试共享输出目录，顺序执行）：

```matlab
addpath('KRE_codegen/tests');
generate_kre_control10k_vectors;
```

```powershell
& MS/tests/run_foc_control_rate_tests.ps1
& MS/tests/run_foc_control10k_startup_tests.ps1
& MS/tests/run_foc_stop_current_tests.ps1
& KRE_codegen/tests/run_kre_external_observer_adapter_tests.ps1 -ControlPeriodUs 50 -Optimization '-Oh'
& KRE_codegen/tests/run_kre_external_observer_adapter_tests.ps1 -ControlPeriodUs 100 -Optimization '-Om'
& FWC_codegen/tests/run_fwc_q15_adapter_tests.ps1
& FWC_codegen/tests/run_fwc_speed_recovery_tests.ps1
& FWC_codegen/tests/run_fwc_speed_recovery_integration_tests.ps1
& 'D:/APP/iar9401/common/bin/iarbuild.exe' 'IAR/cm4_mc/MMEk_Demo_CM4_FOC.ewp' -make 'Multi Motor Evalkit V1.0' -log warnings
```

## 基线、固件与标定快照

20 kHz 局部源码回退基线：`12e672294bbe06539bc64a838cc271bed014256b`
（`chore(foc): snapshot 20 kHz control baseline before decimation`）。
该提交仅保存相关源码，未纳入原有无关脏文件；本机构建可能包含原有平台改动。
回退时使用归档基线镜像，或在单独工作区复核所有构建依赖，不要覆盖现有工作区。
`FOC_CONTROL_PERIOD_US=50` 是回归测试配置，不等价于原基线完整固件。

| 文件 | SHA-256 |
| --- | --- |
| `Build/CM4_FOC/MMEk_Demo_CM4_FOC.hex` | `FB3A61037C292FAAC21B094D2215DDCD240A0303BD2036CF99098C7FFC288342` |
| `Build/CM4_FOC/MMEk_Demo_CM4_FOC.elf` | `AB2901601C7D626595AB5ADBCA5FA19FB846F1BF9A35FC4A25C1C0043E25B7FD` |
| `Build/Control10k_Baseline20k/MMEk_Demo_CM4_FOC.hex` | `1928A614009D6D8F5CD1D9B923F9FF7A49EF2CD865E6A70CE60F719944376E0B` |
| `Build/Control10k_Baseline20k/MMEk_Demo_CM0PLUS.hex` | `9D94C21A3709EDA1D1AB44CFF47E821030CAEF82ECC65501D30FA1D7571FF78D` |

CM0+ 未重新构建或修改 PWM 配置；归档的是本机现有 CM0+ HEX（文件时间 2026-09-10 09:56）。
未读到目标板运行版本，语义版本及与实际板上 CM0+ 的匹配仍需人工填写，不能由文件时间证明。
二进制、日志留在 Build 下，不提交生成缓存。

`control10k_acceptance/calibration_defaults.csv` 保存相关源文件的编译默认标定表达式，
`artifact_manifest.json` 保存镜像、配置、测试证据哈希。此 CSV **不是在线标定读回**；
台架必须另存实际 Cal/Applied、当前 PI 参数、命令源、启动路线及电机参数。
在线校准可覆盖 PI 系数，因此台架需确认 Ki·Ts=80、Kaw·Ts=162，不能只检查头文件。
