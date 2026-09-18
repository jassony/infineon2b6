/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : cantp.c
* Author        : yangming
* Date          : 2024-09-11
* Version       : 1.00 | 2024-01-30 | First creation.
                  2.00 | 2024-09-11 | Restart the architecture, add time parameter handling and 
                  error/unexpected handling according to protocol 15765-2.
* Description   : Can transport protocol file.
* Others        : None
*
****************************************************************************************************/
#include "cantp.h"
#include "can_dcm_cbk_cfg.h"
#include "soft_timer.h"

static uint08 s_cantp_rx_buf[CAN_DATA_LENGTH];
static uint08 s_cantp_hrh_buf[CANTP_PDU_RX_NUM][CAN_DATA_LENGTH];
static stCANTP_RX_CONTEXT s_cantp_rx_context[CANTP_PDU_RX_NUM];
static uint08 s_cantp_tx_buf[CAN_DATA_LENGTH];
static uint08 s_cantp_hth_buf[CANTP_PDU_RX_NUM][CAN_DATA_LENGTH];
static stCANTP_TX_CONTEXT s_cantp_tx_context[CANTP_PDU_TX_NUM];
static stSOFT_TIMER s_mf_rcv_timer;

static void cantp_rx_process(void);
static void cantp_tx_process(void);
static void cantp_rx_sf(PDU_ID hrh, uint08* sdu_data);
static void cantp_rx_ff(PDU_ID hrh, uint08* sdu_data);
static void cantp_rx_cf(PDU_ID hrh, uint08* sdu_data);
static void cantp_rx_fc(PDU_ID hrh, uint08* sdu_data);
static void cantp_tx_fc(PDU_ID hth);
static void cantp_tx_sf(PDU_ID hth, stPDU_INFO* pdu);
static void cantp_tx_ff(PDU_ID hth, stPDU_INFO* pdu);
static void cantp_tx_cf(PDU_ID hth);
static void cantp_rx_data_init(PDU_ID hrh);
static void cantp_tx_data_init(PDU_ID hth);

void cantp_init(void)
{    
    PDU_ID hrh = 0;
    PDU_ID hth = 0;
    
    common_memset((uint08*)s_cantp_rx_buf, 0U, sizeof(s_cantp_rx_buf));
    common_memset((uint08*)s_cantp_hrh_buf, 0U, sizeof(s_cantp_hrh_buf));
    common_memset((uint08*)s_cantp_tx_buf, 0U, sizeof(s_cantp_tx_buf));
    common_memset((uint08*)s_cantp_rx_context, 0U, sizeof(s_cantp_rx_context));
    for (hrh = 0; hrh < CANTP_PDU_RX_NUM; hrh++)
    {
        cantp_rx_data_init(hrh);
    }
    
    common_memset((uint08*)s_cantp_tx_context, 0U, sizeof(s_cantp_tx_context));
    for (hth = 0; hth < CANTP_PDU_TX_NUM; hth++)
    {
        cantp_tx_data_init(hth);
    }

    soft_timer_set(&s_mf_rcv_timer, CANTP_MF_RCV_OVER_WAIT_TIME);
}

void cantp_mainfunction(void)
{
    cantp_rx_process();
    cantp_tx_process();
}

void cantp_period_1ms_process(void)
{    
    PDU_ID hrh = 0;
    PDU_ID hth = 0;

    for (hrh = 0; hrh < CANTP_PDU_RX_NUM; hrh++)
    {
        if (s_cantp_rx_context[hrh].n_ar)
        {
            s_cantp_rx_context[hrh].n_ar--;
        }
        else {}
        
        if (s_cantp_rx_context[hrh].n_br)
        {
            s_cantp_rx_context[hrh].n_br--;
        }
        
        if (s_cantp_rx_context[hrh].n_cr)
        {
            s_cantp_rx_context[hrh].n_cr--;
        }
        else {}
    }

    for (hth = 0; hth < CANTP_PDU_TX_NUM; hth++)
    {
        cantp_tx_confirmation_manage(hth);
        
        if (s_cantp_tx_context[hth].n_as)
        {
            s_cantp_tx_context[hth].n_as--;
        }
        else {}
        
        if (s_cantp_tx_context[hth].n_bs)
        {
            s_cantp_tx_context[hth].n_bs--;
        }
        else {}
        
        if (s_cantp_tx_context[hth].n_cs)
        {
            s_cantp_tx_context[hth].n_cs--;
        }
        else {}

        if (   (CANTP_TX_SENDER_CF_CON_CONTINUE == s_cantp_tx_context[hth].sts)
//            || (CANTP_TX_SENDER_CF_REQ == s_cantp_tx_context[hth].sts)
            || (CANTP_TX_SENDER_CF_CON_CONTINUE == s_cantp_tx_context[hth].sts)
            )
        {
            if (s_cantp_tx_context[hth].stmin_cnt)
            {
                s_cantp_tx_context[hth].stmin_cnt--;
            }
            else {}
        }
    }

    cantp_mainfunction();
}

void cantp_rx_indication(PDU_ID hrh, stPDU_INFO* pdu)
{
    eFRAME_TYPE ft = FRAME_TYPE_INVALID;

    if (DEF_NULL == pdu)
    {
        return;
    }
    
    ft = ((pdu->sdu_data[0] & 0xF0) >> 4U);
    if (ft >= FRAME_TYPE_INVALID)
    {
        ft = FRAME_TYPE_INVALID;
    }
    else {}
    s_cantp_rx_context[hrh].rcv_ft  = ft;
    s_cantp_rx_context[hrh].can_dl = pdu->sdu_len;
    
    if (FRAME_TYPE_INVALID != s_cantp_rx_context[hrh].rcv_ft)
    {
        common_memcpy((uint08*)s_cantp_hrh_buf[hrh], (uint08*)&pdu->sdu_data[0], pdu->sdu_len);
    }
    else
    {
        /* reserved, ignore */
    }

    /* Check whether a PDU is received. */
    if (SINGLE_FRAME == s_cantp_rx_context[hrh].rcv_ft)
    {
        cantp_rx_sf(hrh, (uint08*)s_cantp_hrh_buf[hrh]);
    }
    else if (FIRST_FRAME == s_cantp_rx_context[hrh].rcv_ft)
    {
        cantp_rx_ff(hrh, (uint08*)s_cantp_hrh_buf[hrh]);
    }
    else if (CONSECUTIVE_FRAME == s_cantp_rx_context[hrh].rcv_ft)
    {
        cantp_rx_cf(hrh, (uint08*)s_cantp_hrh_buf[hrh]);
    }
    else if (FLOW_CONTROL == s_cantp_rx_context[hrh].rcv_ft)
    {
        cantp_rx_fc(hrh, (uint08*)s_cantp_hrh_buf[hrh]);
    }
    else {}
    s_cantp_rx_context[hrh].rcv_ft = FRAME_TYPE_INVALID; /* Clear PDU receive flag. */
}

