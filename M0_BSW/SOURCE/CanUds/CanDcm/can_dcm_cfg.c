/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_dcm_cfg.c
* Author        : yangming
* Date          : 2024-01-23
* Version       : 1.00
* Description   : Can diagnostic communication manager configuration file.
* Others        : None
*
****************************************************************************************************/
#include "can_dcm_cfg.h"
#include "can_dcm_cbk_cfg.h"
#include "bootloader.h"
#include "can_dtc_cfg.h"
#include "eeprom_app_cfg.h"

/* sub-function configuration of service */
static const stDCM_SUBFUNC_CFG s_dcm_sid10_subfunc_cfg[] = 
{
    /* sub-function 0x01 */
    {
        .sub_id                         = DCM_SID10_SUB_01,
        .pos_rsp_msg_indication          = 1,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID10_DP_LEN
    },
    /* sub-function 0x02 */
    {
        .sub_id                         = DCM_SID10_SUB_02,
        .pos_rsp_msg_indication         = 1,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID10_DP_LEN
    },
    /* sub-function 0x03 */
    {
        .sub_id                         = DCM_SID10_SUB_03,
        .pos_rsp_msg_indication          = 1,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID10_DP_LEN
    },
    /* this configuration item is fixed cannot be deleted and is placed last!!! */
    {
        .sub_id                         = DCM_SID_SUB_MAX,
        .pos_rsp_msg_indication          = 0,
        .security_lv                    = 0x00,
        .session_lv                     = 0x00,
        .addr_mode                      = 0x00,
        .dp_len                         = 0U
    },
};

static const stDCM_SUBFUNC_CFG s_dcm_sid11_subfunc_cfg[] = 
{
    /* sub-function 0x01 */
    {
        .sub_id                         = DCM_SID11_SUB_01,
        .pos_rsp_msg_indication          = 1,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_PROGRAMMING | DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID11_DP_LEN
    },
    /* this configuration item is fixed cannot be deleted and is placed last!!! */
    {
        .sub_id                         = DCM_SID_SUB_MAX,
        .pos_rsp_msg_indication          = 0,
        .security_lv                    = 0x00,
        .session_lv                     = 0x00,
        .addr_mode                      = 0x00,
        .dp_len                         = 0U
    },
};


static const stDCM_SUBFUNC_CFG s_dcm_sid27_subfunc_cfg[] = 
{
    /* sub-function 0x01 */
    {
        .sub_id                         = DCM_SID27_SUB_01,
        .pos_rsp_msg_indication         = 0,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID27_DP_LEN
    },
    /* sub-function 0x02 */
    {
        .sub_id                         = DCM_SID27_SUB_02,
        .pos_rsp_msg_indication          = 0,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID27_KEY_NUM
    },
    /* sub-function 0x09 */
    {
        .sub_id                         = DCM_SID27_SUB_09,
        .pos_rsp_msg_indication          = 0,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID27_DP_LEN
    },
    /* sub-function 0x0A */
    {
        .sub_id                         = DCM_SID27_SUB_0A,
        .pos_rsp_msg_indication          = 0,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID27_KEY_NUM
    },
    /* this configuration item is fixed cannot be deleted and is placed last!!! */
    {
        .sub_id                         = DCM_SID_SUB_MAX,
        .pos_rsp_msg_indication          = 0,
        .security_lv                    = 0x00,
        .session_lv                     = 0x00,
        .addr_mode                      = 0x00,
        .dp_len                         = 0U
    },
};

static const stDCM_SUBFUNC_CFG s_dcm_sid28_subfunc_cfg[] = 
{
    /* sub-function 0x00 */
    {
        .sub_id                         = DCM_SID28_SUB_00,
        .pos_rsp_msg_indication         = 1,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID28_DP_LEN
    },
    #if 0
    /* sub-function 0x01 */
    {
        .sub_id                         = DCM_SID28_SUB_01,
        .pos_rsp_msg_indication         = 1,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID28_DP_LEN
    },
    /* sub-function 0x02 */
    {
        .sub_id                         = DCM_SID28_SUB_02,
        .pos_rsp_msg_indication         = 1,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID28_DP_LEN
    },
    #endif
    /* sub-function 0x03 */
    {
        .sub_id                         = DCM_SID28_SUB_03,
        .pos_rsp_msg_indication         = 1,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID28_DP_LEN
    },
    /* this configuration item is fixed cannot be deleted and is placed last!!! */
    {
        .sub_id                         = DCM_SID_SUB_MAX,
        .pos_rsp_msg_indication         = 0,
        .security_lv                    = 0x00,
        .session_lv                     = 0x00,
        .addr_mode                      = 0x00,
        .dp_len                         = 0U
    },
};

