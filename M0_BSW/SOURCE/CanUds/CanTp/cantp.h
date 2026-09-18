/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : cantp.h
* Author        : yangming
* Date          : 2024-01-23
* Version       : 1.00
* Description   : Can transport protocol header file.
* Others        : None
*
****************************************************************************************************/
#ifndef _CANTP_H
#define _CANTP_H
#include "cantp_cfg.h"

#define CANTP_MF_RCV_OVER_WAIT_TIME                     50U

extern void cantp_init(void);
extern void cantp_mainfunction(void);
extern void cantp_period_1ms_process(void);
extern void cantp_rx_indication(PDU_ID hrh, stPDU_INFO* pdu);
extern void cantp_transmit(PDU_ID hth, stPDU_INFO* pdu);
extern void cantp_tx_confirmation(PDU_ID hth);
extern void cantp_tx_confirmation_manage(PDU_ID hth);
#endif /* _CANTP_H */

