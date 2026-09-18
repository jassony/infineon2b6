/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : uds_user.h
* Author        : yangming
* Date          : 2024-04-19
* Version       : 1.00
* Description   : 
* Others        : None
*
****************************************************************************************************/
#ifndef _UDS_USER_H
#define _UDS_USER_H
#include "can_dcm.h"

//#define UDS_RX_DATA_COPY_FORM_TASK_EN                           
#define UDS_USER_RX0_NUM                                    CANIF_HRH_UDS_NUM
#define UDS_USER_RX1_NUM                                    CANIF_HRH_UDS_NUM
#define UDS_USER_TX1_NUM                                    CANIF_HRH_UDS_NUM
#define UDS_USER_RX2_NUM                                    CANIF_HRH_UDS_NUM
#define UDS_USER_TX2_NUM                                    CANIF_HTH_UDS_FNUM
#define UDS_RX_DATA_USED                                    0
#define UDS_RX_DATA_UNUSED                                  1

#define SERVICE_11_HARD_RESET                               0x01
#define SERVICE_11_KEY_OFFON_RESET                          0x02
#define SERVICE_11_SOFT_RESET                               0x03

#define SERVICE_11_RESET_TIME_DLY                           10 /* 100ms */
#define UDS_DIAG_START_TIME                                 150U 
#define UDS_DIAG_RESTART_TIME                               50U

#define SERVICE_85_DTCSETTINGTYPE_ON                        0x01
#define SERVICE_85_DTCSETTINGTYPE_OFF                       0x02

#define SERVICE_11_RESET_TIME_DLY                           30 /* 300ms */ 
#define UDS_DIAG_START_TIME                                 150U 
#define UDS_DIAG_RESTART_TIME                               50U

#define DID_VAL_MAX_NUM                                     24U

#define UDS_DIAG_VOL_HIGH                                   305U /* 0.1V */
#define UDS_DIAG_VOL_LOW                                    195U /* 0.1V */

// #define DID_0100_ADDR                                       0x00000020
// #define DID_F190_ADDR                                       0x00000030
// #define DID_F198_ADDR                                       0x00000041
// #define DID_F199_ADDR                                       0x0000004B
// #define DID_F19D_ADDR                                       0x0000004F
// #define DID_F184_ADDR                                       0x00000053

#define DID_BOOT_START_ADDR                                 CY_WFLASH_SM_SBM_BASE +  4 * CY_WORK_SES_SIZE_IN_BYTE   
#define DID_F199_ADDR                                       DID_BOOT_START_ADDR   
#define DID_0200_ADDR                                       DID_F199_ADDR + DID_F199_DATA_LEN 
#define DID_0201_ADDR                                       DID_0200_ADDR + DID_0200_DATA_LEN  


typedef enum _uds_vol_typ                                   eUDS_VOL_TYP;
typedef struct _service_11_ctrl                             stSERVICE_11_CTRL;
typedef struct _service_28_ctrl                             stSERVICE_28_CTRL;
typedef struct _service_85_ctrl                             stSERVICE_85_CTRL;
typedef struct _service_22_ctrl                             stSERVICE_22_CTRL;
typedef struct _service_2e_ctrl                             stSERVICE_2E_CTRL;
typedef struct _service_14_ctrl                             stSERVICE_14_CTRL;
typedef struct _service_19_ctrl                             stSERVICE_19_CTRL;
typedef struct _service_2f_ctrl                             stSERVICE_2F_CTRL;

enum _uds_vol_typ
{
    UDS_VOL_NORMAL = 0,
    UDS_VOL_OVER,
    UDS_VOL_UNDER,
    UDS_VOL_RESTART,
    UDS_VOL_START,

    UDS_VOL_MAX
};

struct _service_11_ctrl
{
    uint08                      reset;
};

struct _service_28_ctrl
{
    uint08                      control_type;
    uint08                      communication_type;
};

struct _service_85_ctrl
{
    uint08                      dtc_setting_ctrl;
};

struct _service_22_ctrl
{
    uint08*                     did_val_point[DCM_DID_NUM];
};

struct _service_2e_ctrl
{
    uint08                      did_val[DID_VAL_MAX_NUM];
    uint16                      did_len;
    eDID_TYP                    did_typ;
    uint08                      did_wr_en;
};

struct _service_14_ctrl
{
    uint32                      dtc;
    uint08                      dtc_typ;
};

struct _service_19_ctrl
{
    uint32                      dtc;
    uint08                      dtc_typ;
    uint16                      dtc_num;
    uint16                      dtc_sts;
    
};

struct _service_2f_ctrl
{
    uint08                      did_4001;
    uint08                      did_4002;
    uint16                      did_4003;
    uint08                      did_4004;
    uint08                      did_4005;
    uint08                      did_4006;
    uint32                      en;
};

extern stSERVICE_22_CTRL g_service_22;
extern stSERVICE_2E_CTRL g_service_2e;
extern uint08                      did_0100_val[DID_0100_DATA_LEN];
extern uint08                      did_f190_val[DID_F190_DATA_LEN];
extern uint08                      did_f184_val[DID_F184_DATA_LEN];
extern uint08                      did_f19d_val[DID_F19D_DATA_LEN];
extern uint08                      did_3001_val[DID_3001_DATA_LEN];
extern uint08                      did_3010_val[DID_3010_DATA_LEN];
extern uint08                      did_f1a8_val[DID_F1A8_DATA_LEN];

extern void uds_user_init(void);
extern void uds_user_process(void);
extern eUDS_VOL_TYP uds_vol_typ_get(void);
extern void dcm_service_85_dtcsettingtype_set(uint08 val);
extern uint08 dcm_service_85_dtcsettingtype_get(void);
extern void can0_uds_rx_cbk(uint32 canid, uint08* pdata, uint32 dlc);
extern void can1_uds_rx_cbk(uint32 canid, uint08* pdata, uint32 dlc);
extern void can2_uds_rx_cbk(uint32 canid, uint08* pdata, uint32 dlc);
extern void can1_uds_tx_cbk(uint32 canid);
extern void can2_uds_tx_cbk(uint32 canid);
extern void uds_can_rx_process(void);
extern uint16 uds_vechicle_speed_get(void);
#endif /* _UDS_USER_H */

