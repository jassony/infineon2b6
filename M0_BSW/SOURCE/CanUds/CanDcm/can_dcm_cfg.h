/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_dcm_cfg.h
* Author        : yangming
* Date          : 2024-01-23
* Version       : 1.00
* Description   : Can diagnostic communication manager configuration header file.
* Others        : None
*
****************************************************************************************************/
#ifndef _CAN_DCM_CFG_H
#define _CAN_DCM_CFG_H
#include "cantp_cfg.h"

#define DCM_PDU_RX_NUM                                  CANTP_PDU_RX_NUM
#define DCM_PDU_TX_NUM                                  CANTP_PDU_TX_NUM
#define DCM_RX_BUF_SIZE                                 1026U /*  >= DCM_SID34_MAX_NUMBER_OF_BLOCK_LENGTH + 2 */
#define DCM_RX_NRC78_BUF_SIZE                           8U
#define DCM_TX_BUF_SIZE                                 256U
#define POSITIVE_RES_SID_MASK                           0x40

#define DCM_SID_10                                      0x10
#define DCM_SID10_SUB_01                                0x01
#define DCM_SID10_SUB_02                                0x02
#define DCM_SID10_SUB_03                                0x03
#define DCM_SID10_DP_LEN                                0U

#define DCM_SID_11                                      0x11
#define DCM_SID11_SUB_01                                0x01
#define DCM_SID11_SUB_02                                0x02
#define DCM_SID11_SUB_03                                0x03
#define DCM_SID11_DP_LEN                                0U

#define DCM_SID_27                                      0x27
#define DCM_SID27_SUB_01                                0x01
#define DCM_SID27_SUB_02                                0x02
#define DCM_SID27_SUB_03                                0x03
#define DCM_SID27_SUB_04                                0x04
#define DCM_SID27_SUB_05                                0x05
#define DCM_SID27_SUB_06                                0x06
#define DCM_SID27_SUB_07                                0x07
#define DCM_SID27_SUB_08                                0x08
#define DCM_SID27_SUB_09                                0x09
#define DCM_SID27_SUB_0A                                0x0A
#define DCM_SID27_SUB_MAX                               0xFF
#define DCM_SID27_DP_LEN                                0U
#define DCM_SECURITY_NUM                                3U /* can not be 0 */
#define DCM_SECURITY_ERR_DLY_TIME                       1000U//3000U /* 10ms uint */    
#define DCM_SID27_SEED_NUM                              4U /* Only 2 or 4 seeds are supported */
#define DCM_SEED_INIT_VAL                               0x20240122
#define DCM_SID27_KEY_NUM                               4U /* Only 2 or 4 keys are supported */

#define DCM_SID_28                                      0x28 
#define DCM_SID28_SUB_00                                0x00 /* enableRxAndTx */
#define DCM_SID28_SUB_01                                0x01 /* enableRxAndDisableTx */
#define DCM_SID28_SUB_02                                0x02 /* disableRxAndEnableTx */
#define DCM_SID28_SUB_03                                0x03 /* disableRxAndTx */
#define DCM_SID28_DP_LEN                                0U
#define DCM_NORMAL_COMM_MSG                             0x01 /* normalCommunicationMessages */
#define DCM_NETWORK_COMM_MSG                            0x02 /* networkManagementCommunicationMessages */
#define DCM_NETWORK_NORMAL_COMM_MSG                     0x03 /* networkManagementCommunicationMessages and normalCommunicationMessages */

#define DCM_SID_3E                                      0x3E
#define DCM_SID3E_SUB_00                                0x00
#define DCM_SID3E_DP_LEN                                0U

#define DCM_SID_85                                      0x85
#define DCM_SID85_SUB_01                                0x01
#define DCM_SID85_SUB_02                                0x02
#define DCM_SID85_DP_LEN                                0U



#define DID_BOOT_PROGRAMMING_COUNTER                    0x0200
#define DID_BOOT_PROGRAMMING_ATTEMPTS_COUNTER           0x0201
#define DID_BOOT_AND_APP_IDENTIFY                       0xF1EF
#define DID_SECURE_FLASH_FAIL_REASON                    0xF1ED
#define DID_SECURE_STARTUP_FAIL_REASON                  0xF1EE
#define DID_CONFIGURATION_WORD                          0xF1A8

