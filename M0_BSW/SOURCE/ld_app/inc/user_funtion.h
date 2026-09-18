#ifndef user_funtion_h
#define user_funtion_h
#include <stdbool.h>
#include <stddef.h>
#include "string.h"
#include <math.h>
#include <stdint.h>
#include "ptc_duty_control.h"
#include "m0_var.h"
#include "m0_canfd_cfg.h"

/*.....COM_Motor.....*/
#define MAXIMUM_SPEED_RPM   10000 //����?��
#define flag_downspeed_by_AC  0//���ƹ��ʱ�ʶλ��0 = ������  1 = ����
#define STOP_SPEED_MIN  5000    //ͣ����Сת��
#define STOP_DELAY_TIMER  3000   //ͣ��������?3s
extern volatile uint16_t Cal_DiagMotStartMinSpd_rpm_u16;
extern volatile uint32_t Cal_DiagOcRstCntClrDelay_ms_u32;
extern volatile uint16_t Cal_DiagOcAutoRecDelay_ms_u16;
extern volatile uint16_t Cal_DiagOcAutoRecLimit_cnt_u16;
/*.....WPTC & TEMPLE.....*/
#define Resolu  131072  //Resolution���ֱ��ʣ�131072 = ��2 �� 17 �η���
#define VREF    3.318   //Voltage Reference���ο���ѹ�����¶Ȳ�����׼�ο���ѹ
#define TEMP_SCALE  1.214  // Ԥ���㣺(3*8*2*1000*VREF)/Resolu = 48000*3.318/131072   =1.2151031494140625
extern volatile uint16_t Cal_DiagPtcOcFltCfmDelay_ms_u16;
extern volatile uint16_t Cal_DiagPtcHtProtCfmDelay_ms_u16;
extern volatile uint16_t Cal_DiagPtcAdCurFltCfmDelay_ms_u16;
extern volatile uint16_t Cal_DiagTmpFltCfmDelay_ms_u16;
extern volatile uint16_t Cal_DiagPtcResFltCfmDelay_ms_u16;
extern volatile uint16_t Cal_DiagTmpFltRecDelay_ms_u16;
extern volatile uint16_t Cal_DiagPcbSicTmpTrip_degC_u16;
extern volatile uint16_t Cal_DiagPcbSicTmpHys_degC_u16;
extern volatile uint16_t Cal_DiagTfTmpTrip_degC_u16;
extern volatile uint16_t Cal_DiagTfTmpHys_degC_u16;
extern volatile uint16_t Cal_DiagInTmpTrip_degC_u16;
extern volatile uint16_t Cal_DiagInTmpHys_degC_u16;
extern volatile uint16_t Cal_DiagOutTmpTrip_degC_u16;
extern volatile uint16_t Cal_DiagOutTmpPwrLim_degC_u16;
extern volatile uint16_t Cal_DiagOutTmpHys_degC_u16;
extern volatile uint16_t Cal_DiagTmpSensMax_adc_u16;
extern volatile uint16_t Cal_DiagTmpSensMin_adc_u16;
extern volatile uint16_t Cal_DiagPtcPwrMax_pct_u16;
extern volatile uint16_t Cal_DiagPtcPwrMin_pct_u16;
extern volatile uint16_t Cal_DiagPtcOcCurTrip_dA_u16;
extern volatile uint16_t Cal_DiagPtcAdCurMin_adc_u16;
extern volatile uint16_t Cal_DiagPtcAdCurMax_adc_u16;
extern volatile uint16_t Cal_DiagPtcHtProtCur_dA_u16;
extern volatile uint16_t Cal_DiagPtcResHi_ohm_u16;
extern volatile uint16_t Cal_DiagPtcResLo_ohm_u16;
extern volatile uint16_t Cal_DiagTfTmpAutoRecLimit_cnt_u16;
extern volatile uint16_t Cal_DiagDryBrnAutoRecLimit_cnt_u16;

