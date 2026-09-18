# KRE 有效磁链联合校正：方程与试验合同

## 范围与来源

这是独立、离线、observer-only 的 MATLAB 试验。没有 Simulink 模型、生产 C、FOC 所有权、A2L 或硬件改动。原始 KRE 的数值基准是当前工作树的 `KRE_codegen/kre_external_observer_diagnostic_step.m`，不是假定 HEAD 内的旧版本。代码默认参数来自 `main_cm4.c`、FOC 配置头和 KRE adapter；不代表实机标定。

理论来源为本仓库 `ges/1-s2.0-S0005109825000299-main.pdf-0dc305e8-a538-499a-9ea5-d7124041c875/full.md` 的 KRE 回归关系及式 (15)。联合校正的动机来自用户指定任务“磁链矫正”的 NFO 论文；只借鉴结构，不移植该任务的电机参数或增益。本试验参数化与 NFO 原文增益量纲/归一化不同。

## 1. 有效磁链及目标

令 d(theta)=[cos(theta),sin(theta)]'，q(theta)=J d(theta)，J=[0,-1;1,0]，DeltaL=Ld-Lq。常参数 PMSM：

```
i = id*d + iq*q
lambda = (Ld*id+psi_f)*d + Lq*iq*q
v = R*i + d(lambda)/dt
x = lambda-Lq*i = rho*d,  rho = psi_f+DeltaL*id
||x||² = rho²
```

这里 rho 是有符号标量，不能把恒定 psi_f 当作 IPMSM 有效磁链幅值。恒定目标仅在 SPMSM (DeltaL=0)、id=0 或可忽略凸极项的近似中成立。真值 id 目标是理想诊断对照，禁止作为可部署候选。

候选从现有电流及估计有效磁链构造 `idHat=i'*sigma(xHat)`；当 `||xHat||<epsilon` 时 sigma=0，与原 KRE 一致。若 rho>epsilon 且方向误差 delta=angle(xHat)-theta：

```
idHat = id*cos(delta)+iq*sin(delta)
rhoRef² = (psi_f+DeltaL*idHat)²
rhoRef²-rho² ~= 2*rho*DeltaL*iq*delta  (small delta)
```

因此目标可由现有测量构造，但不能在未收敛时当作独立、可靠的幅值真值。参数失配会再引入目标偏差。SPMSM 极限下估计方向不影响目标，但等幅相位错误对这种幅值误差反馈完全不可见。

原 KRE 的非零有效磁链、小凸极 `abs(DeltaL)*||i||<psi_f` 及持续激励等条件必须区分。rho→0 时方向对误差高度敏感；rho<0 时有效磁链方向相对转子 d 轴翻转 pi，此时 idHat趋于-id，不再是转子 id。平方目标本身的恒等式仍成立，但不能从该平方量判断方向分支。脚本保存 signed rho、epsilon 和假设标志，不用 abs/限幅把这类工况判为通过。名义 DeltaL=-80 uH 对应 rho=0 的 id=575 A，仅作为解析边界，不是建议实机电流。

## 2. 原始离散实现

输入是 observer 边界的同一拍 PU VI 元组。生产侧若已有电压延迟，应在采样进入该边界前完成；本脚本不另加延迟，更不平移评价时间轴。合成波形是连续电机方程的一致样本，不模拟 PWM、噪声、死区、电流环或机械闭环。

H2(s)=alpha/(s+alpha)，H1(s)=alpha*s/(s+alpha)。对每个 H2 使用 `h=1-exp(-alpha*Ts)`。严格复现当前源码的先 H2 更新、后计算回归量的顺序：

```
hv+ = hv + h*(v-R*i-hv)
hi+ = hi + h*(i-hi)
h1i = alpha*(i-hi+)
w1 = hv+ - Lq*h1i;  w2 = w1-DeltaL*h1i;  phi=w1+w2
hr+ = hr+h*(w2'*w1-hr)
y = DeltaL*hi+'*w1 + (w1'*w1+hr+)/alpha
xHat = lambdaHat[k]-Lq*i[k]
idHat=i'*sigma(xHat); hd+=hd+h*(idHat-hd)
dHat=-psi_f*DeltaL*alpha*(idHat-hd+)
e=phi'*xHat+dHat-y
```

