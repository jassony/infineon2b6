/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : eeprom_app.c
* Author        : yangming
* Date          : 2024-05-06
* Version       : 1.00
* Description   : EEPROM application module.
* Others        : None
*
*******************************************************************************************************/
#include "eeprom_app.h"

static eEE_CTRL_ERR s_ee_wr_err[EE_WR_ITEM_NUM];
static eEE_CTRL_ERR s_ee_rd_err[EE_RD_ITEM_NUM];


void eeprom_app_init(void)
{
    eeprom_ctrl_init();
    common_memset((uint08*)s_ee_wr_err, 0U, sizeof(s_ee_wr_err));
    common_memset((uint08*)s_ee_rd_err, 0U, sizeof(s_ee_rd_err));
}

void eeprom_app_deinit(void)
{
    eeprom_ctrl_deinit();
}

void eeprom_app_process(void)
{
    eeprom_ctrl_process();
}

void eeprom_app_item_read(eEE_RD_ITEM_TYP item)
{
    stEE_RD_CFG* rd_info = (stEE_RD_CFG*)g_eeprom_rd_cfg;
    stEE_ITEM_INFO item_info;
    uint16 i = 0;

    for (i = 0; i < EE_RD_ITEM_NUM; i++)
    {
        if (rd_info[i].item == item)
        {
            item_info.addr = rd_info[i].addr;
            item_info.len = rd_info[i].len;
            item_info.item = item;
            eeprom_ctrl_read(item_info, rd_info[i].dp, &s_ee_rd_err[item]);
            break;
        }
        else
        {
            /* continue */
        }
    }
}

void eeprom_app_buf_write(eEE_WR_ITEM_TYP item, EE_ADDR_INT addr, EE_LEN_INT len, uint08* data)
{
    stEE_ITEM_INFO item_info;

    item_info.item = item;
    item_info.addr = addr;
    item_info.len = len;
    eeprom_ctrl_write_buf(item_info, data, &s_ee_wr_err[item]);
}

void eeprom_app_copy_wr_err(eEE_WR_ITEM_TYP item, eEE_CTRL_ERR* err)
{
    s_ee_wr_err[item] = *err;
}

eEE_CTRL_ERR eeprom_app_wr_err_get(eEE_WR_ITEM_TYP item)
{
    return s_ee_wr_err[item];
}

eEE_CTRL_ERR eeprom_app_rd_err_get(eEE_WR_ITEM_TYP item)
{
    return s_ee_rd_err[item];
}

