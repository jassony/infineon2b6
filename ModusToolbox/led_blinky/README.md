# CYT2B63CAE：IAR → VS Code / ModusToolbox 最小闪灯版

使用 **ModusToolbox 官方构建系统 + GCC_ARM**，运行在上电启动核 **CM0+**。
LED 为用户确认的 **P0.0**，初始化为高电平，每 **500 ms** 翻转一次，完整周期为 1 s。
高/低电平哪个对应点亮取决于板上接法，不影响闪烁验证。

GPIO 映射和初始化参考原工程：
`../../M0_BSW/SOURCE/ld_gpio/inc/m0_gpio_cfg.h`、
`../../M0_BSW/SOURCE/ld_gpio/src/m0_gpio_cfg.c`。
本例不启动 CM4，不包含电机、PTC、CAN、XCP 或原业务调度。

## 打开和构建

1. 在 VS Code 中打开本目录的 **ifl_2b6_led_blinky.code-workspace**。
2. 按 **Ctrl+Shift+B** 执行默认 **Build**。
3. 或在 PowerShell 7 中进入本目录，运行 `./run-mtb.ps1 build`。

输出位于 `build/IFL-2B6-3IN1-BLINK/Debug/`：
`ifl_2b6_led_blinky.elf`、`ifl_2b6_led_blinky.hex`、`ifl_2b6_led_blinky.map`。

本机依赖已准备好，无需 getlibs。重新导出配置使用 `./run-mtb.ps1 vscode`；
脚本保留 Build/Rebuild/Clean/Program 任务，工作区只打开例程目录。

## 烧写和板上验证（尚未执行）

默认调试配置为 **KitProg3 / MiniProg4，SWD**。
连接后自行执行 **Program**，或选择
`Launch TRAVEO T2G CM0+ (KitProg3_MiniProg4)` 按 F5。
调试停在 main 后继续运行，观察 P0.0。其他探针需对应配置；本轮未连接探针。

本例链接于 **0x10000000**，烧写会替换原 CM0+ 启动区。
这是独立测试固件，不是原 FOC 固件的增量补丁。

验收：LED 连续闪烁、复位/断电重启恢复闪烁，测得翻转间隔约 500 ms。
调试变量：`led_init_status` 应为 0；`led_toggle_count` 每秒增加 2；
`led_output_level` 在 0/1 间切换。以上硬件测试均为 **NOT_RUN**。

## 依赖与文件边界

| 内容 | 来源 / 版本 |
| --- | --- |
| MTB tools | C:/Infineon/Tools/ModusToolbox/tools_3.9 |
| GCC_ARM | C:/Users/ASUS/Infineon/Tools/mtb-gcc-arm-eabi/14.2.1/gcc |
| PDL，只读引用 | C:/Users/ASUS/.modustoolbox/global/mtb-pdl-cat1/release-v3.24.0 |
| CMSIS，只读引用 | 原项目 ../../CMSIS/Include |
| core-make | 官方 release-v3.10.0，例程 vendor/ 内 |
| recipe-make-cat1a | 官方 release-v2.8.0，例程 vendor/ 内 |
| core-lib | 官方 release-v1.8.0，例程 vendor/ 内 |

例程专用 BSP 为 `bsps/TARGET_IFL-2B6-3IN1-BLINK`：
启动/系统文件、分区头文件复制自现有 `C:/Infineon/BSP/TARGET_IFL-2B6-3IN1`，保留许可证。
bsp.mk 取消 HAL 和预制 CM0P_SLEEP 组件；应用直接初始化 P0.0，
不使用原 BSP 的 P23.4～P23.6 配置，也不运行 Device Configurator。
GCC 链接文件来自官方
[mtb-template-cat1 release-v1.12.0 CYT2B6 CM0+ 模板](https://github.com/Infineon/mtb-template-cat1/blob/release-v1.12.0/files/templates/cat1a/COMPONENT_MTB/COMPONENT_CM0P/TOOLCHAIN_GCC_ARM/cyt2b6_cm0plus.ld)。

Flash 分区为 `0x10000000..0x10007FFF`；RAM 为 `0x08000800..0x08003FFF`，
低 2 KB SRAM 留给 SROM。GCC 启动完成 SRAM/ECC、向量表、数据初始化并关闭内部 WDT；
应用不释放 CM4。

更换安装路径时修改 Makefile 对应变量并重新导出配置。
vendor/、build/ 是忽略目录。若复制源码后缺少 vendor/，
运行 `./prepare-build-packages.ps1` 下载固定版本构建包。
该脚本仅写本例 vendor/，保留已有目录，不运行 Git 或更改 global。
现有 global 仓库、原 BSP、IAR 配置、A2L 和业务/算法文件均未修改。

## 本次验证（2026-09-18）

- GCC_ARM Debug 编译/链接：**PASS**，Flash 使用 **4,484 字节**。
- MTB 官方 make vscode 导出：**PASS**。
- 向量表 0x10000000、栈顶 0x08004000、入口和 LED 调试符号：已核对。
- 官方链接模板在 GCC 14.2.1 下产生一条 `LOAD segment with RWX permissions` 警告；
  保留原模板且未屏蔽警告，无编译/链接错误。
- HEX SHA256：`B7719589F9AFE2DEB71A5FC991066262ED3303956DA013020218B9CC089464FD`。
- 烧写、板上闪灯、实测周期和复位：**NOT_RUN**。

构建/导出依据：[ModusToolbox 官方构建说明](https://documentation.infineon.com/modustoolbox/docs/modustoolbox-build-system)。
