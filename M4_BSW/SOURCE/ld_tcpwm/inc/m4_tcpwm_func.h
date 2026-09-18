/*
 * ld_tcpwm.h
 *
 *  Created on: 2026Äê1ÔÂ27ÈÕ
 *      Author: hzldy
 */

#ifndef M4_BSW_SOURCE_LD_TCPWM_INC_M4_TCPWM_FUNC_H_
#define M4_BSW_SOURCE_LD_TCPWM_INC_M4_TCPWM_FUNC_H_

#include "cy_project.h"



#define PCLK_TCPWM_U            PCLK_TCPWM0_CLOCKS256
#define PCLK_TCPWM_V            PCLK_TCPWM0_CLOCKS257
#define PCLK_TCPWM_W            PCLK_TCPWM0_CLOCKS258
#define PCLK_TCPWM_CC0          PCLK_TCPWM0_CLOCKS1
#define PCLK_TCPWM_CC1          PCLK_TCPWM0_CLOCKS260
//#define PCLK_TCPWM_THICK        PCLK_TCPWM0_CLOCKS52

#define TCPWMx_GRPx_CNTx_U      TCPWM0_GRP1_CNT0
#define TCPWMx_GRPx_CNTx_V      TCPWM0_GRP1_CNT1
#define TCPWMx_GRPx_CNTx_W      TCPWM0_GRP1_CNT2
#define TCPWMx_GRPx_CNTx_CC0    TCPWM0_GRP0_CNT1
#define TCPWMx_GRPx_CNTx_CC1    TCPWM0_GRP1_CNT4
//#define TCPWMx_GRPx_CNTx_THICK  TCPWM0_GRP0_CNT52

void Tcpwm_Enable_Func(void);


#define PWM_PERIOD   4000 //80,000,000/4000 = 20,000HZ   


#endif /* M4_BSW_SOURCE_LD_TCPWM_INC_M4_TCPWM_FUNC_H_ */
