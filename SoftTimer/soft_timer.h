/****************************************************************************************************
* Copyright (c) LingDong Co.Ltd. All rights reserved.
*
* File Name     : timer_service.c
* Author        : jiangbaoliang
* Date          : 2025-09-17
* Version       : 1.00
* Description   : Provides periodic task timing service and software timing service
*
* Others        : None
*
****************************************************************************************************/

#ifndef BSW_SYSSRV_TIMERS_TIMER_SERVICE_H_
#define BSW_SYSSRV_TIMERS_TIMER_SERVICE_H_
/*==================================================================================================
*                                     HEADER FILE
==================================================================================================*/
#include "platform_common_typdef.h"

/*==================================================================================================
*                                     MACRO DEFINITION
==================================================================================================*/
/* Define periods (based on 1ms timer) */
#define PERIOD_2MS                    2U
#define PERIOD_5MS                    5U
#define PERIOD_10MS                   10U
#define PERIOD_20MS                   20U
#define PERIOD_30MS                   30U
#define PERIOD_40MS                   40U
#define PERIOD_50MS                   50U
#define PERIOD_100MS                  100U
#define PERIOD_200MS                  200U
#define PERIOD_500MS                  500U
#define PERIOD_1000MS                 1000U
#define PERIOD_2S                     2000U
#define PERIOD_5S                     5000U
#define PERIOD_10S                    10000U

/* Maximum number of callback functions */
#define MAX_TIMER_CALLBACKS    	   40u
/* Software timer unit counter (1ms) */
#define SOFT_TIMER_UINT_COUNTER       1U

/*==================================================================================================
*                                  ENUMERATION DEFINITION
==================================================================================================*/
/* Timer task state enumeration */
typedef enum
{
    TIMER_TASK_STATE_UNINIT = 0,   /* Uninitialized */
    TIMER_TASK_STATE_INIT,         /* Initialized but not started */
    TIMER_TASK_STATE_PENDING,      /* Disabled or waiting */
    TIMER_TASK_STATE_READY,        /* Enabled and ready to run */
    TIMER_TASK_STATE_RUNNING       /* Currently executing */
} TimerTaskState_t;

/*==================================================================================================
*                                  STRUCTURE DEFINITION
==================================================================================================*/
/* Timer callback function type */
typedef void (*TimerCallback_t)(void);
/* Software timer callback function type */
typedef void (*soft_timer_func)(void);
/* Software timer structure */
typedef struct _soft_timer_info           stSOFT_TIMER;
/* Extended software timer structure */
typedef struct _soft_timer_info_ex        stSOFT_TIMER_EX;

/* Timer task structure */
typedef struct
{
    TimerCallback_t callback;  /* Callback function */
    uint32 interval;           /* Call interval (ms) */
    uint32 lastTick;           /* Last call time */
    uint32 delay;              /* Initial delay (ms) */
    uint08 enabled;            /* Enable flag */
    TimerTaskState_t state;    /* Task state */
} TimerTask_t;

/* Basic software timer structure */
struct _soft_timer_info
{
    uint32                  start;    /* Start time (ms) */
    uint32                  interval; /* Interval time (ms), i.e., set timeout period */
};

/* Extended software timer structure with first delay and cycle configuration */
struct _soft_timer_info_ex
{
    uint32 start;            /* Timing start time */
    uint32 first_interval;   /* First timeout interval */
    uint32 cycle_interval;   /* Subsequent cycle timeout interval */
    typ_bool is_first_time;  /* Whether it is the first time detection */
} ;


/*==================================================================================================
*                                GLOBAL VARIABLE DECLARATION
==================================================================================================*/



/*==================================================================================================
*                         			 FUNCTION DECLARATION
==================================================================================================*/

/* Initialize timer service */
void timer_service_init(void);
/* System ticks callback function */
void systemticks_callback(void);
/* Register a timer task with extended parameters */
uint08 timer_register_taskex(TimerCallback_t callback,
                            uint32 interval_ms,
                            uint32 delay_ms,
                            uint08 initially_enabled);
/* Delete a timer task by ID */
uint08 timer_delete_task(uint08 taskId);
/* Delete a timer task by callback function */
uint08 timer_delete_task_by_callback(TimerCallback_t callback);
/* Set task state (enabled/disabled) by ID */
void timer_set_task_state(uint08 taskId, uint08 enabled);
/* Set task state (enabled/disabled) by callback function */
void timer_set_task_state_by_callback(TimerCallback_t callback, uint08 enabled);
/* Get count of active tasks */
uint08 timer_get_active_task_count(void);
/*Immediately execute a timer task by task ID*/
uint08 timer_execute_task_immediately(uint08 taskId);
/*Immediately execute a timer task by callback function pointer*/
uint08 timer_execute_task_immediately_by_callback(TimerCallback_t callback);
/*Get task status by task ID*/
TimerTaskState_t timer_get_task_status(uint08 taskId);
/*Get task status by callback function pointer*/
TimerTaskState_t timer_get_task_status_by_callback(TimerCallback_t callback);
/* Process all timer tasks */
void timer_process_tasks(void);

/* Get current system ticks */
uint32 timer_get_ticks(void);
/* Set software timer with interval */
void soft_timer_set(stSOFT_TIMER* t, uint32 interval);
/* Check if software timer has timed out */
typ_bool is_soft_timer_timeout(stSOFT_TIMER* t);
/* Reset software timer */
void soft_timer_reset(stSOFT_TIMER* t);
/* Change software timer interval (suitable for dynamic adjustment) */
void soft_timer_change(stSOFT_TIMER* t, uint32 interval);

/* Set extended software timer with first and cycle intervals */
void soft_timer_set_ex(stSOFT_TIMER_EX* t, uint32 first_interval, uint32 cycle_interval);
/* Check if extended software timer has timed out */
typ_bool is_soft_timer_timeout_ex(stSOFT_TIMER_EX* t);
/* Reset extended software timer */
void soft_timer_reset_ex(stSOFT_TIMER_EX* t);

#endif /* BSW_SYSSRV_TIMERS_TIMER_SERVICE_H_ */




