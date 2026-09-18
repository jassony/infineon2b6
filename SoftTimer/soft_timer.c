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
****************************************************************************************************/
/*==================================================================================================
*                                     HEADER FILE
==================================================================================================*/
#include "soft_timer.h"
#include "common_mem_op.h"

/*==================================================================================================
*                                     MACRO DEFINITION
==================================================================================================*/
#define UINT32_MAX_VALUE                0xFFFFFFFFUL  /* Maximum value of 32-bit unsigned integer */
#define SOFT_TIMER_TEST_ENABLE          0U            /* soft timer debug code switch: 1=Enable, 0=Disable */

/*==================================================================================================
*                                     GLOBAL VARIABLES
==================================================================================================*/


/*==================================================================================================
*                                     STATIC  VARIABLES
==================================================================================================*/
volatile uint32 s_system_ticks = 0;  /* System tick count */
static TimerTask_t s_timer_tasks[MAX_TIMER_CALLBACKS];
static uint08 s_active_task_count = 0;  /* Current active task counter */

#if(SOFT_TIMER_TEST_ENABLE == 1U)
static stSOFT_TIMER s_test_timer1;  /* For testing */
#endif/*(SOFT_TIMER_TEST_ENABLE == 1U)*/

/*==================================================================================================
*                                STATIC FUNCTION PROTOTYPE DECLARATION
==================================================================================================*/
#if(SOFT_TIMER_TEST_ENABLE == 1U)
static void soft_timer_test(void);  /* Software timer test */
#endif/*(SOFT_TIMER_TEST_ENABLE == 1U)*/

/*==================================================================================================
*                                STATIC FUNCTION IMPLEMENTATION
==================================================================================================*/
#if(SOFT_TIMER_TEST_ENABLE == 1U)
/******************************************************************************************
 * @brief  Software timer test function
 * @param  None
 * @return None
 * @note   For testing software timer functionality
 *****************************************************************************************/
static void soft_timer_test(void)
{
    static uint08 flag1 = 0;

    if(flag1 == 0)  /* Initialization */
    {
        flag1 = 1;
        soft_timer_set(&s_test_timer1, PERIOD_10MS);  /* Initialize timer*/
    }
    else  /* Cyclic timeout check */
    {
        if (DEF_TRUE == is_soft_timer_timeout(&s_test_timer1))  /* Check if timeout, return TRUE means timer expired */
        {
            soft_timer_reset(&s_test_timer1);  /* Restart timing */
            /* Do specific task */
            #if 1
            test_gpio();  /* GPIO toggle */
            #endif
        }
    }
}
#endif/*(SOFT_TIMER_TEST_ENABLE == 1U)*/

/*==================================================================================================
*                                FUNCTION IMPLEMENTATION
==================================================================================================*/

/******************************************************************************************
 * @brief  Interrupt callback function, called in 1ms interrupt timer function
 * @param  None
 * @return None
 * @note   Must be called in 1ms timer interrupt
 *****************************************************************************************/
void systemticks_callback(void)
{
    s_system_ticks += SOFT_TIMER_UINT_COUNTER;  /* Update tick count */
}

/******************************************************************************************
 * @brief  Initialize timer service
 * @param  None
 * @return None
 * @note   Must be called once at system startup
 *****************************************************************************************/
void timer_service_init(void)
{
    /* 1. Initialize all variables */
    s_system_ticks = 0;
    s_active_task_count = 0;  /* Reset counter */
    common_memset((uint08*)s_timer_tasks, 0, sizeof(s_timer_tasks)); /* Clear all tasks */
}

/******************************************************************************************
 * @brief  Register a timer task with extended parameters
 * @param[in]  callback: Callback function pointer
 * @param[in]  interval_ms: Execution interval (ms)
 * @param[in]  delay_ms: First execution delay (ms), 0 means execute immediately
 * @param[in]  initially_enabled: Initially enabled or not
 * @return Task ID (0xFF indicates registration failure)
 * @note   Returns 0xFF if no available task slots
 *****************************************************************************************/
