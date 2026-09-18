/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : eeprom_ctrl.h
* Author        : yangming
* Date          : 2024-03-25
* Version       : 1.00
* Description   : EEPROM control.
* Others        : None
*
*******************************************************************************************************/
#ifndef _EEPROM_CTRL_H
#define _EEPROM_CTRL_H
#include "eeprom_driver.h"
#include "eeprom_ctrl_cfg.h"

extern void eeprom_ctrl_init(void);
extern void eeprom_ctrl_deinit(void);
extern void eeprom_ctrl_read(stEE_ITEM_INFO item_info, uint08* data, eEE_CTRL_ERR* err);
extern void eeprom_ctrl_fast_write(stEE_ITEM_INFO item_info, uint08* data);
extern void eeprom_ctrl_write_buf(stEE_ITEM_INFO item_info, uint08* data, eEE_CTRL_ERR* err);
extern void eeprom_ctrl_process(void);
#endif

