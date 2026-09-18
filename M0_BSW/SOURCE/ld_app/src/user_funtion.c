
#include "user_funtion.h"

#if defined(__ICCARM__)
#define PROTECT_PARAM_ROOT __root
#else
#define PROTECT_PARAM_ROOT
#endif

PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagMotStartMinSpd_rpm_u16 = 800u;
PROTECT_PARAM_ROOT volatile uint32_t Cal_DiagOcRstCntClrDelay_ms_u32 = 180000u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagOcAutoRecDelay_ms_u16 = 30000u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagOcAutoRecLimit_cnt_u16 = 5u;

PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPtcOcFltCfmDelay_ms_u16 = 500u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPtcHtProtCfmDelay_ms_u16 = 200u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPtcAdCurFltCfmDelay_ms_u16 = 500u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagTmpFltCfmDelay_ms_u16 = 500u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPtcResFltCfmDelay_ms_u16 = 5u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagTmpFltRecDelay_ms_u16 = 1000u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPcbSicTmpTrip_degC_u16 = 105u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPcbSicTmpHys_degC_u16 = 15u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagTfTmpTrip_degC_u16 = 85u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagTfTmpHys_degC_u16 = 10u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagInTmpTrip_degC_u16 = 85u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagInTmpHys_degC_u16 = 10u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagOutTmpTrip_degC_u16 = 85u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagOutTmpPwrLim_degC_u16 = 75u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagOutTmpHys_degC_u16 = 10u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagTmpSensMax_adc_u16 = 1300u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagTmpSensMin_adc_u16 = 10u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPtcPwrMax_pct_u16 = 100u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPtcPwrMin_pct_u16 = 0u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPtcOcCurTrip_dA_u16 = 550u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPtcAdCurMin_adc_u16 = 500u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPtcAdCurMax_adc_u16 = 3600u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPtcHtProtCur_dA_u16 = 5u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPtcResHi_ohm_u16 = 100;//29u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPtcResLo_ohm_u16 = 5u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagTfTmpAutoRecLimit_cnt_u16 = 10u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagDryBrnAutoRecLimit_cnt_u16 = 10u;

PROTECT_PARAM_ROOT volatile uint8_t Cal_DiagCanCommLostMonEn_u8 = 1u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagCanCommLostTout_ms_u16 = 5000u;

PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagDcBusOvCfmDelay_ms_u16 = 20u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagDcBusUvCfmDelay_ms_u16 = 20u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagDcBusUvQkCfmDelay_ms_u16 = 0u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagDcBusFltRecDelay_ms_u16 = 6000u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagDcBusOvTrip_V_u16 = 950u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagDcBusOvRec_V_u16 = 930u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagDcBusUvRec_V_u16 = 12u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagDcBusUvDnSpd_rpm_u16 = 110u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagDcBusUvTrip_V_u16 = 10u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagDcBusUvQkTrip_V_u16 = 8u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPhCurTrip_A_u16 = 26u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPhCurCfmDelay_cnt_u16 = 100u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPhPkCurTrip_A_u16 = 45u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPhPkCurCfmDelay_cnt_u16 = 2u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagBusCurTrip_dA_u16 = 300u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagBusCurCfmDelay_cnt_u16 = 100u;
PROTECT_PARAM_ROOT volatile uint8_t Cal_DiagPhCurRmsEn_u8 = 1u;
PROTECT_PARAM_ROOT volatile uint8_t Cal_DiagPhPkCurEn_u8 = 1u;
PROTECT_PARAM_ROOT volatile uint8_t Cal_DiagOvldEn_u8 = 1u;
PROTECT_PARAM_ROOT volatile uint8_t Cal_DiagPwrOvEn_u8 = 1u;
PROTECT_PARAM_ROOT volatile uint8_t Cal_DiagPwrOvDnSpdEn_u8 = 0u;

PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPwrOvDef_W_u16 = 1000u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPwrOvHys_W_u16 = 500u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPwrOvFltCfm_cnt_u16 = 5u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPwrOvAutoRecLimit_cnt_u16 = 5u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPwrOvStepDn_rpmPer100ms_u16 = 80u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPwrOvStepUp_rpmPer100ms_u16 = 50u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPwrOvSpdDirLock_cnt_u16 = 3u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPwrOvZ0ProbeCfm_cnt_u16 = 3u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPwrOvMinSpd_rpm_u16 = 2000u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPwrOvFltRstrtDelay_cnt_u16 = 100u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagPwrOvFltRstDelay_cnt_u16 = 300u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagOvldSpdThd_rpm_u16 = 2000u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagOvldPhCurThd_A_u16 = 25u;
PROTECT_PARAM_ROOT volatile uint16_t Cal_DiagOvldCfmDelay_cnt_u16 = 10000u;

#if defined(__ICCARM__)
/* Keep parameters used only by optional fault-recovery paths in the ELF. */
__root const volatile void * const protect_param_root_anchor[] =
{
  &Cal_DiagOcRstCntClrDelay_ms_u32,
  &Cal_DiagOcAutoRecDelay_ms_u16,
  &Cal_DiagOcAutoRecLimit_cnt_u16
};
#endif

#undef PROTECT_PARAM_ROOT


uint8_t SoftVersion=005;  
Ctrl_CMD control_cmd={0};
CAN_RX_MSG canRevData={0};
CAN_DEBUG_T debug_info={0};
uint8_t CanTxBuffer_0x20B[8] = {0},
         CanTxBuffer_0x20D[8] = {0},
         CanTxBuffer_0x20C[8] = {0},
         CanTxBuffer_0x20E[8] = {0},
         CanTxBuffer_0x20F[8] = {0},  /* DEBUG: raw dq for power calibration */
         CanTxBuffer_0x210[8] = {0};  /* DEBUG: modulation ratio & voltage utilization */

