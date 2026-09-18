/*
 * m4_adc_func.c
 *
 *  Created on: 2026年1月28日
 *      Author: hzldy
 */


#include "m4_adc_func.h"
#include "SDL_Adc_Cfg.h"

void Adc_Enable_Func(void)
{
//  Cy_Adc_Channel_Disable(&PASS0_SAR0->CH[ADC_CHN_NUM_CC1]);
  Cy_Adc_Channel_Disable(&ADC_SAR_NUM_CC0->CH[ADC_CHN_NUM_CC0]);
  Cy_Adc_Channel_Disable(&ADC_SAR_NUM_CC1->CH[ADC_CHN_NUM_CC1]);
  Cy_Adc_Channel_Disable(&ADC_SAR_NUM_VDC->CH[ADC_CHN_NUM_VDC]);
  
  Cy_Adc_SetGenericTriggerInput(PASS0_EPASS_MMIO, ADC_NUM_CC1, 0u, 4);//全局变量，adc1,通用触发器0，触发IN4          第一个0代表是通用触发器0，因为CC1 ADC配置的时候，配置的就是CY_ADC_TRIGGER_GENERIC0。   第二个4代表的是IN4通道触发输入，需要与TCPWM进行关联 
  Cy_Adc_SetGenericTriggerInput(PASS0_EPASS_MMIO, ADC_NUM_VDC, 0u, 4);  //绑定adc通道

  Cy_Adc_Channel_Enable(&ADC_SAR_NUM_CC0->CH[ADC_CHN_NUM_CC0]);
  Cy_Adc_Channel_Enable(&ADC_SAR_NUM_CC1->CH[ADC_CHN_NUM_CC1]);
  Cy_Adc_Channel_Enable(&ADC_SAR_NUM_VDC->CH[ADC_CHN_NUM_VDC]);
  
  Cy_TrigMux_Connect1To1(TRIG_IN_1TO1_1_TCPWM_TO_PASS_CH_TR5,CY_TR_MUX_TR_INV_DISABLE,TRIGGER_TYPE_PASS_TR_SAR_CH_IN__EDGE,0u); 
  Cy_TrigMux_Connect(TRIG_IN_MUX_6_TCPWM_16M_TR_OUT14, TRIG_OUT_MUX_6_PASS_GEN_TR_IN4, 0u,TRIGGER_TYPE_PASS_TR_SAR_CH_IN__EDGE, 0u); 
  //用TRIG_IN_MUX_6_TCPWM_16M_TR_OUT14触发IN4


}


