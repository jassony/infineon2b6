/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : canif.h
* Author        : yangming
* Date          : 2024-01-23
* Version       : 1.00
* Description   : Can interface file.
* Others        : None
*
****************************************************************************************************/
#ifndef _CANIF_H
#define _CANIF_H
#include "canif_cfg.h"

extern void canif_init(void);
extern eERR_STS canif_transmit(PDU_ID hth, stPDU_INFO* pdu);
extern void canif_tx_confirm(PDU_ID hth);
extern void canif_rx_indication(PDU_ID hrh, uint32 canid, uint08 dlc, uint08* sdu);
extern void cantp_tx_confirmation(PDU_ID hth);
extern uint08 get_phy_pdu_through_hrh(PDU_ID hrh);
extern uint08 get_phy_pdu_through_hrh_only_func(PDU_ID hrh);
#endif /* _CANIF_H */
