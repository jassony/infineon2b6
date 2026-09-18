/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#include "cy_project.h"
#include "cy_device_headers.h"
#include "main_cm4.h"
/* *INDENT-OFF* */
#ifndef MMEK_USE_SIX_PWM
#error "MMEK_USE_SIX_PWM not defined"
#endif

/* Speed loop interrupt configuration */
cy_stc_sysint_irq_t const SpeedLoopIrqCfg =
{
  .sysIntSrc = tcpwm_0_interrupts_0_IRQn,
  .intIdx = CPUIntIdx1_IRQn,
  .isEnabled = true,
};

/* Current loop interrupt configuration */
cy_stc_sysint_irq_t const CurrentLoopIrqCfg =
{
#if (MMEK_USE_SIX_PWM == 0)
  .sysIntSrc = tcpwm_0_interrupts_1_IRQn,
#elif (MMEK_USE_SIX_PWM == 1)
  .sysIntSrc = tcpwm_0_interrupts_256_IRQn,
#endif
  .intIdx = CPUIntIdx0_IRQn,
  .isEnabled = true,
};

/* *INDENT-ON* */

/* [] END OF FILE */