void cantp_transmit(PDU_ID hth, stPDU_INFO* pdu)
{
    eCANTP_TX_STS tx_sts = cantp_TX_INIT;
    
    if (DEF_NULL == pdu)
    {
        return;
    }

    if (pdu->sdu_len > N_DATA_SF_LEN)
    {
        tx_sts = CANTP_TX_SENDER_FF_REQ;
    }
    else
    {
        tx_sts = CANTP_TX_SENDER_SF_REQ;
    }

//    s_cantp_tx_context[hth - CANIF_HRH_NUM].sts = tx_sts;

    switch (tx_sts)
    {
        case CANTP_TX_SENDER_SF_REQ:
            cantp_tx_sf(hth, pdu);
            break;
        case CANTP_TX_SENDER_FF_REQ:
            cantp_tx_ff(hth, pdu);
            break;
        default:
            break;
    }
}

void cantp_tx_confirmation(PDU_ID hth)
{
    hth -= CANIF_HRH_NUM;
    s_cantp_tx_context[hth].confirmation = E_OK;
}

void cantp_tx_confirmation_manage(PDU_ID hth)
{
    PDU_ID hrh = 0;
    uint08 phy_index = 0xFF;

    if (s_cantp_tx_context[hth].confirmation != E_OK)
    {
        s_cantp_tx_context[hth].confirmation = E_OK; /* Special processing: Once sent, it is considered successful by default!!! */
        return;
    }
    
    hrh = s_cantp_tx_context[hth].hrh;
    phy_index = get_phy_pdu_through_hrh_only_func(hrh);
    if (CANTP_TX_SENDER_SF_REQ == s_cantp_tx_context[hth].sts)
    {
        s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_SF_CON;
    }
    else if (CANTP_TX_SENDER_FF_REQ == s_cantp_tx_context[hth].sts)
    {
        /* process in cantp_tx_process */
    }
    else if (CANTP_TX_SENDER_CF_CON_WAIT == s_cantp_tx_context[hth].sts)
    {
        if (s_cantp_tx_context[hth].mf_over)
        {
            dcm_tx_confirmation(hth, DEF_NULL);
            s_cantp_tx_context[hth].sts = cantp_TX_INIT;
            cantp_tx_data_init(hth);
            CAN_ISR_DEBUG_PRINT("[CANTP-CONF]CF:PDU Send over, HTH = %d. \r\n", hth);
        }
        else
        {
            if ((s_cantp_tx_context[hth].bs) && (0 == s_cantp_tx_context[hth].bs_cnt))
            {
                if (cantp_RX_INIT == s_cantp_rx_context[hrh].sts)
                {
                    cantp_rx_data_init(hrh);
                }
                else {}
                if (CANTP_RX_IDLE == s_cantp_rx_context[hrh].sts)
                {
                    s_cantp_tx_context[hth].n_bs = g_cantp_cfg.tx_cfg[hth].n_bs;
                    s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_CF_CON;
                    s_cantp_rx_context[hrh].sts = CANTP_RX_SENDER_FC_IND_WAIT;
                    s_cantp_rx_context[hrh].fs = N_PCI_FS_INVALID;
                    s_cantp_rx_context[hrh].bs = 0;
                    s_cantp_rx_context[hrh].stmin = 0;
                    CAN_ISR_DEBUG_PRINT("[CANTP-CONF]The CF was successfully sent, ready to send FC, HTH = %d. \r\n", hth);
                }
                else
                {
                    if (s_cantp_rx_context[hrh].sts_dly_cnt) /* waiting status switch */
                    {
                        s_cantp_rx_context[hrh].sts_dly_cnt--;
                    }
                    else
                    {
                        s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_CF_ERR;
                        s_cantp_tx_context[hth].result = CANTP_N_ERROR;
                        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-CONF]Rx busy 2, HTH = %d! \r\n", hth);
                    }
                }
            }
            else
            {
                s_cantp_tx_context[hth].n_cs = g_cantp_cfg.tx_cfg[hth].n_cs;
                s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_CF_CON_CONTINUE;
                CAN_ISR_DEBUG_PRINT("[CANTP-CONF]The CF was successfully sent, HTH = %d. \r\n", hth);
            }
        }
    }
    else {}
}

