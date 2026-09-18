# 状态空间离散化脚本入口

从本技能 `scripts/` 目录执行，或临时只添加此目录。函数使用独立工作区，不读取 base workspace、SLDD 或已有模型；输出写入调用者明确指定的目录。

## 参数化连续矩阵 → 离散矩阵

```matlab
s = sym('plant_s');
T = sym('sample_period','positive');
m = sym('mass','positive');
c = sym('damping','positive');
k = sym('stiffness','positive');

% 质量-弹簧-阻尼系统；x=[位置(m);速度(m/s)]，u=力(N)，y=位置(m)。
A = [0 1;-k/m -c/m];
B = [0;1/m]; C = [1 0]; D = 0;
snapshot = [2 3 20 .01];  % m[kg], c[N*s/m], k[N/m], T[s]
assert(all(isfinite(snapshot)) && all(snapshot>0));

derived = dv_from_ss(A,B,C,D,s,T,'Tustin');
numeric = dv_realize(derived,[m c k T],snapshot);
outDir = fullfile(pwd,'discrete_reference','mechanical_state_tustin');
dv_export(derived,numeric,[m c k T],outDir);

test.t = (0:1000)'*numeric.Ts;
test.u = ones(size(test.t));
test.reset = false(size(test.t));
test.reset(501:505) = true;
test.frequencyHz = unique([logspace(-2,log10(40),180)'; .5]);
test.absTol = 1e-9; test.relTol = 1e-9;
test.magnitudeFloor = 1e-10;
% 若已规定近似质量阈值，可在运行前设置 test.approximationAbsTol。
report = dv_verify(numeric,test,outDir);
disp(report.checks);
```

默认不求 `Gc(s)`、`Gd(z)` 或符号通道多项式，`derived.recurrence` 为空。
`dv_verify` 仍比较矩阵/独立库或方法映射的伯德响应和逐拍输出；未请求的 I/O 多项式检查标 NOT_RUN 并排除在矩阵范围验收之外。

## 方法选择

```matlab
derived = dv_from_ss(A,B,C,D,s,T,'FE');
derived = dv_from_ss(A,B,C,D,s,T,'BE');
derived = dv_from_ss(A,B,C,D,s,T,'ZOH');
wp = sym('prewarp_rad_s','positive');
derived = dv_from_ss(A,B,C,D,s,T,'TustinPrewarp',wp);
numeric = dv_realize(derived,[m c k T wp],[2 3 20 .01 2*pi*.5]);
```

如明确需要低阶通道差分式，传最后一个参数 true：

```matlab
derived = dv_from_ss(A,B,C,D,s,T,'Tustin',[],true);
```

符号参数唯一且是单符号；s/T/wp 必须独立，`dv_z`、`dv_q` 为保留变量。矩阵维度自洽，所有数值参数必须完整绑定并为有限 double。调用者负责执行物理范围校验。

## 状态和初值

```matlab
xi = zeros(size(numeric.Ad,1),1);
[y,xiNext] = dv_step(xi,uColumn,reset, ...
    numeric.Ad,numeric.Bd,numeric.Cd,numeric.Dd);
xi = xiNext;
```

这是零辅助状态的逐拍执行；当拍先输出再更新，复位同时屏蔽直接馈通。给定物理 x0 与 u0 时：

```matlab
xi0 = numeric.initialStateMatrix*x0 + numeric.initialInputMatrix*u0;
```

非零物理初态的完整回放需另做对照；当前 `dv_verify` 固定零辅助初态。

## 输出与验证范围

- `derived`：连续及离散符号矩阵、适用条件、状态含义、初态映射和矩阵恒等式残差；可选通道传函/差分系数。
- `numeric`：完整快照、double 矩阵、sysc/sysd、极点与稳定性。
- `dv_export`：`difference_equations.md` 中的状态递推、矩阵/初态解释；`dv_coefficients_<method>.m` 以明确物理参数/Ts 为输入，输出 `[Ad,Bd,Cd,Dd]`。
- `dv_verify`：逐通道伯德图、`frequency_response.csv`、`time_error.csv`、`poles.csv`、`checks.csv` 与原始数据。结果显式包含 implementationScope、symbolicStatus、implementationStatus、acceptanceStatus。

符号证明与数值通过分开：theta 方法检查矩阵恒等式；ZOH 检查增广转移矩阵的 `dE/dt=F*E` 与 `E(0)=I`，不套用 sMap 证明，数值结果另对照 c2d。符号无法证明时保留 NOT_RUN，能证明不等时为 FAIL。未规定连续近似阈值时，只报告观测误差，总体验收保持 NOT_RUN。

ZOH 符号增广 expm 可能在大矩阵上膨胀；本 helper 不提供自动超时退路。复杂系统应按方法参考保留指数表达式并编写外部数值 expm 系数函数，不能为了输出巨大的 I/O 多项式卡住状态推导流程。

`dv_selftest(outDir)` 验证辅助工具的通用 TF/SS 数学，包括矩阵模式、MIMO、初态、复位和不稳定候选；single、非零初态全回放、双实例、调度、Simulink 和代码生成验收不在该自测结论内。
