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
#include "user_funtion.h"
extern Ctrl_CMD control_cmd;

void dtc_user_process(void)
{
    eDTC_TYP typ = (eDTC_TYP)0;
    stDTC_SNAPSHOT snapshot_info;

    // snapshot_info.hv = 0;
    // snapshot_info.lv = 0;
    // snapshot_info.vehicle_spd = g_vehicle_speed_original;


    snapshot_info.runCmd = control_cmd.runCmd;
    snapshot_info.spdCmd = control_cmd.spdCmd_Rpm;
    snapshot_info.Max_power_acc = control_cmd.Max_power_acc;
    snapshot_info.BusCurrent = debug_info.BusCurrent;
    snapshot_info.PhaseCurrent = debug_info.PhaseCurrent;
    snapshot_info.Temp_COM_SIC = debug_info.Temp_COM_SIC;
    snapshot_info.speed_actal = debug_info.speed_actal;
    snapshot_info.fault_status_flag_motor = debug_info.fault_status_flag_motor.Word;
    snapshot_info.pVdcValue = debug_info.pVdcValue;
    snapshot_info.Temp_PCB = debug_info.Temp_PCB;
    snapshot_info.run_time = g_engine_running_time;
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

uint08 dtc_fault_b100014_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.faultStatus_PTC.bit.OcFault) // [5]   PTC_OcFault_HW
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_b100114_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.fault_status_flag_motor.bit.OvSoftRmsCurrent) // [5]   PTC_OcFault_HW
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_b100214_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.faultStatus_PTC.bit.TempSensorFault_IN)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_b100314_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.faultStatus_PTC.bit.TempSensorFault_OUT)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_b100614_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.faultStatus_PTC.bit.OvTemp_OUT)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_b100814_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.faultStatus_PTC.bit.OvTemp_IN)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_b100c14_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.faultStatus_PTC.bit.M_Resistance_Low_Fault)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_b101014_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.faultStatus_PTC.bit.M_ClntOtltOverTempProtn)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_b101114_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.faultStatus_PTC.bit.OcFault)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_b101214_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.faultStatus_PTC.bit.M_AD_Current)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_b101414_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.faultStatus_PTC.bit.HeatProtect)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_b101577_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.faultStatus_PTC.bit.OcFault)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_b101977_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.fault_status_flag_motor.bit.UvFault)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_u14d677_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.fault_status_flag_motor.bit.OvTemp)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_u14e077_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
    if(debug_info.faultStatus_PTC.bit.ERR_OUTCAN)
    {
        fault = DTC_FAULT_APPEAR;
    }
    else
    {
        fault = DTC_FAULT_DISAPPEAR;
    }
    return fault;
}

uint08 dtc_fault_u14e177_get(void)
{
    uint08 fault = DTC_FAULT_DISAPPEAR;
//    if(can0_user_busoff_cbk())
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
    g_vehicle_speed_original = 0xaa55;
}
