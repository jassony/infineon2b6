/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_dcm.c
* Author        : yangming
* Date          : 2024-01-24
* Version       : 1.00
* Description   : Can diagnostic communication manager file.
* Others        : None
*
****************************************************************************************************/
#include "can_dcm.h"
#include "uds_user.h"
#include <stdlib.h>

static uint08 s_dcm_rx_buf[DCM_PDU_RX_NUM][DCM_RX_BUF_SIZE];
static uint08 s_dcm_tx_buf[DCM_PDU_TX_NUM][DCM_TX_BUF_SIZE];
static uint08 s_dcm_tx_nrc_buf[3];
static uint32 s_dcm_req_len[DCM_PDU_RX_NUM];
static uint32 s_dcm_rx_index[DCM_PDU_RX_NUM];
static uint32 s_dcm_tx_index[DCM_PDU_TX_NUM];
static volatile uint08 s_dcm_rx_flg[DCM_PDU_RX_NUM];
static eDCM_SSERVICE_STS s_dcm_service_sts[DCM_PDU_PHY_NUM];
static eDCM_SESSION_TYPE s_cur_session;
static uint08 s_cur_security;
stSID22_DP_INFO g_sid22_dp_info;
stSID2E_DP_INFO g_sid2e_dp_info;
//stSID14_DP_INFO g_sid14_dp_info;
stSID2F_DP_INFO g_sid2f_dp_info;
stSID31_DP_INFO g_sid31_dp_info;
stSID34_DP_INFO g_sid34_dp_info;
stSID36_DP_INFO s_sid36_dp_info;
uint08 g_service_10_nrc78_result;
uint08 g_service_11_nrc78_result;
uint08 g_service_31_nrc78_result;
static uint08 s_service_common_nrc78_result;
static uint32 s_dcm_10ms_cnt;
static stSID27_STS s_sid27_sts;
static uint32 s_sid27_seed;
static uint08 s_security_num;
static uint32 s_security_err_time;
static uint32 s_s3server_cnt;
static stDCM_MSG_CONTEXT dcm_main_msg[DCM_PDU_PHY_NUM];
stDCM_SERVICE_CFG dcm_main_service_cfg;

static void dcm_processing_done(stDCM_MSG_CONTEXT * msg, uint08 pos_rsp_indication);
static void dcm_set_neg_response(stDCM_MSG_CONTEXT* msg);
static eERR_STS get_cfg_through_sid(uint08 sid, stDCM_SERVICE_CFG* cfg);
static uint32 dcm_get_rand_seed(void);
static uint32 dcm_get_key(uint32 seed);
static void dcm_server_time_process(void);
static typ_bool dcm_get_common_condition(void);
static eDCM_HRH_TYP dcm_hrh_typ_get(PDU_ID hrh);

void dcm_init(void)
{
    s_dcm_10ms_cnt = 0;
    s_sid27_sts = SID27_STS_IDLE;
    s_sid27_seed = 0;
    s_s3server_cnt = 0;
    s_cur_security = DCM_SEC_LV_DEF;
    s_cur_session = SESSION_TYPE_DEFAULT;
    s_security_num = 0;
    s_security_err_time = 0;
    g_service_10_nrc78_result = NRC_78_INIT;
    g_service_11_nrc78_result = NRC_78_INIT;
    common_memset((uint08*)&s_dcm_service_sts, DCM_SERVICE_IDLE, sizeof(s_dcm_service_sts));
    common_memset((uint08*)&g_sid22_dp_info, 0U, sizeof(g_sid22_dp_info));
    common_memset((uint08*)&g_sid2e_dp_info, 0U, sizeof(g_sid2e_dp_info));
//    common_memset((uint08*)&g_sid14_dp_info, 0U, sizeof(g_sid14_dp_info));
    common_memset((uint08*)&g_sid2f_dp_info, 0U, sizeof(g_sid2f_dp_info));
    common_memset((uint08*)&g_sid34_dp_info, 0U, sizeof(g_sid34_dp_info));
    common_memset((uint08*)&s_sid36_dp_info, 0U, sizeof(s_sid36_dp_info));
}

void dcm_main_function(void)
{
    PDU_ID hrh = 0;
    PDU_ID hth = CANIF_HTH0_PHY;
    uint08 phy_index = 0xFF;

    if (UDS_VOL_NORMAL != uds_vol_typ_get())
    {
//        s_s3server_cnt = 0;
//        return;
    }

    for (hrh = 0; hrh < DCM_PDU_RX_NUM; hrh++)
    {
        phy_index = get_phy_pdu_through_hrh(hrh);
        if (0xFF == phy_index) /* check the pdu match cfg */
        {
            break;
        }
        if ((0 != s_dcm_rx_flg[hrh]) && (DCM_SERVICE_IDLE == s_dcm_service_sts[phy_index]))
        {
            common_memset((uint08*)&dcm_main_msg[phy_index], 0U, sizeof(stDCM_MSG_CONTEXT));
            common_memset((uint08*)&dcm_main_service_cfg, 0U, sizeof(dcm_main_service_cfg));
            s_dcm_service_sts[phy_index] = DCM_SERVICE_REQ;
            dcm_main_msg[phy_index].hrh = hrh;
            dcm_main_msg[phy_index].hth = g_pdu_match_cfg[phy_index].phy_hth;
            dcm_main_msg[phy_index].nrc = 0xFF;
            s_dcm_rx_flg[hrh] = 0;
        }
        else {}
    }

    for (phy_index = 0; phy_index < DCM_PDU_PHY_NUM; phy_index++)
    {
        switch (s_dcm_service_sts[phy_index])
        {
            case DCM_SERVICE_IDLE:
                /* wait for service request */
                break;
            case DCM_SERVICE_REQ:
                if (E_OK == get_cfg_through_sid(s_dcm_rx_buf[dcm_main_msg[phy_index].hrh][0], (stDCM_SERVICE_CFG*)&dcm_main_service_cfg))
                {
                    s_s3server_cnt = 0;
                    dcm_main_msg[phy_index].req_data = &s_dcm_rx_buf[dcm_main_msg[phy_index].hrh][0];
                    dcm_main_msg[phy_index].req_data_len = s_dcm_req_len[dcm_main_msg[phy_index].hrh];
                    dcm_main_msg[phy_index].res_data = &s_dcm_tx_buf[dcm_main_msg[phy_index].hth - CANIF_HRH_NUM][0];
                    if (DEF_NULL == dcm_main_service_cfg.nrc78_result)
                    {
                        dcm_main_msg[phy_index].nrc78_result = &s_service_common_nrc78_result;
                    }
                    else
                    {
                        dcm_main_msg[phy_index].nrc78_result = dcm_main_service_cfg.nrc78_result;
                    }
                    if (DEF_NULL != dcm_main_service_cfg.app_func)
                    {
                        dcm_main_service_cfg.app_func((stDCM_MSG_CONTEXT*)&dcm_main_msg[phy_index], dcm_main_service_cfg);
                        if ((dcm_main_msg[phy_index].nrc != NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING) || (DEF_NULL == dcm_main_service_cfg.nrc78_result))
                        {
                            s_dcm_service_sts[phy_index] = DCM_SERVICE_IDLE;
                        }
                        else
                        {
                            s_dcm_service_sts[phy_index] = DCM_SERVICE_RES_NRC78;
                            *dcm_main_msg[phy_index].nrc78_result = NRC_78_INIT;
                        }
                    }
                    else
                    {
                        dcm_main_msg[phy_index].nrc = NRC_SERVICE_NOT_SUPPORTED;
                        dcm_set_neg_response(dcm_main_msg);
                        s_dcm_service_sts[phy_index] = DCM_SERVICE_IDLE;
                    }
                }
                else
                {
                    dcm_main_msg[phy_index].req_data = &s_dcm_rx_buf[dcm_main_msg[phy_index].hrh][0];
                    dcm_main_msg[phy_index].res_data = &s_dcm_tx_buf[dcm_main_msg[phy_index].hth - CANIF_HRH_NUM][0];
                    dcm_main_msg[phy_index].nrc = NRC_SERVICE_NOT_SUPPORTED;
                    dcm_set_neg_response(&dcm_main_msg[phy_index]);
                    s_dcm_service_sts[phy_index] = DCM_SERVICE_IDLE;
                }
                break;
            case DCM_SERVICE_RES:
                break;
            case DCM_SERVICE_RES_NRC78:
                if (s_s3server_cnt >= S3_SERVER_TIME)
                {
                    s_dcm_service_sts[phy_index] = DCM_SERVICE_IDLE;
                }
                else if (*dcm_main_msg[phy_index].nrc78_result)
                {
                    if (NRC_78_POS_RESPONSE == *dcm_main_msg[phy_index].nrc78_result)
                    {
                        dcm_processing_done((stDCM_MSG_CONTEXT*)&dcm_main_msg[phy_index], 0U);
                        *dcm_main_msg[phy_index].nrc78_result = NRC_78_PEDING;
                    }
                    else if (NRC_78_NEG_RESPONSE == *dcm_main_msg[phy_index].nrc78_result)
                    {
                        dcm_set_neg_response((stDCM_MSG_CONTEXT*)&dcm_main_msg[phy_index]);
                        *dcm_main_msg[phy_index].nrc78_result = NRC_78_PEDING;
                    }
                    else {}
                    if (dcm_main_msg[phy_index].nrc != NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING)
                    {
                        s_dcm_service_sts[phy_index] = DCM_SERVICE_IDLE;
                    }
                    else {}
                }
                else {}
                break;
            default:
                break;
        }
    }
}

