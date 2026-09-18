/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : uds_user.c
* Author        : yangming
* Date          : 2024-04-19
* Version       : 1.00
* Description   : 
* Others        : None
*
****************************************************************************************************/
#include "canif.h"
#include "cantp.h"
#include "common_mem_op.h"
#include "uds_user.h"
#include "bootloader.h"
#include "can_user.h"
//#include "system_S32K144.h"
#include "can_dtc.h"
#include "eeprom_app.h"
#include "software_version.h"
//#include "adc.h"

//#define UDS_USER_DEBUG
typedef enum _uu_time
{
    UU_TIME_SERVICE_11 = 0,
    UU_TIME_SERVICE_2E,
    UU_TIME_SERVICE_2F,
    UU_TIME_UDS_VOL_START,
    UU_TIME_UDS_VOL_RESTART,

    UU_TIME_NUM
}eUU_TIME;

typedef enum _uds_2f_did_index
{
    UDS_2F_DID_INDX_4001 = 0,
    UDS_2F_DID_INDX_4002,
    UDS_2F_DID_INDX_4003,
    UDS_2F_DID_INDX_4004,
    UDS_2F_DID_INDX_4005,
    UDS_2F_DID_INDX_4006
}eUDS_2F_DID_INDEX;

static volatile uint08 s_uds_user_data[CAN_CHN_NUM][CAN_DATA_LENGTH];
#ifdef UDS_RX_DATA_COPY_FORM_TASK_EN
static volatile uint08  s_uds_rx0_flg[UDS_USER_RX0_NUM];
static uint32 t_send_test_cnt;
#endif
static eUDS_VOL_TYP s_uds_vol_typ;
static uint32 s_uds_vol;
static uint16 s_uds_vol_sample_cnt;
static uint32 s_uu_time[UU_TIME_NUM];
static stSERVICE_11_CTRL s_service_11;
static stSERVICE_28_CTRL s_service_28;
static stSERVICE_85_CTRL s_service_85;
stSERVICE_22_CTRL g_service_22;
stSERVICE_2E_CTRL g_service_2e;
static stSERVICE_2F_CTRL s_service_2f;
static stSERVICE_14_CTRL s_service_14;
static stSERVICE_19_CTRL s_service_19;

static uint08 s_did_val_len_tbl[DCM_DID_NUM];

uint08                      did_0200_val[DID_0200_DATA_LEN];
uint08                      did_0201_val[DID_0201_DATA_LEN];
uint08                      did_f180_val[DID_F180_DATA_LEN];
uint08                      did_f186_val[DID_F186_DATA_LEN];
uint08                      did_f190_val[DID_F190_DATA_LEN];
uint08                      did_f193_val[DID_F193_DATA_LEN];
uint08                      did_f195_val[DID_F195_DATA_LEN];
uint08                      did_f194_val[DID_F194_DATA_LEN];
uint08                      did_f199_val[DID_F199_DATA_LEN];
uint08                      did_f1ef_val[DID_F1EF_DATA_LEN];
uint08                      did_f1ed_val[DID_F1ED_DATA_LEN];
uint08                      did_f1ee_val[DID_F1EE_DATA_LEN];
uint08                      did_f1a8_val[DID_F1A8_DATA_LEN];

static const uint08 s_did_0200_data[DID_0200_DATA_LEN] = {0};
static const uint08 s_did_0201_data[DID_0201_DATA_LEN] = {0};
static const uint08 s_did_f180_data[DID_F180_DATA_LEN] = {'0'+ LOCAL_BOOT_VER_H , '.', '0','0' + LOCAL_BOOT_VER_M , '.','0','0' + LOCAL_BOOT_VER_L ,0,0,0};
static const uint08 s_did_f186_data[DID_F186_DATA_LEN] = {2};
static const uint08 s_did_f190_data[DID_F190_DATA_LEN] = {0};
static const uint08 s_did_f193_data[DID_F193_DATA_LEN] =  {'0'+ LOCAL_HW_VER_H , '.', '0','0' + LOCAL_HW_VER_M , '.','0','0' + LOCAL_HW_VER_L ,0,0,0};
static const uint08 s_did_f194_data[DID_F194_DATA_LEN] =  {'0'+ LOCAL_HW_VER_H , '.', '0','0' + LOCAL_HW_VER_M , '.','0','0' + LOCAL_HW_VER_L ,0,0,0};
static const uint08 s_did_f195_data[DID_F195_DATA_LEN] =  {'0'+ LOCAL_SW_VER_H , '.', '0','0' + LOCAL_SW_VER_M , '.','0','0' + LOCAL_SW_VER_L ,0,0,0};
static const uint08 s_did_f199_data[DID_F199_DATA_LEN] = {0X20,0x26,0x6,0x24};
static const uint08 s_did_f1ef_data[DID_F1EF_DATA_LEN] = {1};
static const uint08 s_did_f1ed_data[DID_F1ED_DATA_LEN] = {0};
static const uint08 s_did_f1ee_data[DID_F1EE_DATA_LEN] = {0};
static const uint08 s_did_f1a8_data[DID_F1A8_DATA_LEN] = {0};
//static uint08 s_did_0100_data[16] = {0};
//static uint08 s_did_f190_data[17] = {0};
//static uint08 s_did_f198_data[10] = {0};
//static uint08 s_did_f199_data[4] = {0};
//static uint08 s_did_f19d_data[4] = {0};

static void uds_user_period_10ms_process(void);
static void uds_vol_sts_process(void);
static eUDS_VOL_TYP uds_actual_vol_typ_get(uint16 vol);
static void uds_service_process(void);
static void uds_service_11_process(void);
static void uds_service_28_process(void);
static void uds_service_85_process(void);
static void uds_service_2e_process(void);
static void uds_service_14_process(void);
static void uds_service_2f_process(void);
static void uds_service_31_process(void);
static void uds_user_swc_to_bsw(void);
static void uds_user_bsw_to_swc(void);
static void uds_did_get_from_swc(void);

