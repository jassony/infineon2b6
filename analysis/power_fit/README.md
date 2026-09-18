# MF4 功率补偿批处理流程

这个目录提供离线、可追加的 XCP/MF4 功率损耗标定流程。它会把每份日志的来源、SHA-256、通道组、时间对齐质量、单位比例和拟合资格保存到版本化 MAT 数据库中；不会修改 `m4_rte.c`、A2L 或在线标定值。

## 首次运行

在 MATLAB 命令窗口执行：

```matlab
addpath('D:\A_PRJ\infineon3in1\code_xcp\analysis\power_fit');

cfg = powerfit_default_config();
cfg.Profile.FitGroupId = "motorA_invA_fw20260907";
cfg.Profile.HardwareConfigId = "motorA_invA";
cfg.Profile.FirmwareId = "<firmware-hash>";
cfg.Profile.MotorId = "motorA";
cfg.Profile.InverterId = "invA";
cfg.Profile.SwitchingFrequency_Hz = 20000;

% 仅在已有台架/独立仪表证据时改为 true。
cfg.Profile.SupplyVoltageScaleConfirmed = false;
cfg.Profile.SupplyCurrentPolarityConfirmed = false;
cfg.Profile.DqCurrentMatchesFirmware = false;

result = powerfit_run_batch('D:\A_PRJ\infineon3in1\ape', cfg);
```

MATLAB 会显示两张图：测量/模型 DC 功率及残差覆盖图，以及实际用于拟合的 `Vd/Vq/Id/Iq` 图。`result.ImportReport` 给出每个文件的 `Imported`、`SkippedDuplicate` 或 `Rejected` 状态；`result.Fit.ReleaseGate` 列出候选系数不可发布的具体原因。

已提供本次 350 V、1000–8600 rpm 额定工况的快捷入口：

```matlab
result = powerfit_run_rated_350v_20260907;
```

它读取 `D:\A_PRJ\infineon3in1\power-fit\data`，使用独立的 `rated_350V_20260907` 数据库、稳态资格门槛和 1.0 s 零相位拟合低通。硬件/固件 ID 与测量确认仍需在有台架证据后补充，因此首次运行的发布门槛会保持阻断状态。

该额定工况入口还会为每份 MF4 打开一张单独的拟合图：`Pdc` 原始值、低通值和模型值叠加显示；残差图标出 `±100 W`；另附原始/低通后的 `Vd/Vq/Id/Iq`。通用批处理默认不开启逐文件图，按需设置 `cfg.CreatePerSourceFigures = true`。

## 后续加入新测试数据

1. 将新的 `.MF4` 放入测试目录，或显式传入文件列表。默认只扫描当前功率测试命名 `power-fit*.MF4` 和历史拼写 `pwoer-fit*.MF4`，避免同一目录的普通调试日志混入；可通过 `cfg.FilePatterns` 调整。
2. 使用相同硬件、固件和电机时，保持相同的 `FitGroupId` 与 `DatabaseFile`。
3. 重新运行 `powerfit_run_batch`。相同 SHA-256 的文件会自动跳过，不会重复计入拟合。
4. 硬件、固件、开关频率或电机发生变化时，使用新的 `FitGroupId`，避免混合物理条件。

例如：

```matlab
newLogs = [ ...
    "D:\A_PRJ\infineon3in1\ape\power-fit-01.MF4", ...
    "D:\A_PRJ\infineon3in1\ape\power-fit-02.MF4"];
result = powerfit_run_batch(newLogs, cfg);
```

本地数据库和拟合结果默认位于 `local_data/`，该目录被 Git 忽略；原始 MF4 始终只作为外部只读来源保留。

## 必需通道

所有通道均为精确匹配，不做模糊猜测：

| 用途 | 通道 |
| --- | --- |
| 直流电源 | `OutputVoltageRaw`、`OutputCurrentRaw` |
| DQ 电压 | `FocDemoClosedLoop.focController.voltageDQ.real`、`FocDemoClosedLoop.focController.voltageDQ.imag` |
| DQ 电流 | `Meas_FocCurrentFeedbackDQ_D_A_s16`、`Meas_FocCurrentFeedbackDQ_Q_A_s16` |
| 状态/辅助量 | `Meas_FocState`、`Meas_FocDcLinkVoltageFeedback_V_s16`、`Meas_FocSpeedFeedbackEstimated_rpm_s16` |