void dcm_nrc78_posres_set(PDU_ID hrh, uint08* res_data, uint32 res_len)
{
    uint08 phy_index = 0xFF;

    phy_index = get_phy_pdu_through_hrh(hrh);
    if (   (0xFF == phy_index) /* check the pdu match cfg */
        || (res_len > DCM_TX_BUF_SIZE)
        || (0U == res_len)
        || (DEF_NULL == res_data)
        ) 
    {
        return;
    }

    common_memcpy((uint08*)dcm_main_msg[phy_index].res_data, res_data, res_len);
    dcm_main_msg[phy_index].res_data_len = res_len;
    dcm_main_msg[phy_index].nrc = NRC_POSITIVE_RESPONSE;
    *dcm_main_msg[phy_index].nrc78_result = NRC_78_POS_RESPONSE;
}

void dcm_nrc78_negres_set(PDU_ID hrh, uint32 nrc)
{
    uint08 phy_index = 0xFF;

    phy_index = get_phy_pdu_through_hrh(hrh);
    if (0xFF == phy_index) 
    {
        return;
    }

    dcm_main_msg[phy_index].nrc = nrc;
    *dcm_main_msg[phy_index].nrc78_result = NRC_78_NEG_RESPONSE;
}

void dcm_1002_nrc78_form_app_set(PDU_ID hrh)
{    
    s_dcm_rx_flg[hrh] = 1;
    s_dcm_req_len[hrh] = 2;
    s_dcm_rx_buf[hrh][0] = 0x10;
    s_dcm_rx_buf[hrh][1] = 0x02;
}

eERR_STS dcm_start_reception(PDU_ID hrh, uint32 sdu_len, uint32* buf_len)
{
    if ((hrh >= DCM_PDU_RX_NUM) || (0U == sdu_len))
    {
        return E_NOK;
    }

    s_dcm_req_len[hrh] = sdu_len;

    if (0 == s_dcm_rx_flg[hrh])
    {
        if (sdu_len <= DCM_RX_BUF_SIZE)
        {
            if (DEF_NULL != buf_len)
            {
                *buf_len = DCM_RX_BUF_SIZE;
            }
            else {/* nothing */}
        }
        else
        {
            if (DEF_NULL != buf_len)
            {
                *buf_len = 0U;
            }
            else {/* nothing */}
            
            return E_BUF_OVF;
        }
    }
    else
    {
        if (DEF_NULL != buf_len)
        {
            *buf_len = 0U;
        }
        else {/* nothing */}

        return E_BUSY;
    }

    return E_OK;
}

void dcm_rx_indication(PDU_ID hrh, eERR_STS* result)
{
    eERR_STS sts = E_OK;
    
    if (hrh >= DCM_PDU_RX_NUM)
    {
        if (DEF_NULL != result)
        {
            *result = E_NOK;
        }
        else {/* nothing */}
        return;
    }

    if (DEF_NULL != result)
    {
        sts  = *result;
    }
    else {/* nothing */}

    if (E_OK == sts)
    {
        s_dcm_rx_flg[hrh] = 1;
    }
    else
    {
        s_dcm_rx_flg[hrh] = 0;
    }
    s_dcm_rx_index[hrh] = 0;
}

void dcm_tx_confirmation(PDU_ID hth, eERR_STS* result)
{
    if (DEF_NULL != result)
    {
        *result = E_OK;
    }
    else {/* nothing */}
    s_dcm_tx_index[hth] = 0;
}

eERR_STS dcm_copy_rx_data(PDU_ID hrh, stPDU_INFO* pdu, uint32* buf_len)
{
    if ((DEF_NULL == pdu) || (hrh >= DCM_PDU_RX_NUM))
    {
        return E_NOK;
    }
    else if (pdu->sdu_len > (DCM_RX_BUF_SIZE - s_dcm_rx_index[hrh]))
    {
        return E_BUF_OVF;
    }

    common_memcpy((uint08*)&s_dcm_rx_buf[hrh][s_dcm_rx_index[hrh]], (uint08*)pdu->sdu_data, pdu->sdu_len);
    s_dcm_rx_index[hrh] += pdu->sdu_len;
    if (DEF_NULL == buf_len)
    {
        *buf_len = DCM_RX_BUF_SIZE - s_dcm_rx_index[hrh];
    }
    else {/* nothing */}
    return E_OK;
}

eERR_STS dcm_copy_tx_data(PDU_ID hth, stPDU_INFO* pdu, uint32* buf_len)
{
    if ((DEF_NULL == pdu) || (hth >= DCM_PDU_TX_NUM))
    {
        return E_NOK;
    }
    else if (pdu->sdu_len > (DCM_TX_BUF_SIZE - s_dcm_tx_index[hth]))
    {
        return E_BUF_OVF;
    }
    else if (s_dcm_tx_index[hth] > DCM_TX_BUF_SIZE)
    {
        return E_BUF_OVF;
    }
    common_memcpy((uint08*)pdu->sdu_data, (uint08*)&s_dcm_tx_buf[hth][s_dcm_tx_index[hth]], pdu->sdu_len);
    s_dcm_tx_index[hth] += pdu->sdu_len;
    if (DEF_NULL != buf_len)
    {
        *buf_len = DCM_TX_BUF_SIZE - s_dcm_tx_index[hth];
    }
    else
    {
        /* nothing */
    }
    return E_OK;
}

eERR_STS dcm_transmit(PDU_ID hth, stPDU_INFO* pdu)
{
    return E_OK;
}

