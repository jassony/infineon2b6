/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_dtc_cfg.h
* Author        : yangming
* Date          : 2024-04-28
* Version       : 1.00
* Description   : Can dtc.
* Others        : None
*
****************************************************************************************************/
#ifndef _CAN_DTC_CFG_H
#define _CAN_DTC_CFG_H
#include "platform_common_typdef.h"
#include "can_dcm_cfg.h"

/* DTC status */
#define DTC_STS_TESTFAILED                              0
#define DTC_STS_TESTFAILEDTHISOPERATIONCYCLE            1
#define DTC_STS_PENDINGDTC                              2
#define DTC_STS_CONFIRMEDDTC                            3
#define DTC_STS_TESTNOTCOMPLETEDSINCELASTCLEAR          4
#define DTC_STS_TESTFAILEDSINCELASTCLEAR                5
#define DTC_STS_TESTNOTCOMPLETEDTHISOPERATIONCYCLE      6
#define DTC_STS_WARNINGINDICATORREQUESTED               7
#define DTC_SUPPORTED_STS                               ((1 << DTC_STS_TESTFAILED) | (1 << DTC_STS_CONFIRMEDDTC))

#define DTC_FORMAT_14229_1                              1
#define DTC_FORMAT_J1939_73                             2

#define DTC_FAULT_DISAPPEAR                             0
#define DTC_FAULT_APPEAR                                1

#define DTC_OPERATION_CYCLE_OFF                         0
#define DTC_OPERATION_CYCLE_ON                          1

#define DTC_SNAPSHOT_GLOBAL_RECORD_NUMBER               1
#define DTC_SNAPSHOT_LOCAL_RECORD_NUMBER                2
#define DTC_SNAPSHOT_INVALID_RECORD_NUMBER              0xFF
#define DTC_SNAPSHOT_GLOBAL_DID_N1                      DID_3001
#define DTC_SNAPSHOT_GLOBAL_DID_N2                      DID_300F
#define DTC_SNAPSHOT_GLOBAL_DID_N3                      DID_3010
#define DTC_SNAPSHOT_GLOBAL_DID_NUMBER                  3
#define DTC_SNAPSHOT_LOCAL_DID_N1                       DID_3001
#define DTC_SNAPSHOT_LOCAL_DID_N2                       DID_300F
#define DTC_SNAPSHOT_LOCAL_DID_N3                       DID_3010
#define DTC_SNAPSHOT_LOCAL_DID_NUMBER                   3

#define DTC_P333217                                     0x333217
#define DTC_P333216                                     0x333216
#define DTC_U008988                                     0xC08988
#define DTC_U1E0288                                     0xDE0288
#define DTC_U104287                                     0xD04287
#define DTC_U029387                                     0xC29387
#define DTC_U104387                                     0xD04387
#define DTC_P333314                                     0x333314 
#define DTC_P333315                                     0x333315
#define DTC_P333414                                     0x333414
#define DTC_P333415                                     0x333415
#define DTC_P333514                                     0x333514
#define DTC_P333515                                     0x333515
#define DTC_P333601                                     0x333601
#define DTC_P333602                                     0x333602
#define DTC_P333603                                     0x333603
#define DTC_U1E0300                                     0xDE0300
#define DTC_P333722                                     0x333722
#define DTC_P33374B                                     0x33374B
#define DTC_P333721                                     0x333721
#define DTC_P333901                                     0x333901
#define DTC_P333814                                     0x333814
#define DTC_P333815                                     0x333815
#define DTC_P333902                                     0x333902


typedef uint08 (*dtc_fault_func)(void);
typedef enum _dtc_typ                           eDTC_TYP;
typedef struct _dtc_cfg                         stDTC_CFG;

enum _dtc_typ
{
    DTC_TYP_P333217 = 0, /* 蓄电池电压过高 */
    DTC_TYP_P333216, /* 蓄电池电压过低 */
    DTC_TYP_U008988, /* 热管理 CAN BusOff */
    DTC_TYP_U1E0288, /* TMS 私有 CAN BusOff */
    DTC_TYP_U104287, /* 空调面板节点丢失 */
    DTC_TYP_U029387, /* HCU 节点丢失 */
    DTC_TYP_U104387, /* 压缩机节点丢失 */
    DTC_TYP_P333314, /* 高压侧冷媒压力传感器对地短路 */
    DTC_TYP_P333315, /* 高压侧冷媒压力传感器对电源短路或开路 */
    DTC_TYP_P333414, /* 低压冷媒 PT 传感器压力端对地短路 */
    DTC_TYP_P333415, /* 低压冷媒 PT 传感器压力端对电源短路或开路 */
    DTC_TYP_P333514, /* 低压冷媒 PT 传感器温度对地短路 */
    DTC_TYP_P333515, /* 低压冷媒 PT 传感器温度对电源短路或开路 */
    DTC_TYP_P333601, /* Bat_EXV 初始化失败 */
    DTC_TYP_P333602, /* Bat_EXV 电气故障 */
    DTC_TYP_P333603, /* 压缩机本体故障 */
    DTC_TYP_U1E0300, /* Bat_EXV LIN 通信故障 */
    DTC_TYP_P333722, /* 压缩机排气压力过压 */
    DTC_TYP_P33374B, /* 压缩机排气温度过高 */
    DTC_TYP_P333721, /* 压缩机吸气压力过低 */
    DTC_TYP_P333901, /* 电子风扇故障 */
    DTC_TYP_P333814, /* 电池进水温度传感器对地短路 */
    DTC_TYP_P333815, /* 电池进水温度传感器对电源短路或开路 */
    DTC_TYP_P333902, /* 电池水泵故障 */
    

    CAN_DTC_NUM
};

struct _dtc_cfg
{
    uint32                  dtc;
    uint08                  confirmed_cycle_num;
    dtc_fault_func          fault_func;
};

extern const stDTC_CFG g_can_dtc_cfg[CAN_DTC_NUM];
#endif