uint08 timer_register_taskex(TimerCallback_t callback,
                            uint32 interval_ms,
                            uint32 delay_ms,
                            uint08 initially_enabled)
{
    for (uint08 i = 0; i < MAX_TIMER_CALLBACKS; i++)
    {
        if (s_timer_tasks[i].callback == NULL)
        {
            s_timer_tasks[i].callback = callback;
            s_timer_tasks[i].interval = interval_ms;
            s_timer_tasks[i].delay = delay_ms;
            s_timer_tasks[i].lastTick = s_system_ticks;
            s_timer_tasks[i].enabled = initially_enabled;
            s_timer_tasks[i].state = TIMER_TASK_STATE_INIT;  /* Set task state to Init */
            if (initially_enabled)
			{
				s_timer_tasks[i].state = TIMER_TASK_STATE_READY;  /* Set task state to Ready when enabled */
			}
			else
			{
				s_timer_tasks[i].state = TIMER_TASK_STATE_PENDING;  /* Set task state to Pending when disabled */
			}

            s_active_task_count++;  /* Increase active task count */
            return i;
        }
    }
    return 0xFF;
}

/******************************************************************************************
 * @brief  Delete a timer task by task ID
 * @param[in]  taskId: Task ID to delete
 * @return 1 indicates success, 0 indicates failure
 * @note   Returns 0 if task ID is invalid or task doesn't exist
 *****************************************************************************************/
uint08 timer_delete_task(uint08 taskId)
{
    if (taskId < MAX_TIMER_CALLBACKS && s_timer_tasks[taskId].callback != NULL)
    {
        s_timer_tasks[taskId].callback = NULL;  /* Clear callback function pointer */
        s_timer_tasks[taskId].state = TIMER_TASK_STATE_UNINIT;  /* Set task state to Uninit */
        s_active_task_count--;  /* Decrease active task count */
        return 1;
    }
    return 0;
}

/******************************************************************************************
 * @brief  Delete a timer task by callback function pointer
 * @param[in]  callback: Callback function pointer to delete
 * @return 1 indicates success, 0 indicates failure
 * @note   Returns 0 if callback is NULL or task not found
 *****************************************************************************************/
uint08 timer_delete_task_by_callback(TimerCallback_t callback)
{
    if (callback == NULL) return 0;

    for (uint08 i = 0; i < MAX_TIMER_CALLBACKS; i++)
    {
        if (s_timer_tasks[i].callback == callback)
        {
            s_timer_tasks[i].callback = NULL;
            s_timer_tasks[i].state = TIMER_TASK_STATE_UNINIT;  /* Set task state to Uninit */
            s_active_task_count--;
            return 1;
        }
    }

    return 0;
}

/******************************************************************************************
 * @brief  Enable/disable timer task
 * @param[in]  taskId: Task ID
 * @param[in]  enabled: 1=enable, 0=disable
 * @return None
 * @note   Does nothing if task ID is invalid
 *****************************************************************************************/
void timer_set_task_state(uint08 taskId, uint08 enabled)
{
    if (taskId < MAX_TIMER_CALLBACKS)
    {
        s_timer_tasks[taskId].enabled = enabled;
        if (enabled)
		{
			s_timer_tasks[taskId].state = TIMER_TASK_STATE_READY;  /* Set task state to Ready when enabled */
		}
		else
		{
			s_timer_tasks[taskId].state = TIMER_TASK_STATE_PENDING;  /* Set task state to Pending when disabled */
		}
    }
}

/******************************************************************************************
 * @brief  Enable/disable timer task by callback function pointer
 * @param[in]  callback: Callback function pointer
 * @param[in]  enabled: 1=enable, 0=disable
 * @return None
 * @note   Does nothing if callback is NULL
 *****************************************************************************************/