static eERR_STS get_cfg_through_sid(uint08 sid, stDCM_SERVICE_CFG* cfg)
{
    stDCM_SERVICE_CFG* service_cfg = (stDCM_SERVICE_CFG*)g_dcm_service_cfg;

    while (service_cfg->sid != DCM_SID_INVALID)
    {
        if (service_cfg->sid == sid)
        {
            common_memcpy((uint08*)cfg, (uint08*)service_cfg, sizeof(stDCM_SERVICE_CFG));
            return E_OK;
        }
        service_cfg++;
    }

    return E_NOK;
}

static void dcm_server_time_process(void)
{
    if ((s_s3server_cnt >= S3_SERVER_TIME) && (s_cur_session != SESSION_TYPE_DEFAULT))
    {
        s_cur_session = SESSION_TYPE_DEFAULT;
        bl_s3_timeout_cbk();
    }
    else {}

    if ((s_cur_security != DCM_SEC_LV_DEF) && (SESSION_TYPE_DEFAULT == s_cur_session))
    {
        s_cur_security = DCM_SEC_LV_DEF;
    }
    else {}
}

/* 10 service */
void app_diagnostic_session_control(stDCM_MSG_CONTEXT * msg, stDCM_SERVICE_CFG service_cfg)
{
    stDCM_SUBFUNC_CFG* subfunc_cfg = (stDCM_SUBFUNC_CFG*)service_cfg.subfunc;
    uint08 sub_id = DCM_SID_SUB_MAX;
    uint32 valid_min_len = 0;
    uint32 valid_len = 0;
    uint32 temp = 0;
    eDCM_SESSION_TYPE s_recv_session = SESSION_TYPE_DEFAULT;

    valid_min_len++; /* sid */
    valid_min_len++; /* subFunction */
    if (DEF_NULL != subfunc_cfg)
    {
        while (subfunc_cfg->sub_id != DCM_SID_SUB_MAX)
        {
            if (subfunc_cfg->sub_id == (msg->req_data[1] & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT)))
            {
                sub_id = (msg->req_data[1] & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT));
                s_recv_session = ((eDCM_SESSION_TYPE)(sub_id - 1) & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT));
                valid_len = subfunc_cfg->dp_len + valid_min_len;
                break;
            }
            else
            {
                subfunc_cfg++;
            }
        }
    }
    else
    {
        /* continue */
    }

    if (0 == (service_cfg.session_lv & (1U << s_cur_session))) /* 7F */
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (msg->req_data_len < valid_min_len) /* minimum length NRC 13 */
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((DCM_SID_SUB_MAX == sub_id) || (DEF_NULL == subfunc_cfg))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED;
    }
    else if (msg->req_data_len != valid_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    #if 1
    else if (DEF_FALSE == dcm_get_common_condition())
    {
        msg->nrc = NRC_CONNDITIONS_NOT_ERROR;
    }
    else if ((SESSION_TYPE_PROGRAMMING == s_recv_session) && (msg->hrh != CANIF_HRH0_PHY)) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
        return;
    }
    #endif
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }
    
    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        msg->res_data_len = 2U;
        common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, msg->res_data_len);
        msg->res_data_len += 4U;
        temp = P2CAN_SERVER_TIME * DCM_PERIOD_TIME_BASE; /* P2server: 1ms unit */
        msg->res_data[2] = (uint08)(temp >> 8U);
        msg->res_data[3] = (uint08)(temp >> 0U);
        temp = P2_CAN_SERVER_TIME; /* P2*server: 10ms unit */
        msg->res_data[4] = (uint08)(temp >> 8U);
        msg->res_data[5] = (uint08)(temp >> 0U);
                
        switch (s_recv_session)
        {
            case SESSION_TYPE_DEFAULT:
                if (DEF_NULL != service_cfg.callback)
                {
                    service_cfg.callback(msg, service_cfg.cbk_param);
                }
                else {/* nothing */}
                s_cur_session = s_recv_session;
                if (NRC_POSITIVE_RESPONSE == msg->nrc)
                {
                    dcm_processing_done(msg, subfunc_cfg->pos_rsp_msg_indication);
                }
                else
                {
                    dcm_set_neg_response(msg);
                }
                s_cur_security = DCM_SEC_LV_DEF;
                s_sid27_sts = SID27_STS_IDLE;
                break;
            case SESSION_TYPE_PROGRAMMING:
                if (SESSION_TYPE_DEFAULT == s_cur_session)
                {
                    msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED_IN_ACTIVE_SESSION;
                    dcm_set_neg_response(msg);
                }
                else
                {
                    if (DEF_NULL != service_cfg.callback)
                    {
                        service_cfg.callback(msg, service_cfg.cbk_param);
                    }
                    else {/* nothing */}
                    if (NRC_POSITIVE_RESPONSE == msg->nrc)
                    {
                        s_cur_session = s_recv_session;
                        s_cur_security = DCM_SEC_LV_DEF;
                        s_sid27_sts = SID27_STS_IDLE;
                        dcm_processing_done(msg, subfunc_cfg->pos_rsp_msg_indication);
                    }
                    else
                    {
                        dcm_set_neg_response(msg);
                    }
                }
                break;
            case SESSION_TYPE_EXTENDED:
//                if (SESSION_TYPE_PROGRAMMING == s_cur_session)
//                {
//                    msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED_IN_ACTIVE_SESSION;
//                    dcm_set_neg_response(msg);
//                }
//                else
//                {
                    s_cur_session = s_recv_session;
                    msg->nrc = NRC_POSITIVE_RESPONSE;
                    dcm_processing_done(msg, subfunc_cfg->pos_rsp_msg_indication);
                    s_cur_security = DCM_SEC_LV_DEF;
                    s_sid27_sts = SID27_STS_IDLE;
//                }
                if (DEF_NULL != service_cfg.callback)
                {
                    service_cfg.callback(msg, service_cfg.cbk_param);
                }
                else {/* nothing */}
                break;
            default:
                msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED_IN_ACTIVE_SESSION;
                dcm_set_neg_response(msg);
                break;
        }
    }
    else
    {
        dcm_set_neg_response(msg);
    }
}

void dcm_unconditional_session_ctrl(eDCM_SESSION_TYPE session)
{
    s_cur_session = session;
}

eDCM_SESSION_TYPE dcm_session_get(void)
{
    return s_cur_session;
}

