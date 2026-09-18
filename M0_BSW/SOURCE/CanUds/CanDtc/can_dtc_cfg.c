/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_dtc_cfg.c
* Author        : yangming
* Date          : 2024-04-28
* Version       : 1.00
* Description   : Can dtc.
* Others        : None
*
****************************************************************************************************/
#include "can_dtc_cfg.h"
#include "can_dtc_cbk_cfg.h"
   
const stDTC_CFG g_can_dtc_cfg[CAN_DTC_NUM] =
{
    {
        .dtc                        = DTC_B100014,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b100014_get
    },
    {
        .dtc                        = DTC_B100114,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b100114_get
    },
    {
        .dtc                        = DTC_B100214,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b100214_get
    },
    {
        .dtc                        = DTC_B100314,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b100314_get
    },
    {
        .dtc                        = DTC_B100614,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b100614_get
    },
    {
        .dtc                        = DTC_B100814,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b100814_get
    },
    {
        .dtc                        = DTC_B100C14,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b100c14_get
    },
    {
        .dtc                        = DTC_B101014,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b101014_get
    },
    {
        .dtc                        = DTC_B101114,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b101114_get
    },
    {
        .dtc                        = DTC_B101214,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b101214_get
    },
    {
        .dtc                        = DTC_B101414,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b101414_get
    },
    {
        .dtc                        = DTC_B101577,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b101577_get
    },
    {
        .dtc                        = DTC_B101977,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_b101977_get
    },
    {
        .dtc                        = DTC_U14D677,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_u14d677_get
    },
    {
        .dtc                        = DTC_U14E077,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_u14e077_get
    },
    // {
    //     .dtc                        = DTC_U14E177,
    //     .confirmed_cycle_num        = 1,
    //     .fault_func                 = dtc_fault_u14e177_get
    // },
};