候选校正及 Euler 状态更新，右端 Q、Y、lambda 均为更新前值：

```
en = (rhoRef²-xHat'*xHat)/psi_f²
c = en*(br*xHat + bt*directionCommand*J*xHat)
E = -gamma*Y+c
Q+ = Q+Ts*(-a*(Q-phi*phi'))
Y+ = Y+Ts*(-a*(Y-phi*e)+Q*E)
lambdaHat+ = lambdaHat+Ts*(v-R*i+E)
```

directionCommand为外部指令符号，零指令关闭切向项；真实速度不参与该符号选择。仅切向 br=0，联合 br=bt=b。b=0 时显式旁路 c，恢复原始 KRE。br、bt 单位 s^-1。负对照仅把 Y 更新中的 E 换为 -gammaY；它被排除于部署候选及性能验收之外。

rawAngle来自更新前 xHat；PLL保持当前源码的先积分后更新角度语义（首次有效时直接初始化角度），输出 PLL 更新后的角度。所有评价都在源代码同一调用时刻进行，不补偿或隐藏这一混合时序。速度先按极对数从电角速度换算 PU，再经原速度低通。

## 3. Q/Y 一致性、初始滤波瞬态与离散余项

匹配电机参数且相同输入时，xtilde=xHat-x=lambdaHat-lambda，xtilde_dot=E。保留真实回归滤波缺陷：

```
dtilde=dHat-dTrue
epsilon_f=phi'*x+dTrue-y
e=phi'*xtilde+dtilde+epsilon_f
xi_dot=-a*(xi-phi*(dtilde+epsilon_f))
pi=Y-Q*xtilde-xi
pi_dot=-a*pi                  (Y 使用完整 QE)
pi_dot=-a*pi-Q*c              (Y 漏掉 Qc)
```

epsilon_f包含不能被直接忽略的滤波初始条件项；采样保持引起的电机/回归偏离也显式归入它。令 epsilon_f=0 是额外近似，不是代码启动瞬间的恒等式。参考积分单独保留 dTrue 的滤波状态及 epsilon_f，不将二者合并解释为论文已证明的小扰动。

同时 Euler 更新 Q、xtilde、xi、Y 时：

```
pi[k+1]=(1-a*Ts)*pi[k]-Ts²*Qdot[k]*E[k]
```

漏 Qc 还多出 `-Ts*Q[k]*c[k]`。单步正确残差为二阶，累计一般一阶；漏项造成不随步长消失的连续模型偏离。脚本既做 double 代数检查，也保存时间曲线和步长收敛，不能由“接了 Qc”推出闭环稳定。

上述简式适用于参数匹配。若比较不同 R/Lq 定义下的物理有效磁链误差，xtilde_dot还包含 `(Rtrue-Rhat)*i+(Lqtrue-Lqhat)*di/dt`，其 Q 倍项也进入 pi 关系。参数失配回放只作鲁棒性评价，不将匹配参数恒等式直接当作失配证明。

## 4. 切向项的局限

独立纯切向增量 c_t=k*J*x 有 x'*c_t=0，但：

```
||x+Ts*c_t||² = ||x||²+Ts²*||c_t||²
```

连续正交不等于 Euler 幅相解耦；完整观测器中另有电压积分和 E0 的交叉项，不能据此断言完整轨迹幅值总增大。xHat=0 时新增 c=0；等幅且 SPMSM 目标正确时 c=0，即使相位完全错误。原 KRE 仍可能自行恢复，这些是新增校正的无作用情形，不是完整 KRE 的不可启动证明。

对正有效磁链、正确幅值、很小角误差，新增切向项单独贡献：

```
delta_dot ~= [2*bt*directionCommand*rho*DeltaL*iq/psi_f²]*delta
```

其符号依赖 iq、DeltaL及指令，不能用“正转/反转切换 J 符号”保证所有象限恢复。该式不是总 KRE 的特征值；实际稳定性必须由联合动态回放判断。静止/弱激励不承诺收敛，更不宣称解决零速可观测性。

## 5. 显式状态、参数与复位合同

`kre_study_step(s,vi,p,pll,opt,direction,oracleId,reset)` 无 persistent、global、SLX、SLDD或隐藏工作区依赖。列是独立实例，不共享状态。

