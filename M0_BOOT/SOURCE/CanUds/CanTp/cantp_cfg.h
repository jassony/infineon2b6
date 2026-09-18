/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : cantp_cfg.h
* Author        : yangming
* Date          : 2024-01-23
* Version       : 1.00
* Description   : Can transport protocol configuration header file.
* Others        : None
*
****************************************************************************************************/
#ifndef _CANTP_CFG_H
#define _CANTP_CFG_H
#include "canif.h"

#define N_TIME_TYPE                             uint32
#define CANTP_PDU_RX_NUM                        CANIF_HRH_UDS_NUM
#define CANTP_PDU_TX_NUM                        CANIF_HTH_UDS_NUM
#define N_PCI_SF_MASK                           0x0F
#define N_PCI_FF_MASK                           0x1F
#define N_PCI_CF_MASK                           0x2F
#define N_PCI_FC_MASK                           0x3F
#define N_DATA_SF_LEN                           7U
#define N_DATA_FF_LEN                           6U /* fixed length of first frame data filed */
#define N_DATA_CF_LEN                           7U /* fixed length of consecutive frame data filed */
#define N_SN_CF_MAX                             15U /* SN count form 0 to N_SN_CF_MAX */
#define CANTP_RX_MF_BUF_LEN                     16U
#define N_PCI_BS                                0U /* 0:No longer send FC; others:Maximum number of frames that can be sent */
#define N_PCI_STMIN                             0x00 /* 0~7F:0ms~127ms; F1~F9:100us~900us; others:127ms */

#define CANTP_PDU_DEF_VAL                       0x00
#define CANTP_PDU_EXPECTED_DLC                  8U
#define CANTP_PDU_MIN_LEN                       1U
#define CANTP_PDU_MAX_LEN                       4095U
#define CANTP_N_WFTMAX                          3U

#define CANTP_STS_CHG_DLY_NUM                   9U

typedef enum _cantp_n_result                    eCANTP_N_RESULT;
typedef enum _cantp_fs_typ                      eCANTP_FS_TYP;
typedef enum _cantp_rx_sts                      eCANTP_RX_STS;
typedef enum _cantp_tx_sts                      eCANTP_TX_STS;
typedef enum _frame_type                        eFRAME_TYPE;
typedef struct _cantp_rx_cfg                    stCANTP_RX_CFG;
typedef struct _cantp_tx_cfg                    stCANTP_TX_CFG;
typedef struct _cantp_cfg                       stCANTP_CFG;
//typedef struct _n_pci_info                      stN_PCI_INFO;
typedef struct _cantp_rx_context                stCANTP_RX_CONTEXT;
typedef struct _cantp_tx_context                stCANTP_TX_CONTEXT;

enum _cantp_n_result
{
    CANTP_N_OK = 0,
    CANTP_N_TIMEOUT_A,
    CANTP_N_TIMEOUT_BS,
    CANTP_N_TIMEOUT_CR,
    CANTP_N_TIMEOUT_SN,
    CANTP_N_INVALID_FS,
    CANTP_N_UNEXP_PDU,
    CANTP_NWFT_OVRN,
    CANTP_N_BUFFER_OVFLW,
    CANTP_N_ERROR,
    CANTP_N_TIMEOUT_BR,
    CANTP_N_TIMEOUT_AS,
//    CANTP_N_TIMEOUT_CS,

    CANTP_N_MAX
};

enum _cantp_fs_typ
{
    N_PCI_FS_CTS = 0, /* Continuous transmission */
    N_PCI_FS_WT, /* wait */
    N_PCI_FS_OVFLW, /* overflow */

    N_PCI_FS_INVALID
};

enum _cantp_rx_sts
{
    cantp_RX_INIT = 0,
    CANTP_RX_IDLE,
    CANTP_RX_RECEIVER_SF_IND,
    CANTP_RX_RECEIVER_FF_IND,
    CANTP_RX_RECEIVER_CF_IND,
    CANTP_RX_RECEIVER_CF_IND_OVER,
    CANTP_RX_RECEIVER_FC_REQ,
    CANTP_RX_RECEIVER_FC_CON,
    
    CANTP_RX_SENDER_FC_IND_WAIT, /* Sender wait for FC */
    CANTP_RX_SENDER_FC_IND, /* FC is received when FF/CF is sent */

    CANTP_RX_RECEIVER_SF_ERR,
    CANTP_RX_RECEIVER_FF_ERR,
    CANTP_RX_RECEIVER_CF_ERR,
    CANTP_RX_FC_ERR,
    CANTP_RX_FC_CON_ERR,

    CANTP_RX_NUM
};

