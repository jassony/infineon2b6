/*
 * m0_task.c
 *
 *  Created on: 2026年1月22日
 *      Author: hzldy
 */

#include "m4_task.h"
#include "module_header_file.h"
#include <stdio.h>
#include <stdarg.h>
#include "software_version.h"
#include "mcu_load_profiler.h"

void test_100ms_func(void);
static void task_idle(void);

static uint8_t s_task_num;
static uint32_t s_task_tick;
static void Uart_Test_Init(void);

#if (MCU_LOAD_PROFILER_ENABLE != 0u)
static McuLoadProfilerToken s_sysTickProfilerToken;

static void McuLoadProfiler_sysTickBegin(void)
{
    s_sysTickProfilerToken = McuLoadProfiler_begin();
}

static void McuLoadProfiler_sysTickEnd(void)
{
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_SYSTICK,
        s_sysTickProfilerToken);
}
#endif

#pragma location = ".user_info"
 __root const  uint08 g_user_info[USER_INFO_SIZE] = 
{
    LOCAL_SW_M4_VER_H, /* Byte[0] */
    LOCAL_SW_M4_VER_M, /* Byte[1] */
    LOCAL_SW_M4_VER_L, /* Byte[2] */
    LOCAL_HW_VER_H, /* Byte[3] */
    LOCAL_HW_VER_M, /* Byte[4] */
    LOCAL_HW_VER_L, /* Byte[5] */
    LOCAL_BUILD_DATE_YEAR, /* Byte[6] */
    LOCAL_BUILD_DATE_MONTH, /* Byte[7] */
    LOCAL_BUILD_DATE_DAY, /* Byte[8] */
    LOCAL_BOOT_VER_H, /* Byte[9] */
    LOCAL_BOOT_VER_M, /* Byte[10] */
    LOCAL_BOOT_VER_L, /* Byte[11] */
    LOCAL_DEF_VAL, /* Byte[12] */
    LOCAL_DEF_VAL, /* Byte[13] */
    LOCAL_DEF_VAL, /* Byte[14] */
    LOCAL_DEF_VAL, /* Byte[15] */
    
    LOCAL_SPC_VAL, /* Byte[16] fixed,do not modify!!! */
    LOCAL_SPC_VAL, /* Byte[17] fixed,do not modify!!! */
    LOCAL_SPC_VAL, /* Byte[18] fixed,do not modify!!! */
    LOCAL_SPC_VAL, /* Byte[19] fixed,do not modify!!! */
};

#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)
static stSOFT_TIMER s_task_timer;
/* Configuration tasks */
static const stTASK_CFG s_task_cfg_tbl[] = 
{
    {
        .time_init_cnt                      = 0,
        .time_period                        = 100,
        .func                               = test_100ms_func
    },
    
    {
        .time_init_cnt                      = 0,
        .time_period                        = 100,
        .func                               = Rte_Process
    },
};
#endif //#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)


#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)
#define TASK_NUM                            (sizeof(s_task_cfg_tbl) / sizeof(stTASK_CFG))
static stTASK_INFO s_task_info[TASK_NUM];
#else
#define TASK_NUM                            timer_get_active_task_count()
#endif

static void hardware_init(void);
static void software_init(void);
#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)
static void task_data_init(void);
static void task_idle(void);
#endif //#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)
//static void task_startup_info_printf(void);


void task_init(void)
{
    hardware_init();
    software_init();    
//    task_startup_info_printf();
#if (TASK_USE_TIMER_SERVICE_ENABLE == 1)
    //添加BSW任务，最大任务数量：MAX_TIMER_CALLBACKS
    task_register_task(uds_user_process,        PERIOD_2MS,   0,  true);
    task_register_task(eeprom_app_process,      PERIOD_10MS,  0,  true);
    task_register_task(can_driver_process,      PERIOD_10MS,  1,  true);
    task_register_task(can_dtc_process,         PERIOD_2MS,   2,  true);
    task_register_task(can_user_process,        PERIOD_10MS,  3,  true);
    task_register_task(dcm_period_10ms_process, PERIOD_10MS,  4,  true);
    task_register_task(input_process,           PERIOD_10MS,  6,  true);
    task_register_task(output_process,          PERIOD_50MS,  0,  true);
    task_register_task(cmos_74hc4051_process,   PERIOD_50MS,  3,  true);
    task_register_task(power_process,           PERIOD_100MS, 0,  true);
    task_register_task(adc_process,             PERIOD_100MS, 10, true);
    task_register_task(tmc_rte_process,         PERIOD_100MS, 30, true);
    task_register_task(dtc_user_process,        PERIOD_100MS, 40, true);
    task_register_task(lin_user_process,        PERIOD_100MS, 50, true);
    //添加APP任务

#endif
}
/**
 * @brief 注册定时任务
 * @param callback 回调函数指针
 * @param interval_ms 执行间隔(ms)
 * @param delay_ms 首次执行延迟(ms)，0表示立即执行
 * @param initially_enabled 初始是否启用
 * @return 任务ID（0xFF表示注册失败）
 */