DQ 四个量必须位于同一 MDF 通道组。遇到 `CAN_DataFrame*` 或 `LIN_Frame*` 原始帧、缺失通道或同一名称存在多个组时，文件会被拒绝并在 Manifest 中留下原因；不会静默使用错误信号。

## 对齐、样本资格与公式

- 以 DQ 通道时间轴为主；电源、电压、速度采用受最大时间间隙限制的线性插值，状态使用 `previous` 保持。不会外推，也不会自动搜索时间延迟。
- 每个样本保存各源的最近同步距离、`ImportValid`、`TimeAligned`、`SteadyState`、`FitEligible` 和 `ExcludeReason`。默认只要求 `Meas_FocState == 4`、物理值有限、同步有效且为正向 DC 输入功率；可通过 `cfg.EnableSteadyStateGate` 改为只用稳态段。
- `mdfRead` 默认已给出 MDF conversion 后的物理量。当前 `OutputCurrentRaw` 不会再乘一次传输层 `0.01 A/bit`；两个比例均在 `cfg` 中显式记录。
- `OutputVoltageRaw` 的物理标度仍需用独立仪表确认。流程会把它与 FOC Vdc 的差异写入 Manifest，并在未确认前阻止候选发布。
- 默认在**拟合阶段**（不是导入阶段）使用 0.5 s 居中移动平均低通。它按每个源文件和连续时间段分别对直流电压/电流、`Vd/Vq`、`Id/Iq`、FOC Vdc、速度同时滤波，再重新计算 `Pdc`、`Pdq` 和全部损耗基函数；因此不会只平滑目标功率而引入新的模型偏差。原始样本和原始残差仍被保留，发布门槛使用原始 DC 功率 RMSE，不能由低通掩盖模型误差。
- 功率精度要求为 ±100 W：候选必须同时满足原始 DC 功率 `RMSE <= 100 W`，并且每个源文件所代表的稳态测试点的**原始 DC 平均残差绝对值**均 `<= 100 W`。逐样本最大残差仍会保存为诊断数据，但不会因电源/XCP 异步采样的单个尖峰而单独决定候选能否发布。
- 固件端的输出平滑是另一件事：`m4_rte.c` 已在计算 `P_total` 后使用自适应 IIR（功率差大于 800 W 时约 `1/4` 更新、大于 200 W 时约 `1/8`、其余约 `1/16`）。离线拟合低通不会改变这个在线输出滤波；若在线功率仍有明显波动，需要在独立的固件评审/台架回放任务中调整 IIR 策略和验证响应时间。

拟合严格复现固件的无截距模型：

```text
Pdq        = 1.5 * (Vd*Id + Vq*Iq)
Pdc model  = Pdq
           + K_CU       * 20*(Id^2 + Iq^2)
           + K_FE       * 50000*(speed_rpm/10000)^2
           + K_INV_COND * 50000*(Irms/50)
           + K_INV_SW   * 50000*(Irms/50)*(FocVdc/1000)
Irms       = sqrt(Id^2 + Iq^2) * 0.7071
```

`cfg.FitParameterMask` 支持在覆盖不足时仅诊断性拟合其中一部分参数；默认四项都拟合。部分拟合默认不能通过发布门槛，除非明确设置 `cfg.AllowPartialFitForCandidate = true`。

## 发布门槛

最小二乘能得到数值不代表可以写入固件。`ReleaseGate` 会检查：矩阵秩、归一化条件数、特征相关性、速度/电流/Vdc 覆盖、负损耗目标比例、非负系数、±100 W 的原始 DC 功率 RMSE 与逐测试点平均误差、供电电压一致性、同一已知硬件/固件配置，以及电源极性和 DQ 电流与固件算法输入同源的确认。

特别是要独立辨识 `K_INV_COND` 与 `K_INV_SW`，需要在相近电流下有足够的 Vdc 扫描；要辨识 `K_FE`，需要近零/低负载下的多速度点。不要让长时间单一稳态数据在样本数上压倒其他工况；必要时开启稳态门槛并按测试点分批复核。

候选通过 `ReleaseGate` 后，仍应在单独的代码评审和台架回放流程中决定是否更新固件标定。
