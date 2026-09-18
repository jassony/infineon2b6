/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : canif_cfg.h
* Author        : yangming
* Date          : 2024-01-22
* Version       : 1.00
* Description   : Can interface configuartion header file.
* Others        : None
*
****************************************************************************************************/
#ifndef _CANIF_CFG_H
#define _CANIF_CFG_H
#include "can_common.h"
#include "can_driver.h"

#define DCM_PDU_PHY_NUM                     1U
#define DCM_PDU_FUNC_MAX                    3U
#define CAN_ID_EXT_MASK                     0x08000000
#define CANIF_ID_EXT_MASK                   CAN_ID_EXT_MASK

#define CAN_PHY_RX_ID                       0x18DAC3F1
#define CAN_FUNC_RX_ID                      0x18DB33F1
#define CAN_PHY_TX_ID                       0x18DAF1C3
#define CAN_PHY2_RX_ID                      0x18DAC3F1
#define CAN_FUNC2_RX_ID                     0x18DB33F1
#define CAN_PHY2_TX_ID                      0x18DAF1C3

//#define CAN_DATA_LENGTH                     8U
#define CAN_INVALID_ID                      0x1FFFFFFF
#define CANIF_HTH_UDS_NUM                   (CANIF_HTH_UDS_FNUM - CANIF_HRH_NUM)
#define CANIF_HTH_NUM                       (CANIF_HOH_NUM - CANIF_HRH_NUM)

#define J1939_DM1_ID                        0x18FECAC3
#define J1939_TPCM_BAM_ID                   0x1CECFFC3
#define J1939_TPDT_ID                       0x1CEBFFC3
#define J1939_DM1_PGN                       0x00FECA
#define J1939_DM1_CAN_CHN                   CAN_CHN_0

#define CAN_COM_0CFE6C27_RX_ID              0x0CFE6C27
#define CAN_COM_7DF_RX_ID                   0x7DF


typedef enum _canif_dir                     eCANIF_DIR;
typedef enum _canif_hoh                     eCANIF_HOH;
typedef struct _dcm_pdu_match               stDCM_PDU_MATCH;
typedef struct _canif_cfg                   stCANIF_CFG;

enum _canif_dir
{
    CANIF_RX = 0,
    CANIF_TX
};

enum _canif_hoh
{
    CANIF_HRH0_PHY = 0,
    CANIF_HRH0_FUNC,
    CANIF_HRH_UDS_NUM, /* fixed for UDS rx CANID */
//    CANIF_HRH0_COM_0CFE6C27 = CANIF_HRH_UDS_NUM,
    CANIF_HRH0_NUM = CANIF_HRH_UDS_NUM, /* fixed, Used to calculate the number of receives in CAN channel 0 */
    CANIF_HRH_NUM = CANIF_HRH0_NUM,
//
    CANIF_HTH0_PHY = CANIF_HRH_NUM,
    CANIF_HTH_UDS_FNUM, /* fixed for UDS tx CANID */
    CANIF_HOH_NUM = CANIF_HTH_UDS_FNUM
};

struct _dcm_pdu_match
{
    uint08          phy_hrh;
    uint08*         func_hrh;
    uint08          phy_hth;
};

struct _canif_cfg
{
    eCAN_CHN        chn;
    PDU_ID          hoh;
    eCANIF_DIR      dir;
    uint32          canid;
    uint08          dlc;
};

extern const stCANIF_CFG g_canif_cfg_tbl[CANIF_HOH_NUM];
extern const stDCM_PDU_MATCH g_pdu_match_cfg[DCM_PDU_PHY_NUM];

#endif /* _CANIF_CFG_H */

