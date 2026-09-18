/*
 * m4_irq_cfg.c
 *
 *  Created on: 2026Äê1ÔÂ27ÈÕ
 *      Author: hzldy
 */

#include "m4_irq_cfg.h"
#include "SDL_init.h"
#include "mcu_load_profiler.h"
//static void CanfdInterruptHandler(void);
//static void Timer_Handler(void);
//static void Ifx_FOC_periodMatchCallback(void);
extern void Ifx_FOC_periodMatchCallback(void);
extern void Ifx_FOC_speedLoopCallback(void);
extern void ButtonIntHandler(void);


stc_sysint_irq_t irq_cfg[IRQ_MAX_NUM] = {
    {
      .cfg = {
            .sysIntSrc  = tcpwm_0_interrupts_0_IRQn, /* Use interrupt LINE0 */
            .intIdx     = INTIDX_SLEEPLOOP,
            .isEnabled  = true,
    },
    .handler = Ifx_FOC_speedLoopCallback,
    .priority = 3,
  },
  {
    .cfg = {
            .sysIntSrc  = tcpwm_0_interrupts_256_IRQn, /* Use interrupt LINE0 */
            .intIdx     = INTIDX_FOC_PERIOD,
            .isEnabled  = true,
    },
    .handler = Ifx_FOC_periodMatchCallback,
    .priority = 1,
  },
  {
    .cfg = {
          .sysIntSrc  = ioss_interrupts_gpio_14_IRQn,
          .intIdx     = INTIDX_INPUT_GPIO,
          .isEnabled  = true,
    },
    .handler = ButtonIntHandler,
    .priority = 1,
  },
};


void Irq_Init(void)
{
    for (uint8_t i = 0; i < (sizeof(irq_cfg) / sizeof(irq_cfg[0])); i++)
    {
        Cy_SysInt_InitIRQ(&irq_cfg[i].cfg);
        Cy_SysInt_SetSystemIrqVector(irq_cfg[i].cfg.sysIntSrc, irq_cfg[i].handler);
        NVIC_SetPriority(irq_cfg[i].cfg.intIdx, irq_cfg[i].priority);
        NVIC_ClearPendingIRQ(irq_cfg[i].cfg.intIdx);
//        NVIC_EnableIRQ(irq_cfg[i].cfg.intIdx);
    }
}



//static void Ifx_FOC_periodMatchCallback(void)
//{
//    Cy_Tcpwm_Counter_ClearTC_Intr(TCPWM0_GRP1_CNT0);
//
//}
uint8_t IPMFAULT_STATE = 0;
void ButtonIntHandler(void)
{
#if (MCU_LOAD_PROFILER_ENABLE != 0u)
    McuLoadProfilerToken profilerToken;
#endif
    uint32_t intStatus;

#if (MCU_LOAD_PROFILER_ENABLE != 0u)
    profilerToken = McuLoadProfiler_begin();
#endif
    PortInit_GPIO_0();
    /* If falling edge detected */
    intStatus = Cy_GPIO_GetInterruptStatusMasked(INPUT_ISR_PORT, INPUT_ISR_PIN);
    if (intStatus != 0ul)
    {
        Cy_GPIO_ClearInterrupt(INPUT_ISR_PORT, INPUT_ISR_PIN);
        IPMFAULT_STATE = 1;
        

//        TERM_PRINT_H("111\r\n");
        /* Toggle LED */
    }
#if (MCU_LOAD_PROFILER_ENABLE != 0u)
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_GPIO_IRQ, profilerToken);
#endif
}
