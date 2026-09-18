/*
 * m0_boot_wdg.c
 *
 *  Created on: 2026Äê7ÔÂ1ÈÕ
 *      Author: hzldy
 */
#include "m0_boot_wdg.h"
#include "cy_sysreset.h"
#include "cy_device_headers.h"
#include "cy_project.h"

void Wdg_Init(void)
{
//  uint32_t resetReason;
//  uint8_t ddd[8];
//  resetReason = Cy_SysReset_GetResetReason();
//        if((resetReason & CY_SYSRESET_MCWDT0) != 0ul)
//        {
//            /* Clear  Reset reason */
//            Cy_SysReset_ClearAllResetReasons();
////            Cy_GPIO_Pin_Init(USER_LED_PORT, USER_LED_PIN, &user_led_port_pin_cfg);
//            while(1)
//            {
//                Cy_SysTick_DelayInUs(500000ul);
//                 Can_Transmit(CY_CANFD0_TYPE, 0x111, 8, ddd,false);
//
////                Cy_GPIO_Inv(USER_LED_PORT, USER_LED_PIN);
//            }
//        }
//    Cy_MCWDT_SetDebugRun(MCWDT0, CY_MCWDT_COUNTER0, 1ul); // enable
    Cy_MCWDT_CpuSelectForDpSlpPauseAction(MCWDT0, CY_MCWDT_PAUSED_BY_DPSLP_CM0);

    /*********************************************************************/
    /*****                        Set actions                        *****/
    /*********************************************************************/
    Cy_MCWDT_SetLowerAction(MCWDT0, CY_MCWDT_COUNTER0, CY_MCWDT_ACTION_NONE);
    Cy_MCWDT_SetUpperAction(MCWDT0, CY_MCWDT_COUNTER0, CY_MCWDT_ACTION_NONE);
    Cy_MCWDT_SetWarnAction(MCWDT0, CY_MCWDT_COUNTER0, CY_MCWDT_WARN_ACTION_NONE);

    Cy_MCWDT_SetLowerAction(MCWDT0, CY_MCWDT_COUNTER1, CY_MCWDT_ACTION_NONE);
    Cy_MCWDT_SetUpperAction(MCWDT0, CY_MCWDT_COUNTER1, CY_MCWDT_ACTION_FAULT_THEN_RESET); // sub counter 1 upper limit causes to reset
    Cy_MCWDT_SetWarnAction(MCWDT0, CY_MCWDT_COUNTER1, CY_MCWDT_WARN_ACTION_NONE);

    Cy_MCWDT_SetSubCounter2Action(MCWDT0, CY_MCWDT_CNT2_ACTION_NONE);

    /*********************************************************************/
    /*****                      Set limit values                     *****/
    /*********************************************************************/
    Cy_MCWDT_SetLowerLimit(MCWDT0, CY_MCWDT_COUNTER0, 0, 0);
    Cy_MCWDT_SetWarnLimit(MCWDT0, CY_MCWDT_COUNTER0, 0, 0);
    Cy_MCWDT_SetUpperLimit(MCWDT0, CY_MCWDT_COUNTER0, 100, 0);

    Cy_MCWDT_SetLowerLimit(MCWDT0, CY_MCWDT_COUNTER1, 0, 0);
    Cy_MCWDT_SetWarnLimit(MCWDT0, CY_MCWDT_COUNTER1, 0, 0);
    Cy_MCWDT_SetUpperLimit(MCWDT0, CY_MCWDT_COUNTER1, 32000, 0);  /* 2 sec when clk_lf = 32KHz */

    Cy_MCWDT_SetToggleBit(MCWDT0, CY_MCWDT_CNT2_MONITORED_BIT15); // means 32768 count period


    /*********************************************************************/
    /*****                        Set options                        *****/
    /*********************************************************************/
    Cy_MCWDT_SetAutoService(MCWDT0, CY_MCWDT_COUNTER0, 0ul); // disable
    Cy_MCWDT_SetAutoService(MCWDT0, CY_MCWDT_COUNTER1, 0ul); // disable

    Cy_MCWDT_SetSleepDeepPause(MCWDT0, CY_MCWDT_COUNTER0, 1ul); // enable
    Cy_MCWDT_SetSleepDeepPause(MCWDT0, CY_MCWDT_COUNTER1, 1ul); // enable
    Cy_MCWDT_SetSleepDeepPause(MCWDT0, CY_MCWDT_COUNTER2, 1ul); // enable

    Cy_MCWDT_SetDebugRun(MCWDT0, CY_MCWDT_COUNTER0, 1ul); // enable
    Cy_MCWDT_SetDebugRun(MCWDT0, CY_MCWDT_COUNTER1, 1ul); // enable
    Cy_MCWDT_SetDebugRun(MCWDT0, CY_MCWDT_COUNTER2, 1ul); // enable

    Cy_MCWDT_Enable(MCWDT0, 
                    CY_MCWDT_CTR_Msk,  // enable all counter
                    0);

    while(Cy_MCWDT_GetEnabledStatus(MCWDT0, CY_MCWDT_COUNTER0) != 1ul);
    while(Cy_MCWDT_GetEnabledStatus(MCWDT0, CY_MCWDT_COUNTER1) != 1ul);
    while(Cy_MCWDT_GetEnabledStatus(MCWDT0, CY_MCWDT_COUNTER2) != 1ul);


//      Cy_MCWDT_DeInit(MCWDT0);
//    Cy_MCWDT_Init(MCWDT0, &mcwdtConfig);
//    Cy_MCWDT_Unlock(MCWDT0);
//    Cy_MCWDT_SetInterruptMask(MCWDT0, CY_MCWDT_CTR_Msk);
//    Cy_MCWDT_Enable(MCWDT0, 
//                    CY_MCWDT_CTR_Msk,  // enable all counter
//                    0);
//    Cy_MCWDT_Lock(MCWDT0);
//    Cy_WDT_Init();                      /* Upper Limit: 1sec and reset */
//    Cy_WDT_Unlock();
//    Cy_WDT_SetUpperLimit(40000ul);      /* Upper Limit: 1sec (override) */
//    Cy_WDT_SetDebugRun(CY_WDT_ENABLE);  /* This is necessary when using debugger */
//    Cy_WDT_Lock();
//    Cy_WDT_Enable();
}

void Feed_Dog(void)
{

    Cy_MCWDT_ClearWatchdog(MCWDT0, CY_MCWDT_COUNTER0);
    Cy_MCWDT_ClearWatchdog(MCWDT0, CY_MCWDT_COUNTER1);


}
