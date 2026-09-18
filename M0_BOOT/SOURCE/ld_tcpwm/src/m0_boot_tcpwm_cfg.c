/*
 * m0_pwm_cfg.c
 *
 *  Created on: 2026年1月22日
 *      Author: hzldy
 */
#include "m0_boot_tcpwm_cfg.h"


#define PWM_DEADTIME 120
#define SLEEP_LOOP_PERIOD 1000 // 20,000,000/1000 = 2000hz
#define M0_1MS_LOOP_PERIOD 2000 // 20,000,000/2000 = 1000hz
#define PWM_PERIOD   4000 //80,000,000/4000 = 20,000HZ   
cy_stc_tcpwm_counter_config_t const M4_Sleep_Loop_S0_config =
{
    .period             = SLEEP_LOOP_PERIOD - 1,                      		// 2,000,000 / 1000 = 2000hz
    .clockPrescaler     = CY_TCPWM_PRESCALER_DIVBY_1,  		// 2,000,000Hz / 1 = 2,000,000
    .runMode            = CY_TCPWM_COUNTER_CONTINUOUS,
    .countDirection     = CY_TCPWM_COUNTER_COUNT_UP,
    .debug_pause        = 0uL,
    .compareOrCapture   = CY_TCPWM_COUNTER_MODE_COMPARE,
    .compare0           = 0,
    .compare0_buff      = 0,
    .compare1           = 0,
    .compare1_buff      = 0,
    .enableCompare0Swap = false,
    .enableCompare1Swap = false,
    .interruptSources   = CY_TCPWM_INT_NONE,
    .capture0InputMode  = CY_TCPWM_INPUT_LEVEL,
    .capture0Input      = 0uL,
    .reloadInputMode    = CY_TCPWM_INPUT_LEVEL,
    .reloadInput        = 0uL,
    .startInputMode     = CY_TCPWM_INPUT_LEVEL,
    .startInput         = 0uL,
    .stopInputMode      = CY_TCPWM_INPUT_LEVEL,
    .stopInput          = 0uL,
    .capture1InputMode  = CY_TCPWM_INPUT_LEVEL,
    .capture1Input      = 0uL,
    .countInputMode     = CY_TCPWM_INPUT_LEVEL,
    .countInput         = 1uL,
    .trigger0EventCfg   = CY_TCPWM_COUNTER_OVERFLOW,
    .trigger1EventCfg   = CY_TCPWM_COUNTER_OVERFLOW,
};


cy_stc_tcpwm_counter_config_t const M0_1ms_Loop_S50_config =
{
    .period             = M0_1MS_LOOP_PERIOD - 1,                      		// 2,000,000 / 2000 = 1000hz
    .clockPrescaler     = CY_TCPWM_PRESCALER_DIVBY_1,  		// 2,000,000Hz / 1 = 2,000,000
    .runMode            = CY_TCPWM_COUNTER_CONTINUOUS,
    .countDirection     = CY_TCPWM_COUNTER_COUNT_UP,
    .debug_pause        = 0uL,
    .compareOrCapture   = CY_TCPWM_COUNTER_MODE_COMPARE,
    .compare0           = 0,
    .compare0_buff      = 0,
    .compare1           = 0,
    .compare1_buff      = 0,
    .enableCompare0Swap = false,
    .enableCompare1Swap = false,
    .interruptSources   = CY_TCPWM_INT_NONE,
    .capture0InputMode  = CY_TCPWM_INPUT_LEVEL,
    .capture0Input      = 0uL,
    .reloadInputMode    = CY_TCPWM_INPUT_LEVEL,
    .reloadInput        = 0uL,
    .startInputMode     = CY_TCPWM_INPUT_LEVEL,
    .startInput         = 0uL,
    .stopInputMode      = CY_TCPWM_INPUT_LEVEL,
    .stopInput          = 0uL,
    .capture1InputMode  = CY_TCPWM_INPUT_LEVEL,
    .capture1Input      = 0uL,
    .countInputMode     = CY_TCPWM_INPUT_LEVEL,
    .countInput         = 1uL,
    .trigger0EventCfg   = CY_TCPWM_COUNTER_OVERFLOW,
    .trigger1EventCfg   = CY_TCPWM_COUNTER_OVERFLOW,
};

