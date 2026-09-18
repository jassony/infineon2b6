/*
 * m0_var.h
 *
 *  Created on: 2026年2月24日
 *      Author: hzldy
 */

#ifndef M0_BSW_SOURCE_LD_APP_INC_M0_VAR_H_
#define M0_BSW_SOURCE_LD_APP_INC_M0_VAR_H_


#include "cy_project.h"
#include "cy_device_headers.h"

/***********************CAN配置************************/

#define CAN_PRESCALER 10   //CAN频率40M，计算方式：40/（CAN_PRESCALER）/（CAN_TSEG1+CAN_TSEG2+1） = 500kbps
#define CAN_TSEG1 5
#define CAN_TSEG2 2       //采样点：（1+CAN_TSEG1）/（CAN_TSEG1+CAN_TSEG2+1） = 75

#define CANFD_PRESCALER 2 //CANFD频率40M，计算方式：40/（CAN_PRESCALER）/（CAN_TSEG1+CAN_TSEG2+1） = 2Mbps
#define CANFD_TSEG1 7
#define CANFD_TSEG2 2   //采样点：（1+CAN_TSEG1）/（CAN_TSEG1+CAN_TSEG2+1） = 80

///波特率采样点设置


#define TSET_STD_CAN_ID0        0x10
#define TSET_STD_CAN_ID1        0x20
///////////////////STD ID 配置

//#define TSET_STD_CAN_FID0       0x30
//#define TSET_STD_CAN_MASK0      0x7f0
//#define TSET_STD_CAN_FID1       0x40
//#define TSET_STD_CAN_MASK1      0x7f0
//
/////////////////////STD FILTER 配置



#define TSET_EXT_CAN_ID0                    0x10010
#define TSET_EXT_CAN_ID1                    0x10020
#define CAN_EXT_PHY_RX_ID                   0x18DAC3F1
#define CAN_EXT_FUNC_RX_ID                  0x18DB33F1
//#define CAN_EXT_PHY_TX_ID                   0x18DAF1C3
///////////////////EXT ID 配置

//#define TSET_EXT_CAN_FID0       0x10030
//#define TSET_EXT_CAN_MASK0      0x1ffffff0
//#define TSET_EXT_CAN_FID1       0x10040
//#define TSET_EXT_CAN_MASK1      0x1ffffff0
//
/////////////////////EXT FILTER 配置

#define BUSOFF_FAST_RELOAD_TIME 100
#define BUSOFF_FAST_TICK 5

#define BUSOFF_SLOW_RELOAD_TIME 1000
#define BUSOFF_SLOW_TICK 0 //0表示持续

/////////////////////busoff配置

typedef enum
{
  TEST_STD_CAN_INDEX0,
  TEST_STD_CAN_INDEX1,
  STD_CAN_ID_END,
  TEST_EXT_CAN_INDEX0 = STD_CAN_ID_END,
  TEST_EXT_CAN_INDEX1 , 
  CAN_EXT_PHY_RX_INDEX2,
  CAN_EXT_FUNC_RX_INDEX3,
  EXT_CAN_ID_END,
  CAN_ID_NUM = EXT_CAN_ID_END
}_EM_CAN_ID_INDEX; 

typedef struct
{
  bool                    rx_flag;
  bool                    canFDFormat;
  bool                    extended;
  uint32_t                id;
  uint8_t                 data[64];
  uint16_t                datalen;
  
  
}_ST_CAN_RX_MSG;



//extern uint8_t g_pin_low_voltage_overvoltage;//低压过压（获取）
//extern uint8_t g_pin_low_voltage_undervoltage;//低压欠压（获取）
//#define LED(x)                  Cy_GPIO_Write(LED_PORT, LED_PIN, x)//LED状态灯（0为低，1为高）
//#define SLEEP(x)                Cy_GPIO_Write(GPIO_SLEEP_OD_OUT_PROT, GPIO_SLEEP_OD_OUT_PIN, x)
//#define MAIN_SWITCH_ENABLE(x)   Cy_GPIO_Write(GPIO_MAIN_SWITCH_OD_OUT_PORT, GPIO_MAIN_SWITCH_OD_OUT_PIN, x)//-总正开关（0为低，1为高）



extern _ST_CAN_RX_MSG can_rx_data[CAN_ID_NUM];

//
//extern uint8_t g_pwm_thick_film;//厚膜pwm占空比0-100
//
//extern uint16_t g_ad_thick_film_current;//厚膜电流
//extern uint16_t g_ad_thick_film_temp;//厚膜温度
//extern uint16_t g_ad_inlet_water_temp;//进水温度
//extern uint16_t g_ad_outlet_water_temp;//出水温度
//extern uint16_t g_ad_compressor_SIC_temp;//压缩机SIC温度
//extern uint16_t g_ad_bus_current;//母线电流(未确认）
//extern uint16_t g_ad_thick_film_SIC_temp;//厚膜SIC温度
//extern uint16_t g_ad_PCB_temp;//pcb温度
////extern uint16_t g_ad_vbat_sense;
//
//
//uint16_t Bus_Voltage_Get(void);
//uint16_t Phase_Current_Get(void);
//uint16_t Motor_Speed_Get(void);
//uint16_t Bus_Current_Get(void);
//uint16_t Motor_Err_Status_Get(void);//(电机故障状态信息)
//uint16_t Thick_Film_Err_Status_Get(void);//（厚膜故障状态信息）

//uint16_t Err_Status_Get(void);
//uint16_t Running_Status_Get(void);
//uint16_t Vbat_Sense_Get(void);
//void Expect_Speed_Set(uint16_t speed) ;
//void Power_Status_Set(uint8_t sts);//(开关机）
//void Acceleration_Time_Set(uint16_t AccTime);//(转速从o加速到最大转速的时间)
//void Reduction_Time_Set(uint16_t AccTime);//(转速从最大转速降到l0的时间)
//void Motor_Control_Set(uint8_t enable_command,uint16_t speed,uint16_t power_Limiter) ;
//void Motor_Err_Clear_Set(uint8_t motor_fault_type);//(电机故障清除)

#endif /* M0_BSW_SOURCE_LD_APP_INC_M0_VAR_H_ */