/* 11 */
void app_ecu_reset(stDCM_MSG_CONTEXT * msg, stDCM_SERVICE_CFG service_cfg)
{
    stDCM_SUBFUNC_CFG* subfunc_cfg = (stDCM_SUBFUNC_CFG*)service_cfg.subfunc;
    uint32 valid_min_len = 0;
    uint32 valid_len = 0;
    uint08 sub_id = DCM_SID_SUB_MAX;

    valid_min_len++; /* sid */
    valid_min_len++; /* subFunction */
    if (DEF_NULL != subfunc_cfg)
    {
        while (subfunc_cfg->sub_id != DCM_SID_SUB_MAX)
        {
            if (subfunc_cfg->sub_id == (msg->req_data[1] & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT)))
            {
                sub_id = msg->req_data[1];
                valid_len = subfunc_cfg->dp_len + valid_min_len;
                break;
            }
            else
            {
                subfunc_cfg++;
            }
        }
    }
    else {/* continue */}

    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session))
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (msg->req_data_len < valid_min_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((DCM_SID_SUB_MAX == sub_id) || (DEF_NULL == subfunc_cfg))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED;
    }
    else if (msg->req_data_len != valid_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((subfunc_cfg->session_lv & (1 << s_cur_session)) != (1 << s_cur_session))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (DEF_FALSE == dcm_get_common_condition())
    {
        msg->nrc = NRC_CONNDITIONS_NOT_ERROR;
    }
    else if (   ((subfunc_cfg->addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        msg->res_data_len = 2U;
        common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, msg->res_data_len);
        dcm_processing_done(msg, subfunc_cfg->pos_rsp_msg_indication);
    }
    else
    {
        dcm_set_neg_response(msg);
    }
}


/* 27 */
void app_security_access(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg)
{
    stDCM_SUBFUNC_CFG* subfunc_cfg = (stDCM_SUBFUNC_CFG*)service_cfg.subfunc;
    stDCM_DATA_PARAM_CFG* dp_cfg = service_cfg.dp;
    uint08 sub_id = DCM_SID_SUB_MAX;
    uint32 valid_min_len = 0;
    uint32 valid_len = 0;
    stSID27_STS recv_sts = 0;
    uint32 temp = 0;
    uint08 temp_array[DCM_SID27_SEED_NUM] = {0};

    if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
         && (msg->hrh != CANIF_HRH0_PHY)
         ) /* function address is not supported */
    {
        return;
    }
    
    valid_min_len++; /* sid */
    valid_min_len++; /* subFunction */

    if ((DEF_NULL != subfunc_cfg) && (msg->req_data_len >= subfunc_cfg->dp_len))
    {
        while (subfunc_cfg->sub_id != DCM_SID_SUB_MAX)
        {
            if (subfunc_cfg->sub_id == (msg->req_data[1] & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT)))
            {
                sub_id = msg->req_data[1];
                valid_len = subfunc_cfg->dp_len + valid_min_len;
                if (0U == (sub_id % 2U)) /* receive key */
                {
                    recv_sts = SID27_STS_RECV_KEY;
                }
                else /* receive seed request */
                {
                    recv_sts = SID27_STS_RECV_SEED;
                }
                break;
            }
            else
            {
                subfunc_cfg++;
            }
        }
    }
    else {/* continue */}

    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session))
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (msg->req_data_len < subfunc_cfg->dp_len + valid_min_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((DEF_NULL == subfunc_cfg) || (DCM_SID_SUB_MAX == sub_id))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED;
    }
    else if (msg->req_data_len != valid_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((subfunc_cfg->session_lv & (1 << s_cur_session)) != (1 << s_cur_session))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (DEF_FALSE == dcm_get_common_condition())
    {
        msg->nrc = NRC_CONNDITIONS_NOT_ERROR;
    }
    else if (   ((subfunc_cfg->addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, 2U);
        common_memset((uint08*)temp_array, 0U, DCM_SID27_SEED_NUM);
        switch (s_sid27_sts)
        {
            case SID27_STS_IDLE:
                if (SID27_STS_RECV_SEED == recv_sts)
                {
                    s_sid27_sts = SID27_STS_RECV_SEED;
                    s_security_num = DCM_SECURITY_NUM;
                }
                else
                {
                    msg->nrc = NRC_REQUEST_SEQUENCE_ERROR;
                }
                break;
            case SID27_STS_RECV_SEED:
                if (SID27_STS_RECV_SEED == recv_sts)
                {
                    /* send seed */
                    s_security_num = DCM_SECURITY_NUM;
                }
                else /* Receive the key after sending the seed */
                {
                    #if (DCM_SID27_KEY_NUM == 4)
                    temp  = ((uint32)msg->req_data[2] << 24U) +
                            ((uint32)msg->req_data[3] << 16U) +
                            ((uint32)msg->req_data[4] << 8U) +
                            ((uint32)msg->req_data[5] << 0U);
                    #else
                    temp  = ((uint32)msg->req_data[2] << 8U) +
                            ((uint32)msg->req_data[3] << 0U);
                    #endif
                    if (temp == dcm_get_key(s_sid27_seed))
                    {
                        /* res key */
                        s_sid27_sts = SID27_STS_IDLE;
                        
                    }
                    else
                    {
                        s_sid27_sts = SID27_STS_RECV_KEY;
                        s_security_num--;
                        if (0 == s_security_num)
                        {
                            s_sid27_sts = SID27_STS_DLY_TIMER_ACTIVE;
                            s_security_err_time = 0; /* active delay timer */
                            msg->nrc = NRC_EXCEED_NUMBER_OF_ATTEMPTS;
                        }
                        else
                        {
                            msg->nrc = NRC_INVALID_KEY;
                        }
                    }
                }
                break;
            case SID27_STS_RECV_KEY:
                if (SID27_STS_RECV_SEED == recv_sts)
                {
                    /* send seed */
                    s_sid27_sts = SID27_STS_RECV_SEED;
                }
                else
                {
                    msg->nrc = NRC_REQUEST_SEQUENCE_ERROR;
                }
                break;
            case SID27_STS_DLY_TIMER_ACTIVE:
                if (SID27_STS_RECV_SEED == recv_sts)
                {
                    if (s_security_err_time < DCM_SECURITY_ERR_DLY_TIME)
                    {
                        msg->nrc = NRC_REQUIRD_TIME_DELAY_NOT_EXPIRED;
                    }
                    else
                    {
                        /* send seed */
                        s_sid27_sts = SID27_STS_RECV_SEED;
                        s_security_num = DCM_SECURITY_NUM; /* Security access times reset */
                    }
                }
                else
                {
                    msg->nrc = NRC_REQUEST_SEQUENCE_ERROR;
                }
                break;
            default:
                return;
        }
    }

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        if (SID27_STS_RECV_SEED == recv_sts)
        {
            if (   ((DCM_SEC_LV_L1 == s_cur_security) && (DCM_SEC_LV_L1_SEED == sub_id))
                || ((DCM_SEC_LV_L2 == s_cur_security) && (DCM_SEC_LV_L2_SEED == sub_id))
                || ((DCM_SEC_LV_L3 == s_cur_security) && (DCM_SEC_LV_L3_SEED == sub_id))
                )
            {
                temp = 0;
            }
            else
            {
                temp = dcm_get_rand_seed();
            }
            s_sid27_seed = temp;
            #if (DCM_SID27_SEED_NUM == 4)
            temp_array[0] = ((temp >> 24U) & 0xFF);
            temp_array[1] = ((temp >> 16U) & 0xFF);
            temp_array[2] = ((temp >> 8U) & 0xFF);
            temp_array[3] = ((temp >> 0U) & 0xFF);
            #else
            temp_array[0] = ((temp >> 8U) & 0xFF);
            temp_array[1] = ((temp >> 0U) & 0xFF);
            #endif
            valid_min_len += DCM_SID27_SEED_NUM;
            common_memcpy((uint08*)(msg->res_data + 2U), (uint08*)temp_array, DCM_SID27_SEED_NUM);
            msg->res_data_len = valid_min_len;
        }
        else
        {
            msg->res_data_len = 2;
            temp = msg->res_data[1] - 1U;
            if (DCM_SEC_LV_L1_SEED == temp)
            {
                s_cur_security = DCM_SEC_LV_L1;
            }
            else if (DCM_SEC_LV_L2_SEED == temp)
            {
                s_cur_security = DCM_SEC_LV_L2;
            }
            else if (DCM_SEC_LV_L3_SEED == temp)
            {
                s_cur_security = DCM_SEC_LV_L3;
            }
            else {}
        }
        dcm_processing_done(msg, subfunc_cfg->pos_rsp_msg_indication);
    }
    else
    {
        dcm_set_neg_response(msg);
    }

    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
    else {/* nothing */}
}

