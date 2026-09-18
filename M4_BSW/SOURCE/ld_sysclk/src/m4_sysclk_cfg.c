/*
 * m4_sysclk.c
 *
 *  Created on: 2026年1月27日
 *      Author: hzldy
 */


#include "m4_sysclk_cfg.h"
#include "m4_task.h"
void system_time_1ms(void);

//_st_sysclk_cfg sysclk_cfg[CLK_CFG_NUM] = {
//  
//  {
//    .ipBlock = PCLK_TCPWM0_CLOCKS0,
//    .dividerType = CY_SYSCLK_DIV_16_BIT,
//    .dividerNum = CLK_TIMER_DIV
//  },
//
//};
//
//
//
//
//_St_sysclk_div_cfg sysclk_div_cfg[CLK_DIV_NUM] =  //主频80m
//{
//  {
//    .dividerValue = 8-1, //20M
//  
//  },
//  
//};
void Sysclk_Init(void)
{
//        Cy_SysTick_Init(CY_SYSTICK_CLOCK_SOURCE_CLK_CPU, 160000ul); //160,000,000 / 160,000 = 1000hz
//        Cy_SysTick_SetCallback(0ul, system_time_1ms);

}


void system_time_1ms(void)
{
    static uint16_t test = 0;
    test++;
    if(test > 999)
    {
//      Cy_GPIO_Inv(GPIO_PRT0, 0);   
      test = 0;
//      TERM_PRINT_H("11");

    }

  
}



