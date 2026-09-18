/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : performance_test.c
* Author        : yangming
* Date          : 2024-05-23
* Version       : 1.00
* Description   : performance test.
* Others        : None
*
****************************************************************************************************/
#include "performance_test.h"
//#include "device_registers.h"
#include "common_mem_op.h"
//#include "m0_task.h"
extern uint32 SystemCoreClock;
extern uint08 task_num_get(void);


#define TIMEBASE_CUR_CNT                       SystemCoreClock
#ifdef PERFORMANCE_TEST_EN
static stMONITOR_INFO s_monitor_obj[MONITOR_MEMBER_NUM];
static uint32 s_monitor_member_us[MONITOR_MEMBER_NUM];
#endif
uint32 g_heartbeat_cnt;

void performance_test_init(void)
{
    #ifdef PERFORMANCE_TEST_EN
    common_memset((uint08*)s_monitor_obj, 0U, sizeof(s_monitor_obj));
    #endif
}

void performance_test_start(eMONITOR_MEMBER member)
{
    #ifdef PERFORMANCE_TEST_EN
    if (member >= MONITOR_MEMBER_NUM)
    {
        return;
    }

    s_monitor_obj[member].time_start = TIMEBASE_CUR_CNT;
    if (   (s_monitor_obj[member].time_start < BASETIME_START_DLT)
        || (s_monitor_obj[member].time_start > (BASETIME_PERIOD_CNT - BASETIME_START_DLT))
        )
    {
        s_monitor_obj[member].time_test_lock = 1;
    }
    else
    {
        s_monitor_obj[member].time_test_lock = 0;
        s_monitor_obj[member].cur_interval = 0;
        s_monitor_obj[member].time_period_num = 0;
    }
    #endif
}

void performance_test_end(eMONITOR_MEMBER member)
{
    #ifdef PERFORMANCE_TEST_EN
   
    uint32 basetime_cur_val = 0;
    uint32 temp = 0;
    if ((member >= MONITOR_MEMBER_NUM) || (1 == s_monitor_obj[member].time_test_lock))
    {
        return;
    }

    basetime_cur_val = TIMEBASE_CUR_CNT;
    temp = s_monitor_obj[member].time_period_num * BASETIME_PERIOD_CNT;
    s_monitor_obj[member].cur_interval = temp + s_monitor_obj[member].time_start - basetime_cur_val;
    if (s_monitor_obj[member].cur_interval > s_monitor_obj[member].max_interval)
    {
        s_monitor_obj[member].max_interval = s_monitor_obj[member].cur_interval;
    }
    else {}
    s_monitor_obj[member].time_period_num = 0;
    #endif
}

uint32 performance_test_get_member_cur_interval_us(eMONITOR_MEMBER member)
{
    #ifdef PERFORMANCE_TEST_EN
    if (member >= MONITOR_MEMBER_NUM)
    {
        return 0;
    }
    return (s_monitor_obj[member].cur_interval / BASETIME_US_CNT);
    #else
    return 0;
    #endif
}

uint32 performance_test_get_member_max_interval_us(eMONITOR_MEMBER member)
{
    #ifdef PERFORMANCE_TEST_EN
    if (member >= MONITOR_MEMBER_NUM)
    {
        return 0;
    }
    return (s_monitor_obj[member].max_interval / BASETIME_US_CNT);
    #else
    return 0;
    #endif
}


uint32 performance_test_get_member_max_interval_ms(eMONITOR_MEMBER member)
{
    #ifdef PERFORMANCE_TEST_EN
    if (member >= MONITOR_MEMBER_NUM)
    {
        return 0;
    }
    return (s_monitor_obj[member].max_interval / BASETIME_PERIOD_CNT);
    #else
    return 0;
    #endif
}

void performance_test_isr(void)
{
    #ifdef PERFORMANCE_TEST_EN
    uint08 i = 0;
    
    for (i = 0; i < MONITOR_MEMBER_NUM; i++)
    {
        s_monitor_obj[i].time_period_num++;
    }
    #endif
}

/* period: 1s */
void performance_test_printf(void)
{
    uint32 cur_us = 0;
    
    g_heartbeat_cnt++;
    #ifdef PERFORMANCE_TEST_EN
    static uint08 member_cnt = 0;
    cur_us = performance_test_get_member_cur_interval_us(member_cnt);
    s_monitor_member_us[member_cnt] = performance_test_get_member_max_interval_us(member_cnt);
//    DEF_PRINTF_TIME(RTT_CTRL_TEXT_GREEN"monitor member[%d] cur: %dus; max: %dus.\r\n", member_cnt, cur_us, s_monitor_member_us[member_cnt]);
    member_cnt++;
    if (member_cnt >= task_num_get())
    {
        member_cnt = 0;
    }
    #endif
}

extern void bl_printf(void);
/* The recommended call period is 5 seconds */
void print_period_task(void)
{
//    DEF_PRINTF_TIME(RTT_CTRL_TEXT_GREEN"heartbeat cycle count: %d.\r\n", g_heartbeat_cnt);
    bl_printf();
}

