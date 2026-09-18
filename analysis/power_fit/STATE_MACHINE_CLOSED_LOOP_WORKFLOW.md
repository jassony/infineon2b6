# 状态机子状态闭环拟合

执行：

```matlab
result = powerfit_run_state_machine_closedloop_rated_350v_20260907;
```

唯一的样本资格是：

```matlab
Meas_FocState == 4 & Meas_FocSubState == 2
```

即主状态 `run` 且子状态 `closedLoop`。不加入稳态、时长、导数、功率方向或功率阈值判断。低通在样本选择后对拟合输入执行，不会修改资格标志。

对于后续数据，设置独立的 `cfg.DatabaseFile`、`cfg.FitResultFile` 和 `FitGroupId` 后调用 `powerfit_state_machine_closedloop_batch(inputFolder, cfg)`。