void uds_user_init(void)
{
    uint16 i = 0;

    cantp_init();
    dcm_init();
    s_uds_vol_typ = UDS_VOL_NORMAL;
    s_uds_vol = 240U;
    s_uds_vol_sample_cnt = 0;
    common_memset((uint08*)&s_uds_user_data, 0U, sizeof(s_uds_user_data));
    #ifdef UDS_RX_DATA_COPY_FORM_TASK_EN
    common_memset((uint08*)&s_uds_rx0_flg, 0U, sizeof(s_uds_rx0_flg));
    #endif
    common_memset((uint08*)&s_service_11, 0U, sizeof(stSERVICE_11_CTRL));
    common_memset((uint08*)&s_service_28, 0U, sizeof(stSERVICE_28_CTRL));
    common_memset((uint08*)&g_service_22, 0U, sizeof(stSERVICE_22_CTRL));
    common_memset((uint08*)&g_service_2e, 0U, sizeof(stSERVICE_2E_CTRL));
    common_memset((uint08*)&s_service_2f, 0U, sizeof(stSERVICE_2F_CTRL));
    common_memset((uint08*)s_uu_time, 0U, sizeof(s_uu_time));

    for (i = 0; i < DCM_DID_NUM; i++)
    {
        s_did_val_len_tbl[i] = g_dcm_did_cfg[i].did_len;
    }

    g_service_22.did_val_point[DID_TYP_0200] = (uint08*)did_0200_val;
    g_service_22.did_val_point[DID_TYP_0201] = (uint08*)did_0201_val;
    g_service_22.did_val_point[DID_TYP_F180] = (uint08*)did_f180_val;
    g_service_22.did_val_point[DID_TYP_F186] = (uint08*)did_f186_val;
    g_service_22.did_val_point[DID_TYP_F190] = (uint08*)did_f190_val;
    g_service_22.did_val_point[DID_TYP_F194] = (uint08*)did_f194_val;
    g_service_22.did_val_point[DID_TYP_F195] = (uint08*)did_f195_val;
    g_service_22.did_val_point[DID_TYP_F193] = (uint08*)did_f193_val;
    g_service_22.did_val_point[DID_TYP_F199] = (uint08*)did_f199_val;
    g_service_22.did_val_point[DID_TYP_F1EF] = (uint08*)did_f1ef_val;
    g_service_22.did_val_point[DID_TYP_F1ED] = (uint08*)did_f1ed_val;
    g_service_22.did_val_point[DID_TYP_F1EE] = (uint08*)did_f1ee_val;
    g_service_22.did_val_point[DID_TYP_F1A8] = (uint08*)did_f1a8_val;
    
     uint8_t sw_ver[6];


    eeprom_drv_read(DID_F199_ADDR, (uint08*)did_f199_val, DID_F199_DATA_LEN);
    eeprom_drv_read(DID_0200_ADDR, (uint08*)did_0200_val, DID_0200_DATA_LEN);
    eeprom_drv_read(DID_0201_ADDR, (uint08*)did_0201_val, DID_0201_DATA_LEN);
    if(did_0200_val[0] == 0xff)
      common_memcpy((uint08*)did_0200_val, (uint08*)s_did_0200_data, DID_0200_DATA_LEN);
    if(did_0201_val[0] == 0xff)
      common_memcpy((uint08*)did_0201_val, (uint08*)s_did_0201_data, DID_0201_DATA_LEN);
    common_memcpy((uint08*)did_f180_val, (uint08*)s_did_f180_data, DID_F180_DATA_LEN);
    common_memcpy((uint08*)did_f186_val, (uint08*)s_did_f186_data, DID_F186_DATA_LEN);
//    common_memcpy((uint08*)did_f190_val, (uint08*)s_did_f190_data, DID_F190_DATA_LEN);
    common_memcpy((uint08*)did_f195_val, (uint08*)s_did_f195_data, DID_F195_DATA_LEN);
    common_memcpy((uint08*)did_f193_val, (uint08*)s_did_f193_data, DID_F193_DATA_LEN);
    common_memcpy((uint08*)did_f194_val, (uint08*)s_did_f194_data, DID_F194_DATA_LEN);

    if(did_f199_val[0] == 0xff)
      common_memcpy((uint08*)did_f199_val, (uint08*)s_did_f199_data, DID_F199_DATA_LEN);
    
    common_memcpy((uint08*)did_f1ef_val, (uint08*)s_did_f1ef_data, DID_F1EF_DATA_LEN);
    common_memcpy((uint08*)did_f1ed_val, (uint08*)s_did_f1ed_data, DID_F1ED_DATA_LEN);
    common_memcpy((uint08*)did_f1ee_val, (uint08*)s_did_f1ee_data, DID_F1EE_DATA_LEN);
//    common_memcpy((uint08*)did_f1a8_val, (uint08*)s_did_f1a8_data, DID_F1A8_DATA_LEN);
    eeprom_app_item_read(EE_WR_ITEM_DID_F190);
    eeprom_app_item_read(EE_WR_ITEM_DID_F1A8);
    
    
    common_memcpy((uint08*)sw_ver, (uint08*)APP_USER_M0_INFO_ADDR, 6);
    did_f194_val[0] = sw_ver[0]+'0';
    did_f194_val[3] = sw_ver[1]+'0';
    did_f194_val[6] = sw_ver[2]+'0';
    if(sw_ver[3] != 0xff)
    {
      did_f193_val[0] = sw_ver[3]+'0';
      did_f193_val[3] = sw_ver[4]+'0';
      did_f193_val[6] = sw_ver[5]+'0';
    
    }

    common_memcpy((uint08*)sw_ver, (uint08*)APP_USER_M4_INFO_ADDR, 3);
    did_f195_val[0] = sw_ver[0]+'0';
    did_f195_val[3] = sw_ver[1]+'0';
    did_f195_val[6] = sw_ver[2]+'0';

    common_memcpy((uint08*)sw_ver, (uint08*)(BOOT_START_ADDR + 0x409), 3);
    did_f180_val[0] = sw_ver[0]+'0';
    did_f180_val[3] = sw_ver[1]+'0';
    did_f180_val[6] = sw_ver[2]+'0';

}

void uds_user_process(void)
{    
//    cantp_mainfunction();
    static uint08 uds_user_main_cnt = 0;
    
//    cantp_mainfunction();
    dcm_main_function();
    bootloader_main_process();
    uds_vol_sts_process();
    uds_service_process();

    uds_user_main_cnt++;
    if (uds_user_main_cnt >= 5)
    {
        uds_user_main_cnt = 0;
//        dcm_period_10ms_process();
        uds_user_period_10ms_process();
        uds_user_swc_to_bsw();
        uds_user_bsw_to_swc();
    }
    else {}
}