static const stDCM_SUBFUNC_CFG s_dcm_sid3e_subfunc_cfg[] = 
{
    /* sub-function 0x00 */
    {
        .sub_id                         = DCM_SID3E_SUB_00,
        .pos_rsp_msg_indication          = 1,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = 0U
    },
    /* this configuration item is fixed cannot be deleted and is placed last!!! */
    {
        .sub_id                         = DCM_SID_SUB_MAX,
        .pos_rsp_msg_indication          = 0,
        .security_lv                    = 0x00,
        .session_lv                     = 0x00,
        .addr_mode                      = 0x00,
        .dp_len                         = 0U
    },
};

static const stDCM_SUBFUNC_CFG s_dcm_sid85_subfunc_cfg[] = 
{
    /* sub-function 0x01 */
    {
        .sub_id                         = DCM_SID85_SUB_01,
        .pos_rsp_msg_indication         = 1,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID85_DP_LEN
    },
    /* sub-function 0x02 */
    {
        .sub_id                         = DCM_SID85_SUB_02,
        .pos_rsp_msg_indication         = 1,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = DCM_SID85_DP_LEN
    },
    /* this configuration item is fixed cannot be deleted and is placed last!!! */
    {
        .sub_id                         = DCM_SID_SUB_MAX,
        .pos_rsp_msg_indication         = 0,
        .security_lv                    = 0x00,
        .session_lv                     = 0x00,
        .addr_mode                      = 0x00,
        .dp_len                         = 0U
    },
};

static const stDCM_SUBFUNC_CFG s_dcm_sid19_subfunc_cfg[] = 
{
    /* sub-function 0x01 */
    {
        .sub_id                         = DCM_SID19_SUB_01,
        .pos_rsp_msg_indication         = 0,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_DEFAULT,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = 0U
    },
    /* sub-function 0x02 */
    {
        .sub_id                         = DCM_SID19_SUB_02,
        .pos_rsp_msg_indication         = 0,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_DEFAULT,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = 0U
    },
    /* sub-function 0x03 */
    {
        .sub_id                         = DCM_SID19_SUB_03,
        .pos_rsp_msg_indication         = 0,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_DEFAULT,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = 0U
    },
    /* sub-function 0x04 */
    {
        .sub_id                         = DCM_SID19_SUB_04,
        .pos_rsp_msg_indication         = 0,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_DEFAULT,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = 0U
    },
    #if 0
    /* sub-function 0x06 */
    {
        .sub_id                         = DCM_SID19_SUB_06,
        .pos_rsp_msg_indication         = 0,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_DEFAULT,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = 0U
    },
    #endif
    /* sub-function 0x0A */
    {
        .sub_id                         = DCM_SID19_SUB_0A,
        .pos_rsp_msg_indication         = 0,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_DEFAULT,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .dp_len                         = 0U
    },
    /* this configuration item is fixed cannot be deleted and is placed last!!! */
    {
        .sub_id                         = DCM_SID_SUB_MAX,
        .pos_rsp_msg_indication         = 0,
        .security_lv                    = 0x00,
        .session_lv                     = 0x00,
        .addr_mode                      = 0x00,
        .dp_len                         = 0U
    },
};