static void cantp_rx_process(void)
{
    PDU_ID hrh = 0;
    PDU_ID hth = 0;
    eERR_STS err_sts = E_OK;
    
    for (hrh = 0; hrh < CANTP_PDU_RX_NUM; hrh++)
    {
        hth = s_cantp_rx_context[hrh].hth;

        /* Check whether a PDU is received. */
//        if (SINGLE_FRAME == s_cantp_rx_context[hrh].rcv_ft)
//        {
//            cantp_rx_sf(hrh, (uint08*)s_cantp_hrh_buf[hrh]);
//        }
//        else if (FIRST_FRAME == s_cantp_rx_context[hrh].rcv_ft)
//        {
//            cantp_rx_ff(hrh, (uint08*)s_cantp_hrh_buf[hrh]);
//        }
//        else if (CONSECUTIVE_FRAME == s_cantp_rx_context[hrh].rcv_ft)
//        {
//            cantp_rx_cf(hrh, (uint08*)s_cantp_hrh_buf[hrh]);
//        }
//        else if (FLOW_CONTROL == s_cantp_rx_context[hrh].rcv_ft)
//        {
//            cantp_rx_fc(hrh, (uint08*)s_cantp_hrh_buf[hrh]);
//        }
//        else {}
//        s_cantp_rx_context[hrh].rcv_ft = FRAME_TYPE_INVALID; /* Clear PDU receive flag. */
        
        switch (s_cantp_rx_context[hrh].sts)
        {
            case cantp_RX_INIT:
                cantp_rx_data_init(hrh);
                break;
            case CANTP_RX_IDLE:
                break;
            case CANTP_RX_RECEIVER_SF_IND:
                s_cantp_rx_context[hrh].sts = cantp_RX_INIT;
                cantp_rx_data_init(hrh);
                break;
            case CANTP_RX_RECEIVER_FF_IND:
                if (0U == s_cantp_rx_context[hrh].n_br)
                {
                    s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_FF_ERR;
                    s_cantp_rx_context[hrh].result = CANTP_N_TIMEOUT_BR;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN-R]N_Br timeout, HRH = %d! \r\n", hrh);
                }
                else
                {
                    if (CANTP_TX_IDLE == s_cantp_tx_context[hth].sts)
                    {
                        s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_FC_REQ;
                        s_cantp_tx_context[hth].sts = CANTP_TX_RECEIVER_FC_REQ;
                        s_cantp_tx_context[hth].confirmation = E_BUSY;
                        s_cantp_tx_context[hth].hrh = hrh;
                    }
                    else
                    {
                        if (s_cantp_tx_context[hth].sts_dly_cnt) /* waiting status switch */
                        {
                            s_cantp_tx_context[hth].sts_dly_cnt--;
                        }
                        else
                        {
                            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_FF_ERR;
                            s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
                            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN-R]Tx busy 1, HRH = %d! \r\n", hrh);
                        }
                    }
                }
                break;
            case CANTP_RX_RECEIVER_CF_IND:
                /* Receive CF PDU continue */
                if (0 == s_cantp_rx_context[hrh].n_cr)
                {
                    s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_ERR;
                    s_cantp_tx_context[hrh].result = CANTP_N_TIMEOUT_CR;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN-R]N_Cr timeout, HRH = %d! \r\n", hrh);
                }
                else {}
                break;
            case CANTP_RX_RECEIVER_CF_IND_OVER:
                s_cantp_rx_context[hrh].sts = cantp_RX_INIT;
                cantp_rx_data_init(hrh);
                if (CANTP_TX_RECEIVER_CF_WAIT == s_cantp_tx_context[hrh].sts)
                {
                    s_cantp_tx_context[hrh].sts = CANTP_TX_RECEIVER_CF_END;
                    soft_timer_set(&s_mf_rcv_timer, CANTP_MF_RCV_OVER_WAIT_TIME);
                }
                else {}
                break;
            case CANTP_RX_RECEIVER_FC_REQ:
                err_sts = s_cantp_tx_context[hth].confirmation;
                if (E_OK == err_sts)
                {
                    s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_FC_CON;
                    CAN_ISR_DEBUG_PRINT("[CANTP-MAIN-R]The flow control frame was successfully sent, HRH = %d. \r\n", hrh);
                    s_cantp_rx_context[hrh].n_cr = g_cantp_cfg.rx_cfg[hrh].n_cr;
                }
                else if (0U == s_cantp_rx_context[hrh].n_ar)
                {
                    s_cantp_rx_context[hrh].sts = CANTP_RX_FC_CON_ERR;
                    s_cantp_rx_context[hrh].result = CANTP_N_TIMEOUT_A;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN-R]N_Ar timeout, HRH = %d! \r\n", hrh);
                }
                else { /* wait */ }
                break;
            case CANTP_RX_SENDER_FC_IND_WAIT:
                /* Wait to receive the FC frame. */
                break;
            case CANTP_RX_SENDER_FC_IND:
                if (N_PCI_FS_CTS == s_cantp_rx_context[hrh].fs)
                {
                    s_cantp_rx_context[hrh].sts = cantp_RX_INIT;
                    cantp_rx_data_init(hrh);
                    if ((CANTP_TX_SENDER_FF_CON == s_cantp_tx_context[hth].sts) || (CANTP_TX_SENDER_CF_CON == s_cantp_tx_context[hth].sts))
                    {
                        s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_FC_IND;
                        CAN_ISR_DEBUG_PRINT("[CANTP-MAIN-R]The flow control frame was successfully received, HTH = %d. \r\n", hth);
                    }
                    else if ((CANTP_TX_SENDER_FF_REQ == s_cantp_tx_context[hth].sts) || (CANTP_TX_SENDER_CF_REQ == s_cantp_tx_context[hth].sts))
                    {
                        s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_FC_IND;
                        CAN_ISR_DEBUG_PRINT("[CANTP-MAIN-R]The flow control frame was successfully received, HTH = %d. But FF/CF may not be sent successfully. \r\n", hth);
                    }
                    else
                    {
                        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN-R]Tx busy 2, HTH = %d. \r\n", hth);
                    }
                }
                else if (N_PCI_FS_WT == s_cantp_rx_context[hrh].fs)
                {
                    /* wait */
                    s_cantp_rx_context[hrh].sts = CANTP_RX_SENDER_FC_IND_WAIT;
                    s_cantp_tx_context[hth].n_bs = g_cantp_cfg.tx_cfg[hth].n_bs;
                    CAN_ISR_DEBUG_PRINT_WRN("[CANTP-MAIN-R]The receiver requests wait, HTH = %d! \r\n", s_cantp_rx_context[hrh].hth);
                }
                else if (N_PCI_FS_OVFLW == s_cantp_rx_context[hrh].fs)
                {
                    s_cantp_rx_context[hrh].sts = cantp_RX_INIT;
                    cantp_rx_data_init(hrh);
                    s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_FC_ERR;
                    s_cantp_tx_context[hth].result = CANTP_N_BUFFER_OVFLW;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN-R]The receiver reported a buffer overflow, HTH = %d! \r\n", s_cantp_rx_context[hrh].hth);
                }
                else
                {
                    s_cantp_rx_context[hrh].sts = cantp_RX_INIT;
                    cantp_rx_data_init(hrh);
                    s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_FC_ERR;
                    s_cantp_tx_context[hth].result = CANTP_N_INVALID_FS;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN-R]Received error FS, HTH = %d! \r\n", s_cantp_rx_context[hrh].hth);
                }                
                break;
            case CANTP_RX_RECEIVER_FC_CON:
                if (0 == s_cantp_rx_context[hrh].n_cr)
                {
                    s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_ERR;
                    s_cantp_tx_context[hrh].result = CANTP_N_TIMEOUT_CR;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN-R]N_Cr timeout, HRH = %d! \r\n", hrh);
                }
                else {}
                break;
            case CANTP_RX_RECEIVER_SF_ERR:
            case CANTP_RX_RECEIVER_FF_ERR:
            case CANTP_RX_RECEIVER_CF_ERR:
            case CANTP_RX_FC_CON_ERR:
                err_sts = E_NOK;
                if (   (CANTP_TX_RECEIVER_CF_WAIT == s_cantp_tx_context[hth].sts)
                    && (CANTP_RX_RECEIVER_CF_ERR == s_cantp_rx_context[hrh].sts)
                    )
                {
                    cantp_tx_data_init(hth); /* If CF receives an error, s_cantp_tx_context[hth].sts is reset */
                }
                else {}
                s_cantp_rx_context[hrh].sts = cantp_RX_INIT;
                cantp_rx_data_init(hrh);
                dcm_rx_indication(hrh, &err_sts);
                break;
            default:
                break;
        }
    }
}