void can0_uds_rx_cbk(uint32 canid, uint08* pdata, uint32 dlc)
{
    uint08 i = 0;
    stCANIF_CFG* canif_cfg = (stCANIF_CFG*)g_canif_cfg_tbl;
    
    for (i = 0; i < UDS_USER_RX0_NUM; i++)
    {
        if (   (CAN_CHN_0 == canif_cfg[i].chn)
            && (CANIF_RX == canif_cfg[i].dir)
            && (canid == canif_cfg[i].canid)
            )
        {
            common_memcpy((uint08*)s_uds_user_data[CAN_CHN_0], pdata, canif_cfg[i].dlc);
            #ifdef UDS_RX_DATA_COPY_FORM_TASK_EN
            s_uds_rx0_flg[i] = UDS_RX_DATA_UNUSED;
            #else
            canif_rx_indication(canif_cfg[i].hoh,
                                canif_cfg[i].canid,
                                canif_cfg[i].dlc,
                                (uint08*)s_uds_user_data[CAN_CHN_0]);
            #endif
            break;
        }
        else { /* continue */ }
    }
}

void can1_uds_rx_cbk(uint32 canid, uint08* pdata, uint32 dlc)
{
    uint08 i = 0;
    stCANIF_CFG* canif_cfg = (stCANIF_CFG*)g_canif_cfg_tbl;
    
    for (i = 0; i < UDS_USER_RX1_NUM; i++)
    {
        if (   (CAN_CHN_1 == canif_cfg[i].chn)
            && (CANIF_RX == canif_cfg[i].dir)
            && (canid == canif_cfg[i].canid)
            )
        {
            common_memcpy((uint08*)s_uds_user_data[CAN_CHN_1], pdata, canif_cfg[i].dlc);
            #ifdef UDS_RX_DATA_COPY_FORM_TASK_EN
            s_uds_rx0_flg[i] = UDS_RX_DATA_UNUSED;
            #else
            canif_rx_indication(canif_cfg[i].hoh,
                                canif_cfg[i].canid,
                                canif_cfg[i].dlc,
                                (uint08*)s_uds_user_data[CAN_CHN_1]);
            #endif
            break;
        }
        else { /* continue */ }
    }
}

void can1_uds_tx_cbk(uint32 canid)
{
    uint08 i = 0;
    stCANIF_CFG* canif_cfg = (stCANIF_CFG*)g_canif_cfg_tbl;

    for (i = CANIF_HRH_NUM; i < UDS_USER_TX1_NUM; i++)
    {
        if (   (CAN_CHN_1 == canif_cfg[i].chn)
            && (CANIF_TX == canif_cfg[i].dir)
            && (canid == canif_cfg[i].canid)
            )
        {
            canif_tx_confirm(i);
            break;
        }
        else { /* continue */ }
    }
}


void can2_uds_rx_cbk(uint32 canid, uint08* pdata, uint32 dlc)
{
    uint08 i = 0;
    stCANIF_CFG* canif_cfg = (stCANIF_CFG*)g_canif_cfg_tbl;
    
    for (i = 0; i < UDS_USER_RX2_NUM; i++)
    {
        if (   (CAN_CHN_2 == canif_cfg[i].chn)
            && (CANIF_RX == canif_cfg[i].dir)
            && (canid == canif_cfg[i].canid)
            )
        {
            common_memcpy((uint08*)s_uds_user_data[CAN_CHN_2], pdata, canif_cfg[i].dlc);
            #ifdef UDS_RX_DATA_COPY_FORM_TASK_EN
            s_uds_rx2_flg[i] = UDS_RX_DATA_UNUSED;
            #else
            canif_rx_indication(canif_cfg[i].hoh,
                                canif_cfg[i].canid,
                                dlc,
                                (uint08*)s_uds_user_data[CAN_CHN_2]);
            #endif
            break;
        }
        else { /* continue */ }
    }
}

void can2_uds_tx_cbk(uint32 canid)
{
    uint08 i = 0;
    stCANIF_CFG* canif_cfg = (stCANIF_CFG*)g_canif_cfg_tbl;

    for (i = CANIF_HRH_NUM; i < UDS_USER_TX2_NUM; i++)
    {
        if (   (CAN_CHN_2 == canif_cfg[i].chn)
            && (CANIF_TX == canif_cfg[i].dir)
            && (canid == canif_cfg[i].canid)
            )
        {
            canif_tx_confirm(i);
            break;
        }
        else { /* continue */ }
    }
}

/* You are advised to call it once every 1 to 10ms */
void uds_can_rx_process(void)
{
    #ifdef UDS_RX_DATA_COPY_FORM_TASK_EN
    stCANIF_CFG* canif_cfg = (stCANIF_CFG*)g_canif_cfg_tbl;
    uint08 i = 0;

    for (i = 0; i < CANIF_HRH_UDS_NUM; i++)
    {

        if (UDS_RX_DATA_UNUSED == s_uds_rx0_flg[i])
        {
            s_uds_rx0_flg[i] = UDS_RX_DATA_USED;
            canif_rx_indication(canif_cfg[i].hoh,
                                canif_cfg[i].canid,
                                canif_cfg[i].dlc,
                                (uint08*)s_uds_user_data[canif_cfg[i].chn]);
            break; /* Only one frame of the UDS packet can be processed */
        }
        else
        {}
    }
    #endif
}

uint16 uds_vechicle_speed_get(void)
{
    return g_vehicle_speed;
}

eUDS_VOL_TYP uds_vol_typ_get(void)
{
    return s_uds_vol_typ;
}

void dcm_service_85_dtcsettingtype_set(uint08 val)
{
    s_service_85.dtc_setting_ctrl = val;
}

uint08 dcm_service_85_dtcsettingtype_get(void)
{
    return s_service_85.dtc_setting_ctrl;
}


void dcm_service_11_cbk(void* cbk_param1, void* cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;

    (void)cbk_param2;

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        s_service_11.reset = (msg->req_data[1] & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT));
        msg->nrc = NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING;
        msg->res_data_len = 2U;
        common_memcpy((uint08*)msg->res_data, (uint08*)msg->req_data, msg->res_data_len);
    }
    else {}
}

void dcm_service_28_cbk(void* cbk_param1, void* cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;

    (void)cbk_param2;

    s_service_28.control_type = msg->req_data[1];
    s_service_28.communication_type = msg->req_data[2];
}

void dcm_service_85_cbk(void* cbk_param1, void* cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;

    (void)cbk_param2;

    s_service_85.dtc_setting_ctrl = msg->req_data[1];
}