void RecDataAnalyze()//�������ݴ�������
{
    
  if(can_rx_data[TEST_STD_CAN_INDEX0].id==0x20A)
  {
        if(can_rx_data[TEST_STD_CAN_INDEX0].rx_flag)
        {
            debug_info.Cal_CanCommLostTimeout_ms_u16_Count=0;
            debug_info.fault_status_flag_motor.bit.ERR_OUTCAN=0;
            debug_info.faultStatus_PTC.bit.ERR_OUTCAN=0;
        }
        control_cmd.runCmd=can_rx_data[TEST_STD_CAN_INDEX0].data[0]&0x1;//CMD ����ָ��
        control_cmd.spdCmd_Rpm=can_rx_data[TEST_STD_CAN_INDEX0].data[1]+can_rx_data[TEST_STD_CAN_INDEX0].data[2]*256;
        control_cmd.Max_power_acc=can_rx_data[TEST_STD_CAN_INDEX0].data[3]*40;//COM �޹���
        
         control_cmd.PtcRunCmd=(can_rx_data[TEST_STD_CAN_INDEX0].data[0]>>1)&0x1;//ptc_cmd ����ָ��
         debug_info.PTC_Power=can_rx_data[TEST_STD_CAN_INDEX0].data[4]+can_rx_data[TEST_STD_CAN_INDEX0].data[5]*256;//Ŀ�깦��

         /* 故障清除命令: data[0] bit 2 */
         if ((can_rx_data[TEST_STD_CAN_INDEX0].data[0] >> 2) & 0x01)
         {
             Motor_Err_Clear_Set(1);
             OvCurrentClear();
         }

         can_rx_data[TEST_STD_CAN_INDEX0].rx_flag = false;
//           debug_info.DutyPoint= debug_info.PTC_Power;
         if(debug_info.PTC_Power>Cal_DiagPtcPwrMax_pct_u16)debug_info.PTC_Power=Cal_DiagPtcPwrMax_pct_u16;
         if(debug_info.PTC_Power<Cal_DiagPtcPwrMin_pct_u16 && control_cmd.PtcRunCmd && debug_info.PTC_Power>0)debug_info.PTC_Power=Cal_DiagPtcPwrMin_pct_u16;
         
         if((control_cmd.spdCmd_Rpm>MAXIMUM_SPEED_RPM)||(control_cmd.spdCmd_Rpm<Cal_DiagMotStartMinSpd_rpm_u16))
         {
            control_cmd.spdCmd_Rpm=0;
            control_cmd.runCmd=0;
         }
         if(control_cmd.runCmd == 0 || control_cmd.max_speed_down == 0)
         {
             control_cmd.max_speed_down = MAXIMUM_SPEED_RPM;
         }
         uint16_t effective_speed = control_cmd.spdCmd_Rpm;
         if(effective_speed > control_cmd.max_speed_down)
         {
             effective_speed = control_cmd.max_speed_down;
         }
         Motor_Control_Set(control_cmd.runCmd, effective_speed, control_cmd.Max_power_acc);//COM

  }
}
void MotorStart(uint8_t runCmd, uint16_t spdCmd,uint16_t powerLimt)
{
  if((runCmd==1)&(debug_info.fault_status_flag_motor.Word==0))
  {
     Motor_Control_Set(runCmd,spdCmd,powerLimt);
  }
  else
  {
      if(debug_info.fault_status_flag_motor.Word!=0)
      {
        motorStop(0);//flag=0 ����ͣ�� ��flag=1 ����ͣ��
      }
      else
      {
        motorStop(1);
      }
  }
}
float d=0.005;
float SampleTime=70,dd=7.5;
void wptc_pwm_gradient_control(uint8_t runCmd, uint16_t q_pwm_thick_film)
{

    static float current_pwm = 0.0f;  
    if(( control_cmd.PtcRunCmd==1)&&(debug_info.faultStatus_PTC.Word==0)&&(!debug_info.fault_status_flag_motor.bit.UvFault)&&(!debug_info.fault_status_flag_motor.bit.UvFaultLow))
    {
        MAIN_SWITCH_ENABLE(1); 
      // 0~100%
      if (q_pwm_thick_film < 0)
      {
          q_pwm_thick_film = 0;
      }
      else if (q_pwm_thick_film >= 100)
      {
          q_pwm_thick_film = 99;
      }  
      if (q_pwm_thick_film > current_pwm)
      {
          // 20���????0��Ŀ�꣬1ms���� = Ŀ��ֵ / 20000
          current_pwm += d;
          if (current_pwm > q_pwm_thick_film)
          {
              current_pwm = q_pwm_thick_film;
          }
      }
      else if (q_pwm_thick_film < current_pwm)
      {
          current_pwm = q_pwm_thick_film;
      }    
      g_pwm_thick_film=(uint8_t)current_pwm;//debug_info.PTC_Power;//
      SampleTime=g_pwm_thick_film*dd;
    Adc_SetSampleTime_Dynamic(SampleTime);
    }
    else
    {
        MAIN_SWITCH_ENABLE(0); 
      Adc_SetSampleTime_Dynamic(70);
      current_pwm=0;
        g_pwm_thick_film=0;
      debug_info.PtcPeakCurrents=0;//WPTC ��ֵ����
      debug_info.PtcCurrents=0;//WPTC ��Ч����
      debug_info.PTC_RealPower=0;
       debug_info.PTC_Resistance =0;
       PTC_Controller_Reset();
    }
}
uint16_t speed_actal_filter(void) 
{
   static uint16_t speed_actal_file=0;
    uint16_t new_speed=Motor_Speed_Feedback_Q15_Get();// motor_speed
   speed_actal_file = (uint16_t)(((uint32_t)speed_actal_file*62258 + (uint32_t)new_speed*3278)>>16 );
     if((speed_actal_file< control_cmd.spdCmd_Rpm)&&speed_actal_file>500)
          speed_actal_file=speed_actal_file+(50-speed_actal_file%50);
   return (uint16_t)(speed_actal_file);
}
void initAD_value(void)
{
    uint8_t tempValue=65;
    debug_info.Temp_PCB=tempValue;
    debug_info.Temp_PTC_SIC = tempValue;
    debug_info.Temp_IN=tempValue;
    debug_info.Temp_OUT=tempValue;
    debug_info.MC_ADCBUF_THERM_HEAT_IN=tempValue;
    debug_info.MC_ADCBUF_THERM_HEAT_OUT=tempValue;
}
float ddd=1.48f;
void getDebugInfoData_1ms(void)
{
  static uint16_t g_ad_thick_film_current_filter=0;
    g_ad_thick_film_current_filter = g_ad_thick_film_current_filter - (g_ad_thick_film_current_filter>>3) + (g_ad_thick_film_current>>3);
   if(g_pwm_thick_film>0)
     {
        debug_info.PtcPeakCurrents=ad_to_current(g_ad_thick_film_current_filter)*ddd;//1.414;//1.39;//WPTC ��ֵ��
     }
    if((g_pwm_thick_film>1)&&(g_pwm_thick_film<8)&&(g_ad_thick_film_current>200))
    {
      debug_info.PTC_RealPower=P_MAX*g_pwm_thick_film/100;
      debug_info.PtcCurrents=(float)debug_info.PTC_RealPower/debug_info.pVdcValue*10;
     // debug_info.PtcPeakCurrents=debug_info.PtcCurrents*100/g_pwm_thick_film;
    }
    else
    {
     debug_info.PtcCurrents=debug_info.PtcPeakCurrents*g_pwm_thick_film/100;//WPTC ��Ч����a*g_pwm_thick_film/100;//
     uint32_t PTC_RealPower_file=(uint32_t)(debug_info.pVdcValue*((float)debug_info.PtcCurrents/10));
     debug_info.PTC_RealPower=(((uint32_t)debug_info.PTC_RealPower*62258 + (uint32_t)PTC_RealPower_file*3278)>>16 );
    }
    debug_info.PtcPeakCurrents_fast=ad_to_current(g_ad_thick_film_current)*0.148;//WPTC ��ֵ��  ��©���˱������õ�
    if(g_pwm_thick_film>5)
    {
      if(debug_info.PtcPeakCurrents_fast>0)
      {
        debug_info.PTC_Resistance = (uint32_t)debug_info.pVdcValue / debug_info.PtcPeakCurrents_fast;
      }
       else
       {
         debug_info.PTC_Resistance = 0;
       }
    }
    else
    {
      debug_info.PTC_Resistance =0;
    }
}

void getDebugInfoData_100ms(void)
{
   static uint16_t VdcValue=0;
    
    VdcValue=ad_to_voltage(Bus_Voltage_Get());
    debug_info.pVdcValue =VdcValue;// (uint16_t)(((uint32_t)debug_info.pVdcValue*58982 + (uint32_t)VdcValue*6554)>>16 ); //COM WPTC
    //gUDC.uDCFilter = gUDC.uDCFilter - (gUDC.uDCFilter>>3) + (gUDC.uDC>>3)-2; //ƽ���˲�
    debug_info.speed_actal=speed_actal_filter();//speed_actal_filter();//COM
    {
        static uint16_t ph_curr_filt=0;
        uint16_t raw;
        debug_info.COM_Power=Motor_power_Q15_Get();
        debug_info.BusCurrent=Bus_Current_Get();
        raw=Phase_Current_Get();
        /* 电机坜�?�时夝佝滤波�?，靿兝下次坯动瞬思滞�? */
        if(Motor_Running_Status_Get()!=Ifx_MS_FocSolutionF16_State_run)
        {
            ph_curr_filt=0;
        }
        /* 一阶IIR低通滤�?: y=(52429*y+13107*x)>>16, α�?0.2, τ�?400ms @100ms */
        ph_curr_filt=(uint16_t)(((uint32_t)ph_curr_filt*52429u+(uint32_t)raw*13107u)>>16);
        debug_info.PhaseCurrent=ph_curr_filt;
    }
    debug_info.MotorState=Motor_Running_Status_Get();//COM
    debug_info.ErrStatus=Motor_Err_Status_Get();//����״̬  COM WPTC
    debug_info.fault_status_flag_motor.bit.OcFault = IPMFAULT_STATE_Get();//(debug_info.ErrStatus & 0x00100000u) ? 1 : 0;
    COM_PowerOverFault_Detect();
    Motor_Fault_All_Set((uint32_t)(debug_info.fault_status_flag_motor.Word));//COM ������й���״�?

    if(debug_info.MotorState!=Ifx_MS_FocSolutionF16_State_run)
    {
        debug_info.speed_actal=0;
        debug_info.COM_Power=0;
        debug_info.BusCurrent=0;
        debug_info.PhaseCurrent=0;
    }
//    debug_info.PtcCurrents=ad_to_current(g_ad_thick_film_current);//WPTC ��Ч����
//    debug_info.PtcPeakCurrents=0;//WPTC ��ֵ����
//    debug_info.PTC_RealPower=0;//WPTC ʵ�ʹ���
    
    //��Ĥ��������
  //  debug_info.PhasePeakCurrent=Phase_Current_U_Get();//COM ����������
     debug_info.Temp_COM_SIC=Temp_ADtoTemp(g_ad_compressor_SIC_temp);
    debug_info.Temp_PCB=Temp_ADtoTemp(g_ad_PCB_temp);
//    //debug_info.Temp_M=Temp_ADtoTemp(g_ad_thick_film_temp);
    debug_info.Temp_PTC_SIC = Temp_ADtoTemp(g_ad_thick_film_SIC_temp);
    debug_info.Temp_IN=Temp_ADtoTemp(g_ad_inlet_water_temp);
    debug_info.Temp_OUT=Temp_ADtoTemp(g_ad_outlet_water_temp);
    debug_info.MC_ADCBUF_THERM_HEAT_IN=g_ad_inlet_water_temp;
    debug_info.MC_ADCBUF_THERM_HEAT_OUT=g_ad_outlet_water_temp;
}