/* DID of the UDS standard */
#define DID_BOOT_SOFTWARE_IDENTIFICATION                0xF180
#define DID_APPLICATION_SOFTWARE_IDENTIFICATION         0xF181
#define DID_APPLICATION_DATA_IDENTIFICATION             0xF182
#define DID_BOOT_SOFTWARE_FINGERPRINT                   0xF183
#define DID_PPLICATIONT_SOFTWARE_FINGERPRINT            0xF184
#define DID_PPLICATIONT_DATA_FINGERPRINT                0xF185
#define DID_ACTIVE_DIAGNOSTIC_SESSION                   0xF186
#define DID_VEHICLE_MANUFACTURE_SPACE_PART_NUMBER       0xF187
#define DID_VEHICLE_MANUFACTURE_ECU_SOFTWARE_NUMBER     0xF188
#define DID_VEHICLE_MANUFACTURE_ECU_SOFTWARE_VERSION    0xF189
#define DID_SYSTEM_SUPPLIER_IDENTIFIER                  0xF18A
#define DID_ECU_MANUFACTURING_DATE                      0xF18B
#define DID_ECU_SERIAL_NUMBER                           0xF18C
#define DID_SUPPORTED_FUNCTIONAL_UNITS                  0xF18D
#define DID_VEHICLE_MANUFACTURE_KITASSEMBLY_PART_NUMBER 0xF18E
//#define DID_ISOSAE_RESERVED_STANDARDIZED                0xF18F
#define DID_VIN                                         0xF190
#define DID_VEHICLE_MANUFACTURE_ECU_HARDWARE_NUMBER     0xF191
#define DID_SYSTEM_SUPPLIER_ECU_HARDWARE_NUMBER         0xF192
#define DID_SYSTEM_SUPPLIER_ECU_HARDWARE_VERSION        0xF193
#define DID_SYSTEM_SUPPLIER_ECU_M0_SOFTWARE_VERSION     0xF194
#define DID_SYSTEM_SUPPLIER_ECU_M4_SOFTWARE_VERSION     0xF195
#define DID_EXHAUST_REGULATION_OR_TYPE_APPROVAL_NUMBER  0xF196
#define DID_SYSTEM_NAME_OR_ENGINE_TYPE                  0xF197
#define DID_REPAIR_SHOP_CODE_OR_TESTER_SERIAL_NUMBER    0xF198
#define DID_PROGRAMMING_DATE                            0xF199
#define DID_CALIBRATION_REPAIR_SHOP_CODE                0xF19A
#define DID_CALIBRATION_DATE                            0xF19B
#define DID_CALIBRATION_EQUIPMENT_SOFTWARE_NUMBER       0xF19C
#define DID_ECU_INSTALLATION_DATE                       0xF19D
#define DID_ODX_FILE_DATA                               0xF19E
#define DID_ENTITY_DATA                                 0xF19F

#define DID_INVALID                                     0xFFFF

/* Customer customized DID */
#define DID_0100                                        0x0100 /* ConfigurationInformation */
#define DID_F1A1                                        0xF1A1 /* Supplier name */
#define DID_F1C0                                        0xF1C0 /* vehicleManufacturerHardwareVersionNumber */
#define DID_3001                                        0x3001 /* Battery Voltage */
#define DID_3002                                        0x3002 /* the battery loop Coolant valve switch state. */
#define DID_3003                                        0x3003 /* status of the air conditioner S-TXV */
#define DID_3004                                        0x3004 /* the actual water inlet temperature of the battery */
#define DID_3005                                        0x3005 /* refrigerant low pressure PT sensor Temperature value. */
#define DID_3006                                        0x3006 /* compressor exhaust refrigerant temperature sensor Value */
#define DID_3007                                        0x3007 /* high pressure pressure value of refrigerant */
#define DID_3008                                        0x3008 /* refrigerant low pressure PT sensor pressure value */
#define DID_3009                                        0x3009 /* actual compressor speed */
#define DID_300A                                        0x300A /* compressor operating current. */
#define DID_300B                                        0x300B /* high voltage of the compressor */
#define DID_300C                                        0x300C /* the electronic fan requests PWM duty */
#define DID_300D                                        0x300D /* the battery water pump requests PWM duty */
#define DID_300E                                        0x300E /* bat_EXV actual open ratio. */
#define DID_300F                                        0x300F /* Speed of the vehicle registered by the tachograph(raw data). */
#define DID_3010                                        0x3010 /* Battery HV */
/* Customer customized IODID */
#define DID_4001                                        0x4001 /* the battery loop Coolant valve control request. */
#define DID_4002                                        0x4002 /* the air conditioner S-TXV control request. */
#define DID_4003                                        0x4003 /* compressor speed control request */
#define DID_4004                                        0x4004 /* the electronic fan PWM duty request */
#define DID_4005                                        0x4005 /* the battery water pump PWM duty request */
#define DID_4006                                        0x4006 /* bat_EXV actual open ratio request */

