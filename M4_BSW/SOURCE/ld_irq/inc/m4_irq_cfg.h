/*
 * m4_irq_cfg.h
 *
 *  Created on: 2026Äê1ÔÂ27ÈÕ
 *      Author: hzldy
 */

#ifndef M4_BSW_SOURCE_LD_IRQ_INC_M4_IRQ_CFG_H_
#define M4_BSW_SOURCE_LD_IRQ_INC_M4_IRQ_CFG_H_

#include "cy_project.h"

#define INTIDX_SLEEPLOOP        CPUIntIdx3_IRQn
#define INTIDX_FOC_PERIOD       CPUIntIdx4_IRQn
#define INTIDX_INPUT_GPIO       CPUIntIdx5_IRQn
#define INTIDX_IPC              CPUIntIdx7_IRQn


#define INPUT_ISR_PORT                  GPIO_PRT14
#define INPUT_ISR_PIN                   0u



typedef enum
{
  IRQ_SLEEP_LOOP_TIMER_USE,
  IRQ_TCPWM_USE,
  IRQ_GPIO_INPUT,
  IRQ_MAX_NUM
}_em_irq_cfg;   


typedef struct
{
    cy_stc_sysint_irq_t cfg;
    cy_systemIntr_Handler handler;
    uint32_t priority;
}stc_sysint_irq_t;

void Irq_Init(void);

#endif /* M4_BSW_SOURCE_LD_IRQ_INC_M4_IRQ_CFG_H_ */