/* 28 */
void app_communication_control(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg)
{
    stDCM_SUBFUNC_CFG* subfunc_cfg = (stDCM_SUBFUNC_CFG*)service_cfg.subfunc;
    uint32 valid_min_len = 0;
    uint32 valid_len = 0;
    uint08 sub_id = DCM_SID_SUB_MAX;

    valid_min_len++; /* sid */
    valid_min_len++; /* subFunction */
    valid_min_len++; /* communicationType */

    if (DEF_NULL != subfunc_cfg)
    {
        while (subfunc_cfg->sub_id != DCM_SID_SUB_MAX)
        {
            if (subfunc_cfg->sub_id == (msg->req_data[1] & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT)))
            {
                sub_id = msg->req_data[1];
                valid_len = subfunc_cfg->dp_len + valid_min_len;
                break;
            }
            else
            {
                subfunc_cfg++;
            }
        }
    }
    else {/* continue */}

    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session))
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (msg->req_data_len < valid_min_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((DCM_SID_SUB_MAX == sub_id) || (DEF_NULL == subfunc_cfg))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED;
    }
    else if (msg->req_data_len != valid_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((subfunc_cfg->session_lv & (1 << s_cur_session)) != (1 << s_cur_session))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if ((msg->req_data[2] & 0x0F) == 0)
    {
        msg->nrc = NRC_REQUEST_OUT_OF_RANGE;
    }
    else if (DEF_FALSE == dcm_get_common_condition())
    {
        msg->nrc = NRC_CONNDITIONS_NOT_ERROR;
    }
    else if (   ((subfunc_cfg->addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        msg->res_data_len = 2U;
        common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, msg->res_data_len);
        dcm_processing_done(msg, subfunc_cfg->pos_rsp_msg_indication);
    }
    else
    {
        dcm_set_neg_response(msg);
    }

    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
}


/* 3E */
void app_tester_present(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg)
{
    stDCM_SUBFUNC_CFG* subfunc_cfg = (stDCM_SUBFUNC_CFG*)service_cfg.subfunc;
    uint32 valid_min_len = 0;
    uint32 valid_len = 0;
    uint08 sub_id = DCM_SID_SUB_MAX;

    valid_min_len++; /* sid */
    valid_min_len++; /* subFunction */
    if (DEF_NULL != subfunc_cfg)
    {
        while (subfunc_cfg->sub_id != DCM_SID_SUB_MAX)
        {
            if (subfunc_cfg->sub_id == (msg->req_data[1] & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT)))
            {
                sub_id = msg->req_data[1];
                valid_len = subfunc_cfg->dp_len + valid_min_len;
                break;
            }
            else
            {
                subfunc_cfg++;
            }
        }
    }
    else {/* continue */}

    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session))
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (msg->req_data_len < valid_min_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((DCM_SID_SUB_MAX == sub_id) || (DEF_NULL == subfunc_cfg))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED;
    }
    else if (msg->req_data_len != valid_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((subfunc_cfg->session_lv & (1 << s_cur_session)) != (1 << s_cur_session))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (   ((subfunc_cfg->addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        msg->res_data_len = 2U;
        common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, msg->res_data_len);
        dcm_processing_done(msg, subfunc_cfg->pos_rsp_msg_indication);
    }
    else
    {
        dcm_set_neg_response(msg);
    }

    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
    else {/* nothing */}
}

/* 85 */
void app_dtc_setting(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg)
{
    stDCM_SUBFUNC_CFG* subfunc_cfg = (stDCM_SUBFUNC_CFG*)service_cfg.subfunc;
    uint32 valid_min_len = 0;
    uint32 valid_len = 0;
    uint08 sub_id = DCM_SID_SUB_MAX;

    valid_min_len++; /* sid */
    valid_min_len++; /* subFunction */

    if (DEF_NULL != subfunc_cfg)
    {
        while (subfunc_cfg->sub_id != DCM_SID_SUB_MAX)
        {
            if (subfunc_cfg->sub_id == (msg->req_data[1] & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT)))
            {
                sub_id = msg->req_data[1];
                valid_len = subfunc_cfg->dp_len + valid_min_len;
                break;
            }
            else
            {
                subfunc_cfg++;
            }
        }
    }
    else {/* continue */}

    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session))
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (msg->req_data_len < valid_min_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((DCM_SID_SUB_MAX == sub_id) || (DEF_NULL == subfunc_cfg))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED;
    }
    else if (msg->req_data_len != valid_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((subfunc_cfg->session_lv & (1 << s_cur_session)) != (1 << s_cur_session))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (DEF_FALSE == dcm_get_common_condition())
    {
        msg->nrc = NRC_CONNDITIONS_NOT_ERROR;
    }
    else if (   ((subfunc_cfg->addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        msg->res_data_len = 2U;
        common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, msg->res_data_len);
        dcm_processing_done(msg, subfunc_cfg->pos_rsp_msg_indication);
    }
    else
    {
        dcm_set_neg_response(msg);
    }

    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
}

/* 22 */
void app_read_data_by_identifier(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg)
{
    stDCM_DATA_PARAM_CFG* dp_cfg = service_cfg.dp;
    stDCM_DID_CFG* did_cfg = DEF_NULL;
    uint32 valid_min_len = 0;
    uint32 valid_len = 0;
    uint32 temp = 0;
    uint16 i = 0;
    uint16 j = 0;
    uint16 req_did_num = 0;

    common_memset((uint08*)g_sid22_dp_info.did, 0xFF, sizeof(g_sid22_dp_info.did));
    
    valid_min_len++; /* sid */
    valid_len = valid_min_len;
    valid_min_len += DCM_SID22_DP_LEN_MIN; /* did length */
    
    if (DEF_NULL != dp_cfg)
    {
        did_cfg = (stDCM_DID_CFG*)dp_cfg->data;
        for (i = 1; i < msg->req_data_len; i += dp_cfg->len)
        {
            temp = msg->req_data[i];
            temp <<= 8U;
            temp += msg->req_data[i + 1]; /* did */
            for (j = 0; j < DCM_DID_NUM; j++)
            {
                if (   (did_cfg[j].did == temp) 
                    && ((did_cfg[j].session_lv  & (1 << s_cur_session)) == (1 << s_cur_session))
                    )
                {
                    g_sid22_dp_info.did[req_did_num] = temp;
                    g_sid22_dp_info.typ[req_did_num] = (eDID_TYP)j;
                    req_did_num++;
                    valid_len += dp_cfg->len;
                    break;
                }
                else {}
            }
        }
    }
    else
    {
        req_did_num = 0;
    }
    if (valid_len < valid_min_len)
    {
        valid_len = valid_min_len;
    }
    g_sid22_dp_info.did_num = req_did_num;
    
    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session)) /* 7F */
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (msg->req_data_len < valid_min_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if (msg->req_data_len != valid_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if (0 == req_did_num)
    {
        msg->nrc = NRC_REQUEST_OUT_OF_RANGE;
    }
    else if (msg->req_data_len > DCM_TX_BUF_SIZE)
    {
        msg->nrc = NRC_RESPONSE_TOO_LONG;
    }
    else if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    msg->res_data_len = 1;
    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
    else {/* nothing */}
    
    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        msg->res_data[0] = msg->req_data[0];
        dcm_processing_done(msg, 0);
    }
    else
    {
        dcm_set_neg_response(msg);
    }
}

