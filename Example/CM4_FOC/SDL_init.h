/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

/**
 * \file SDL_init.h
 * \brief This module initializes the peripherals of the CYT2B7 with the help of the Sample Driver Library.
 */

#ifndef SDL_INIT_H
#define SDL_INIT_H

#include "types.h"

/* Peripheral clock divider used for TCPWM for PWM phases U/V/W */
#define TCPWM_PERI_CLK_DIVIDER_NO_PWMUVW    0u

/* Peripheral clock divider used for TCPWM for trigger of SpeedLoop */
#define TCPWM_PERI_CLK_DIVIDER_NO_SPEEDLOOP 1u

/* Peripheral clock divider used for Adc */
#define TCPWM_PERI_CLK_DIVIDER_NO_ADC       2u

/* Peripheral clock frequency in Hz */
#define PCLK_FREQ_HZ                        80000000u

/* Structure which represents a combination of divider and period value to reach a target frequency */
typedef struct
{
    uint32 divider;
    uint32 period;
} SDL_SysClkDivAndPeriod_t;




typedef struct
{
    volatile stc_GPIO_PRT_t* portReg;
    uint8_t pinNum;
    cy_stc_gpio_pin_config_t cfg;
}stc_pin_config_m4;


#define TCPWMx_LINEx_PORT_U             GPIO_PRT6
#define TCPWMx_LINEx_PORT_V             GPIO_PRT6
#define TCPWMx_LINEx_PORT_W             GPIO_PRT6
#define TCPWMx_LINEx_PORT_U_COMPL       GPIO_PRT6
#define TCPWMx_LINEx_PORT_V_COMPL       GPIO_PRT6
#define TCPWMx_LINEx_PORT_W_COMPL       GPIO_PRT6


#define TCPWMx_LINEx_PIN_U              0u
#define TCPWMx_LINEx_PIN_V              2u
#define TCPWMx_LINEx_PIN_W              4u
#define TCPWMx_LINEx_PIN_U_COMPL        1u
#define TCPWMx_LINEx_PIN_V_COMPL        3u
#define TCPWMx_LINEx_PIN_W_COMPL        5u






/* SDL initialization */
extern uint32 SDL_Init(void);



void PortInit_UVW(void);
void PortInit_GPIO(void);//配合Set_UVW_GPIO()一起使用
void PortInit_GPIO_0(void);//配合Set_UVW_GPIO()一起使用

#endif /* SDL_INIT_H */
