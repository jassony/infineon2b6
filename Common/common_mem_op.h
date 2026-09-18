/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : common_mem_op.h
* Author        : yangming
* Date          : 2024-01-22
* Version       : 1.00
* Description   : Common memory operation header file.
* Others        : None
*
****************************************************************************************************/
#ifndef _COMMON_MEM_OP_H
#define _COMMON_MEM_OP_H
#include "platform_common_typdef.h"

extern void common_memcpy(uint08* dest, uint08* src, uint32 length);
extern void common_memset(uint08* dest, uint32 val, uint32 length);
extern typ_bool common_compare(uint08* buf1, uint08* buf2, uint32 length);
#endif /* _COMMON_MEM_OP_H */

