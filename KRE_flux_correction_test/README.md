# KRE 磁链校正独立回放

本目录实现用户批准的三个关键问题验证方案。只做解析与 MATLAB，禁止把这里的结果直接作为 Simulink、FOC、硬件通过证据。

## 运行

在 MATLAB 中进入本目录，依次执行：

```matlab
run_kre_flux_correction_study
run_kre_flux_correction_supplement
```

第一项运行源代码一致性门禁及主回放；第二项执行独立连续参考、边界/实例隔离检查和汇总绘图。全量运行需要数十分钟，低频10秒波形样本数较多。结果写入本目录 `results/`，再次运行会覆盖本目录同名试验结果，不影响仓库其他文件。完整数据默认保留，包括失败用例，不使用已有 NFO 测试结果。

MATLAB单独单测：`runtests('tests')`。快速专项：`cfg=kre_study_parameters; a=kre_study_analytics(cfg);`。单组回放：`[T,trace]=kre_study_replay(cfg,2,"steady",50e-6,"single");`。

完整数据生成后，可用`kre_study_verify`复核单元测试、Code Analyzer、CSV完整行数及参考接受状态；结果见`coverage.csv`、`unit_tests.csv`、`code_analysis.csv`。本轮验证环境是R2026a Update 5，图表使用Theme API，未认证更早版本。

`kre_study_parameters` 可从仓库读取当前代码默认值。其他核心函数显式接收参数/状态/输入，不读取 SLX/SLDD。复现旧批次时从 `results/preflight.mat` 读取冻结 cfg，传给 replay；不要刷新为后续代码的新默认值。主运行器始终采集当时的默认快照。

## 文件与证据

- `EQUATIONS.md`：方程、单位、状态/复位、时序、适用边界及评价口径。
- `REPORT.md`：本次实际结果、三个问题的结论及未执行项。
- `results/parameters.csv`、`preflight.mat`：代码默认值和实现/代数门禁。
- `steady.csv`、`dynamic.csv`、`standstill.csv`：完整主矩阵和动态指标。
- `step_sensitivity.csv`：25/100 us专项；`target_controls.csv`：恒定/真值目标理想对照；`negative_Qc.csv`：错误连接负对照，绝非合格候选。
- `target_surface.csv`、`active_flux_boundary.csv`：目标误差和变号边界。
- `algebra_euler.csv`、`manifold_convergence.csv`、`continuous_reference.csv`：代数、累计残差和连续参考收敛。
- `decision.csv`、`by_frequency.csv`、`summary.mat`：固定增益跨工况判定。
- `supplement.mat`：独立参考时序、Q/Y残差、边界测试结果。
- `trace_*.mat`：id=-2 A、iq=+2 A、名义参数、全部9算法的全采样率波形。其余工况保留全指标；用对应函数参数可重跑取得其他列。大体积 trace 不纳入 Git，但作为本地交付保留。
- `*.png`：三项专项图表及代表性低速/反转/停止/复位/静止曲线。

采用合成的电机方程一致输入，**不是实测数据回放或闭环电机仿真**。未验证电压采样误差、噪声、饱和、变参数电机和负载机械动态。真值角/速度只供输入生成及评价，真值 id 仅在明确标记的 target=2 理想对照启用。