static void cantp_tx_process(void)
{
    PDU_ID hrh = 0;
    PDU_ID hth = 0;
    eERR_STS err_sts = E_OK;
    uint08 phy_index = 0xFF;
    
    for (hth = 0; hth < CANTP_PDU_TX_NUM; hth++)
    {
        hrh = s_cantp_tx_context[hth].hrh;
        phy_index = get_phy_pdu_through_hrh_only_func(hrh);
        switch (s_cantp_tx_context[hth].sts)
        {
            case cantp_TX_INIT:
                cantp_tx_data_init(hth);
                break;
            case CANTP_TX_IDLE:
                break;
            case CANTP_TX_SENDER_SF_REQ:
                if (0U == s_cantp_tx_context[hth].n_as)
                {
                    s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_SF_ERR;
                    s_cantp_tx_context[hth].result = CANTP_N_TIMEOUT_AS;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN]SF:N_As timeout, HTH = %d! \r\n", hth);
                }
                else { /* wait */ }
                break;
            case CANTP_TX_SENDER_SF_CON:
                CAN_ISR_DEBUG_PRINT("[CANTP-MAIN]The SF was successfully sent, HTH = %d. \r\n", hth);
                s_cantp_tx_context[hth].sts = cantp_TX_INIT;
                cantp_tx_data_init(hth);
                dcm_tx_confirmation(hth, DEF_NULL);
                break;
            case CANTP_TX_SENDER_FF_REQ:
                err_sts = s_cantp_tx_context[hth].confirmation;
                if (E_OK == err_sts)
                {
                    if (cantp_RX_INIT == s_cantp_rx_context[hrh].sts)
                    {
                        cantp_rx_data_init(hrh);
                    }
                    else {}
                    
                    if (CANTP_RX_IDLE == s_cantp_rx_context[hrh].sts)
                    {
                        if ((phy_index != 0xFF) && (CANTP_RX_IDLE == s_cantp_rx_context[g_pdu_match_cfg[phy_index].phy_hrh].sts))
                        {
                            s_cantp_rx_context[g_pdu_match_cfg[phy_index].phy_hrh].sts = CANTP_RX_SENDER_FC_IND_WAIT;
                            s_cantp_rx_context[g_pdu_match_cfg[phy_index].phy_hrh].fs = N_PCI_FS_INVALID;
                            s_cantp_rx_context[g_pdu_match_cfg[phy_index].phy_hrh].bs = 0;
                            s_cantp_rx_context[g_pdu_match_cfg[phy_index].phy_hrh].stmin = 0;
                        }
                        else {}
                        
                        s_cantp_tx_context[hth].n_bs = g_cantp_cfg.tx_cfg[hth].n_bs;
                        s_cantp_rx_context[hrh].sts = CANTP_RX_SENDER_FC_IND_WAIT;
                        s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_FF_CON;
//                        s_cantp_rx_context[hrh].fs = N_PCI_FS_INVALID;
                        s_cantp_rx_context[hrh].bs = 0;
                        s_cantp_rx_context[hrh].stmin = 0;
                        CAN_ISR_DEBUG_PRINT("[CANTP-MAIN]The FF was successfully sent, HTH = %d. \r\n", hth);
                    }
                    else
                    {
                        if (s_cantp_rx_context[hrh].sts_dly_cnt) /* waiting status switch */
                        {
                            s_cantp_rx_context[hrh].sts_dly_cnt--;
                        }
                        else
                        {
                            s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_FF_ERR;
                            s_cantp_tx_context[hth].result = CANTP_N_ERROR;
                            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN]Rx busy 3, HTH = %d! \r\n", hth);
                        }
                    }
                }
                else if (0U == s_cantp_tx_context[hth].n_as)
                {
                    s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_FF_ERR;
                    s_cantp_tx_context[hth].result = CANTP_N_TIMEOUT_AS;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN]FF:N_As timeout, HTH = %d! \r\n", hth);
                }
                else { /* wait */ }
                break;
            case CANTP_TX_SENDER_FF_CON:
                if (0 == s_cantp_tx_context[hth].n_bs)
                {
                    s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_FF_ERR;
                    s_cantp_tx_context[hth].result = CANTP_N_TIMEOUT_BS;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN]FF was sent, but FC was not received, HTH = %d! \r\n", hth);
                }
                else { /* wait */ }
                break;
            case CANTP_TX_SENDER_CF_REQ:
                cantp_tx_cf(hth);
                break;
            case CANTP_TX_SENDER_CF_CON_WAIT:
                if (0 == s_cantp_tx_context[hth].n_as)
                {
                    s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_CF_ERR;
                    s_cantp_tx_context[hth].result = CANTP_N_TIMEOUT_AS;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN]CF:N_As timeout, HTH = %d! \r\n", hth);
                }
                else { /* wait */ }
                break;
            case CANTP_TX_SENDER_CF_CON_CONTINUE:
                if (0 == s_cantp_tx_context[hth].n_cs)
                {
                    s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_CF_ERR;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN]N_Cs timeout, HTH = %d! \r\n", hth);
                }
                else if (0 == s_cantp_tx_context[hth].stmin_cnt)
                {
                    s_cantp_tx_context[hth].stmin_cnt = s_cantp_tx_context[hth].stmin;
                    s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_CF_REQ;
                    cantp_tx_cf(hth);
                }
                else { /* wait */ }
                break;
            case CANTP_TX_SENDER_CF_CON:
                if (0 == s_cantp_tx_context[hth].n_bs)
                {
                    s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_CF_ERR;
                    s_cantp_tx_context[hth].result = CANTP_N_TIMEOUT_BS;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN]CF was sent, but FC was not received, HTH = %d! \r\n", hth);
                }
                else { /* wait */ }
                break;
            case CANTP_TX_SENDER_FC_IND:
//                s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_CF_REQ;
                cantp_tx_cf(hth);
                CAN_ISR_DEBUG_PRINT("[CANTP-MAIN]The flow control frame was received successfully, HTH = %d. \r\n", hth);
                break;
            case CANTP_TX_RECEIVER_FC_REQ:
                cantp_tx_fc(hth);
                if (N_PCI_FS_OVFLW == s_cantp_tx_context[hth].fs)
                {
                    s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_ERR;
                    s_cantp_rx_context[hrh].result = CANTP_N_BUFFER_OVFLW;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN]FC OVFLW:Receive buffer over flower, HRH = %d! \r\n", hrh);
                }
                else if (N_PCI_FS_WT == s_cantp_tx_context[hth].fs)
                {
                    s_cantp_tx_context[hth].wt_cnt++;
                    if (s_cantp_tx_context[hth].wt_cnt >= CANTP_N_WFTMAX)
                    {
                        s_cantp_tx_context[hth].wt_cnt = CANTP_N_WFTMAX;
                        s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_ERR;
                        s_cantp_rx_context[hrh].result = CANTP_NWFT_OVRN;
                        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-MAIN]Receiver request wait times exceeded, HRH = %d! \r\n", hrh);
                    }
                    else
                    {
                        CAN_ISR_DEBUG_PRINT_WRN("[CANTP-MAIN]The receiver requests wait, HRH = %d! \r\n", hrh);
                    }
                }
                else
                {
                    s_cantp_rx_context[hrh].n_ar = g_cantp_cfg.rx_cfg[hrh].n_ar;
                    CAN_ISR_DEBUG_PRINT("[CANTP-MAIN]Restart N_Ar, HTH = %d. \r\n", hth);
                }
                s_cantp_tx_context[hth].sts = CANTP_TX_RECEIVER_CF_WAIT;
                break;
            case CANTP_TX_RECEIVER_CF_WAIT:
                /* For timeout, see N_Cr */
                break;
            case CANTP_TX_RECEIVER_CF_END:
                if (DEF_TRUE == is_soft_timer_timeout(&s_mf_rcv_timer))
                {
                    dcm_tx_confirmation(hth, DEF_NULL);
                    cantp_tx_data_init(hth);
                }
                else {}
                break;
            case CANTP_TX_SENDER_SF_ERR:
            case CANTP_TX_SENDER_FF_ERR:
            case CANTP_TX_SENDER_CF_ERR:
            case CANTP_TX_SENDER_FC_ERR:
                s_cantp_tx_context[hth].sts = cantp_TX_INIT;
                dcm_tx_confirmation(hth, DEF_NULL);
                cantp_tx_data_init(hth);
                break;
            default:
                break;
        }
    }
}

