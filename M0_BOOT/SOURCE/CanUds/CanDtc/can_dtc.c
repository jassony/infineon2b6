/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_dtc.c
* Author        : yangming
* Date          : 2024-04-28
* Version       : 1.00
* Description   : Can dtc.
* Others        : None
*
****************************************************************************************************/
#include "can_dtc.h"
#include "eeprom_app.h"
#include "common_mem_op.h"

static uint08 s_dtc_operation_cycle_sw;
static uint08 s_dtc_operation_cycle_sts;
stDTC_SNAPSHOT g_dtc_snapshot[CAN_DTC_NUM]; /* write to eeprom */
stDTC_SNAPSHOT g_dtc_local_snapshot[CAN_DTC_NUM]; /* write to eeprom */
uint08 g_dtc_sts[CAN_DTC_NUM]; /* write to eeprom */
static uint08 s_dtc_operation_cycle[CAN_DTC_NUM];
uint08 g_dtc_pending_clr[CAN_DTC_NUM]; /* write to eeprom */
static eDTC_APPEAR_STS s_dtc_appear_this_oc_sts[CAN_DTC_NUM]; /* this operation cycle appear the DTC */
static typ_bool s_dtc_clr_flg[CAN_DTC_NUM];
static uint08 s_dtc_wr_flg;
static uint08 s_dtc_wr_wait_cnt;

static void dtc_data_read_from_nvm(void);
static void dtc_data_store_to_nvm(void);
static void dtc_sts_chk(void);
static void dtc_operation_cycle_update(void);
static void dtc_operation_cycle_add(void);
static void status_test_failed(eDTC_TYP typ);
static void status_test_failed_this_operation_cycle(eDTC_TYP typ);
static void status_pending_dtc(eDTC_TYP typ);
static void status_confirmed_dtc(eDTC_TYP typ);
static void dtc_snapshot_process(void);

void can_dtc_init(void)
{
    common_memset((uint08*)g_dtc_sts, 0U, sizeof(g_dtc_sts));
    common_memset((uint08*)s_dtc_appear_this_oc_sts, 0U, sizeof(s_dtc_appear_this_oc_sts));
    common_memset((uint08*)s_dtc_clr_flg, DEF_FALSE, sizeof(s_dtc_clr_flg));
    s_dtc_operation_cycle_sw = DTC_OPERATION_CYCLE_ON;
    s_dtc_operation_cycle_sts = DTC_OPERATION_CYCLE_OFF;
    s_dtc_wr_flg = 0;
    s_dtc_wr_wait_cnt = 0;
}

void can_dtc_process(void)
{
    dtc_data_read_from_nvm();
    dtc_sts_chk();
    dtc_operation_cycle_add();
    dtc_data_store_to_nvm();
    dtc_snapshot_process();
    dtc_operation_cycle_update();
}

uint08 dtc_operation_cycle_switch_get(uint08 sw)
{
    return s_dtc_operation_cycle_sw;
}

void dtc_operation_cycle_switch_set(uint08 sw)
{
    s_dtc_operation_cycle_sw = sw;
}

void dtc_clear(eDTC_TYP typ)
{
    s_dtc_clr_flg[typ] = DEF_TRUE;
}

void dtc_all_clear(void)
{
    eDTC_TYP typ = (eDTC_TYP)0;
    
    for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
    {
        s_dtc_clr_flg[typ] = DEF_TRUE;
    }
}

void dtc_cur_snapshot_get(eDTC_TYP typ, stDTC_SNAPSHOT shnapshot)
{
    if (typ >= CAN_DTC_NUM)
    {
        return;
    }
    
    if (   (DTC_APPEAR_SNAPSHOT_FIRST == s_dtc_appear_this_oc_sts[typ])
        && ((1 << DTC_STS_TESTFAILED) == (g_dtc_sts[typ] & (1 << DTC_STS_TESTFAILED)))
        )
    {
        shnapshot.record_number = DTC_SNAPSHOT_GLOBAL_RECORD_NUMBER;
        common_memcpy((uint08*)&g_dtc_snapshot[typ], (uint08*)&shnapshot, sizeof(stDTC_SNAPSHOT));
        s_dtc_appear_this_oc_sts[typ] = DTC_APPEAR_SNAPSHOT_OVER;
    }
    else if (  (DTC_APPEAR_SNAPSHOT_LAST == s_dtc_appear_this_oc_sts[typ])
            && ((1 << DTC_STS_TESTFAILED) == (g_dtc_sts[typ] & (1 << DTC_STS_TESTFAILED)))
            )
    {
        shnapshot.record_number = DTC_SNAPSHOT_LOCAL_RECORD_NUMBER;
        common_memcpy((uint08*)&g_dtc_local_snapshot[typ], (uint08*)&shnapshot, sizeof(stDTC_SNAPSHOT));
        s_dtc_appear_this_oc_sts[typ] = DTC_APPEAR_SNAPSHOT_OVER;
    }
    else
    {}
}

