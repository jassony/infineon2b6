# FE 真实物理参数输入与功能命名记录

日期：2026-09-10。范围：独立 SOGI 试例的 FE 接口、基础模块实现、外围连接和规范。本轮按用户选择展开 f0/k/Ts 三端口，允许直接使用物理参数作 FE 差分；未运行脚本快速仿真或模型数值仿真。历史数值报告不适用于本轮新结构。

## 实际接口与方程

模型仍保存为 `sogi_discrete_comparison.slx`，其中 FE 子系统已命名为 `SOGI_DualOutput_Filter_FE`。文件名及其它方法的功能命名迁移尚未进行。

| 输入顺序 | 正式名称 | 类型 / 单位 | 作用 |
| --- | --- | --- | --- |
| 1 | Input_PU | single / PU | 滤波输入 |
| 2 | Reset_States | uint8 | 非零时复位优先 |
| 3 | Cal_SOGI_F0_Hz_f32 | single / Hz | 计算 w0=2*pi*f0 |
| 4 | Cal_SOGI_K_f32 | single / 1 | 对输入与同相状态之差加权 |
| 5 | Cal_SOGI_Ts_s_f32 | single / s | 计算差分步长 Ts*w0 |

两输出为 `InPhase_D_PU`、`Quadrature_Q_PU`，均为 single 标量。外层 XCP 输出名称保持不变。

来源：任务给定连续状态方程 `dD/dt=k*w0*(u-D)-w0*Q; dQ/dt=w0*D`，零初始状态。本轮未新增方程来源或声称独立来源交叉验证。

```text
w0 = 2*pi*f0
step = Ts*w0
Dnext = D + step*(k*(u-D)-Q)
Qnext = Q + step*D
y = [D; Q]
```

`D`、`Q` 为当前物理状态，Q 更新使用当前 D。输出读取经过复位选择的当前状态；复位时有效输入、状态与输出均为零，保存的下一状态也为零（有效有限参数前提）。图中仅使用常数、Product、Sum、Switch、Unit Delay 和局部路由；FE 已移除原全局字典系数 Gain。

比较模型增加相同三参数的根输入；验证模型在核心外以字典 Constant 提供参数。两 Unit Delay 继承实例采样周期。Ts 数值参与运算，但不控制调度；参数应在运行前给定并冻结。实际端口输入的有限性、范围、稳定性、冻结及 Ts 一致性保护尚未实施/验收，原字典检查不能替代它。

`run_sogi_validation`、`run_sogi_parameter_validation` 已追加三个外部输入的数据集构造，方法查找、参数面板与布局导出也已适配 FE 新名称；这些数值脚本本轮没有执行。

## 已执行检查及实际范围

| 检查 | 状态 | 证据与限制 |
| --- | --- | --- |
| model_read | PASS | 已读取比较模型根、FE 内部及验证模型根，确认三个物理端口均参与运算 |
| model_check | PASS | 两模型均为 healthy；无未连接端口、悬空线或 Stateflow lint 问题 |
| 编译 | PASS | 两模型编译成功；没有执行 sim 或数值回放 |
| FE 编译类型 | PASS | 输入/三参数/两状态/两输出 single；reset 为 uint8 |
| FE 编译周期 | PASS | 上述端口和状态均为 `[4.9999998736893758e-05, 0]`，对应 single 量化的 50 μs |
| 标准自动检查 | 范围内 PASS | standard_checks.csv：213 PASS、0 FAIL、1 NOT_RUN；整体状态仍为 NOT_RUN |
| 线间几何检查 | PASS | line_separation_checks.csv：三个作用域共 93 条线、1438 对检查，交叉/重叠/斜线均为 0；不含其它方法内部 |
| 人工图面复核 | PASS | 覆盖比较模型根、FE 内部、验证模型根；不把其它旧方法内部命名视为已迁移 |
| 实际参数输入保护 | NOT_RUN | 尚未实施/验收；CSV 唯一 NOT_RUN 项 |
| 新脚本门禁与模型数值一致性 | NOT_RUN | 用户要求暂不验证结果；不能用历史 PASS 放行新接口 |
| 双实例隔离、独立复位及不同参数 | NOT_RUN | 后续复用验收 |

FE 方法块采用 900×600，验证模型的 Model Reference 采用 900×600，字体 Arial 14，编辑缩放 100%，关闭内容预览。普通信号从左到右，参数区域在上、状态更新在下。已消除两处参数乘法输入交叉，并将两处 Zero/Reset 线路拆成独立通道，避免 T 形假连接观感。开关图标内部的斜线属于符号，不是外部信号走线。

局部清晰预览见 [fe_interface.png](fe_interface.png)；完整图面见 [fe_layout.png](fe_layout.png)、[model_layout.png](model_layout.png)、[validation_layout.png](validation_layout.png)。自动块避障检查不能代替线间交叉/重合检查或人工图面审查。

## 规范与后续边界

规则、流程及接口合同已同步至仓库 `AGENTS.md` 的新模型部分、`MODELING_RULES.md` 与 README。参数化以真实模型输入为准；根据方法可直接展开物理参数差分，离线脚本仍负责参数化推导和参考。交换乘法/加法输入以改善布局时必须保持数学等价，减法、除法及矩阵顺序不可随意交换。

BE/Tustin/ZOH 仍使用旧字典系数方案，未声明其参数端口已迁移。51 字段总线草稿本轮不采用。FOC 主路径、其它旧模型与 A2L 不属于本轮修改范围；工作区其它已有修改保留。本轮没有 Git 提交。

## 信号定义规则补充

随后按用户要求补充信号定义合同、SOGI 信号示例、Simulink.Signal 对象绑定要求及相应流程/记录项。此次补充属于文档工作，没有新增或修改信号对象、没有据此重新认证已有模型。新增信号定义与对象绑定门禁仍需在后续实施时逐项执行，状态为 NOT_RUN；上述结构和图面证据不能替代它。