enum _cantp_tx_sts
{
    cantp_TX_INIT = 0,
    CANTP_TX_IDLE,
    CANTP_TX_SENDER_SF_REQ,
    CANTP_TX_SENDER_SF_CON,
    CANTP_TX_SENDER_FF_REQ,
    CANTP_TX_SENDER_FF_CON,
    CANTP_TX_SENDER_CF_REQ,
    CANTP_TX_SENDER_CF_CON_WAIT,
    CANTP_TX_SENDER_CF_CON_CONTINUE,
    CANTP_TX_SENDER_CF_CON,
    CANTP_TX_SENDER_FC_IND,
    CANTP_TX_RECEIVER_FC_REQ,
    CANTP_TX_RECEIVER_CF_WAIT, /* wait until the CF receive ends */
    CANTP_TX_RECEIVER_CF_END, /* wait until the CF receive ends */
    CANTP_TX_SENDER_SF_ERR,
    CANTP_TX_SENDER_FF_ERR,
    CANTP_TX_SENDER_CF_ERR,
    CANTP_TX_SENDER_FC_ERR,

    CANTP_TX_NUM
};

enum _frame_type
{
    SINGLE_FRAME = 0,
    FIRST_FRAME,
    CONSECUTIVE_FRAME,
    FLOW_CONTROL,
    
    FRAME_TYPE_INVALID
};

struct _cantp_rx_cfg
{
    PDU_ID              hrh;
    PDU_ID              hth;
    N_TIME_TYPE         n_ar; /* time for transmission of the can frame on the receiver side */
    N_TIME_TYPE         n_br; /* time until transmission of the next flow control N_PDU */
    N_TIME_TYPE         n_cr; /* time until reception of the next consecutive frame N_PDU */
    N_TIME_TYPE         bs; /* BS */
    N_TIME_TYPE         stmin; /* STmin */
};

struct _cantp_tx_cfg
{
    PDU_ID              hth;
    N_TIME_TYPE         n_as; /* time for transmission of the can frame on the sender side */
    N_TIME_TYPE         n_bs; /* time until reception of the next flow control N_PDU */
    N_TIME_TYPE         n_cs; /* time until transmission of the next consecutive frame N_PDU */
};

struct _cantp_cfg
{
    stCANTP_RX_CFG*     rx_cfg;
    stCANTP_TX_CFG*     tx_cfg;
};

//struct _n_pci_info
//{
//    eFRAME_TYPE         ft; /* frame type */
//    uint32              dl; /* data length */
//    uint08              sn; /* the SN of CF */
//    uint08              fs; /* the FS of FC */
//    uint08              bs; /* the BS of FC */
//    uint08              stmin; /* the STmin of FC */
//};

struct _cantp_rx_context
{
    eFRAME_TYPE         rcv_ft;
    eCANTP_RX_STS       sts;
    uint32              sts_dly_cnt; /* Status switching delay count value */
    eCANTP_N_RESULT     result;
    PDU_ID              hth;
    uint08              can_dl; /* can data length */
    uint08              last_sn; /* last sn */
    uint32              mf_cnt; /* multiframe count */
    uint08              bs; /* the BS of FC */
    uint08              bs_cnt; /* FC bs count */
    eCANTP_FS_TYP       fs; /* the FS of FC; 0--CTS(Continuous transmission), 1--WT(wait), 2--OVFLW */
    N_TIME_TYPE         stmin; /* the STmin of FC */
    uint32              sdu_len; /* total SDU length */
    N_TIME_TYPE         n_ar; /* time for transmission of the CAN frame on the receiver side */
    N_TIME_TYPE         n_br; /* time until transmission of the next flow control N_PDU */
    N_TIME_TYPE         n_cr; /* time until reception of the next consecutive frame N_PDU */
};

struct _cantp_tx_context
{
    eCANTP_TX_STS       sts;
    uint32              sts_dly_cnt; /* Status switching delay count value */
    eCANTP_N_RESULT     result;
    PDU_ID              hrh;
    eERR_STS            confirmation;
    uint08              last_sn; /* last sn */
    uint08              mf_cnt; /* multiframe count */
    uint08              mf_num; /* multiframe total numbers */
    uint08              mf_over;
    uint08              mf_tail_len; /* The length of the last frame in multiple frames */
    uint08              bs; /* the BS of FC */
    uint08              bs_cnt; /* FC bs count */
    uint08              wt_cnt; /* WT count */
    eCANTP_FS_TYP       fs; /* the FS of FC; 0--CTS(Continuous transmission), 1--WT(wait), 2--OVFLW */
    N_TIME_TYPE         stmin; /* the STmin of FC */
    N_TIME_TYPE         stmin_cnt; /* the STmin of FC */
    uint32              sdu_len; /* total SDU length */
    N_TIME_TYPE         n_as; /* time for transmission of the can frame on the sender side */
    N_TIME_TYPE         n_bs; /* time until reception of the next flow control N_PDU */
    N_TIME_TYPE         n_cs; /* time until transmission of the next consecutive frame N_PDU */
};

extern const stCANTP_CFG g_cantp_cfg;
#endif /* _CANTP_CFG_H */

