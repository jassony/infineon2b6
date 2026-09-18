# APSFSM 电机台架测试用例：标定与观测变量定义

**状态：** 台架准备阶段  
**更新时间：** 2026-08-27  
**对象：** CM4 FOC 固件中的 APSFSM 一阶机械谐波转矩补偿  
**执行周期：** 500 us（2 kHz 速度环）

## 1. 目的和边界

本文定义 APSFSM 台架测试所使用的标定量和观测量。变量名称、类型、默认值、校验规则和运行时语义以
[`apsfsm_torque_compensation_adapter.h`](../../../../APSFSM_codegen/apsfsm_torque_compensation_adapter.h)
和
[`apsfsm_torque_compensation_adapter.c`](../../../../APSFSM_codegen/apsfsm_torque_compensation_adapter.c)
为准。

本阶段只使用固件中已经存在的 11 个 `Cal_APSFSM_*` 标定量和 12 个
`Meas_APSFSM_*` 观测量，不新增固件接口，不修改主 FOC 模型，也不修改 A2L。

## 2. 公共缩放和发布规则

- 所有变量均为 `NO_OPT volatile`，并放置在 `.xcp_cal_m4` 段中。
- APSFSM 每 500 us 读取一次标定量并发布一次观测量。
- Q15 与 PU 的换算为 `PU = Q15 / 32768`。因此 `-32768` 对应 `-1 PU`，
  `32767` 对应约 `0.999969 PU`。
- 机械速度基值为 `10000 rpm`；速度 PU 与 rpm 的换算为
  `speed_PU = speed_rpm / 10000`。
- 默认补偿限值 `+1638/-1638 Q15` 分别约为
  `+0.0499878/-0.0499878 PU`，台架记录中可按 `+/-0.05 PU` 表示。
- 当前 A2L 不自动包含这些变量。台架测试前必须确认测量标定工具使用的
  A2L 或人工符号映射与当前固件 ELF/map 地址一致。

## 3. 标定变量

除选择器关闭和显式复位外，任一参数校验失败都会使算法清空自适应状态、
保持原始 dq 指令不变，并发布 `PARAMETER_INVALID(4)`。

| 标定变量 | C 类型 | 单位/缩放 | 默认值 | 有效值及约束 | 台架用途和异常行为 |
| --- | --- | --- | ---: | --- | --- |
| `Cal_APSFSM_Sel_u8` | `uint8_t` | 枚举 | `0` | `0=Off`、`1=Shadow`、`2=Apply` | 上电必须为 `0`。大于 `2` 时按参数无效处理。台架首次运行先使用 Shadow，确认状态和观测量正常后才允许切换 Apply。 |
| `Cal_APSFSM_Rst_u8` | `uint8_t` | 电平复位 | `0` | `0=正常`，任意非零值均请求复位；台架统一使用 `1` | 置 `1` 时每个速度环采样都复位并发布 `IDLE(0)`；观测确认后必须恢复为 `0`，否则算法无法进入学习。该值非零不是参数错误。 |
| `Cal_APSFSM_KHat_f32` | `float` | 算法增益 | `0.25` | 有限且 `KHat > 0`；`0.5*KHat^2` 必须为有限正数 | 控制 B/C 自适应更新强度及协方差初值。零、负数、NaN、Inf 或平方溢出时发布 `PARAMETER_INVALID(4)`。 |
| `Cal_APSFSM_Rho_rad_f32` | `float` | rad | `-pi/2`，约 `-1.5707963` | 任意有限值 | 自适应正弦/余弦相位偏置；计算时 `Theta + Rho` 会包装到 `[0,2*pi)`。NaN/Inf 时参数无效。 |
| `Cal_APSFSM_Lambda_f32` | `float` | 无量纲 | `0.98` | 有限，`0 < lambda < 1`，并满足下述收敛约束 | 遗忘因子。只满足 `0 < lambda < 1` 仍可能被拒绝；违反收敛条件时发布 `PARAMETER_INVALID(4)`。 |
| `Cal_APSFSM_IqHi_Q15_s16` | `Ifx_Math_Fract16` | Q15/PU | `1638` | 大于 `0`，大于低限，且不超过系统速度 PI 的 q 轴上限 | 限制正向补偿及 B/C 系数半径。越过系统 q 轴上限或与低限关系错误时参数无效。 |
| `Cal_APSFSM_IqLo_Q15_s16` | `Ifx_Math_Fract16` | Q15/PU | `-1638` | 小于 `0`，小于高限，且不低于系统速度 PI 的 q 轴下限 | 限制负向补偿及 B/C 系数半径。越过系统 q 轴下限或与高限关系错误时参数无效。 |
| `Cal_APSFSM_SpdLo_rpm_u16` | `uint16_t` | rpm | `800` | `0 < SpdLo < SpdHi` | 参考速度和反馈速度都必须不低于该值，边界值包含在有效窗口内。低于窗口时状态清零并发布 `WAIT_SPEED(1)`。 |
| `Cal_APSFSM_SpdHi_rpm_u16` | `uint16_t` | rpm | `4000` | `SpdLo < SpdHi <= 10000`，且与 `Lambda` 联合满足收敛约束 | 参考速度和反馈速度都必须不高于该值，边界值包含在有效窗口内。高于窗口时状态清零并发布 `WAIT_SPEED(1)`。 |
| `Cal_APSFSM_Settle_ms_u16` | `uint16_t` | ms | `500` | 大于 `0` | 速度和控制条件连续满足后等待的学习准入时间；2 kHz 下计数值为 `2*Settle_ms`。等待期间不学习、不输出补偿。 |
| `Cal_APSFSM_Ramp_ms_u16` | `uint16_t` | ms | `500` | 大于 `0` | 从 Shadow 或非 Apply 状态进入 Apply 时，补偿增益由 0 线性增加至 1；2 kHz 下计数值为 `2*Ramp_ms`。当前变量集中没有渐入系数观测量。 |