/*.....CAN.....*/
extern volatile uint8_t Cal_DiagCanCommLostMonEn_u8;
extern volatile uint16_t Cal_DiagCanCommLostTout_ms_u16;
#define CanRxID 0X20A    //����ID 
/*.....Voltage.....*/
extern volatile uint16_t Cal_DiagDcBusOvCfmDelay_ms_u16;
extern volatile uint16_t Cal_DiagDcBusUvCfmDelay_ms_u16;
extern volatile uint16_t Cal_DiagDcBusUvQkCfmDelay_ms_u16;
extern volatile uint16_t Cal_DiagDcBusFltRecDelay_ms_u16;
extern volatile uint16_t Cal_DiagDcBusOvTrip_V_u16;
extern volatile uint16_t Cal_DiagDcBusOvRec_V_u16;
extern volatile uint16_t Cal_DiagDcBusUvRec_V_u16;
extern volatile uint16_t Cal_DiagDcBusUvDnSpd_rpm_u16;
extern volatile uint16_t Cal_DiagDcBusUvTrip_V_u16;
extern volatile uint16_t Cal_DiagDcBusUvQkTrip_V_u16;
extern volatile uint16_t Cal_DiagPhCurTrip_A_u16;
extern volatile uint16_t Cal_DiagPhCurCfmDelay_cnt_u16;
extern volatile uint16_t Cal_DiagPhPkCurTrip_A_u16;
extern volatile uint16_t Cal_DiagPhPkCurCfmDelay_cnt_u16;
extern volatile uint16_t Cal_DiagBusCurTrip_dA_u16;
extern volatile uint16_t Cal_DiagBusCurCfmDelay_cnt_u16;
extern volatile uint8_t Cal_DiagPhCurRmsEn_u8;
extern volatile uint8_t Cal_DiagPhPkCurEn_u8;
extern volatile uint8_t Cal_DiagOvldEn_u8;
extern volatile uint8_t Cal_DiagPwrOvEn_u8;
extern volatile uint8_t Cal_DiagPwrOvDnSpdEn_u8;
/*.....COM Over-Power Fault.....*/
extern volatile uint16_t Cal_DiagPwrOvDef_W_u16;
extern volatile uint16_t Cal_DiagPwrOvHys_W_u16;
extern volatile uint16_t Cal_DiagPwrOvFltCfm_cnt_u16;
extern volatile uint16_t Cal_DiagPwrOvAutoRecLimit_cnt_u16;
extern volatile uint16_t Cal_DiagPwrOvStepDn_rpmPer100ms_u16;
extern volatile uint16_t Cal_DiagPwrOvStepUp_rpmPer100ms_u16;
extern volatile uint16_t Cal_DiagPwrOvSpdDirLock_cnt_u16;
extern volatile uint16_t Cal_DiagPwrOvZ0ProbeCfm_cnt_u16;
extern volatile uint16_t Cal_DiagPwrOvMinSpd_rpm_u16;
extern volatile uint16_t Cal_DiagPwrOvFltRstrtDelay_cnt_u16;
extern volatile uint16_t Cal_DiagPwrOvFltRstDelay_cnt_u16;
extern volatile uint16_t Cal_DiagOvldSpdThd_rpm_u16;
extern volatile uint16_t Cal_DiagOvldPhCurThd_A_u16;
extern volatile uint16_t Cal_DiagOvldCfmDelay_cnt_u16;

