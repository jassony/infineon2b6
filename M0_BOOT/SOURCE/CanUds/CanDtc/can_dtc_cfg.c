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
        .dtc                        = DTC_P333217,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333217_get
    },
    {
        .dtc                        = DTC_P333216,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333216_get
    },
    {
        .dtc                        = DTC_U008988,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_u008988_get
    },
    {
        .dtc                        = DTC_U1E0288,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_u1e0288_get
    },
    {
        .dtc                        = DTC_U104287,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_u104287_get
    },
    {
        .dtc                        = DTC_U029387,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_u029387_get
    },
    {
        .dtc                        = DTC_U104387,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_u104387_get
    },
    {
        .dtc                        = DTC_P333314,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333314_get
    },
    {
        .dtc                        = DTC_P333315,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333315_get
    },
    {
        .dtc                        = DTC_P333414,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333414_get
    },
    {
        .dtc                        = DTC_P333415,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333415_get
    },
    {
        .dtc                        = DTC_P333514,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333514_get
    },
    {
        .dtc                        = DTC_P333515,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333515_get
    },
    {
        .dtc                        = DTC_P333601,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333601_get
    },
    {
        .dtc                        = DTC_P333602,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333602_get
    },
    {
        .dtc                        = DTC_P333603,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333603_get
    },
    {
        .dtc                        = DTC_U1E0300,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_u1e0300_get
    },
    {
        .dtc                        = DTC_P333722,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333722_get
    },
    {
        .dtc                        = DTC_P33374B,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p33374b_get
    },
    {
        .dtc                        = DTC_P333721,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333721_get
    },
    {
        .dtc                        = DTC_P333901,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333901_get
    },
    {
        .dtc                        = DTC_P333814,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333814_get
    },
    {
        .dtc                        = DTC_P333815,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333815_get
    },
    {
        .dtc                        = DTC_P333902,
        .confirmed_cycle_num        = 1,
        .fault_func                 = dtc_fault_p333902_get
    },
};



