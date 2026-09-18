/*
 * m0_sysclk_cfg.c
 *
 *  Created on: 2026年1月23日
 *      Author: hzldy
 */


#include "m0_boot_sysclk_cfg.h"
//#include "m0_tcpwm_cfg.h"

_st_sysclk_cfg sysclk_cfg[CLK_CFG_NUM] = {
  
//  {
//    .ipBlock = PCLK_PASS0_CLOCK_SAR0,
//    .dividerType = CY_SYSCLK_DIV_16_BIT,
//    .dividerNum = CLK_ADC_DIV
//  },
//
//  {
//    .ipBlock = PCLK_PASS0_CLOCK_SAR1,
//    .dividerType = CY_SYSCLK_DIV_16_BIT,
//    .dividerNum = CLK_ADC_DIV
//  },
//  
//  {
//    .ipBlock = PCLK_PASS0_CLOCK_SAR2,
//    .dividerType = CY_SYSCLK_DIV_16_BIT,
//    .dividerNum = CLK_ADC_DIV
//  },
  
//  {
//    .ipBlock = PCLK_TCPWM0_CLOCKS0,
//    .dividerType = CY_SYSCLK_DIV_16_BIT,
//    .dividerNum = CLK_TIMER_DIV
//  },

  {
    .ipBlock = PCLK_TCPWM0_CLOCKS50,
    .dividerType = CY_SYSCLK_DIV_16_BIT,
    .dividerNum = CLK_TIMER_DIV
  },
  
//  {
//    .ipBlock = PCLK_TCPWM_U,
//    .dividerType = CY_SYSCLK_DIV_16_BIT,
//    .dividerNum = CLK_PWM_DIV
//  },
//    
//  {
//    .ipBlock = PCLK_TCPWM_V,
//    .dividerType = CY_SYSCLK_DIV_16_BIT,
//    .dividerNum = CLK_PWM_DIV
//  },
//    
//  {
//    .ipBlock = PCLK_TCPWM_W,
//    .dividerType = CY_SYSCLK_DIV_16_BIT,
//    .dividerNum = CLK_PWM_DIV
//  },
//  
//  {
//    .ipBlock = PCLK_TCPWM_CC0,
//    .dividerType = CY_SYSCLK_DIV_16_BIT,
//    .dividerNum = CLK_PWM_DIV
//  },
//  
//  {
//    .ipBlock = PCLK_TCPWM_CC1,
//    .dividerType = CY_SYSCLK_DIV_16_BIT,
//    .dividerNum = CLK_PWM_DIV
//  },
//
//  {
//    .ipBlock = PCLK_TCPWM_THICK,
//    .dividerType = CY_SYSCLK_DIV_16_BIT,
//    .dividerNum = CLK_PWM_DIV
//  },
//  
  {
    .ipBlock = CY_CANFD0_PCLK,
    .dividerType = CY_SYSCLK_DIV_8_BIT,
    .dividerNum = CLK_CAN_DIV - DIV_16BIT_NUM
  },
};




_St_sysclk_div_cfg sysclk_div_cfg[CLK_DIV_NUM] =  //主频80m
{
//  {
//    .dividerValue = 4, //16M CLK_ADC_DIV
//  
//  },
  {
    .dividerValue = 39, //2M CLK_TIMER_DIV
  
  },
//  {
//    .dividerValue = 0, //80M CLK_PWM_DIV
//  
//  },
  
  {
  
    .dividerValue = 1, //40M CLK_CAN_DIV

  }


  
};//Cy_SysClk_PeriphAssignDivider最后一个参数与Cy_SysClk_PeriphSetDivider第二个参数保持一致
void Sysclk_Init(void)
{
  uint8_t i = 0;
  for(;i < CLK_CFG_NUM;i++)
  {
    Cy_SysClk_PeriphAssignDivider(sysclk_cfg[i].ipBlock, sysclk_cfg[i].dividerType, sysclk_cfg[i].dividerNum);
  }
  for(i = 0;i < DIV_16BIT_NUM;i++)
  {
  Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, i, sysclk_div_cfg[i].dividerValue); 
  Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, i);
  }
  for(i = 0;i < DIV_8BIT_NUM;i++)
  {
  Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_8_BIT, i, sysclk_div_cfg[i+DIV_16BIT_NUM].dividerValue); 
  Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_8_BIT, i);
  }
}

