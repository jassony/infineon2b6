/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : crc32.h
* Author        : yangming
* Date          : 2024-04-16
* Version       : 1.00
* Description   : crc32.
* Others        : None
*
****************************************************************************************************/
#ifndef _CRC32_H
#define _CRC32_H
#include "platform_common_typdef.h"

//extern uint32 calculate_crc32(uint32 *data, uint32 len);
extern uint32 CalcCrc32(uint32 crcInit, uint08* pBuf, uint32 len);
extern void crc32_test(void);
#endif /* _CRC32_H */

