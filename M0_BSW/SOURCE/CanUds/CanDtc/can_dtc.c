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
#include "m0_var.h"

/* The source Power module fixes this signal low; keep that behavior locally. */
#define IS_KL15_OFF 0

stDTC_SNAPSHOT g_dtc_snapshot[CAN_DTC_NUM]; /* write to eeprom */
stDTC_SNAPSHOT g_dtc_local_snapshot[CAN_DTC_NUM]; /* write to eeprom */
uint08 g_dtc_confirmed_sts[CAN_DTC_NUM]; /* write to eeprom */
stDTC_EXTENDED  g_dtc_extended[CAN_DTC_NUM]; /* write to eeprom */

static uint08 s_dtc_operation_cycle_sw;
static uint08 s_dtc_operation_cycle_sts;
static uint08 s_dtc_sts[CAN_DTC_NUM];
static eDTC_APPEAR_STS s_dtc_appear_this_oc_sts[CAN_DTC_NUM]; /* this operation cycle appear the DTC */
static typ_bool s_dtc_clr_flg[CAN_DTC_NUM];
static uint08 s_dtc_wr_flg[CAN_DTC_NUM];
static uint08 s_dtc_wr_wait_cnt;
static uint08 s_dtc_fault_cleared_flg[CAN_DTC_NUM];
static uint32 s_dtc_fault_clr_cnt[CAN_DTC_NUM];
static uint08 s_dtc_oc_fault_cnt[CAN_DTC_NUM];
static uint08 s_dtc_ext_fault_occ_flg[CAN_DTC_NUM];

static uint08 s_dtc_ignon_condition;

static void dtc_data_read_from_nvm(void);
static void dtc_data_store_to_nvm(void);
static void dtc_fault_handle(void);
static void dtc_fault_once_handle(eDTC_TYP typ);
static void dtc_operation_cycle_update(void);
static void status_test_failed(eDTC_TYP typ, uint08 sts);
static void status_test_failed_this_operation_cycle(eDTC_TYP typ, uint08 sts);
static void status_pending_dtc(eDTC_TYP typ, uint08 sts);
static void status_confirmed_dtc(eDTC_TYP typ, uint08 sts);
static void status_test_not_completed_since_last_clear(eDTC_TYP typ, uint08 sts);
static void status_test_failed_since_last_clear(eDTC_TYP typ, uint08 fault_sts);
static void status_test_not_completed_this_operation_cycle(eDTC_TYP typ, uint08 sts);
static void dtc_clear_process(eDTC_TYP typ);
static void dtc_extend_process(eDTC_TYP typ);

void can_dtc_init(void)
{
    eDTC_TYP typ = (eDTC_TYP)0;
    
    common_memset((uint08*)s_dtc_sts, 0U, sizeof(s_dtc_sts));
    for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
    {
        status_test_not_completed_since_last_clear(typ, 1);
        status_test_not_completed_this_operation_cycle(typ, 1);
        s_dtc_fault_clr_cnt[typ] = DTC_FAULT_CLR_LAST_TIME;
    }
    common_memset((uint08*)s_dtc_appear_this_oc_sts, 0U, sizeof(s_dtc_appear_this_oc_sts));
    common_memset((uint08*)s_dtc_clr_flg, DEF_FALSE, sizeof(s_dtc_clr_flg));
    common_memset((uint08*)s_dtc_fault_cleared_flg, 1U, sizeof(s_dtc_fault_cleared_flg));
    common_memset((uint08*)s_dtc_oc_fault_cnt, 1U, sizeof(s_dtc_oc_fault_cnt));
    common_memset((uint08*)s_dtc_ext_fault_occ_flg, 1U, sizeof(s_dtc_ext_fault_occ_flg));
    common_memset((uint08*)s_dtc_wr_flg, DEF_FALSE, sizeof(s_dtc_wr_flg));
    s_dtc_operation_cycle_sw = DTC_OPERATION_CYCLE_ON;
    s_dtc_operation_cycle_sts = DTC_OPERATION_CYCLE_OFF;
    s_dtc_wr_wait_cnt = 50;
    s_dtc_ignon_condition = 0;
}

