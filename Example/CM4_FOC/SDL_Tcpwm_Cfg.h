/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#ifndef SDL_TCPWM_CFG_H
#define SDL_TCPWM_CFG_H

#include "cy_device_headers.h"
#include "main_cm4.h"
#include "m4_tcpwm_func.h"
#ifndef MMEK_USE_SIX_PWM
#error "MMEK_USE_SIX_PWM not defined"
#endif

/* Peripheral clock for TCPWM channel which is used to trigger the speed loop */
#define PCLK_TCPWM_SPEEDLOOP PCLK_TCPWM0_CLOCKS0

///* Peripheral clock for TCPWM channel which is used as ADC CC0 trigger */
//#define PCLK_TCPWM_CC0 PCLK_TCPWM0_CLOCKS1
//
///* Peripheral clock for TCPWM channel which is used as ADC CC1 trigger */
//#define PCLK_TCPWM_CC1 PCLK_TCPWM0_CLOCKS260

///* TCPWM channel used to trigger phase current measurement on CC0 match */
//#define TCPWMx_GRPx_CNTx_CC0 TCPWM0_GRP0_CNT1
//
///* TCPWM channel used to trigger phase current measurement on CC1 match */
//#define TCPWMx_GRPx_CNTx_CC1 TCPWM0_GRP1_CNT4
//
///* TCPWM channel number used to trigger phase current measurement on CC0 match */
//#define TCPWM_CHN_NUM_CC0 3u
//
///* TCPWM channel number used to trigger phase current measurement on CC1 match */
//#define TCPWM_CHN_NUM_CC1 10u
//
///* Trigger In MUX channel for output of TCPWM CC0 */
//#define TRIG_IN_MUX_TCPWM_CC0 TRIG_IN_MUX_6_TCPWM_16M_TR_OUT13
//
///* Trigger In MUX channel for output of TCPWM CC1 */
//#define TRIG_IN_MUX_TCPWM_CC1 TRIG_IN_MUX_6_TCPWM_16M_TR_OUT110
//
///* Trigger Out MUX channel for input of ADC CC0 */
//#define TRIG_OUT_MUX_ADC_CC0 TRIG_OUT_MUX_6_PASS_GEN_TR_IN3
//
///* Trigger Out MUX channel for input of ADC CC1 */
//#define TRIG_OUT_MUX_ADC_CC1 TRIG_OUT_MUX_6_PASS_GEN_TR_IN10
//
/* TCPWM channel which is used to trigger the speed loop */
#define TCPWM_GRPx_CNTx_SPEEDLOOP TCPWM0_GRP0_CNT0
//
//#if (MMEK_USE_SIX_PWM == 0)
//
//#define PCLK_TCPWM_U PCLK_TCPWM0_CLOCKS1
//#define PCLK_TCPWM_V PCLK_TCPWM0_CLOCKS2
//#define PCLK_TCPWM_W PCLK_TCPWM0_CLOCKS3
//
//#define TCPWMx_GRPx_CNTx_U TCPWM0_GRP0_CNT1
//#define TCPWMx_GRPx_CNTx_V TCPWM0_GRP0_CNT2
//#define TCPWMx_GRPx_CNTx_W TCPWM0_GRP0_CNT3
//
//#define TCPWMx_LINEx_PORT_U GPIO_PRT6
//#define TCPWMx_LINEx_PORT_V GPIO_PRT6
//#define TCPWMx_LINEx_PORT_W GPIO_PRT6
//
//#define TCPWMx_LINEx_PIN_U 3u
//#define TCPWMx_LINEx_PIN_V 5u
//#define TCPWMx_LINEx_PIN_W 7u
//
//#elif (MMEK_USE_SIX_PWM == 1)
//
//#define PCLK_TCPWM_U PCLK_TCPWM0_CLOCKS256
//#define PCLK_TCPWM_V PCLK_TCPWM0_CLOCKS257
//#define PCLK_TCPWM_W PCLK_TCPWM0_CLOCKS258
//
//#define TCPWMx_GRPx_CNTx_U TCPWM0_GRP1_CNT0
//#define TCPWMx_GRPx_CNTx_V TCPWM0_GRP1_CNT1
//#define TCPWMx_GRPx_CNTx_W TCPWM0_GRP1_CNT2
//
//#define TCPWMx_LINEx_PORT_U             GPIO_PRT6
//#define TCPWMx_LINEx_PORT_V             GPIO_PRT6
//#define TCPWMx_LINEx_PORT_W             GPIO_PRT6
//#define TCPWMx_LINEx_PORT_U_COMPL       GPIO_PRT6
//#define TCPWMx_LINEx_PORT_V_COMPL       GPIO_PRT6
//#define TCPWMx_LINEx_PORT_W_COMPL       GPIO_PRT6
//
//#define TCPWMx_LINEx_PIN_U              0u
//#define TCPWMx_LINEx_PIN_V              2u
//#define TCPWMx_LINEx_PIN_W              4u
//#define TCPWMx_LINEx_PIN_U_COMPL        1u
//#define TCPWMx_LINEx_PIN_V_COMPL        3u
//#define TCPWMx_LINEx_PIN_W_COMPL        5u
//
//#endif

extern cy_stc_tcpwm_counter_config_t SpeedLoopTimerCfg;
extern cy_stc_tcpwm_pwm_config_t PwmUVWCfg;


#endif /* SDL_TCPWM_CFG_H */