/* Customer customized DID */
#define DID_5011                                        0x5011 /* 请求开机指令状态 */
#define DID_5012                                        0x5012 /* 请求转速 */
#define DID_5013                                        0x5013 /* 功率限制值 */
#define DID_5014                                        0x5014 /* 母线电流 */
#define DID_5015                                        0x5015 /* 相电流 */
#define DID_5016                                        0x5016 /* SIC温度 */
#define DID_5017                                        0x5017 /* 电机转速 */
#define DID_5018                                        0x5018 /* 内部故障 */
#define DID_5019                                        0x5019 /* 电机运行到故障停机时间 */
#define DID_501A                                        0x501A /* 母线电压 */
#define DID_501B                                        0x501B /* PCB温度 */


#define DID_0200_DATA_LEN                               2U
#define DID_0201_DATA_LEN                               1U
#define DID_F180_DATA_LEN                               10U
#define DID_F186_DATA_LEN                               1U
#define DID_F195_DATA_LEN                               10U
#define DID_F194_DATA_LEN                               10U
#define DID_F193_DATA_LEN                               10U
#define DID_F1EF_DATA_LEN                               1U
#define DID_F1ED_DATA_LEN                               1U
#define DID_F1EE_DATA_LEN                               1U
#define DID_F1A8_DATA_LEN                               3U


#define DID_0100_DATA_LEN                               16U
#define DID_F182_DATA_LEN                               40U
#define DID_F184_DATA_LEN                               40U
#define DID_F187_DATA_LEN                               24U
#define DID_F190_DATA_LEN                               17U
#define DID_F191_DATA_LEN                               16U
#define DID_F192_DATA_LEN                               3U
#define DID_F193_DATA_LEN                               10U
#define DID_F194_DATA_LEN                               10U
#define DID_F195_DATA_LEN                               10U
#define DID_F188_DATA_LEN                               16U
#define DID_F189_DATA_LEN                               8U
#define DID_F198_DATA_LEN                               10U
#define DID_F199_DATA_LEN                               4U
#define DID_F19C_DATA_LEN                               16U
#define DID_F19D_DATA_LEN                               4U
#define DID_F1A1_DATA_LEN                               16U
#define DID_F1C0_DATA_LEN                               8U
#define DID_3001_DATA_LEN                               1U
#define DID_3002_DATA_LEN                               1U
#define DID_3003_DATA_LEN                               1U
#define DID_3004_DATA_LEN                               1U
#define DID_3005_DATA_LEN                               1U
#define DID_3006_DATA_LEN                               1U
#define DID_3007_DATA_LEN                               1U
#define DID_3008_DATA_LEN                               1U
#define DID_3009_DATA_LEN                               1U
#define DID_300A_DATA_LEN                               1U
#define DID_300B_DATA_LEN                               1U
#define DID_300C_DATA_LEN                               1U
#define DID_300D_DATA_LEN                               1U
#define DID_300E_DATA_LEN                               1U
#define DID_300F_DATA_LEN                               2U
#define DID_3010_DATA_LEN                               2U
#define DID_3011_3045_DATA_LEN                          1U
#define DID_POWER_DATA_LEN                              2U
#define DID_RW_READ                                     0x01
#define DID_RW_WRITE                                    0x02
#define DID_RW_IO                                       0x04
#define DID_5011_DATA_LEN                                        1U /* 请求开机指令状态 */
#define DID_5012_DATA_LEN                                        2U /* 请求转速 */
#define DID_5013_DATA_LEN                                        2U /* 功率限制值 */
#define DID_5014_DATA_LEN                                        1U /* 母线电流 */
#define DID_5015_DATA_LEN                                        1U /* 相电流 */
#define DID_5016_DATA_LEN                                        1U /* SIC温度 */
#define DID_5017_DATA_LEN                                        2U /* 电机转速 */
#define DID_5018_DATA_LEN                                        2U /* 内部故障 */
#define DID_5019_DATA_LEN                                        4U /* 电机运行到故障停机时间 */
#define DID_501A_DATA_LEN                                        2U /* 母线电压 */
#define DID_501B_DATA_LEN                                        1U /* PCB温度 */