void timer_set_task_state_by_callback(TimerCallback_t callback, uint08 enabled)
{
    if (callback == NULL)
    {
        return;
    }
    for (uint08 i = 0; i < MAX_TIMER_CALLBACKS; i++)
    {
        if (s_timer_tasks[i].callback == callback)
        {
            s_timer_tasks[i].enabled = enabled;
            if (enabled)
			{
				s_timer_tasks[i].state = TIMER_TASK_STATE_READY;  /* Set task state to Ready when enabled */
			}
			else
			{
				s_timer_tasks[i].state = TIMER_TASK_STATE_PENDING;  /* Set task state to Pending when disabled */
			}
        }
    }
}

/******************************************************************************************
 * @brief  Get current active task count
 * @param  None
 * @return Current registered and not deleted task count
 * @note   Returns the number of active timer tasks
 *****************************************************************************************/
uint08 timer_get_active_task_count(void)
{
    return s_active_task_count;
}

/******************************************************************************************
 * @brief  Update task counter (internal use)
 * @param  None
 * @return None
 * @note   Recalculates the active task count
 *****************************************************************************************/
static void timer_update_task_counter(void)
{
    uint08 count = 0;
    for (uint08 i = 0; i < MAX_TIMER_CALLBACKS; i++)
    {
        if (s_timer_tasks[i].callback != NULL)
        {
            count++;
        }
    }
    s_active_task_count = count;
}
/******************************************************************************************
 * @brief  Immediately execute a timer task by task ID
 * @param[in]  taskId: Task ID to execute immediately
 * @return 1 indicates success, 0 indicates failure
 * @note   Returns 0 if task ID is invalid or task doesn't exist
 *****************************************************************************************/
uint08 timer_execute_task_immediately(uint08 taskId)
{
    if (taskId < MAX_TIMER_CALLBACKS &&
        s_timer_tasks[taskId].callback != NULL &&
        s_timer_tasks[taskId].enabled)
    {
        s_timer_tasks[taskId].state = TIMER_TASK_STATE_RUNNING;  /* Set task state to Running */
        s_timer_tasks[taskId].callback();
        s_timer_tasks[taskId].lastTick = s_system_ticks;  /* Reset last execution time */
        s_timer_tasks[taskId].delay = 0;  /* Clear any pending delay */
        s_timer_tasks[taskId].state = TIMER_TASK_STATE_READY;  /* Set task state back to Ready after execution */
        return 1;
    }
    return 0;
}

/******************************************************************************************
 * @brief  Immediately execute a timer task by callback function pointer
 * @param[in]  callback: Callback function pointer to execute immediately
 * @return 1 indicates success, 0 indicates failure
 * @note   Returns 0 if callback is NULL or task not found
 *****************************************************************************************/
uint08 timer_execute_task_immediately_by_callback(TimerCallback_t callback)
{
    if (callback == NULL) return 0;

    for (uint08 i = 0; i < MAX_TIMER_CALLBACKS; i++)
    {
        if (s_timer_tasks[i].callback == callback && s_timer_tasks[i].enabled)
        {
            s_timer_tasks[i].state = TIMER_TASK_STATE_RUNNING;  /* Set task state to Running */
            s_timer_tasks[i].callback();
            s_timer_tasks[i].lastTick = s_system_ticks;  /* Reset last execution time */
            s_timer_tasks[i].delay = 0;  /* Clear any pending delay */
            s_timer_tasks[i].state = TIMER_TASK_STATE_READY;  /* Set task state back to Ready after execution */
            return 1;
        }
    }

    return 0;
}
/******************************************************************************************
 * @brief  Get task status by task ID
 * @param[in]  taskId: Task ID
 * @return Task status (TIMER_TASK_STATE_UNINIT if task ID is invalid)
 * @note   Returns TIMER_TASK_STATE_UNINIT if task ID is invalid or task doesn't exist
 *****************************************************************************************/
