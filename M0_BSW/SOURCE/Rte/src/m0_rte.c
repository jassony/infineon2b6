/*
 * rte.c
 *
 *  Created on: 2026Äê2ÔÂ24ÈÕ
 *      Author: hzldy
 */


#include "m0_rte.h"

static void Rte_Bsw_To_Swc(void);
static void Rte_Swc_To_Bsw(void);

void Rte_Process(void)
{
    Rte_Bsw_To_Swc();
    Rte_Swc_To_Bsw();
}

static void Rte_Bsw_To_Swc(void)
{
  g_pin_low_voltage_overvoltage = Cy_GPIO_Read(GPIO_OVERVOLT_IN_PORT, GPIO_OVERVOLT_IN_PIN);
  g_pin_low_voltage_undervoltage = Cy_GPIO_Read(GPIO_UNDERVOLT_IN_PORT, GPIO_UNDERVOLT_IN_PIN);
  g_ad_thick_film_current = Adc_Original_Val_Get(ADC_HEAT_M);
  g_ad_outlet_water_temp = Adc_Original_Val_Get(ADC_HEAT_OUT);
  g_ad_inlet_water_temp = Adc_Original_Val_Get(ADC_HEAT_IN);
  g_ad_compressor_SIC_temp = Adc_Original_Val_Get(ADC_T_SENSE_SIC_COMPRESS);
  g_ad_thick_film_SIC_temp = Adc_Original_Val_Get(ADC_T_SENSE_SIC_HEAT);
  g_ad_PCB_temp = Adc_Original_Val_Get(ADC_PCB_SENSE);
//  Expect_Speed_Set(100);
//  Power_Status_Set(1);
//  Motor_Err_Clear_Set(1);
//  Motor_Control_Set(1,2,3);
//  Acceleration_Time_Set(1000);
//  Reduction_Time_Set(500);
  
}


static void Rte_Swc_To_Bsw(void)
{
    Tcpwm_Thick_Control_Set(g_pwm_thick_film);
//    THICK_PWM(1);

}
