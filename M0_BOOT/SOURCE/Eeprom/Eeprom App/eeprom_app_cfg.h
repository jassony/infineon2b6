/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : eeprom_app_cfg.h
* Author        : yangming
* Date          : 2024-05-06
* Version       : 1.00
* Description   : EEPROM app configuration module.
* Others        : None
*
*******************************************************************************************************/
#ifndef _EEPROM_APP_CFG_H
#define _EEPROM_APP_CFG_H
#include "eeprom_ctrl_cfg.h"
#include "uds_user.h"

#define EE_DTC_STATUS_BYTES                                 1U
#define EE_DTC_PENDING_CLR_BYTES                            1U
#define EE_DTC_SNAPSHOT_BYTES                               6U
#define EE_DTC_LOCAL_SNAPSHOT_BYTES                         6U
#define EE_DTC_EXTENDED_BYTES                               7U

/* 0x00000020~0x0000011F for DID */
#define EE_DID_ADDR                                         0x00000020
#define EE_DID_F183_ADDR                                    EE_DID_ADDR

/* 0x000000120~0x001FF for DTC status and pending clear flag */
#define EE_DTC_STATUS_ADDR                                  0x000000120
#define EE_DTC_P333217_STATUS_ADDR                          EE_DTC_STATUS_ADDR
#define EE_DTC_P333216_STATUS_ADDR                          (EE_DTC_P333217_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_U008988_STATUS_ADDR                          (EE_DTC_P333216_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_U1E0288_STATUS_ADDR                          (EE_DTC_U008988_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_U104287_STATUS_ADDR                          (EE_DTC_U1E0288_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_U029387_STATUS_ADDR                          (EE_DTC_U104287_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_U104387_STATUS_ADDR                          (EE_DTC_U029387_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333314_STATUS_ADDR                          (EE_DTC_U104387_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333315_STATUS_ADDR                          (EE_DTC_P333314_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333414_STATUS_ADDR                          (EE_DTC_P333315_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333415_STATUS_ADDR                          (EE_DTC_P333414_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333514_STATUS_ADDR                          (EE_DTC_P333415_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333515_STATUS_ADDR                          (EE_DTC_P333514_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333601_STATUS_ADDR                          (EE_DTC_P333515_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333602_STATUS_ADDR                          (EE_DTC_P333601_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333603_STATUS_ADDR                          (EE_DTC_P333602_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_U1E0300_STATUS_ADDR                          (EE_DTC_P333603_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333722_STATUS_ADDR                          (EE_DTC_U1E0300_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P33374B_STATUS_ADDR                          (EE_DTC_P333722_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333721_STATUS_ADDR                          (EE_DTC_P33374B_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333901_STATUS_ADDR                          (EE_DTC_P333721_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333814_STATUS_ADDR                          (EE_DTC_P333901_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333815_STATUS_ADDR                          (EE_DTC_P333814_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_P333902_STATUS_ADDR                          (EE_DTC_P333815_STATUS_ADDR + EE_DTC_STATUS_BYTES)

#define EE_DTC_P1A0400_STATUS_ADDR                          EE_DTC_STATUS_ADDR
#define EE_DTC_P1A0401_STATUS_ADDR                          (EE_DTC_P1A0400_STATUS_ADDR + EE_DTC_STATUS_BYTES)

