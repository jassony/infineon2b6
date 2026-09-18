---
name: matlab-discretize-transfer-function
description: Derive and validate discrete input-output difference equations from continuous transfer functions in MATLAB. Use for symbolic TF derivation and simplification, selectable discretization methods, normalized z-inverse coefficients, Bode comparison, and independent sample-by-sample verification.
---

# MATLAB 传递函数离散化

从资料中的连续传递函数得到参数化 `Gd(z)`、明确索引的 I/O 差分方程及可复查的验证结果。适用于一般控制器、滤波器和被控对象，不固定 SOGI、阶数或参数名称。

主输入若是 A/B/C/D 状态方程且目标是状态递推，改用 `matlab-discretize-state-space`。本技能可为 ZOH 或交叉核对派生辅助 SS，但不把派生 SS 冒称为另一份独立来源。

## 1. 源方程和合同

- 接收 `G(s)`，或从资料中的联立方程/串并联/反馈关系明确符号后推导整体 TF。保留来源版本、通道/单位、初态和假设；同名通道须核对完整分子分母、DC、高频、直通及相位方向。
- 定义物理参数、范围、Ts、保持方式、频带、容差、实现类型与复位语义。已有上下文能确定的值沿用；需要的缺项再问，不强制频率/增益等算法专用字段。
- 读取 [资料到差分方程流程](references/source-workflow.md)。用户的推导示例只能用于提炼通用操作，不能作为每次运行的文件依赖或默认算法。
- 检查 MATLAB、Symbolic Math Toolbox 和 Control System Toolbox；不需要加载 Simulink。缺少 Symbolic 时仅能完成数值部分，解析推导标 NOT_RUN。

## 2. 选择并推导离散化

读 [方法与公式](references/methods.md)，按用户选择或目标频段/保持/成本选择一个方法或比较列表：

| 方法 | 符号路径 |
| --- | --- |
| FE | `s=(z-1)/Ts`，额外检查稳定性 |
| BE | `s=(z-1)/(Ts*z)`，保留当前输入项 |
| Tustin | `s=(2/Ts)*(z-1)/(z+1)` |
| 预畸变 Tustin | `s=wp/tan(wp*Ts/2)*(z-1)/(z+1)`，wp 单位 rad/s |
| ZOH | 通过状态实现的增广矩阵指数，再构造 Gd；没有通用有理 s 代换 |

FE/BE 不是 `c2d` 的方法字符串。其它库方法在任务需要时按已安装版本的官方文档扩展，不沿用错误保持和初态约定。

1. 以符号形式保留所有物理参数和 Ts。对完整传函变换；不要先只给数字再声称参数化。
2. 用 `collect`、`numden`、`simplifyFraction`、有限步 `simplify` 整理，不用忽略解析约束选项掩盖条件。保存未经约消的来源与奇异条件。
3. 公共子表达式保留到物理参数的依赖表，回代证明等价；数值化前检查没有未绑定符号。派生公共量不能成为独立调参项。
4. 令 `q=z^-1`，消去负幂，使用 `coeffs(...,q,'All')` 补齐零项并转换为 q 升幂，统一 a0=1。回代 a/b 重建 Gd，核对全部系数位置和符号。不能用 `sym2poly` 提取未赋值的参数系数。
5. 明确输出 `y[n]=-sum(a_i*y[n-i])+sum(b_j*u[n-j])`，b0 为当前输入项。分母系数 a 与加法反馈乘数 -a 区分命名；历史量初始化和移位次序写清楚。
6. 生成参数顺序明确的外部系数函数；参数快照先检查、生成并冻结。逐拍实现显式传入实例历史/复位和系数，不查 base workspace/SLDD/共享 persistent 状态。Ts 参数不改变后续模型实际调度。

## 3. 验证

读 [验证合同](references/verification.md)，在运行前明确容差、频点、零幅值阈值和测试向量。

- 符号 Gd、独立方法变换及重建 a/b 核对；抽样一致不能替代解析证明。复杂解析证明未完成要如实标记。
- 连续 TF、符号结果数值化、c2d 参考和时间回放使用同一个参数快照与 Ts。FE/BE 另用独立频率映射核对，ZOH/Tustin 对照 c2d。
- 输出连续/离散伯德幅相叠图以及复响应、幅值/相位误差 CSV。频率统一 Hz，调用频响函数时转 rad/s；全部点严格低于 Nyquist。零幅值处相位未定义，不以裁剪值判定。图题与正弦激励单位从实际参数生成。
- 显式 I/O 差分递推与独立状态/库参考逐拍比较，覆盖非零首拍、合适激励、固定种子、复位和重复运行；不平移时间轴。高阶/病态多项式改用另行验证的 SOS/SS，不靠放宽容差隐藏问题。
- 分开判定实现等价与连续近似质量；伯德图重叠不是定量通过。不稳定候选记录并跳过长回放；边界稳定系统用单独有限时域目标。非零物理初值、single、双实例按实际需求补测。

## 4. 交付及脚本

交付来源/参数合同、方法理由、G(s)→Gd(z)→q 系数推导、适用条件与公共量依赖、符号和数值 a/b、可逐拍执行的差分方程、参数函数、伯德图及频域/时域/极点数据、PASS/FAIL/NOT_RUN 报告。只有实际通过目标门禁的方法和配置才交给后续建模；不自动编辑 SLX 或 A2L。

读取 [脚本接口](references/scripts.md)。首选 `scripts/dv_from_tf.m`；它调用本技能自带的 `dv_derive`、`dv_realize`、`dv_export`、`dv_verify` 和 `dv_step`。脚本目录自包含，不要求安装另一个技能。`dv_selftest` 只验证辅助工具，不代表用户算法或模型已验收。

helper 支持无延时的 SISO proper rational TF；MIMO TF 需逐通道且保留通道求和/共享状态关系，或转向状态空间流程。纯延时、非真有理、分数阶、非线性/时变问题先明确专门处理方案；不静默做 Padé、降阶或线性化。