void PeriodReportData(uint16_t id)
{
  /* Whitelist: only known frame IDs are allowed to be transmitted.
   * This also neutralizes any local-variable shadowing of the `id`
   * parameter inside branches — the literal CAN_ID used here is
   * guaranteed regardless of inner scope declarations.                */
  switch (id)
  {
  case 0x20B: /* fallthrough */
  case 0x20C:
  case 0x20D:
  case 0x20E:
  case 0x20F:
  case 0x210:
      break;
  default:
      return;   /* unknown ID → reject */
  }

  if(id ==0x20B)//ACCM_1
  {
      CanTxBuffer_0x20B[0]=(uint8_t)(debug_info.pVdcValue&0xff);
      CanTxBuffer_0x20B[1]=(uint8_t)(debug_info.pVdcValue>>8);
      CanTxBuffer_0x20B[2]=(uint8_t)(debug_info.speed_actal&0xff);//debug_info.speed_actal
      CanTxBuffer_0x20B[3]=(uint8_t)(debug_info.speed_actal>>8);
       //CanTxBuffer_0x20B[2]=(uint8_t)(debug_info.speed_actal&0xff);//
      // CanTxBuffer_0x20B[3]=(uint8_t)(debug_info.speed_actal>>8);
      
//            CanTxBuffer_0x20B[0]=(uint8_t)(g_ad_compressor_SIC_temp&0xff);
//      CanTxBuffer_0x20B[1]=(uint8_t)(g_ad_compressor_SIC_temp>>8);
//      CanTxBuffer_0x20B[2]=(uint8_t)(g_ad_PCB_temp&0xff);
//      CanTxBuffer_0x20B[3]=(uint8_t)(g_ad_PCB_temp>>8);
      
      if(debug_info.MotorState==Ifx_MS_FocSolutionF16_State_run)
      {
       CanTxBuffer_0x20B[4]=1;
      }
      else if(debug_info.MotorState==Ifx_MS_FocSolutionF16_State_fault)//Fault
      {
           CanTxBuffer_0x20B[4]=3;
      }
      else
      {
            CanTxBuffer_0x20B[4]=2; //stop
      }
      CanTxBuffer_0x20B[5]=(uint8_t)(debug_info.BusCurrent);
      CanTxBuffer_0x20B[6]=(uint8_t)(debug_info.PhaseCurrent*10);
      CanTxBuffer_0x20B[7]=(uint8_t)(debug_info.COM_Power/100);
      Can_Transmit(CY_CANFD0_TYPE, 0x20B, 8, CanTxBuffer_0x20B,false);
  }
  
  else if(id == 0x20C)//ACCM&WPTC
      {
        CanTxBuffer_0x20C[0]=debug_info.Temp_COM_SIC;
        CanTxBuffer_0x20C[1]=debug_info.Temp_PCB;
        CanTxBuffer_0x20C[2]=debug_info.Temp_PTC_SIC;
        CanTxBuffer_0x20C[3]=debug_info.PTC_Resistance/4;
        CanTxBuffer_0x20C[4]=debug_info.Temp_OUT;
        CanTxBuffer_0x20C[5]=debug_info.Temp_IN;
           if(debug_info.fault_status_flag_motor.bit.StallDetected!=0)//��ת
           {
            CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]|0x1;
           }
           else
           {
                CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]&(~0x1);
           }
           if(debug_info.fault_status_flag_motor.bit.OcFault!=0)//Ӳ������
           {
            CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]|0x2;
           }
           else
           {
                CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]&(~0x2);
           }
          if(debug_info.fault_status_flag_motor.bit.OvFault!=0)//��ѹ
           {
            CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]|0x04;
           }
           else
           {
                CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]&(~0x04);
           }
          if(debug_info.fault_status_flag_motor.bit.UvFault!=0)//Ƿѹ
           {
            CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]|0x08;
           }
           else
           {
                CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]&(~0x08);
           }
          if(debug_info.fault_status_flag_motor.bit.OvTemp!=0)//����
           {
            CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]|0x20;
           }
           else
           {
                CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]&(~0x20);
           }
             if(debug_info.fault_status_flag_motor.bit.speedFault!=0)//ʧ��
           {
            CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]|0x40;
           }
           else
           {
                CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]&(~0x40);
           }
          if(debug_info.fault_status_flag_motor.bit.OvSoftRmsCurrent!=0)//�������� ��Чֵ
           {
            CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]|0x80;
           }
           else
           {
                CanTxBuffer_0x20C[6]= CanTxBuffer_0x20C[6]&(~0x80);
           }
           if(debug_info.fault_status_flag_motor.bit.OvSoftwarePeakCurrent!=0)//������ֵ����
           {
            CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]|0x01;
           }
           else
           {
                CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]&(~0x01);
           }
           if(debug_info.fault_status_flag_motor.bit.OvLoadFault!=0)//����
           {
            CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]|0x04;
           }
           else
           {
                CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]&(~0x04);
           }
           if(debug_info.fault_status_flag_motor.bit.ERR_OUTCAN!=0)//CANͨѶ��ʧ
           {
            CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]|0x08;
           }
           else
           {
                CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]&(~0x08);
           }
           if(debug_info.fault_status_flag_motor.bit.BusOff!=0)//CAN busoff
           {
            CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]|0x10;
           }
           else
           {
                CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]&(~0x10);
           }
           if(debug_info.fault_status_flag_motor.bit.OvFaultLow!=0)//��ѹ��ѹ
           {
            CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]|0x20;
           }
           else
           {
                CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]&(~0x20);
           }
           if(debug_info.fault_status_flag_motor.bit.UvFaultLow!=0)//��ѹǷѹ
           {
            CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]|0x40;
           }
           else
           {
                CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]&(~0x40);
           }
          if(debug_info.faultStatus_PTC.bit.M_AD_Current!=0)//����  Ĥ��ֵ�ж�
           {
            CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]|0x80;
           }
           else
           {
                CanTxBuffer_0x20C[7]= CanTxBuffer_0x20C[7]&(~0x80);
           }
            Can_Transmit(CY_CANFD0_TYPE, 0x20C, 8, CanTxBuffer_0x20C,false);
      }
  else if(id == 0x20D) //WPTC
      {
          CanTxBuffer_0x20D[0]=g_pwm_thick_film;//debug_info.DutyPoint;
          CanTxBuffer_0x20D[1]=(uint8_t)(debug_info.PTC_RealPower&0xff);
          CanTxBuffer_0x20D[2]=(uint8_t)(debug_info.PTC_RealPower>>8);
          CanTxBuffer_0x20D[3]=debug_info.PtcCurrents;
          CanTxBuffer_0x20D[4]=debug_info.PtcPeakCurrents/2;
          
          if(debug_info.faultStatus_PTC.bit.M_Resistance_High_Fault!=0)
           {
            CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]|0x1;
           }
           else
           {
                CanTxBuffer_0x20D[5]=CanTxBuffer_0x20D[5]&(~0x1);
           }
          if(debug_info.faultStatus_PTC.bit.OcFault!=0)
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]|0x02;//WPTC����
          }
          else
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]&(~0x02);
          }
          if(debug_info.faultStatus_PTC.bit.DryBurn!=0)//����  �����ٶ��ж�
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]|0x04;//WPTC����
          }
          else
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]&(~0x04);
          }
          if(debug_info.faultStatus_PTC.bit.OvTemp_IN!=0)
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]|0x08;//��ˮ����
          }
          else
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]&(~0x08);
          }
          if(debug_info.faultStatus_PTC.bit.OvTemp_OUT!=0)
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]|0x10;//��ˮ����
          }
          else
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]&(~0x010);
          }
          if(debug_info.faultStatus_PTC.bit.M_ClntOtltOverTempProtn!=0)
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]|0x20;//����ˮ�²����????
          }
          else
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]&(~0x020);
          }
          if(debug_info.faultStatus_PTC.bit.TempSensorFault_IN!=0)
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]|0x40;//��ˮ�¶Ȳ�������
          }
          else
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]&(~0x040);
          }
          if(debug_info.faultStatus_PTC.bit.TempSensorFault_OUT!=0)
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]|0x80;//��ˮ�¶Ȳ�������
          }
          else
          {
               CanTxBuffer_0x20D[5]= CanTxBuffer_0x20D[5]&(~0x080);
          }
          CanTxBuffer_0x20D[6]= debug_info.faultStatus_PTC.Word>>8;
          CanTxBuffer_0x20D[7]=SoftVersion;//�汾��
           Can_Transmit(CY_CANFD0_TYPE, 0x20D, 8, CanTxBuffer_0x20D,false);
      }
   else if(id == 0x20E)//ACCM_Fault
       {
           CanTxBuffer_0x20E[0] = (uint8_t)(debug_info.fault_status_flag_motor.Word & 0xff);
           CanTxBuffer_0x20E[1] = (uint8_t)(debug_info.fault_status_flag_motor.Word >> 8);
           CanTxBuffer_0x20E[2] = (uint8_t)(debug_info.fault_status_flag_motor.Word >> 16);
           CanTxBuffer_0x20E[2] |= (IPMFAULT_STATE_Get() << 1);
            Can_Transmit(CY_CANFD0_TYPE, 0x20E, 8, CanTxBuffer_0x20E,false);
       }
   else if(id == 0x20F)//DEBUG: raw dq for power calibration
       {
           uint16_t vd = Debug_Vd_Q15_Get();
           uint16_t vq = Debug_Vq_Q15_Get();
           uint16_t id_q15 = Debug_Id_Q15_Get();
           uint16_t iq = Debug_Iq_Q15_Get();

           CanTxBuffer_0x20F[0] = (uint8_t)(vd & 0xff);
           CanTxBuffer_0x20F[1] = (uint8_t)(vd >> 8);
           CanTxBuffer_0x20F[2] = (uint8_t)(vq & 0xff);
           CanTxBuffer_0x20F[3] = (uint8_t)(vq >> 8);
           CanTxBuffer_0x20F[4] = (uint8_t)(id_q15 & 0xff);
           CanTxBuffer_0x20F[5] = (uint8_t)(id_q15 >> 8);
           CanTxBuffer_0x20F[6] = (uint8_t)(iq & 0xff);
           CanTxBuffer_0x20F[7] = (uint8_t)(iq >> 8);
           Can_Transmit(CY_CANFD0_TYPE, 0x20F, 8, CanTxBuffer_0x20F, false);
       }
   else if(id == 0x210)//DEBUG: modulation ratio & voltage utilization
       {
           uint16_t mq   = Mod_Ratio_Q15_Get();
           uint16_t upct = Volt_Util_Pct_Get();
           uint8_t  ov   = Overmod_Flag_Get();

           CanTxBuffer_0x210[0] = (uint8_t)(mq & 0xff);
           CanTxBuffer_0x210[1] = (uint8_t)(mq >> 8);
           CanTxBuffer_0x210[2] = (uint8_t)(upct & 0xff);
           CanTxBuffer_0x210[3] = (uint8_t)(upct >> 8);
           CanTxBuffer_0x210[4] = ov;
           CanTxBuffer_0x210[5] = 0;
           CanTxBuffer_0x210[6] = 0;
           CanTxBuffer_0x210[7] = 0;
           Can_Transmit(CY_CANFD0_TYPE, 0x210, 8, CanTxBuffer_0x210, false);
       }
}

/**********************************************--WPTC���ϱ���--**********************************************************************************************************/