uint08 task_register_task(TimerCallback_t callback,
        uint32 interval_ms,
        uint32 delay_ms,
        uint08 initially_enabled)
{
	return timer_register_taskex(callback,interval_ms,delay_ms,initially_enabled);
}

/**
 * @brief 通过回调函数指针删除任务
 * @param callback 要删除的任务回调函数指针
 * @return 1表示成功，0表示失败
 */
uint08 task_delete_task(TimerCallback_t callback)
{
	return timer_delete_task_by_callback(callback);
}

void task_deinit(void)
{
    s_task_num = 0;
}

void task_process(void)
{
#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)
    uint8_t  i = 0;

//    if (1 == can_busoff_recover_flg_get(CAN_CHN_0))
//	{
//        can_busoff_recover_flg_set(CAN_CHN_0, 2);
//        s_task_info[TASK_can_USER_INDEX].sts = TASK_STS_READY;
//        s_task_info[TASK_can_USER_INDEX].time_last_cnt = s_task_tick;
//        can0_tx_period_reset();
//	}
//
//    if (1 == can_busoff_recover_flg_get(CAN_CHN_1))
//	{
//        can_busoff_recover_flg_set(CAN_CHN_1, 2);
//        s_task_info[TASK_can_USER_INDEX].sts = TASK_STS_READY;
//        s_task_info[TASK_can_USER_INDEX].time_last_cnt = s_task_tick;
//        can1_tx_period_reset();
//	}
	
    for (i = 0; i < TASK_NUM; i++)
    {
        switch (s_task_info[i].sts)
        {
            case TASK_STS_PENDING:
                if ((s_task_tick - s_task_info[i].time_last_cnt) >= s_task_cfg_tbl[i].time_period)
                {
                    s_task_info[i].time_last_cnt = s_task_tick;
                    s_task_info[i].sts = TASK_STS_READY;
                    
                }
                else
                {
                }
                break;
            case TASK_STS_READY:
                if (s_task_cfg_tbl[i].func != NULL)
                {
#if (MCU_LOAD_PROFILER_ENABLE != 0u)
                    McuLoadProfilerToken profilerToken;
                    McuLoadProfilerContext profilerContext;

                    profilerContext = (i == 0u)
                        ? MCU_LOAD_CONTEXT_TEST_500MS_TASK
                        : MCU_LOAD_CONTEXT_RTE_TASK;
#endif
                    s_task_info[i].sts = TASK_STS_RUNNING;
#if (MCU_LOAD_PROFILER_ENABLE != 0u)
                    profilerToken = McuLoadProfiler_begin();
#endif
                    s_task_cfg_tbl[i].func();
#if (MCU_LOAD_PROFILER_ENABLE != 0u)
                    (void)McuLoadProfiler_end(profilerContext, profilerToken);
#endif
//                    s_task_info[i].sts = TASK_STS_PENDING;
                }
                else {}
                break;
            case TASK_STS_RUNNING:
                s_task_info[i].sts = TASK_STS_PENDING;
                break;
            case TASK_STS_INIT:
                if ((s_task_tick - s_task_info[i].time_last_cnt) >= s_task_cfg_tbl[i].time_init_cnt)
                {
                    s_task_info[i].time_last_cnt = s_task_tick;
                    s_task_info[i].sts = TASK_STS_PENDING;
                }
                else {}
                break;
            default:
                s_task_info[i].sts = TASK_STS_INIT;
                break;
        }
    }

    task_idle();
#else
    timer_process_tasks();
#endif
}

void task_isr(void)
{
    s_task_tick++;
}

uint08 task_num_get(void)
{
    return TASK_NUM;
}

static void hardware_init(void)
{
  Sysclk_Init();
  Ipc_Init_Pipe();
  Irq_Init();
  Cy_SysTick_Init(CY_SYSTICK_CLOCK_SOURCE_CLK_CPU, 160000ul); //160,000,000 / 160,000 = 1000hz
#if (MCU_LOAD_PROFILER_ENABLE != 0u)
  Cy_SysTick_SetCallback(0ul, McuLoadProfiler_sysTickBegin);
  Cy_SysTick_SetCallback(1ul, task_isr);
  Cy_SysTick_SetCallback(4ul, McuLoadProfiler_sysTickEnd);
#else
  Cy_SysTick_SetCallback(0ul, task_isr);
#endif
//  Wdg_Init();
}

