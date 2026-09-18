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
#define EE_DTC_SNAPSHOT_BYTES                               24U
#define EE_DTC_LOCAL_SNAPSHOT_BYTES                         24U
#define EE_DTC_EXTENDED_BYTES                               24U

/* 0x00000020~0x0000011F for DID */
#define DID_APP_START_ADDR                                  CY_WFLASH_SM_SBM_BASE +  5 * CY_WORK_SES_SIZE_IN_BYTE   
#define DID_F190_ADDR                                       DID_APP_START_ADDR   
#define DID_F1A8_ADDR                                       DID_F190_ADDR + DID_F190_DATA_LEN   

/* 0x000000120~0x001FF for DTC status and pending clear flag */
#define EE_DTC_STATUS_ADDR                                  CY_WFLASH_SM_SBM_BASE +  6 * CY_WORK_SES_SIZE_IN_BYTE 
#define EE_DTC_B100014_STATUS_ADDR                          EE_DTC_STATUS_ADDR 
#define EE_DTC_B100114_STATUS_ADDR                          (EE_DTC_B100014_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_B100214_STATUS_ADDR                          (EE_DTC_B100114_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_B100314_STATUS_ADDR                          (EE_DTC_B100214_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_B100614_STATUS_ADDR                          (EE_DTC_B100314_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_B100814_STATUS_ADDR                          (EE_DTC_B100614_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_B100C14_STATUS_ADDR                          (EE_DTC_B100814_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_B101014_STATUS_ADDR                          (EE_DTC_B100C14_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_B101114_STATUS_ADDR                          (EE_DTC_B101014_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_B101214_STATUS_ADDR                          (EE_DTC_B101114_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_B101414_STATUS_ADDR                          (EE_DTC_B101214_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_B101577_STATUS_ADDR                          (EE_DTC_B101414_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_B101977_STATUS_ADDR                          (EE_DTC_B101577_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_U14D677_STATUS_ADDR                          (EE_DTC_B101977_STATUS_ADDR + EE_DTC_STATUS_BYTES)
#define EE_DTC_U14E077_STATUS_ADDR                          (EE_DTC_U14D677_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_U14E177_STATUS_ADDR                          (EE_DTC_U14E077_STATUS_ADDR + EE_DTC_STATUS_BYTES)

// #define EE_DTC_P333217_STATUS_ADDR                          EE_DTC_STATUS_ADDR
// #define EE_DTC_P333216_STATUS_ADDR                          (EE_DTC_P333217_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_U008988_STATUS_ADDR                          (EE_DTC_P333216_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_U1E0288_STATUS_ADDR                          (EE_DTC_U008988_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_U104287_STATUS_ADDR                          (EE_DTC_U1E0288_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_U029387_STATUS_ADDR                          (EE_DTC_U104287_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_U104387_STATUS_ADDR                          (EE_DTC_U029387_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333314_STATUS_ADDR                          (EE_DTC_U104387_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333315_STATUS_ADDR                          (EE_DTC_P333314_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333414_STATUS_ADDR                          (EE_DTC_P333315_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333415_STATUS_ADDR                          (EE_DTC_P333414_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333514_STATUS_ADDR                          (EE_DTC_P333415_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333515_STATUS_ADDR                          (EE_DTC_P333514_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333601_STATUS_ADDR                          (EE_DTC_P333515_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333602_STATUS_ADDR                          (EE_DTC_P333601_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333603_STATUS_ADDR                          (EE_DTC_P333602_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_U1E0300_STATUS_ADDR                          (EE_DTC_P333603_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333722_STATUS_ADDR                          (EE_DTC_U1E0300_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P33374B_STATUS_ADDR                          (EE_DTC_P333722_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333721_STATUS_ADDR                          (EE_DTC_P33374B_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333901_STATUS_ADDR                          (EE_DTC_P333721_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333814_STATUS_ADDR                          (EE_DTC_P333901_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333815_STATUS_ADDR                          (EE_DTC_P333814_STATUS_ADDR + EE_DTC_STATUS_BYTES)
// #define EE_DTC_P333902_STATUS_ADDR                          (EE_DTC_P333815_STATUS_ADDR + EE_DTC_STATUS_BYTES)

// #define EE_DTC_P1A0400_STATUS_ADDR                          EE_DTC_STATUS_ADDR
// #define EE_DTC_P1A0401_STATUS_ADDR                          (EE_DTC_P1A0400_STATUS_ADDR + EE_DTC_STATUS_BYTES)


#define EE_DTC_PENDING_CLR_FLG_ADDR                         CY_WFLASH_SM_SBM_BASE +  7 * CY_WORK_SES_SIZE_IN_BYTE
/* 0x000000200~0x003FF for DTC global snapshot */
#define EE_DTC_SNAPSHOT_ADDR                                CY_WFLASH_SM_SBM_BASE +  8 * CY_WORK_SES_SIZE_IN_BYTE
#define EE_DTC_B100014_SNAPSHOT_ADDR                        EE_DTC_SNAPSHOT_ADDR 
#define EE_DTC_B100114_SNAPSHOT_ADDR                        (EE_DTC_B100014_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B100214_SNAPSHOT_ADDR                        (EE_DTC_B100114_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B100314_SNAPSHOT_ADDR                        (EE_DTC_B100214_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B100614_SNAPSHOT_ADDR                        (EE_DTC_B100314_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B100814_SNAPSHOT_ADDR                        (EE_DTC_B100614_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B100C14_SNAPSHOT_ADDR                        (EE_DTC_B100814_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B101014_SNAPSHOT_ADDR                        (EE_DTC_B100C14_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B101114_SNAPSHOT_ADDR                        (EE_DTC_B101014_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B101214_SNAPSHOT_ADDR                        (EE_DTC_B101114_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B101414_SNAPSHOT_ADDR                        (EE_DTC_B101214_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B101577_SNAPSHOT_ADDR                        (EE_DTC_B101414_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B101977_SNAPSHOT_ADDR                        (EE_DTC_B101577_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_U14D677_SNAPSHOT_ADDR                        (EE_DTC_B101977_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_U14E077_SNAPSHOT_ADDR                        (EE_DTC_U14D677_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
//#define EE_DTC_U14E177_SNAPSHOT_ADDR                        (EE_DTC_U14E077_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)

