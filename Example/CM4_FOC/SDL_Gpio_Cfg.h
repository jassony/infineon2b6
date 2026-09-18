/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#ifndef SDL_GPIO_CFG_H
#define SDL_GPIO_CFG_H

/* GPIO port for ADC input VDC */
#define GPIO_ADC_PORT_VDC GPIO_PRT10

/* GPIO pin for ADC input VDC */
#define GPIO_ADC_PIN_VDC 5u

/* GPIO port for ADC input CC0 */
#define GPIO_ADC_PORT_CC0 GPIO_PRT10

/* GPIO pin for ADC input CC0 */
#define GPIO_ADC_PIN_CC0 4u

/* GPIO port for ADC input CC1 */
#define GPIO_ADC_PORT_CC1 GPIO_PRT10

/* GPIO pin for ADC input CC1 */
#define GPIO_ADC_PIN_CC1 4u

/* GPIO port for TCPWM output of ADC Trigger0
 * (only used to verify that the trigger time of the first current measurement within a PWM cycle is correct) */
#define GPIO_PWM_PORT_ADC_TRIG0 GPIO_PRT19

/* GPIO pin for TCPWM output of ADC Trigger0
 * (only used to verify that the trigger time of the first current measurement within a PWM cycle is correct) */
#define GPIO_PWM_PIN_ADC_TRIG0 0u

/* GPIO port for TCPWM output of ADC Trigger1 */
/* (only used to verify that the trigger time of the second current measurement within a PWM cycle is correct) */
#define GPIO_PWM_PORT_ADC_TRIG1 GPIO_PRT13

/* GPIO pin for TCPWM output of ADC Trigger1 */
/* (only used to verify that the trigger time of the second current measurement within a PWM cycle is correct) */
#define GPIO_PWM_PIN_ADC_TRIG1 4u

/* Analog input configuration for VDC */
extern cy_stc_gpio_pin_config_t const GpioAdcVdcCfg;

/* Analog input configuration for phase currents */
extern cy_stc_gpio_pin_config_t const GpioAdcPhaseCurrCfg;

/* Digital output pin configuration for ADC Trigger
 * (only used to verify that the trigger time of the first current measurement within a PWM cycle is correct) */
extern cy_stc_gpio_pin_config_t const GpioPwmAdcTrig0Cfg;

/* Digital output pin configuration for ADC Trigger
 * (only used to verify that the trigger time of the second current measurement within a PWM cycle is correct) */
extern cy_stc_gpio_pin_config_t const GpioPwmAdcTrig1Cfg;

/* Output pin configuration for Phase U (LS) */
extern cy_stc_gpio_pin_config_t const GpioPwmUCfg;

/* Output pin configuration for Phase U (HS) */
extern cy_stc_gpio_pin_config_t const GpioPwmUComplCfg;

/* Output pin configuration for Phase V (LS) */
extern cy_stc_gpio_pin_config_t const GpioPwmVCfg;

/* Output pin configuration for Phase V (HS) */
extern cy_stc_gpio_pin_config_t const GpioPwmVComplCfg;

/* Output pin configuration for Phase W (LS) */
extern cy_stc_gpio_pin_config_t const GpioPwmWCfg;

/* Output pin configuration for Phase W (HS) */
extern cy_stc_gpio_pin_config_t const GpioPwmWComplCfg;

/* Digital output configuration for a simple toggle pin to measure task runtimes */
extern cy_stc_gpio_pin_config_t GpioTogglePinCfg;

#endif /* SDL_GPIO_CFG_H */