//#define DCM_DID_NUM                                     16
#define DCM_SID_22                                      0x22
#define DCM_SID22_DP_LEN                                2U
#define DCM_SID22_DP_LEN_MIN                            2U

#define DCM_SID_2E                                      0x2E
#define DCM_SID2E_DP_LEN_MIN                            (DCM_DID_NUM << 1U)

#define DCM_SID_14                                      0x14
#define DCM_SID14_DP_LEN_MIN                            4U

#define DCM_SID_19                                      0x19
#define DCM_SID19_DP_LEN_MIN                            4U
#define DCM_SID19_SUB_01                                0x01
#define DCM_SID19_SUB_02                                0x02
#define DCM_SID19_SUB_03                                0x03
#define DCM_SID19_SUB_04                                0x04
#define DCM_SID19_SUB_06                                0x06
#define DCM_SID19_SUB_0A                                0x0A

#define DCM_IODID_NUM                                   6U
#define DCM_SID_2F                                      0x2F
#define DCM_SID2F_DP_LEN_MIN                            (DCM_IODID_NUM << 1U)

#define IOCP_RETURN_CONTROL_TO_ECU                      0x00
#define IOCP_RESET_TO_DEFAULT                           0x01
#define IOCP_FREEZE_CURRENT_STATE                       0x02
#define IOCP_SHORT_TERM_ADJUSTMENT                      0x03


#define DCM_SID_31                                      0x31
#define DCM_SID31_SUB_01                                0x01
#define DCM_SID31_SUB_02                                0x02
#define DCM_SID31_SUB_03                                0x03
#define DCM_RID_NUM                                     4U
#define DCM_SID31_DP_LEN_MIN                            0U
#define DCM_SID31_ROUTINE_STATUS_RECORD_LENGTH          0U /* length of routineStatusRecord paramter */
#define DCM_SID31_INVALID_RID                           0xFFFF
#define RID_ERASE_MEMORY                                0xFF00
#define RID_CHECK_PROGRAMMING_DEPENDENCIES              0xFF01
#define RID_CHECK_PROGRAMMING_INTEGRITY                 0x0202
#define RID_CHECK_PROGRAMMING_PRE_CONDITIONS            0x0203

#define DCM_SID_34                                      0x34
#define DCM_SID34_DATA_FORMAT                           0x00
/* MIN data parameter length: dataFormatIdntifier(1byte) + addressAndLengthFormatIdentifier(1byte) */
#define DCM_SID34_DP_LEN_MIN                            2U
/* bit7-4:Length(number of bytes) of the maxNumberOfBlockLength parameter */
#define DCM_SID34_LENGTH_FORMAT_IDENTIFIER              0x20
#define DCM_SID34_MAX_NUMBER_OF_BLOCK_LENGTH            1024U

#define DCM_SID_36                                      0x36

#define DCM_SID_37                                      0X37

#define DCM_SID_INVALID                                 0xFF
#define DCM_SID_SUB_MAX                                 0xFF

#define DCM_SEC_LV_UNLOCK                               0x00
#define DCM_SEC_LV_L1                                   (1 << 0U)
#define DCM_SEC_LV_L2                                   (1 << 1U)
#define DCM_SEC_LV_L3                                   (1 << 2U)
#define DCM_SEC_LV_L4                                   (1 << 3U)
#define DCM_SEC_LV_LOCK                                 0xFF
#define DCM_SEC_LV_DEF                                  DCM_SEC_LV_LOCK
#define DCM_SEC_LV_L1_SEED                              0x01
#define DCM_SEC_LV_L2_SEED                              0x03
#define DCM_SEC_LV_L3_SEED                              0x09

#define DCM_SESSION_DEFAULT                             0x01
#define DCM_SESSION_EXTENDED                            0x04
#define DCM_SESSION_PROGRAMMING                         0x02

#define DCM_PHY_REQ_SUPPORTED                           0x01
#define DCM_FUNC_REQ_SUPPORTED                          0x02

#define SUPPRESS_POS_RESP_MSG_INDICATION_BIT            0x80

