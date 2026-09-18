/*
 * m0_task.c
 *
 *  Created on: 2026��1��22��
 *      Author: hzldy
 */

#include "m0_task.h"
#include "module_header_file.h"
#include "software_version.h"
#include "xcp_port.h"




void test_100ms_func(void);
void Task_1ms_cycle(void);

static void task_idle(void);

static uint8_t s_task_num;
static uint32_t s_task_tick;

#pragma location = ".user_info"
 __root const  uint08 g_user_info[USER_INFO_SIZE] = 
{
    LOCAL_SW_VER_H, /* Byte[0] */
    LOCAL_SW_VER_M, /* Byte[1] */
    LOCAL_SW_VER_L, /* Byte[2] */
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
        .func                               = Adc_Process
    },
    {
        .time_init_cnt                      = 0,
        .time_period                        = 1,
        .func                               = Task_1ms_cycle
    },
    {
        .time_init_cnt                      = 0,
        .time_period                        = 1,
        .func                               = XcpPort_MainFunction
    },
    {
        .time_init_cnt                      = 0,
        .time_period                        = 100,
        .func                               = Rte_Process
    },
    {
        .time_init_cnt                      = 0,
        .time_period                        = 2,
        .func                               = uds_user_process
    },
    {
        .time_init_cnt                      = 2,
        .time_period                        = 10,
        .func                               = can_dtc_process
    },
//    #endif
    {
        .time_init_cnt                      = 3,
        .time_period                        = 10,
        .func                               = can_user_process /* TASK_can_USER_INDEX = 4 */
    },
//    #ifdef TASK_UDS_EN
    {
        .time_init_cnt                      = 4,
        .time_period                        = 10,
        .func                               = dcm_period_10ms_process
    },
    {
        .time_init_cnt                      = 40,
        .time_period                        = 100,
        .func                               = dtc_user_process
    },
    {
        .time_init_cnt                      = 0,
        .time_period                        = 10,
        .func                               = eeprom_app_process
    },
    {
        .time_init_cnt                      = 0,
        .time_period                        = 0,
        .func                               = Flash_Task_Process
    },
//    #endif
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
    //����BSW�����������������MAX_TIMER_CALLBACKS
    task_register_task(uds_user_process,        PERIOD_2MS,   0,  true);
    task_register_task(eeprom_app_process,      PERIOD_10MS,  0,  true);
    task_register_task(can_driver_process,      PERIOD_10MS,  1,  true);
    task_register_task(can_dtc_process,         PERIOD_2MS,   2,  true);
    task_register_task(can_user_process,        PERIOD_10MS,  3,  true);
    task_register_task(dcm_period_10ms_process, PERIOD_10MS,  4,  true);
    task_register_task(XcpPort_MainFunction,    PERIOD_1MS,   0,  true);
    task_register_task(input_process,           PERIOD_10MS,  6,  true);
    task_register_task(output_process,          PERIOD_50MS,  0,  true);
    task_register_task(cmos_74hc4051_process,   PERIOD_50MS,  3,  true);
    task_register_task(power_process,           PERIOD_100MS, 0,  true);
    task_register_task(adc_process,             PERIOD_100MS, 10, true);
    task_register_task(tmc_rte_process,         PERIOD_100MS, 30, true);
    task_register_task(dtc_user_process,        PERIOD_100MS, 40, true);
    task_register_task(lin_user_process,        PERIOD_100MS, 50, true);
    //����APP����

#endif
}
/**
 * @brief ע�ᶨʱ����
 * @param callback �ص�����ָ��
 * @param interval_ms ִ�м��(ms)
 * @param delay_ms �״�ִ���ӳ�(ms)��0��ʾ����ִ��
 * @param initially_enabled ��ʼ�Ƿ�����
 * @return ����ID��0xFF��ʾע��ʧ�ܣ�
 */
uint08 task_register_task(TimerCallback_t callback,
        uint32 interval_ms,
        uint32 delay_ms,
        uint08 initially_enabled)
{
	return timer_register_taskex(callback,interval_ms,delay_ms,initially_enabled);
}

/**
 * @brief ͨ���ص�����ָ��ɾ������
 * @param callback Ҫɾ��������ص�����ָ��
 * @return 1��ʾ�ɹ���0��ʾʧ��
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
    can_busoff_process();
    can_busoff_1ms_period_cbk(CAN_CHN_0);
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

    TASK_DISABLE_IRQ(interruptState);
    Sysclk_Init();
    PortInit();
   
    Irq_Init();
    Tcpwm_Init();
    Adc_Init();
    Canfd_Init();
    Ipc_Init_Pipe();
    Flash_Init();
    TASK_ENABLE_IRQ(interruptState);
    Cy_SysEnableApplCore(CY_CORTEX_M4_APPL_ADDR);
    Wdg_Init();
//    Cy_SysTick_Init(CY_SYSTICK_CLOCK_SOURCE_CLK_CPU, 80000ul); //80,000,000 / 80,000 = 1000hz
//    Cy_SysTick_SetCallback(0ul, task_isr);
}
extern void Flash_Test(void);

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
//	dtc_user_init();
#endif //#if (TASK_USE_TIMER_SERVICE_ENABLE == 0)
    XcpPort_Init();
//  Flash_Test();
    PTC_Controller_Init();
     initAD_value();
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

//uint16_t adcChResult[4];
void test_100ms_func(void)
{
    PeriodReportData(0x20B);
    PeriodReportData(0x20C);
    PeriodReportData(0x20D);
    Feed_Dog();
    PeriodReportData(0x20E);
    PeriodReportData(0x20F);  /* DEBUG: raw dq for power calibration */
    PeriodReportData(0x210);  /* DEBUG: modulation ratio & voltage utilization */
}


void Task_1ms_cycle(void)
{
  RecDataAnalyze();
  Fault_Detect();
  Execute_Funtion_1ms();
  Execute_Funtion_500ms();
  Adc_Fast_Process();
}
