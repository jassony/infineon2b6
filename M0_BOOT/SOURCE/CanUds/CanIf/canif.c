/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : canif.c
* Author        : yangming
* Date          : 2024-01-23
* Version       : 1.00
* Description   : Can interface file.
* Others        : None
*
****************************************************************************************************/
#include "canif.h"
#include "cantp.h"
#include "can_driver.h"

/****************************************************************************************************
* Function Name : canif_init
* Description   : module initialization.
* Argument      : void
* Return Value  : void
* Notes         : None
****************************************************************************************************/
void canif_init(void)
{

}

/****************************************************************************************************
* Function Name : canif_transmit
* Description   : can transmit request
* Argument      : hth: hardware transmit handle
                  pdu: pdu infomation.
* Return Value  : error status
* Notes         : None
****************************************************************************************************/
eERR_STS canif_transmit(PDU_ID hth, stPDU_INFO* pdu)
{
    stCANIF_CFG* cfg = (stCANIF_CFG*)g_canif_cfg_tbl;
    eERR_STS sts = E_NOK;

    if (   (DEF_NULL == pdu)
        || (DEF_NULL == pdu->sdu_data)
        || (pdu->sdu_len < CAN_DATA_LENGTH)
        || (hth >= CANIF_HOH_NUM)
        || (hth < CANIF_HRH_NUM)
        )
    {
        return E_NOK;
    }

    Can_Transmit(CY_CANFD0_TYPE, cfg[hth].canid, cfg[hth].dlc, pdu->sdu_data,false);
    return E_OK;
}

/****************************************************************************************************
* Function Name : canif_tx_confirm
* Description   : callback function, send confirmation
* Argument      : hth: hardware transmit handle
                  pdu: pdu infomation.
* Return Value  : void
* Notes         : None
****************************************************************************************************/
void canif_tx_confirm(PDU_ID hth)
{
    cantp_tx_confirmation(hth);
}

/****************************************************************************************************
* Function Name : canif_rx_indication
* Description   : callback function, receive indication
* Argument      : hrh: hardware receive handle
                  pdu: pdu infomation.
* Return Value  : void
* Notes         : None
****************************************************************************************************/
void canif_rx_indication(PDU_ID hrh, uint32 canid, uint08 dlc, uint08* sdu)
{
    stPDU_INFO pdu;
    
    if (   (DEF_NULL == sdu)
        || (hrh >= CANIF_HRH_NUM)
        || (canid > CAN_INVALID_ID)
        || (dlc > CAN_DATA_LENGTH)
        )
    {
        return;
    }

    pdu.sdu_data = sdu;
    pdu.sdu_len = dlc;
    cantp_rx_indication(hrh, &pdu);
}

uint08 get_phy_pdu_through_hrh(PDU_ID hrh)
{
    uint08 i = 0;
    uint08 j = 0;

    for (i = 0; i < DCM_PDU_PHY_NUM; i++)
    {
        for (j = 0; j < DCM_PDU_FUNC_MAX; j++)
        {
            if ((hrh == g_pdu_match_cfg[i].func_hrh[j]) || (hrh == g_pdu_match_cfg[i].phy_hrh))
            {
                return i;
            }
        }
    }

    if (i < DCM_PDU_PHY_NUM)
    {
        return i;
    }
    else
    {
        return 0xFF;
    }
}
uint08 get_phy_pdu_through_hrh_only_func(PDU_ID hrh)
{
    uint08 i = 0;
    uint08 j = 0;

    for (i = 0; i < DCM_PDU_PHY_NUM; i++)
    {
        for (j = 0; j < DCM_PDU_FUNC_MAX; j++)
        {
            if (hrh == g_pdu_match_cfg[i].func_hrh[j])
            {
                return i;
            }
        }
    }

    if (i < DCM_PDU_PHY_NUM)
    {
        return i;
    }
    else
    {
        return 0xFF;
    }
}

