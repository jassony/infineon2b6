# 子状态机闭环拟合

在 MATLAB 中运行：

```matlab
result = powerfit_run_closedloop_rated_350v_20260907_refit;
```

数据资格唯一为：

```matlab
Meas_FocState == 4 & Meas_FocSubState == 2
```

这分别表示 FOC 主状态 `run` 和子状态 `closedLoop`。不会使用稳态时长、转速/电流/功率导数、直流功率方向或低功率门限。低通在样本选中后运行，且不改变 `FitEligible`。

新数据库写入 `local_data/rated_350V_20260907_closedloop_database.mat`，不会覆盖之前的稳态数据库。后续批量测试调用 `powerfit_closedloop_refit_batch(inputFolder, cfg)`，并为每个测试组配置不同的数据库和拟合结果文件。
