/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : platform_common_typdef.h
* Author        : yangming
* Date          : 2024-01-22
* Version       : 1.00
* Description   : Platform common definition file.
* Others        : None
*
****************************************************************************************************/
#ifndef _PLATFORM_COMMON_TYPDEF_H
#define _PLATFORM_COMMON_TYPDEF_H
//#include "SEGGER_RTT.h"
//#ifdef 
//#define CHIP_S32K144                    0
//#define CHIP_PIC33EP                    1
//#define CHIP_RH850                      2
//#define PLATFORM_CHIP_SELECT            CHIP_S32K144
#include "stddef.h"
#if 0
#define PLATFORM_S32DS
#endif

#ifdef USE_M0
typedef signed long int                 sint32;
typedef unsigned long int               uint32;
#elif  USE_M4
#include "types.h"
#endif

extern uint32 g_heartbeat_cnt;
//#define _DEFBUG_PRINTF_EN
#ifdef  _DEFBUG_PRINTF_EN
#define DEF_PRINTF(...)                 SEGGER_RTT_printf(0, ##__VA_ARGS__)
#define DEF_PRINTF_TIME(...)            DEF_PRINTF("[sys:%d]", g_heartbeat_cnt);SEGGER_RTT_printf(0, ##__VA_ARGS__)
#else
#define DEF_PRINTF(...)
#define DEF_PRINTF_TIME(...)

#endif

#ifdef  PLATFORM_S32DS
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-const-variable="
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif

#define DEF_NULL                        (void*)0
#define DEF_TRUE                        1
#define DEF_FALSE                       0

#define MIN2(a, b)                      ((a) < (b) ? (a) : (b))
#define MAX2(a, b)                      ((a) > (b) ? (a) : (b))

#define BYTE_IS_BCD(byte)               ((((byte >> 4U) < 10U) && ((byte & 0x0F) < 10U)) ? (DEF_TRUE) : (DEF_FALSE))

typedef signed char                     sint08;
typedef signed short int                sint16;
typedef signed long long int            sint64;
typedef unsigned char                   uint08;
typedef unsigned short int              uint16;
typedef unsigned long long int          uint64;
typedef uint08                          typ_bool;
typedef enum _err_sts                   eERR_STS;
typedef union _uint32_byte              unUINT32_B;

enum _err_sts
{
    E_OK = 0,
    E_NOK,
    E_BUF_OVF,
    E_BUSY,

    E_MAX
};

union _uint32_byte
{
    uint32 bytes;
    struct
    {
        uint08 byte0;
        uint08 byte1;
        uint08 byte2;
        uint08 byte3;
    }B;
};

#endif /* _PLATFORM_COMMON_TYPDEF_H */