/* 2E */
void app_write_data_by_identifier(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg)
{
    stDCM_DATA_PARAM_CFG* dp_cfg = service_cfg.dp;
    stDCM_DID_CFG* did_cfg = DEF_NULL;
    uint32 valid_min_len = 0;
    uint32 valid_len = 0;
    uint32 temp = 0;
    uint16 i = 0;
    uint08 req_invalid = 0;

    g_sid2e_dp_info.did = DID_INVALID;
    g_sid2e_dp_info.len = 0;
    
    valid_min_len++; /* sid */
    temp = msg->req_data[1];
    temp <<= 8U;
    temp += msg->req_data[2];
    valid_min_len += 2; /* did */
    valid_len = valid_min_len;
    
    if (DEF_NULL != dp_cfg)
    {
        did_cfg = (stDCM_DID_CFG*)dp_cfg->data;
        for (i = 0; i < DCM_DID_NUM; i++)
        {
            
            if ((did_cfg[i].did == temp) && ((did_cfg[i].rw & DID_RW_WRITE) == DID_RW_WRITE))
            {
                req_invalid = 1;
                g_sid2e_dp_info.did = temp;
                g_sid2e_dp_info.typ = (eDID_TYP)i;
                valid_len += did_cfg[i].did_len;
                break;
            }
            else {}
        }

        if ((msg->req_data_len - 3) != did_cfg[i].did_len)
        {
        }
        else
        {
            g_sid2e_dp_info.len = did_cfg[i].did_len;
        }
    }
    else
    {
    }
    
    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session)) /* 7F */
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (msg->req_data_len < valid_min_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((service_cfg.security_lv & s_cur_security) != s_cur_security) /* 33 for SID */
    {
        msg->nrc = NRC_SECURITY_ACCESS_DENIED;
    }
    else if (0 == req_invalid)
    {
        msg->nrc = NRC_REQUEST_OUT_OF_RANGE;
    }
    else if (msg->req_data_len != valid_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
    else {/* nothing */}
    
    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        msg->res_data_len = 3U;
        common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, msg->res_data_len);
        dcm_processing_done(msg, 0);
    }
    else
    {
        dcm_set_neg_response(msg);
    }
}

/* 14 */
void app_clear_diagnostic_information(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg)
{
    #if 1
    stDCM_DATA_PARAM_CFG* dp_cfg = service_cfg.dp;
//    stDTC_CFG* dtc_cfg = DEF_NULL;
    uint32 valid_min_len = 0;
    uint32 temp = 0;
    uint16 i = 0;
    uint08 req_invalid = 0;

    
    valid_min_len++; /* sid */
    temp = ((uint32)msg->req_data[1] << 16U) + 
           ((uint32)msg->req_data[2] << 8U) + 
           ((uint32)msg->req_data[3] << 0U);
    valid_min_len += 3; /* dtc */

    #if 0
    if (DEF_NULL != dp_cfg)
    {
        dtc_cfg = (stDTC_CFG*)dp_cfg->data;
        for (i = 0; i < DCM_DID_NUM; i++)
        {
            
            if ((dtc_cfg[i].dtc == temp) || (0xFFFFFF == temp))
            {
                req_invalid = 1;
                g_sid14_dp_info.dtc = temp;
                g_sid14_dp_info.typ = (eDTC_TYP)i;
                break;
            }
            else {}
        }
    }
    else
    {
    }
    #endif
    
    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session)) /* 7F */
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
//    else if (msg->req_data_len < valid_min_len)
//    {
//        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
//    }
//    else if (msg->req_data_len > (1U + CAN_DTC_NUM * 2U))
//    {
//        msg->nrc = NRC_RESPONSE_TOO_LONG;
//    }
//    else if (0 == req_invalid)
//    {
//        msg->nrc = NRC_REQUEST_OUT_OF_RANGE;
//    }
//    else if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
//             && (msg->hrh != CANIF_HRH0_PHY)
//             ) /* function address is not supported */
//    {
//        msg->nrc = NRC_GENERAL_REJECT;
//    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
    else {/* nothing */}
    
    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        msg->res_data_len = 1U;
        common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, msg->res_data_len);
        dcm_processing_done(msg, 0);
    }
    else
    {
        dcm_set_neg_response(msg);
    }
    #endif
}


/* 19 */
void app_read_dtc_information(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg)
{
    stDCM_SUBFUNC_CFG* subfunc_cfg = (stDCM_SUBFUNC_CFG*)service_cfg.subfunc;
    stDCM_DATA_PARAM_CFG* dp_cfg = service_cfg.dp;
    uint32 valid_min_len = 0;
    uint32 valid_len = 0;
    uint08 sub_id = DCM_SID_SUB_MAX;
    uint16 num = 0;
    uint32 req_dtc = 0;

    valid_min_len++; /* sid */
    valid_min_len++; /* subFunction */

    if (DEF_NULL != subfunc_cfg)
    {
        while (subfunc_cfg->sub_id != DCM_SID_SUB_MAX)
        {
            if (subfunc_cfg->sub_id == (msg->req_data[1] & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT)))
            {
                sub_id = msg->req_data[1];
                #if 1
                if ((DCM_SID19_SUB_01 == sub_id) || (DCM_SID19_SUB_02 == sub_id))
                {
                    valid_len = 3;
                }
                else if ((DCM_SID19_SUB_04 == sub_id) || (DCM_SID19_SUB_06 == sub_id))
                {
                    valid_len = 6;
                }
                else if ((DCM_SID19_SUB_0A == sub_id) || (DCM_SID19_SUB_03 == sub_id))
                {
                    valid_len = 2;
                }
                else {}
                #endif
                break;
            }
            else
            {
                subfunc_cfg++;
            }
        }
    }
    else {/* continue */}
        
    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session)) /* 7F */
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (msg->req_data_len < valid_min_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((DCM_SID_SUB_MAX == sub_id) || (DEF_NULL == subfunc_cfg))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED;
    }
    else if (msg->req_data_len != valid_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
    
    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        //dcm_processing_done(msg, subfunc_cfg->pos_rsp_msg_indication);
    }
    else
    {
        dcm_set_neg_response(msg);
    }
}


/* 2F */
void app_input_output_control_by_identifier(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg)
{
    stDCM_DATA_PARAM_CFG* dp_cfg = service_cfg.dp;
    stDCM_RID_CFG* rid_cfg = DEF_NULL;
    uint32 temp = 0;
    uint32 temp1 = 0;

    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session)) /* 7F */
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (msg->req_data_len < 4U)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if (service_cfg.security_lv != s_cur_security) /* 33 for SID */
    {
        msg->nrc = NRC_SECURITY_ACCESS_DENIED;
    }
    else if (DEF_FALSE == dcm_get_common_condition())
    {
        msg->nrc = NRC_CONNDITIONS_NOT_ERROR;
    }
    else if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        temp = msg->req_data[1];
        temp <<= 16U;
        temp += msg->req_data[2];
        g_sid2f_dp_info.did = temp;
        g_sid2f_dp_info.iocp =  msg->req_data[3];
        temp = msg->req_data_len;
        if ((temp < 5U) || (6U == temp))
        {
            msg->nrc = NRC_REQUEST_OUT_OF_RANGE;
        }
        else
        {
            if (temp > 6U)
            {
                temp -= 4U;
                temp1 = (temp / 8U);
                g_sid2f_dp_info.cs_len = temp - temp1;
                g_sid2f_dp_info.cm_len = temp1;
                
            }
            else /* one cs */
            {
                g_sid2f_dp_info.cs_len = 1;
                g_sid2f_dp_info.cm_len = 0;
            }
            msg->res_data_len = msg->req_data_len - g_sid2f_dp_info.cm_len;
            common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, msg->res_data_len);
            dcm_processing_done(msg, 0);
        }
    }
    else
    {
        dcm_set_neg_response(msg);
    }

    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
    else {/* nothing */}
}