/* negative response code */
#define NRC_POSITIVE_RESPONSE                           0x00
#define NRC_GENERAL_REJECT                              0x10
#define NRC_SERVICE_NOT_SUPPORTED                       0x11
#define NRC_SUBFUNC_NOT_SUPPORTED                       0x12
#define NRC_INCORRECT_MSG_LENGTH_OR_INVALID_FORMAT      0x13
#define NRC_RESPONSE_TOO_LONG                           0x14
#define NRC_BUSY_REPEAT_REQUEST                         0x21
#define NRC_CONNDITIONS_NOT_ERROR                       0x22
#define NRC_REQUEST_SEQUENCE_ERROR                      0x24
#define NRC_REQUEST_OUT_OF_RANGE                        0x31
#define NRC_SECURITY_ACCESS_DENIED                      0x33
#define NRC_INVALID_KEY                                 0x35
#define NRC_EXCEED_NUMBER_OF_ATTEMPTS                   0x36
#define NRC_REQUIRD_TIME_DELAY_NOT_EXPIRED              0x37
#define NRC_UPLOAD_DOWNLOAD_NOT_ACCEPTED                0x70
#define NRC_TRANSFER_DATA_SUSPENDED                     0x71
#define NRC_GENERAL_PROGRAMMING_FALIURE                 0x72
#define NRC_WRONG_BLOCK_SEQUENCE_COUNTER                0x73
#define NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING 0x78
#define NRC_SUBFUNC_NOT_SUPPORTED_IN_ACTIVE_SESSION     0x7E
#define NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION     0x7F
#define NRC_ENGINE_IS_RUNNING                           0x83
#define NRC_ENGINE_IS_NOT_RUNNING                       0x84
#define NRC_SHIFTER_LEVER_NOT_PARK                      0x90
#define NRC_VOLTAGE_TOO_HIGH                            0x92
#define NRC_VOLTAGE_TOO_LOW                             0x93

#define NRC_78_POS_RESPONSE                             0xAA
#define NRC_78_NEG_RESPONSE                             0xAE
#define NRC_78_INIT                                     0x00
#define NRC_78_PEDING                                   0xEE

#define DCM_PERIOD_TIME_BASE                            10U
#define S3_SERVER_TIME                                  500U /* 5S */
#define P2CAN_SERVER_TIME                               5U
#define P2_CAN_SERVER_TIME                              500U
#define DCM_START_TIME                                  30U /* 0.3s */

typedef enum _dcm_sesssion_type                         eDCM_SESSION_TYPE;
typedef enum _did_typ                                   eDID_TYP;
typedef struct _sid22dp_info                            stSID22_DP_INFO;
typedef struct _sid2edp_info                            stSID2E_DP_INFO;
typedef struct _sid14dp_info                            stSID14_DP_INFO;
typedef struct _sid2fdp_info                            stSID2F_DP_INFO;
typedef struct _sid31dp_info                            stSID31_DP_INFO;
typedef struct _sid34dp_info                            stSID34_DP_INFO;
typedef struct _sid36dp_info                            stSID36_DP_INFO;
typedef struct _dcm_msg_context                         stDCM_MSG_CONTEXT;
typedef struct _dcm_did_cfg                             stDCM_DID_CFG;
typedef struct _dcm_ioctrl_cfg                          stDCM_IOCTRL_CFG;
typedef struct _dcm_rid_cfg                             stDCM_RID_CFG;
typedef struct _dcm_subfunc_cfg                         stDCM_SUBFUNC_CFG;
typedef struct _dcm_data_param_cfg                      stDCM_DATA_PARAM_CFG;
typedef struct _dcm_service_cfg                         stDCM_SERVICE_CFG;
typedef void (*dcm_app_func)(stDCM_MSG_CONTEXT*, stDCM_SERVICE_CFG);
typedef void (*dcm_did_func)(void); 
typedef uint08 (*dcm_rid_func)(void); 
typedef void (*dcm_cbk)(void*, void*);

enum _dcm_sesssion_type
{
    SESSION_TYPE_DEFAULT = 0,
    SESSION_TYPE_PROGRAMMING,
    SESSION_TYPE_EXTENDED,

    SESSION_TYPE_MAX
};

enum _did_typ
{
    DID_TYP_0200 = 0,
    DID_TYP_0201,
    DID_TYP_F180,
    DID_TYP_F186,
    DID_TYP_F194,
    DID_TYP_F195,
    DID_TYP_F193,
    DID_TYP_F190,
    DID_TYP_F199,
    DID_TYP_F1EF,
    DID_TYP_F1ED,
    DID_TYP_F1EE,
    DID_TYP_F1A8,
    DID_TYP_5011,
    DID_TYP_5012,  
    DID_TYP_5013,  
    DID_TYP_5014,  
    DID_TYP_5015,  
    DID_TYP_5016,  
    DID_TYP_5017,  
    DID_TYP_5018,  
    DID_TYP_5019,  
    DID_TYP_501A,  
    DID_TYP_501B,          
    DCM_DID_NUM
};

