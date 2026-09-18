/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : canif_cfg.c
* Author        : yangming
* Date          : 2024-01-22
* Version       : 1.00
* Description   : Can interface configuartion file.
* Others        : None
*
****************************************************************************************************/
#include "canif_cfg.h"

static const uint08 s_phy_hrh_cfg[DCM_PDU_PHY_NUM][DCM_PDU_FUNC_MAX] = 
{
    /* first physical addressing */
    {
        CANIF_HRH0_FUNC,
        0xFF, /* 0xff indicates invalid */
        0xFF, /* 0xff indicates invalid */
    },
};

/* Configure functional addressing for physical addressing */
const stDCM_PDU_MATCH g_pdu_match_cfg[DCM_PDU_PHY_NUM] = 
{
    /* first physical addressing */
    {
        .phy_hrh        = CANIF_HRH0_PHY,
        .func_hrh       = (uint08*)s_phy_hrh_cfg[0],
        .phy_hth        = CANIF_HTH0_PHY,
    },
};
const stCANIF_CFG g_canif_cfg_tbl[CANIF_HOH_NUM] = 
{
    {
        .chn		= CAN_CHN_0,
        .hoh            = CANIF_HRH0_PHY,
        .dir            = CANIF_RX,
        .canid          = CAN_PHY_RX_ID,
        .dlc            = CAN_DATA_LENGTH
    },
    {
        .chn		= CAN_CHN_0,
        .hoh            = CANIF_HRH0_FUNC,
        .dir            = CANIF_RX,
        .canid          = CAN_FUNC_RX_ID,
        .dlc            = CAN_DATA_LENGTH
    },
//    {
//        .chn		= CAN_CHN_0,
//        .hoh            = CANIF_HRH0_COM_0CFE6C27,
//        .dir            = CANIF_RX,
//        .canid          = CAN_COM_0CFE6C27_RX_ID,
//        .dlc            = CAN_DATA_LENGTH
//    },
//    {
//        .chn		= CAN_CHN_1,
//        .hoh            = CANIF_HTH0_PHY,
//        .dir            = CANIF_TX,
//        .canid          = CAN_PHY_TX_ID,
//        .dlc            = CAN_DATA_LENGTH
//    },
//    {
//        .chn			= CAN_CHN_0,
//        .hoh            = CANIF_HTH0_J1939_DM1,
//        .dir            = CANIF_TX,
//        .canid          = J1939_DM1_ID,
//        .dlc            = CAN_DATA_LENGTH
//    },
//    {
//        .chn			= CAN_CHN_0,
//        .hoh            = CANIF_HTH0_J1939_TPCM_BAM,
//        .dir            = CANIF_TX,
//        .canid          = J1939_TPCM_BAM_ID,
//        .dlc            = CAN_DATA_LENGTH
//    },
    {
        .chn			= CAN_CHN_0,
        .hoh            = CANIF_HTH0_PHY,
        .dir            = CANIF_TX,
        .canid          = CAN_PHY_TX_ID,
        .dlc            = CAN_DATA_LENGTH
    },
};
        
