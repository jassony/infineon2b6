/*
 * m0_irq_cfg.c
 *
 *  Created on: 2026Äê1ÔÂ22ÈÕ
 *      Author: hzldy
 */


#include "m0_irq_cfg.h"


extern void CanfdInterruptHandler(void);
extern void task_isr(void);
extern void Flash_Isr(void);

//static void Timer_Handler(void);
//static void Ifx_FOC_periodMatchCallback(void);

stc_sysint_irq_t irq_cfg[IRQ_MAX_NUM] = {
  {
    .cfg = {
            .sysIntSrc  = CY_CANFD0_IRQN, /* Use interrupt LINE0 */
            .intIdx     = INTIDX_CANFD,
            .isEnabled  = true,
    },
    .handler = CanfdInterruptHandler,
    .priority = 1,
  },
  {
    .cfg = {
            .sysIntSrc  = tcpwm_0_interrupts_50_IRQn, /* Use interrupt LINE0 */
            .intIdx     = INTIDX_TCPWM_50,
            .isEnabled  = true,
    },
    .handler = task_isr,
    .priority = 1,
  },
//  {
//    .cfg = {
//            .sysIntSrc  = cpuss_interrupt_fm_IRQn, /* Use interrupt LINE0 */
//            .intIdx     = INTIDX_FLASH,
//            .isEnabled  = true,
//    },
//    .handler = Flash_Isr,
//    .priority = 0,
//  },
};


void Irq_Init(void)
{
    for (uint8_t i = 0; i < (sizeof(irq_cfg) / sizeof(irq_cfg[0])); i++)
    {
        Cy_SysInt_InitIRQ(&irq_cfg[i].cfg);
        Cy_SysInt_SetSystemIrqVector(irq_cfg[i].cfg.sysIntSrc, irq_cfg[i].handler);
        NVIC_SetPriority(irq_cfg[i].cfg.intIdx, irq_cfg[i].priority);
        NVIC_ClearPendingIRQ(irq_cfg[i].cfg.intIdx);
        NVIC_EnableIRQ(irq_cfg[i].cfg.intIdx);
    }
}





//static void Timer_Handler(void)
//{
////  static uint16_t test = 0;
//    if(Cy_Tcpwm_Counter_GetTC_IntrMasked(TCPWM0_GRP0_CNT0))
//    {
////        test++;
////        if(test > 999)
////        {
////          Cy_GPIO_Inv(GPIO_PRT0, 0);   
////          test = 0;
////        }
//        Cy_Tcpwm_Counter_ClearTC_Intr(TCPWM0_GRP0_CNT0);
//    }
//}


//static void Ifx_FOC_periodMatchCallback(void)
//{
//    Cy_Tcpwm_Counter_ClearTC_Intr(TCPWM0_GRP1_CNT0);
//
//}