void Err_event_temple_pcb(void)
{
   static uint16_t bOverHeat_counter_A = 0;
   static uint16_t bOverHeat_counter_B = 0;

	if((debug_info.Temp_PCB > (Cal_DiagPcbSicTmpTrip_degC_u16+40)  && debug_info.Temp_PCB < 199)
        ||(debug_info.Temp_COM_SIC > (Cal_DiagPcbSicTmpTrip_degC_u16+40)  && debug_info.Temp_COM_SIC < 199)
        ||(debug_info.Temp_PTC_SIC > (Cal_DiagPcbSicTmpTrip_degC_u16+40)  && debug_info.Temp_PTC_SIC < 199))
	{
		bOverHeat_counter_A++;			
		if (bOverHeat_counter_A >= Cal_DiagTmpFltCfmDelay_ms_u16)
		{	
                    bOverHeat_counter_A = 0;
                    debug_info.OvTemp_flag=1;
                     debug_info.fault_status_flag_motor.bit.OvTemp=1;
		}
	}
	else if(( debug_info.OvTemp_flag)&&(debug_info.Temp_COM_SIC <= ((Cal_DiagPcbSicTmpTrip_degC_u16+40) - Cal_DiagPcbSicTmpHys_degC_u16))
             &&(debug_info.Temp_PCB <= ((Cal_DiagPcbSicTmpTrip_degC_u16+40) - Cal_DiagPcbSicTmpHys_degC_u16))
             &&(debug_info.Temp_PTC_SIC <= ((Cal_DiagPcbSicTmpTrip_degC_u16+40) - Cal_DiagPcbSicTmpHys_degC_u16)))
	{
                bOverHeat_counter_B++;
                if(bOverHeat_counter_B>=Cal_DiagTmpFltRecDelay_ms_u16)
                  {
                     debug_info.OvTemp_flag=0;
                    debug_info.fault_status_flag_motor.bit.OvTemp=0;
                    bOverHeat_counter_B = 0;
                  }
	}
	else
	{
		bOverHeat_counter_A = 0;
		bOverHeat_counter_B = 0;
	}
}

void Err_event_temple_M(void)
{
 static uint16_t bOverHeat_counter_A = 0;
 static uint16_t bOverHeat_counter_C = 0;
 static uint16_t bOverHeat_counter_D = 0;
    if(debug_info.MC_ADCBUF_THERM_HEAT_M>Cal_DiagTmpSensMax_adc_u16||debug_info.MC_ADCBUF_THERM_HEAT_M<Cal_DiagTmpSensMin_adc_u16)
    {
        bOverHeat_counter_A++;			
		if (bOverHeat_counter_A >= Cal_DiagTmpFltCfmDelay_ms_u16)
		{	
			bOverHeat_counter_A = 0;
           debug_info.faultStatus_PTC.bit.TempSensorFault_M=1;
		}
    }
    else
    {
         debug_info.faultStatus_PTC.bit.TempSensorFault_M=0;
         bOverHeat_counter_A=0;
    }
	if((debug_info.Temp_M > (Cal_DiagTfTmpTrip_degC_u16+40)  && debug_info.Temp_M < 199))
	{
		bOverHeat_counter_C++;			
		if (bOverHeat_counter_C >= Cal_DiagTmpFltCfmDelay_ms_u16)
		{	
			bOverHeat_counter_C = 0;
            if(control_cmd.PtcRunCmd==1)
            {
                if( debug_info.faultStatus_PTC.bit.OvTemp_M==0)
                {
                debug_info.OvTemp_Lock_Count++;
                debug_info.OvTemp_M_flag=1;
                debug_info.faultStatus_PTC.bit.OvTemp_M=1;
                }
            }
		}
	}
	else if(( debug_info.OvTemp_M_flag)&&(debug_info.Temp_M <= ((Cal_DiagTfTmpTrip_degC_u16+40) - Cal_DiagTfTmpHys_degC_u16)))
	{
			bOverHeat_counter_D++;
			if((bOverHeat_counter_D>=Cal_DiagTmpFltRecDelay_ms_u16)&&(debug_info.OvTemp_Lock_Count<Cal_DiagTfTmpAutoRecLimit_cnt_u16))
				{
                                    debug_info.OvTemp_M_flag=0;
                                    debug_info.faultStatus_PTC.bit.OvTemp_M=0;
                                    bOverHeat_counter_D = 0;
				}
	}
	else
	{
		bOverHeat_counter_C = 0;
		bOverHeat_counter_D = 0;
	}
}

void Err_event_temple_IN(void)
{
 static uint16_t bOverHeat_counter_A = 0;
 static uint16_t bOverHeat_counter_C = 0;
 static uint16_t bOverHeat_counter_D = 0;
    if(debug_info.MC_ADCBUF_THERM_HEAT_IN>Cal_DiagTmpSensMax_adc_u16||debug_info.MC_ADCBUF_THERM_HEAT_IN<Cal_DiagTmpSensMin_adc_u16)
    {
        bOverHeat_counter_A++;			
		if (bOverHeat_counter_A >= Cal_DiagTmpFltCfmDelay_ms_u16)
		{	
                    bOverHeat_counter_A = 0;
                    debug_info.faultStatus_PTC.bit.TempSensorFault_IN=1;//�¶ȴ���������
		}
    }
    else
    {
         debug_info.faultStatus_PTC.bit.TempSensorFault_IN=0;
         bOverHeat_counter_A=0;
    }
	if(debug_info.Temp_IN > (Cal_DiagInTmpTrip_degC_u16+40)  && debug_info.Temp_IN < 199)
	{
		bOverHeat_counter_C++;			
		if (bOverHeat_counter_C >= Cal_DiagTmpFltCfmDelay_ms_u16)
		{	
			bOverHeat_counter_C = 0;
                    if(control_cmd.PtcRunCmd==1)
                    {
                        if(debug_info.faultStatus_PTC.bit.OvTemp_IN==0)
                        {
                           debug_info.OvTemp_Lock_Count++;
                          debug_info.OvTemp_IN_flag=1;
                          debug_info.faultStatus_PTC.bit.OvTemp_IN=1;
                        }
                    }
		}
	}
	else if(( debug_info.OvTemp_IN_flag) &&(debug_info.Temp_IN <= ((Cal_DiagInTmpTrip_degC_u16+40) - Cal_DiagInTmpHys_degC_u16)))
	{
                bOverHeat_counter_D++;
                if((bOverHeat_counter_D>=Cal_DiagTmpFltRecDelay_ms_u16)&&(debug_info.OvTemp_Lock_Count<Cal_DiagTfTmpAutoRecLimit_cnt_u16))
                        {
                            debug_info.OvTemp_IN_flag=0;
                            debug_info.faultStatus_PTC.bit.OvTemp_IN=0;
                            bOverHeat_counter_D = 0;
                        }
	}
	else
	{
		bOverHeat_counter_C = 0;
		bOverHeat_counter_D = 0;
	}
}

void Err_event_temple_OUT(void)
{
 static uint16_t bOverHeat_counter_A = 0;
 static uint16_t bOverHeat_counter_C = 0;
 static uint16_t bOverHeat_counter_D = 0;
    if(debug_info.MC_ADCBUF_THERM_HEAT_OUT>Cal_DiagTmpSensMax_adc_u16||debug_info.MC_ADCBUF_THERM_HEAT_OUT<Cal_DiagTmpSensMin_adc_u16)
    {
        bOverHeat_counter_A++;			
		if (bOverHeat_counter_A >= Cal_DiagTmpFltCfmDelay_ms_u16)//�¶ȴ���������
		{	
			bOverHeat_counter_A = 0;
           debug_info.faultStatus_PTC.bit.TempSensorFault_OUT=1;
		}
    }
    else
    {
         debug_info.faultStatus_PTC.bit.TempSensorFault_OUT=0;
         bOverHeat_counter_A=0;
    }
	if(debug_info.Temp_OUT > (Cal_DiagOutTmpTrip_degC_u16+40)  && debug_info.Temp_OUT < 199)
	{
		bOverHeat_counter_C++;			
		if (bOverHeat_counter_C >= Cal_DiagTmpFltCfmDelay_ms_u16)
		{	
			bOverHeat_counter_C = 0;
                    if(control_cmd.PtcRunCmd==1)
                    {
                        if(debug_info.faultStatus_PTC.bit.OvTemp_OUT==0)
                        {
                           debug_info.OvTemp_Lock_Count++;
                          debug_info.OvTemp_OUT_flag=1;
                          debug_info.faultStatus_PTC.bit.OvTemp_OUT=1;
                        }
                    }
		}
	}
	else if(( debug_info.OvTemp_OUT_flag) &&(debug_info.Temp_OUT <= ((Cal_DiagOutTmpTrip_degC_u16+40) - Cal_DiagOutTmpHys_degC_u16)))
	{
			bOverHeat_counter_D++;
			if((bOverHeat_counter_D>=Cal_DiagTmpFltRecDelay_ms_u16)&&(debug_info.OvTemp_Lock_Count<Cal_DiagTfTmpAutoRecLimit_cnt_u16))
				{
                                    debug_info.OvTemp_OUT_flag=0;
                                    debug_info.faultStatus_PTC.bit.OvTemp_OUT=0;
                                    bOverHeat_counter_D = 0;
				}
	}
	else
	{
		bOverHeat_counter_C = 0;
		bOverHeat_counter_D = 0;
	}
}

void Err_event_temple(void)
{
    Err_event_temple_pcb();
   // Err_event_temple_M();
    Err_event_temple_IN();
    Err_event_temple_OUT();
}

uint16_t Resistance_Threshold_Value(void) //���ձ�����Ĥ��ֵ����
{
    if(debug_info.pVdcValue<500)
    {
     return 175;
    }
    else  if(debug_info.pVdcValue<600)
    {
     return 176;
    }
    else  if(debug_info.pVdcValue<700)
    {
     return 179;
    }
    else  if(debug_info.pVdcValue<800)
    {
     return 185;
    }
    else  if(debug_info.pVdcValue<900)
    {
     return 186;
    }
    else
    {
      return 186;
    }
}

