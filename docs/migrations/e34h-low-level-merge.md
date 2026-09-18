# E34H 底层移植记录

## 基线与范围

- 当前工程：`D:\A_PRJ\infineon3in1\code_xcp`
- 来源工程：`D:\A_PRJ\infineon3in1\GuoChuang_E34H350V_code_0822`
- 移植前基线：`af32f98e0addd043af3574a8c953748a6ec693fe`
- CM0：语义合并 Boot、M0 应用诊断、DTC/NVM、EEPROM、Flash、CAN、看门狗及必要工程配置。
- CM4：仅合并看门狗、版本信息及必要工程配置；不改变电机控制、FOC 参数、KRE、ADRC、IdMap、APSFSM、RRC-DOB、VAFID 和负载统计行为。
- 保持当前 LED P0.0、主开关控制、三个 DBC 和 A2L 文件不变。

## 实现提交

- `62b40d01886471908f0591b19fddaf8c062f2840`：Boot 双镜像编程流程。
- `4bcd0890edfef6383cd538c5e55d155216b30833`：CM0 异步 Flash 与应用看门狗。
- `4bd3135c6847570a0111fd6b536c6a0739a68f87`：CM0 E34H 诊断与 NVM。
- `39e42f5ac07e4a8eb1b69dbc0ca82ff56c92c0b3`：CM4 看门狗与版本信息。
- `cb97d89b11b02d78be24ef8f261388c3ff16caa8`：IAR 工程与链接配置。

## 镜像契约

| 项目 | 地址或版本 |
| --- | --- |
| Boot Flash | `0x10000000–0x1000FFFF` |
| CM0 总保留 / M0 应用 Flash | 192 KiB / `0x10010000–0x1002FFFF`（128 KiB） |
| M0 用户信息 / SPC | `0x10010400` / `0x10010410` |
| M4 应用 Flash | `0x10030000–0x1008FFFF` |
| M4 用户信息 / SPC | `0x10030500` / `0x10030510` |
| M0 / M4 / HW 版本 | `1.1.2` / `2.2.2` / `1.5.0` |
| CM0 heap / stack | 1 KiB / 4 KiB |
| CM4 XCP RAM | `0x400` bytes |

## 已移植内容

- Boot：M0/M4 顺序下载、下载依赖与完整性检查、编程计数和 Work Flash 持久化、应用有效性判断与跳转、MCWDT0。
- CM0 诊断：E34H DID `0x5011–0x501B`、DTC/快照/NVM 数据链、`0x19`、`0x31`、`0x85` 服务。
- Flash/EEPROM：16 项异步队列、4096 字节数据缓冲、`Flash_Task_Process()` / `Flash_Task_Wait()`、同步主 Flash 与 Work Flash 接口。
- CAN：Bus-Off 恢复回调和来源版 CM0 CAN 用户层有效变化。
- 看门狗：Boot/M0 使用 MCWDT0，CM4 使用 MCWDT1；初始化和喂狗均接入既有任务边界。
- 工程：四个 IAR 工程加入所需源文件和包含路径；未加入 IPE、Power 空壳、旧 CM4 main/RTE/IRQ、旧死区补偿或旧算法实现。

## 构建与布局验证

使用 IAR Embedded Workbench for ARM 9.40.1、配置 `Multi Motor Evalkit V1.0`：

| 工程 | 结果 | map 摘要 |
| --- | --- | --- |
| CM0 Boot | 成功，0 error / 68 warnings | 代码结束于 `0x1000A3E2`，Boot Flash 尚余 `0x5C1D`；stack 1 KiB |
| CM0 | 成功，0 error / 102 warnings | 用户信息位于 `0x10010400`；代码/常量结束于 `0x1001CE2F`，128 KiB 应用区尚余 `0x131D0`；stack 4 KiB |
| CM4 FOC | 成功，0 error / 14 warnings | 用户信息位于 `0x10030500`；代码/初始化数据结束于 `0x100417FF`，应用区尚余 `0x4E800`；stack 8 KiB，XCP RAM 1 KiB |
| CM0+CM4 组合工程 | 成功，0 error / 102 warnings | 组合工程配置可解析并成功生成输出 |

