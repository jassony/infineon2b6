/*
 * m0_pwm_cfg.h
 *
 *  Created on: 2026Äê1ÔÂ22ÈÕ
 *      Author: hzldy
 */

#ifndef M0_BSW_SOURCE_LD_TCPWM_INC_M0_TCPWM_CFG_H_
#define M0_BSW_SOURCE_LD_TCPWM_INC_M0_TCPWM_CFG_H_
#include "cy_device_headers.h"
#include "cy_project.h"

#define PCLK_TCPWM_U            PCLK_TCPWM0_CLOCKS256
#define PCLK_TCPWM_V            PCLK_TCPWM0_CLOCKS257
#define PCLK_TCPWM_W            PCLK_TCPWM0_CLOCKS258
#define PCLK_TCPWM_CC0          PCLK_TCPWM0_CLOCKS1
#define PCLK_TCPWM_CC1          PCLK_TCPWM0_CLOCKS260
#define PCLK_TCPWM_THICK        PCLK_TCPWM0_CLOCKS20

#define TCPWMx_GRPx_CNTx_U      TCPWM0_GRP1_CNT0
#define TCPWMx_GRPx_CNTx_V      TCPWM0_GRP1_CNT1
#define TCPWMx_GRPx_CNTx_W      TCPWM0_GRP1_CNT2
#define TCPWMx_GRPx_CNTx_CC0    TCPWM0_GRP0_CNT1
#define TCPWMx_GRPx_CNTx_CC1    TCPWM0_GRP1_CNT4
#define TCPWMx_GRPx_CNTx_THICK  TCPWM0_GRP0_CNT20



void Tcpwm_Init(void);
void Tcpwm_Thick_Control_Set(uint8_t per);


#endif /* M0_BSW_SOURCE_LD_PWM_INC_M0_PWM_CFG_H_ */
