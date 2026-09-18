/*
 * m0_var.c
 *
 *  Created on: 2026��2��24��
 *      Author: hzldy
 */


#include "m0_var.h"
#include "m0_m4_ipc.h"
#include "user_funtion.h"

_ST_CAN_RX_MSG can_rx_data[CAN_ID_NUM];

uint8_t g_pin_low_voltage_overvoltage;//��ѹ��ѹ����ȡ��
uint8_t g_pin_low_voltage_undervoltage;//��ѹǷѹ����ȡ��

uint8_t g_pwm_thick_film;//��Ĥpwmռ�ձ�0-100

uint16_t g_ad_thick_film_current;//��Ĥ����
uint16_t g_ad_thick_film_temp;//��Ĥ�¶�
uint16_t g_ad_inlet_water_temp;//��ˮ�¶�
uint16_t g_ad_outlet_water_temp;//��ˮ�¶�
uint16_t g_ad_compressor_SIC_temp;//ѹ����SIC�¶�
uint16_t g_ad_bus_current;//ĸ�ߵ���(δȷ�ϣ�
uint16_t g_ad_thick_film_SIC_temp;//��ĤSIC�¶�
uint16_t g_ad_PCB_temp;//pcb�¶�
//uint16_t g_ad_vbat_sense;//ĸ�ߵ�ѹ��to m0)

uint8_t g_dtc_nvm_store;
uint32_t g_engine_running_time;


/***************GET********************/


uint16_t Bus_Voltage_Get(void)
{
  return m0_get_data.bus_voltage.value;
}
uint16_t Phase_Current_Get(void)
{
  return m0_get_data.phase_current.value;

}
uint16_t Motor_power_Q15_Get(void)
{
  return m0_get_data.motor_power.value;

}

/* ---- DEBUG: unpack filtered dq physical-Q values for calibration ----
 * 0x20F frame (after M4 改造):
 *   debug_VdVq: bits[15:0]=Vd_q8(V), bits[31:16]=Vq_q8(V)
 *   debug_IdIq: bits[15:0]=Id_q4(A), bits[31:16]=Iq_q4(A)
 *   Generated as int16; return raw to preserve sign semantics.
 */
uint16_t Debug_Vd_Q15_Get(void)
{
  return (uint16_t)(m0_get_data.debug_VdVq.value & 0xFFFFu);
}
uint16_t Debug_Vq_Q15_Get(void)
{
  return (uint16_t)(m0_get_data.debug_VdVq.value >> 16);
}
uint16_t Debug_Id_Q15_Get(void)
{
  return (uint16_t)(m0_get_data.debug_IdIq.value & 0xFFFFu);
}
uint16_t Debug_Iq_Q15_Get(void)
{
  return (uint16_t)(m0_get_data.debug_IdIq.value >> 16);
}

/* ---- DEBUG: modulation ratio & voltage utilization (0x210 frame) ----
 * debug_mod: bits[15:0]=mod_ratio_q15, bits[29:16]=v_util_pct×100, bit30=overmod
 */
uint16_t Mod_Ratio_Q15_Get(void)
{
  return (uint16_t)(m0_get_data.debug_mod.value & 0xFFFFu);
}
uint16_t Volt_Util_Pct_Get(void)
{
  return (uint16_t)((m0_get_data.debug_mod.value >> 16) & 0x3FFFu);
}
uint8_t  Overmod_Flag_Get(void)
{
  return (uint8_t)((m0_get_data.debug_mod.value >> 30) & 0x1u);
}

uint16_t Motor_Speed_Feedback_Q15_Get(void)
{
  /* The CM4 speed Q15 base is 10000 rpm. Keep this tied to the M0 command
   * ceiling because the M0 project does not include CM4 ConfigWizard headers. */
  return (uint16_t)(((uint32_t)m0_get_data.motor_speed.value * MAXIMUM_SPEED_RPM) / 32768u);

}
uint16_t Bus_Current_Get(void)
{
  return m0_get_data.bus_current.value;

}
int16_t Phase_Current_U_Get(void)
{
  
    return (int16_t)m0_get_data.phase_current_u.value;

}
int16_t Phase_Current_V_Get(void)
{
  
    return (int16_t)m0_get_data.phase_current_v.value;

}
int16_t Phase_Current_W_Get(void)
{
  
    return (int16_t)m0_get_data.phase_current_w.value;

}

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
//uint16_t Vbat_Sense_Get(void)//ĸ�ߵ�ѹ��to m0)
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
//void Power_Status_Set(uint8_t sts)//(���ػ���
//{
//  m0_set_data.power_status = sts;
//  Ipc_Pipe_Set();
//
//}

uint32_t Motor_Err_Status_Get(void)//(�������״̬��Ϣ)
{
  return m0_get_data.err_status.value;
  
}


uint16_t Motor_Running_Status_Get(void)//�����״̬���������������ػ������ϵȣ�
{
  return m0_get_data.running_status.value;

}



/***************SET********************/

void Motor_Err_Clear_Set(uint8_t motor_fault_type)//(����������)
{
  m0_set_data.err_clr.receive_flag = 1;
  m0_set_data.err_clr.value = motor_fault_type;
  Ipc_Pipe_Set();

}

void Motor_Control_Set(uint8_t enable_command,uint16_t speed,uint16_t power_Limiter) 
{
    m0_set_data.motor_control.receive_flag = 1;
    m0_set_data.motor_control.speed = speed;
    m0_set_data.motor_control.enable_command = enable_command;
    m0_set_data.motor_control.power_limiter = power_Limiter;
    Ipc_Pipe_Set();

}


void Acceleration_Time_Set(uint16_t AccTime)//(ת�ٴ�o���ٵ����ת�ٵ�ʱ��)
{
  m0_set_data.acctime.receive_flag = 1;
  m0_set_data.acctime.value = AccTime;
  Ipc_Pipe_Set();

}


void Reduction_Time_Set(uint16_t RedTime)//(ת�ٴ����ת�ٽ���l0��ʱ��)
{
  m0_set_data.redtime.receive_flag = 1;
  m0_set_data.redtime.value = RedTime;
  Ipc_Pipe_Set();
  

}
 
uint8_t IPMFAULT_STATE_Get(void)
{
  return (uint8_t)m0_get_data.ipm_fault_state.value;
}

void Motor_Fault_All_Set(uint32_t fault_flag_motor) 
{
  m0_set_data.Word.value = fault_flag_motor;
  m0_set_data.Word.receive_flag = 1;

    Ipc_Pipe_Set();
    
}