struct _sid22dp_info 
{
    uint16                      did[DCM_DID_NUM];
    eDID_TYP                    typ[DCM_DID_NUM];
    uint16                      did_num;
};

struct _sid2edp_info
{
    uint16                      did;
    uint16                      len;
    eDID_TYP                    typ;
};

struct _sid14dp_info
{
    uint32                      dtc;
    uint08                      typ;
};

struct _sid2fdp_info
{
    uint16                      did;
    uint08                      iocp;
    uint08                      cs_len;
    uint08                      cm_len;
};

struct _sid31dp_info
{
    uint16                      req_rid;
};

struct _sid34dp_info
{
    uint08                      transmit_start;
    uint08                      data_format;
    uint08                      mem_addr_bytelen;
    uint08                      mem_size_bytelen;
    uint32                      mem_start_addr;
    uint32                      wr_data_size;
};

struct _sid36dp_info
{
    uint08                      block_seq_cnt; /* blockSequenceCounter */
    uint08                      recv_over;
    uint32                      recv_data_len;
};

struct _dcm_msg_context
{
    PDU_ID                      hrh;
    PDU_ID                      hth;
    uint08*                     req_data;
    uint32                      req_data_len;
    uint08*                     res_data;
    uint32                      res_data_len;
    uint32                      nrc; /* negative response code */
    uint08*                     nrc78_result;
};

struct _dcm_did_cfg
{
    uint16                      did;
    uint08                      did_len;
    uint08                      rw;
    uint08                      session_lv;
};

struct _dcm_ioctrl_cfg
{
    uint16                      did;
    uint16                      cs_len;
//    uint08                      security_lv;
//    uint08                      session_lv;
};

struct _dcm_rid_cfg
{
    uint16                      rid;
    uint16                      dp_len;
    uint08                      security_lv;
    uint08                      session_lv;
    dcm_rid_func                rid_func;
};

struct _dcm_subfunc_cfg
{
    uint08                      sub_id;
    uint08                      pos_rsp_msg_indication;
    uint08                      security_lv;
    uint08                      session_lv;
    uint08                      addr_mode;
    uint08                      dp_len; /* data parameter */
};

struct _dcm_data_param_cfg
{
    uint08*                     data;
    uint32                      len; /* min dp length */
};

struct _dcm_service_cfg
{
    uint08                      sid;
    const stDCM_SUBFUNC_CFG*    subfunc;
    stDCM_DATA_PARAM_CFG*       dp;
    uint08                      security_lv;
    uint08                      session_lv;
    uint08                      addr_mode;
    dcm_app_func                app_func;
    dcm_cbk                     callback;
    void*                       cbk_param;
    uint08*                     nrc78_result;
};

extern stSID22_DP_INFO g_sid22_dp_info;
extern stSID2E_DP_INFO g_sid2e_dp_info;
extern stSID14_DP_INFO g_sid14_dp_info;
extern stSID2F_DP_INFO g_sid2f_dp_info;
extern stSID31_DP_INFO g_sid31_dp_info;
extern stSID34_DP_INFO g_sid34_dp_info;
extern stSID36_DP_INFO s_sid36_dp_info;
extern uint08 g_service_10_nrc78_result;
extern uint08 g_service_11_nrc78_result;
extern uint08 g_service_31_nrc78_result;

extern const stDCM_DID_CFG g_dcm_did_cfg[DCM_DID_NUM];

extern void app_diagnostic_session_control(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_security_access(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_routine_control(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_request_download(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_transfer_data(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void app_request_transfer_exit(stDCM_MSG_CONTEXT* msg, stDCM_SERVICE_CFG service_cfg);
extern void bl_service_10_cbk(void * cbk_param1, void * cbk_param2);
extern void bl_service_28_cbk(void * cbk_param1, void * cbk_param2);
extern void bl_service_31_cbk(void * cbk_param1, void * cbk_param2);
extern void bl_service_34_cbk(void * cbk_param1, void * cbk_param2);
extern void bl_service_36_cbk(void * cbk_param1, void * cbk_param2);
extern void bl_service_37_cbk(void * cbk_param1, void * cbk_param2);
extern uint08 routine_erase_memory(void);
extern uint08 routine_check_programming_dependencies(void);
extern uint08 routine_check_programming_integrity(void);
extern uint08 routine_check_programming_pre_conditions(void);
extern const stDCM_SERVICE_CFG g_dcm_service_cfg[];
#endif /* _CAN_DCM_CFG_H */