static void software_init(void)
{
  
  Adc_Enable_Func();
  Tcpwm_Enable_Func();
#ifdef TEST_UART
    Uart_Test_Init();

#endif

}


#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)
static void task_data_init(void)
{
    uint08 i = 0;
    
    s_task_num = TASK_NUM;
    s_task_tick = 0;
    for (i = 0; i < TASK_NUM; i++)
    {
        s_task_info[i].time_last_cnt = s_task_tick;
        s_task_info[i].sts = TASK_STS_INIT;
    }
    soft_timer_set(&s_task_timer, TASK_IDLE_PRINT_TIME_MS);
}
extern void FOC_Loop_Check(void);

static void task_idle(void)
{
    uint08 i = 0;
    
    for (i = 0; i < TASK_NUM; i++)
    {
        if (TASK_STS_RUNNING == s_task_info[i].sts)
        {
            break;
        }
    }
    if (i >= TASK_NUM) /* No task was executed at this time */
    {
        /* Print information or set the amount of observations */
        if (DEF_TRUE == is_soft_timer_timeout(&s_task_timer))
        {
            soft_timer_reset(&s_task_timer);
//            DEF_PRINTF_TIME(RTT_CTRL_TEXT_GREEN"Idle task printing. \r\n");
        }
        FOC_Loop_Check();
    }
    else
    {
        
    }
}
#endif //#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)

void test_100ms_func(void)
{

  Cy_GPIO_Inv(GPIO_PRT0, 0); 
//  Feed_Dog();

}



/////uart_test

#define E_UART_RECV_THRESHOLD    8
#define E_UART_RING_BUF_SIZE     512
#define E_UART_USER_BUF_SIZE     512
#define E_UART_RX_INTR_FACTER     (                              \
                                 CY_SCB_UART_RX_TRIGGER      |   \
                               /*CY_SCB_UART_RX_NOT_EMPTY    | */\
                               /*CY_SCB_UART_RX_FULL         | */\
                                 CY_SCB_UART_RX_OVERFLOW     |   \
                                 CY_SCB_UART_RX_UNDERFLOW    |   \
                                 CY_SCB_UART_RX_ERR_FRAME    |   \
                                 CY_SCB_UART_RX_ERR_PARITY   |   \
                                 CY_SCB_UART_RX_BREAK_DETECT |   \
                                 0                               \
                                )
#define E_UART_TX_INTR_FACTER     (                              \
                                 CY_SCB_UART_TX_TRIGGER      |   \
                               /*CY_SCB_UART_TX_NOT_FULL     | */\
                               /*CY_SCB_UART_TX_EMPTY        | */\
                                 CY_SCB_UART_TX_OVERFLOW     |   \
                               /*CY_SCB_UART_TX_UNDERFLOW    | */\
                                 CY_SCB_UART_TX_DONE         |   \
                               /*CY_SCB_UART_TX_NACK         | */\
                               /*CY_SCB_UART_TX_ARB_LOST     | */\
                                 0                               \
                                )

cy_stc_scb_uart_context_t   g_stc_uart_context;
cy_stc_scb_uart_config_t    g_stc_uart_config = {
                                                   .uartMode                   = CY_SCB_UART_STANDARD,
                                                   .oversample                 = 1,
                                                   .dataWidth                  = 8,
                                                   .enableMsbFirst             = false,
                                                   .stopBits                   = CY_SCB_UART_STOP_BITS_1,
                                                   .parity                     = CY_SCB_UART_PARITY_NONE,
                                                   .enableInputFilter          = false,
                                                   .dropOnParityError          = false,
                                                   .dropOnFrameError           = false,
                                                   .enableMutliProcessorMode   = false,
                                                   .receiverAddress            = 0,
                                                   .receiverAddressMask        = 0,
                                                   .acceptAddrInFifo           = false,
                                                   .irdaInvertRx               = false,
                                                   .irdaEnableLowPowerReceiver = false,
                                                   .smartCardRetryOnNack       = false,
                                                   .enableCts                  = false,
                                                   .ctsPolarity                = CY_SCB_UART_ACTIVE_LOW,
                                                   .rtsRxFifoLevel             = 0,
                                                   .rtsPolarity                = CY_SCB_UART_ACTIVE_LOW,
                                                   .breakWidth                 = 0,
                                                   .rxFifoTriggerLevel         = 0,
                                                   .rxFifoIntEnableMask        = E_UART_RX_INTR_FACTER,
                                                   .txFifoTriggerLevel         = 0,
                                                   .txFifoIntEnableMask        = E_UART_TX_INTR_FACTER
                                                };