static void cantp_rx_sf(PDU_ID hrh, uint08* sdu_data)
{
    uint32 buf_len = 0;
    eERR_STS err_sts = E_OK;
    stPDU_INFO temp_pdu;

    temp_pdu.sdu_len = (sdu_data[0] & 0x0F);
    temp_pdu.sdu_data = (uint08*)s_cantp_rx_buf;

    if (   (s_cantp_rx_context[hrh].can_dl < CANTP_PDU_EXPECTED_DLC) /* error CAN DLC */
        || (temp_pdu.sdu_len < CANTP_PDU_MIN_LEN) /* error PDU length */
        || (temp_pdu.sdu_len > N_DATA_SF_LEN) /* error PDU length */
        )
    {
        if (CANTP_RX_IDLE == s_cantp_rx_context[hrh].sts)
        {
            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_SF_ERR;
            s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
        }
        else { /* nothing */ }
        if (s_cantp_rx_context[hrh].can_dl < CANTP_PDU_EXPECTED_DLC)
        {
            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-SF-R]The CAN frame data length is incorrect, HRH = %d! \r\n", hrh);
        }
        else
        {
            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-SF-R]The PDU length is incorrect, HRH = %d! \r\n", hrh);
        }
        return;
    }
    else if (  (CANTP_TX_IDLE != s_cantp_tx_context[s_cantp_rx_context[hrh].hth].sts)
            && (cantp_TX_INIT != s_cantp_tx_context[s_cantp_rx_context[hrh].hth].sts)
            && (CANTP_TX_SENDER_SF_REQ != s_cantp_tx_context[s_cantp_rx_context[hrh].hth].sts)
            && (CANTP_TX_SENDER_SF_CON != s_cantp_tx_context[s_cantp_rx_context[hrh].hth].sts)
            && (CANTP_TX_RECEIVER_CF_WAIT != s_cantp_tx_context[s_cantp_rx_context[hrh].hth].sts)
            && (CANTP_RX_RECEIVER_SF_IND != s_cantp_tx_context[s_cantp_rx_context[hrh].hth].sts)
            )
    {
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-SF-R]SF received during sending, ignored, HRH = %d! \r\n", hrh);
        return;
    }
    else if ((s_cantp_rx_context[hrh].sts >= CANTP_RX_RECEIVER_FF_IND) && (s_cantp_rx_context[hrh].sts <= CANTP_RX_RECEIVER_FC_CON))
    {
        s_cantp_rx_context[hrh].result = CANTP_N_UNEXP_PDU;
        err_sts = E_NOK;
        dcm_rx_indication(hrh, &err_sts);
        cantp_tx_data_init(s_cantp_rx_context[hrh].hth);
        dcm_tx_confirmation(s_cantp_rx_context[hrh].hth, DEF_NULL);
        CAN_ISR_DEBUG_PRINT_WRN("[CANTP-SF-R]Segmented receive in progress received SF, HRH = %d! \r\n", hrh);
    }
    else { /* continue */ }

    if (E_OK == dcm_start_reception(hrh, temp_pdu.sdu_len, &buf_len))
    {
        common_memcpy((uint08*)temp_pdu.sdu_data, (uint08*)&sdu_data[1], temp_pdu.sdu_len);
        err_sts = dcm_copy_rx_data(hrh, &temp_pdu, (uint32*)&buf_len);
        if (E_OK == err_sts)
        {
            dcm_rx_indication(hrh, &err_sts);
            if (E_OK == err_sts)
            {
                s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_SF_IND;
                s_cantp_rx_context[hrh].sdu_len = temp_pdu.sdu_len;
                s_cantp_tx_context[s_cantp_rx_context[hrh].hth].hrh = hrh;
//                CAN_ISR_DEBUG_PRINT("[CANTP-SF-R]Receive PDU, HRH = %d, Length = %d. \r\n", hrh, s_cantp_rx_context[hrh].sdu_len);
                CAN_ISR_DEBUG_PRINT("[CANTP-SF-R]Receive PDU, HRH = %d, Length = %d, %02X %02X %02X. \r\n", hrh, s_cantp_rx_context[hrh].sdu_len,sdu_data[0],sdu_data[1],sdu_data[2]);
            }
            else
            {
                s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_SF_ERR;
                s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
                CAN_ISR_DEBUG_PRINT_ERR("[CANTP-SF-R]The DCM fails to receive data, HRH = %d! \r\n", hrh);
            }
        }
        else if (E_BUF_OVF == err_sts)
        {
            /* error process */
            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_SF_ERR;
            s_cantp_rx_context[hrh].result = CANTP_N_BUFFER_OVFLW;
            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-SF-R]The buffer of the DCM overflowed, HRH = %d! \r\n", hrh);
        }
        else
        {
            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_SF_ERR;
            s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-SF-R]Failed to copy data to the DCM, HRH = %d! \r\n", hrh);
        }
    }
    else
    {
        /* error process */
        s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_SF_ERR;
        s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-SF-R]The DCM is not ready, HRH = %d! \r\n", hrh);
    }
}

static void cantp_rx_ff(PDU_ID hrh, uint08* sdu_data)
{
    uint32 buf_len = 0;
    uint32 total_len = 0;
    eERR_STS err_sts = E_OK;
    stPDU_INFO temp_pdu;
    PDU_ID hth = s_cantp_rx_context[hrh].hth;

    total_len = (sdu_data[0] & 0x0F);
    total_len <<= 8U;
    total_len += sdu_data[1];
    temp_pdu.sdu_len = N_DATA_FF_LEN;
    temp_pdu.sdu_data = (uint08*)s_cantp_rx_buf;

    if (CANIF_HRH0_FUNC == hrh)
    {
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-FF-R]Functional addressing mode does not support multi-packet transmission, HRH = %d! \r\n", hrh);
        return;
    }
    
    if (   (s_cantp_rx_context[hrh].can_dl < CANTP_PDU_EXPECTED_DLC) /* error CAN DLC */
        || (total_len < N_DATA_SF_LEN) /* error PDU length */
        || (total_len > CANTP_PDU_MAX_LEN) /* error PDU length */
        )
    {
        if (CANTP_RX_IDLE == s_cantp_rx_context[hrh].sts)
        {
            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_FF_ERR;
            s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
        }
        else { /* nothing */ }
        if (s_cantp_rx_context[hrh].can_dl < CANTP_PDU_EXPECTED_DLC)
        {
            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-FF-R]The CAN frame data length is incorrect, HRH = %d! \r\n", hrh);
        }
        else
        {
            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-FF-R]The PDU length is incorrect, HRH = %d! \r\n", hrh);
        }
        return;
    }
