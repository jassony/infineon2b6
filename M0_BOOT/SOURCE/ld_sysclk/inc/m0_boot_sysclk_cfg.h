/*
 * m0_sysclk_cfg.h
 *
 *  Created on: 2026Äê1ÔÂ23ÈÕ
 *      Author: hzldy
 */

#ifndef M0_BSW_SOURCE_LD_SYSCLK_INC_M0_SYSCLK_CFG_H_
#define M0_BSW_SOURCE_LD_SYSCLK_INC_M0_SYSCLK_CFG_H_


#include "cy_project.h"
#include "cy_device_headers.h"





typedef enum{
//  PASS0_SAR0_CLK,
//  PASS0_SAR1_CLK,
//  PASS0_SAR2_CLK,
//  ADC_CLK_CFG,
//  TIMER_CLKS0 = ADC_CLK_CFG,
  TIMER_CLKS50,
  TIMER_CLK_CFG,
//  PWM_U_CLK = TIMER_CLK_CFG,
//  PWM_V_CLK,
//  PWM_W_CLK,
//  PWM_CC0_CLK,
//  PWM_CC1_CLK,
//  PWM_THICK_CLK,
//  PWM_CLK_CFG,
  CANFD0_CLK = TIMER_CLK_CFG,
  CAN_CLK_CFG,
//  timer_CLK_CFG,
  CLK_CFG_NUM = CAN_CLK_CFG
}_em_clk_cfg;

typedef enum{
//  CLK_ADC_DIV,
  CLK_TIMER_DIV,
//  CLK_PWM_DIV,
  DIV_16BIT ,
  CLK_CAN_DIV = DIV_16BIT,
  DIV_8BIT,
  CLK_DIV_NUM = DIV_8BIT,
  
}_em_clk_div_type_cfg;

#define DIV_16BIT_NUM  DIV_16BIT
#define DIV_8BIT_NUM  (DIV_8BIT - DIV_16BIT)

typedef struct
{
  en_clk_dst_t ipBlock;
  cy_en_divider_types_t dividerType; 
  uint32_t dividerNum;
}_st_sysclk_cfg;

typedef struct
{
  uint32_t dividerValue;


}_St_sysclk_div_cfg;

void Sysclk_Init(void);

#endif /* M0_BSW_SOURCE_LD_SYSCLK_INC_M0_SYSCLK_CFG_H_ */