### 3.1 遗忘因子收敛约束

固件使用以下完整条件校验 `Lambda` 与速度上限的组合：

```text
omegaHigh_radps = 2*pi*SpdHi_rpm/60
(2*Lambda - 1)^2 + (omegaHigh_radps*0.0005)^2 < 1
```

等价地，在 `omegaHigh_radps*0.0005 < 1` 时：

```text
(1 - sqrt(1 - (omegaHigh_radps*0.0005)^2))/2 < Lambda
Lambda < (1 + sqrt(1 - (omegaHigh_radps*0.0005)^2))/2
```

当速度上限为默认 `4000 rpm` 时，允许区间约为
`0.0111 < Lambda < 0.9889`，默认 `0.98` 有效。离线源数据回放使用的
`Lambda=0.999` 不满足该台架默认速度窗口的校验条件，不得直接作为台架标定值。

### 3.2 标定操作约束

1. 上电及修改算法增益、限值、速度窗口或时间参数时，将选择器保持在 Off。
2. 完成标定后先进入 Shadow，并等待 `Meas_APSFSM_Stat_u8=2`、
   `Meas_APSFSM_Valid_u8=1`。
3. 只有在 Shadow 观测稳定且未出现故障后，才允许将选择器切换为 Apply。
4. 显式复位时将 `Cal_APSFSM_Rst_u8` 置 `1`，确认状态和学习量清零后立即恢复为 `0`。
5. 台架测试不得直接采用离线源回放的高增益、高限幅参数。

## 4. 观测变量

| 观测变量 | C 类型 | 单位/缩放 | 含义和判读方法 |
| --- | --- | --- | --- |
| `Meas_APSFSM_Act_u8` | `uint8_t` | boolean | 学习采样是否有效。完成准入等待并成功执行一次算法更新时为 `1`；Off、控制条件不满足、速度窗口外、等待稳定、复位或无效状态时为 `0`。 |
| `Meas_APSFSM_OutAct_u8` | `uint8_t` | boolean | 当前采样是否处于有效 Apply 输出状态。只有 `Stat=3` 时为 `1`；Shadow 中始终为 `0`。 |
| `Meas_APSFSM_Valid_u8` | `uint8_t` | boolean | 当前算法状态和输出是否有限且有效。有效 Shadow/Apply 为 `1`，其他状态为 `0`。它不能替代 `Stat` 和 `OutAct` 的模式判定。 |
| `Meas_APSFSM_Stat_u8` | `uint8_t` | 状态枚举 | APSFSM 当前状态，编码见第 5 节。它是台架步骤切换和结果判定的首要状态量。 |
| `Meas_APSFSM_Clipped_u8` | `uint8_t` | boolean | 原始补偿碰到 APSFSM Iq 限值，或 Apply 后总 q/电流圆限幅时为 `1`。现有变量不能区分“补偿自身限幅”和“总电流外部限幅”。 |
| `Meas_APSFSM_IqRaw_PU_f32` | `float` | PU | `BHat*sin(Theta)+CHat*cos(Theta)` 的原始周期补偿。仅在有效学习采样中发布；Inactive/Wait/Reset/Invalid 时清零。该量不是最终施加到电流环的 Iq。 |
| `Meas_APSFSM_IqOut_Q15_s16` | `Ifx_Math_Fract16` | Q15/PU | APSFSM 处理后的最终 q 轴指令。Off、Wait、Shadow、Reset 和 Invalid 时等于输入的基础 q 指令；Apply 时包含渐入、补偿限幅、系统 q 限幅及电流圆限幅后的结果。 |
| `Meas_APSFSM_BHat_PU_f32` | `float` | PU | 正弦系数的更新后状态。有效学习时发布；状态复位、控制失效、速度越界或参数/数值无效后清零。 |
| `Meas_APSFSM_CHat_PU_f32` | `float` | PU | 余弦系数的更新后状态。有效学习时发布；复位规则与 `BHat` 相同。`hypot(BHat,CHat)` 不得超过由 Iq 上下限形成的有效系数半径。 |
| `Meas_APSFSM_Theta_rad_f32` | `float` | rad | 当前有效采样完成积分后的机械角，范围为 `[0,2*pi)`。总 q/电流圆限幅时仍继续积分；Inactive/Reset/Invalid 时清零。 |
| `Meas_APSFSM_SpdErr_PU_f32` | `float` | PU | `SpeedReference_PU-SpeedFeedback_PU`。只在有效学习采样中发布；等待或无效状态时清零。 |
| `Meas_APSFSM_Cov_f32` | `float` | 算法状态 | 自适应协方差的更新后状态，正常学习时必须为有限正数。复位/无效时观测值清零；准入等待阶段可能发布 `0.5*KHat^2` 的初始化值。 |