uint8_t check_resistance_protection(void)//���ձ���(TCR) ������ֵ�仯�ٶȽ��б���
{
    static uint8_t result=0;
    static uint16_t  Resistance_High_Count=0;
    static uint16_t Resistance_Low_Count=0;
    static uint16_t Normal_Count=0;
       if(g_pwm_thick_film==0||debug_info.pVdcValue==0)
       {
            debug_info.PTC_Resistance = 0;
            Resistance_High_Count = 0;
            Resistance_Low_Count = 0;
            Normal_Count = 0;
            result = 0;
       }
       /* if((debug_info.PtcPeakCurrents_fast>0)&&(g_pwm_thick_film>1))
       {
          debug_info.PTC_Resistance = (uint32_t)debug_info.pVdcValue / debug_info.PtcPeakCurrents_fast;//PtcPeakCurrents_fast;
       }
        else
        {
           debug_info.PTC_Resistance = (uint32_t)debug_info.pVdcValue / 24;
        }*/
       // debug_info.PTC_Resistance = (uint32_t)debug_info.pVdcValue / debug_info.PtcPeakCurrents_fast;//PtcPeakCurrents_fast;
       if(g_pwm_thick_film>5)
       {
        if (debug_info.PTC_Resistance > Cal_DiagPtcResHi_ohm_u16) //Resistance_Threshold_Value())//
        {
            Resistance_High_Count++;
            Resistance_Low_Count = 0; // ??????????
            Normal_Count = 0;
            if(Resistance_High_Count>=Cal_DiagPtcResFltCfmDelay_ms_u16)
            {
                Resistance_High_Count=0;
                debug_info.faultStatus_PTC.bit.M_Resistance_High_Fault=1;
                 result = 1;
            }
        } 
        else if (debug_info.PTC_Resistance <= Cal_DiagPtcResLo_ohm_u16)
        {
            /* ????????????? */
             Resistance_Low_Count++;
             Resistance_High_Count = 0; // ??????????
             Normal_Count = 0;
            if(Resistance_Low_Count>=(Cal_DiagPtcResFltCfmDelay_ms_u16+5))
            {
                Resistance_Low_Count=0;
                debug_info.faultStatus_PTC.bit.M_Resistance_Low_Fault=1;
                result = 1;
            }
        }
        else
        {
            Normal_Count++;
            if(Normal_Count>Cal_DiagPtcResFltCfmDelay_ms_u16)
            {
                Normal_Count=0;
                Resistance_Low_Count=0;
                Resistance_High_Count=0;
            }
             result = 0;
        }
       }
    return result;
}

uint8_t TempOffsetDetect_Oneself(uint16_t Param_Temp)//���ձ���  ���ݳ�ˮ���������жϸ��ձ���
{
    static uint8_t getTempFlag=0,result=0;
    static uint16_t startTimeCount=0,lastTemp=60;
   if(g_ad_thick_film_current<=100) return 0 ;
         if(!getTempFlag)
            {
                lastTemp=Param_Temp;
                getTempFlag=1;
            }
        startTimeCount++;
        if(startTimeCount>=2500)//2.5s   ����Ĥ����ʱ���¶ȱ궨
        {
            startTimeCount=0;
            getTempFlag=0;
            if( debug_info.PtcCurrents>0)//
              {
                if((Param_Temp>lastTemp)&&((Param_Temp-lastTemp)>20))//20
                {
                        result=1; //
                      debug_info.faultStatus_PTC.bit.DryBurn=1;
                }
             }
        }
        
    return result;
}

uint8_t TempOffsetDetect_OutAndIn_0(void)//���ձ��� ���ݽ���ˮ�²��жϸ��ձ���
{
    static uint16_t bOverHeat_counter_A = 0,bOverHeat_counter_B = 0;
    static uint8_t result=0;
     if(((debug_info.Temp_OUT>debug_info.Temp_IN)&&(debug_info.Temp_OUT-debug_info.Temp_IN)>25)||
      ((debug_info.Temp_IN>debug_info.Temp_OUT)&&(debug_info.Temp_IN-debug_info.Temp_OUT)>25))
    {
          bOverHeat_counter_A++;			
        if (bOverHeat_counter_A >=Cal_DiagTmpFltCfmDelay_ms_u16)
        {	
            bOverHeat_counter_A = 0;
            result=1;
        }
    }
    else if((debug_info.Temp_OUT>debug_info.Temp_IN)&&(debug_info.Temp_OUT-debug_info.Temp_IN)<5)//if(debug_info.PTC_POWER_REDUCTION==0)
    {
       bOverHeat_counter_A = 0;
       bOverHeat_counter_B++;			
      if (bOverHeat_counter_B >=Cal_DiagTmpFltCfmDelay_ms_u16)
      {	
           result=0;
          debug_info.PTC_POWER_REDUCTION=0;
           bOverHeat_counter_B = 0;
      }
    }
    else
    {
      bOverHeat_counter_A = 0;
      bOverHeat_counter_B = 0;
    }
    return result;
}

uint8_t TempOffsetDetect_OutAndIn(void)
{
    int16_t dT    = debug_info.Temp_OUT - debug_info.Temp_IN;
    int16_t dT_1    = debug_info.Temp_IN - debug_info.Temp_OUT;
    int16_t Tin   = debug_info.Temp_IN;
    int16_t limit;
    static uint16_t bOverHeat_counter_A = 0;  // ���ϼ�ʱ
    static uint16_t bOverHeat_counter_B = 0;  // �ָ���ʱ
    static uint8_t  result = 0;               // ���ձ���״̬ 1:���� 0:����

    uint8_t temp = 0;
    if (Tin < 0)
    {
        if (dT > 40)
        {
            temp = 1;
        }
    }
    else if (Tin < 15)
    {
        limit = 40 - Tin;
        if (dT > limit)
        {
            temp = 1;
        }
    }
    else // Tin >= 15��
    {
        if (dT > 25)
        {
            temp = 1;
        }
    }
    if(dT_1>10)//��ˮ�¶�-��ˮ�¶ȴ���10�汣��
    {
      temp = 1;
    }
    if (temp == 1)
    {
        bOverHeat_counter_B = 0;       // ���ϳ��֣���ջָ����?
        if (bOverHeat_counter_A < Cal_DiagTmpFltCfmDelay_ms_u16)
        {
            bOverHeat_counter_A++;
        }
        if (bOverHeat_counter_A >= Cal_DiagTmpFltCfmDelay_ms_u16)
        {
            result = 1;
        }
    }
    else
    {
        bOverHeat_counter_A = 0;       // ������������չ��ϼ��?
        if (result == 1)
        {
            // �ָ����������� ��5��
            if (dT <= 5)
            {
                if (bOverHeat_counter_B < Cal_DiagTmpFltCfmDelay_ms_u16)
                {
                    bOverHeat_counter_B++;
                }
                if (bOverHeat_counter_B >= Cal_DiagTmpFltCfmDelay_ms_u16)
                {
                    result = 0;
                    debug_info.PTC_POWER_REDUCTION = 0;
                    bOverHeat_counter_B = 0;
                }
            }
            else
            {
                bOverHeat_counter_B = 0; 
            }
        }
    }
    return result;
}
void TempOffsetDetect(void)//100ms
{
    static uint8_t DryBurnCnt=0;
 if(TempOffsetDetect_OutAndIn())
    {
      if(control_cmd.PtcRunCmd==1)
      {
          if(debug_info.faultStatus_PTC.bit.M_ClntOtltOverTempProtn==0)
          {
               DryBurnCnt++;
               debug_info.faultStatus_PTC.bit.M_ClntOtltOverTempProtn=1;
          }
      }
    }
    else
    {
        if(DryBurnCnt<Cal_DiagDryBrnAutoRecLimit_cnt_u16)
        {
          debug_info.faultStatus_PTC.bit.M_ClntOtltOverTempProtn=0;
        }
    }
 
//  if(TempOffsetDetect_Oneself(debug_info.Temp_OUT)==1)||
//    {
//        DryBurnCnt=Cal_DiagDryBrnAutoRecLimit_cnt_u16+1;
//    }
}

void Wptc_OcFault_Detect(void)//WPTC��������  
{
  static uint16_t OcFault_Delay_time = 0;
  if((debug_info.PtcPeakCurrents>Cal_DiagPtcOcCurTrip_dA_u16)&&(g_pwm_thick_film>2))
  {
    if(OcFault_Delay_time>Cal_DiagPtcOcFltCfmDelay_ms_u16)//500MS
    {
      debug_info.faultStatus_PTC.bit.OcFault=1;
    }
    else
    {
      OcFault_Delay_time++;
    }
  }
  else
  {
    OcFault_Delay_time=0;
  }
}
void Wptc_AD_Current_Detect(void)//WPTC�������� ADֵ�ж�
{
  static uint16_t AD_Delay_time = 0;
  if(((g_ad_thick_film_current>Cal_DiagPtcAdCurMax_adc_u16)&&(g_pwm_thick_film>5))||
    ((g_ad_thick_film_current>Cal_DiagPtcAdCurMin_adc_u16)&&(g_pwm_thick_film<1)))
  {
    if(AD_Delay_time>Cal_DiagPtcAdCurFltCfmDelay_ms_u16) //500MS
    {
      debug_info.faultStatus_PTC.bit.M_AD_Current=1;
    }
    else
    {
      AD_Delay_time++;
    }
  }
  else
  {
    AD_Delay_time=0;
  }
}
void Wptc_HeatProtect_Detect(void)//pwm ���ʹܶ�·����  �ж������????>5Aʱ���� ©����   ����Ӧ�ضϸ�ѹ
{
  static uint16_t HeatProtect_Delay_time = 0;
  if((debug_info.PtcPeakCurrents_fast>Cal_DiagPtcHtProtCur_dA_u16)&&(g_pwm_thick_film<1)&&(debug_info.fault_status_flag_motor.bit.UvFault==0))
  {
    if(HeatProtect_Delay_time>Cal_DiagPtcHtProtCfmDelay_ms_u16)
    {
      debug_info.faultStatus_PTC.bit.HeatProtect=1;
    }
    else
    {
      HeatProtect_Delay_time++;
    }
  }
  else
  {
    HeatProtect_Delay_time=0;
  }
}
uint16_t Wptc_limit_Power(uint16_t Param_Temp)//�򵥳�ˮ�¶ȹ������ƹ��ʣ�����һ������
{
  uint16_t PTC_Power_Temp=0;
  if ((Param_Temp-40)>(Cal_DiagOutTmpTrip_degC_u16+5))
  {
    PTC_Power_Temp=debug_info.PTC_Power*0.3;
  }
  else  if((Param_Temp-40)>(Cal_DiagOutTmpTrip_degC_u16-10))
  {
    PTC_Power_Temp=debug_info.PTC_Power*0.5;
  }
  else if((Param_Temp-40)>(Cal_DiagOutTmpTrip_degC_u16-15))
  {
    PTC_Power_Temp=debug_info.PTC_Power*0.7;
  }
  else if((Param_Temp-40)>(Cal_DiagOutTmpTrip_degC_u16-20))
  {
    PTC_Power_Temp=debug_info.PTC_Power*0.9;
  } 
  else
  {
    PTC_Power_Temp=debug_info.PTC_Power;
  }
  return PTC_Power_Temp;
}
/**********************************************--COM�޹���--**********************************************************************************************************/

