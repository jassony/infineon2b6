/*
 * m0_irq_cfg.h
 *
 *  Created on: 2026Äê1ÔÂ22ÈÕ
 *      Author: hzldy
 */

#ifndef M0_BSW_SOURCE_LD_IRQ_INC_M0_IRQ_CFG_H_
#define M0_BSW_SOURCE_LD_IRQ_INC_M0_IRQ_CFG_H_
#include "cy_project.h"



typedef enum
{
  IRQ_CANFD_USE,
  IRQ_TIMER_USE,
//  IRQ_FLASH_USE,
  IRQ_MAX_NUM
}_em_irq_cfg;

#define INTIDX_CANFD    CPUIntIdx3_IRQn
#define INTIDX_TCPWM_50 CPUIntIdx4_IRQn
#define INTIDX_FLASH    CPUIntIdx5_IRQn

#define INTIDX_IPC      CPUIntIdx7_IRQn

typedef struct
{
    cy_stc_sysint_irq_t cfg;
    cy_systemIntr_Handler handler;
    uint32_t priority;
}stc_sysint_irq_t;

void Irq_Init(void);

#endif /* M0_BSW_SOURCE_LD_IRQ_INC_M0_IRQ_CFG_H_ */