生成 HEX 的 SHA-256：

- Boot：`3C5A6C3F86D8566316EDA34B9C00B866769F871396C5BD55635E5375BF225D46`
- CM0：`83828EDE1B6D95BD1865511059C8633C873FC5B1DBAE2861CF390E56FB561CBD`
- CM4：`A577C8B4681342809C580A47226EAFF01D66C9846F3B60816369BF648D62716D`

HEX 地址复核结果分别为 Boot `0x10000000–0x1000A3E2`、CM0
`0x10010000–0x1001CE2F`、CM4 `0x10030000–0x100417FF`。CM0/M4 用户信息区
首字节分别为 `M0 1.1.2, HW 1.5.0` 和 `M4 2.2.2, HW 1.5.0`，两核 SPC 均在
用户信息偏移 `0x10` 处包含四个 `0x55`。

## 严格保留的来源行为与风险

以下问题按本次约定未修正，因此完成来源一致性移植不等同于可直接量产发布：

1. 异步 Flash 接口仍存在 byte/word 长度混用，以及超时后继续出队的来源行为。
2. 同步 Flash 擦写接口保留来源版返回/错误传播语义。
3. F194/F195 DID 映射和 DCM 声明/实现命名不一致仍保留。
4. M0 下载结束地址边界判断仍保留来源实现。
5. 来源列出 16 个 E34H DTC，但 `U14E177` 的枚举、配置与 EEPROM 地址均被注释，实际仅 15 个生效。
6. `0x5011–0x501B` 已配置，但来源未为其设置 `g_service_22.did_val_point`，直接读取可能访问空指针。
7. `EE_DTC_EXTENDED_BYTES=24` 可能与扩展数据结构实际布局不一致。
8. EEPROM 全局临时缓冲为 1024 字节，但部分清零长度使用 `EE_WR_ITEM_TOTAL_SIZE`，存在越界风险。
9. MCWDT 上限值 `32000` 按来源保留，其“2 秒”注释与时钟配置可能不一致。
10. M0 与 M4 看门狗实现均使用全局名 `mcwdtConfig`；分核链接无冲突，若未来直接合并对象需重新处理命名。
11. Boot SID34 只检查下载地址不低于 M0 起始地址，未对镜像白名单、上界及“地址 + 长度”终点做完整检查；更高的任意地址可能进入复制流程并被按 M4 处理。
12. Boot 跳转只检查跳转标志及 M0 首字非 `0xFFFFFFFF`，未校验 MSP、ResetHandler、两核 SPC 或 M4 镜像；M0 应用随后仍无条件启动 CM4。
13. SID22 在请求包含重复合法 DID 时，可能在长度检查之前使 `req_did_num` 超出固定数组 `DCM_DID_NUM`，造成越界写。
14. EEPROM 分页写对长度小于 32 字节的条目仍先写固定 32 字节，随后又从 `addr+32` 写余数，存在越界和错位写入。
15. CAN 错误回调未区分错误类型，任意 CAN 错误均会标记 Bus-Off；任意发送完成又会清除恢复状态，且恢复流程可在 1 ms 中断路径执行完整反初始化/初始化。
16. Linker 允许 CM4 使用 `0x10030000–0x1008FFFF`，而 Boot 使用 `0x10060000–0x1008FFFF` 作为暂存区；当前 CM4 镜像未越过 `0x10060000`，后续增长不会由链接器自动阻止重叠。

## 尚未验证

- 未进行实车/台架验证：M0/M4 单独和连续刷写、掉电恢复、编程计数、非法地址、无效应用不跳转。
- 未进行诊断仪验证：DID 会话/权限/长度、DTC 快照与清除、`0x85`、掉电后 EEPROM 数据。
- 未进行看门狗故障注入、Boot 到应用切换复位原因验证。
- 未进行 CM4 启停、调速、反转、重复启动及实时负载回归；代码差异审计确认未改动其控制路径。
