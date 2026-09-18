/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : eeprom_app.h
* Author        : yangming
* Date          : 2024-05-06
* Version       : 1.00
* Description   : EEPROM application module.
* Others        : None
*
*******************************************************************************************************/
#ifndef _EEPROM_APP_H
#define _EEPROM_APP_H
#include "eeprom_ctrl.h"
#include "eeprom_app_cfg.h"

extern eEE_BUF_WR_STS g_ee_buf_wr_sts[EE_WR_ITEM_NUM];

extern void eeprom_app_init(void);
extern void eeprom_app_deinit(void);
extern void eeprom_app_process(void);
extern void eeprom_app_item_read(eEE_RD_ITEM_TYP item);
extern void eeprom_app_buf_write(eEE_WR_ITEM_TYP item, EE_ADDR_INT addr, EE_LEN_INT len, uint08* data);
extern eEE_CTRL_ERR eeprom_app_wr_err_get(eEE_WR_ITEM_TYP item);
extern eEE_CTRL_ERR eeprom_app_rd_err_get(eEE_WR_ITEM_TYP item);
#endif /* _EEPROM_APP_H */