#define EE_DTC_PENDING_CLR_FLG_ADDR                         0x000000140
/* 0x000000200~0x003FF for DTC global snapshot */
#define EE_DTC_SNAPSHOT_ADDR                                0x000000200
#define EE_DTC_P333217_SNAPSHOT_ADDR                        EE_DTC_SNAPSHOT_ADDR
#define EE_DTC_P333216_SNAPSHOT_ADDR                        (EE_DTC_P333217_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_U008988_SNAPSHOT_ADDR                        (EE_DTC_P333216_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_U1E0288_SNAPSHOT_ADDR                        (EE_DTC_U008988_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_U104287_SNAPSHOT_ADDR                        (EE_DTC_U1E0288_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_U029387_SNAPSHOT_ADDR                        (EE_DTC_U104287_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_U104387_SNAPSHOT_ADDR                        (EE_DTC_U029387_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333314_SNAPSHOT_ADDR                        (EE_DTC_U104387_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333315_SNAPSHOT_ADDR                        (EE_DTC_P333314_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333414_SNAPSHOT_ADDR                        (EE_DTC_P333315_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333415_SNAPSHOT_ADDR                        (EE_DTC_P333414_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333514_SNAPSHOT_ADDR                        (EE_DTC_P333415_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333515_SNAPSHOT_ADDR                        (EE_DTC_P333514_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333601_SNAPSHOT_ADDR                        (EE_DTC_P333515_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333602_SNAPSHOT_ADDR                        (EE_DTC_P333601_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333603_SNAPSHOT_ADDR                        (EE_DTC_P333602_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_U1E0300_SNAPSHOT_ADDR                        (EE_DTC_P333603_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333722_SNAPSHOT_ADDR                        (EE_DTC_U1E0300_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P33374B_SNAPSHOT_ADDR                        (EE_DTC_P333722_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333721_SNAPSHOT_ADDR                        (EE_DTC_P33374B_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333901_SNAPSHOT_ADDR                        (EE_DTC_P333721_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333814_SNAPSHOT_ADDR                        (EE_DTC_P333901_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333815_SNAPSHOT_ADDR                        (EE_DTC_P333814_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_P333902_SNAPSHOT_ADDR                        (EE_DTC_P333815_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
/* 0x000000400~0x005FF for DTC local snapshot */
#define EE_DTC_LOCAL_SNAPSHOT_ADDR                          0x000000400
/* 0x000000600~0x007FF for DTC extended */
#define EE_DTC_EXTENDED_ADDR                                0x000000600


typedef enum _ee_wr_item_typ                                eEE_WR_ITEM_TYP;
typedef enum _ee_rd_item_typ                                eEE_RD_ITEM_TYP;
typedef enum _ee_buf_wr_sts                                 eEE_BUF_WR_STS;
typedef struct _ee_rd_cfg                                   stEE_RD_CFG;
//typedef struct _ee_wr_cfg                                   stEE_WR_CFG;

enum _ee_wr_item_typ
{
    /* DID */
    EE_WR_ITEM_DID_F199 = 0,
//    EE_WR_ITEM_DID_F184,
//    EE_WR_ITEM_DID_F185,
//    EE_WR_ITE_DID_F199,
//    /* DTC status */
    EE_WR_ITEM_DTC_STATUS,
    /* DTC pending clear flag */
    EE_WR_ITEM_DTC_PENDING,
    /* DTC snapshot */
    EE_WR_ITEM_DTC_SNAPSHOT,
    EE_WR_ITEM_DTC_LOCAL_SNAPSHOT,
    /* DTC extended */
    EE_WR_ITEM_DTC_EXTENDED,

    EE_WR_ITEM_NUM
};

enum _ee_rd_item_typ
{
    /* DID */
    EE_RD_ITEM_F199 = 0,
//    EE_RD_ITEM_F184,
//    EE_RD_ITEM_F185,
//    EE_RD_ITEM_F190,
    #if  0
    /* DTC status */
    EE_RD_ITEM_DTC_STATUS,
    /* DTC pending clear flag */
    EE_RD_ITEM_DTC_PENDING,
    /* DTC snapshot */
    EE_RD_ITEM_DTC_SNAPSHOT,
    EE_RD_ITEM_DTC_LOCAL_SNAPSHOT,
    /* DTC extended */
    EE_RD_ITEM_DTC_EXTENDED,
    #endif

    EE_RD_ITEM_NUM
};

enum _ee_buf_wr_sts
{
    EE_BWS_NONE = 0,
    EE_BWS_START,
    EE_BWS_ERR,
    EE_BWS_SUCCESS,

    EE_BWS_MAX
};

struct _ee_rd_cfg
{
    eEE_RD_ITEM_TYP                     item;
    EE_ADDR_INT                         addr;
    uint08*                             dp; /* data point */
    EE_LEN_INT                          len;
};

//struct _ee_wr_cfg
//{
//    eEE_WR_ITEM_TYP                     item;
//    EE_ADDR_INT                         addr;
//    uint08*                             dp; /* data point */
//    EE_LEN_INT                          len;
//};


extern const stEE_RD_CFG g_eeprom_rd_cfg[EE_RD_ITEM_NUM];
#endif /* _EEPROM_APP_CFG_H */
