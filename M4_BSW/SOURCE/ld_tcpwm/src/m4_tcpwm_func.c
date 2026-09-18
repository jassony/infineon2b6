/*
 * ld_tcpwm.c
 *
 *  Created on: 2026年1月27日
 *      Author: hzldy
 */
#include "m4_tcpwm_func.h"


void Tcpwm_Enable_Func(void)
{
    Cy_Tcpwm_Counter_Enable(TCPWM0_GRP0_CNT0);
  
    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_U);
    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_V);
    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_W);
    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_CC0);
    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_CC1);

//    Cy_Tcpwm_TriggerStart(TCPWMx_GRPx_CNTx_U);
//    Cy_Tcpwm_TriggerStart(TCPWMx_GRPx_CNTx_V);
//    Cy_Tcpwm_TriggerStart(TCPWMx_GRPx_CNTx_W);
    Cy_Tcpwm_TriggerStart(TCPWM0_GRP0_CNT0);  
    Cy_Tcpwm_TriggerStart(TCPWMx_GRPx_CNTx_U);  

    /* Enable Interrupt */
    Cy_Tcpwm_Counter_SetTC_IntrMask(TCPWM0_GRP0_CNT0);
//    Cy_Tcpwm_Counter_SetTC_IntrMask(TCPWMx_GRPx_CNTx_U);
    TCPWMx_GRPx_CNTx_CC0->unTR_OUT_SEL.stcField.u3OUT1 = CY_TCPWM_COUNTER_CC0_MATCH; //触发源选择trigger1EventCfg
  //TCPWM0_16_TR_OUT1对应u3OUT1
    TCPWMx_GRPx_CNTx_CC1->unTR_OUT_SEL.stcField.u3OUT1 = CY_TCPWM_COUNTER_CC1_MATCH;

}