typedef union {
    struct
    {
        unsigned StallDetected : 1;	// [0] ��ת����־λ, 0: �½��أ����������? ,  1: �����أ����ϴ�����      0: Falling edge  1: Rising edge 
        unsigned OcFault : 1; // [1] Ӳ���������ϱ�־λ
        unsigned OvFault : 1;//[2] ��ѹ���ϱ�־λ  
        unsigned UvFault : 1;// [3] Ƿѹ���ϱ�־λ
        unsigned OvLoadFault : 1; // [4] �ű��������ϱ�־λ������������쳣��? inputencoder
        unsigned OvTemp : 1;// [5] ���¹��ϱ�־λ
        unsigned OffsetError : 1;// [6] ƫ������־λ����������ƫ���쳣��
        unsigned OvSoftRmsCurrent : 1;// [7] ����RMS������־λ
        unsigned OvSoftwarePeakCurrent : 1;// [8] ������ֵ������־λ
        unsigned OvBusCurrent : 1;// [9] ���߹�����־λ
        unsigned CurrentUnbalance : 1;// [10] ������ƽ���־λ��������������?
        unsigned ERR_OUTCAN:1;// [11]ͨѶ����
        unsigned speedFault:1; //[12]ʧ��
        unsigned BusOff:1; //[13]can busoff
        unsigned OvFaultLow:1; //[14]��ѹ��ѹ
        unsigned UvFaultLow:1; //[15]��ѹǷѹ
        unsigned PowerOverFault:1; // [16]
        unsigned unused : 15; // [17-31]
    }bit;
    uint32_t Word;
} FAULT_STATUS_FLAG_COM;

typedef union {
    struct
    {
        unsigned DryBurn : 1;	//0 NO USE
        unsigned OcFault : 1; //1 [1] �������ϱ�־λ
        unsigned OvTemp_M: 1;//2
        unsigned OvTemp_IN: 1;//3
        unsigned OvTemp_OUT: 1;//4
        unsigned TempSensorFault_M: 1;//5
        unsigned TempSensorFault_IN: 1;//6
        unsigned TempSensorFault_OUT: 1;//7
        unsigned M_Resistance_Low_Fault : 1; //8
        unsigned M_Resistance_High_Fault : 1; //9
        unsigned M_ClntOtltOverTempProtn : 1;  //10 ����������ˮ�²����?
        unsigned ERR_OUTCAN:1;  //11 ͨѶ����
        unsigned M_AD_Current:1; //12 ����AD��������  ����������ƫ��
        unsigned HeatProtect:1 ; //13 PWM���ʹ��쳣��ͨ��©����   δ���������е���>7A
        unsigned unused : 2; // [14-15] ����δʹ�õ�λ
    }bit;
    uint32_t Word;
} FAULT_STATUS_FLAG_PTC;

typedef struct
{
   bool    OvTemp_flag,
            OvTemp_M_flag,
            OvTemp_IN_flag,
            OvTemp_OUT_flag,
            OCS_Fault_Lock_flag,//����������־
            PTC_Running_flag,
            PTC_STOP,
            toggleFlag,
            Communication_lost_Flag,
            PTC_POWER_REDUCTION;
   uint8_t OvTemp_Lock_Count; //Ĥ�¶�+��ˮ�¶�+��ˮ�¶ȱ��������ۼ�ֵ
    
  uint32_t PtcCurrents,
           PTC_RealPower, 
           PTC_Resistance,
           PtcPeakCurrents,
           PtcPeakCurrents_fast;
  uint16_t  pVdcValue,
            PTC_Power,
            DutyPoint,
            pIbus,
            IphaseAvg,
            speed_actal,
            COM_Power,
            BusCurrent,
            PhaseCurrent,
            PhasePeakCurrent,
            MotorState,
            Cal_CanCommLostTimeout_ms_u16_Count,
            Temp_PTC_SIC,
            Temp_COM_SIC,
            Temp_PCB,
            Temp_M,
            Temp_IN,
            Temp_OUT,
            MC_ADCBUF_THERM_HEAT_M,//Ĥ�¶Ȳ���ֵ
            MC_ADCBUF_THERM_HEAT_IN,//��ˮ�¶Ȳ���ֵ
            MC_ADCBUF_THERM_HEAT_OUT,//��ˮ�¶Ȳ���ֵ
            MC_ADCBUF_THERM_PTC_OUT;//WPTC��������ֵ
  
  uint32_t  ErrStatus;

  FAULT_STATUS_FLAG_COM fault_status_flag_motor;
  FAULT_STATUS_FLAG_PTC faultStatus_PTC;  
  FAULT_STATUS_FLAG_COM motor_fault_all;
            
} CAN_DEBUG_T;
extern CAN_DEBUG_T debug_info;
typedef struct
{
    uint8_t runCmd;     // 1:run  0:stop
    uint16_t spdCmd_Rpm;     // unit:RPM
    uint8_t PtcRunCmd;     // 1:run  0:stop
    uint16_t Max_power_acc;
    uint16_t max_speed_down; 
} Ctrl_CMD;