uint16_t Down_Speed_For_power_limit(void)//COM �޹���
{
     static uint16_t count1=0,count2=0;
     static uint8_t max_speed_down_spdCmd_Rpm_Flag=0;
     static uint16_t power_offer=0;
       if(debug_info.MotorState!=Ifx_MS_FocSolutionF16_State_run)
       {
           control_cmd.max_speed_down=MAXIMUM_SPEED_RPM;
           max_speed_down_spdCmd_Rpm_Flag=0;
            power_offer=0;
           count1=0;
           count2=0;
       }
      if((debug_info.COM_Power>control_cmd.Max_power_acc)&&(flag_downspeed_by_AC == 1 )) //???
        {
          if(control_cmd.max_speed_down>control_cmd.spdCmd_Rpm&&max_speed_down_spdCmd_Rpm_Flag==0)
          {
             control_cmd.max_speed_down= control_cmd.spdCmd_Rpm;
             max_speed_down_spdCmd_Rpm_Flag=1;
          }
            count1++;
            if(count1>=10)
                {
                    count1= 0;
                    control_cmd.max_speed_down--;
                    //flag_down_speed = 1;
                    count2=0;
                }
            if( control_cmd.max_speed_down<=Cal_DiagMotStartMinSpd_rpm_u16) control_cmd.max_speed_down = Cal_DiagMotStartMinSpd_rpm_u16;
        }
      else
        {
          if(control_cmd.Max_power_acc>8000)
          {
              if(control_cmd.max_speed_down<(control_cmd.spdCmd_Rpm+500))
              {
                    control_cmd.max_speed_down+=1;
              }
               max_speed_down_spdCmd_Rpm_Flag=0;
          }
          else
          {
            count2++;
            if(count2>=100)//10
                { 
                    count2=0;
                    if(debug_info.COM_Power<control_cmd.Max_power_acc)
                    {
                      power_offer=control_cmd.Max_power_acc-debug_info.COM_Power;
                    }
                    else
                    {
                      power_offer=0;
                    }
                    if(power_offer>300)
                    {
                        if(control_cmd.max_speed_down<(control_cmd.spdCmd_Rpm+500))
                        {
                         control_cmd.max_speed_down+=10;
                        }
                    }
                    else  if(power_offer>0)
                    {
                        if(control_cmd.max_speed_down<(control_cmd.spdCmd_Rpm+500))
                        {
                         control_cmd.max_speed_down+=1;
                        }
                    }
                     max_speed_down_spdCmd_Rpm_Flag=0;
                }
          }
        }
        if( control_cmd.max_speed_down>=MAXIMUM_SPEED_RPM)
        {
            control_cmd.max_speed_down = MAXIMUM_SPEED_RPM;
        }
     uint16_t qTargetVelocity =control_cmd.spdCmd_Rpm;
        if( qTargetVelocity>=control_cmd.max_speed_down)
        {
           qTargetVelocity =control_cmd.max_speed_down;
        }
        return qTargetVelocity;
 }
/**********************************************--COM���ϱ���--**********************************************************************************************************/

void Can_Communication_OvTime_Detect(void)//CANͨѶ��ʱ���????
{
  if(Cal_DiagCanCommLostMonEn_u8 == 0u)
  {
    debug_info.Cal_CanCommLostTimeout_ms_u16_Count=0;
    debug_info.fault_status_flag_motor.bit.ERR_OUTCAN=0;
    debug_info.faultStatus_PTC.bit.ERR_OUTCAN=0;
    return;
  }

  if(debug_info.Cal_CanCommLostTimeout_ms_u16_Count>=Cal_DiagCanCommLostTout_ms_u16)
  {
    debug_info.fault_status_flag_motor.bit.ERR_OUTCAN=1;
    debug_info.faultStatus_PTC.bit.ERR_OUTCAN=1;
  }
  else
  {
     debug_info.Cal_CanCommLostTimeout_ms_u16_Count++;
  }
}

void Err_event_vol(void) //��Ƿѹ����
{
     static uint32_t  bDCBUS_OV_counter = 0;
     static uint32_t  bDCBUS_LV_counter = 0;
     static uint32_t  bDCBUS_OV_counter_back = 0;
     static uint32_t  bDCBUS_LV_counter_back = 0;
     static uint8_t   start_cmd_flag=0;
         if(control_cmd.runCmd||control_cmd.PtcRunCmd)
         {
           start_cmd_flag=1;
         }
         else
         {
           start_cmd_flag=0;
         }
	if(debug_info.pVdcValue > Cal_DiagDcBusOvTrip_V_u16)
	{
		bDCBUS_OV_counter++;			
		if (bDCBUS_OV_counter >= Cal_DiagDcBusOvCfmDelay_ms_u16)
		{	
			bDCBUS_OV_counter=0;
			debug_info.fault_status_flag_motor.bit.OvFault=1;
		}
	}
	else if((debug_info.pVdcValue <= Cal_DiagDcBusOvRec_V_u16) &&(debug_info.fault_status_flag_motor.bit.OvFault))//
	{
		bDCBUS_OV_counter_back++;
		if(bDCBUS_OV_counter_back>=Cal_DiagDcBusFltRecDelay_ms_u16)
			{
				debug_info.fault_status_flag_motor.bit.OvFault=0;
				bDCBUS_OV_counter_back=0;
			}
	}
	else
	{
		bDCBUS_OV_counter_back = 0;
		bDCBUS_OV_counter = 0;
	}
    

	if((debug_info.pVdcValue < Cal_DiagDcBusUvTrip_V_u16)&&(start_cmd_flag))
	{	
		bDCBUS_LV_counter++;			
		if (bDCBUS_LV_counter >= Cal_DiagDcBusUvCfmDelay_ms_u16)
		{
			bDCBUS_LV_counter=0;
			debug_info.fault_status_flag_motor.bit.UvFault=1;
		}
	}
	else if((debug_info.pVdcValue <= Cal_DiagDcBusUvQkTrip_V_u16)&&(start_cmd_flag))
	{	
                debug_info.fault_status_flag_motor.bit.UvFault=1;
	}
	else if((debug_info.pVdcValue >= Cal_DiagDcBusUvRec_V_u16)&&(debug_info.fault_status_flag_motor.bit.UvFault))
	{
              bDCBUS_LV_counter_back++;
              if(bDCBUS_LV_counter_back>=Cal_DiagDcBusFltRecDelay_ms_u16)
                    {
                            debug_info.fault_status_flag_motor.bit.UvFault=0;
                            bDCBUS_LV_counter_back=0;
                    }
	}
	else
	{
                bDCBUS_LV_counter_back=0;
                bDCBUS_LV_counter = 0;
	}
}

