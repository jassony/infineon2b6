/*
 * m0_var.h
 *
 *  Created on: 2026��2��24��
 *      Author: hzldy
 */

#ifndef M0_BSW_SOURCE_LD_APP_INC_M0_VAR_H_
#define M0_BSW_SOURCE_LD_APP_INC_M0_VAR_H_


#include "cy_project.h"
#include "cy_device_headers.h"
#include "user_funtion.h"
/***********************CAN����************************/

#define CAN_PRESCALER 10   //CANƵ��40M�����㷽ʽ��40/��CAN_PRESCALER��/��CAN_TSEG1+CAN_TSEG2+1�� = 500kbps
#define CAN_TSEG1 5
#define CAN_TSEG2 2       //�����㣺��1+CAN_TSEG1��/��CAN_TSEG1+CAN_TSEG2+1�� = 75

#define CANFD_PRESCALER 2 //CANFDƵ��40M�����㷽ʽ��40/��CAN_PRESCALER��/��CAN_TSEG1+CAN_TSEG2+1�� = 2Mbps
#define CANFD_TSEG1 7
#define CANFD_TSEG2 2   //�����㣺��1+CAN_TSEG1��/��CAN_TSEG1+CAN_TSEG2+1�� = 80

///�����ʲ���������


#define TSET_STD_CAN_ID0        0x20a
#define TSET_STD_CAN_ID1        0x20
///////////////////STD ID ����

//#define TSET_STD_CAN_FID0       0x30
//#define TSET_STD_CAN_MASK0      0x7f0
//#define TSET_STD_CAN_FID1       0x40
//#define TSET_STD_CAN_MASK1      0x7f0
//
/////////////////////STD FILTER ����



#define TSET_EXT_CAN_ID0        0x10010
#define TSET_EXT_CAN_ID1        0x10020
#define CAN_EXT_PHY_RX_ID                   0x18DAC3F1
#define CAN_EXT_FUNC_RX_ID                  0x18DB33F1

///////////////////EXT ID ����

//#define TSET_EXT_CAN_FID0       0x10030
//#define TSET_EXT_CAN_MASK0      0x1ffffff0
//#define TSET_EXT_CAN_FID1       0x10040
//#define TSET_EXT_CAN_MASK1      0x1ffffff0
//
/////////////////////EXT FILTER ����
#define BUSOFF_SEND_ID                  0x666

#define BUSOFF_FAST_RELOAD_TIME         100
#define BUSOFF_FAST_TICK                5

#define BUSOFF_SLOW_RELOAD_TIME         1000
#define BUSOFF_SLOW_TICK                0 //0��ʾ����

/////////////////////busoff����

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




extern uint8_t g_pin_low_voltage_overvoltage;//��ѹ��ѹ����ȡ��
extern uint8_t g_pin_low_voltage_undervoltage;//��ѹǷѹ����ȡ��
#define LED(x)                  Cy_GPIO_Write(LED_PORT, LED_PIN, x)//LED״̬�ƣ�0Ϊ�ͣ�1Ϊ�ߣ�
#define SLEEP(x)                Cy_GPIO_Write(GPIO_SLEEP_OD_OUT_PROT, GPIO_SLEEP_OD_OUT_PIN, x)
#define MAIN_SWITCH_ENABLE(x)   Cy_GPIO_Write(GPIO_MAIN_SWITCH_OD_OUT_PORT, GPIO_MAIN_SWITCH_OD_OUT_PIN, x)//-�������أ�0Ϊ�ͣ�1Ϊ�ߣ�
//#define THICK_PWM(x)            Cy_GPIO_Write(TCPWM_THICK_FILM_OD_PROT, TCPWM_THICK_FILM_OD_PIN, x);Adc_Fast_Process();g_ad_thick_film_current = Adc_Original_Val_Get(ADC_HEAT_M);




extern _ST_CAN_RX_MSG can_rx_data[CAN_ID_NUM];


extern uint8_t g_pwm_thick_film;//��Ĥpwmռ�ձ�0-100

extern uint16_t g_ad_thick_film_current;//��Ĥ����
extern uint16_t g_ad_thick_film_temp;//��Ĥ�¶�
extern uint16_t g_ad_inlet_water_temp;//��ˮ�¶�
extern uint16_t g_ad_outlet_water_temp;//��ˮ�¶�
extern uint16_t g_ad_compressor_SIC_temp;//ѹ����SIC�¶�
extern uint16_t g_ad_bus_current;//ĸ�ߵ���(δȷ�ϣ�
extern uint16_t g_ad_thick_film_SIC_temp;//��ĤSIC�¶�
extern uint16_t g_ad_PCB_temp;//pcb�¶�
//extern uint16_t g_ad_vbat_sense;

extern uint8_t g_dtc_nvm_store;
extern uint32_t g_engine_running_time;


uint16_t Bus_Voltage_Get(void);
uint16_t Phase_Current_Get(void);
uint16_t Motor_Speed_Feedback_Q15_Get(void);
uint16_t Bus_Current_Get(void);
uint32_t Motor_Err_Status_Get(void);//(�������״̬��Ϣ)
uint16_t Thick_Film_Err_Status_Get(void);//����Ĥ����״̬��Ϣ��
int16_t Phase_Current_U_Get(void);
int16_t Phase_Current_V_Get(void);
int16_t Phase_Current_W_Get(void);

/* ---- DEBUG: raw dq data for power model calibration ---- */
uint16_t Debug_Vd_Q15_Get(void);
uint16_t Debug_Vq_Q15_Get(void);
uint16_t Debug_Id_Q15_Get(void);
uint16_t Debug_Iq_Q15_Get(void);

/* ---- DEBUG: modulation ratio & voltage utilization (0x210 frame) ---- */
uint16_t Mod_Ratio_Q15_Get(void);   /* |Vref|/Vdc, Q15 */
uint16_t Volt_Util_Pct_Get(void);   /* 0.01%/LSB, 100% = linear-modulation limit */
uint8_t  Overmod_Flag_Get(void);     /* SVPWM overmodulation flag */


//uint16_t Err_Status_Get(void);
//uint16_t Running_Status_Get(void);
//uint16_t Vbat_Sense_Get(void);
//void Expect_Speed_Set(uint16_t speed) ;
//void Power_Status_Set(uint8_t sts);//(���ػ���
void Acceleration_Time_Set(uint16_t AccTime);//(ת�ٴ�o���ٵ����ת�ٵ�ʱ��)
void Reduction_Time_Set(uint16_t AccTime);//(ת�ٴ����ת�ٽ���l0��ʱ��)
void Motor_Control_Set(uint8_t enable_command,uint16_t speed,uint16_t power_Limiter) ;
void Motor_Err_Clear_Set(uint8_t motor_fault_type);//(����������)
uint8_t IPMFAULT_STATE_Get(void);
void Motor_Fault_All_Set(uint32_t fault_flag_motor);
#endif /* M0_BSW_SOURCE_LD_APP_INC_M0_VAR_H_ */
