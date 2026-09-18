# 输入来源快照

采集日期：2026-09-14。仓库基线 HEAD：`38cd748d336289828bfe375507d5d94ff1d18c1f`。工作树已有未提交改动，本试验读取的是当前文件，并非声称它们与 HEAD 相同。以下 SHA-256 为磁盘文件字节校验；本机文件保护层可能使 MATLAB 读取文本与磁盘编码不同。

| 相对仓库路径 | SHA-256 |
|---|---|
| KRE_codegen/kre_external_observer_diagnostic_step.m | 84AF9CD559AD93759883CDD52D5DEDC0F64C3035544255D751ED38340E5E3E7C |
| KRE_codegen/kre_external_observer_adapter.c | 9CD7DFEEE36DC4E9339DE8E3418F86C045D8BF89C4781DAE9E9089D1DDA8FBDB |
| Example/CM4_FOC/main_cm4.c | C4686C35371E0EBCC0583DCDB3BF8BCCA7B6817D2551F8A0EAF8743D06636E3B |
| ConfigWizard/Ifx_MS_FocSolutionF16_Cfg.h | 1DBF91B4C91D14BD7E1297E8727D26887E9BA518F3A5409637AD30E68691F188 |

默认参数：R=0.5 Ohm，Ld=1.3 mH，Lq=1.38 mH，psi_f=0.046 Wb；电压/电流/机械转速 PU 基准=1000 V/50 A/10000 rpm；极对数=4。KRE alpha=1256.637061 s^-1，a=125.663704 s^-1，gamma=1，epsilon=1e-5 Wb，速度低通=150 Hz。PLL带宽=100 Hz，阻尼=0.70710678。快照不是标定推荐值。

复现实验时优先读取 `results/preflight.mat` 中 cfg，或显式传递同一数值。`parameters.csv` 同时列出 PLL 参数。源实现依赖仍从当前仓库读取，因此若源码已变更，必须重新做一致性门禁；旧结果不自动认证新源码。