typ_bool is_dtc_snapshot_recorded(eDTC_TYP typ, uint08 record_number)
{
    if (   (DTC_SNAPSHOT_GLOBAL_RECORD_NUMBER == record_number)
        && (DTC_SNAPSHOT_GLOBAL_RECORD_NUMBER == g_dtc_snapshot[typ].record_number)
        )
    {
        return DEF_TRUE;
    }
    else if (   (DTC_SNAPSHOT_LOCAL_RECORD_NUMBER == record_number)
             && (DTC_SNAPSHOT_LOCAL_RECORD_NUMBER == g_dtc_local_snapshot[typ].record_number)
             )
    {
        return DEF_TRUE;
    }

    return DEF_FALSE;
}

void dtc_shapshot_global_data_get(eDTC_TYP typ, stDTC_SNAPSHOT* snapshot_data)
{
    common_memcpy((uint08*)snapshot_data, (uint08*)&g_dtc_snapshot[typ], sizeof(stDTC_SNAPSHOT));    
}


void dtc_shapshot_local_data_get(eDTC_TYP typ, stDTC_SNAPSHOT* snapshot_data)
{
    common_memcpy((uint08*)snapshot_data, (uint08*)&g_dtc_local_snapshot[typ], sizeof(stDTC_SNAPSHOT));  
}

uint08 dtc_sts_get(eDTC_TYP typ)
{
    return (g_dtc_sts[typ] & DTC_SUPPORTED_STS);
}

uint16 dtc_number_get(void)
{
    eDTC_TYP typ = (eDTC_TYP)0;
    uint16 num = 0;
    
    for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
    {
        if (0 != (g_dtc_sts[typ] & (1 << DTC_STS_CONFIRMEDDTC)))
        {
            num++;
        }
        else
        {}
    }

    return num;
}

uint16 dtc_number_get_by_sts(uint08 sts)
{
    eDTC_TYP typ = (eDTC_TYP)0;
    uint16 num = 0;
    
    if (sts > DTC_SUPPORTED_STS)
    {
        sts = DTC_SUPPORTED_STS;
    }
    else {}

    for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
    {
        if (0 != (g_dtc_sts[typ] & sts))
        {
            num++;
        }
        else
        {}
    }

    return num;
}

static void dtc_data_read_from_nvm(void)
{
    eDTC_TYP typ = (eDTC_TYP)0;
    
    if (   (DTC_OPERATION_CYCLE_ON == s_dtc_operation_cycle_sw)
        && (DTC_OPERATION_CYCLE_OFF == s_dtc_operation_cycle_sts)
        )
    {
        /* read data form eeprom */
//        eeprom_app_item_read(EE_RD_ITEM_DTC_STATUS);
//        eeprom_app_item_read(EE_RD_ITEM_DTC_PENDING);
//        eeprom_app_item_read(EE_RD_ITEM_DTC_SNAPSHOT);
//        eeprom_app_item_read(EE_RD_ITEM_DTC_LOCAL_SNAPSHOT);
//        SEGGER_RTT_printf(0, "eeprom read dtc to ram.\r\n");
        
        for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
        {
            if (0xFF == g_dtc_sts[typ])
            {
                g_dtc_sts[typ] = 0;
            }
            else {}

            if (0xFF == g_dtc_pending_clr[typ])
            {
                g_dtc_pending_clr[typ] = 0;
            }
            else {}
        }
    }
    else
    {}
}

