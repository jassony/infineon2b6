# 状态机闭环功率拟合流程

运行 `powerfit_run_closedloop_rated_350v_20260907` 可重新导入当前 9 份 350 V 日志、拟合并在 MATLAB 窗口打开汇总图和每份 MF4 的图。

样本资格唯一为：

```matlab
Meas_FocState == 4 & Meas_FocSubState == 2
```

其中 `4` 是状态机 `run`，`2` 是子状态机 `closedLoop`。不使用稳态时长、转速/电流/功率导数、正向功率或低功率门限。低通仅在样本选中后处理拟合输入，不决定样本是否进入拟合。

对于后续测试，配置输入目录、拟合组和独立数据库路径后调用：

```matlab
cfg = powerfit_default_config();
cfg.Profile.FitGroupId = "new_campaign_closedloop";
cfg.FitGroupId = cfg.Profile.FitGroupId;
cfg.DatabaseFile = "D:\\A_PRJ\\infineon3in1\\code_xcp\\analysis\\power_fit\\local_data\\new_campaign_closedloop_database.mat";
cfg.FitResultFile = "D:\\A_PRJ\\infineon3in1\\code_xcp\\analysis\\power_fit\\local_data\\new_campaign_closedloop_fit_result.mat";
cfg.CreatePerSourceFigures = true;
result = powerfit_closedloop_run_batch("D:\\path\\to\\mf4", cfg);
```

新的闭环数据库与之前的稳态数据库分开保存，避免改变已有结果。
