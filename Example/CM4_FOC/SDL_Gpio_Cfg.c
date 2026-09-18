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
#include "SDL_Gpio_Cfg.h"

/* *INDENT-OFF* */
#ifndef MMEK_USE_SIX_PWM
#error "MMEK_USE_SIX_PWM not defined"
#endif

/* Analog input configuration for VDC */
cy_stc_gpio_pin_config_t const GpioAdcVdcCfg =
{
  .outVal = 0u,
  .driveMode = CY_GPIO_DM_ANALOG,
//  .hsiom = P10_5_GPIO,
  .intEdge = 0u,
  .intMask = 0u,
  .vtrip = 0u,
  .slewRate = 0u,
  .driveSel = 0u,
};

/* Analog input configuration for phase currents */
cy_stc_gpio_pin_config_t const GpioAdcPhaseCurrCfg =
{
  .outVal = 0u,
  .driveMode = CY_GPIO_DM_ANALOG,
//  .hsiom = P10_4_GPIO,
  .intEdge = 0u,
  .intMask = 0u,
  .vtrip = 0u,
  .slewRate = 0u,
  .driveSel = 0u,
};

/* Digital output pin configuration for ADC Trigger
 * (only used to verify that the trigger time of the first current measurement within a PWM cycle is correct) */
cy_stc_gpio_pin_config_t const GpioPwmAdcTrig0Cfg =
{
  .outVal = 0u,
  .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
//  .hsiom = P19_0_TCPWM0_LINE259,
  .intEdge = 0u,
  .intMask = 0u,
  .vtrip = 0u,
  .slewRate = 0u,
  .driveSel = 0u,
};

/* Digital output pin configuration for ADC Trigger
 * (only used to verify that the trigger time of the second current measurement within a PWM cycle is correct) */
cy_stc_gpio_pin_config_t const GpioPwmAdcTrig1Cfg =
{
  .outVal = 0u,
  .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
//  .hsiom = P13_4_TCPWM0_LINE266,
  .intEdge = 0u,
  .intMask = 0u,
  .vtrip = 0u,
  .slewRate = 0u,
  .driveSel = 0u,
};

/* Digital output configuration for a simple toggle pin to measure task runtimes */
cy_stc_gpio_pin_config_t GpioTogglePinCfg =
{
  .outVal = 0x00u,
  .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
//  .hsiom = P20_0_GPIO,
  .intEdge = 0u,
  .intMask = 0u,
  .vtrip = 0u,
  .slewRate = 0u,
  .driveSel = 0u,
};

#if (MMEK_USE_SIX_PWM == 0)
/* Output pin configuration for Phase U */
cy_stc_gpio_pin_config_t const GpioPwmUCfg =
{
  .outVal    = 0u,
  .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
  .hsiom     = P6_3_TCPWM0_LINE1,
  .intEdge   = 0u,
  .intMask   = 0u,
  .vtrip     = 0u,
  .slewRate  = 0u,
  .driveSel  = 0u,
};

/* Output pin configuration for Phase V */
cy_stc_gpio_pin_config_t const GpioPwmVCfg =
{
  .outVal    = 0u,
  .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
  .hsiom     = P6_5_TCPWM0_LINE2,
  .intEdge   = 0u,
  .intMask   = 0u,
  .vtrip     = 0u,
  .slewRate  = 0u,
  .driveSel  = 0u,
};

/* Output pin configuration  for Phase W */
cy_stc_gpio_pin_config_t const GpioPwmWCfg =
{
  .outVal    = 0u,
  .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
  .hsiom     = P6_7_TCPWM0_LINE3,
  .intEdge   = 0u,
  .intMask   = 0u,
  .vtrip     = 0u,
  .slewRate  = 0u,
  .driveSel  = 0u,
};

#elif (MMEK_USE_SIX_PWM == 1)
/* Output pin configuration for Phase U (LS) */
cy_stc_gpio_pin_config_t const GpioPwmUCfg =
{
  .outVal = 0u,
  .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
  .hsiom = P6_0_TCPWM0_LINE256,
  .intEdge = 0u,
  .intMask = 0u,
  .vtrip = 0u,
  .slewRate = 0u,
  .driveSel = 0u,
};

/* Output pin configuration for Phase U (HS) */
cy_stc_gpio_pin_config_t const GpioPwmUComplCfg =
{
  .outVal = 0u,
  .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
  .hsiom = P6_1_TCPWM0_LINE_COMPL256,
  .intEdge = 0u,
  .intMask = 0u,
  .vtrip = 0u,
  .slewRate = 0u,
  .driveSel = 0u,
};

/* Output pin configuration for Phase V (LS) */
cy_stc_gpio_pin_config_t const GpioPwmVCfg =
{
  .outVal = 0u,
  .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
  .hsiom = P6_2_TCPWM0_LINE257,
  .intEdge = 0u,
  .intMask = 0u,
  .vtrip = 0u,
  .slewRate = 0u,
  .driveSel = 0u,
};

/* Output pin configuration for Phase V (HS) */
cy_stc_gpio_pin_config_t const GpioPwmVComplCfg =
{
  .outVal = 0u,
  .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
  .hsiom = P6_3_TCPWM0_LINE_COMPL257,
  .intEdge = 0u,
  .intMask = 0u,
  .vtrip = 0u,
  .slewRate = 0u,
  .driveSel = 0u,
};

/* Output pin configuration for Phase W (LS) */
cy_stc_gpio_pin_config_t const GpioPwmWCfg =
{
  .outVal = 0u,
  .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
  .hsiom = P6_4_TCPWM0_LINE258,
  .intEdge = 0u,
  .intMask = 0u,
  .vtrip = 0u,
  .slewRate = 0u,
  .driveSel = 0u,
};

/* Output pin configuration for Phase W (HS) */
cy_stc_gpio_pin_config_t const GpioPwmWComplCfg =
{
  .outVal = 0u,
  .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
  .hsiom = P6_5_TCPWM0_LINE_COMPL258,
  .intEdge = 0u,
  .intMask = 0u,
  .vtrip = 0u,
  .slewRate = 0u,
  .driveSel = 0u,
};
#endif
/* *INDENT-ON* */

/* [] END OF FILE */
