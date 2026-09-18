/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_dcm.h
* Author        : yangming
* Date          : 2024-01-23
* Version       : 1.00
* Description   : Can diagnostic communication manager header file.
* Others        : None
*
****************************************************************************************************/
#ifndef _CAN_DCM_H
#define _CAN_DCM_H
#include "cantp.h"
#include "can_dcm_cfg.h"

typedef enum _sid27_sts                             stSID27_STS;
typedef enum _dcm_hrh_typ                           eDCM_HRH_TYP;
typedef enum _dcm_service_sts                       eDCM_SSERVICE_STS;

enum _sid27_sts
{
    SID27_STS_IDLE = 0,
    SID27_STS_RECV_SEED,
    SID27_STS_RECV_KEY,
    SID27_STS_DLY_TIMER_ACTIVE,

    SID27_STS_MAX
};

enum _dcm_hrh_typ
{
    DCM_HRH0_TYP_PHY0 = 0,
    DCM_HRH0_TYP_PHY1,
    DCM_HRH0_TYP_FUNC0,

    DCM_HRH_TYP_MAX
};

enum _dcm_service_sts
{
    DCM_SERVICE_IDLE = 0,
    DCM_SERVICE_REQ,
    DCM_SERVICE_RES,
    DCM_SERVICE_RES_NRC78,

    DCM_SERVICE_MAX
    
};

extern eERR_STS dcm_transmit(PDU_ID hth, stPDU_INFO* pdu);
extern void dcm_tx_confirmation(PDU_ID hth, eERR_STS* result);
extern void dcm_rx_indication(PDU_ID hrh, eERR_STS* result);
extern void dcm_init(void);
extern void dcm_main_function(void);
extern void dcm_1002_nrc78_form_app_set(PDU_ID hrh);
extern void dcm_unconditional_session_ctrl(eDCM_SESSION_TYPE session);
extern eDCM_SESSION_TYPE dcm_session_get(void);
extern void dcm_period_10ms_process(void);
#endif /* _CAN_DCM_H */