## 5. 状态码

| 数值 | 状态 | 台架含义 | 主要观测期望 |
| ---: | --- | --- | --- |
| `0` | `IDLE` | 选择器 Off、显式复位，或 FOC 未满足运行/闭环/非直接接口条件 | `Act=0`、`OutAct=0`、`Valid=0`；Iq 指令旁路，学习状态清零。 |
| `1` | `WAIT_SPEED` | 参考/反馈速度超出有效窗口，或连续准入时间尚未达到 `Settle_ms` | `Act=0`、`OutAct=0`、`Valid=0`、`IqRaw=0`，Iq 指令旁路。速度越界会清空状态；稳定等待阶段协方差可为初始化值。 |
| `2` | `SHADOW_VALID` | Shadow 学习有效 | `Act=1`、`OutAct=0`、`Valid=1`；B/C/Theta/Cov 更新，但 `IqOut` 必须保持基础 q 指令。 |
| `3` | `APPLY_VALID` | Apply 学习和补偿输出有效 | `Act=1`、`OutAct=1`、`Valid=1`；`IqOut` 为渐入和所有限幅后的最终 q 指令。 |
| `4` | `PARAMETER_INVALID` | 标定值或组合约束无效 | 补偿关闭、状态清零、Iq 指令旁路；修正参数后必须重新经历准入等待。 |
| `5` | `NUMERICAL_INVALID` | 输入、内部状态或计算结果出现非有限值等数值错误 | 补偿关闭、状态清零、Iq 指令旁路；不得继续切换 Apply。 |

## 6. 模式观测矩阵

| 场景 | `Act` | `OutAct` | `Valid` | `Stat` | `IqRaw` | `IqOut` | B/C/Theta/Cov |
| --- | ---: | ---: | ---: | --- | --- | --- | --- |
| Off 或显式复位 | 0 | 0 | 0 | 0 | 0 | 基础 q 指令 | 清零 |
| 速度窗口外 | 0 | 0 | 0 | 1 | 0 | 基础 q 指令 | 清零 |
| 准入等待阶段 | 0 | 0 | 0 | 1 | 0 | 基础 q 指令 | B/C/Theta 为 0；Cov 可为初始化值 |
| Shadow 有效 | 1 | 0 | 1 | 2 | 周期补偿原始值 | 基础 q 指令 | 每个有效采样更新 |
| Apply 有效 | 1 | 1 | 1 | 3 | 周期补偿原始值 | 补偿及限幅后的 q 指令 | 每个有效采样更新；总 q/电流圆限幅时 B/C/Cov 保持、Theta 继续积分 |
| 参数无效 | 0 | 0 | 0 | 4 | 0 | 基础 q 指令 | 清零 |
| 数值无效 | 0 | 0 | 0 | 5 | 0 | 基础 q 指令 | 清零 |

## 7. 当前观测边界

现有变量集中没有以下独立观测量：

- Apply 渐入系数；
- 实际施加的增量补偿 `IqApplied`；
- APSFSM 输入端的补偿前 d/q 指令；
- APSFSM 输入端的速度参考和速度反馈。

因此本用例不得用不存在的 `Meas_APSFSM_*` 名称。后续台架步骤如果需要计算
Apply 增量、验证渐入曲线或区分具体限幅来源，应从平台已有 FOC/XCP 通道同步采集
相应输入信号，或者另行评审新增观测接口。

## 8. 文档一致性检查

- 标定变量数量必须为 11，观测变量数量必须为 12。
- 变量名、C 类型和默认值必须与适配器头文件、源文件一致。
- 选择器、状态码、Q15 缩放、2 kHz 周期及收敛条件必须与当前固件实现一致。
- 本文不得形成或修改 A2L 地址，也不得宣称未发布的内部量可以直接观测。
- 台架数据记录必须包含固件版本/哈希、A2L 或符号映射版本、标定快照和原始采集文件。
