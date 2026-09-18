/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : can_busoff.c
* Author        : yangming
* Date          : 2024-05-27
* Version       : 1.00
* Description   : CAN busoff recovery mechanism.
* Others        : None
*
*******************************************************************************************************/
#include "can_busoff.h"

#define CAN_BUSOFF_RECOVER_WAIT_TIME            5U
static const stBUSOFF_CFG s_busoff_cfg[CAN_CHN_NUM] =
{
    /* CAN_CHN_0 */
    {
        100, /* fast_period : 1ms */
        200, /* slow_period : 1ms */
        5, /* fast_times */
        0 /* slow_times : recovery times is 0, unlimited */
    },
    /* CAN_CHN_1 */
    {
        100, /* fast_period : 1ms */
        200, /* slow_period : 1ms */
        5, /* fast_times */
        0 /* slow_times : recovery times is 0, unlimited */
    },
    /* CAN_CHN_2 */
    {
        100, /* fast_period : 1ms */
        200, /* slow_period : 1ms */
        5, /* fast_times */
        0 /* slow_times : recovery times is 0, unlimited */
    }
};

static stBUSOFF_INFO s_busoff_info[CAN_CHN_NUM];

void can_busoff_init(void)
{
    common_memset((uint08*)s_busoff_info, 0U, sizeof(s_busoff_info));
    
}

void can_busoff_process(void)
{
    eBUSOFF_STS sts[CAN_CHN_NUM];
    uint08 msg[8] = {0};
    uint08 chn = 0;

    for (chn = CAN_CHN_0; chn < CAN_CHN_NUM; chn++)
    {
        sts[chn] = s_busoff_info[chn].sts;

        switch (sts[chn])
        {
            case BUSOFF_STS_NORMAL:
                if (1 == s_busoff_info[chn].busoff_flag)
                {
                    CAN_BUSOFF_CAN_STOP(chn);
                    sts[chn] = BUSOFF_STS_PENDING;
                    s_busoff_info[chn].fast_time_cnt = s_busoff_cfg[chn].fast_times;
                    s_busoff_info[chn].slow_time_cnt = s_busoff_cfg[chn].slow_times;
                }
                else {}
                break;
            case BUSOFF_STS_RECOVER:
                s_busoff_info[chn].busoff_recover_flg = 1;
                sts[chn] = BUSOFF_STS_RECOVER_WAIT;
                break;
            case BUSOFF_STS_RECOVER_WAIT:
                if (2 == s_busoff_info[chn].busoff_recover_flg)
                {
                    s_busoff_info[chn].busoff_recover_flg = 0;
                }
                
                if (0 == s_busoff_info[chn].busoff_flag) /* can communication is normal */
                {
                    sts[chn] = BUSOFF_STS_NORMAL;
                }
                else if (1 == s_busoff_info[chn].busoff_flag)
                {
                    CAN_BUSOFF_CAN_STOP(chn);
                    sts[chn] = BUSOFF_STS_PENDING;
                    if (s_busoff_info[chn].fast_time_cnt > 0)
                    {
                        s_busoff_info[chn].fast_period_cnt = s_busoff_cfg[chn].fast_period;
                    }
                    else if (s_busoff_info[chn].slow_time_cnt > 0)
                    {
                        s_busoff_info[chn].slow_period_cnt = s_busoff_cfg[chn].slow_period;
                    }
                    else
                    {
                        s_busoff_info[chn].slow_period_cnt = s_busoff_cfg[chn].slow_period;
                    }
                }
                else
                {
                }
                break;
            case BUSOFF_STS_PENDING:
                if (s_busoff_info[chn].fast_time_cnt > 0)
                {
                    s_busoff_info[chn].fast_period_cnt = s_busoff_cfg[chn].fast_period;
                }
                else if (s_busoff_info[chn].slow_time_cnt > 0)
                {
                    s_busoff_info[chn].slow_period_cnt = s_busoff_cfg[chn].slow_period;
                }
                else
                {
                    s_busoff_info[chn].slow_period_cnt = s_busoff_cfg[chn].slow_period;
                }
                sts[chn] = BUSOFF_STS_SW;
                break;
            case BUSOFF_STS_SW:
                if (s_busoff_info[chn].fast_time_cnt > 0)
                {
                    sts[chn] = BUSOFF_STS_FAST;
                }
                else if (s_busoff_info[chn].slow_time_cnt > 0)
                {
                    sts[chn] = BUSOFF_STS_SLOW;
                }
                else
                {
                    sts[chn] = BUSOFF_STS_SLOW; /* Stay in this status */
                } 
                break;
            case BUSOFF_STS_FAST:
                if (0 == s_busoff_info[chn].fast_period_cnt)
                {
                    CAN_BUSOFF_CAN_REINIT(chn);
                    s_busoff_info[chn].busoff_flag = 2;
                    if (s_busoff_info[chn].fast_time_cnt)
                    {
                        s_busoff_info[chn].fast_time_cnt--;
                    }
                    else {}
                    sts[chn] = BUSOFF_STS_RECOVER;
                }
                else
                {
                    /* wait */
                }
                break;
            case BUSOFF_STS_SLOW:
                if (0 == s_busoff_info[chn].slow_period_cnt)
                {
                    CAN_BUSOFF_CAN_REINIT(chn);
                    s_busoff_info[chn].busoff_flag = 2;
                    if (s_busoff_info[chn].slow_time_cnt)
                    {
                        s_busoff_info[chn].slow_time_cnt--;
                    }
                    else {}
                    sts[chn] = BUSOFF_STS_RECOVER;
                }
                else
                {
                    /* wait */
                }
                break;
            default:
                sts[chn] = BUSOFF_STS_NORMAL;
                break;
        }
        
        s_busoff_info[chn].sts = sts[chn];
    }
}

void can_busoff_occur_cbk(eCAN_CHN chn)
{
    s_busoff_info[chn].busoff_flag = 1;
}

void can_busoff_recovered_cbk(eCAN_CHN chn)
{
    s_busoff_info[chn].busoff_flag = 0;
}

void can_busoff_1ms_period_cbk(eCAN_CHN chn)
{
    if (s_busoff_info[chn].fast_period_cnt)
    {
        s_busoff_info[chn].fast_period_cnt--;
    }
    else {}

    if (s_busoff_info[chn].slow_period_cnt)
    {
        s_busoff_info[chn].slow_period_cnt--;
    }
    else {}

//    can_busoff_process();
}

void can_busoff_recover_flg_set(eCAN_CHN chn, uint08 val)
{
    s_busoff_info[chn].busoff_recover_flg = val;
}

uint08 can_busoff_recover_flg_get(eCAN_CHN chn)
{
    return s_busoff_info[chn].busoff_recover_flg;
}

eBUSOFF_STS can_busoff_sts_get(eCAN_CHN chn)
{
    return s_busoff_info[chn].sts;
}