void COM_CurrRMSDetect(void)//motor������������-������Чֵ�������ֵ��ĸ�ߵ��������ٹ���????
{
    static uint16_t PhaseMaxCount=0;
    static uint16_t PhasePeakMaxCount=0;
    static uint16_t BusMaxCount=0;
    static uint16_t OverLoadMaxCount=0;
    uint8_t fault_status_changed = 0u;

    if (Cal_DiagPhCurRmsEn_u8 == 0u)
    {
        if ((PhaseMaxCount != 0u)
            || (debug_info.fault_status_flag_motor.bit.OvSoftRmsCurrent != 0u))
        {
            fault_status_changed = 1u;
        }
        PhaseMaxCount = 0u;
        debug_info.fault_status_flag_motor.bit.OvSoftRmsCurrent = 0u;
    }

    if (Cal_DiagPhPkCurEn_u8 == 0u)
    {
        if ((PhasePeakMaxCount != 0u)
            || (debug_info.fault_status_flag_motor.bit.OvSoftwarePeakCurrent != 0u))
        {
            fault_status_changed = 1u;
        }
        PhasePeakMaxCount = 0u;
        debug_info.fault_status_flag_motor.bit.OvSoftwarePeakCurrent = 0u;
    }

    if (Cal_DiagOvldEn_u8 == 0u)
    {
        if ((OverLoadMaxCount != 0u)
            || (debug_info.fault_status_flag_motor.bit.OvLoadFault != 0u))
        {
            fault_status_changed = 1u;
        }
        OverLoadMaxCount = 0u;
        debug_info.fault_status_flag_motor.bit.OvLoadFault = 0u;
    }

    if(debug_info.MotorState!=Ifx_MS_FocSolutionF16_State_run)
    {
        PhaseMaxCount=0;PhasePeakMaxCount=0;BusMaxCount=0;OverLoadMaxCount=0;
        if (fault_status_changed != 0u)
        {
            Motor_Fault_All_Set((uint32_t)(debug_info.fault_status_flag_motor.Word));
        }
        return;
    }

    if ((Cal_DiagPhCurRmsEn_u8 != 0u)
        && (debug_info.PhaseCurrent>=Cal_DiagPhCurTrip_A_u16))
    {
         if(PhaseMaxCount>Cal_DiagPhCurCfmDelay_cnt_u16)
          {
             PhaseMaxCount=0;
             debug_info.fault_status_flag_motor.bit.OvSoftRmsCurrent=1;
          }
         else
         {
             PhaseMaxCount++;
         }
    }
    else
    {
        PhaseMaxCount=0;
    }
     if ((Cal_DiagPhPkCurEn_u8 != 0u)
         && (debug_info.PhasePeakCurrent>=Cal_DiagPhPkCurTrip_A_u16))
    {
         if(PhasePeakMaxCount>Cal_DiagPhCurCfmDelay_cnt_u16)
          {
             PhasePeakMaxCount=0;
             debug_info.fault_status_flag_motor.bit.OvSoftwarePeakCurrent=1;
          }
         else
         {
             PhasePeakMaxCount++;
         }
    }
    else
    {
        PhasePeakMaxCount=0;
    }
//   if(debug_info.BusCurrent>=Cal_DiagBusCurTrip_dA_u16)
//    {
//         if(BusMaxCount>Cal_DiagBusCurCfmDelay_cnt_u16)
//          {
//             BusMaxCount=0;
//             debug_info.fault_status_flag_motor.bit.OvBusCurrent=1;
//          }
//         else
//         {
//             BusMaxCount++;
//         }
//    }
//    else
//    {
//        BusMaxCount=0;
//    }
    if ((Cal_DiagOvldEn_u8 != 0u)
        && (debug_info.speed_actal<Cal_DiagOvldSpdThd_rpm_u16))//���ٹ���
    {
      if(debug_info.PhaseCurrent>=Cal_DiagOvldPhCurThd_A_u16)
      {
           if(OverLoadMaxCount>Cal_DiagBusCurCfmDelay_cnt_u16)
            {
               OverLoadMaxCount=0;
               debug_info.fault_status_flag_motor.bit.OvLoadFault=1;
            }
           else
           {
               OverLoadMaxCount++;
           }
      }
      else
      {
          OverLoadMaxCount=0;
      }
    }

    if (fault_status_changed != 0u)
    {
        Motor_Fault_All_Set((uint32_t)(debug_info.fault_status_flag_motor.Word));
    }
}

bool OvCurrentHappened(void)
{
      if((debug_info.fault_status_flag_motor.bit.OcFault != 0)
          ||(debug_info.fault_status_flag_motor.bit.OvBusCurrent != 0)
          ||(debug_info.fault_status_flag_motor.bit.OvSoftRmsCurrent != 0)
          ||(debug_info.fault_status_flag_motor.bit.OvSoftwarePeakCurrent != 0)
          ||(debug_info.fault_status_flag_motor.bit.OvLoadFault != 0))
            {
                return true;
            }
            else
            {
                return false;
            }
}

void OvCurrentClear(void)//�����������????
{
  debug_info.fault_status_flag_motor.bit.OcFault = 0;
  debug_info.fault_status_flag_motor.bit.OvBusCurrent = 0;
  debug_info.fault_status_flag_motor.bit.OvSoftRmsCurrent = 0;
  debug_info.fault_status_flag_motor.bit.OvSoftwarePeakCurrent = 0;
  debug_info.fault_status_flag_motor.bit.OvLoadFault = 0;
  debug_info.fault_status_flag_motor.bit.PowerOverFault = 0;
}

void COM_PowerOverFault_Detect(void)
{
    static uint16_t power_over_cnt = 0;
    static uint16_t curr_run_speed = 0;
    static uint32_t fault_restart_timer = 0;
    static uint8_t  fault_restart_cnt = 0;
    static uint32_t fault_lock_timer = 0;
    static uint8_t  fault_lock_flag = 0;
    static uint32_t fault_restart_reset_timer = 0;
    static uint16_t dir_lock_cnt = 0;
    static uint16_t zone0_probe_cnt = 0;
    static uint8_t  last_zone = 0;
    static uint8_t  power_drop_flag = 0;

    uint16_t target_speed = control_cmd.spdCmd_Rpm;
    uint16_t power_limit = control_cmd.Max_power_acc;

    if (power_limit == 0)
    {
        power_limit = Cal_DiagPwrOvDef_W_u16;
    }

    if (target_speed > MAXIMUM_SPEED_RPM) target_speed = MAXIMUM_SPEED_RPM;
    if (target_speed < Cal_DiagMotStartMinSpd_rpm_u16) target_speed = Cal_DiagMotStartMinSpd_rpm_u16;

    if (Cal_DiagPwrOvEn_u8 == 0u)
    {
        power_over_cnt = 0u;
        curr_run_speed = target_speed;
        fault_restart_timer = 0u;
        fault_restart_cnt = 0u;
        fault_lock_timer = 0u;
        fault_lock_flag = 0u;
        fault_restart_reset_timer = 0u;
        dir_lock_cnt = 0u;
        zone0_probe_cnt = 0u;
        last_zone = 0u;
        power_drop_flag = 0u;
        debug_info.fault_status_flag_motor.bit.PowerOverFault = 0u;
        control_cmd.max_speed_down = target_speed;
        return;
    }

    if ((curr_run_speed == 0) || (curr_run_speed > MAXIMUM_SPEED_RPM))
    {
        curr_run_speed = target_speed;
    }

    if (Cal_DiagPwrOvDnSpdEn_u8 == 0u)
    {
        /* Keep fault monitoring active while bypassing speed reduction. */
        curr_run_speed = target_speed;
        dir_lock_cnt = 0u;
        zone0_probe_cnt = 0u;
        power_drop_flag = 0u;
        control_cmd.max_speed_down = target_speed;
    }

    if (debug_info.MotorState != Ifx_MS_FocSolutionF16_State_run)
    {
        curr_run_speed = target_speed;
        power_over_cnt = 0;
        dir_lock_cnt = 0;
        zone0_probe_cnt = 0;
        last_zone = 0;
        power_drop_flag = 0;
        fault_restart_reset_timer = 0;

        if (debug_info.fault_status_flag_motor.bit.PowerOverFault
            && (fault_lock_flag == 0)
            && (fault_restart_cnt < Cal_DiagPwrOvAutoRecLimit_cnt_u16))
        {
            fault_restart_timer++;
            if (fault_restart_timer >= Cal_DiagPwrOvFltRstrtDelay_cnt_u16)
            {
                fault_restart_timer = 0;
                fault_restart_cnt++;
                debug_info.fault_status_flag_motor.bit.PowerOverFault = 0;
            }
        }
        else
        {
            fault_restart_timer = 0;
        }

        if (fault_lock_flag)
        {
            fault_lock_timer++;
            if (fault_lock_timer >= Cal_DiagPwrOvFltRstDelay_cnt_u16)
            {
                fault_lock_timer = 0;
                fault_lock_flag = 0;
                fault_restart_cnt = 0;
            }
        }
        return;
    }

    if (fault_lock_flag)
    {
        return;
    }

    if (curr_run_speed > target_speed)
    {
        curr_run_speed = target_speed;
        dir_lock_cnt = 0;
    }

    if (dir_lock_cnt > 0)
    {
        dir_lock_cnt--;
    }

    uint8_t current_zone;

    if (debug_info.COM_Power > power_limit)
    {
        current_zone = 2;
    }
    else if (debug_info.COM_Power > (power_limit - Cal_DiagPwrOvHys_W_u16))
    {
        current_zone = 1;
    }
    else
    {
        current_zone = 0;
    }

    if (current_zone != 0 && last_zone == 0)
    {
        power_drop_flag = 1;
        zone0_probe_cnt = 0;
    }

    if (current_zone == 2)
    {
        power_over_cnt++;
        fault_restart_reset_timer = 0;

        if ((Cal_DiagPwrOvDnSpdEn_u8 != 0u) && (dir_lock_cnt == 0))
        {
            if (curr_run_speed > Cal_DiagPwrOvMinSpd_rpm_u16)
            {
                if (curr_run_speed > (Cal_DiagPwrOvMinSpd_rpm_u16 + Cal_DiagPwrOvStepDn_rpmPer100ms_u16))
                    curr_run_speed -= Cal_DiagPwrOvStepDn_rpmPer100ms_u16;
                else
                    curr_run_speed = Cal_DiagPwrOvMinSpd_rpm_u16;
                dir_lock_cnt = Cal_DiagPwrOvSpdDirLock_cnt_u16;
            }
        }

        if (power_over_cnt >= Cal_DiagPwrOvFltCfm_cnt_u16)
        {
            power_over_cnt = Cal_DiagPwrOvFltCfm_cnt_u16;
            if (fault_restart_cnt >= Cal_DiagPwrOvAutoRecLimit_cnt_u16)
            {
                fault_lock_flag = 1;
                fault_restart_cnt = 0;
            }
            debug_info.fault_status_flag_motor.bit.PowerOverFault = 1;
        }
    }
    else if (current_zone == 1)
    {
        power_over_cnt = 0;
        fault_restart_reset_timer = 0;

        if ((Cal_DiagPwrOvDnSpdEn_u8 != 0u) && (dir_lock_cnt == 0))
        {
            if (curr_run_speed > Cal_DiagPwrOvMinSpd_rpm_u16)
            {
                if (curr_run_speed > (Cal_DiagPwrOvMinSpd_rpm_u16 + (Cal_DiagPwrOvStepDn_rpmPer100ms_u16 / 2)))
                    curr_run_speed -= (Cal_DiagPwrOvStepDn_rpmPer100ms_u16 / 2);
                else
                    curr_run_speed = Cal_DiagPwrOvMinSpd_rpm_u16;
                dir_lock_cnt = Cal_DiagPwrOvSpdDirLock_cnt_u16;
            }
        }
    }
    else
    {
        power_over_cnt = 0;
        fault_restart_reset_timer++;
        if (fault_restart_reset_timer >= Cal_DiagPwrOvFltRstDelay_cnt_u16)
        {
            fault_restart_reset_timer = 0;
            fault_restart_cnt = 0;
        }

        if ((Cal_DiagPwrOvDnSpdEn_u8 != 0u) && power_drop_flag)
        {
            zone0_probe_cnt++;
            if (zone0_probe_cnt >= Cal_DiagPwrOvZ0ProbeCfm_cnt_u16)
            {
                if ((dir_lock_cnt == 0) && (curr_run_speed < target_speed))
                {
                    curr_run_speed += Cal_DiagPwrOvStepUp_rpmPer100ms_u16;
                    if (curr_run_speed > target_speed)
                        curr_run_speed = target_speed;
                    dir_lock_cnt = Cal_DiagPwrOvSpdDirLock_cnt_u16;
                }
                zone0_probe_cnt = 0;
                power_drop_flag = (curr_run_speed >= target_speed) ? 0 : 1;
            }
        }
        else if (Cal_DiagPwrOvDnSpdEn_u8 != 0u)
        {
            zone0_probe_cnt = 0;
            if ((dir_lock_cnt == 0) && (curr_run_speed < target_speed))
            {
                curr_run_speed += Cal_DiagPwrOvStepUp_rpmPer100ms_u16;
                if (curr_run_speed > target_speed)
                    curr_run_speed = target_speed;
            }
        }

        debug_info.fault_status_flag_motor.bit.PowerOverFault = 0;
        fault_lock_timer = 0;
    }

    last_zone = current_zone;

    if (curr_run_speed > MAXIMUM_SPEED_RPM)
        curr_run_speed = MAXIMUM_SPEED_RPM;
    if (curr_run_speed < Cal_DiagPwrOvMinSpd_rpm_u16)
        curr_run_speed = Cal_DiagPwrOvMinSpd_rpm_u16;

    if (Cal_DiagPwrOvDnSpdEn_u8 == 0u)
        curr_run_speed = target_speed;

    control_cmd.max_speed_down = curr_run_speed;
}