/* Configuration using six PWM channels from group 1 (TCPWM0_GRP1_CNT0, TCPWM0_GRP1_CNT1, TCPWM0_GRP1_CNT2)*/
cy_stc_tcpwm_pwm_config_t PwmUVWCfg =
{
  .pwmMode = CY_TCPWM_PWM_MODE_DEADTIME,
  .clockPrescaler = CY_TCPWM_PRESCALER_DIVBY_1,
  .debug_pause = false,
  .countDirection = 0u, /* Set count direction Up */
  .cc0MatchMode = CY_TCPWM_PWM_TR_CTRL2_SET,
  .overflowMode = CY_TCPWM_PWM_TR_CTRL2_CLEAR,
  .underflowMode = CY_TCPWM_PWM_TR_CTRL2_CLEAR,
  .cc1MatchMode = CY_TCPWM_PWM_TR_CTRL2_CLEAR,
  .deadTime = PWM_DEADTIME,///待实现
  .deadTimeComp = 10,///待实现
  .runMode = CY_TCPWM_PWM_CONTINUOUS,
  .period = PWM_PERIOD - 1, 
  .period_buff = 0u,
  .compare0 = 0,
  .compare1 = 0u,
  .enablePeriodSwap = false, /* Auto Reload Period = OFF */
  .enableCompare0Swap = true, /* Auto Reload CC0 = ON */
  .enableCompare1Swap = true, /* Auto Reload CC1 = ON */
  .interruptSources = CY_TCPWM_INT_NONE, /* Interrupt Mask for TC, CC0/CC1_MATCH (0:OFF, 1:TC, 2:CC0 MATCH, 4:CC1 MATCH, 7:all) */
  .invertPWMOut = 0u,
  .invertPWMOutN = 0u,
  .killMode = CY_TCPWM_PWM_NOT_STOP_ON_KILL,
  .switchInputMode = CY_TCPWM_INPUT_LEVEL, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .switchInput = 0u, /* Select the constant 0 */
  .reloadInputMode = CY_TCPWM_INPUT_LEVEL, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .reloadInput = 7u, /* Select the TCPWM_ALL_CNT_TR_IN[2] */
  .startInputMode = CY_TCPWM_INPUT_LEVEL, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .startInput = 0u, /* Select the constant 0 */
  .kill0InputMode = CY_TCPWM_INPUT_LEVEL, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .kill0Input = 0u, /* Select the constant 0 */
  .kill1InputMode = CY_TCPWM_INPUT_LEVEL, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .kill1Input = 0u, /* Select the constant 0 */
  .countInputMode = CY_TCPWM_INPUT_LEVEL, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .countInput = 1u, /* Select the constant 1 */
};