void dcm_service_31_cbk(void * cbk_param1, void * cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    uint16 req_rid = 0;
    uint32 temp = 0;
	
    (void)cbk_param2;
    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
    	temp = msg->req_data[2];
	    temp <<= 8U;
	    temp += msg->req_data[3];
	    req_rid = temp;
//		if (RID_SELF_CHK_CMD == req_rid)
//		{
//			if (DCM_SID31_SUB_01 == msg->req_data[1]) /* start */
//			{
////				rtU.BSW_RID_SELFTEST_START = 1;
//				msg->res_data[4] = 0; /* Success by default */
//				msg->res_data_len += 1;
//			}
//			else if (DCM_SID31_SUB_02 == msg->req_data[1]) /* stop */
//			{
////				rtU.BSW_RID_SELFTEST_START = 0;
//				msg->res_data[4] = 0; /* Success by default */
//				msg->res_data_len += 1;
//			}
//			else if (DCM_SID31_SUB_03 == msg->req_data[1]) /* result */
//			{
////				msg->res_data[4] = rtY.BSW_RID_SELFTEST_STS_FB;
//				msg->res_data_len += 1;
//			}
//			else {}
//		}
    }
    else { /* nothing */ }
}

void dcm_service_22_cbk(void* cbk_param1, void* cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    stSID22_DP_INFO* dp = (stSID22_DP_INFO*)cbk_param2;
    uint16 i = 0;
    uint16 index = 1;

    if (NRC_POSITIVE_RESPONSE != msg->nrc)
    {
        return;
    }
    else { /* continue */ }
    
    for (i = 0; i < dp->did_num; i++)
    {
        msg->res_data[index] = ((dp->did[i] >> 8) & 0xFF);
        msg->res_data[index + 1] = (dp->did[i] & 0xFF);
        index += 2;
        common_memcpy((uint08*)&msg->res_data[index], 
                      (uint08*)g_service_22.did_val_point[dp->typ[i]],
                      s_did_val_len_tbl[dp->typ[i]]);
        index += (s_did_val_len_tbl[dp->typ[i]]);
        msg->res_data_len += (s_did_val_len_tbl[dp->typ[i]] + 2);
    }
}

static void uds_service_85_process(void)
{
    if (SESSION_TYPE_DEFAULT == dcm_session_get())
    {
        s_service_85.dtc_setting_ctrl = SERVICE_85_DTCSETTINGTYPE_ON;
    }
    else {}
}

void dcm_service_2e_cbk(void* cbk_param1, void* cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    stSID2E_DP_INFO* dp = (stSID2E_DP_INFO*)cbk_param2;

    if (NRC_POSITIVE_RESPONSE != msg->nrc)
    {
        return;
    }
    else { /* continue */ }
    g_service_2e.did_typ = dp->typ;
    g_service_2e.did_len = dp->len;
    g_service_2e.did_wr_en = 1;
    common_memcpy((uint08*)g_service_2e.did_val, (uint08*)&msg->req_data[3], dp->len);
}

void dcm_service_14_cbk(void* cbk_param1, void* cbk_param2)
{
//    #if 0
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    stSID14_DP_INFO* dp = DEF_NULL;

    if (NRC_POSITIVE_RESPONSE != msg->nrc)
    {
        return;
    }
    else { /* continue */ }
    dp = (stSID14_DP_INFO*)cbk_param2;
    s_service_14.dtc = dp->dtc;
    s_service_14.dtc_typ = dp->typ;
//    #endif
}

void dcm_service_19_cbk(void* cbk_param1, void* cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
//    stSID14_DP_INFO* dp = DEF_NULL;

    if (NRC_POSITIVE_RESPONSE != msg->nrc)
    {
        return;
    }
    else { /* continue */ }
//    dp = (stSID2E_DP_INFO*)cbk_param2;
}

void dcm_service_2f_cbk(void* cbk_param1, void* cbk_param2)
{
    #if 0
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    stSID2F_DP_INFO* dp = (stSID2F_DP_INFO*)cbk_param2;
    uint08 cs = 0;
    uint32 temp = 0;

    if (NRC_POSITIVE_RESPONSE != msg->nrc)
    {
        return;
    }
    else { /* continue */ }

    s_uu_time[UU_TIME_SERVICE_2F] = 0;
    cs = msg->req_data[4];
    switch (dp->did)
    {
        case DID_4001:
            if (IOCP_RETURN_CONTROL_TO_ECU == dp->iocp)
            {
                s_service_2f.en &= ~(1 <<  UDS_2F_DID_INDX_4001);
            }
            else if (IOCP_SHORT_TERM_ADJUSTMENT == dp->iocp)
            {
                s_service_2f.en |= (1 <<  UDS_2F_DID_INDX_4001);
                s_service_2f.did_4001 = cs;
            }
            else
            { /* nothing */ }
            break;
        case DID_4002:
            if (IOCP_RETURN_CONTROL_TO_ECU == dp->iocp)
            {
                s_service_2f.en &= ~(1 <<  UDS_2F_DID_INDX_4002);
            }
            else if (IOCP_SHORT_TERM_ADJUSTMENT == dp->iocp)
            {
                s_service_2f.en |= (1 <<  UDS_2F_DID_INDX_4002);
                s_service_2f.did_4002 = cs;
            }
            break;
        case DID_4003:
            if (IOCP_RETURN_CONTROL_TO_ECU == dp->iocp)
            {
                s_service_2f.en &= ~(1 <<  UDS_2F_DID_INDX_4003);
            }
            else if (IOCP_SHORT_TERM_ADJUSTMENT == dp->iocp)
            {
                s_service_2f.en |= (1 <<  UDS_2F_DID_INDX_4003);
                if (cs > 0xF0)
                {
                    s_service_2f.did_4003 = 12000U;
                }
                else
                {
                    s_service_2f.did_4003 = ((uint32)cs * 50U);
                }
            }
            break;
        case DID_4004:
            if (IOCP_RETURN_CONTROL_TO_ECU == dp->iocp)
            {
                s_service_2f.en &= ~(1 <<  UDS_2F_DID_INDX_4004);
            }
            else if (IOCP_SHORT_TERM_ADJUSTMENT == dp->iocp)
            {
                s_service_2f.en |= (1 <<  UDS_2F_DID_INDX_4004);
                if (cs > 100)
                {
                    s_service_2f.did_4004 = 100;
                }
                else
                {
                    s_service_2f.did_4004 = cs;
                }
            }
            break;
        case DID_4005:
            if (IOCP_RETURN_CONTROL_TO_ECU == dp->iocp)
            {
                s_service_2f.en &= ~(1 <<  UDS_2F_DID_INDX_4005);
            }
            else if (IOCP_SHORT_TERM_ADJUSTMENT == dp->iocp)
            {
                s_service_2f.en |= (1 <<  UDS_2F_DID_INDX_4005);
                if (cs > 100)
                {
                    s_service_2f.did_4005 = 100;
                }
                else
                {
                    s_service_2f.did_4005 = cs;
                }
            }
            break;
        case DID_4006:
            if (IOCP_RETURN_CONTROL_TO_ECU == dp->iocp)
            {
                s_service_2f.en &= ~(1 <<  UDS_2F_DID_INDX_4006);
            }
            else if (IOCP_SHORT_TERM_ADJUSTMENT == dp->iocp)
            {
                s_service_2f.en |= (1 <<  UDS_2F_DID_INDX_4006);
                if (cs > 100)
                {
                    s_service_2f.did_4006 = 100;
                }
                else
                {
                    s_service_2f.did_4006 = cs;
                }
            }
            break;
        default:
            break;
    }
    #endif
}