void  Restart_motor(void)//�����ָ�
{
  static uint8_t restarttimes = 0,lock_flag=0,count_restart=0;
  static uint32_t count_restart_back = 0;
  static uint32_t time_for_restart_back = 0 ;
  if((OvCurrentHappened())&&(lock_flag==0))
    {
      if(count_restart == 0)
          {
            count_restart = 1;
            restarttimes ++;
            time_for_restart_back++;
            if(restarttimes>=Cal_DiagOcAutoRecLimit_cnt_u16)//6��
              {
                debug_info.OCS_Fault_Lock_flag=1;
                lock_flag=1;
              }
            else
            {
              if(time_for_restart_back>Cal_DiagOcAutoRecDelay_ms_u16)
              {
                 time_for_restart_back=0;
                OvCurrentClear();
              }
            }
          }
    }
  else if((count_restart == 1)&&(!OvCurrentHappened()))
    {
            count_restart = 0;
    }
  if(restarttimes>0 && (debug_info.MotorState == Ifx_MS_FocSolutionF16_State_run)) 
    {
       count_restart_back++;              
       if(count_restart_back>=Cal_DiagOcRstCntClrDelay_ms_u32)
          {		
            restarttimes = 0;	
            count_restart_back = 0;	
          }
    }
    else
    {
        count_restart_back = 0;
    }
}

void motorStop(uint8_t flag)//flag=0 ����ͣ�� ��flag=1 ����ͣ��
{
  static uint16_t Stop_Delay_Timer_Count=0;
  if(flag)
  {
    if(debug_info.speed_actal>STOP_SPEED_MIN)
    {
      control_cmd.spdCmd_Rpm=STOP_SPEED_MIN;
    }
    else
    {
      control_cmd.runCmd=0;
      control_cmd.spdCmd_Rpm=0;
      Stop_Delay_Timer_Count=0;
    }
    if(Stop_Delay_Timer_Count>STOP_DELAY_TIMER)
    {
      control_cmd.runCmd=0;
      control_cmd.spdCmd_Rpm=0;
      Stop_Delay_Timer_Count=0;
    }
  }
  else
  {
      control_cmd.runCmd=0;
      control_cmd.spdCmd_Rpm=0;
      Stop_Delay_Timer_Count=0;
  }
}

uint16_t ad_to_voltage(uint16_t ad_value)
{
  static uint16_t vol_value=0;
    if (ad_value > 4096)
    {
        return 0;
    }
  vol_value = (ad_value *1650) / 4096.0f+1;
    return vol_value;
}
uint16_t ad_to_current(uint16_t ad_value)
{
  static uint16_t cur_value=0;
    if (ad_value > 4096)
    {
        return 0;
    }
  cur_value = (ad_value *360) / 4096.0f;//360 ����10��
    return cur_value;
}
float calc_smooth_temp_scale(uint16_t adc_buf_therm)
{
    if(adc_buf_therm>870)
    {
      return 1.24f;
    }
    else if(adc_buf_therm>630)
    {
      return 1.214f;
    }
    else if(adc_buf_therm>145)
    {
      return 1.18f;
    }
    else
    {
      const ThermScaleMap therm_scale_map[] = 
      {
        {133, 1.11},
        {118, 1.10},
        {107, 1.02},
        {98,  0.96},
        {86,  0.88}
      };
      if (adc_buf_therm >= therm_scale_map[0].adc_value)
      {
          return therm_scale_map[0].scale_value;
      }
      if (adc_buf_therm <= therm_scale_map[MAP_COUNT - 1].adc_value)
      {
          return therm_scale_map[MAP_COUNT - 1].scale_value;
      }
      for (int i = 0; i < MAP_COUNT - 1; i++) 
      {
          if (adc_buf_therm <= therm_scale_map[i].adc_value && 
              adc_buf_therm >= therm_scale_map[i+1].adc_value)
          {              
              // ���Բ�ֵ��ʽ��y = y1 + (y2 - y1) * (x - x1) / (x2 - x1)
              float x1 = therm_scale_map[i].adc_value;
              float y1 = therm_scale_map[i].scale_value;
              float x2 = therm_scale_map[i+1].adc_value;
              float y2 = therm_scale_map[i+1].scale_value;
              float x = adc_buf_therm;

              float scale = y1 + (y2 - y1) * (x - x1) / (x2 - x1);
              return scale;
          }
      }
    }
    return 1.214f;
}

uint16_t Temp_ADtoTemp(uint16_t ADCBUF_THERM)
{
  uint32_t fltAdLow_TepV=0;
  uint16_t result_Temp=0;
 // fltAdLow_TepV = (uint32_t)(((ADCBUF_THERM)*3*8)*VREF/Resolu*2*1000);
  fltAdLow_TepV = (uint32_t)(ADCBUF_THERM * calc_smooth_temp_scale(ADCBUF_THERM));//TEMP_SCALE
	long  rr = 0;
	long  rr2=0;
	long  tt = 0;
	long  rr1 = 0;
	rr1 = (20000*fltAdLow_TepV) / (5000-fltAdLow_TepV);
	rr = (10000*rr1)/(10000-rr1);
	rr2 = rr / 10;
    if(rr > 190500)
    {
        result_Temp = -40+40;
    }
    else if(rr>99000) //-40~-27
    {
	   result_Temp =(195320-rr)/(7147)+2;//(rr+94800)/(-7000);
    }
    else if(rr > 56000 ) //-26~-16
    {
        result_Temp =(96127-rr)/(3581)+14+1;//(rr+25000)/(-4600);
    }
    else if(rr >29000 )  //-15~-1
    {
        result_Temp = (55676-rr)/(1868)+25;//(rr-24000)/(-2000);
    }
   else if(rr >20800 ) //0~7
    {
        result_Temp = (28500-rr)/(1083)+40;//(rr-28600)/(-1000);
    }
     else if(rr >12500 )//8~20
    {
        result_Temp = (20400-rr)/(582)+48;//(rr-28900)/(-800);
    }
     else
     {
        tt = (1830000 + 254*rr - (159*rr2*rr2/100)) / (rr + 1148 );
        result_Temp = (int)tt;
        result_Temp = result_Temp/10+40;
     }
     return  result_Temp;
}

void Fault_Detect(void)//���ϼ��???? 1ms
{
  Err_event_temple();
  TempOffsetDetect();
  // TempOffsetDetect_Oneself(debug_info.Temp_OUT);
  // TempOffsetDetect_Oneself(debug_info.Temp_IN);
  Wptc_OcFault_Detect();
  Wptc_AD_Current_Detect();
  Wptc_HeatProtect_Detect();
  Can_Communication_OvTime_Detect();
  Err_event_vol();
  COM_CurrRMSDetect();
}

void Execute_Funtion_1ms(void)
{
    wptc_pwm_gradient_control(control_cmd.PtcRunCmd,debug_info.DutyPoint);
//    PWM_DUTY();
       getDebugInfoData_1ms();
}

void Execute_Funtion_500ms(void)
{
   static uint16_t q_pwm_thick_film=0;
    static uint16_t count_500=0;
    count_500++;
    if(count_500>100)
    {
      count_500=0;
      debug_info.DutyPoint=ThickFilm_Heat_Ctrl(debug_info.PTC_Power,debug_info.pVdcValue);
     // debug_info.DutyPoint=PTC_Controller_Update(control_cmd.PtcRunCmd);
    check_resistance_protection();
       getDebugInfoData_100ms();
    }
}
