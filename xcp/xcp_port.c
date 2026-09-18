#include "xcp_port.h"

#include "XcpProf.h"
#include "xcp_cal_test.h"
#include "cy_canfd.h"
#include "cy_project.h"

#define XCP_CAN_INSTANCE CY_CANFD0_TYPE
#define XCP_CAN_FRAME_LENGTH 8u

static volatile vuint8 xcpRxPending;
static volatile vuint8 xcpTxPending;
static volatile vuint8 xcpResetPending;
static vuint8 xcpCriticalDepth;
static uint32_t xcpCriticalState;
static vuint32 xcpMainTick;

void XcpPort_EnterCritical(void)
{
    uint32_t state = Cy_SysLib_EnterCriticalSection();

    if (xcpCriticalDepth == 0u)
    {
        xcpCriticalState = state;
    }
    if (xcpCriticalDepth < 0xFFu)
    {
        xcpCriticalDepth++;
    }
}

void XcpPort_ExitCritical(void)
{
    if (xcpCriticalDepth > 0u)
    {
        xcpCriticalDepth--;
        if (xcpCriticalDepth == 0u)
        {
            Cy_SysLib_ExitCriticalSection(xcpCriticalState);
        }
    }
}

void XcpPort_Init(void)
{
    vuint8 i;
    uint32_t state = Cy_SysLib_EnterCriticalSection();

    xcpRxPending = 0u;
    xcpTxPending = 0u;
    xcpResetPending = 0u;
    xcpCriticalDepth = 0u;
    xcpCriticalState = 0u;
    xcpMainTick = 0u;
    Xcp_message_RXD.msgId = XCP_CANMSG_RXID;
    Xcp_message_TXD.msgId = XCP_CANMSG_TXID;
    for (i = 0u; i < XCP_CAN_FRAME_LENGTH; i++)
    {
        Xcp_message_RXD.data[i] = 0u;
        Xcp_message_TXD.data[i] = 0u;
    }

    Cy_SysLib_ExitCriticalSection(state);
    XcpInit();
}

void XcpPort_RxIndication(const uint8_t *data, uint8_t length)
{
    vuint8 i;

    if ((data == NULL) || (length == 0u) || (length > XCP_CAN_FRAME_LENGTH))
    {
        return;
    }

    if (xcpRxPending != 0u)
    {
        return;
    }

    for (i = 0u; i < length; i++)
    {
        Xcp_message_RXD.data[i] = data[i];
    }
    for (; i < XCP_CAN_FRAME_LENGTH; i++)
    {
        Xcp_message_RXD.data[i] = 0u;
    }
    xcpRxPending = 1u;
}

void XcpPort_ResetIndication(void)
{
    xcpResetPending = 1u;
}

vuint8 XcpCanIf_Transmit(void)
{
    cy_en_canfd_status_t result;
    cy_en_canfd_tx_buffer_status_t status;
    cy_stc_canfd_msg_t message = {0};

    XcpPort_EnterCritical();
    status = Cy_CANFD_GetTxBufferStatus(XCP_CAN_INSTANCE, XCP_CAN_TX_BUFFER_INDEX);
    if ((xcpTxPending != 0u) ||
        (status == CY_CANFD_TX_BUFFER_PENDING) ||
        (status == CY_CANFD_TX_BUFFER_CANCEL_REQUESTED))
    {
        XcpPort_ExitCritical();
        return (vuint8)kCanTxNOk;
    }

    message.canFDFormat = false;
    message.idConfig.extended = true;
    message.idConfig.identifier = XCP_CANMSG_TXID;
    message.dataConfig.dataLengthCode = XCP_CAN_FRAME_LENGTH;
    XcpMemCpy((DAQBYTEPTR)message.dataConfig.data,
              (const DAQBYTEPTR)Xcp_message_TXD.data,
              XCP_CAN_FRAME_LENGTH);

    result = Cy_CANFD_UpdateAndTransmitMsgBuffer(XCP_CAN_INSTANCE,
                                                 XCP_CAN_TX_BUFFER_INDEX,
                                                 &message);
    if (result == CY_CANFD_SUCCESS)
    {
        xcpTxPending = 1u;
        XcpPort_ExitCritical();
        return (vuint8)kCanTxOk;
    }

    XcpPort_ExitCritical();
    return (vuint8)kCanTxNOk;
}

void XcpPort_TxConfirmation(void)
{
    cy_en_canfd_tx_buffer_status_t status;
    vuint8 action = 0u;
    uint32_t state;

    state = Cy_SysLib_EnterCriticalSection();
    if (xcpTxPending == 0u)
    {
        Cy_SysLib_ExitCriticalSection(state);
        return;
    }

    status = Cy_CANFD_GetTxBufferStatus(XCP_CAN_INSTANCE, XCP_CAN_TX_BUFFER_INDEX);
    if (status == CY_CANFD_TX_BUFFER_TRANSMIT_OCCURRED)
    {
        xcpTxPending = 0u;
        action = 1u;
    }
    else if (status == CY_CANFD_TX_BUFFER_CANCEL_FINISHED)
    {
        xcpTxPending = 0u;
        action = 2u;
    }
    else
    {
    }
    Cy_SysLib_ExitCriticalSection(state);

    if (action == 1u)
    {
        XcpConfirmation();
    }
    else if (action == 2u)
    {
        XcpPort_ResetIndication();
    }
    else
    {
    }
}

void XcpPort_MainFunction(void)
{
    XcpCanMessageType command;
    vuint8 commandPending = 0u;
    vuint8 resetPending = 0u;
    vuint8 i;
    uint32_t state;

    state = Cy_SysLib_EnterCriticalSection();
    if (xcpResetPending != 0u)
    {
        xcpResetPending = 0u;
        xcpRxPending = 0u;
        xcpTxPending = 0u;
        xcpCriticalDepth = 0u;
        xcpCriticalState = 0u;
        xcpMainTick = 0u;
        resetPending = 1u;
    }
    Cy_SysLib_ExitCriticalSection(state);

    if (resetPending != 0u)
    {
        XcpInit();
        return;
    }

    XcpPort_TxConfirmation();

    state = Cy_SysLib_EnterCriticalSection();
    if (xcpRxPending != 0u)
    {
        command.msgId = Xcp_message_RXD.msgId;
        for (i = 0u; i < XCP_CAN_FRAME_LENGTH; i++)
        {
            command.data[i] = Xcp_message_RXD.data[i];
        }
        xcpRxPending = 0u;
        commandPending = 1u;
    }
    Cy_SysLib_ExitCriticalSection(state);

    if (commandPending != 0u)
    {
        (void)XcpPreCopy(&command);
    }

    (void)XcpBackground();
    XcpCalM0Apply();

    xcpMainTick++;
    (void)XcpEvent(XcpEventChannel_1ms);
    if ((xcpMainTick % 2u) == 0u)
    {
        (void)XcpEvent(XcpEventChannel_2ms);
    }
    if ((xcpMainTick % 10u) == 0u)
    {
        (void)XcpEvent(XcpEventChannel_10ms);
    }
    if (xcpMainTick >= 100u)
    {
        xcpMainTick = 0u;
        (void)XcpEvent(XcpEventChannel_100ms);
    }
}