TimerTaskState_t timer_get_task_status(uint08 taskId)
{
    if (taskId < MAX_TIMER_CALLBACKS && s_timer_tasks[taskId].callback != NULL)
    {
        return s_timer_tasks[taskId].state;
    }
    return TIMER_TASK_STATE_UNINIT;
}
/******************************************************************************************
 * @brief  Get task status by callback function pointer
 * @param[in]  callback: Callback function pointer
 * @return Task status (TIMER_TASK_STATE_UNINIT if callback is NULL or task not found)
 * @note   Returns TIMER_TASK_STATE_UNINIT if callback is NULL or task not found
 *****************************************************************************************/
TimerTaskState_t timer_get_task_status_by_callback(TimerCallback_t callback)
{
    if (callback == NULL) return TIMER_TASK_STATE_UNINIT;

    for (uint08 i = 0; i < MAX_TIMER_CALLBACKS; i++)
    {
        if (s_timer_tasks[i].callback == callback)
        {
            return s_timer_tasks[i].state;
        }
    }

    return TIMER_TASK_STATE_UNINIT;
}
/******************************************************************************************
 * @brief  Process timer tasks in main loop
 * @param  None
 * @return None
 * @note   Must be called periodically in main loop
 *****************************************************************************************/
void timer_process_tasks(void)
{
    for (uint08 i = 0; i < MAX_TIMER_CALLBACKS; i++)
    {
        if (s_timer_tasks[i].callback && s_timer_tasks[i].enabled)
        {
            uint32 elapsed = s_system_ticks - s_timer_tasks[i].lastTick;

            /* Handle initial delay */
            if (s_timer_tasks[i].delay > 0)
            {
                if (elapsed >= s_timer_tasks[i].delay)
                {
                    s_timer_tasks[i].lastTick = s_system_ticks;
                    s_timer_tasks[i].delay = 0;  /* Clear delay flag */
                    s_timer_tasks[i].state = TIMER_TASK_STATE_RUNNING;  /* Set task state to Running */
				    s_timer_tasks[i].callback();
				    s_timer_tasks[i].state = TIMER_TASK_STATE_READY;  /* Set task state back to Ready after execution */
                }else
                {
                    s_timer_tasks[i].state = TIMER_TASK_STATE_PENDING;  /* Set task state to Pending while waiting for delay */
                }
            }
            /* Normal periodic execution */
            else if (elapsed >= s_timer_tasks[i].interval)
            {
                s_timer_tasks[i].lastTick = s_system_ticks;
                s_timer_tasks[i].state = TIMER_TASK_STATE_RUNNING;  /* Set task state to Running */
				s_timer_tasks[i].callback();
				s_timer_tasks[i].state = TIMER_TASK_STATE_READY;  /* Set task state back to Ready after execution */
            }else
            {
                s_timer_tasks[i].state = TIMER_TASK_STATE_PENDING;  /* Set task state to Pending when waiting for interval */
            }
        }
    }

    #if(SOFT_TIMER_TEST_ENABLE == 1U)
    soft_timer_test();  /* Software timer test */
    #endif/*(SOFT_TIMER_TEST_ENABLE == 1U)*/
}




/******************************************************************************************
 * @brief  Get system tick count
 * @param  None
 * @return Current system tick count
 * @note   Returns the value of s_system_ticks
 *****************************************************************************************/
uint32 timer_get_ticks(void)
{
    return s_system_ticks;
}

/******************************************************************************************
 * @brief  Set software timer timeout and reset timing start point
 * @param[in]  t: Pointer to software timer structure
 * @param[in]  interval: Timeout interval
 * @return None
 * @note   Sets both start time and interval.Usually called at initialization time
 *****************************************************************************************/
void soft_timer_set(stSOFT_TIMER* t, uint32 interval)
{
    t->start = s_system_ticks;
    t->interval = interval;
}

/******************************************************************************************
 * @brief  Change software timer interval only
 * @param[in]  t: Pointer to software timer structure
 * @param[in]  interval: New timeout interval
 * @return None
 * @note   Only changes interval, suitable for dynamic adjustment
 *****************************************************************************************/
