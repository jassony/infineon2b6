#ifndef HFIPD_INJECTION_H
#define HFIPD_INJECTION_H
#include "hfipd_core.h"
extern volatile uint8_t Cal_HFIPD_Enable_u8;
extern volatile uint16_t Cal_HFIPD_Seq_u16;
extern volatile float Cal_HFIPD_Hf_V_f32, Cal_HFIPD_Pulse_V_f32;
extern volatile float Cal_HFIPD_Kp_radpsperA_f32, Cal_HFIPD_Ki_radps2perA_f32;
extern volatile float Cal_HFIPD_Init_rad_f32, Cal_HFIPD_Delay_tick_f32;
extern volatile float Cal_HFIPD_Contrast_PU_f32, Cal_HFIPD_AxisHi_rad_f32, Cal_HFIPD_IsHi_A_f32;
extern volatile uint16_t Cal_HFIPD_Track_tick_u16, Cal_HFIPD_Settle_tick_u16;
extern volatile uint16_t Cal_HFIPD_Pulse_tick_u16, Cal_HFIPD_Tail_tick_u16, Cal_HFIPD_Gap_tick_u16;
extern volatile uint8_t Meas_HFIPD_Act_u8, Meas_HFIPD_Valid_u8, Meas_HFIPD_Stat_u8;
extern volatile float Meas_HFIPD_Init_rad_f32, Meas_HFIPD_Track_rad_f32;
extern volatile float Meas_HFIPD_Pos_As_f32, Meas_HFIPD_Neg_As_f32, Meas_HFIPD_Axis_rad_f32;
extern volatile float Meas_HFIPD_Err_A_f32, Meas_HFIPD_Hf_A_f32, Meas_HFIPD_Out_V_f32;
extern volatile uint32_t Meas_HFIPD_Time_tick_u32;
/* begin/consume/cancel: 2 kHz only. begin is the stopped->run boundary,
 * before the first fast callback. Same preemption priority excludes nesting. */
void HFIPDInjection_begin(void);
void HFIPDInjection_cancel(void);
uint8_t HFIPDInjection_isActive(void);
uint8_t HFIPDInjection_isComplete(void);
/* Leaves *angleQ32 unchanged unless a valid result is consumed once. */
uint8_t HFIPDInjection_consume(uint32_t *angleQ32);
/* 20 kHz only while isActive; SI inputs, signed injection magnitude and angle. */
void HFIPDInjection_execute(float alphaA,float betaA,float availableV,
                            float *voltageV,uint32_t *angleQ32);
#endif
