/*
 * m0_gpio_cfg.c
 *
 *  Created on: 2026年1月22日
 *      Author: hzldy
 */
#include "m0_boot_gpio_cfg.h"


static const stc_pin_config pin_cfg[] =
{
      /* LED */
    {
        .portReg = LED_PORT, 
        .pinNum  = LED_PIN,
        {
            .outVal = 1,
            .driveMode = CY_GPIO_DM_STRONG,
            .hsiom = P0_0_GPIO,
            .intEdge = 0,
            .intMask = 0,
            .vtrip = 0,
            .slewRate = 0,
            .driveSel = 0,
            .vregEn = 0,
            .ibufMode = 0,
            .vtripSel = 0,
            .vrefSel = 0,
            .vohSel = 0,
        }
    },
    /* CAN0 RX */
    {
        .portReg = CY_CANFD_RX_PORT, 
        .pinNum  = CY_CANFD_RX_PIN,
        {
            .outVal = 0,
            .driveMode = CY_GPIO_DM_HIGHZ,
            .hsiom = CY_CANFD0_RX_MUX,
            .intEdge = 0,
            .intMask = 0,
            .vtrip = 0,
            .slewRate = 0,
            .driveSel = 0,
            .vregEn = 0,
            .ibufMode = 0,
            .vtripSel = 0,
            .vrefSel = 0,
            .vohSel = 0,
        }
    },
    /* CAN0 TX */
    {
        .portReg = CY_CANFD_TX_PORT,
        .pinNum  = CY_CANFD_TX_PIN,
        {
            .outVal = 1,
            .driveMode = CY_GPIO_DM_STRONG,
            .hsiom = CY_CANFD0_TX_MUX,
            .intEdge = 0,
            .intMask = 0,
            .vtrip = 0,
            .slewRate = 0,
            .driveSel = 0,
            .vregEn = 0,
            .ibufMode = 0,
            .vtripSel = 0,
            .vrefSel = 0,
            .vohSel = 0,
        }
    },
    
//    {
//        .portReg = TCPWMx_LINEx_PORT_U,
//        .pinNum  = TCPWMx_LINEx_PIN_U,
//        {
//        .outVal = 0u,
//        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
//        .hsiom = P6_0_TCPWM0_LINE256,
//        .intEdge = 0u,
//        .intMask = 0u,
//        .vtrip = 0u,
//        .slewRate = 0u,
//        .driveSel = 0u,
//        }
//    },
//    {
//        .portReg = TCPWMx_LINEx_PORT_U_COMPL,
//        .pinNum  = TCPWMx_LINEx_PIN_U_COMPL,
//        {
//        .outVal = 0u,
//        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
//        .hsiom = P6_1_TCPWM0_LINE_COMPL256,
//        .intEdge = 0u,
//        .intMask = 0u,
//        .vtrip = 0u,
//        .slewRate = 0u,
//        .driveSel = 0u,
//        }
//    },
//    {
//        .portReg = TCPWMx_LINEx_PORT_V,
//        .pinNum  = TCPWMx_LINEx_PIN_V,
//        {
//        .outVal = 0u,
//        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
//        .hsiom = P6_2_TCPWM0_LINE257,
//        .intEdge = 0u,
//        .intMask = 0u,
//        .vtrip = 0u,
//        .slewRate = 0u,
//        .driveSel = 0u,
//        }
//    },
//    {
//        .portReg = TCPWMx_LINEx_PORT_V_COMPL,
//        .pinNum  = TCPWMx_LINEx_PIN_V_COMPL,
//        {
//        .outVal = 0u,
//        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
//        .hsiom = P6_3_TCPWM0_LINE_COMPL257,
//        .intEdge = 0u,
//        .intMask = 0u,
//        .vtrip = 0u,
//        .slewRate = 0u,
//        .driveSel = 0u,
//        }
//    },
//    {
//        .portReg = TCPWMx_LINEx_PORT_W,
//        .pinNum  = TCPWMx_LINEx_PIN_W,
//        {
//        .outVal = 0u,
//        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
//        .hsiom = P6_4_TCPWM0_LINE258,
//        .intEdge = 0u,
//        .intMask = 0u,
//        .vtrip = 0u,
//        .slewRate = 0u,
//        .driveSel = 0u,
//        }
//    },
//    {
//        .portReg = TCPWMx_LINEx_PORT_W_COMPL,
//        .pinNum  = TCPWMx_LINEx_PIN_W_COMPL,
//        {
//        .outVal = 0u,
//        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
//        .hsiom = P6_5_TCPWM0_LINE_COMPL258,
//        .intEdge = 0u,
//        .intMask = 0u,
//        .vtrip = 0u,
//        .slewRate = 0u,
//        .driveSel = 0u,
//        }
//    },
//    {
//        .portReg = TCPWM_THICK_FILM_OD_PROT,
//        .pinNum  = TCPWM_THICK_FILM_OD_PIN,
//        {
//        .outVal = 0u,
//        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
//        .hsiom = P18_5_TCPWM0_LINE52,
//        .intEdge = 0u,
//        .intMask = 0u,
//        .vtrip = 0u,
//        .slewRate = 0u,
//        .driveSel = 0u,
//        }
//    },////pwm
//    {
//        .portReg = INPUT_ISR_PORT,
//        .pinNum  = INPUT_ISR_PIN,
//        {
//        .outVal    = 0ul,
//        .driveMode = CY_GPIO_DM_HIGHZ,
//        .hsiom     = P14_0_GPIO,
//        .intEdge   = CY_GPIO_INTR_FALLING,
//        .intMask   = 1ul,
//        .vtrip     = 0ul,
//        .slewRate  = 0ul,
//        .driveSel  = 0ul,
//        }
//    }, //外部中断
//    
//    {
//      .portReg = GPIO_SLEEP_OD_OUT_PROT,
//      .pinNum  = GPIO_SLEEP_OD_OUT_PIN,
//      {
//      .outVal    = 1ul,
//      .driveMode = CY_GPIO_DM_OD_DRIVESLOW,
//      .hsiom     = P0_3_GPIO,
//      .intEdge   = 0UL,
//      .intMask   = 0ul,
//      .vtrip     = 0ul,
//      .slewRate  = 0ul,
//      .driveSel  = 0ul,
//      }
//    }, 
//    {
//      .portReg = GPIO_OVERVOLT_IN_PORT,
//      .pinNum  = GPIO_OVERVOLT_IN_PIN,
//      {
//      .outVal    = 0ul,
//      .driveMode = CY_GPIO_DM_HIGHZ,
//      .hsiom     = P5_0_GPIO,
//      .intEdge   = 0UL,
//      .intMask   = 0ul,
//      .vtrip     = 0ul,
//      .slewRate  = 0ul,
//      .driveSel  = 0ul,
//      }
//    },  
//    {
//      .portReg = GPIO_UNDERVOLT_IN_PORT,
//      .pinNum  = GPIO_UNDERVOLT_IN_PIN,
//      {
//      .outVal    = 0ul,
//      .driveMode = CY_GPIO_DM_HIGHZ,
//      .hsiom     = P5_1_GPIO,
//      .intEdge   = 0UL,
//      .intMask   = 0ul,
//      .vtrip     = 0ul,
//      .slewRate  = 0ul,
//      .driveSel  = 0ul,
//      }
//    }, 
//    {
//      .portReg = GPIO_HVLOCKIN_IN_PROT,
//      .pinNum  = GPIO_HVLOCKIN_IN_PIN,
//      {
//      .outVal    = 0ul,
//      .driveMode = CY_GPIO_DM_HIGHZ,
//      .hsiom     = P7_0_GPIO,
//      .intEdge   = 0UL,
//      .intMask   = 0ul,
//      .vtrip     = 0ul,
//      .slewRate  = 0ul,
//      .driveSel  = 0ul,
//      }
//    },  
//    {
//      .portReg = GPIO_MAIN_SWITCH_OD_OUT_PORT,
//      .pinNum  = GPIO_MAIN_SWITCH_OD_OUT_PIN,
//      {
//      .outVal    = 1ul,
//      .driveMode = CY_GPIO_DM_OD_DRIVESHIGH,
//      .hsiom     = P18_7_GPIO,
//      .intEdge   = 0UL,
//      .intMask   = 0ul,
//      .vtrip     = 0ul,
//      .slewRate  = 0ul,
//      .driveSel  = 0ul,
//      }
//    },  //GPIO
    

};

void PortInit(void)
{
    for (uint8_t i = 0; i < (sizeof(pin_cfg) / sizeof(pin_cfg[0])); i++)
    {
        Cy_GPIO_Pin_Init(pin_cfg[i].portReg, pin_cfg[i].pinNum, &pin_cfg[i].cfg);
    }
}


void Port_Write( void* base, uint32_t pinNum, uint32_t value)
{

  Cy_GPIO_Write((stc_GPIO_PRT_t*) base,pinNum,value);


}

