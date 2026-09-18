/*
 * m0_var.c
 *
 *  Created on: 2026年2月24日
 *      Author: hzldy
 */


#include "m0_boot_var.h"
//#include "m0_m4_ipc.h"

_ST_CAN_RX_MSG can_rx_data[CAN_ID_NUM];

//uint8_t g_pin_low_voltage_overvoltage;//低压过压（获取）
//uint8_t g_pin_low_voltage_undervoltage;//低压欠压（获取）
//
//uint8_t g_pwm_thick_film;//厚膜pwm占空比0-100
//
//uint16_t g_ad_thick_film_current;//厚膜电流
//uint16_t g_ad_thick_film_temp;//厚膜温度
//uint16_t g_ad_inlet_water_temp;//进水温度
//uint16_t g_ad_outlet_water_temp;//出水温度
//uint16_t g_ad_compressor_SIC_temp;//压缩机SIC温度
//uint16_t g_ad_bus_current;//母线电流(未确认）
//uint16_t g_ad_thick_film_SIC_temp;//厚膜SIC温度
//uint16_t g_ad_PCB_temp;//pcb温度
////uint16_t g_ad_vbat_sense;//母线电压（to m0)


/***************GET********************/

//
//uint16_t Bus_Voltage_Get(void)
//{
//  return m0_get_data.bus_voltage.value;
//}
//uint16_t Phase_Current_Get(void)
//{
//  return m0_get_data.phase_current.value;
//
//}
//uint16_t Motor_Speed_Get(void)
//{
//  return m0_get_data.motor_speed.value;
//
//}
//uint16_t Bus_Current_Get(void)
//{
//  return m0_get_data.bus_current.value;
//
//}

//uint16_t Err_Status_Get(void)
//{
//  return m0_get_data.err_status;
//
//}
//uint16_t Running_Status_Get(void)
//{
//  return m0_get_data.running_status;
//
//}
//
//uint16_t Vbat_Sense_Get(void)//母线电压（to m0)
//{
//  return m0_get_data.g_ad_vbat_sense;
//
//}

//void Expect_Speed_Set(uint16_t speed) 
//{
//  m0_set_data.expect_speed = speed;
//  Ipc_Pipe_Set();
//}
//
//void Power_Status_Set(uint8_t sts)//(开关机）
//{
//  m0_set_data.power_status = sts;
//  Ipc_Pipe_Set();
//
//}
//
//uint16_t Motor_Err_Status_Get(void)//(电机故障状态信息)
//{
//  return m0_get_data.err_status.value;
//  
//}
//
//
//uint16_t Motor_Running_Status_Get(void)//（电机状态：待机、开机、关机、故障等）
//{
//  return m0_get_data.running_status.value;
//
//}
//
//
//
///***************SET********************/
//
//void Motor_Err_Clear_Set(uint8_t motor_fault_type)//(电机故障清除)
//{
//  m0_set_data.err_clr.receive_flag = 1;
//  m0_set_data.err_clr.value = motor_fault_type;
//  Ipc_Pipe_Set();
//
//}
//
//void Motor_Control_Set(uint8_t enable_command,uint16_t speed,uint16_t power_Limiter) 
//{
//    m0_set_data.motor_control.receive_flag = 1;
//    m0_set_data.motor_control.speed = speed;
//    m0_set_data.motor_control.enable_command = enable_command;
//    m0_set_data.motor_control.power_limiter = power_Limiter;
//    Ipc_Pipe_Set();
//
//}
//
//
//void Acceleration_Time_Set(uint16_t AccTime)//(转速从o加速到最大转速的时间)
//{
//  m0_set_data.acctime.receive_flag = 1;
//  m0_set_data.acctime.value = AccTime;
//  Ipc_Pipe_Set();
//
//}
//
//
//void Reduction_Time_Set(uint16_t RedTime)//(转速从最大转速降到l0的时间)
//{
//  m0_set_data.redtime.receive_flag = 1;
//  m0_set_data.redtime.value = RedTime;
//  Ipc_Pipe_Set();
//
//}

