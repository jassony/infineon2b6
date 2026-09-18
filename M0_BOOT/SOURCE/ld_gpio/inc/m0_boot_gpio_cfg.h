/*
 * m0_gpio_cfg.h
 *
 *  Created on: 2026Äê1ÔÂ22ÈÕ
 *      Author: hzldy
 */

#ifndef M0_BSW_SOURCE_LD_GPIO_INC_M0_GPIO_CFG_H_
#define M0_BSW_SOURCE_LD_GPIO_INC_M0_GPIO_CFG_H_
#include "cy_project.h"


typedef struct
{
    volatile stc_GPIO_PRT_t* portReg;
    uint8_t pinNum;
    cy_stc_gpio_pin_config_t cfg;
}stc_pin_config;

#define LED_PORT                        GPIO_PRT0
#define GPIO_SLEEP_OD_OUT_PROT          GPIO_PRT0
#define CY_CANFD_RX_PORT                GPIO_PRT2
#define CY_CANFD_TX_PORT                GPIO_PRT2
#define GPIO_OVERVOLT_IN_PORT           GPIO_PRT5
#define GPIO_UNDERVOLT_IN_PORT          GPIO_PRT5
#define TCPWMx_LINEx_PORT_U             GPIO_PRT6
#define TCPWMx_LINEx_PORT_V             GPIO_PRT6
#define TCPWMx_LINEx_PORT_W             GPIO_PRT6
#define TCPWMx_LINEx_PORT_U_COMPL       GPIO_PRT6
#define TCPWMx_LINEx_PORT_V_COMPL       GPIO_PRT6
#define TCPWMx_LINEx_PORT_W_COMPL       GPIO_PRT6
#define GPIO_HVLOCKIN_IN_PROT           GPIO_PRT7
#define INPUT_ISR_PORT                  GPIO_PRT14
#define TCPWM_THICK_FILM_OD_PROT        GPIO_PRT18
#define GPIO_MAIN_SWITCH_OD_OUT_PORT    GPIO_PRT18

#define LED_PIN                         0u
#define GPIO_SLEEP_OD_OUT_PIN           3u
#define CY_CANFD_RX_PIN                 1u
#define CY_CANFD_TX_PIN                 0u
#define GPIO_OVERVOLT_IN_PIN            0u
#define GPIO_UNDERVOLT_IN_PIN           1u
#define TCPWMx_LINEx_PIN_U              0u
#define TCPWMx_LINEx_PIN_V              2u
#define TCPWMx_LINEx_PIN_W              4u
#define TCPWMx_LINEx_PIN_U_COMPL        1u
#define TCPWMx_LINEx_PIN_V_COMPL        3u
#define TCPWMx_LINEx_PIN_W_COMPL        5u
#define GPIO_HVLOCKIN_IN_PIN            0u
#define INPUT_ISR_PIN                   0u
#define TCPWM_THICK_FILM_OD_PIN         5u
#define GPIO_MAIN_SWITCH_OD_OUT_PIN     7u




void PortInit(void);
void Port_Write( void* base, uint32_t pinNum, uint32_t value);

#endif /* M0_BSW_SOURCE_LD_GPIO_INC_M0_GPIO_CFG_H_ */