static void uds_vol_sample_process(void)
{
    uint32 u32_temp = 0;
    float r32_temp = 0.0F;
    float real_vol = 0.0F;
    static uint32 s_uds_pre_vol = 0;

    s_uds_vol_sample_cnt++;
    if (s_uds_vol_sample_cnt >= 15U) /* 150ms delay */
    {
        s_uds_vol_sample_cnt = 0;
//        real_vol = (float)adc_original_val_get(ADC_TYP_KL30) / 4096.0F * 5.0F * 11.0F;
        r32_temp = real_vol;
        r32_temp *= 10.0F;
        u32_temp = (uint32)r32_temp;
        s_uds_vol = u32_temp;
        s_uds_vol = u32_temp + 10; /* voltage compensation 1V */
    }
    else {}
}

void uds_user_period_10ms_process(void)
{
    uint08 i = 0;
    static uint16 s_vol_print_cnt = 0;

    for (i = 0; i < UU_TIME_NUM; i++)
    {
        if (s_uu_time[i] < 0xFFFFFFFF)
        {
            s_uu_time[i]++;
        }
    }

    if (SESSION_TYPE_EXTENDED == dcm_session_get())
    {
        s_uu_time[UU_TIME_SERVICE_2F] = 0;
        s_service_2f.en = 0;
    }
    else
    {}

    #ifdef UDS_USER_DEBUG
    t_send_test_cnt++;
    if (t_send_test_cnt >= 200)
    {
        t_send_test_cnt = 0;
        pcan_test_can_send();
    }
    #endif

    uds_vol_sample_process();

    s_vol_print_cnt++;
    if (s_vol_print_cnt >= 100)
    {
        s_vol_print_cnt = 0;
//        CAN_DEBUG_PRINT("[UDS USER]battery voltage = %d.%dV.\r\n", s_uds_vol / 10, s_uds_vol % 10);
    }
}

static void uds_vol_sts_process(void)
{
    eUDS_VOL_TYP typ = s_uds_vol_typ;
    eUDS_VOL_TYP actual_vol_typ = uds_actual_vol_typ_get(s_uds_vol);
    
//    switch (typ)
//    {
//        case UDS_VOL_START:
//            if (s_uu_time[UU_TIME_UDS_VOL_START] >= UDS_DIAG_START_TIME)
//            {
//                typ = UDS_VOL_NORMAL;
//                CAN_DEBUG_PRINT("CAN UDS start.\r\n");
//            }
//            else {}
//            break;
//        case UDS_VOL_RESTART:
//            if (s_uu_time[UU_TIME_UDS_VOL_RESTART] >= UDS_DIAG_RESTART_TIME)
//            {
//                typ = UDS_VOL_NORMAL;
//                CAN_DEBUG_PRINT("CAN UDS restart.\r\n");
//            }
//            else if (UDS_VOL_OVER == actual_vol_typ)
//            {
//                typ = UDS_VOL_OVER;
//                CAN_DEBUG_PRINT_WRN("The UDS detects a high voltage during recovery!\r\n");
//            }
//            else if (UDS_VOL_UNDER == actual_vol_typ)
//            {
//                typ = UDS_VOL_UNDER;
//                CAN_DEBUG_PRINT_WRN("The UDS detects a high voltage during recovery!\r\n");
//            }
//            else {}
//            break;
//        case UDS_VOL_NORMAL:
//            if (UDS_VOL_OVER == actual_vol_typ)
//            {
//                typ = UDS_VOL_OVER;
//                CAN_DEBUG_PRINT_WRN("The UDS detects that the voltage is too high!\r\n");
//            }
//            else if (UDS_VOL_UNDER == actual_vol_typ)
//            {
//                typ = UDS_VOL_UNDER;
//                CAN_DEBUG_PRINT_WRN("The UDS detects that the voltage is too low!\r\n");
//            }
//            else {}
//            break;
//        case UDS_VOL_OVER:
//            if (UDS_VOL_NORMAL == actual_vol_typ)
//            {
//                typ = UDS_VOL_RESTART;
//                s_uu_time[UU_TIME_UDS_VOL_RESTART] = 0;
//            }
//            else {}
//            break;
//        case UDS_VOL_UNDER:
//            if (UDS_VOL_NORMAL == actual_vol_typ)
//            {
//                typ = UDS_VOL_RESTART;
//                s_uu_time[UU_TIME_UDS_VOL_RESTART] = 0;
//            }
//            else {}
//            break;
//    }

    s_uds_vol_typ = typ;
}

static eUDS_VOL_TYP uds_actual_vol_typ_get(uint16 vol)
{
    if (vol >= UDS_DIAG_VOL_HIGH)
    {
        return UDS_VOL_OVER;
    }
    else if (vol <= UDS_DIAG_VOL_LOW)
    {
        return UDS_VOL_UNDER;
    }
    else
    {
        return UDS_VOL_NORMAL;
    }
}
static void uds_service_process(void)
{
    did_f186_val[0] = (uint8_t)dcm_session_get() + 1;
    uds_service_11_process();
    uds_service_28_process();
    uds_service_85_process();
    uds_service_14_process();
    uds_service_2e_process();
    uds_service_2f_process();
    uds_service_31_process();
}

static void uds_service_11_process(void)
{
    if ((SERVICE_11_HARD_RESET == s_service_11.reset) || (SERVICE_11_SOFT_RESET == s_service_11.reset))
    {
        if ((s_uu_time[UU_TIME_SERVICE_11] >= SERVICE_11_RESET_TIME_DLY) && (NRC_78_INIT == g_service_11_nrc78_result))
        {
            g_service_11_nrc78_result = NRC_78_POS_RESPONSE;
            s_uu_time[UU_TIME_SERVICE_11] = 0;
            dcm_unconditional_session_ctrl(SESSION_TYPE_DEFAULT);
        }
        else if ((NRC_78_PEDING == g_service_11_nrc78_result) && (s_uu_time[UU_TIME_SERVICE_11] >= SERVICE_11_RESET_TIME_DLY + 5))
        {
            g_service_11_nrc78_result = NRC_78_INIT;
//            SystemSoftwareReset();
            Cy_SysReset_SoftResetCM4();
            NVIC_SystemReset();
        }
        else {/* wait */}
    }
    else
    {
        s_uu_time[UU_TIME_SERVICE_11] = 0;
    }
}