//    else if (CANTP_TX_IDLE != s_cantp_tx_context[hth].sts)
    else if (  (CANTP_TX_IDLE != s_cantp_tx_context[s_cantp_rx_context[hrh].hth].sts)
            && (cantp_TX_INIT != s_cantp_tx_context[s_cantp_rx_context[hrh].hth].sts)
            && (CANTP_TX_SENDER_SF_REQ != s_cantp_tx_context[s_cantp_rx_context[hrh].hth].sts)
            && (CANTP_TX_SENDER_SF_CON != s_cantp_tx_context[s_cantp_rx_context[hrh].hth].sts)
            && (CANTP_TX_RECEIVER_CF_WAIT != s_cantp_tx_context[s_cantp_rx_context[hrh].hth].sts)
            )
    {
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-FF-R]FF received during sending, ignored, HRH = %d! \r\n", hrh);
        return;
    }
    else if ((s_cantp_rx_context[hrh].sts >= CANTP_RX_RECEIVER_FF_IND) && (s_cantp_rx_context[hrh].sts <= CANTP_RX_RECEIVER_FC_CON))
    {
        s_cantp_rx_context[hrh].result = CANTP_N_UNEXP_PDU;
        err_sts = E_NOK;
        dcm_rx_indication(hrh, &err_sts);
        cantp_tx_data_init(hth);
        dcm_tx_confirmation(hth, DEF_NULL);
        CAN_ISR_DEBUG_PRINT_WRN("[CANTP-FF-R]Segmented receive in progress received FF, HRH = %d! \r\n", hrh);
    }
    else { /* continue */ }
    
    if (E_OK == dcm_start_reception(hrh, total_len, &buf_len))
    {
        common_memcpy((uint08*)temp_pdu.sdu_data, (uint08*)&sdu_data[2], temp_pdu.sdu_len);
        err_sts = dcm_copy_rx_data(hrh, &temp_pdu, (uint32*)&buf_len);
        if (E_OK == err_sts)
        {
            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_FF_IND;
            s_cantp_rx_context[hrh].sdu_len = total_len;
            s_cantp_rx_context[hrh].last_sn = 1;
            s_cantp_rx_context[hrh].mf_cnt = 0;
            s_cantp_tx_context[hth].confirmation = E_BUSY;
//            CAN_ISR_DEBUG_PRINT("[CANTP-FF-R]Receive PDU, HRH = %d, Length = %d. \r\n", hrh, s_cantp_rx_context[hrh].sdu_len);
                CAN_ISR_DEBUG_PRINT("[CANTP-FF-R]Receive PDU, HRH = %d, Length = %d, %02X %02X %02X. \r\n", hrh, s_cantp_rx_context[hrh].sdu_len,sdu_data[0],sdu_data[1],sdu_data[2]);
            s_cantp_rx_context[hrh].n_br = g_cantp_cfg.rx_cfg[hrh].n_br;
        }
        else if (E_BUF_OVF == err_sts)
        {
            /* error process */
            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_FF_ERR;
            s_cantp_rx_context[hrh].result = CANTP_N_BUFFER_OVFLW;
            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-FF-R]The buffer of the DCM overflowed, HRH = %d! \r\n", hrh);
        }
        else
        {
            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_FF_ERR;
            s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-FF-R]Failed to copy data to the DCM, HRH = %d! \r\n", hrh);
        }
    }
    else
    {
        /* error process */
        s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_FF_ERR;
        s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-FF-R]The DCM is not ready, HRH = %d! \r\n", hrh);
    }
}

static void cantp_rx_cf(PDU_ID hrh, uint08* sdu_data)
{
    PDU_ID hth = 0;
    uint32 buf_len = 0;
    eERR_STS err_sts = E_OK;
    stPDU_INFO temp_pdu;
    uint08 sn = 0xFF;
    uint32 temp = 0;

    sn = (sdu_data[0] & 0x0F);
    hth = s_cantp_rx_context[hrh].hth;
    if (s_cantp_rx_context[hrh].can_dl < CANTP_PDU_EXPECTED_DLC) /* error CAN DLC */
    {
        if (CANTP_RX_IDLE == s_cantp_rx_context[hrh].sts)
        {
            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_FF_ERR;
            s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
        }
        else { /* nothing */ }
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-CF-R]The CAN frame data length is incorrect, HRH = %d! \r\n", hrh);
        return;
    }
    else if (  (CANTP_TX_IDLE != s_cantp_tx_context[hth].sts)
            && (cantp_TX_INIT != s_cantp_tx_context[hth].sts)
            && (CANTP_TX_RECEIVER_CF_WAIT != s_cantp_tx_context[hth].sts)
            )
    {
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-FF-R]CF received during sending, ignored, HRH = %d! \r\n", hrh);
        return;
    }
    else if (   (CANTP_RX_RECEIVER_FC_CON != s_cantp_rx_context[hrh].sts)
             && (CANTP_RX_RECEIVER_CF_IND != s_cantp_rx_context[hrh].sts)
             && (CANTP_RX_RECEIVER_FC_REQ != s_cantp_rx_context[hrh].sts)
             )
    {
        if (CANTP_RX_IDLE == s_cantp_rx_context[hrh].sts)
        {
            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_ERR;
            s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
        }
        else { /* nothing */ }
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-CF-R]Multi-frame receiving timing error, HRH = %d! \r\n", hrh);
        return;
    }

    if (sn == ((s_cantp_rx_context[hrh].last_sn) & 0x0F)) /* correct SN */
    {
        temp = s_cantp_rx_context[hrh].mf_cnt;
        if ((N_DATA_FF_LEN + temp * N_DATA_CF_LEN) + N_DATA_CF_LEN < s_cantp_rx_context[hrh].sdu_len)
        {
            temp_pdu.sdu_len = N_DATA_CF_LEN;
            temp = 0;
        }
        else
        {
            temp_pdu.sdu_len = ((s_cantp_rx_context[hrh].sdu_len - N_DATA_FF_LEN) % N_DATA_CF_LEN);
            if (0 == temp_pdu.sdu_len)
            {
                temp_pdu.sdu_len = N_DATA_CF_LEN;
            }
            temp = 1;
        }
        temp_pdu.sdu_data = (uint08*)s_cantp_rx_buf;
        
        common_memcpy((uint08*)temp_pdu.sdu_data, (uint08*)&sdu_data[1], temp_pdu.sdu_len);
        err_sts = dcm_copy_rx_data(hrh, &temp_pdu, (uint32*)&buf_len);
        if (E_OK == err_sts)
        {
            if (1 == temp) /* the last CF */
            {
                dcm_rx_indication(hrh, &err_sts);
                if (E_OK == err_sts)
                {
                    s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_IND_OVER;
                    s_cantp_rx_context[hrh].sdu_len = temp_pdu.sdu_len;
                    CAN_ISR_DEBUG_PRINT("[CANTP-CF-R]Receive PDU over, HRH = %d, Length = %d. \r\n", hrh, s_cantp_rx_context[hrh].sdu_len);
                }
                else
                {
                    s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_ERR;
                    s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
                    CAN_ISR_DEBUG_PRINT_ERR("[CANTP-CF-R]The DCM fails to receive data, HRH = %d! \r\n", hrh);
                }
            }
            else
            {
                /* continue */
                CAN_ISR_DEBUG_PRINT("[CANTP-CF-R]Receive PDU continue, HRH = %d, SN = %d, mf cnt = %d. \r\n", hrh, s_cantp_rx_context[hrh].last_sn, s_cantp_rx_context[hrh].mf_cnt);
                if (s_cantp_rx_context[hrh].bs)
                {
                    s_cantp_rx_context[hth].bs_cnt--;                      
                    if (0 == s_cantp_rx_context[hth].bs_cnt)
                    {
                        if (CANTP_TX_IDLE == s_cantp_tx_context[hth].sts)
                        {
                            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_FC_REQ;
                            s_cantp_tx_context[hth].sts = CANTP_TX_RECEIVER_FC_REQ;
                            s_cantp_tx_context[hth].confirmation = E_BUSY;
                            s_cantp_tx_context[hth].hrh = hrh;
                            CAN_ISR_DEBUG_PRINT("[CANTP-CF-R]Ready to resend the FC, HRH = %d. \r\n", hrh);
                        }
                        else
                        {
                            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_ERR;
                            s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
                            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-CF-R]Tx busy 3, HRH = %d! \r\n", hrh);
                        }
                    }
                    else
                    {
                        s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_IND;
                        s_cantp_rx_context[hrh].n_cr = g_cantp_cfg.rx_cfg[hrh].n_cr;
                    }
                }
                else
                {
                    s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_IND;
                    s_cantp_rx_context[hrh].n_cr = g_cantp_cfg.rx_cfg[hrh].n_cr;
                }
            }
        }
        else if (0 == s_cantp_rx_context[hrh].n_cr)
        {
            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_ERR;
            s_cantp_rx_context[hrh].result = CANTP_N_TIMEOUT_CR;
            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-CF-R]N_Cr timeout, HRH = %d! \r\n", hrh);
        }
        else if (E_BUF_OVF == err_sts)
        {
            /* error process */
            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_ERR;
            s_cantp_rx_context[hrh].result = CANTP_N_BUFFER_OVFLW;
            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-CF-R]The buffer of the DCM overflowed, HRH = %d! \r\n", hrh);
        }
        else
        {
            s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_FF_ERR;
            s_cantp_rx_context[hrh].result = CANTP_N_ERROR;
            CAN_ISR_DEBUG_PRINT_ERR("[CANTP-CF-R]Failed to copy data to the DCM, HRH = %d! \r\n", hrh);
        }
        s_cantp_rx_context[hrh].mf_cnt++;
        s_cantp_rx_context[hrh].last_sn++;
        if (s_cantp_rx_context[hrh].last_sn > 0x0F)
        {
            s_cantp_rx_context[hrh].last_sn = 0;
        }
    }
    else
    {
        /* error process */
        s_cantp_rx_context[hrh].sts = CANTP_RX_RECEIVER_CF_ERR;
        s_cantp_rx_context[hrh].result = CANTP_N_TIMEOUT_SN;
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-CF-R]The sequence number is incorrect, HRH = %d, last sn = %d, cur sn = %d! \r\n", hrh, s_cantp_rx_context[hrh].last_sn, sn);
    }
}

