# SOGI 试例归档

日期：2026-09-10；MATLAB R2026a Update 5。范围为独立滤波器建模和仿真；交付两个 SLX、一份 SLDD、系数/参考/验证/布局/导出脚本及规则流程。

| 验证 | 结果 | 证据 |
|---|---|---|
| 两模型 model_read / model_check | healthy，无未连接错误 | 模型结构读取及 MATLAB MCP 检查 |
| 结构、几何、字典与编译接口 | 188 PASS，0 FAIL，0 NOT_RUN | [standard_checks.csv](standard_checks.csv) |
| 逐拍回放、重复运行、采样扫描 | 79 PASS，0 FAIL，1 不稳定点不运行 | [implementation.csv](implementation.csv) |
| 频率响应 | 40 PASS | [frequency.csv](frequency.csv) |
| 非法参数 | 10 PASS | [parameters.csv](parameters.csv) |
| 直流响应 | 8 PASS | [dc.csv](dc.csv) |
| 连续参考精度及 ZOH 采样匹配 | PASS | [continuous_reference.md](continuous_reference.md) |
| 七个作用域图面 | PASS，独立记录 | [layout_review.md](layout_review.md) |

基准四种方法的极点均在单位圆内。FE 在 Ts=5 ms 的极点半径约 1.1162，因此标记 NOT_RUN_UNSTABLE，不靠限幅抑制发散，也不作为通过的仿真。50 Hz 两路幅值误差均低于 2%，相位误差均低于 1°，具体测量值见 [report.md](report.md)。

精确文件 SHA-256 见 [manifest.csv](manifest.csv)。清单包含本次本地原始数据：`validation_raw.mat` 与 `continuous_reference_raw.mat`；这两个可复现的大文件及仿真缓存不进入 Git。模型、字典已检查为可读取的标准 ZIP 容器并经 MATLAB 重新打开。

本次只新增 SOGI_discrete_test 文件并补充仓库 AGENTS.md 的新模型规则入口。开始工作的 Git 基线为 `8ca96960cf6b293c0c74dc0ab393e5bc318920bd`。提交前通过 Git MCP 对比确认其余既有未提交差异保持原样；旧模型、FOC、PLL/FLL 和 A2L 未在本次任务中修改。未做代码生成、固件构建或台架测试。

后续规则改进必须同时更新 MODELING_RULES.md、README.md 工作流程和相应检查/图面证据；新建或调整模型前先阅读 AGENTS.md 指定入口。
