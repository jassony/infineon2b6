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
#define DTC_SNAPSHOT_ALL_RECORD_NUMBER                  0xFF
#define DTC_SNAPSHOT_INVALID_RECORD_NUMBER              0xFF
#define DTC_SNAPSHOT_GLOBAL_DID_N1                      DID_5011
#define DTC_SNAPSHOT_GLOBAL_DID_N2                      DID_5012
#define DTC_SNAPSHOT_GLOBAL_DID_N3                      DID_5013
#define DTC_SNAPSHOT_GLOBAL_DID_N4                      DID_5014
#define DTC_SNAPSHOT_GLOBAL_DID_N5                      DID_5015
#define DTC_SNAPSHOT_GLOBAL_DID_N6                      DID_5016
#define DTC_SNAPSHOT_GLOBAL_DID_N7                      DID_5017
#define DTC_SNAPSHOT_GLOBAL_DID_N8                      DID_5018
#define DTC_SNAPSHOT_GLOBAL_DID_N9                      DID_5019
#define DTC_SNAPSHOT_GLOBAL_DID_N10                     DID_501A
#define DTC_SNAPSHOT_GLOBAL_DID_N11                     DID_501B
#define DTC_SNAPSHOT_GLOBAL_DID_NUMBER                  11
#define DTC_SNAPSHOT_LOCAL_DID_N1                      DID_5011
#define DTC_SNAPSHOT_LOCAL_DID_N2                      DID_5012
#define DTC_SNAPSHOT_LOCAL_DID_N3                      DID_5013
#define DTC_SNAPSHOT_LOCAL_DID_N4                      DID_5014
#define DTC_SNAPSHOT_LOCAL_DID_N5                      DID_5015
#define DTC_SNAPSHOT_LOCAL_DID_N6                      DID_5016
#define DTC_SNAPSHOT_LOCAL_DID_N7                      DID_5017
#define DTC_SNAPSHOT_LOCAL_DID_N8                      DID_5018
#define DTC_SNAPSHOT_LOCAL_DID_N9                      DID_5019
#define DTC_SNAPSHOT_LOCAL_DID_N10                     DID_501A
#define DTC_SNAPSHOT_LOCAL_DID_N11                     DID_501B
#define DTC_SNAPSHOT_LOCAL_DID_NUMBER                   11

#define DTC_B100014	                                0x900014
#define DTC_B100114	                                0x900114
#define DTC_B100214	                                0x900214
#define DTC_B100314	                                0x900314
#define DTC_B100614	                                0x900614
#define DTC_B100814	                                0x900814
#define DTC_B100C14	                                0x900C14
#define DTC_B101014	                                0x901014
#define DTC_B101114	                                0x901114
#define DTC_B101214	                                0x901214
#define DTC_B101414	                                0x901414
#define DTC_B101577	                                0x901577
#define DTC_B101977	                                0x901977
#define DTC_U14D677	                                0xD4D677
#define DTC_U14E077	                                0xD4E077
// #define DTC_U14E177	                                0xD4E177

#define DTC_FAULT_CLR_LAST_TIME                         300U /* 300--3000ms */                           


typedef uint08 (*dtc_fault_func)(void);
typedef enum _dtc_typ                           eDTC_TYP;
typedef struct _dtc_cfg                         stDTC_CFG;

enum _dtc_typ
{
    DTC_TYP_B100014 = 0,//硬件过流
    DTC_TYP_B100114,//软件过流
    DTC_TYP_B100214,//进水温度传感器故障
    DTC_TYP_B100314,//出水温度传感器故障
    DTC_TYP_B100614,//出水口温度保护故障
    DTC_TYP_B100814,//进水口温度保护故障
    DTC_TYP_B100C14,//TCR故障
    DTC_TYP_B101014,//进出水温差保护故障
    DTC_TYP_B101114,//过流保护故障
    DTC_TYP_B101214,//电流ad采样故障保护
    DTC_TYP_B101414,//漏电流保护
    DTC_TYP_B101577,//过压保护故障
    DTC_TYP_B101977,//欠压保护故障
    DTC_TYP_U14D677,//通讯丢失保护故障
    DTC_TYP_U14E077,//通讯丢失保护故障
    // DTC_TYP_U14E177,//CAN Busoff故障
    CAN_DTC_NUM
};








struct _dtc_cfg
{
    eDTC_TYP                typ;
    uint32                  dtc;
    uint08                  confirmed_cycle_num;
    uint08                  aging_num;
    dtc_fault_func          fault_func;
//    dtc_condition_func      condition_func;
};
extern const stDTC_CFG g_can_dtc_cfg[CAN_DTC_NUM];
#endif