typedef enum
{
    Ifx_MS_FocSolutionF16_State_init            = 0, /**<FOC is in init state*/
    Ifx_MS_FocSolutionF16_State_off             = 1, /**<FOC is in off state*/
    Ifx_MS_FocSolutionF16_State_standBy         = 2, /**<FOC is in stand by state*/
    Ifx_MS_FocSolutionF16_State_fault           = 3, /**<FOC is in fault state*/
    Ifx_MS_FocSolutionF16_State_run             = 4, /**<FOC is in run state*/
    Ifx_MS_FocSolutionF16_State_rampDown        = 5, /**<FOC is in ramp down state*/
    Ifx_MS_FocSolutionF16_State_startAngleIdent = 6  /**<FOC is in start angle identification state*/

}MCAPP_STATE_T;

typedef enum
{
    WPTCAPP_INIT = 0,                     /* Initialize Run time parameters */
    WPTCAPP_RUN = 1,                      /* Run the WPTC */
    WPTCAPP_STOP = 2,                     /* Stop the WPTC */
    WPTCAPP_FAULT = 3,                    /* WPTC is in Fault mode */
}WPTCAPP_STATE_T;
typedef struct
{
//  bool                    rx_flag;
  bool                    canFDFormat;
  bool                    extended;
  uint32_t                id;
  uint8_t                 data[8];//[64];
  uint16_t                datalen;
}CAN_RX_MSG;
extern CAN_RX_MSG canRevData;

typedef struct {
    uint16_t adc_value;    // ADCBUF_THERMֵ  �¶�
    float scale_value;     // ��Ӧ��TEMP_SCALEֵ
} ThermScaleMap;
#define MAP_COUNT (sizeof(therm_scale_map) / sizeof(ThermScaleMap))

extern void RecDataAnalyze();
extern uint16_t speed_actal_filter(void) ;
extern void initAD_value(void);
extern void getDebugInfoData_1ms(void);
extern void getDebugInfoData_100ms(void);
extern void PeriodReportData(uint16_t id);
extern void Err_event_temple_pcb(void);
extern void Err_event_temple_M(void);
extern void Err_event_temple_IN(void);
extern void Err_event_temple_OUT(void);
extern void Err_event_temple(void);
extern uint16_t Resistance_Threshold_Value(void);
extern uint8_t check_resistance_protection(void);
extern uint8_t TempOffsetDetect_Oneself(uint16_t Param_Temp);
extern uint8_t TempOffsetDetect_OutAndIn(void);
extern void TempOffsetDetect(void);
extern void Wptc_OcFault_Detect(void);
extern uint16_t Down_Speed_For_power_limit(void);
extern void Can_Communication_OvTime_Detect(void);
extern void Err_event_vol(void);
extern void COM_CurrRMSDetect(void);
extern void COM_PowerOverFault_Detect(void);
extern bool OvCurrentHappened(void);
extern void OvCurrentClear(void);
extern void  Restart_motor(void);
extern void motorStop(uint8_t flag);
extern uint16_t Temp_ADtoTemp(uint16_t ADCBUF_THERM);
extern void Fault_Detect(void);
extern uint16_t ad_to_voltage(uint16_t ad_value);
extern uint16_t ad_to_current(uint16_t ad_value);
extern void wptc_pwm_gradient_control(uint8_t runCmd, uint16_t q_pwm_thick_film);
extern void Execute_Funtion_1ms(void);
extern void Execute_Funtion_500ms(void);
extern void Wptc_AD_Current_Detect(void);
extern void Wptc_HeatProtect_Detect(void);
#endif
