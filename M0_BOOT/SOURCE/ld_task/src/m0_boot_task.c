/*
 * m0_task.c
 *
 *  Created on: 2026年1月22日
 *      Author: hzldy
 */

#include "m0_boot_task.h"
#include "module_header_file.h"
#include "can_user.h"
#include "software_version.h"



void Feed_Dog_100ms_func(void);
void Task_1ms_cycle(void);

static void task_idle(void);

static uint8_t s_task_num;
static uint32_t s_task_tick;

#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)
static stSOFT_TIMER s_task_timer;
/* Configuration tasks */
static const stTASK_CFG s_task_cfg_tbl[] = 
{
    {
        .time_init_cnt                      = 0,
        .time_period                        = 100,
        .func                               = Feed_Dog_100ms_func
    },
//    {
//        .time_init_cnt                      = 0,
//        .time_period                        = 100,
//        .func                               = Adc_Process
//    },
//    {
//        .time_init_cnt                      = 0,
//        .time_period                        = 1,
//        .func                               = Task_1ms_cycle
//    },
//    {
//        .time_init_cnt                      = 0,
//        .time_period                        = 100,
//        .func                               = Rte_Process
//    },
//    {
//        .time_init_cnt                      = 1,
//        .time_period                        = 10,
//        .func                               = can_driver_process
//    },
    {
        .time_init_cnt                      = 3,
        .time_period                        = 10,
        .func                               = can_user_process
    },
    {
        .time_init_cnt                      = 4,
        .time_period                        = 10,
        .func                               = dcm_period_10ms_process
    },
    {
        .time_init_cnt                      = 5,
        .time_period                        = 10,
        .func                               = uds_user_period_10ms_process
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
extern void BACK_TO_START(void);
uint32_t flag;

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
    uds_user_process();
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
                    s_task_info[i].sts = TASK_STS_RUNNING;
                    performance_test_start(i);
                    s_task_cfg_tbl[i].func();
                    performance_test_end(i);
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
    Cy_Tcpwm_Counter_ClearTC_Intr(TCPWM0_GRP0_CNT50);
    s_task_tick++;
    systemticks_callback();
    cantp_period_1ms_process();
}

uint08 task_num_get(void)
{
    return TASK_NUM;
}

static void hardware_init(void)
{
    uint32_t interruptState;
    SystemInit();


    __enable_irq();
    
    /* Enable CM4. CY_CORTEX_M4_APPL_ADDR is calculated in linker script, check it in case of problems. */
    Cy_Flashc_MainWriteEnable();
    Cy_Flashc_WorkWriteEnable();

    Work_Flash_Read(APP_BOOT_JUMP_FLAG_ADDR,(uint8_t*)&flag,4);
    if(flag == TO_APP_FLAG && *(uint32_t *)APP_M0_A_START_ADDR != 0xFFFFFFFF)
    {
      __disable_irq();
      BACK_TO_START();
    }
    TASK_DISABLE_IRQ(interruptState);
    Sysclk_Init();
    PortInit();
    Irq_Init();
    Tcpwm_Init();
    Canfd_Init();
    Flash_Init();
    TASK_ENABLE_IRQ(interruptState);
    Wdg_Init();
}
extern void Flash_Test(void);
extern void eeprom_drv_test(void);

static void software_init(void)
{
#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)
    task_data_init();
    bootloader_init();
//	soft_timer_init();
    can_user_init();
    uds_user_init();
    can_dtc_init();
    eeprom_app_init();
#endif //#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)
//    Flash_Test();
//    eeprom_drv_test();
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
    }
    else
    {
        
    }
}
#endif //#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)

//uint8_t test[8] = {0,1,2,3,4,5,6,7};
////uint16_t adcChResult[4];
//cy_stc_adc_ch_status_t adcChStatus;
//uint8_t tta = 0,ttb = 0;
void Feed_Dog_100ms_func(void)
{
//        Can_Transmit(CY_CANFD0_TYPE, 0x555, 8, test,false);
    Feed_Dog();
}


void Task_1ms_cycle(void)
{



}
