#ifndef XCP_PORT_H
#define XCP_PORT_H

#include <stdint.h>
#include "xcp_cfg.h"

#define XCP_CAN_TX_BUFFER_INDEX 31u

void XcpPort_Init(void);
void XcpPort_MainFunction(void);
void XcpPort_RxIndication(const uint8_t *data, uint8_t length);
void XcpPort_TxConfirmation(void);
void XcpPort_ResetIndication(void);

#endif