static void uds_service_28_process(void)
{
    if (   (DCM_NORMAL_COMM_MSG == s_service_28.communication_type)
        || (DCM_NETWORK_NORMAL_COMM_MSG == s_service_28.communication_type)
        )
    {
        if (DCM_SID28_SUB_00 == s_service_28.control_type)
        {
            can0_trx_ctrl_by_uds_cbk(0, 0);
        }
        else if (DCM_SID28_SUB_01 == s_service_28.control_type)
        {
            can0_trx_ctrl_by_uds_cbk(1, 0);
        }
        else if (DCM_SID28_SUB_02 == s_service_28.control_type)
        {
            can0_trx_ctrl_by_uds_cbk(0, 1);
        }
        else if (DCM_SID28_SUB_03 == s_service_28.control_type)
        {
            can0_trx_ctrl_by_uds_cbk(1, 1);
        }
        else {}
    }
    else
    {/* nothing */}
}

static void uds_service_2e_process(void)
{
    eEE_CTRL_ERR err = EE_CTRL_ERR_NONE;
    eEE_WR_ITEM_TYP item = EE_WR_ITEM_NUM;
    uint32 addr = 0xFFFFFFFF;

    if ((g_service_2e.did_wr_en) && (s_uu_time[UU_TIME_SERVICE_2E] >= 10))
    {
        if (DID_TYP_F190 == g_service_2e.did_typ)
        {
            addr = DID_F190_ADDR;
            common_memcpy((uint08*)g_service_22.did_val_point[DID_TYP_F190], (uint08*)g_service_2e.did_val, DID_F190_DATA_LEN);
        }
        else if (DID_TYP_F1A8 == g_service_2e.did_typ)
        {
            addr = DID_F1A8_ADDR;
            common_memcpy((uint08*)g_service_22.did_val_point[DID_TYP_F1A8], (uint08*)g_service_2e.did_val, DID_F1A8_DATA_LEN);
        }

        if (addr != 0xFFFFFFFF)
        {
            eeprom_drv_write(addr, g_service_2e.did_val, g_service_2e.did_len);
        }
        else {}
        g_service_2e.did_wr_en = 0;
    }
    else {}
    #if 0
    if (g_service_2e.did_wr_en)
    {
        if (DID_TYP_0100 == g_service_2e.did_typ)
        {
            addr = EE_DID_0100_ADDR;
            item = EE_WR_ITEM_1;
            common_memcpy((uint08*)g_service_22.did_val_point[DID_TYP_0100], (uint08*)g_service_2e.did_val, DID_0100_DATA_LEN);
        }
        else if (DID_TYP_F190 == g_service_2e.did_typ)
        {
            addr = EE_DID_F190_ADDR;
            item = EE_WR_ITEM_2;
            common_memcpy((uint08*)g_service_22.did_val_point[DID_TYP_F190], (uint08*)g_service_2e.did_val, DID_F190_DATA_LEN);
        }
        else if (DID_TYP_F198 == g_service_2e.did_typ)
        {
            addr = EE_DID_F198_ADDR;
            item = EE_WR_ITEM_3;
            common_memcpy((uint08*)g_service_22.did_val_point[DID_TYP_F198], (uint08*)g_service_2e.did_val, DID_F198_DATA_LEN);
        }
        else if (DID_TYP_F199 == g_service_2e.did_typ)
        {
            addr = EE_DID_F199_ADDR;
            item = EE_WR_ITEM_4;
            common_memcpy((uint08*)g_service_22.did_val_point[DID_TYP_F199], (uint08*)g_service_2e.did_val, DID_F199_DATA_LEN);
        }
        else if (DID_TYP_F19D == g_service_2e.did_typ)
        {
            addr = EE_DID_F19D_ADDR;
            item = EE_WR_ITEM_5;
            common_memcpy((uint08*)g_service_22.did_val_point[DID_TYP_F19D], (uint08*)g_service_2e.did_val, DID_F19D_DATA_LEN);
        }
        else {}

        err = eeprom_app_wr_err_get(item);
        if ((EE_CTRL_ERR_NONE == err) || (EE_CTRL_ERR_WR_SUCCESS == err) || (EE_CTRL_ERR_BUF_OVF == err))
        {
            eeprom_app_buf_write(item, addr, g_service_2e.did_len, g_service_2e.did_val);
            g_service_2e.did_wr_en = 0;
        }
        else {}  
    }
    else {}
    #endif
}

static void uds_service_14_process(void)
{
//    #if 0
    if (0xFFFFFF == s_service_14.dtc)
    {
        dtc_all_clear();
        s_service_14.dtc = 0;
        if (s_service_14.dtc_typ < CAN_DTC_NUM)
        {
            s_service_14.dtc_typ = CAN_DTC_NUM;
        }
        else {}
    }
    else if (s_service_14.dtc != 0)
    {
        s_service_14.dtc = 0;
        dtc_clear(s_service_14.dtc_typ);
        if (s_service_14.dtc_typ < CAN_DTC_NUM)
        {
            s_service_14.dtc_typ = CAN_DTC_NUM;
        }
        else {}
    }
    else {}
//    #endif
}

static void uds_service_2f_process(void)
{    
    #if 0
    /* BSW ---> SWC */
    rtU.BSW_UDS_2F_Ctrl_En = ((s_service_2f.en == 0) ? 0 : 1 );
    rtU.BSW_UDS_2F_4001 = s_service_2f.did_4001;
    rtU.BSW_UDS_2F_4002 = s_service_2f.did_4002;
    rtU.BSW_UDS_2F_4003 = s_service_2f.did_4003;
    rtU.BSW_UDS_2F_4004 = s_service_2f.did_4004;
    rtU.BSW_UDS_2F_4005 = s_service_2f.did_4005;
    rtU.BSW_UDS_2F_4006 = s_service_2f.did_4006;
    #endif
}

static void uds_service_31_process(void)
{
//	if (0x01 == rtU.BSW_RID_SELFTEST_START) /* start or result */
//	{
//		if (SESSION_TYPE_DEFAULT == dcm_session_get())
//		{
//            rtU.BSW_RID_SELFTEST_START = 0;
//        }
//        else {}
//	}
//	else if (0x00 == rtU.BSW_RID_SELFTEST_START) /* stop */
//	{
//		//rtU.BSW_RID_SELFTEST_START = 0;
//	}
//	else {}
}