/* 31 */
void app_routine_control(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg)
{
    stDCM_SUBFUNC_CFG* subfunc_cfg = (stDCM_SUBFUNC_CFG*)service_cfg.subfunc;
    stDCM_DATA_PARAM_CFG* dp_cfg = service_cfg.dp;
    stDCM_RID_CFG* rid_cfg = DEF_NULL;
    uint08 sub_id = DCM_SID_SUB_MAX;
    uint32 valid_min_len = 0;
    uint32 valid_len = 0;
    uint16 req_rid = 0;
    uint32 temp = 0;
    uint08 spc_hand_flg = 0;

    if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
         && (msg->hrh != CANIF_HRH0_PHY)
         ) /* function address is not supported */
    {
        return;
    }
    
    valid_min_len++;

    if (DEF_NULL != subfunc_cfg)
    {
        while (subfunc_cfg->sub_id != DCM_SID_SUB_MAX)
        {
            if (subfunc_cfg->sub_id == (msg->req_data[1] & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT)))
            {
                sub_id = msg->req_data[1];
                valid_min_len++;
                break;
            }
            else
            {
                subfunc_cfg++;
            }
        }
    }
    else {/* continue */}

    valid_len = valid_min_len;
    temp = msg->req_data[2];
    temp <<= 8U;
    temp += msg->req_data[3];
    req_rid = temp;
    temp = 0;
    if (DEF_NULL != dp_cfg)
    {
        rid_cfg = (stDCM_RID_CFG*)dp_cfg->data;
        while (temp < DCM_RID_NUM)
        {
            if (rid_cfg[temp].rid == req_rid)
            {
                valid_min_len += 2U;
                valid_len = valid_min_len;
                valid_len += rid_cfg[temp].dp_len;
                break;
            }
            temp++;
        }
        if (temp >= DCM_RID_NUM)
        {
            req_rid = DCM_SID31_INVALID_RID;
        }
        else {/* nothing */}
    }
    else
    {
        req_rid = DCM_SID31_INVALID_RID;
    }
    g_sid31_dp_info.req_rid = req_rid;

    if ((DCM_SEC_LV_L1 == s_cur_security) && (RID_CHECK_PROGRAMMING_PRE_CONDITIONS == req_rid)) /* special handling */
    {
        spc_hand_flg = 1;
    }
    else
    {
        spc_hand_flg = 0;
    }

    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session)) /* 7F */
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (msg->req_data_len < valid_min_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((DCM_SID_SUB_MAX == sub_id) || (DEF_NULL == subfunc_cfg))
    {
        msg->nrc = NRC_SUBFUNC_NOT_SUPPORTED;
    }
    else if (  (DCM_SID_SUB_MAX) != sub_id 
            && (subfunc_cfg->security_lv != s_cur_security) 
//            && (subfunc_cfg->security_lv != DCM_SEC_LV_UNLOCK)
            && (0 == spc_hand_flg)
            ) /* 33 for RID */
    {
        msg->nrc = NRC_SECURITY_ACCESS_DENIED;
    }
    else if (DCM_SID31_INVALID_RID == req_rid)
    {
        msg->nrc = NRC_REQUEST_OUT_OF_RANGE;
    }
    else if (msg->req_data_len != valid_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if (DEF_FALSE == dcm_get_common_condition())
    {
        msg->nrc = NRC_CONNDITIONS_NOT_ERROR;
    }
    else if (   ((subfunc_cfg->addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
    else {/* nothing */}
    
    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        if (DEF_NULL != rid_cfg[temp].rid_func)
        {
            msg->nrc = rid_cfg[temp].rid_func();
        }
        else {/* nothing */}
        if (NRC_POSITIVE_RESPONSE == msg->nrc)
        {
            msg->res_data_len = 4U + DCM_SID31_ROUTINE_STATUS_RECORD_LENGTH;
            common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, msg->res_data_len);
            dcm_processing_done(msg, subfunc_cfg->pos_rsp_msg_indication);
        }
        else
        {
            dcm_set_neg_response(msg);
        }
    }
    else
    {
        if (NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING == msg->nrc)
        {
            if (DEF_NULL != rid_cfg[temp].rid_func)
            {
                (void)rid_cfg[temp].rid_func();
            }
            else {/* nothing */}
        }
        else {}
        dcm_set_neg_response(msg);
    }
}

/* 34 */
void app_request_download(stDCM_MSG_CONTEXT * msg, stDCM_SERVICE_CFG service_cfg)
{
    stDCM_DATA_PARAM_CFG* dp_cfg = service_cfg.dp;
    uint32 valid_min_len = 0;
    uint08 mem_addr_len = (msg->req_data[2] & 0x0F);
    uint08 mem_size_len = ((msg->req_data[2] & 0xF0) >> 4U);
    uint32 temp = 0;
    uint32 i = 0;
    uint32 j = 0;

    if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
         && (msg->hrh != CANIF_HRH0_PHY)
         ) /* function address is not supported */
    {
        return;
    }
    
    valid_min_len++; /* sid len +1 */
    valid_min_len += dp_cfg->len; /* minimum data parameter length */

    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session)) /* 7F */
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if ((service_cfg.security_lv & s_cur_security) != s_cur_security)
    {
        msg->nrc = NRC_SECURITY_ACCESS_DENIED;
    }
    else if (msg->req_data_len < valid_min_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if ((*dp_cfg->data != msg->req_data[1])
            || (0 == mem_size_len)
            || (0 == mem_addr_len)
            || (mem_addr_len > 4U)
            || (mem_size_len > 4U)
            )
    {
        msg->nrc = NRC_REQUEST_OUT_OF_RANGE;
    }
    else if (msg->req_data_len != (valid_min_len + mem_addr_len + mem_size_len))
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        g_sid34_dp_info.transmit_start = 1;
        g_sid34_dp_info.data_format = msg->req_data[1];
        g_sid34_dp_info.mem_addr_bytelen = mem_addr_len;
        g_sid34_dp_info.mem_size_bytelen = mem_size_len;
        g_sid34_dp_info.mem_start_addr = 0;
        s_sid36_dp_info.recv_over = 0;
        s_sid36_dp_info.block_seq_cnt = 1;
        for (i = 0; i < mem_addr_len; i++)
        {
            temp = 0;
            temp = msg->req_data[3U + i];
            temp <<= (8U * (mem_addr_len - i - 1U));
            g_sid34_dp_info.mem_start_addr |= temp;
        }
        g_sid34_dp_info.wr_data_size = 0;
        for (j = 0; j < mem_size_len; j++)
        {
            temp = 0;
            temp = msg->req_data[3U + i + j];
            temp <<= (8U * (mem_size_len - j - 1U));
            g_sid34_dp_info.wr_data_size |= temp;
        }

        msg->res_data[0] = msg->req_data[0]; /*  SID */
        msg->res_data[1] = DCM_SID34_LENGTH_FORMAT_IDENTIFIER; /* maxNumberOfBlockLength */
        valid_min_len = (DCM_SID34_LENGTH_FORMAT_IDENTIFIER >> 4U);
        temp = DCM_SID34_MAX_NUMBER_OF_BLOCK_LENGTH + 2U;
        for (i = 0; i < valid_min_len; i++)
        {
            msg->res_data[2U + valid_min_len - i - 1U] = (temp & 0xFF);
            temp >>= 8U;
        }
        msg->res_data_len = 2U + valid_min_len;
        dcm_processing_done(msg, 0);
    }
    else
    {
        dcm_set_neg_response(msg);
    }

    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
    else {/* nothing */}
}

