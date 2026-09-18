# SOGI 状态空间离散化参考

本目录只实现独立 MATLAB 数学参考和 single 算术门禁。源方程为用户指定的标准双输出 SOGI；不从旧 SLX、SLDD 或基础工作区读取数值。脚本存在不代表验证已经执行，运行结果以输出目录中的 checks.csv 为准。

## 参数、状态与时序

`cfg = struct('f0_Hz',50,'k',sqrt(2),'Ts_s',50e-6,'method','FE')`。方法可选 FE、BE、Tustin、ZOH。三个数值参数为有限正 double 标量，且 f0 小于 Nyquist；逐拍 single 实现将完整配置转换后固定，Ts 必须和外部调度一致。输入和输出为 PU 标量，1 PU 是试例幅值，不是通用量程。复位非零有效，状态/输出均为二元素列向量。

令 w=2*pi*f0，x=[D;Q]，则

```text
A = [-k*w, -w; w, 0]; B = [k*w;0]; C = I; D_feedthrough = 0
xdot = A*x+B*u; y = [InPhase_D_PU; Quadrature_Q_PU] = x
```

其输出传函为 k*w*s/(s²+k*w*s+w²) 和 k*w²/(s²+k*w*s+w²)。Q 在中心频率相对输入滞后 90 度；Q 的直流增益为 k，不是直流抑制通道。

## 统一 theta 离散化

FE、BE、Tustin 分别取 theta=0、1、1/2。由 theta 加权积分得到

```text
(I-theta*T*A)*x[n+1] = (I+(1-theta)*T*A)*x[n]
                       +T*B*((1-theta)*u[n]+theta*u[n+1])
M = I-theta*T*A
xi[n] = M*x[n]-theta*T*B*u[n]
Ad = M\(I+(1-theta)*T*A); Bd = M\(T*B)
Cd = C/M; Dd = D_feedthrough+theta*T*Cd*B
y[n] = Cd*xi[n]+Dd*u[n]
xi[n+1] = Ad*xi[n]+Bd*u[n]
```

FE 的 xi 是当前物理状态；BE 的 xi 是上一拍物理状态；Tustin 的 xi 是变换后的辅助状态。BE/Tustin 保留正确当前输入直通，不能把离散 Ad/Bd 与原 C/D 任意拼接。不同方法不能直接对比内部 xi。

零初始 xi 是本次统一递推和独立 I/O 参考的初态。若指定物理 x0 且 u0 非零，必须使用 `xi0=M*x0-theta*T*B*u0`；零 xi 不自动等价于零物理 x0。参考门禁检查该映射的首拍输出，完整非零初态回放仍单独列为 NOT_RUN。

FE 物理端口实现保持以下次序，与 Simulink Product/Sum/Unit Delay 一一对应：

```text
w = single(2*pi)*f0; h = Ts*w
e = u-D; v = k*e-Q
y = [D;Q]
Dnext = D+h*v; Qnext = Q+h*D
```

复位优先于上述计算：本拍输出与下一拍存储状态置零，本拍输入不进入历史；释放后从零状态恢复。其余方法的逐拍函数同样屏蔽直接馈通。函数无 persistent/global，状态由调用者显式持有；两个实例必须各自保存状态和配置。

## ZOH

采用精确增广指数 `E=expm(T*[A B;zeros(1,3)])`；Ad=E(1:2,1:2)，Bd=E(1:2,3)，Cd=C，Dd=D。输入在一个采样区间保持，xi 为采样时刻物理状态。公式不要求 A 可逆。此处保留矩阵指数解析形式，未声称完成符号指数展开；数值结果对照 Control System Toolbox 的 c2d。

## 独立参考与门禁

FE/BE/Tustin 的 I/O 多项式从源传函直接代入 s=p(q)/d(q)，q=z^-1，并归一化 a0=1；不用待验证 Ad 再反求传函。FE 的 p=[1,-1]、d=[0,T]；BE 的 d=[T,0]；Tustin 的 p=[2,-2]、d=[T,T]。使用 MATLAB filter 在连续非复位区间从零历史运行；不平移输出时间轴。ZOH 的独立 b/a 来自源连续状态空间的 c2d。

double 状态实现对独立 I/O 要求 `abs(error)<=1e-9+1e-9*abs(ref)`；single 算术要求 `5e-5+5e-5*abs(ref)`。数值工况默认 1 秒，含零、阶跃、50 Hz、固定随机、运行中复位、幅值变化、相位变化、三/五次谐波和直流偏置。每项重复执行并检查有限值。50 Hz 最后 0.2 秒拟合相对连续理论要求幅值误差<=2%、相位误差<=1°；40/50/60/150/250 Hz 同时报告自身离散理论差异。自身拟合幅值百分比、相位度数各限 0.02；宽频 Bode 数据严格低于 Nyquist。幅值接近零时不绘制无意义相位。

极点检查包含 double 和 single 实现系数。FE 的 single 极点从 single 物理参数计算得到的系数检查；浮点舍入的非线性效应另外通过逐拍误差检查。Ts 扫描 50/100/500/1000/5000 us，先检查极点，稳定者才回放；不稳定探索明确记 NOT_RUN_UNSTABLE，不靠限幅隐藏发散。该跳过不是 PASS，也不影响已单独通过的基准配置。

## 运行入口与产物

需要 Control System Toolbox；符号推导另外需要 Symbolic Math Toolbox。若本机 MATLAB 读文件触发保护软件，应复制本目录到独立运行目录，保留本源码为明文，再运行：

```matlab
addpath('明确指定的独立运行目录');
derivation = sogi_ss_symbolic_derivation('明确指定的输出目录');
summary = run_sogi_ss_preflight('明确指定的输出目录');
```

输出包含符号推导、参数快照、double/single 矩阵、逐拍输入/复位/输出原始 MAT 数据、checks.csv、poles.csv、time_errors.csv、frequency_fit.csv、frequency_response.csv、sample_period_scan.csv，以及四法双通道 Bode PNG。数值门禁有失败时写出结果后 assert 阻止继续；符号恒等式由符号入口单独断言并记录。MATLAB 门禁不证明 Simulink 编译、布线、模型回放、模型多实例隔离或生成代码验收。
