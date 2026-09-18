/*
 * m4_sysclk_cfg.h
 *
 *  Created on: 2026Äê1ÔÂ27ÈÕ
 *      Author: hzldy
 */

#ifndef M4_BSW_SOURCE_LD_SYSCLK_INC_M4_SYSCLK_CFG_H_
#define M4_BSW_SOURCE_LD_SYSCLK_INC_M4_SYSCLK_CFG_H_

#include "cy_project.h"
#include "cy_device_headers.h"

//typedef enum{
//  TIMER_CLK ,
//  TIMER_CLK_CFG,
//  CLK_CFG_NUM = TIMER_CLK_CFG
//}_em_clk_cfg;
//
//typedef enum{
//  CLK_TIMER_DIV,
//  DIV_16BIT ,
//  DIV_8BIT = DIV_16BIT,
//  CLK_DIV_NUM = DIV_8BIT,
//  
//}_em_clk_div_type_cfg;
//
//#define DIV_16BIT_NUM  DIV_16BIT
//#define DIV_8BIT_NUM  (DIV_8BIT - DIV_16BIT)

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



#endif /* M4_BSW_SOURCE_LD_SYSCLK_INC_M4_SYSCLK_CFG_H_ */