uint8_t                     g_uart_out_data[128];                       // TX Buffer for Terminal Print
uint8_t                     g_uart_in_data[128];                        // RX Buffer
uint8_t                     g_uart_rx_ring[E_UART_RING_BUF_SIZE] = {0}; // RX Ring Buffer
uint8_t                     g_uart_user_buf[E_UART_USER_BUF_SIZE];      // User Buffer for coping from Ring Buffer




//void Scb_UART_IntrISR(void)
//{
//#if defined(E_UART_ECHO_INTR_1BYTE)
//    /* UART Echo Test (High-Level)            */
//    /* (2) Interrupt & Receive by 1 byte unit */
//    uint32_t num = Cy_SCB_UART_GetNumInRxFifo(CY_USB_SCB_UART_TYPE);
//    if (num != 0) {
//        Cy_SCB_UART_Receive(CY_USB_SCB_UART_TYPE, &g_uart_in_data[0], num, &g_stc_uart_context);
//        Cy_SCB_UART_Transmit(CY_USB_SCB_UART_TYPE, &g_uart_in_data[0], num, &g_stc_uart_context);
//        Cy_SCB_SetRxFifoLevel(CY_USB_SCB_UART_TYPE, 0);
//    }
//#endif
//
//    /* UART interrupt handler */
//    Cy_SCB_UART_Interrupt(CY_USB_SCB_UART_TYPE, &g_stc_uart_context);
//    NVIC_ClearPendingIRQ(CPUIntIdx1_IRQn);
//}
static void Uart_Test_Init(void)
{
    cy_stc_gpio_pin_config_t    stc_port_pin_cfg_uart = {0};
    cy_stc_sysint_irq_t         stc_sysint_irq_cfg_uart;

    stc_port_pin_cfg_uart.driveMode = CY_GPIO_DM_HIGHZ;
    stc_port_pin_cfg_uart.hsiom     = CY_USB_SCB_UART_RX_PIN_MUX;
    Cy_GPIO_Pin_Init(GPIO_PRT0, 0, &stc_port_pin_cfg_uart);

    /* P13.1 -> scb[3].uart_tx */
    stc_port_pin_cfg_uart.driveMode = CY_GPIO_DM_STRONG_IN_OFF;
    stc_port_pin_cfg_uart.hsiom     = CY_USB_SCB_UART_TX_PIN_MUX;
    Cy_GPIO_Pin_Init(GPIO_PRT0, 1, &stc_port_pin_cfg_uart);

    Cy_SysClk_PeriphAssignDivider(PCLK_SCB0_CLOCK, CY_SYSCLK_DIV_24_5_BIT, 0u);
    Cy_SysClk_PeriphSetFracDivider(CY_SYSCLK_DIV_24_5_BIT, 0u, 9,  0);   // 80,000,000/ (9+1+0/32)/8 = 2M
    g_stc_uart_config.oversample = 8;

    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_24_5_BIT, 0u);
//    stc_sysint_irq_cfg_uart.sysIntSrc = CY_USB_SCB_UART_IRQN;
//    stc_sysint_irq_cfg_uart.intIdx    = CPUIntIdx7_IRQn;
//    stc_sysint_irq_cfg_uart.isEnabled = true;
//    Cy_SysInt_InitIRQ(&stc_sysint_irq_cfg_uart);
//    Cy_SysInt_SetSystemIrqVector(stc_sysint_irq_cfg_uart.sysIntSrc, Scb_UART_IntrISR);
//    
//
//    /* Initilize & Enable UART */
//    Cy_SCB_UART_DeInit(CY_USB_SCB_UART_TYPE);
    Cy_SCB_UART_Init(SCB0, &g_stc_uart_config, &g_stc_uart_context);
//    Cy_SCB_UART_RegisterCallback(CY_USB_SCB_UART_TYPE, (scb_uart_handle_events_t)Scb_UART_Event, &g_stc_uart_context);
    Cy_SCB_UART_Enable(SCB0);

    
    
}

void Term_Printf(void *fmt, ...)
{
#ifdef TEST_UART  
    va_list arg;

    /* UART Print */
    va_start(arg, fmt);
    vsprintf((char*)&g_uart_out_data[0], (char*)fmt, arg);
    while (Cy_SCB_UART_IsTxComplete(SCB0) != true) {};
    Cy_SCB_UART_PutArray(SCB0, g_uart_out_data, strlen((char *)g_uart_out_data));
    va_end(arg);
#endif
}



