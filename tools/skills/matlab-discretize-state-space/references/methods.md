# 方法选择、推导和状态语义

## 范围与选择

以 `xdot=A*x+B*u; y=C*x+D*u`、`G(s)=C*(sI-A)^(-1)*B+D` 为共同表示。
TF 的状态实现通常是辅助坐标，不自动等于资料中的物理状态。

| 方法 | 优先考虑的目标 | 约束或代价 |
| --- | --- | --- |
| FE | 显式状态积分、低计算量、与原状态更新易映射 | 必须检查所有 `1+Ts*lambda(A)`；稳定连续系统也可能离散不稳定 |
| BE | 隐式欧拉的因果实现，需要较强数值阻尼 | 消除隐式求解，不能漏掉当前输入直通；会产生幅相误差 |
| Tustin | 保持目标频段的频域形状 | 存在频率扭曲；离散状态为辅助状态 |
| TustinPrewarp | 在指定 `wp` 准确匹配复频响 | `wp` 单位 rad/s，`0<wp<pi/Ts`；匹配一个频率不保证全频段 |
| ZOH | 输入每拍保持恒定的系统 | 采样时刻与相同阶梯输入的连续解一致；不是无保持正弦全频段精确匹配 |

不要要求每个项目比较全部方法；按关注频带、时间响应、实现成本和用户指定目标选择。
FOH、impulse、matched、least-squares 是可选后续方法；需分别记录保持、延时和初态约定，本版 helper 不宣称支持它们。

## FE / BE / Tustin 的统一解析实现

取 `theta=0/1/1/2` 分别表示 FE/BE/Tustin；通常 `h=Ts`。
预畸变 Tustin 使用 `theta=1/2, h=2*tan(wp*Ts/2)/wp`，实际采样周期仍为 Ts。

离散积分关系：

```text
x[k+1]-x[k] = h*((1-theta)*(A*x[k]+B*u[k])
                    + theta*(A*x[k+1]+B*u[k+1]))
M = I-theta*h*A
xi[k] = M*x[k]-theta*h*B*u[k]
Ad = M \ (I+(1-theta)*h*A)
Bd = M \ (h*B)
Cd = C / M
Dd = D+theta*h*(C/M)*B
xi[k+1] = Ad*xi[k]+Bd*u[k]
y[k] = Cd*xi[k]+Dd*u[k]
```

要求 M 非奇异，数值求系数时用线性求解，不在逐拍函数计算矩阵逆。
通过消除 x[k] 得到以上形式；该状态定义是一个明确的实现选择，不要求与其他库的缩放一致。

| 方法 | TF 代换 `sMap` | 状态说明 |
| --- | --- | --- |
| FE | `(z-1)/Ts` | xi=x；旧状态生成输出，输入影响下一拍状态 |
| BE | `(z-1)/(Ts*z)` | `xi=(I-Ts*A)*x-Ts*B*u`，按 BE 递推可解释为上一拍物理状态 |
| Tustin | `2/Ts*(z-1)/(z+1)` | `xi=(I-Ts*A/2)*x-Ts*B*u/2` |
| 预畸变 Tustin | `wp/tan(wp*Ts/2)*(z-1)/(z+1)` | 同上，以 h 代替 Ts 的积分系数 |

状态空间默认验证矩阵残差 `Ad*M=I+(1-theta)*h*A`、`Ad*theta*h*B+(1-theta)*h*B=Bd`、`Cd*M=C`、`Cd*theta*h*B+D=Dd`。只有明确需要通道 TF 时才验证 `Cd*(zI-Ad)^(-1)*Bd+Dd == G(sMap)`。符号化简无法证明时记录 UNKNOWN/NOT_RUN 并增加数值抽样，不能把抽样当解析证明。

初态与复位必须分开：

- 零辅助状态 xi0=0 表示离散系统的零历史状态；非零 u0 时 BE/Tustin 的输出可以有 Dd*u0。
- 如指定物理 x0，则 `xi0=M*x0-theta*h*B*u0`，这样首拍 y0=C*x0+D*u0。该输入相关映射不是“修改初值以隐藏时间误差”。
- 复位优先时当拍输出为零（连直通项也屏蔽），状态清零，复位拍输入不进入历史；释放后的零历史约定需在合同中明确。

## ZOH

```text
E = expm(Ts * [A B; zeros(nu,n+nu)])
Ad = E(1:n,1:n)
Bd = E(1:n,n+1:n+nu)
Cd = C; Dd = D; xi=x
```

此表达式对奇异 A 同样成立，不使用 `A^(-1)*(Ad-I)*B`。
小规模符号系统可直接 `expm(symMatrix)` 化简；大规模时保留增广指数公式并在外部配置阶段数值计算。ZOH 没有一个通用的有理 s 代换可直接套用所有 TF。

## 可执行差分形式

设 `q=z^-1; H(q)=B(q)/A(q)`，`A(q)=1+a1*q+...+an*q^n`：

```text
y[k] = -a1*y[k-1]-...-an*y[k-n]
       +b0*u[k]+b1*u[k-1]+...+bm*u[k-m]
```

MATLAB a/b 数组按 `q^0,q^1,...` 排列，因此 `a(1)=1`、`b(1)=b0`。
零系数占位不能删除，特别是 FE/ZOH 严格真有理系统常有 `b0=0`。
不要直接把 z 降幂系数交给该递推；先代入 `z=1/q`，消去负幂、归一化、重建核验。
MIMO helper 输出每个 I/O 通道的独立递推 v_ij，再 `y_i=sum_j(v_ij)`；共享物理状态时优先采用状态递推以避免冗余和病态多项式。

符号约消不证明内部稳定；完整 SS 的不可观测/不可控不稳定状态仍须报告。高阶直接型数值敏感时改用 SS/SOS，并单独核验实现精度。

## 官方依据

- [连续离散转换方法与状态映射](https://www.mathworks.com/help/control/ug/continuous-discrete-conversion-methods.html)
- [c2d 支持的方法与初值映射](https://www.mathworks.com/help/control/ref/dynamicsystem.c2d.html)
- [c2dOptions 预畸变频率](https://www.mathworks.com/help/control/ref/c2doptions.html)
- [coeffs 的 All 选项与系数顺序](https://www.mathworks.com/help/symbolic/sym.coeffs.html)
- [matlabFunction 参数顺序](https://www.mathworks.com/help/symbolic/sym.matlabfunction.html)

已于 MATLAB R2026a 核对 API。FE/BE 及统一 theta 实现是本技能的解析推导，不把它们冒称为 c2d 内建方法。
