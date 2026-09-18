/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : performance_test.h
* Author        : yangming
* Date          : 2024-05-23
* Version       : 1.00
* Description   : performance test.
* Others        : None
*
****************************************************************************************************/
#ifndef _PERFORMANCE_TEST_H
#define _PERFORMANCE_TEST_H
#include "platform_common_typdef.h"

#define PERFORMANCE_TEST_EN

#define BASETIME_FREQ                               48000000U
#define BASETIME_PERIOD_CNT                         (BASETIME_FREQ / 1000U)
#define BASETIME_US_CNT                             (BASETIME_FREQ / 1000000U)
#define BASETIME_START_DLT                          2400u /* 50us */

typedef enum _monitor_member                        eMONITOR_MEMBER;
typedef struct _monitor_info                        stMONITOR_INFO;

enum _monitor_member
{
    MONITOR_MEMBER_1 = 0,
    MONITOR_MEMBER_2,
    MONITOR_MEMBER_3,
    MONITOR_MEMBER_4,
    MONITOR_MEMBER_5,
    MONITOR_MEMBER_6,
    MONITOR_MEMBER_7,
    MONITOR_MEMBER_8,
    MONITOR_MEMBER_9,
    MONITOR_MEMBER_10,
    MONITOR_MEMBER_11,
    MONITOR_MEMBER_12,
    MONITOR_MEMBER_13,
    MONITOR_MEMBER_14,
    MONITOR_MEMBER_15,
    MONITOR_MEMBER_16,
    MONITOR_MEMBER_17,
    MONITOR_MEMBER_18,
    MONITOR_MEMBER_19,
    MONITOR_MEMBER_20,
    MONITOR_MEMBER_21,
    MONITOR_MEMBER_22,
    MONITOR_MEMBER_23,
    MONITOR_MEMBER_24,
    MONITOR_MEMBER_25,
    MONITOR_MEMBER_26,
    MONITOR_MEMBER_27,
    MONITOR_MEMBER_28,
    MONITOR_MEMBER_29,
    MONITOR_MEMBER_30,
    MONITOR_MEMBER_31,
    MONITOR_MEMBER_32,

    MONITOR_MEMBER_NUM
};

struct _monitor_info
{
    uint32                  time_start;
    uint32                  cur_interval;
    uint32                  max_interval;
    uint32                  time_period_num;
    uint08                  time_test_lock;
};

extern void performance_test_init(void);
extern void performance_test_start(eMONITOR_MEMBER member);
extern void performance_test_end(eMONITOR_MEMBER member);
extern uint32 performance_test_get_member_cur_interval_us(eMONITOR_MEMBER member);
extern uint32 performance_test_get_member_max_interval_us(eMONITOR_MEMBER member);
extern uint32 performance_test_get_member_max_interval_ms(eMONITOR_MEMBER member);
extern void performance_test_printf(void);
extern void print_period_task(void);
extern void performance_test_isr(void);

#endif /* _PERFORMANCE_TEST_H */
