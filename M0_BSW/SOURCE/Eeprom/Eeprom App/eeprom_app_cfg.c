/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : eeprom_app_cfg.c
* Author        : yangming
* Date          : 2024-05-06
* Version       : 1.00
* Description   : EEPROM app configuration module.
* Others        : None
*
*******************************************************************************************************/
#include "eeprom_app_cfg.h"
#include "can_dtc.h"

const stEE_RD_CFG g_eeprom_rd_cfg[EE_RD_ITEM_NUM] = 
{
    #if 0

    {
        .item                       = EE_RD_ITEM_F183,
        .addr                       = EE_DID_F183_ADDR,
        .dp                         = (uint08*)did_f183_val,
        .len                        = DID_F183_DATA_LEN
    },
    {
        .item                       = EE_RD_ITEM_F184,
        .addr                       = EE_DID_F184_ADDR,
        .dp                         = (uint08*)did_f184_val,
        .len                        = DID_F184_DATA_LEN
    },
    {
        .item                       = EE_RD_ITEM_F185,
        .addr                       = EE_DID_F185_ADDR,
        .dp                         = (uint08*)g_service_22.did_f185_val,
        .len                        = DID_F185_DATA_LEN
    },
    {
        .item                       = EE_RD_ITEM_F190,
        .addr                       = EE_DID_F190_ADDR,
        .dp                         = (uint08*)g_service_22.did_f190_val,
        .len                        = DID_F190_DATA_LEN
    },
    #endif
    {
        .item                       = EE_RD_ITEM_F190,
        .addr                       = DID_F190_ADDR,
        .dp                         = (uint08*)did_f190_val,
        .len                        = DID_F190_DATA_LEN
    },
    {
        .item                       = EE_RD_ITEM_F1A8,
        .addr                       = DID_F1A8_ADDR,
        .dp                         = (uint08*)did_f1a8_val,
        .len                        = DID_F1A8_DATA_LEN
    },
    {
        .item                       = EE_RD_ITEM_DTC_STATUS,
        .addr                       = EE_DTC_STATUS_ADDR,
        .dp                         = (uint08*)g_dtc_confirmed_sts,
        .len                        = EE_DTC_STATUS_BYTES * CAN_DTC_NUM
    },
    {
        .item                       = EE_RD_ITEM_DTC_SNAPSHOT,
        .addr                       = EE_DTC_SNAPSHOT_ADDR,
        .dp                         = (uint08*)g_dtc_snapshot,
        .len                        = EE_DTC_SNAPSHOT_BYTES * CAN_DTC_NUM
    },
    {
        .item                       = EE_RD_ITEM_DTC_LOCAL_SNAPSHOT,
        .addr                       = EE_DTC_LOCAL_SNAPSHOT_ADDR,
        .dp                         = (uint08*)g_dtc_local_snapshot,
        .len                        = EE_DTC_LOCAL_SNAPSHOT_BYTES * CAN_DTC_NUM
    },
   {
       .item                       = EE_RD_ITEM_DTC_EXTENDED,
       .addr                       = EE_DTC_EXTENDED_ADDR,
       .dp                         = (uint08*)g_dtc_extended,
       .len                        = EE_DTC_EXTENDED_BYTES * CAN_DTC_NUM
   },
};
