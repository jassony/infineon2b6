# 10 kHz control update

With FocTiming_Cfg.h's default configuration, FOC_FAST total CPU load includes
every 20 kHz PWM ISR, including the 10 kHz intervening hold calls. The sampled
breakdown starts only on actual 10 kHz control calls: divider 16 gives about
625 samples/s (about 312 or 313 per 500 ms). Percentages are shares of sampled
control-bearing ISR time, not direct percentages of total CPU time. Do not
multiply them by total FOC_FAST load to infer exact component CPU load.
Meas_MCU_FocFastLast/MaxCycles refer to full profiled PWM callbacks;
Meas_Foc_FastLoopLast/MaxCycles now refer to the actual control call only.
Meas_Foc_PwmIrqLast/MaxCycles include the profiler bookkeeping as well.
See MS/tests/CONTROL_10KHZ.md for current acceptance and limitations.

## Original 20 kHz baseline record

# CM4 快环分段耗时诊断

默认开启 MCU_FAST_PROFILE_ENABLE，每 MCU_FAST_PROFILE_DIVIDER=16 次快环
采样一次；20 kHz 下约 1250 次/秒，每 500 ms 约 625 个样本。
结算与原有 MCU 负载共用窗口及 Meas_MCU_UpdateSeq_u32。
适用于当前 CURRENT_LOOP_FACTOR=1 的直接快环；不覆盖 PendSV 分频路径。

## IAR Live Watch 变量

| 变量 | 统计范围 |
| --- | --- |
| Meas_MCU_FastAdc_pct_f32 | ADC读取、电流重构、Clarke |
| Meas_MCU_FastObs_pct_f32 | KRE/诊断Flux观测器，以及退出运行时的观测器复位 |
| Meas_MCU_FastCtrl_pct_f32 | FOC电流控制或V/f分支，包括该分支中的RRC-DOB处理 |
| Meas_MCU_FastVPre_pct_f32 | 电压路径资格判断、HFI施加/复位 |
| Meas_MCU_FastMod_pct_f32 | 调制器执行、输出和状态读取 |
| Meas_MCU_FastVPost_pct_f32 | FWC饱和快照/抗饱和、电压坐标转换、VAFID/RRC反馈采集 |
| Meas_MCU_FastPwm_pct_f32 | PWM模式生成器及其开关/故障处理 |
| Meas_MCU_FastOther_pct_f32 | 外层快环计时范围中未划入上述七段的剩余时间 |
| Meas_MCU_FastSamples_u32 | 本次已发布窗口内完成的分段采样数，不是累计调用数 |

每段占比 = 该段采样独占周期之和 / 同批采样快环独占周期之和 * 100。
有有效采样时间时八项之和约为100%，不是占整个MCU的百分比。
无采样窗口输出全零。中断抢占按现有已完成中断独占周期累计差扣除。
分段只作归因，不加入原有总负载/中断累计，避免重复计时。
采样起点包含在原有快环DWT起点之后，终点复用原有end测得的独占周期。
统计代码自身的时间仍包含在测量内，部分会落在相邻段，不代表纯算法WCET。

## 使用方法与限制

1. 下载本次ELF对应固件，使用该ELF的IAR Live Watch按名称观察。
   未修改A2L；新变量插入XCP区后其他变量地址可能变化，不要直接沿用旧A2L地址。
2. 同时观察原有七项任务负载、Status、WindowCycles、UpdateSeq和上述九项。
   等待UpdateSeq变化后比较运行和停机状态，不在断点暂停状态判读。
   发布是顺序写入，Live Watch不是原子快照，避免将更新中的跨窗口值相加。
3. 默认抽样用于持续性问题定位，可能漏掉单次停机复位，也可能与周期性分支
   同步产生抽样偏差。需要瞬态复现时可在编译配置定义
   MCU_FAST_PROFILE_DIVIDER=1，全采样会增加开销，先确认快环余量。
4. MCU_FAST_PROFILE_ENABLE=0 可编译掉本次全部分段钩子、状态和变量，
   原有MCU总负载统计保留。MCU_LOAD_PROFILER_ENABLE=0 默认也关闭分段。
5. 不修改控制流程、优先级、M0、模型、A2L或链接区。未连接板卡，
   尚未验证快环Max/Overrun、采样扰动或停机94.5%的真正来源。

## 验证

run_mcu_load_profiler_tests.ps1 使用IAR/C-SPY可注入周期测试。
支持 -FastProfileDivider 1、默认16和 -FastProfileEnable 0。
覆盖抢占扣除、分段归一化、不重复计总负载、500ms窗口、抽样、
停止分支零工作、无样本、回绕、异常钳位及DWT不可用；
同时保留原有负载和最大值保持测试。

本次默认构建相对修改前map：readonly code +704 B，readonly data不变，
readwrite data +88 B（36 B发布量、52 B内部状态），XCP区0x3FB/0x400。
service使用局部快照，另有栈开销；上述RAM差不代表动态栈峰值。
原有“新增不超过80 cycles”和工作RAM净不增加的约束不能据此宣称满足：
这是额外的诊断开销，必须通过板上启用/关闭A/B实测决定是否长期保留。

已验证默认16分频、1分频以及关闭分段的模拟器测试全部通过。
CM4启用/关闭构建均为0错误、8个既有警告；关闭构建map中无分段符号，
且代码/RAM用量回到基线。最终交付恢复默认开启配置。
最终ELF SHA256：
ADB9CB2FC57FB5509549F4CB64681EB1CE03C8EE30CC17871245EEF54E53B5AB。
