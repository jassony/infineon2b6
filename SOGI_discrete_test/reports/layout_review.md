# 最终图面复核记录

日期：2026-09-10。结论：**PASS**。这是代理逐图阅读后的独立图面记录，不是 `check_sogi_standard` 自动生成的视觉结论，也不代表用户已签字批准。

| 作用域 | 最终图面 | 结果 |
|---|---|---|
| 算法顶层 | [model_layout.png](model_layout.png) | PASS |
| FE | [fe_layout.png](fe_layout.png) | PASS |
| BE | [be_layout.png](be_layout.png) | PASS |
| Tustin | [tustin_layout.png](tustin_layout.png) | PASS |
| ZOH | [zoh_layout.png](zoh_layout.png) | PASS |
| 验证顶层 | [validation_layout.png](validation_layout.png) | PASS |
| 连续参考 | [continuous_layout.png](continuous_layout.png) | PASS |

四方法内部由主代理分别查看；两模型顶层和连续参考由另一代理独立复核。全层级模块与注释属性已读取为 14 pt；信号文字在布局函数中统一 Arial 14 pt。七个作用域保存为 100% 缩放，重新打开两模型仍为 100%。导出的是完整原始尺寸 PNG，阅读时可按原始尺寸放大或滚动，不以聊天界面自动缩放后的缩略图作字号标准。

| 规则 | 检查与证据 | 结果 |
|---|---|---|
| 普通信号按工序左到右 | 输入/预处理在左，输出与状态方程在右；验证后处理及输出在算法右方 | PASS |
| local 标签不掩盖前向倒置 | U_k 的生产区在消费方程左方；Held_Input、Discrete_DQ、Reference_DQ 的前向消费者位于生产区右方 | PASS |
| 状态返回与控制分发明确 | 有效状态在底部更新和发布，X1_k/X2_k 对应物理或辅助状态；Reset_k、Zero_k 有独立来源 | PASS |
| 连续状态的普通输出打包向右 | 输出 From 位于状态发布区域右方，随后依次为 Pack_DQ、DQ；反馈 From 保留在积分方程左侧 | PASS |
| 字号和模块尺寸 | 块名、信号名、注释 14 pt；Gain 宽 120 px，Sum 宽约 53 px，Unit Delay 宽 75 px，四方法外框 300×165 px | PASS |
| 长名称空间 | 完整 XCP 输出名及最长 Tustin 状态名不截断；Four_Methods 为 620×480 px | PASS |
| 预览不干扰端口名 | 六个层级块的 ContentPreviewEnabled 均为 off，重新打开后读取确认 | PASS |
| 正交与遮挡 | 自动检查零斜线、零穿过无关模块；逐图未见连线遮挡名称、标签或注释，未见无关线段交叉 | PASS |
| 数值与接口保持 | 最终全套离散回放 PASS；连续输出区移动后再次连续回放 PASS；结构与编译接口自动检查 188 PASS | PASS |

复核中发现的“预览压住端口名”和“连续输出打包回到左侧”均已修复，并写入规则及操作流程。未来修改后必须重新导出、检查和记录，不能沿用本次 PASS。
