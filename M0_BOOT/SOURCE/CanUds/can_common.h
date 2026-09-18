/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_common.h
* Author        : yangming
* Date          : 2024-01-22
* Version       : 1.00
* Description   : Can common header file.
* Others        : None
*
****************************************************************************************************/
#ifndef _CAN_COMMON_H
#define _CAN_COMMON_H
#include "platform_common_typdef.h"
#include "common_mem_op.h"

//#define _CAN_DEBUG_PRINTF_ISR_EN

#ifdef _DEFBUG_PRINTF_EN
#define CAN_DEBUG_PRINT(...)               SEGGER_RTT_printf(0, ##__VA_ARGS__)
#define CAN_DEBUG_PRINT_ERR(...)           DEF_PRINTF(RTT_CTRL_TEXT_RED"\r\n");SEGGER_RTT_printf(0, ##__VA_ARGS__);DEF_PRINTF(RTT_CTRL_TEXT_GREEN"\r\n")
#define CAN_DEBUG_PRINT_WRN(...)           DEF_PRINTF(RTT_CTRL_TEXT_YELLOW"\r\n");SEGGER_RTT_printf(0, ##__VA_ARGS__);DEF_PRINTF(RTT_CTRL_TEXT_GREEN"\r\n")
#ifdef _CAN_DEBUG_PRINTF_ISR_EN
#define CAN_ISR_DEBUG_PRINT(...)           SEGGER_RTT_printf(0, ##__VA_ARGS__)
#define CAN_ISR_DEBUG_PRINT_ERR(...)       DEF_PRINTF(RTT_CTRL_TEXT_RED"\r\n");SEGGER_RTT_printf(0, ##__VA_ARGS__);DEF_PRINTF(RTT_CTRL_TEXT_GREEN"\r\n")
#define CAN_ISR_DEBUG_PRINT_WRN(...)       DEF_PRINTF(RTT_CTRL_TEXT_YELLOW"\r\n");SEGGER_RTT_printf(0, ##__VA_ARGS__);DEF_PRINTF(RTT_CTRL_TEXT_GREEN"\r\n")
#else
#define CAN_ISR_DEBUG_PRINT(...)           
#define CAN_ISR_DEBUG_PRINT_ERR(...)       
#define CAN_ISR_DEBUG_PRINT_WRN(...)       
#endif
#else
#define CAN_DEBUG_PRINT(...)
#define CAN_DEBUG_PRINT_ERR(...)
#define CAN_DEBUG_PRINT_WRN(...)
#define CAN_ISR_DEBUG_PRINT(...)           
#define CAN_ISR_DEBUG_PRINT_ERR(...)       
#define CAN_ISR_DEBUG_PRINT_WRN(...)       
#endif

typedef uint16                              PDU_ID;
typedef enum _canid_type                    eCANID_TYP;
typedef struct _pdu_info                    stPDU_INFO;

enum _canid_type
{
    STANDARD_CAN = 0,
    EXTEND_CAN,
    MIXD_CAN
};

struct _pdu_info
{
    uint08*         sdu_data;
    uint32          sdu_len;
};

#endif /* _CAN_COMMON_H */

