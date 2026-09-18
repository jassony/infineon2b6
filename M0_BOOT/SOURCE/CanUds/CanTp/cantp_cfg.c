/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : cantp_cfg.c
* Author        : yangming
* Date          : 2024-01-23
* Version       : 1.00
* Description   : Can transport protocol configuration file.
* Others        : None
*
****************************************************************************************************/
#include "cantp_cfg.h"

static const stCANTP_RX_CFG s_cantp_rx_cfg[CANTP_PDU_RX_NUM] = 
{
    {
        .hrh                    = CANIF_HRH0_PHY,
        .hth                    = CANIF_HTH0_PHY,
        .n_ar                   = 20U,
        .n_br                   = 50U,
        .n_cr                   = 150U,
        .bs                     = 0U,
        .stmin                  = 150U
    },
    
    {
        .hrh                    = CANIF_HRH0_FUNC,
        .hth                    = CANIF_HTH0_PHY,
        .n_ar                   = 20U,
        .n_br                   = 50U,
        .n_cr                   = 150U,
        .bs                     = 0U,
        .stmin                  = 150U
    },
};

static const stCANTP_TX_CFG s_cantp_tx_cfg[CANTP_PDU_TX_NUM] = 
{
    {
        .hth                    = CANIF_HTH0_PHY,
        .n_as                   = 20U,
        .n_bs                   = 75U,
        .n_cs                   = 150U
    },
};

const stCANTP_CFG g_cantp_cfg = 
{
    .rx_cfg                     = (stCANTP_RX_CFG*)s_cantp_rx_cfg,
    .tx_cfg                     = (stCANTP_TX_CFG*)s_cantp_tx_cfg
};