| 参数/状态 | 含义和单位 | 尺寸/初始化 |
|---|---|---|
| p | R [Ohm], Ld/Lq [H], psi [Wb], Ts [s], baseV [V], baseI [A], baseSpeed [rpm], polePairs, alpha/a [s^-1], gamma [1/(V² s)], epsilon [Wb], speedFilter [Hz] | 14×M，运行期间冻结，正有限值 |
| pll | 带宽 [Hz]、阻尼 | 2×M，原始默认值 |
| opt | br/bt [s^-1]、target 0/1/2、是否包含 Qc | 4×M，显式试验配置 |
| vi | vAlpha/vBeta/currentAlpha/currentBeta，按 p 指定基准归一化 | 4×M，single 或 double |
| s(1:2), s(3:4) | H2(v-Ri) [V]、H2(i) [A] | 零初值 |
| s(5),s(6) | H2(w2'w1) [V²]、H2(idHat) [A] | 零初值 |
| s(7:10),s(11:12) | Q(:) [V²]、Y [V² Wb] | 零初值 |
| s(13:14) | lambdaHat [Wb]，不是 xHat | 零初值；故有电流时首拍 xHat=-Lq*i |
| s(15:18) | PLL角 [rad]、积分 [rad/s]、速度 PU、初始化标志 | 零初值 |
| reset | 非零有效，每列独立；清零后同拍执行 | 不改参数、不影响其他列 |
| oracleId | 仅 target=2 时使用的真值诊断 id [A] | 默认候选虽传入但完全不读取其数值 |

输出 o 的 12 行：xAlpha、xBeta、rawAngle、PLLangle、omega、speedPU、mag、cAlpha、cBeta、rhoRef²、en、regressionError。x/c分别为 Wb/V；角 rad，omega rad/s，mag Wb，目标 Wb²，en无量纲，e为 Wb·V。

快速核心假设快照已验证，不做参数热写或部署保护。回放外围发现 NaN/Inf、磁链 >10*psi默认值 或任何状态绝对值>1e12时，终止该列并保留失败时刻；这些是试验停止判据，不是运行限幅，也不清零后继续。之后记录 NaN，不将失败用例从总体均值中静默去除。

## 6. 评价预定义

- 主矩阵：10个正负电频率×6组 id/iq×9组独立参数失配×9种算法=4860。失配只变观测器参数，合成电机不变。
- 50 us、single；H2精确系数，Q/Y/lambda Euler。25/100 us敏感性覆盖±2/±100 Hz全部电流、失配和增益；不是每个频率的全三步长笛卡尔积。
- 原始角和PLL角分别算环绕 RMSE/最大绝对误差，幅值算相对 RMSE/最大误差。窗口为最后 max(T/2,一个电周期)，低频每例运行5个周期；普通最低2秒，动态4秒。没有时间轴平移。
- 收敛阈值以 PLL 角<5°、幅值<5% 同时成立，持续累计一个实际电周期为准；必须一直保持到结束才标 `Converged`。`Settling_s` 是首次达到持续周期的区间起点，不能单独替代最终收敛标志；复位重新计时。动态按积分 abs(f)*dt 累积周期，不用反转零点的瞬时 1/f。静止和停止末态不记收敛。
- 持续激励不以“有转速”自动判定。记录后半程 Q 最小特征值作为诊断，不设任意阈值宣布理论PE通过。静止只记有限/误差，不宣称可观测。
- 固定候选值得继续：低速 abs(f)<=2 Hz 的所有匹配用例平均 raw及PLL RMSE均改善≥10%；所有基线已收敛稳态/动态用例不新增失败、不丢收敛，raw、PLL、幅值RMSE逐例恶化都≤5%。这是对计划“误差不恶化”的保守、显式解释；同时导出原始比值，避免总体平均掩盖个别退化。
- double RK4 为独立连续 RHS、每个 Ts 内输入零阶保持，4/8子步交叉验证，最大磁链差<1e-7 Wb才接受参考。比较覆盖2/50 Hz、KRE/T20/RT20、100/50/25 us。参考不收敛的行不用于连续解结论；连续参考不替代源代码离散一致性检查。
