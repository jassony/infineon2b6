/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : eeprom_ctrl_cfg.h
* Author        : yangming
* Date          : 2024-03-25
* Version       : 1.00
* Description   : EEPROM control configuration.
* Others        : None
*
*******************************************************************************************************/
#ifndef _EEPROM_CTRL_CFG_H
#define _EEPROM_CTRL_CFG_H
#include "platform_common_typdef.h"
#include "ringbuf.h"
#include "soft_timer.h"

#define EE_WR_BUF_NUM                       512U /* size must be a power of 2(2^n) */
#define EE_ITEM_OCCUPIED_BYTE               sizeof(stEE_ITEM_INFO)
#define EE_WR_ITEM_BUF_SIZE                 256U /* the size of an item write buffer */
#define EE_WR_ITEM_TOTAL_SIZE               (EE_ITEM_OCCUPIED_BYTE + EE_WR_ITEM_BUF_SIZE)
#define EE_WR_CNT_NUM                       3U
#define EE_WR_DRV_FB_WAIT_TIME              6U//10U
#define EE_WR_PAGE_BYTE_SIZE                32U

typedef uint32                              EE_ADDR_INT;
typedef uint16                              EE_LEN_INT;
typedef uint16                              EE_ITEM_INT;
typedef enum _ee_ctrl_sts                   eEE_CTRL_STS;
typedef enum _ee_ctrl_err                   eEE_CTRL_ERR;
typedef enum _ee_wr_sts                     eEE_WR_STS;
typedef struct _ee_item_info                stEE_ITEM_INFO;
typedef struct _ee_wr_info                  stEE_WR_INFO;

enum _ee_ctrl_sts
{
    EE_CTRL_IDLE = 0,
    EE_CTRL_WR_PROCEED,
    EE_CTRL_RD_PROCEED,

    EE_CTRL_MAX
};

enum _ee_ctrl_err
{
    EE_CTRL_ERR_NONE = 0,
    EE_CTRL_ERR_WR_PROCESS,
    EE_CTRL_ERR_WR_SUCCESS,
    EE_CTRL_ERR_RD_SUCCESS,
    EE_CTRL_ERR_BUSY,
    EE_CTRL_ERR_HW,
    EE_CTRL_ERR_BUF_OVF,
    EE_CTRL_ERR_IN_PARAM,
    EE_CTRL_ERR_OTR,
    
    EE_CTRL_ERR_MAX
};

enum _ee_wr_sts
{
    EE_WR_STS_IDLE = 0,
    EE_WR_STS_START,
    EE_WR_STS_PROCEED,
    EE_WR_STS_CHK,
    
    EE_WR_STS_MAX
};

struct _ee_item_info
{
    EE_ADDR_INT                             addr;
    EE_LEN_INT                              len; /* the passed target length */
    EE_ITEM_INT                             item;
};

struct _ee_wr_info
{
    eEE_WR_STS                              wr_sts;
    stSOFT_TIMER                            timer;
    stSOFT_TIMER                            fast_timer;
    eEE_CTRL_ERR                            wr_err;
    uint08                                  wr_cnt;
    uint08                                  quotient;
    uint08                                  remainder;
    uint08                                  page;
};
#endif /* _EEPROM_CTRL_CFG_H */