static void cantp_rx_fc(PDU_ID hrh, uint08* sdu_data)
{
    uint08 stmin = 0;
    PDU_ID hth = 0;

    hth = s_cantp_rx_context[hrh].hth;
    if (s_cantp_rx_context[hrh].can_dl < CANTP_PDU_EXPECTED_DLC) /* error CAN DLC */
    {
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-FC-R]The CAN frame data length is incorrect, HRH = %d! \r\n", hrh);
        return;
    }
    else if (CANIF_HRH0_FUNC == hrh)
    {
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-FC-R]FC without function addressing, HRH = %d! \r\n", hrh);
        return;
    }
    else if ((s_cantp_tx_context[hth].mf_cnt) && (0 == s_cantp_tx_context[hth].bs)) /* Have already received FC. */
    {
        CAN_ISR_DEBUG_PRINT_WRN("[CANTP-FC-R]Have already received FC, HRH = %d! \r\n", hrh);
        return;
    }
    
    if (   (CANTP_RX_SENDER_FC_IND_WAIT == s_cantp_rx_context[hrh].sts)
        || (CANTP_TX_SENDER_FF_REQ == s_cantp_tx_context[hth].sts)
        || (CANTP_TX_SENDER_CF_REQ == s_cantp_tx_context[hth].sts)
        || (CANTP_TX_SENDER_FF_CON == s_cantp_tx_context[hth].sts)
        || (CANTP_TX_SENDER_CF_CON_WAIT == s_cantp_tx_context[s_cantp_rx_context[hrh].hth].sts)
        )
    {
//        hth = s_cantp_rx_context[hrh].hth;
        s_cantp_rx_context[hrh].sts = CANTP_RX_SENDER_FC_IND;
        s_cantp_rx_context[hrh].fs = (sdu_data[0] & 0x0F);
        s_cantp_tx_context[hth].bs = (sdu_data[1] & 0xFF);
        s_cantp_tx_context[hth].bs_cnt = s_cantp_tx_context[hth].bs;
        stmin = (sdu_data[2] & 0xFF);
        if ((stmin >= 0x80) && (stmin <= 0xF0))
        {
            stmin = 0x7F;
        }
        else if ((stmin >= 0xFA) && (stmin <= 0xFF))
        {
            stmin = 0x7F;
        }
        else if ((stmin >= 0xF1) && (stmin <= 0xF9))
        {
            stmin = 1;
        }
        else {}
        if (stmin > g_cantp_cfg.rx_cfg[hrh].stmin)
        {
            stmin = g_cantp_cfg.rx_cfg[hrh].stmin;
        }
        else {}
        s_cantp_tx_context[hth].stmin = stmin;
        s_cantp_tx_context[hth].stmin_cnt = stmin;
    }
    else
    {
        /* ignore */
    }
}

static void cantp_tx_sf(PDU_ID hth, stPDU_INFO* pdu)
{
    stPDU_INFO temp_pdu;
    PDU_ID hth_p = hth;

    hth -= CANIF_HRH_NUM;
    s_cantp_tx_context[hth].confirmation = E_BUSY;
    s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_SF_REQ;
    s_cantp_tx_context[hth].n_as = g_cantp_cfg.tx_cfg[hth].n_as;
    common_memset((uint08*)s_cantp_tx_buf, CANTP_PDU_DEF_VAL, sizeof(s_cantp_tx_buf));
    s_cantp_tx_buf[0] = (0x00 | (pdu->sdu_len & 0x0F));
    common_memcpy((uint08*)&s_cantp_tx_buf[1], (uint08*)pdu->sdu_data, pdu->sdu_len);
    temp_pdu.sdu_data = s_cantp_tx_buf;
    temp_pdu.sdu_len = CAN_DATA_LENGTH;
    canif_transmit(hth_p, (stPDU_INFO*)&temp_pdu);
    cantp_tx_data_init(hth); /* After the send function is executed, the send is complete by default. */
    CAN_ISR_DEBUG_PRINT("[CANTP-SF-T]The SF was sent, HTH = %d. \r\n", hth);
}