void soft_timer_change(stSOFT_TIMER* t, uint32 interval)
{
    t->interval = interval;
}

/******************************************************************************************
 * @brief  Check if software timer has timed out
 * @param[in]  t: Pointer to software timer structure
 * @return DEF_TRUE if timeout, DEF_FALSE otherwise
 * @note   Returns TRUE continuously after timeout, call soft_timer_reset() for periodic use
 *****************************************************************************************/
typ_bool is_soft_timer_timeout(stSOFT_TIMER* t)
{
    uint32 current = s_system_ticks;
    uint32 elapsed;

    /* Handle counter overflow situation */
    if (current >= t->start)  /* No overflow case */
    {
        elapsed = current - t->start;  /* Calculate elapsed time */

    } else
    {
        /* Overflow occurred: current < t->start */
        elapsed = (UINT32_MAX_VALUE - t->start) + current + 1;  /* Calculate elapsed time */
    }
     if(elapsed >= t->interval)
       return DEF_TRUE;
    
      return DEF_FALSE;
//    return (elapsed >= t->interval) ? DEF_TRUE : DEF_FALSE;  /* Return TRUE if elapsed time >= set time, else FALSE */
}

/******************************************************************************************
 * @brief  Reset software timer to start timing again
 * @param[in]  t: Pointer to software timer structure
 * @return None
 * @note   Resets the start time to current system ticks
 *****************************************************************************************/
void soft_timer_reset(stSOFT_TIMER* t)
{
    t->start = s_system_ticks;
}

/******************************************************************************************
 * @brief  Set timer with first delay and subsequent cycle
 * @param[in]  t: Pointer to extended software timer structure
 * @param[in]  first_interval: First execution delay
 * @param[in]  cycle_interval: Subsequent cycle interval
 * @return None
 * @note   Suitable for CAN message staggered sending scenarios
 *****************************************************************************************/
void soft_timer_set_ex(stSOFT_TIMER_EX* t, uint32 first_interval, uint32 cycle_interval)
{
    t->start = s_system_ticks;
    t->first_interval = first_interval;
    t->cycle_interval = cycle_interval;
    t->is_first_time = DEF_TRUE;  /* Mark as first detection */
}

/******************************************************************************************
 * @brief  Check if extended software timer has timed out
 * @param[in]  t: Pointer to extended software timer structure
 * @return DEF_TRUE if timeout, DEF_FALSE otherwise
 * @note   Supports first delay and cycle execution, suitable for power-on CAN staggered sending
 *****************************************************************************************/
typ_bool is_soft_timer_timeout_ex(stSOFT_TIMER_EX* t)
{
    uint32 current = s_system_ticks;
    uint32 elapsed;
    uint32 target_interval;

    /* Determine current interval time */
    if (t->is_first_time)
    {
        target_interval = t->first_interval;
    } else {
        target_interval = t->cycle_interval;
    }

    /* Handle counter overflow situation */
    if (current >= t->start)
    {
        elapsed = current - t->start;  /* No overflow */
    } else
    {
        /* Overflow occurred */
        elapsed = (UINT32_MAX_VALUE - t->start) + current + 1;
    }

    /* Check if timeout */
    if (elapsed >= target_interval)
    {
        if (t->is_first_time)
        {
            t->is_first_time = DEF_FALSE;  /* After first timeout, mark as not first */
        }
        t->start = s_system_ticks;  /* Reset timing start point, enables cyclic execution */
        return DEF_TRUE;
    }

    return DEF_FALSE;
}

/******************************************************************************************
 * @brief  Reset extended timer to first detection state
 * @param[in]  t: Pointer to extended software timer structure
 * @return None
 * @note   Resets both start time and first time flag
 *****************************************************************************************/
void soft_timer_reset_ex(stSOFT_TIMER_EX* t)
{
    t->start = s_system_ticks;
    t->is_first_time = DEF_TRUE;  /* Reset to first detection state */
}










