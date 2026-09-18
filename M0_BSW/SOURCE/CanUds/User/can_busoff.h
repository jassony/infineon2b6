/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : can_busoff.h
* Author        : yangming
* Date          : 2024-05-27
* Version       : 1.00
* Description   : Header file: CAN busoff recovery mechanism.
* Others        : None
*
*******************************************************************************************************/
#ifndef _CAN_BUSOFF_H
#define _CAN_BUSOFF_H

#include "can_driver.h"

//extern void EcanInit( void );
//extern void can_busoff_recover_test_send(void);
//extern void can_tx_abort(unsigned char msgbuf_id);

#define CAN_BUSOFF_CAN_REINIT(chn)      Canfd_Init() /* Get the can re-initialize inferface */
#define CAN_BUSOFF_RECOVER_TEST()       //PCan_ASWSend_TMC_SysSt1_MainFunction()//can_busoff_recover_test_send()
#define CAN_BUSOFF_CAN_STOP(chn)        Canfd_Deinit()
       

#ifndef _PLATFORM_COMMON_TYPDEF_H
typedef signed char                     sint08;
typedef signed short int                sint16;
typedef signed long int                 sint32;
typedef signed long long int            sint64;
typedef unsigned char                   uint08;
typedef unsigned short int              uint16;
typedef unsigned long int               uint32;
typedef unsigned long long int          uint64;
typedef uint08                          typ_bool;
#endif

typedef enum _busoff_sts                eBUSOFF_STS;
typedef struct _busoff_cfg              stBUSOFF_CFG;
typedef struct _busoff_info             stBUSOFF_INFO;

enum _busoff_sts
{
    BUSOFF_STS_NORMAL = 0,
    BUSOFF_STS_RECOVER,
    BUSOFF_STS_PENDING,
    BUSOFF_STS_RECOVER_WAIT,
    BUSOFF_STS_SW,
    BUSOFF_STS_FAST,
    BUSOFF_STS_SLOW,

    BUSOFF_STS_MAX
};

struct _busoff_cfg
{
    uint16                              fast_period;
    uint16                              slow_period;
    uint08                              fast_times;
    uint08                              slow_times;
};

struct _busoff_info
{
    uint16                              fast_period_cnt;
    uint16                              slow_period_cnt;
    eBUSOFF_STS                         sts;
    uint08                              busoff_flag;
    uint08                              fast_time_cnt;
    uint08                              slow_time_cnt;
    uint08                              busoff_recover_flg;

};

extern void can_busoff_init(void);
extern void can_busoff_process(void);
extern void can_busoff_occur_cbk(eCAN_CHN chn);
extern void can_busoff_recovered_cbk(eCAN_CHN chn);
extern eBUSOFF_STS can_busoff_sts_get(eCAN_CHN chn);
extern void can_busoff_1ms_period_cbk(eCAN_CHN chn);
#endif /* _CAN_BUSOFF_H */

