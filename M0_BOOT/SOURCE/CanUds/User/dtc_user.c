/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : dtc_user.c
* Author        : yangming
* Date          : 2024-05-07
* Version       : 1.00
* Description   : 
* Others        : None
*
****************************************************************************************************/
#include "dtc_user.h"
#include "can_dtc.h"
#include "can_user.h"

void dtc_user_process(void)
{
    eDTC_TYP typ = (eDTC_TYP)0;
    stDTC_SNAPSHOT snapshot_info;

    snapshot_info.hv = 0;
    snapshot_info.lv = 0;
    snapshot_info.vehicle_spd = g_vehicle_spped_original;
    for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
    {
        dtc_cur_snapshot_get(typ, snapshot_info);
    }
}

uint08 dtc_fault_p333217_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333217)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333216_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333216)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_u008988_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_U008988)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_u1e0288_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_U1E0288)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_u104287_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_U104287)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_u029387_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_U029387)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_u104387_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_U104387)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333314_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333314)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333315_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333315)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333414_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333414)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333415_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333415)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333514_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333514)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333515_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333515)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333601_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333601)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333602_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333602)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333603_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333603)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_u1e0300_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_U1E0300)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333722_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333722)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p33374b_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P33374B)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333721_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333721)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333901_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333901)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333814_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333814)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333815_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;

//    if (rtY.BSW_DTC_P333815)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 dtc_fault_p333902_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
//
//    if (rtY.BSW_DTC_P333902)
//    {
//        fault = DTC_FAULT_APPEAR;
//    }
//    else
//    {
//        fault = DTC_FAULT_DISAPPEAR;
//    }
    return fault;
}

uint08 t_dtc_fault;
void dtc_user_test(void)
{
//    rtY.BSW_DTC_P333217 = 1;
//    rtY.BSW_DTC_P333216 = 1;
//    rtY.BSW_DTC_U008988 = t_dtc_fault;
//    rtY.BSW_DTC_U1E0288 = t_dtc_fault;
//    rtY.BSW_DTC_P333722 = t_dtc_fault;
//    rtY.BSW_DTC_P33374B = t_dtc_fault;
//    rtY.BSW_DTC_P333721 = t_dtc_fault;
//    rtY.BSW_DTC_P333902 = 1;
    g_vehicle_spped_original = 0xaa55;
}

