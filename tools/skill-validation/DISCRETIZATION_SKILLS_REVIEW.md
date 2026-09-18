# 两个离散化技能的创建与验证记录

创建日期：2026-09-10。范围为通用 MATLAB 技能与独立脚本；未加载、修改或验证既有 SOGI/FOC 模型，也未改 A2L。

## 技能与入口

| 技能 | 默认推导主线 | 交付方程 |
| --- | --- | --- |
| [matlab-discretize-transfer-function](../skills/matlab-discretize-transfer-function/SKILL.md) | 符号 G(s) → 方法变换 → Gd(z) → q=z^-1 系数 | I/O 历史递推；参数化 `[b,a]` 函数 |
| [matlab-discretize-state-space](../skills/matlab-discretize-state-space/SKILL.md) | 连续 A/B/C/D → 离散 Ad/Bd/Cd/Dd → 状态/初态映射 | 可执行状态递推；参数化矩阵系数函数 |

两者支持 FE、BE、Tustin、预畸变 Tustin 和 ZOH。各自包含完整 helper，互不依赖安装；状态空间默认不强制展开 TF 或通道多项式。ZOH 使用增广矩阵指数，并检查其转移 ODE 和初值恒等式；符号无法证明与确实不等分别报告。

技能安装位置为 `C:/Users/ASUS/.codex/skills/` 下的上述两个目录。创建期间的合并技能只是临时工作目录，已移除，没有安装第三个合并技能。

## 现有技能与用户示例

检查了本机 `matlab-use-symbolic-math`、`matlab-design-digital-filter`、`matlab-identify-linear-system` 等技能。已有符号运算、滤波器设计及辨识能力，没有完整覆盖本次所需的推导、方法选择、差分实现与验证流程。

只读参考了用户指定 `D:/桌面/sogi2` 中的 MLX/ASV 推导及 `Orthogonal_Generator.m`。采用“源方程 → 符号变换 → 化简/公共量 → 系数 → 数值/伯德 → 差分回放”的通用流程；补充通道变体身份、同一参数快照、Hz/rad/s、公共量回代、反馈符号、显式状态和复位规则。示例未执行、未复制为技能默认算法，也不是运行时依赖。

## 实际验证

环境：MATLAB R2026a Update 5，Control System Toolbox 26.1、Symbolic Math Toolbox 26.1。

两个 SKILL.md 均通过 skill-creator 的 `quick_validate.py`。独立审阅覆盖方法/状态语义、通用性、两个入口的分工及参考文件一致性。

完整 MATLAB 自测以独立 `-batch` 会话运行，退出码 0。MCP 最后一次调用超时，未用它声明通过；数值证据以 [final 目录](matlab-discretize-verify/final) 和 [批处理输出](matlab-discretize-verify/final_stdout.log) 为准。

12 类断言用例通过：

1. 一阶参数化 TF 的 FE。
2. 一阶参数化 TF 的 BE。
3. 一阶参数化 TF 的 Tustin。
4. 一阶参数化 TF 的预畸变 Tustin，以及预畸变频点匹配。
5. 一阶参数化 TF 的 ZOH。
6. 含耦合和直通项的 MIMO。
7. 不生成 TF 的状态矩阵默认入口。
8. 奇异 A 的 ZOH、单位圆边界识别和跳过默认长回放。
9. 分子缺项和非零直通 TF。
10. 零与常量 TF。
11. 不稳定 FE 候选的拒绝门禁。
12. 非法参数、未绑定符号、非真有理与延时输入的拒绝。

前五类还覆盖生成的矩阵/IO 参数函数实际调用、换参数快照、首拍物理初态映射、复位、重复执行。测试使用独立局部随机流，不更改用户全局随机状态。

独立频域/时域一致性容差为 `1e-9 + 1e-9*abs(reference)`，已记录的最大一致性绝对误差为 `2.8127798314928402e-13`。详见 [逐工况检查汇总](discretization_check_summary.csv) 和各目录 checks/time_error/frequency_response/poles CSV。

一阶基准为 K=2、tau=0.02 s、Ts=0.001 s，预畸变频率为 2*pi*5 rad/s。这些值只用于工具回归，不是技能默认配置。

自测原汇总的数组被 MATLAB 写成单行多列，实际含 12 个 PASS。已保留 `.csv.original` 和原 stdout，并修正脚本为列向量表。`ObservedMaxError` 包含连续近似差异，**不是实现等价误差**；不能把约 0.02～0.06 的该列当成实现误差或验收阈值。

## 验收边界

- 自测证明辅助工具覆盖范围，不证明任何用户算法达到其目标精度。
- 完整自测没有设置连续近似质量门限，相应项目为 NOT_RUN；不能将数值实现一致性 PASS 扩大为全频段精度合格。
- 单位圆边界不自动叫临界稳定，Jordan 块可能增长；半单性和有限时域要求需具体判断。
- single 全回放、非零物理初态完整回放、双实例隔离、调度、Simulink、代码生成及硬件测试均未执行。
- 已自动生成各通道伯德幅相 PNG；本机外部图像查看器无法解码 MATLAB 导出的 PNG，因此图面人工验收为 NOT_RUN。原始复响应 CSV 已独立数值核对，图像生成不替代这些检查。
- 大规模符号 ZOH 的自动超时退路尚未提供。技能要求必要时保留严格矩阵指数表达式并在外部数值计算，不强制展开庞大公式。

最后的参数快照记录另用 TF/Tustin 与 SS/ZOH 两个微型入口检查，两者的总体验收、参数记录和证明说明断言均通过。该程序随后因 `.original` 扩展名未显式指定 FileType 而在读取历史汇总时退出 1，日志保留在 [smoke 输出](smoke_stdout.log) 与 [错误输出](smoke_stderr.log)。这不被记为整体 smoke 通过。

汇总读取改为明确 CSV 文本格式后单独复核，退出码 0，确认 12 行且全部 PASS，见 [汇总格式复核](summary_stdout.log) 和 [MATLAB 复核后的汇总](matlab-discretize-verify/smoke/verified_selftest_summary.csv)。安装后的两个技能再次通过格式检查，全部文件与对应仓库源文件哈希一致。