void can_dtc_process(void)
{
    dtc_data_read_from_nvm();
    dtc_fault_handle();
//    if(
    dtc_data_store_to_nvm();
    dtc_operation_cycle_update(); /* do the update at the end */
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
        && ((1 << DTC_STS_TESTFAILED) == (s_dtc_sts[typ] & (1 << DTC_STS_TESTFAILED)))
        )
    {
        shnapshot.record_number = DTC_SNAPSHOT_GLOBAL_RECORD_NUMBER;
        common_memcpy((uint08*)&g_dtc_snapshot[typ], (uint08*)&shnapshot, sizeof(stDTC_SNAPSHOT));
        s_dtc_appear_this_oc_sts[typ] = DTC_APPEAR_SNAPSHOT_OVER;
        s_dtc_wr_flg[typ] |= ((1 << 0) | (1 << 2));
    }
    else if (  (DTC_APPEAR_SNAPSHOT_LAST == s_dtc_appear_this_oc_sts[typ])
            && ((1 << DTC_STS_TESTFAILED) == (s_dtc_sts[typ] & (1 << DTC_STS_TESTFAILED)))
            )
    {
        shnapshot.record_number = DTC_SNAPSHOT_LOCAL_RECORD_NUMBER;
        common_memcpy((uint08*)&g_dtc_local_snapshot[typ], (uint08*)&shnapshot, sizeof(stDTC_SNAPSHOT));
        s_dtc_appear_this_oc_sts[typ] = DTC_APPEAR_SNAPSHOT_OVER;
        s_dtc_wr_flg[typ] |= ((1 << 0) | (1 << 3));
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
    else if (   (DTC_SNAPSHOT_ALL_RECORD_NUMBER == record_number)
             && (DTC_SNAPSHOT_GLOBAL_RECORD_NUMBER == g_dtc_snapshot[typ].record_number)
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
    return (s_dtc_sts[typ] & DTC_SUPPORTED_STS);
}

uint16 dtc_number_get(void)
{
    eDTC_TYP typ = (eDTC_TYP)0;
    uint16 num = 0;
    
    for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
    {
        if (0 != (s_dtc_sts[typ] & (1 << DTC_STS_CONFIRMEDDTC)))
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
        if (0 != (s_dtc_sts[typ] & sts))
        {
            num++;
        }
        else
        {}
    }

    return num;
}

void dtc_ignon_condition_set(uint08 val)
{
    s_dtc_ignon_condition = val;
}

uint08 dtc_ignon_condition_get(void)
{
    return s_dtc_ignon_condition;
}

static void dtc_data_read_from_nvm(void)
{
    eDTC_TYP typ = (eDTC_TYP)0;
    
    if (   (DTC_OPERATION_CYCLE_ON == s_dtc_operation_cycle_sw)
        && (DTC_OPERATION_CYCLE_OFF == s_dtc_operation_cycle_sts)
        )
    {
        /* read data form eeprom */
        eeprom_app_item_read(EE_RD_ITEM_DTC_STATUS);
//        eeprom_app_item_read(EE_RD_ITEM_DTC_PENDING);
        eeprom_app_item_read(EE_RD_ITEM_DTC_SNAPSHOT);
        eeprom_app_item_read(EE_RD_ITEM_DTC_LOCAL_SNAPSHOT);
//        eeprom_app_item_read(EE_RD_ITEM_DTC_EXTENDED);
//        SEGGER_RTT_printf(0, "eeprom read dtc to ram.\r\n");
        
        for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
        {
            if ((0xFF == g_dtc_confirmed_sts[typ]) || (0 == g_dtc_confirmed_sts[typ]))
            {
                status_confirmed_dtc(typ, 0);
            }
            else if (g_dtc_confirmed_sts[typ] & ((1 << DTC_STS_TESTFAILED) | (1 << DTC_STS_PENDINGDTC) | (1 << DTC_STS_CONFIRMEDDTC)))
            {
                status_confirmed_dtc(typ, 1);
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
    static uint08 wr_flg = 0;
    
    for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
    {
        if (s_dtc_wr_flg[typ] & (1 << 0))
        {
            wr_flg |= (1 << 0);
        }
        else {}

        if (s_dtc_wr_flg[typ] & (1 << 1))
        {
            wr_flg |= (1 << 1);
        }
        else {}

        if (s_dtc_wr_flg[typ] & (1 << 2))
        {
            wr_flg |= (1 << 2);
        }
        else {}

        if (s_dtc_wr_flg[typ] & (1 << 3))
        {
            wr_flg |= (1 << 3);
        }
        else {}
        s_dtc_wr_flg[typ] = 0;
    }
    
    if ((wr_flg)&& (0 == s_dtc_wr_wait_cnt) )//&& (0 == s_dtc_wr_wait_cnt)
    {
        if (wr_flg & (1 << 0))
        {
            err1 = eeprom_app_wr_err_get(EE_WR_ITEM_DTC_STATUS);
            if ((EE_CTRL_ERR_NONE == err1) || (EE_CTRL_ERR_WR_SUCCESS == err1) || (EE_CTRL_ERR_BUF_OVF == err1))
            {
                eeprom_app_buf_write(EE_WR_ITEM_DTC_STATUS,
                                     EE_DTC_STATUS_ADDR,
                                     EE_DTC_STATUS_BYTES * CAN_DTC_NUM,
                                     (uint08*)s_dtc_sts);
                wr_flg &= ~(1 << 0);
                s_dtc_wr_wait_cnt = 50;
            }
            else {}
        }
        else if (wr_flg & (1 << 1))
        {
            wr_flg &= ~(1 << 1);
        }
        else if (wr_flg & (1 << 2))
        {
            err2 = eeprom_app_wr_err_get(EE_WR_ITEM_DTC_SNAPSHOT);
            if ((EE_CTRL_ERR_NONE == err2) || (EE_CTRL_ERR_WR_SUCCESS == err2) || (EE_CTRL_ERR_BUF_OVF == err2))
            {
                eeprom_app_buf_write(EE_WR_ITEM_DTC_SNAPSHOT,
                                     EE_DTC_SNAPSHOT_ADDR,
                                     EE_DTC_SNAPSHOT_BYTES * CAN_DTC_NUM,
                                     (uint08*)g_dtc_snapshot);
                wr_flg &= ~(1 << 2);
                s_dtc_wr_wait_cnt = 50;
            }
            else {}
        }
        else if (wr_flg & (1 << 3))
        {
           if(g_dtc_nvm_store )
           {
                err3 = eeprom_app_wr_err_get(EE_WR_ITEM_DTC_LOCAL_SNAPSHOT);
                if ((EE_CTRL_ERR_NONE == err3) || (EE_CTRL_ERR_WR_SUCCESS == err3) || (EE_CTRL_ERR_BUF_OVF == err3))
                {
                    eeprom_app_buf_write(EE_WR_ITEM_DTC_LOCAL_SNAPSHOT,
                                        EE_DTC_LOCAL_SNAPSHOT_ADDR,
                                        EE_DTC_SNAPSHOT_BYTES * CAN_DTC_NUM,
                                        (uint08*)g_dtc_local_snapshot);
                    wr_flg &= ~(1 << 3);
                }
                else {}
            }
        }
        else if (wr_flg & (1 << 4))
        {
//            err3 = eeprom_app_wr_err_get(EE_WR_ITEM_DTC_EXTENDED);
//            if ((EE_CTRL_ERR_NONE == err3) || (EE_CTRL_ERR_WR_SUCCESS == err3) || (EE_CTRL_ERR_BUF_OVF == err3))
//            {
//                eeprom_app_buf_write(EE_WR_ITEM_DTC_EXTENDED,
//                                     EE_DTC_EXTENDED_ADDR,
//                                     EE_DTC_EXTENDED_BYTES * CAN_DTC_NUM,
//                                     (uint08*)g_dtc_local_snapshot);
//                wr_flg &= ~(1 << 3);
//            }
//            else {}
        }
        else
        {
            wr_flg = 0;
        }
    }
    else {}
    if (s_dtc_wr_wait_cnt)
    {
        s_dtc_wr_wait_cnt--;
    }
    else {}
}

static void dtc_fault_handle(void)
{
    eDTC_TYP typ = (eDTC_TYP)0;
    
    for (typ = (eDTC_TYP)0; typ < CAN_DTC_NUM; typ++)
    {
        dtc_fault_once_handle(typ);
    }
}

static void dtc_fault_once_handle(eDTC_TYP typ)
{
    stDTC_CFG* cfg = (stDTC_CFG*)g_can_dtc_cfg;
    uint08 fault_sts = DTC_FAULT_DISAPPEAR;

    if (s_dtc_fault_clr_cnt[typ] < DTC_FAULT_CLR_LAST_TIME)
    {
        s_dtc_fault_clr_cnt[typ]++;
    }
    else {}
    
    if (   (cfg[typ].fault_func != DEF_NULL)
        && (SERVICE_85_DTCSETTINGTYPE_ON == dcm_service_85_dtcsettingtype_get())
        && (!IS_KL15_OFF)
        && (s_dtc_fault_clr_cnt[typ] >= DTC_FAULT_CLR_LAST_TIME)
//        && (cfg[typ].condition_func != DEF_NULL)
//        && (DTC_CONDITION_PASS == cfg[typ].condition_func())
        )
    {
        fault_sts = cfg[typ].fault_func();
        if (DTC_FAULT_APPEAR == fault_sts)
        {
            status_test_failed(typ, 1);
            status_test_failed_this_operation_cycle(typ, 1);
            if (0 == s_dtc_ext_fault_occ_flg[typ]) /* DTC status bit0 0->1 trigger failure occurrence count. */
            {
                s_dtc_ext_fault_occ_flg[typ] = 1;
                if (g_dtc_extended[typ].fault_occ_cnt < 0xFFFFFFFF)
                {
                    g_dtc_extended[typ].fault_occ_cnt++;
                }
                else {}

                s_dtc_oc_fault_cnt[typ]++;
                if (s_dtc_oc_fault_cnt[typ] >= cfg[typ].confirmed_cycle_num)
                {
                    s_dtc_oc_fault_cnt[typ] = 0;
                    status_pending_dtc(typ, 1);
                    #if 0
                    status_confirmed_dtc(typ, 1);
                    #endif
                    if (DTC_DISAPPEAR == s_dtc_appear_this_oc_sts[typ]&&(0 == (s_dtc_sts[typ] & (1 << DTC_STS_CONFIRMEDDTC))))//(DTC_DISAPPEAR == s_dtc_appear_this_oc_sts[typ])
                    {
                        s_dtc_appear_this_oc_sts[typ] = DTC_APPEAR_SNAPSHOT_FIRST; /* write to eeprom */
                    }
                    else
                    {
                        s_dtc_appear_this_oc_sts[typ] = DTC_APPEAR_SNAPSHOT_LAST; /* write to eeprom */
                    }
                        
                    if (g_dtc_extended[typ].fault_pending_cnt < 0xFFFFFFFF)
                    {
                        g_dtc_extended[typ].fault_pending_cnt++;
                    }
                    else {}
                }
                else {}
            }
            else {}
            
            if (s_dtc_fault_cleared_flg[typ])
            {
                s_dtc_fault_cleared_flg[typ] = 0;
                status_test_failed_since_last_clear(typ, 1);
                status_test_not_completed_since_last_clear(typ, 0);
                status_test_not_completed_this_operation_cycle(typ, 0);
            }
            else {}
        }
        else
        {
            status_test_failed(typ, 0);
            s_dtc_ext_fault_occ_flg[typ] = 0;
            if (s_dtc_fault_cleared_flg[typ])
            {
                s_dtc_fault_cleared_flg[typ] = 0;
                status_test_not_completed_since_last_clear(typ, 0);
                status_test_not_completed_this_operation_cycle(typ, 0);
            }
            else {}
        }
    }
    else {}
    
    dtc_clear_process(typ);
    dtc_extend_process(typ);
}

static void dtc_operation_cycle_update(void)
{  
    s_dtc_operation_cycle_sts = s_dtc_operation_cycle_sw;
    
}

static void status_test_failed(eDTC_TYP typ, uint08 sts)
{
    if (sts)
    {
        s_dtc_sts[typ] |= (1 << DTC_STS_TESTFAILED);
    }
    else
    {
        s_dtc_sts[typ] &= ~(1 << DTC_STS_TESTFAILED);
    }
}

static void status_test_failed_this_operation_cycle(eDTC_TYP typ, uint08 sts)
{
    if (sts)
    {
        s_dtc_sts[typ] |= (1 << DTC_STS_TESTFAILEDTHISOPERATIONCYCLE);
    }
    else
    {
        s_dtc_sts[typ] &= ~(1 << DTC_STS_TESTFAILEDTHISOPERATIONCYCLE);
    }
}

static void status_pending_dtc(eDTC_TYP typ, uint08 sts)
{
    if (sts)
    {
        s_dtc_sts[typ] |= (1 << DTC_STS_PENDINGDTC);
    }
    else
    {
        s_dtc_sts[typ] &= ~(1 << DTC_STS_PENDINGDTC);
    }
}

static void status_confirmed_dtc(eDTC_TYP typ, uint08 sts)
{
    if (sts)
    {
        s_dtc_sts[typ] |= (1 << DTC_STS_CONFIRMEDDTC);
    }
    else
    {
        s_dtc_sts[typ] &= ~(1 << DTC_STS_CONFIRMEDDTC);
    }
}

static void status_test_not_completed_since_last_clear(eDTC_TYP typ, uint08 sts)
{
    if (sts)
    {
        s_dtc_sts[typ] |= (1 << DTC_STS_TESTNOTCOMPLETEDSINCELASTCLEAR);
    }
    else
    {
        s_dtc_sts[typ] &= ~(1 << DTC_STS_TESTNOTCOMPLETEDSINCELASTCLEAR);
    }
}

static void status_test_failed_since_last_clear(eDTC_TYP typ, uint08 sts)
{
    if (sts)
    {
        s_dtc_sts[typ] |= (1 << DTC_STS_TESTFAILEDSINCELASTCLEAR);
    }
    else
    {
        s_dtc_sts[typ] &= ~(1 << DTC_STS_TESTFAILEDSINCELASTCLEAR);
    }
}

static void status_test_not_completed_this_operation_cycle(eDTC_TYP typ, uint08 sts)
{
    if (sts)
    {
        s_dtc_sts[typ] |= (1 << DTC_STS_TESTNOTCOMPLETEDTHISOPERATIONCYCLE);
    }
    else
    {
        s_dtc_sts[typ] &= ~(1 << DTC_STS_TESTNOTCOMPLETEDTHISOPERATIONCYCLE);
    }
}

static void dtc_clear_process(eDTC_TYP typ)
{
    if (DEF_TRUE == s_dtc_clr_flg[typ])
    {
        s_dtc_clr_flg[typ] = DEF_FALSE;
        s_dtc_fault_cleared_flg[typ] = 1;
        s_dtc_fault_clr_cnt[typ] = 0;
        s_dtc_sts[typ] = 0;
        s_dtc_ext_fault_occ_flg[typ] = 0;
        g_dtc_confirmed_sts[typ] = 0;
        s_dtc_wr_flg[typ] = 0x0F;
        status_test_not_completed_since_last_clear(typ, 1);
        status_test_not_completed_this_operation_cycle(typ, 1);
        common_memset((uint08*)&g_dtc_snapshot[typ], 0U, sizeof(stDTC_SNAPSHOT));
        g_dtc_snapshot[typ].record_number = DTC_SNAPSHOT_GLOBAL_RECORD_NUMBER;
        common_memset((uint08*)&g_dtc_local_snapshot[typ], 0U, sizeof(stDTC_SNAPSHOT));
        g_dtc_local_snapshot[typ].record_number = DTC_SNAPSHOT_LOCAL_RECORD_NUMBER;
        common_memset((uint08*)&g_dtc_extended[typ], 0U, sizeof(stDTC_EXTENDED));
        common_memset((uint08*)s_dtc_appear_this_oc_sts, 0U, sizeof(s_dtc_appear_this_oc_sts));
    }
    else {}
}

static void dtc_extend_process(eDTC_TYP typ) //无上电power，无法做老化
{
    stDTC_CFG* cfg = (stDTC_CFG*)g_can_dtc_cfg;
    
    if (   (DTC_OPERATION_CYCLE_OFF == s_dtc_operation_cycle_sw)
        && (DTC_OPERATION_CYCLE_ON == s_dtc_operation_cycle_sts) /* IGN ON -> IGN OFF */
        )
    {
        if (   (0 == (s_dtc_sts[typ] & (1 << DTC_STS_TESTFAILED)))
            && (0 == (s_dtc_sts[typ] & (1 << DTC_STS_TESTFAILEDTHISOPERATIONCYCLE)))
            )
        {
            g_dtc_extended[typ].aging_cnt++;
            if (g_dtc_extended[typ].aging_cnt >= cfg[typ].aging_num) /* aging success, clear dtc except aged counter */
            {
                g_dtc_extended[typ].aging_cnt = 0;
                g_dtc_extended[typ].aged_cnt++;
                s_dtc_sts[typ] = 0;
                common_memset((uint08*)&g_dtc_snapshot[typ], 0U, sizeof(stDTC_SNAPSHOT));
                g_dtc_snapshot[typ].record_number = DTC_SNAPSHOT_GLOBAL_RECORD_NUMBER;
                common_memset((uint08*)&g_dtc_local_snapshot[typ], 0U, sizeof(stDTC_SNAPSHOT));
                g_dtc_local_snapshot[typ].record_number = DTC_SNAPSHOT_GLOBAL_RECORD_NUMBER;
            }
        }
        else {}
    }
    else {}
}
