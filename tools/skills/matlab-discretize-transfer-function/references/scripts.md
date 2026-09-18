# 脚本接口与通用示例

从本技能 `scripts/` 目录调用函数，或仅将此目录临时加入 MATLAB 路径。所有任务输出应放在调用者明确指定的目录，不能运行时写入技能目录。函数工作区隔离现有 base workspace；不要执行 clear all、close all 或 reset(symengine) 清除用户会话。

## 参数化 TF 入口

以下示例是一阶系统，参数名称、数值和测试要求仅用于演示接口：

```matlab
s = sym('plant_s');
Ts = sym('sample_period', 'positive');
K = sym('plant_gain', 'positive');
tau = sym('plant_tau', 'positive');
source = struct('kind','tf','s',s,'G',K/(tau*s+1));

% 调用者先验证领域参数，Ts 与实际调度另行匹配。
snapshot = [2, 0.02, 0.001];  % K [1], tau [s], Ts [s]
assert(all(isfinite(snapshot)) && all(snapshot>0));
derived = dv_from_tf(source.G,s,Ts,'Tustin');
numeric = dv_realize(derived,[K tau Ts],snapshot);
outDir = fullfile(pwd,'discrete_reference','first_order_tustin');
dv_export(derived,numeric,[K tau Ts],outDir);

test.t = (0:1000)'*numeric.Ts;
test.u = ones(size(test.t));
test.reset = false(size(test.t));
test.reset(501:505) = true;
test.frequencyHz = logspace(-1,2,200)';
test.absTol = 1e-9;
test.relTol = 1e-9;
test.magnitudeFloor = 1e-10;
% 如已有允许的复频响误差阈值，在运行前设 test.approximationAbsTol。
report = dv_verify(numeric,test,outDir);
disp(report.checks);
```

`dv_derive` 支持 `FE`、`BE`、`Tustin`、`TustinPrewarp`、`ZOH`。
预畸变再传入单独的符号：

```matlab
wp = sym('prewarp_rad_s','positive');
derived = dv_from_tf(source.G,s,Ts,'TustinPrewarp',wp);
numeric = dv_realize(derived,[K tau Ts wp],[2 .02 .001 2*pi*5]);
```

参数列表必须完整、符号唯一且是单符号，数值为有限 double。所有未绑定自由符号都会被拒绝。`s`、Ts、wp 必须不同；`dv_z`、`dv_q` 为 helper 保留符号。
领域合法范围需由调用者验证，不能用 generic helper 的有限性检查代替。

## 状态空间入口

```matlab
source = struct('kind','ss','s',s, ...
    'A',[-3 1;-2 -4],'B',[1 2;0 1], ...
    'C',[1 0;1 2],'D',[.2 0;0 -.1]);
derived = dv_derive(source,Ts,'ZOH');
numeric = dv_realize(derived,Ts,0.01);
```

A/B/C/D 可以包含任意外部物理参数符号。维度必须一致，A 方阵；不接收带延时或隐式 E 的对象。
MIMO 的 test.u 是 N×nu，所有输出保留 N×ny；不能只验证第一路。

## 结果与运行函数

`derived` 保存连续/离散符号 TF、A/B/C/D、Ad/Bd/Cd/Dd、适用条件、辅助状态含义、初态映射和 `recurrence{iy,iu}.a/.b`。
`numeric` 保存本次数值参数、sysc/sysd、数值矩阵和递推系数、完整 Ad 极点。

```matlab
xi = zeros(size(numeric.Ad,1),1);
[y,xiNext] = dv_step(xi,uColumn,reset, ...
    numeric.Ad,numeric.Bd,numeric.Cd,numeric.Dd);
xi = xiNext;
```

`dv_step` 是算法原语，假设调用者已检查参数、类型、维度；不在每拍重新求系数。
可接受一致的 single 输入/状态/矩阵，但本版 `dv_verify` 默认验证 double，不能据此声称 single 已通过。

`dv_export` 生成：

- `difference_equations.md`：符号与数值状态递推、q 升幂 a/b、单位/初态/直通/复位说明。
- `dv_coefficients_<method>.m`：显式物理参数及 Ts 输入，输出 Ad/Bd/Cd/Dd；按传入的符号顺序生成。
- `dv_io_coefficients_<method>_o<iy>_i<iu>.m`：相同参数顺序，直接输出该通道的 `[b,a]`；两者均按 q 升幂排列，a(1)=1。

`dv_verify` 输出 `checks.csv`、`poles.csv`、`frequency_response.csv`、`time_error.csv`、原始验证数据及每个 I/O 通道的伯德图。文件名以实际返回的 report 为准。
`implementationStatus` 判断离散实现一致性；`acceptanceStatus` 还包含近似质量阈值、稳定性和覆盖。未设置近似阈值时将总体验收保留 NOT_RUN。

## 自测与限制

```matlab
summary = dv_selftest(fullfile(pwd,'skill_selftest_output'));
```

自测覆盖一阶参数化 TF 的五种方法、耦合 MIMO/直通、缺项多项式、零/常量 TF、奇异 A 的 ZOH、初态映射、复位/重复运行、非法输入和不稳定候选门禁。不是 SOGI 专用测试，也不会加载 SLX/SLDD。

本版重点是小规模 LTI 的确定性流程。通道多项式条件差时可能无法满足等价误差阈值，应报告失败并采用状态空间/SOS，不随意放宽阈值。非零物理初态时域、single 全回放、双实例复用、延时和描述系统需要任务专门验证；通用技能可指导扩展，但 helper 未自动覆盖的部分不算已通过。
