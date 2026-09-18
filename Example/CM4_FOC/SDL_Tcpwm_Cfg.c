/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#include "cy_project.h"
#include "cy_device_headers.h"
#include "SDL_Tcpwm_Cfg.h"
#include "main_cm4.h"
#include "Ifx_MHA_PatternGen_Cfg.h"

/* *INDENT-OFF* */

cy_stc_tcpwm_counter_config_t SpeedLoopTimerCfg =
{
  .period = 0u, /* Period will be set by init function */
//  .clockPrescaler = CY_TCPWM_COUNTER_PRESCALER_DIVBY_1,
  .runMode = CY_TCPWM_PWM_CONTINUOUS,
  .countDirection = CY_TCPWM_COUNTER_COUNT_UP,
  .debug_pause = false,
//  .CompareOrCapture = CY_TCPWM_COUNTER_MODE_COMPARE,
  .compare0 = 0u,
  .compare0_buff = 0u,
  .compare1 = 0u,
  .compare1_buff = 0u,
  .enableCompare0Swap = false,
  .enableCompare1Swap = false,
  .interruptSources = 0u,
  .capture0InputMode = 3u,
  .capture0Input = 0u,
  .reloadInputMode = 3u,
  .reloadInput = 7u,
  .startInputMode = 3u,
  .startInput = 0u,
  .stopInputMode = 3u,
  .stopInput = 0u,
  .capture1InputMode = 3u,
  .capture1Input = 0u,
  .countInputMode = 3u,
  .countInput = 1u,
//  .trigger1 = CY_TCPWM_COUNTER_OVERFLOW,
};

/* Configuration for U/V/W-phase Timer */
#if (MMEK_USE_SIX_PWM == 0)
cy_stc_tcpwm_pwm_config_t PwmUVWCfg =
{
  .pwmMode = CY_TCPWM_PWM_MODE_PWM,
  .clockPrescaler = CY_TCPWM_PWM_PRESCALER_DIVBY_1,
  .debug_pause = false,
  .countDirection = 0u, /* Set count direction Up */
  .Cc0MatchMode = CY_TCPWM_PWM_TR_CTRL2_SET,
  .OverflowMode = CY_TCPWM_PWM_TR_CTRL2_CLEAR,
  .UnderflowMode = CY_TCPWM_PWM_TR_CTRL2_CLEAR,
  .Cc1MatchMode = CY_TCPWM_PWM_TR_CTRL2_CLEAR,
  .deadTime = 0u, /* Right side dead time */
  .deadTimeComp = 0u, /* Left side dead time */
  .runMode = CY_TCPWM_PWM_CONTINUOUS,
  .period = 0u, /* Period will be set by init function */
  .period_buff = 0u,
  .compare0 = 0u,
  .compare1 = 0u,
  .enablePeriodSwap = false, /* Auto Reload Period = OFF */
  .enableCompare0Swap = true, /* Auto Reload CC0 = ON */
  .enableCompare1Swap = true, /* Auto Reload CC1 = ON */
  .interruptSources = 0u, /* Interrupt Mask for TC, CC0/CC1_MATCH (0:OFF, 1:TC, 2:CC0 MATCH, 4:CC1 MATCH, 7:all) */
  .invertPWMOut = 0u,
  .invertPWMOutN = 0u,
  .killMode = CY_TCPWM_PWM_NOT_STOP_ON_KILL,
  .switchInputMode = 3u, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .switchInput = 0u, /* Select the constant 0 */
  .reloadInputMode = 3u, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .reloadInput = 7u, /* Select the TCPWM_ALL_CNT_TR_IN[2] */
  .startInputMode = 3u, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .startInput = 0u, /* Select the constant 0 */
  .kill0InputMode = 3u, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .kill0Input = 0u, /* Select the constant 0 */
  .kill1InputMode = 3u, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .kill1Input = 0u, /* Select the constant 0 */
  .countInputMode = 3u, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .countInput = 1u, /* Select the constant 1 */
};

#elif (MMEK_USE_SIX_PWM == 1)

/* Configuration using six PWM channels from group 1 (TCPWM0_GRP1_CNT0, TCPWM0_GRP1_CNT1, TCPWM0_GRP1_CNT2)*/
cy_stc_tcpwm_pwm_config_t PwmUVWCfg =
{
  .pwmMode = CY_TCPWM_PWM_MODE_DEADTIME,
//  .clockPrescaler = CY_TCPWM_PWM_PRESCALER_DIVBY_1,
  .debug_pause = false,
  .countDirection = 0u, /* Set count direction Up */
//  .Cc0MatchMode = CY_TCPWM_PWM_TR_CTRL2_SET,
//  .OverflowMode = CY_TCPWM_PWM_TR_CTRL2_CLEAR,
//  .UnderflowMode = CY_TCPWM_PWM_TR_CTRL2_CLEAR,
//  .Cc1MatchMode = CY_TCPWM_PWM_TR_CTRL2_CLEAR,
  .deadTime = IFX_MHA_PATTERNGEN_CFG_DEADTIME_TICKS,
  .deadTimeComp = IFX_MHA_PATTERNGEN_CFG_DEADTIME_TICKS,
  .runMode = CY_TCPWM_PWM_CONTINUOUS,
  .period = 0u,
  .period_buff = 0u,
  .compare0 = 0u,
  .compare1 = 0u,
  .enablePeriodSwap = false, /* Auto Reload Period = OFF */
  .enableCompare0Swap = true, /* Auto Reload CC0 = ON */
  .enableCompare1Swap = true, /* Auto Reload CC1 = ON */
  .interruptSources = 0u, /* Interrupt Mask for TC, CC0/CC1_MATCH (0:OFF, 1:TC, 2:CC0 MATCH, 4:CC1 MATCH, 7:all) */
  .invertPWMOut = 0u,
  .invertPWMOutN = 0u,
  .killMode = CY_TCPWM_PWM_NOT_STOP_ON_KILL,
  .switchInputMode = 3u, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .switchInput = 0u, /* Select the constant 0 */
  .reloadInputMode = 3u, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .reloadInput = 7u, /* Select the TCPWM_ALL_CNT_TR_IN[2] */
  .startInputMode = 3u, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .startInput = 0u, /* Select the constant 0 */
  .kill0InputMode = 3u, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .kill0Input = 0u, /* Select the constant 0 */
  .kill1InputMode = 3u, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .kill1Input = 0u, /* Select the constant 0 */
  .countInputMode = 3u, /* NO_EDGE_DET: No edge detection, use trigger as is */
  .countInput = 1u, /* Select the constant 1 */
};

#endif
/* *INDENT-ON* */

/* [] END OF FILE */