/* 36 */
void app_transfer_data(stDCM_MSG_CONTEXT * msg, stDCM_SERVICE_CFG service_cfg)
{
    uint32 valid_min_len = 0;

    if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
         && (msg->hrh != CANIF_HRH0_PHY)
         ) /* function address is not supported */
    {
        return;
    }
    
    valid_min_len++; /* sid: 1byte */
    valid_min_len++; /* blockSequenceCounter: 1byte */

    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session)) /* 7F */
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if ((service_cfg.security_lv & s_cur_security) != s_cur_security) /* 33 for SID */
    {
        msg->nrc = NRC_SECURITY_ACCESS_DENIED;
    }
    else if (msg->req_data_len < valid_min_len)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if (0 == g_sid34_dp_info.transmit_start)
    {
        msg->nrc = NRC_REQUEST_SEQUENCE_ERROR;
    }
    else if (s_sid36_dp_info.block_seq_cnt != msg->req_data[1])
    {
        msg->nrc = NRC_WRONG_BLOCK_SEQUENCE_COUNTER;
    }
    else if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if(s_sid36_dp_info.block_seq_cnt == 1)
    {
      
      
    
    }
    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
    else {/* nothing */}

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
//        s_sid36_dp_info.block_seq_cnt++;
        msg->res_data_len = 2U;
        common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, msg->res_data_len);
        dcm_processing_done(msg, 0);

        s_sid36_dp_info.recv_data_len += (msg->req_data_len - 2U);
        if (s_sid36_dp_info.recv_data_len >= g_sid34_dp_info.wr_data_size)
        {
//            s_sid36_dp_info.recv_over = 1;
        }
    }
    else
    {
        dcm_set_neg_response(msg);
    }
}

/* 37 */
void app_request_transfer_exit(stDCM_MSG_CONTEXT * msg, stDCM_SERVICE_CFG service_cfg)
{
    if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
         && (msg->hrh != CANIF_HRH0_PHY)
         ) /* function address is not supported */
    {
        return;
    }
    
    if ((service_cfg.session_lv & (1 << s_cur_session)) != (1 << s_cur_session)) /* 7F */
    {
        msg->nrc = NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if ((service_cfg.security_lv & s_cur_security) != s_cur_security) /* 33 for SID */
    {
        msg->nrc = NRC_SECURITY_ACCESS_DENIED;
    }
    else if (0 == s_sid36_dp_info.recv_over)
    {
        msg->nrc = NRC_REQUEST_SEQUENCE_ERROR;
    }
    else if (msg->req_data_len != 1U)
    {
        msg->nrc = NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT;
    }
    else if (   ((service_cfg.addr_mode & DCM_FUNC_REQ_SUPPORTED) != DCM_FUNC_REQ_SUPPORTED)
             && (msg->hrh != CANIF_HRH0_PHY)
             ) /* function address is not supported */
    {
        msg->nrc = NRC_GENERAL_REJECT;
    }
    else
    {
        msg->nrc = NRC_POSITIVE_RESPONSE;
    }

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        msg->res_data_len = 1U;
        common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, msg->res_data_len);
        dcm_processing_done(msg, 0);
    }
    else
    {
        dcm_set_neg_response(msg);
    }

    if (DEF_NULL != service_cfg.callback)
    {
        service_cfg.callback(msg, service_cfg.cbk_param);
    }
    else {/* nothing */}
}

static void dcm_processing_done(stDCM_MSG_CONTEXT * msg, uint08 pos_rsp_indication)
{
    stPDU_INFO d_sdu;

    if (   (0 != pos_rsp_indication) 
        && (SUPPRESS_POS_RESP_MSG_INDICATION_BIT == (msg->res_data[1] & SUPPRESS_POS_RESP_MSG_INDICATION_BIT))
        )
    {
        return;
    }
    
    common_memset((uint08*)&d_sdu, 0U, sizeof(d_sdu));
    msg->res_data[0] |= POSITIVE_RES_SID_MASK;

    d_sdu.sdu_data = msg->res_data;
    d_sdu.sdu_len = msg->res_data_len;
    cantp_transmit(msg->hth, (stPDU_INFO*)&d_sdu);
}

static void dcm_set_neg_response(stDCM_MSG_CONTEXT * msg)
{
    stPDU_INFO d_sdu;

    if (DCM_HRH0_TYP_FUNC0 == dcm_hrh_typ_get(msg->hrh))
    {
        if (   (NRC_SERVICE_NOT_SUPPORTED == msg->nrc)
            || (NRC_SUBFUNC_NOT_SUPPORTED == msg->nrc)
            || (NRC_REQUEST_OUT_OF_RANGE == msg->nrc)
            || (NRC_SUBFUNC_NOT_SUPPORTED_IN_ACTIVE_SESSION == msg->nrc)
            || (NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION == msg->nrc)
            )
        {
            return;
        }
        else {}
    }
    else {}
    
    common_memset((uint08*)&d_sdu, 0U, sizeof(d_sdu));
    s_dcm_tx_nrc_buf[0] = 0x7F;
    s_dcm_tx_nrc_buf[1] = msg->req_data[0]; /* service id */
    s_dcm_tx_nrc_buf[2] = msg->nrc; /* NRC valid */

    d_sdu.sdu_data = s_dcm_tx_nrc_buf;
    d_sdu.sdu_len = 3U;
    cantp_transmit(msg->hth, (stPDU_INFO*)&d_sdu);
}

void dcm_period_10ms_process(void)
{
    s_dcm_10ms_cnt++;
    s_security_err_time++;
    if (s_s3server_cnt < 0xFFFFFFFF)
    {
        s_s3server_cnt++;
    }

    dcm_server_time_process();
}

static uint32 dcm_get_rand_seed(void)
{
    uint32 seed = DCM_SEED_INIT_VAL;
    uint32 cur_cnt = s_dcm_10ms_cnt;

    srand(cur_cnt);
    seed = (rand() % 65534U) + 1U;

    return seed;
}

static uint32 dcm_get_key(uint32 seed)
{
    const unsigned int Constant_1 = 0x9A34E0BF; /* flash */
//    const unsigned int Constant_1 = 0xBC252ADF;
    unsigned int key = 0u;  
    unsigned int Temp1 = 0u;
    unsigned int Temp2 = 0u;
    unsigned int mask = 0;

    if (SESSION_TYPE_PROGRAMMING == s_cur_session)
    {
        mask = 0x9A34E0BF;
    }
    else if (SESSION_TYPE_EXTENDED == s_cur_session)
    {
        mask = 0xBC252ADF;
    }
    else
    {
        return key;
    }
    
    Temp1 = ((mask ^ seed) ^ (mask & seed)) & 0xFFFF0000;
    Temp2 = ((mask ^ seed) ^ (mask | seed)) & 0x0000FFFF;
    key = Temp1  | Temp2;
     
    return key;
}

static typ_bool dcm_get_common_condition(void)
{
    if (uds_vechicle_speed_get() <= 5)
    {
        return DEF_TRUE;
    }
    else
    {
        return DEF_FALSE;
    }
}

static eDCM_HRH_TYP dcm_hrh_typ_get(PDU_ID hrh)
{
    if (CANIF_HRH0_PHY == hrh)
    {
        return DCM_HRH0_TYP_PHY0;
    }
    else if (CANIF_HRH0_FUNC == hrh)
    {
        return DCM_HRH0_TYP_FUNC0;
    }
    else
    {
        return DCM_HRH0_TYP_PHY0;
    }
}

