/*
 * m0_task.h
 *
 *  Created on: 2026Äê1ÔÂ22ÈÕ
 *      Author: hzldy
 */

#ifndef M0_BSW_SOURCE_LD_TASK_INC_M0_TASK_H_
#define M0_BSW_SOURCE_LD_TASK_INC_M0_TASK_H_

#include "cy_project.h"
#include "m0_adc_cfg.h"
#include "m0_canfd_cfg.h"
#include "m0_irq_cfg.h"
#include "m0_tcpwm_cfg.h"
#include "m0_gpio_cfg.h"
#include "m0_sysclk_cfg.h"
#include "m0_m4_ipc.h"
#include "m0_rte.h"
#include "platform_common_typdef.h"
#include "m0_var.h"
#include "m0_flash.h"
#include "can_busoff.h"
#include "user_funtion.h"
#include "m0_wdg.h"

#define TASK_DISABLE_IRQ(state)  state = Cy_SysLib_EnterCriticalSection();
#define TASK_ENABLE_IRQ(state)  Cy_SysLib_ExitCriticalSection(state);
#define TASK_IDLE_PRINT_TIME_MS                     5000U
#define TASK_can_USER_INDEX                         4U

typedef void (*task_func)(void);
typedef enum _task_sts                              eTASK_STS;
typedef struct _task_cfg                            stTASK_CFG;
typedef struct _task_info                           stTASK_INFO;

enum _task_typ
{
    TASK_1 = 0,
    TASK_2,
    TASK_3,
};

enum _task_sts
{
    TASK_STS_INIT = 0,
    TASK_STS_PENDING,
    TASK_STS_READY,
    TASK_STS_RUNNING,

    TASK_STS_MAX
};

struct _task_cfg
{
    uint32_t                  time_init_cnt; /* The initial value of the task time, which can increase the time interval for the same period of the task */
    uint32_t                  time_period; /* Task period value */
    task_func               func; /* Task execution function */
};

struct _task_info
{
    uint32_t                  time_last_cnt;
    eTASK_STS               sts;                  
};

extern void task_init(void);
extern void task_deinit(void);
extern void task_process(void);
extern void task_isr(void);
extern uint08 task_num_get(void);

#endif /* M0_BSW_SOURCE_LD_TASK_INC_M0_TASK_H_ */