static void uds_user_swc_to_bsw(void)
{
//    uds_did_get_from_swc();
}

static void uds_user_bsw_to_swc(void)
{
//    rtU.BSW_UDS_2F_Ctrl_En = ((s_service_2f.en == 0) ? 0 : 1 );
//    rtU.BSW_UDS_2F_4001 = s_service_2f.did_4001;
//    rtU.BSW_UDS_2F_4002 = s_service_2f.did_4002;
//    rtU.BSW_UDS_2F_4003 = s_service_2f.did_4003;
//    rtU.BSW_UDS_2F_4004 = s_service_2f.did_4004;
//    rtU.BSW_UDS_2F_4005 = s_service_2f.did_4005;
//    rtU.BSW_UDS_2F_4006 = s_service_2f.did_4006;
}

static void uds_did_get_from_swc(void)
{
//    uint32 u32_temp = 0;
//    sint32 s32_temp = 0;
//    uint08 u08_temp = 0;
//    uint16 u16_temp = 0;
//    real32_T r32_temp = 0.0F;
//    real64_T r64_temp = 0.0;
//
//    u32_temp = s_uds_vol;
//    u32_temp >>= 1U;
//    if (u32_temp > 250U)
//    {
//        u32_temp = 250U;
//    }
//    else {}
//    u08_temp = (uint08)u32_temp;
//    did_3001_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_3004;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_3004_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_3006;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_3006_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_3007;
//    r32_temp *= 10.0F;
//    u32_temp = (uint32)r32_temp;
//    u32_temp /= 2U;
//    if (u32_temp > 250)
//    {
//        u32_temp = 250;
//    }
//    else {}
//    u08_temp = (uint08)u32_temp;
//    did_3007_val[0] = u08_temp;
//
////    r64_temp = rtY.BSW_UDS_22_3009;
//    u32_temp = (uint32)r64_temp;
//    u32_temp /= 50;
//    if (u32_temp > 240)
//    {
//        u32_temp = 240;
//    }
//    else {}
//    u08_temp = (uint08)u32_temp;
//    did_3009_val[0] = u08_temp;
//
////    r64_temp = rtY.BSW_UDS_22_300A;
//    r64_temp *= 100.0;
//    u32_temp = (uint32)r64_temp;
//    u32_temp /= 25;
//    if (u32_temp > 250)
//    {
//        u32_temp = 250;
//    }
//    else {}
//    u08_temp = (uint08)u32_temp;
//    did_300a_val[0] = u08_temp;
//
////    r64_temp = rtY.BSW_UDS_22_300B;
//    u32_temp = (uint32)r64_temp;
//    u32_temp /= 4;
//    if (u32_temp > 255)
//    {
//        u32_temp = 255;
//    }
//    else {}
//    u08_temp = (uint08)u32_temp;
//    did_300b_val[0] = u08_temp;
//
////    u08_temp = rtY.BSW_UDS_22_300D;
//    if (u08_temp > 100)
//    {
//        u32_temp = 100;
//    }
//    else {}
//    did_300d_val[0] = u08_temp;
//
////    u16_temp = rtY.BSW_UDS_22_300E;
//    u16_temp *= 2;
//    if (u16_temp > 200)
//    {
//        u16_temp = 200;
//    }
//    else {}
//    u08_temp = (uint08)u16_temp;
//    did_300e_val[0] = u08_temp;
//
//    u16_temp = g_vehicle_spped_original;
//    if (u16_temp > 64000)
//    {
//        u16_temp = 64000;
//    }
//    else {}
//    did_300f_val[0] = ((u16_temp >> 8U) & 0xFF);
//    did_300f_val[1] = (u16_temp & 0xFF);
//
////    r64_temp = rtY.BSW_UDS_22_3010;
//    r64_temp *= 100.0;
//    u32_temp = r64_temp;
//    u32_temp /= 5;
//    u16_temp = (uint16)u32_temp;
//    if (u16_temp > 64255)
//    {
//        u16_temp = 64255;
//    }
//    else {}
//    did_3010_val[0] = ((u16_temp >> 8U) & 0xFF);
//    did_3010_val[1] = (u16_temp & 0xFF);
//
////    did_3011_val[0] = rtY.BSW_UDS_22_3011;
////
////    did_3012_val[0] = rtY.BSW_UDS_22_3012;
////
////    r32_temp = rtY.BSW_UDS_22_3013;
//    r32_temp *= 10.0F;
//    u32_temp = (uint32)r32_temp;
//    u32_temp /= 2U;
//    if (u32_temp > 250)
//    {
//        u32_temp = 250;
//    }
//    else {}
//    u08_temp = (uint08)u32_temp;
//    did_3013_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_3014;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_3014_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_3015;
//    r32_temp *= 10.0F;
//    u32_temp = (uint32)r32_temp;
//    if (u32_temp > 250)
//    {
//        u32_temp = 250;
//    }
//    else {}
//    u08_temp = (uint08)u32_temp;
//    did_3015_val[0] = u08_temp;
//
//    r32_temp = rtY.BSW_UDS_22_3016;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_3016_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_3017;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_3017_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_3018;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_3018_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_3019;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_3019_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_301A;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_301a_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_301B;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_301b_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_301C;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_301c_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_301D;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_301d_val[0] = u08_temp;
//
////    r32_temp = rtY.BSW_UDS_22_301E;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_301e_val[0] = u08_temp;
//
////    u08_temp = rtY.BSW_UDS_22_301F;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_301f_val[0] = u08_temp;
//
////    u08_temp = rtY.BSW_UDS_22_3020;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_3020_val[0] = u08_temp;
//
////    u08_temp = rtY.BSW_UDS_22_3021;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_3021_val[0] = u08_temp;
//
////    did_3022_val[0] = rtY.BSW_UDS_22_3022;
////
////    u08_temp = rtY.BSW_UDS_22_3023;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_3023_val[0] = u08_temp;
//
////    did_3024_val[0] = rtY.BSW_UDS_22_3024;
////
////    u08_temp = rtY.BSW_UDS_22_3025;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_3025_val[0] = u08_temp;
//
//    did_3026_val[0] = rtY.BSW_UDS_22_3026;
//
//    u08_temp = rtY.BSW_UDS_22_3027;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_3027_val[0] = u08_temp;
//
//    did_3028_val[0] = rtY.BSW_UDS_22_3028;
//
//    u16_temp = rtY.BSW_UDS_22_3029;
//    u16_temp /= 100U;
//    if (u16_temp > 100)
//    {
//        u16_temp = 100;
//    }
//    u08_temp = (uint08)u16_temp;
//    did_3029_val[0] = u08_temp;
//
//    u08_temp = rtY.BSW_UDS_22_302A;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_302a_val[0] = u08_temp;
//
//    u16_temp = rtY.BSW_UDS_22_302B;
//    u16_temp /= 100U;
//    if (u16_temp > 100)
//    {
//        u16_temp = 100;
//    }
//    u08_temp = (uint08)u16_temp;
//    did_302b_val[0] = u08_temp;
//
//    u08_temp = rtY.BSW_UDS_22_302C;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_302c_val[0] = u08_temp;
//
//    u08_temp = rtY.BSW_UDS_22_302D;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_302d_val[0] = u08_temp;
//
//    u08_temp = rtY.BSW_UDS_22_302E;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_302e_val[0] = u08_temp;
//
//    u08_temp = rtY.BSW_UDS_22_302F;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_302f_val[0] = u08_temp;
//
//    u08_temp = rtY.BSW_UDS_22_3030;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_3030_val[0] = u08_temp;
//
//    u08_temp = rtY.BSW_UDS_22_3031;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_3031_val[0] = u08_temp;
//
//    u08_temp = rtY.BSW_UDS_22_3032;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_3032_val[0] = u08_temp;
//
//    u08_temp = rtY.BSW_UDS_22_3033;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_3033_val[0] = u08_temp;
//
//    u08_temp = rtY.BSW_UDS_22_3034;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_3034_val[0] = u08_temp;
//
//    u08_temp = rtY.BSW_UDS_22_3035;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_3035_val[0] = u08_temp;
//
//    u08_temp = rtY.BSW_UDS_22_3036;
//    if (u08_temp > 100)
//    {
//        u08_temp = 100;
//    }
//    else {}
//    did_3036_val[0] = u08_temp;
//
//    u16_temp = rtY.BSW_UDS_22_3037;
//    if (u16_temp > 64255)
//    {
//        u16_temp = 64255;
//    }
//    else {}
//    did_3037_val[0] = ((u16_temp >> 8U) & 0xFF);
//    did_3037_val[1] = (u16_temp & 0xFF);
//
//    u16_temp = rtY.BSW_UDS_22_3038;
//    if (u16_temp > 64255)
//    {
//        u16_temp = 64255;
//    }
//    else {}
//    did_3038_val[0] = ((u16_temp >> 8U) & 0xFF);
//    did_3038_val[1] = (u16_temp & 0xFF);
//
//    r64_temp = rtY.BSW_UDS_22_3039;
//    u32_temp = (uint32)r64_temp;
//    u32_temp /= 4;
//    if (u32_temp > 255)
//    {
//        u32_temp = 255;
//    }
//    else {}
//    u08_temp = (uint08)u32_temp;
//    did_3039_val[0] = u08_temp;
//
//    r32_temp = rtY.BSW_UDS_22_303A;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 255)
//    {
//        s32_temp = 255;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_303a_val[0] = u08_temp;
//
//    r32_temp = rtY.BSW_UDS_22_303B;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 255)
//    {
//        s32_temp = 255;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_303b_val[0] = u08_temp;
//
//    u16_temp = rtY.BSW_UDS_22_303C;
//    if (u16_temp > 64255)
//    {
//        u16_temp = 64255;
//    }
//    else {}
//    did_303c_val[0] = ((u16_temp >> 8U) & 0xFF);
//    did_303c_val[1] = (u16_temp & 0xFF);
//
//    u16_temp = rtY.BSW_UDS_22_303D;
//    if (u16_temp > 64255)
//    {
//        u16_temp = 64255;
//    }
//    else {}
//    did_303d_val[0] = ((u16_temp >> 8U) & 0xFF);
//    did_303d_val[1] = (u16_temp & 0xFF);
//
//    r64_temp = rtY.BSW_UDS_22_303E;
//    u32_temp = (uint32)r64_temp;
//    u32_temp /= 4;
//    if (u32_temp > 255)
//    {
//        u32_temp = 255;
//    }
//    else {}
//    u08_temp = (uint08)u32_temp;
//    did_303e_val[0] = u08_temp;
//
//    r64_temp = rtY.BSW_UDS_22_303F;
//    u32_temp = (uint32)r64_temp;
//    u32_temp /= 4;
//    if (u32_temp > 255)
//    {
//        u32_temp = 255;
//    }
//    else {}
//    u08_temp = (uint08)u32_temp;
//    did_303f_val[0] = u08_temp;
//
//    did_3040_val[0] = rtY.BSW_UDS_22_3040;
//
//    did_3041_val[0] = rtY.BSW_UDS_22_3041;
//
//    did_3042_val[0] = rtY.BSW_UDS_22_3042;
//
//    did_3043_val[0] = rtY.BSW_UDS_22_3043;
//
//    did_3044_val[0] = rtY.BSW_UDS_22_3044;
//
//    r32_temp = rtY.BSW_UDS_22_3045;
//    r32_temp += 40.0F;
//    r32_temp *= 10.0F;
//    s32_temp = (sint32)r32_temp;
//    s32_temp /= 5;
//    if (s32_temp < 0)
//    {
//        s32_temp = 0;
//    }
//    else if (s32_temp > 250)
//    {
//        s32_temp = 250;
//    }
//    else{}
//    u08_temp = (uint08)s32_temp;
//    did_3045_val[0] = u08_temp;
}

void uds_user_test(void)
{
//    rtY.BSW_UDS_22_3001 = 12.8F;
//    rtY.BSW_UDS_22_3002 = 2;
//    rtY.BSW_UDS_22_3003 = 1;
//    rtY.BSW_UDS_22_3004 = 25.3F;
//    rtY.BSW_UDS_22_3005 = 56.4F;
//    rtY.BSW_UDS_22_3006 = 90.4F;
//    rtY.BSW_UDS_22_3007 = 20.4F;
//    rtY.BSW_UDS_22_3008 = 15.6F;
//    rtY.BSW_UDS_22_3009 = 8900.0F;
//    rtY.BSW_UDS_22_300A = 17.1F;
//    rtY.BSW_UDS_22_300B = 999.2F;
//    rtY.BSW_UDS_22_300C = 45;
//    rtY.BSW_UDS_22_300D = 45;
//    rtY.BSW_UDS_22_300E = 45.5F;
//    rtY.BSW_UDS_22_300F = 122.6;
//    rtY.BSW_UDS_22_3010 = 1989.3;
}