// #define EE_DTC_P333217_SNAPSHOT_ADDR                        EE_DTC_SNAPSHOT_ADDR
// #define EE_DTC_P333216_SNAPSHOT_ADDR                        (EE_DTC_P333217_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_U008988_SNAPSHOT_ADDR                        (EE_DTC_P333216_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_U1E0288_SNAPSHOT_ADDR                        (EE_DTC_U008988_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_U104287_SNAPSHOT_ADDR                        (EE_DTC_U1E0288_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_U029387_SNAPSHOT_ADDR                        (EE_DTC_U104287_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_U104387_SNAPSHOT_ADDR                        (EE_DTC_U029387_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333314_SNAPSHOT_ADDR                        (EE_DTC_U104387_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333315_SNAPSHOT_ADDR                        (EE_DTC_P333314_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333414_SNAPSHOT_ADDR                        (EE_DTC_P333315_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333415_SNAPSHOT_ADDR                        (EE_DTC_P333414_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333514_SNAPSHOT_ADDR                        (EE_DTC_P333415_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333515_SNAPSHOT_ADDR                        (EE_DTC_P333514_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333601_SNAPSHOT_ADDR                        (EE_DTC_P333515_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333602_SNAPSHOT_ADDR                        (EE_DTC_P333601_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333603_SNAPSHOT_ADDR                        (EE_DTC_P333602_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_U1E0300_SNAPSHOT_ADDR                        (EE_DTC_P333603_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333722_SNAPSHOT_ADDR                        (EE_DTC_U1E0300_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P33374B_SNAPSHOT_ADDR                        (EE_DTC_P333722_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333721_SNAPSHOT_ADDR                        (EE_DTC_P33374B_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333901_SNAPSHOT_ADDR                        (EE_DTC_P333721_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333814_SNAPSHOT_ADDR                        (EE_DTC_P333901_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333815_SNAPSHOT_ADDR                        (EE_DTC_P333814_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
// #define EE_DTC_P333902_SNAPSHOT_ADDR                        (EE_DTC_P333815_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)


/* 0x000000400~0x005FF for DTC local snapshot */
#define EE_DTC_LOCAL_SNAPSHOT_ADDR                          CY_WFLASH_SM_SBM_BASE +  13 * CY_WORK_SES_SIZE_IN_BYTE
#define EE_DTC_B100014_LOCAL_SNAPSHOT_ADDR                        EE_DTC_LOCAL_SNAPSHOT_ADDR 
#define EE_DTC_B100114_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B100014_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B100214_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B100114_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B100314_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B100214_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B100614_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B100314_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B100814_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B100614_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B100C14_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B100814_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B101014_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B100C14_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B101114_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B101014_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B101214_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B101114_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B101414_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B101214_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B101577_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B101414_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_B101977_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B101577_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_U14D677_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_B101977_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
#define EE_DTC_U14E077_LOCAL_SNAPSHOT_ADDR                        (EE_DTC_U14D677_LOCAL_SNAPSHOT_ADDR + EE_DTC_SNAPSHOT_BYTES)
/* 0x000000600~0x007FF for DTC extended */
#define EE_DTC_EXTENDED_ADDR                                CY_WFLASH_SM_SBM_BASE +  18 * CY_WORK_SES_SIZE_IN_BYTE


typedef enum _ee_wr_item_typ                                eEE_WR_ITEM_TYP;
typedef enum _ee_rd_item_typ                                eEE_RD_ITEM_TYP;
typedef enum _ee_buf_wr_sts                                 eEE_BUF_WR_STS;
typedef struct _ee_rd_cfg                                   stEE_RD_CFG;
//typedef struct _ee_wr_cfg                                   stEE_WR_CFG;

enum _ee_wr_item_typ
{
    /* DID */
    EE_WR_ITEM_DID_F190 = 0,
    EE_WR_ITEM_DID_F1A8,
    // EE_WR_ITEM_DID_F185,
    // EE_WR_ITE_DID_F190,
    /* DTC status */
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
    EE_RD_ITEM_F190 = 0,
    EE_RD_ITEM_F1A8,
//    EE_RD_ITEM_F185,
//    EE_RD_ITEM_F190,
//    #if  0
    /* DTC status */
    EE_RD_ITEM_DTC_STATUS,
    /* DTC pending clear flag */
//    EE_RD_ITEM_DTC_PENDING, 
    /* DTC snapshot */
    EE_RD_ITEM_DTC_SNAPSHOT,
    EE_RD_ITEM_DTC_LOCAL_SNAPSHOT,
    /* DTC extended */
    EE_RD_ITEM_DTC_EXTENDED,
//    #endif

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

