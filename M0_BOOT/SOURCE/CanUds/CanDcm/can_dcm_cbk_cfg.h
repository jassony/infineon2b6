/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_dcm_cbk_cfg.h
* Author        : yangming
* Date          : 2024-01-23
* Version       : 1.00
* Description   : Can dcm callback configuration header file.
* Others        : None
*
****************************************************************************************************/
#ifndef _CAN_DCM_CBK_CFG_H
#define _CAN_DCM_CBK_CFG_H
#include "can_dcm.h"

#define CANTP_RX_FROM_DCM_BUF_SIZE                      DCM_RX_BUF_SIZE

extern eERR_STS dcm_start_reception(PDU_ID hrh, uint32 sdu_len, uint32* buf_len);
extern void dcm_rx_indication(PDU_ID hrh, eERR_STS* result);
extern void dcm_tx_confirmation(PDU_ID hth, eERR_STS* result);
extern eERR_STS dcm_copy_rx_data(PDU_ID hrh, stPDU_INFO* pdu, uint32* buf_len);
extern eERR_STS dcm_copy_tx_data(PDU_ID hth, stPDU_INFO* pdu, uint32* buf_len);
extern void app_ecu_reset(stDCM_MSG_CONTEXT * msg, stDCM_SERVICE_CFG service_cfg);
extern void app_diagnostic_session_control(stDCM_MSG_CONTEXT * msg, stDCM_SERVICE_CFG service_cfg);
extern void app_security_access(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_communication_control(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_tester_present(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_dtc_setting(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_read_data_by_identifier(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_write_data_by_identifier(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_clear_diagnostic_information(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_read_dtc_information(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_input_output_control_by_identifier(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_routine_control(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_request_download(stDCM_MSG_CONTEXT * msg, stDCM_SERVICE_CFG service_cfg);
extern void app_transfer_data(stDCM_MSG_CONTEXT * msg, stDCM_SERVICE_CFG service_cfg);
extern void app_request_transfer_exit(stDCM_MSG_CONTEXT * msg, stDCM_SERVICE_CFG service_cfg);
extern void bl_service_10_cbk(void* cbk_param1, void* cbk_param2);
extern void bl_service_28_cbk(void * cbk_param1, void * cbk_param2);
extern void bl_service_31_cbk(void* cbk_param1, void* cbk_param2);
extern void bl_service_34_cbk(void* cbk_param1, void* cbk_param2);
extern void bl_service_36_cbk(void* cbk_param1, void* cbk_param2);
extern void bl_service_37_cbk(void* cbk_param1, void* cbk_param2);
extern void dcm_service_2f_cbk(void* cbk_param1, void* cbk_param2);
extern void dcm_service_11_cbk(void* cbk_param1, void* cbk_param2);
extern void dcm_service_28_cbk(void* cbk_param1, void* cbk_param2);
extern void dcm_service_22_cbk(void* cbk_param1, void* cbk_param2);
extern void dcm_service_2e_cbk(void* cbk_param1, void* cbk_param2);
extern void dcm_service_14_cbk(void* cbk_param1, void* cbk_param2);
extern void dcm_service_19_cbk(void* cbk_param1, void* cbk_param2);
#endif /* _CAN_DCM_CBK_CFG_H */