static void dtc_data_store_to_nvm(void)
{
    eEE_CTRL_ERR err1 = EE_CTRL_ERR_NONE;
    eEE_CTRL_ERR err2 = EE_CTRL_ERR_NONE;
    eEE_CTRL_ERR err3 = EE_CTRL_ERR_NONE;
    eDTC_TYP typ = (eDTC_TYP)0;
    uint08 dtc_clr = 0;
    
    for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
    {
        if (DEF_TRUE == s_dtc_clr_flg[typ])
        {
            dtc_clr = 1;
            break;
        }
        else {}
    }
    
    if (   (DTC_OPERATION_CYCLE_OFF == s_dtc_operation_cycle_sw)
        && (DTC_OPERATION_CYCLE_ON == s_dtc_operation_cycle_sts)
        )
    {
        /* write data to eeprom */
        s_dtc_wr_flg = 0x0F;
        s_dtc_wr_wait_cnt = 5;
    }
    else if (dtc_clr)
    {
        s_dtc_wr_flg = 0x0F;
        s_dtc_wr_wait_cnt = 0;
    }
    else 
    {}

    if ((s_dtc_wr_flg) && (0 == s_dtc_wr_wait_cnt))
    {
        if (s_dtc_wr_flg & (1 << 0))
        {
            err1 = eeprom_app_wr_err_get(EE_WR_ITEM_DTC_STATUS);
            if ((EE_CTRL_ERR_NONE == err1) || (EE_CTRL_ERR_WR_SUCCESS == err1) || (EE_CTRL_ERR_BUF_OVF == err1))
            {
                eeprom_app_buf_write(EE_WR_ITEM_DTC_STATUS,
                                     EE_DTC_STATUS_ADDR,
                                     EE_DTC_STATUS_BYTES * CAN_DTC_NUM,
                                     (uint08*)g_dtc_sts);
                s_dtc_wr_flg &= ~(1 << 0);
            }
            else {}
        }
        else if (s_dtc_wr_flg & (1 << 1))
        {
            err1 = eeprom_app_wr_err_get(EE_WR_ITEM_DTC_PENDING);
            if ((EE_CTRL_ERR_NONE == err1) || (EE_CTRL_ERR_WR_SUCCESS == err1) || (EE_CTRL_ERR_BUF_OVF == err1))
            {
                eeprom_app_buf_write(EE_WR_ITEM_DTC_PENDING,
                                     EE_DTC_PENDING_CLR_FLG_ADDR,
                                     EE_DTC_PENDING_CLR_BYTES * CAN_DTC_NUM,
                                     (uint08*)g_dtc_pending_clr);
                s_dtc_wr_flg &= ~(1 << 1);
            }
            else {}
        }
        else if (s_dtc_wr_flg & (1 << 2))
        {
            err2 = eeprom_app_wr_err_get(EE_WR_ITEM_DTC_SNAPSHOT);
            if ((EE_CTRL_ERR_NONE == err2) || (EE_CTRL_ERR_WR_SUCCESS == err2) || (EE_CTRL_ERR_BUF_OVF == err2))
            {
                eeprom_app_buf_write(EE_WR_ITEM_DTC_SNAPSHOT,
                                     EE_DTC_SNAPSHOT_ADDR,
                                     EE_DTC_SNAPSHOT_BYTES * CAN_DTC_NUM,
                                     (uint08*)g_dtc_snapshot);
                s_dtc_wr_flg &= ~(1 << 2);
            }
            else {}
        }
        else if (s_dtc_wr_flg & (1 << 3))
        {
            err3 = eeprom_app_wr_err_get(EE_WR_ITEM_DTC_LOCAL_SNAPSHOT);
            if ((EE_CTRL_ERR_NONE == err3) || (EE_CTRL_ERR_WR_SUCCESS == err3) || (EE_CTRL_ERR_BUF_OVF == err3))
            {
                eeprom_app_buf_write(EE_WR_ITEM_DTC_LOCAL_SNAPSHOT,
                                     EE_DTC_LOCAL_SNAPSHOT_ADDR,
                                     EE_DTC_SNAPSHOT_BYTES * CAN_DTC_NUM,
                                     (uint08*)g_dtc_local_snapshot);
                s_dtc_wr_flg &= ~(1 << 3);
            }
            else {}
        }
        else
        {
            s_dtc_wr_flg = 0;
        }
    }
    else {}
    if (s_dtc_wr_wait_cnt)
    {
        s_dtc_wr_wait_cnt--;
    }
    else {}
}

static void dtc_sts_chk(void)
{
    eDTC_TYP typ = (eDTC_TYP)0;

    for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
    {
        status_test_failed(typ);
        status_test_failed_this_operation_cycle(typ);
        status_pending_dtc(typ);
        status_confirmed_dtc(typ);
    }
}

static void dtc_operation_cycle_update(void)
{  
    s_dtc_operation_cycle_sts = s_dtc_operation_cycle_sw;
}

static void dtc_operation_cycle_add(void)
{
    eDTC_TYP typ = (eDTC_TYP)0;

    if (   (DTC_OPERATION_CYCLE_ON == s_dtc_operation_cycle_sw)
        && (DTC_OPERATION_CYCLE_OFF == s_dtc_operation_cycle_sts)
        )
    {
        for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
        {
            if (g_dtc_sts[typ] & (1 << DTC_STS_PENDINGDTC))
            {
                s_dtc_operation_cycle[typ]++;
            }
            else {}
        }
    }
    else {}
}