static const stDCM_SUBFUNC_CFG s_dcm_sid31_subfunc_cfg[] = 
{
    /* sub-function 0x01 */
    {
        .sub_id                         = DCM_SID31_SUB_01,
        .pos_rsp_msg_indication          = 0,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED,
        .dp_len                         = 0U
    },
    {
        .sub_id                         = DCM_SID31_SUB_02,
        .pos_rsp_msg_indication          = 0,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED,
        .dp_len                         = 0U
    },
    {
        .sub_id                         = DCM_SID31_SUB_03,
        .pos_rsp_msg_indication          = 0,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED,
        .dp_len                         = 0U
    },
    /* this configuration item is fixed cannot be deleted and is placed last!!! */
    {
        .sub_id                         = DCM_SID_SUB_MAX,
        .pos_rsp_msg_indication          = 0,
        .security_lv                    = 0x00,
        .session_lv                     = 0x00,
        .addr_mode                      = 0x00,
        .dp_len                         = 0U
    },
};
const stDCM_DID_CFG g_dcm_did_cfg[DCM_DID_NUM] = 
{
    {
        .did                            = DID_BOOT_PROGRAMMING_COUNTER,
        .did_len                        = DID_0200_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING
    },
    {
        .did                            = DID_BOOT_PROGRAMMING_ATTEMPTS_COUNTER, /* DID_TYP_0201 */
        .did_len                        = DID_0201_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING
    },
    {
        .did                            = DID_BOOT_SOFTWARE_IDENTIFICATION, /* DID_TYP_F180 */
        .did_len                        = DID_F180_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING
    },
    {
        .did                            = DID_ACTIVE_DIAGNOSTIC_SESSION, /* DID_TYP_F186 */
        .did_len                        = DID_F186_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING
    },
    {
        .did                            = DID_SYSTEM_SUPPLIER_ECU_M0_SOFTWARE_VERSION, /* DID_TYP_F195 */
        .did_len                        = DID_F194_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_PROGRAMMING | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_SYSTEM_SUPPLIER_ECU_M4_SOFTWARE_VERSION, /* DID_TYP_F195 */
        .did_len                        = DID_F195_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_PROGRAMMING | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_SYSTEM_SUPPLIER_ECU_HARDWARE_VERSION, /* DID_TYP_F193 */
        .did_len                        = DID_F193_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_PROGRAMMING | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_VIN, /* DID_TYP_F190 */
        .did_len                        = DID_F190_DATA_LEN,
        .rw                             = DID_RW_READ | DID_RW_WRITE,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_PROGRAMMING_DATE, /* DID_TYP_F199 */
        .did_len                        = DID_F199_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING
    },
    {
        .did                            = DID_BOOT_AND_APP_IDENTIFY, /* DID_TYP_F1EF */
        .did_len                        = DID_F1EF_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING
    },
    {
        .did                            = DID_SECURE_FLASH_FAIL_REASON, /* DID_TYP_F1ED */
        .did_len                        = DID_F1ED_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING
    },
    {
        .did                            = DID_SECURE_STARTUP_FAIL_REASON, /* DID_TYP_F1EE */
        .did_len                        = DID_F1EE_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING
    },
    {
        .did                            = DID_CONFIGURATION_WORD, /* DID_TYP_F1A8 */
        .did_len                        = DID_F1A8_DATA_LEN,
        .rw                             = DID_RW_READ | DID_RW_WRITE,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_5011, /* DID_5011 */
        .did_len                        = DID_5011_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_5012, /* DID_5012 */
        .did_len                        = DID_5012_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_5013, /* DID_5013 */
        .did_len                        = DID_5013_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_5014, /* DID_5014 */
        .did_len                        = DID_5014_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_5015, /* DID_5015 */
        .did_len                        = DID_5015_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_5016, /* DID_5016 */
        .did_len                        = DID_5016_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_5017, /* DID_5017 */
        .did_len                        = DID_5017_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_5018, /* DID_5018 */
        .did_len                        = DID_5018_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_5019, /* DID_5019 */
        .did_len                        = DID_5019_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_501A, /* DID_501A */
        .did_len                        = DID_501A_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
    {
        .did                            = DID_501B, /* DID_501B */
        .did_len                        = DID_501B_DATA_LEN,
        .rw                             = DID_RW_READ,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED
    },
};


static const stDCM_IOCTRL_CFG s_dcm_ioctrl_cfg[DCM_IODID_NUM] = 
{
    {
        .did                            = DID_4001,
        .cs_len                         = 1
    },
};

/* routine identifier configuration */
static const stDCM_RID_CFG s_dcm_rid_cfg[DCM_RID_NUM] = 
{
    {
        .rid                            = RID_ERASE_MEMORY, /* erase memory */
        .dp_len                         = 9U,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_PROGRAMMING,
        .rid_func                       = routine_erase_memory
    },
    {
        .rid                            = RID_CHECK_PROGRAMMING_DEPENDENCIES, /* check programming dependencies */
        .dp_len                         = 0U,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_PROGRAMMING,
        .rid_func                       = routine_check_programming_dependencies
    },
    {
        .rid                            = RID_CHECK_PROGRAMMING_INTEGRITY, /* check programming integrity */
        .dp_len                         = 4U,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .rid_func                       = routine_check_programming_integrity
    },
    {
        .rid                            = RID_CHECK_PROGRAMMING_PRE_CONDITIONS, /* check programming pre-conditions */
        .dp_len                         = 0U,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .rid_func                       = routine_check_programming_pre_conditions
    },
};

static const uint08 s_dcm_sid34_data_format = DCM_SID34_DATA_FORMAT;

/* data parameter configuration of service */
static const stDCM_DATA_PARAM_CFG s_dcm_sid22_dp_cfg = 
{
    .data                               = (uint08*)g_dcm_did_cfg,
    .len                                = DCM_SID22_DP_LEN_MIN 
};

static const stDCM_DATA_PARAM_CFG s_dcm_sid2e_dp_cfg = 
{
    .data                               = (uint08*)g_dcm_did_cfg,
    .len                                = DCM_SID2E_DP_LEN_MIN 
};

static const stDCM_DATA_PARAM_CFG s_dcm_sid14_dp_cfg = 
{
    .data                               = (uint08*)g_can_dtc_cfg,
    .len                                = DCM_SID14_DP_LEN_MIN 
};

static const stDCM_DATA_PARAM_CFG s_dcm_sid19_dp_cfg = 
{
    .data                               = (uint08*)g_can_dtc_cfg,
    .len                                = DCM_SID19_DP_LEN_MIN 
};

static const stDCM_DATA_PARAM_CFG s_dcm_sid2f_dp_cfg = 
{
    .data                               = (uint08*)s_dcm_ioctrl_cfg,
    .len                                = DCM_SID2F_DP_LEN_MIN
};

static const stDCM_DATA_PARAM_CFG s_dcm_sid31_dp_cfg = 
{
    .data                               = (uint08*)s_dcm_rid_cfg,
    .len                                = DCM_SID31_DP_LEN_MIN
};

static const stDCM_DATA_PARAM_CFG s_dcm_sid34_dp_cfg = 
{
    .data                               = (uint08*)&s_dcm_sid34_data_format,
    .len                                = DCM_SID34_DP_LEN_MIN
};

const stDCM_SERVICE_CFG g_dcm_service_cfg[] = 
{
    /* 0x10 */
    {
        .sid                            = DCM_SID_10,
        .subfunc                        = (const stDCM_SUBFUNC_CFG*)s_dcm_sid10_subfunc_cfg,
        .dp                             = DEF_NULL,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_diagnostic_session_control,
        .callback                       = bl_service_10_cbk,
        .cbk_param                      = DEF_NULL,
        .nrc78_result                   = &g_service_10_nrc78_result
    },
    /* 0x11 */
    {
        .sid                            = DCM_SID_11,
        .subfunc                        = (const stDCM_SUBFUNC_CFG*)s_dcm_sid11_subfunc_cfg,
        .dp                             = DEF_NULL,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_ecu_reset,
        .callback                       = dcm_service_11_cbk,
        .cbk_param                      = DEF_NULL,
        .nrc78_result                   = &g_service_11_nrc78_result
    },
    /* 0x27 */
    {
        .sid                            = DCM_SID_27,
        .subfunc                        = (const stDCM_SUBFUNC_CFG*)s_dcm_sid27_subfunc_cfg,
        .dp                             = DEF_NULL,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_security_access,
        .callback                       = DEF_NULL,
        .cbk_param                      = DEF_NULL,
        .nrc78_result                   = DEF_NULL
    },
    /* 0x28 */
    {
        .sid                            = DCM_SID_28,
        .subfunc                        = (const stDCM_SUBFUNC_CFG*)s_dcm_sid28_subfunc_cfg,
        .dp                             = DEF_NULL,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_communication_control,
        .callback                       = dcm_service_28_cbk,
        .cbk_param                      = DEF_NULL,
        .nrc78_result                   = DEF_NULL
    },
    /* 0x3E */
    {
        .sid                            = DCM_SID_3E,
        .subfunc                        = (const stDCM_SUBFUNC_CFG*)s_dcm_sid3e_subfunc_cfg,
        .dp                             = DEF_NULL,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_tester_present,
        .callback                       = DEF_NULL,
        .cbk_param                      = DEF_NULL,
        .nrc78_result                   = DEF_NULL
    },
    /* 0x85 */
    {
        .sid                            = DCM_SID_85,
        .subfunc                        = (const stDCM_SUBFUNC_CFG*)s_dcm_sid85_subfunc_cfg, 
        .dp                             = DEF_NULL,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_dtc_setting,
        .callback                       = dcm_service_85_cbk,
        .cbk_param                      = DEF_NULL,
        .nrc78_result                   = DEF_NULL
    },
    /* 0x22 */
    {
        .sid                            = DCM_SID_22,
        .subfunc                        = DEF_NULL,
        .dp                             = (stDCM_DATA_PARAM_CFG*)&s_dcm_sid22_dp_cfg,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_read_data_by_identifier,
        .callback                       = dcm_service_22_cbk,
        .cbk_param                      = &g_sid22_dp_info,
        .nrc78_result                   = DEF_NULL
    },
    /* 0x2E */
    {
        .sid                            = DCM_SID_2E,
        .subfunc                        = DEF_NULL,
        .dp                             = (stDCM_DATA_PARAM_CFG*)&s_dcm_sid2e_dp_cfg,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_write_data_by_identifier,
        .callback                       = dcm_service_2e_cbk,
        .cbk_param                      = &g_sid2e_dp_info,
        .nrc78_result                   = DEF_NULL
    },
    /* 0x14 */
    {
        .sid                            = DCM_SID_14,
        .subfunc                        = DEF_NULL,
        .dp                             = (stDCM_DATA_PARAM_CFG*)&s_dcm_sid14_dp_cfg,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_clear_diagnostic_information,
        .callback                       = dcm_service_14_cbk,
        .cbk_param                      = &g_sid14_dp_info,
        .nrc78_result                   = DEF_NULL
    },
    /* 0x19 */
    {
        .sid                            = DCM_SID_19,
        .subfunc                        = (const stDCM_SUBFUNC_CFG*)s_dcm_sid19_subfunc_cfg,
        .dp                             = (stDCM_DATA_PARAM_CFG*)&s_dcm_sid19_dp_cfg,
        .security_lv                    = DCM_SEC_LV_UNLOCK,
        .session_lv                     = DCM_SESSION_DEFAULT | DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_FUNC_REQ_SUPPORTED | DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_read_dtc_information,
        .callback                       = dcm_service_19_cbk,
        .cbk_param                      = DEF_NULL,
        .nrc78_result                   = DEF_NULL
    },
    /* 0x2F */
    {
        .sid                            = DCM_SID_2F,
        .subfunc                        = DEF_NULL,
        .dp                             = (stDCM_DATA_PARAM_CFG*)&s_dcm_sid2f_dp_cfg,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_EXTENDED,
        .addr_mode                      = DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_input_output_control_by_identifier,
        .callback                       = dcm_service_2f_cbk,
        .cbk_param                      = &g_sid2f_dp_info,
        .nrc78_result                   = DEF_NULL
    },
    /* 0x31 */
    {
        .sid                            = DCM_SID_31,
        .subfunc                        = (const stDCM_SUBFUNC_CFG*)s_dcm_sid31_subfunc_cfg,
        .dp                             = (stDCM_DATA_PARAM_CFG*)&s_dcm_sid31_dp_cfg,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_EXTENDED | DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_routine_control,
        .callback                       = dcm_service_31_cbk,
        .cbk_param                      = DEF_NULL,
        .nrc78_result                   = DEF_NULL
    },
    /* 0x34 */
    {
        .sid                            = DCM_SID_34,
        .subfunc                        = DEF_NULL,
        .dp                             = (stDCM_DATA_PARAM_CFG*)&s_dcm_sid34_dp_cfg,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_request_download,
        .callback                       = bl_service_34_cbk,
        .cbk_param                      = &g_sid34_dp_info,
        .nrc78_result                   = DEF_NULL
    },
    /* 0x36 */
    {
        .sid                            = DCM_SID_36,
        .subfunc                        = DEF_NULL,
        .dp                             = DEF_NULL,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_transfer_data,
        .callback                       = bl_service_36_cbk,
        .cbk_param                      = &s_sid36_dp_info,
        .nrc78_result                   = DEF_NULL
    },
    /* 0x37 */
    {
        .sid                            = DCM_SID_37,
        .subfunc                        = DEF_NULL,
        .dp                             = DEF_NULL,
        .security_lv                    = DCM_SEC_LV_L1,
        .session_lv                     = DCM_SESSION_PROGRAMMING,
        .addr_mode                      = DCM_PHY_REQ_SUPPORTED,
        .app_func                       = app_request_transfer_exit,
        .callback                       = bl_service_37_cbk,
        .cbk_param                      = DEF_NULL,
        .nrc78_result                   = DEF_NULL
    },
    /* invalid service */
    {
        .sid                            = DCM_SID_INVALID,
        .subfunc                        = DEF_NULL,
        .dp                             = DEF_NULL,
        .security_lv                    = 0U,
        .session_lv                     = 0U,
        .addr_mode                      = 0U,
        .app_func                       = DEF_NULL,
        .callback                       = DEF_NULL,
        .cbk_param                      = DEF_NULL,
        .nrc78_result                   = DEF_NULL
    },
};