cy_stc_tcpwm_pwm_config_t Stand_PWM =
{
  .pwmMode = CY_TCPWM_PWM_MODE_PWM,
  .clockPrescaler = CY_TCPWM_PRESCALER_DIVBY_1,
  .debug_pause = false,
  .countDirection = 0u, /* Set count direction Up */
  .cc0MatchMode = CY_TCPWM_PWM_TR_CTRL2_CLEAR,
  .overflowMode = CY_TCPWM_PWM_TR_CTRL2_SET,
  .underflowMode = CY_TCPWM_PWM_TR_CTRL2_NO_CHANGE,
  .cc1MatchMode = CY_TCPWM_PWM_TR_CTRL2_NO_CHANGE,
  .deadTime = 0,///待实现
  .deadTimeComp = 0,///待实现
  .runMode = CY_TCPWM_PWM_CONTINUOUS,
  .period = PWM_PERIOD - 1, 
  .period_buff = 0u,
  .compare0 = 0u,
  .compare1 = 0u,
  .enablePeriodSwap = false, /* Auto Reload Period = OFF */
  .enableCompare0Swap = false, /* Auto Reload CC0 = ON */
  .enableCompare1Swap = false, /* Auto Reload CC1 = ON */
  .interruptSources = CY_TCPWM_INT_NONE, /* Interrupt Mask for TC, CC0/CC1_MATCH (0:OFF, 1:TC, 2:CC0 MATCH, 4:CC1 MATCH, 7:all) */
  .invertPWMOut = 0u,
  .invertPWMOutN = 0u,
  .killMode = CY_TCPWM_PWM_STOP_ON_KILL,
  .switchInputMode = CY_TCPWM_INPUT_LEVEL, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .switchInput = 0u, /* Select the constant 0 */
  .reloadInputMode = CY_TCPWM_INPUT_LEVEL, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .reloadInput = 0u, /* Select the TCPWM_ALL_CNT_TR_IN[2] */
  .startInputMode = CY_TCPWM_INPUT_LEVEL, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .startInput = 0u, /* Select the constant 0 */
  .kill0InputMode = CY_TCPWM_INPUT_LEVEL, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .kill0Input = 0u, /* Select the constant 0 */
  .kill1InputMode = CY_TCPWM_INPUT_LEVEL, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .kill1Input = 0u, /* Select the constant 0 */
  .countInputMode = CY_TCPWM_INPUT_LEVEL, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .countInput = 1u, /* Select the constant 1 */
};

void Tcpwm_Init(void)
{

//    Cy_Tcpwm_Counter_Init(TCPWM0_GRP0_CNT0, &M4_Sleep_Loop_S0_config);
    Cy_Tcpwm_Counter_Init(TCPWM0_GRP0_CNT50, &M0_1ms_Loop_S50_config);

    Cy_Tcpwm_Counter_Enable(TCPWM0_GRP0_CNT50);
    Cy_Tcpwm_TriggerStart(TCPWM0_GRP0_CNT50);  
//    /* Enable Interrupt */
    Cy_Tcpwm_Counter_SetTC_IntrMask(TCPWM0_GRP0_CNT50);
//
//    Cy_Tcpwm_Pwm_Init(TCPWMx_GRPx_CNTx_U, &PwmUVWCfg);
//    Cy_Tcpwm_Pwm_Init(TCPWMx_GRPx_CNTx_V, &PwmUVWCfg);
//    Cy_Tcpwm_Pwm_Init(TCPWMx_GRPx_CNTx_W, &PwmUVWCfg);
//    Cy_Tcpwm_Pwm_Init(TCPWMx_GRPx_CNTx_CC0, &PwmUVWCfg);
//    Cy_Tcpwm_Pwm_Init(TCPWMx_GRPx_CNTx_CC1, &PwmUVWCfg);
//    Cy_Tcpwm_Pwm_Init(TCPWMx_GRPx_CNTx_THICK, &Stand_PWM);
//    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_THICK);
    
//    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_V);
//    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_W);
//    Cy_Tcpwm_TriggerStart(TCPWMx_GRPx_CNTx_THICK);
//    Cy_Tcpwm_TriggerStart(TCPWMx_GRPx_CNTx_V);
//    Cy_Tcpwm_TriggerStart(TCPWMx_GRPx_CNTx_W);
//    Cy_Tcpwm_Counter_SetTC_IntrMask(TCPWMx_GRPx_CNTx_U);

}



//void Tcpwm_Thick_Control_Set(uint8_t per)
//{
//  Cy_Tcpwm_Counter_SetCompare0(TCPWMx_GRPx_CNTx_THICK, (uint32_t)per*PWM_PERIOD/100);
//
//}