static void status_test_failed(eDTC_TYP typ)
{
    stDTC_CFG* cfg = (stDTC_CFG*)g_can_dtc_cfg;

    if (   (cfg[typ].fault_func != DEF_NULL)
        && (DTC_OPERATION_CYCLE_ON == s_dtc_operation_cycle_sts)
        )
    {
        if (DTC_FAULT_APPEAR == cfg[typ].fault_func())
        {
            g_dtc_sts[typ] |= (1 << DTC_STS_TESTFAILED);
        }
        else
        {
            g_dtc_sts[typ] &= ~(1 << DTC_STS_TESTFAILED);
        }
    }
    else
    {}
}

static void status_test_failed_this_operation_cycle(eDTC_TYP typ)
{
    if (   (0 == (g_dtc_sts[typ] & (1 << DTC_STS_TESTFAILEDTHISOPERATIONCYCLE)))
        && (DTC_OPERATION_CYCLE_ON == s_dtc_operation_cycle_sts)
        )
    {
        if (0 != (g_dtc_sts[typ] & (1 << DTC_STS_TESTFAILED)))
        {
            g_dtc_sts[typ] |= (1 << DTC_STS_TESTFAILEDTHISOPERATIONCYCLE);
        }
        else {}
    }
    else {}
}

static void status_pending_dtc(eDTC_TYP typ)
{
    if (DTC_OPERATION_CYCLE_ON == s_dtc_operation_cycle_sts)
    {
        if (DEF_TRUE == s_dtc_clr_flg[typ])
        {
            g_dtc_sts[typ] &= ~(1 << DTC_STS_PENDINGDTC);
            g_dtc_pending_clr[typ] = 0;
        }
        else {}
    }
    else
    {
        if (0 != (g_dtc_sts[typ] & (1 << DTC_STS_TESTFAILEDTHISOPERATIONCYCLE)))
        {
            g_dtc_sts[typ] |= (1 << DTC_STS_PENDINGDTC);
            g_dtc_pending_clr[typ] = 0;
        }
        else
        {
            if (g_dtc_pending_clr[typ])
            {
                g_dtc_sts[typ] &= ~(1 << DTC_STS_PENDINGDTC);
            }
            else
            {
                g_dtc_pending_clr[typ] = 1;
            }
        }
    }
}

static void status_confirmed_dtc(eDTC_TYP typ)
{
    stDTC_CFG* cfg = (stDTC_CFG*)g_can_dtc_cfg;

    if (DTC_OPERATION_CYCLE_OFF == s_dtc_operation_cycle_sts)
    {
        return;
    }
    
    if (0 != (g_dtc_sts[typ] & (1 << DTC_STS_CONFIRMEDDTC)))
    {
        if (0 != (g_dtc_sts[typ] & (1 << DTC_STS_TESTFAILED)))
        {
            g_dtc_sts[typ] |= (1 << DTC_STS_CONFIRMEDDTC);
            s_dtc_appear_this_oc_sts[typ] = DTC_APPEAR_SNAPSHOT_LAST;
        }
        else
        {
            if (DEF_TRUE == s_dtc_clr_flg[typ])
            {
                g_dtc_sts[typ] &= ~(1 << DTC_STS_CONFIRMEDDTC);
            }
            else {}
        }
    }
    else 
    {
        if (   (0 != (g_dtc_sts[typ] & (1 << DTC_STS_TESTFAILED)))
            && (0 != (g_dtc_sts[typ] & (1 << DTC_STS_PENDINGDTC)))
            && (s_dtc_operation_cycle[typ] >= cfg[typ].confirmed_cycle_num)
            && (DEF_FALSE == s_dtc_clr_flg[typ])
            )
        {

            g_dtc_sts[typ] |= (1 << DTC_STS_CONFIRMEDDTC);
            s_dtc_appear_this_oc_sts[typ] = DTC_APPEAR_SNAPSHOT_FIRST;
        }
        else
        {}
    }
}

static void dtc_snapshot_process(void)
{
    eDTC_TYP typ = (eDTC_TYP)0;
    
    for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
    {
        if (DEF_TRUE == s_dtc_clr_flg[typ])
        {
            s_dtc_clr_flg[typ] = DEF_FALSE;
            g_dtc_sts[typ] = 0;
            g_dtc_pending_clr[typ] = 0;
            common_memset((uint08*)&g_dtc_snapshot[typ], 0U, sizeof(g_dtc_snapshot));
            g_dtc_snapshot[typ].record_number = DTC_SNAPSHOT_INVALID_RECORD_NUMBER;
            common_memset((uint08*)&g_dtc_local_snapshot[typ], 0U, sizeof(g_dtc_local_snapshot));
        }
        else {}
    }
}