static void cantp_tx_ff(PDU_ID hth, stPDU_INFO* pdu)
{
    stPDU_INFO temp_pdu;
    uint32 buf_len = 0;
    uint32 temp = 0;
    PDU_ID hth_p = hth;

    hth -= CANIF_HRH_NUM;
    s_cantp_tx_context[hth].confirmation = E_BUSY;
    s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_FF_REQ;
    s_cantp_tx_context[hth].n_as = g_cantp_cfg.tx_cfg[hth].n_as;
    s_cantp_tx_context[hth].sdu_len = pdu->sdu_len;
    s_cantp_tx_context[hth].last_sn = 0;
    s_cantp_tx_context[hth].mf_cnt = 0;
    s_cantp_tx_context[hth].mf_num = 0;
    s_cantp_tx_context[hth].mf_tail_len = 0;
    temp = s_cantp_tx_context[hth].sdu_len - N_DATA_FF_LEN;
    if (temp % N_DATA_CF_LEN)
    {
        s_cantp_tx_context[hth].mf_num += ((temp / N_DATA_CF_LEN) + 1);
        s_cantp_tx_context[hth].mf_tail_len = (temp % N_DATA_CF_LEN);
    }
    else
    {
        s_cantp_tx_context[hth].mf_num += (temp / N_DATA_CF_LEN);
        s_cantp_tx_context[hth].mf_tail_len = N_DATA_CF_LEN;
    }
    common_memset((uint08*)s_cantp_tx_buf, CANTP_PDU_DEF_VAL, sizeof(s_cantp_tx_buf));
    s_cantp_tx_buf[0] = (0x10 | (pdu->sdu_len >> 8U));
    s_cantp_tx_buf[1] = (pdu->sdu_len & 0xFF);
    temp_pdu.sdu_data = &s_cantp_tx_buf[2];
    temp_pdu.sdu_len = N_DATA_FF_LEN;
    dcm_copy_tx_data(hth, (stPDU_INFO*)&temp_pdu, &buf_len);
    temp_pdu.sdu_data = s_cantp_tx_buf;
    temp_pdu.sdu_len = CAN_DATA_LENGTH;
    canif_transmit(hth_p, (stPDU_INFO*)&temp_pdu);
    CAN_ISR_DEBUG_PRINT("[CANTP-FF-T]The FF was sent, HTH = %d. \r\n", hth);
}

static void cantp_tx_cf(PDU_ID hth)
{
    stPDU_INFO temp_pdu;
    uint32 buf_len = 0;
    PDU_ID hrh = s_cantp_tx_context[hth].hrh;
    eERR_STS err_sts = E_OK;
    
    if (N_SN_CF_MAX == s_cantp_tx_context[hth].last_sn)
    {
        s_cantp_tx_context[hth].last_sn = 0;
    }
    else
    {
        s_cantp_tx_context[hth].last_sn++;
    }
    s_cantp_tx_context[hth].mf_cnt++;
    if (s_cantp_tx_context[hth].bs_cnt)
    {
        s_cantp_tx_context[hth].bs_cnt--;
    }
    else {}
    
    common_memset((uint08*)s_cantp_tx_buf, CANTP_PDU_DEF_VAL, sizeof(s_cantp_tx_buf));
    s_cantp_tx_buf[0] = (0x20 | s_cantp_tx_context[hth].last_sn);
    temp_pdu.sdu_data = &s_cantp_tx_buf[1];
    if (s_cantp_tx_context[hth].mf_cnt >= s_cantp_tx_context[hth].mf_num)
    {
        temp_pdu.sdu_len = s_cantp_tx_context[hth].mf_tail_len;
        s_cantp_tx_context[hth].mf_over = 1;
    }
    else
    {
        temp_pdu.sdu_len = N_DATA_CF_LEN;
        s_cantp_tx_context[hth].mf_over = 0;
    }
    err_sts = dcm_copy_tx_data(hth, (stPDU_INFO*)&temp_pdu, &buf_len);
    if (E_OK == err_sts)
    {
        temp_pdu.sdu_data = s_cantp_tx_buf;
        temp_pdu.sdu_len = CAN_DATA_LENGTH;
        canif_transmit(hth + CANIF_HRH_NUM, (stPDU_INFO*)&temp_pdu);
        s_cantp_tx_context[hth].confirmation = E_BUSY;
        s_cantp_tx_context[hth].n_as = g_cantp_cfg.tx_cfg[hth].n_as;
        s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_CF_CON_WAIT;
        CAN_ISR_DEBUG_PRINT("[CANTP-CF-T]The CF was sent, HTH = %d. \r\n", hth);
    }
    else if (E_BUF_OVF == err_sts)
    {
        s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_CF_ERR;
        s_cantp_tx_context[hth].result = CANTP_N_BUFFER_OVFLW;
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-CF-T]The buffer of the DCM overflowed, HTH = %d! \r\n", hth);
    }
    else
    {
        s_cantp_tx_context[hth].sts = CANTP_TX_SENDER_CF_ERR;
        s_cantp_tx_context[hth].result = CANTP_N_ERROR;
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-CF-T]Failed to copy data to the DCM, HTH = %d! \r\n", hth);
    }    
}

static void cantp_tx_fc(PDU_ID hth)
{
    stPDU_INFO temp_pdu;
    uint32 buf_len = 0;
    PDU_ID hth_p = 0;
    PDU_ID hrh = 0;
    eCANTP_FS_TYP fs_typ = N_PCI_FS_CTS;

    hth_p += CANIF_HRH_NUM;
    hrh = s_cantp_tx_context[hth].hrh;
    
    common_memset((uint08*)s_cantp_tx_buf, CANTP_PDU_DEF_VAL, sizeof(s_cantp_tx_buf));
    if (s_cantp_rx_context[hrh].sdu_len > CANTP_RX_FROM_DCM_BUF_SIZE)
    {
        fs_typ = N_PCI_FS_OVFLW;
        s_cantp_tx_context[hth].fs = fs_typ;
        CAN_ISR_DEBUG_PRINT_ERR("[CANTP-FC-T]FC OVFLW, HTH = %d! \r\n", hth);
    }
    else if (0)
    {
        fs_typ = N_PCI_FS_WT;
    }
    else {}
    s_cantp_rx_context[hrh].bs = N_PCI_BS;
    s_cantp_rx_context[hrh].bs_cnt = s_cantp_rx_context[hrh].bs;
    s_cantp_tx_buf[0] = (0x30 | (fs_typ & 0x0F));
    s_cantp_tx_buf[1] = s_cantp_tx_context[hth].bs;
    s_cantp_tx_buf[2] = N_PCI_STMIN;
    temp_pdu.sdu_data = s_cantp_tx_buf;
    temp_pdu.sdu_len = CAN_DATA_LENGTH;
    canif_transmit(hth_p, (stPDU_INFO*)&temp_pdu);
}

static void cantp_rx_data_init(PDU_ID hrh)
{    
    s_cantp_rx_context[hrh].sts = CANTP_RX_IDLE;
    s_cantp_rx_context[hrh].sts_dly_cnt = CANTP_STS_CHG_DLY_NUM;
    s_cantp_rx_context[hrh].hth = g_cantp_cfg.rx_cfg[hrh].hth - CANIF_HRH_NUM;
    s_cantp_rx_context[hrh].rcv_ft = FRAME_TYPE_INVALID;
}

static void cantp_tx_data_init(PDU_ID hth)
{    
    s_cantp_tx_context[hth].sts = CANTP_TX_IDLE;
    s_cantp_tx_context[hth].sts_dly_cnt = CANTP_STS_CHG_DLY_NUM;
}

