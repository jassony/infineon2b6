/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : eeprom_driver.h
* Author        : yangming
* Date          : 2024-03-01
* Version       : 1.00
* Description   : EEPROM driver.
* Others        : None
*
*******************************************************************************************************/
#ifndef _EEPROM_DRIVER_H
#define _EEPROM_DRIVER_H
//#include "m95xxx_driver.h"
#include "platform_common_typdef.h"
#include "common_mem_op.h"


#define EEPROM_DRV_DEBUG_ENABLE

extern void eeprom_drv_init(void);
extern void eeprom_drv_deinit(void);
extern typ_bool s32k144_flash_read(uint32 addr, uint08* data, uint32 len);
extern typ_bool s32k144_flash_write(uint32 addr, uint08* data, uint32 len);
extern typ_bool s32k144_flash_erase_sector(uint32 addr, uint32 size);
extern void eeprom_drv_write(uint32 addr, uint08* data, uint32 len);
extern void eeprom_drv_read(uint32 addr, uint08* data, uint32 len);
extern typ_bool eeprom_drv_wr_sts_get(void);
extern typ_bool eeprom_drv_rd_sts_get(void);

#ifdef EEPROM_DRV_DEBUG_ENABLE
extern void eeprom_drv_test(void);
#endif

#endif /* _EEPROM_DRIVER_H */

