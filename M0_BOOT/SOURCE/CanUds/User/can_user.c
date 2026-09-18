/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_user.c
* Author        : yangming
* Date          : 2024-05-29
* Version       : 1.00 : First version creation.
                  2.00 : Add interface to the SWC.
* Description   : Can user communcation.
* Others        : None
*
****************************************************************************************************/
#include "can_user.h"
#include "soft_timer.h"
//#include "output.h"

/* can0 */
//static volatile unCAN0_RX_DATA      s_can0_rx_data;
//static uint08                       s_can0_busoff_flg;
//static uint08                       s_can0_rx_disable;
//static uint08                       s_can0_tx_disable;

static uint8_t s_can_drv_tx_repeat;
static _ST_CAN_TX_MSG s_can_msg_repeat;
static uint8_t                       s_can0_busoff_flg;
static uint8_t                       s_can0_rx_disable;
static uint8_t                       s_can0_tx_disable;

//static volatile uint08              s_can0_rx_flg[CAN0_RX_NUM];
//static uint08                       s_can0_rx_timeout_flg[CAN0_RX_NUM];
//static uint16                       s_can0_rx_timeout_cnt[CAN0_RX_NUM];
//static uint08                       s_can0_rx_lock[CAN0_RX_NUM];
//static const uint16                 s_can0_rx_timeout_cfg[CAN0_RX_NUM] = 
//{
//    50, /* CANIF_HRH0_COM_0CFE6C27 HCU_TCO1 cycle:50ms */
//};
//static const uint16                 s_can0_tx_init_cnt[CAN0_TX_NUM] = 
//{
//    0,
//};
//static uint16                       s_can0_tx_period_cnt[CAN0_TX_NUM];


/* can1 */
//static volatile unCAN1_RX_DATA      s_can1_rx_data;
//static uint08                       s_can1_busoff_flg;
//static volatile uint08              s_can1_rx_flg[CAN1_RX_NUM];
//static uint08                       s_can1_rx_timeout_flg[CAN1_RX_NUM];
//static uint16                       s_can1_rx_timeout_cnt[CAN1_RX_NUM];
//static uint08                       s_can1_rx_lock[CAN1_RX_NUM];
//static const uint16                 s_can1_rx_timeout_cfg[CAN1_RX_NUM] = 
//{
//    500, /* CANIF_HRH1_COM_ACP_RSP */
//};
//static const uint16                 s_can1_tx_init_cnt[CAN1_TX_NUM] = 
//{
//    0,
//};
//static uint16                       s_can1_tx_period_cnt[CAN1_TX_NUM];

uint16 g_vehicle_spped;
uint16 g_vehicle_spped_original;

//static void can_rx_process(void);
//static void can_tx_process(void);
//static void can_0cfe6C27_rx_process(void);

void can_user_init(void)
{
//    uint08 i = 0;
//    #if 0
//
//    common_memset((uint08*)s_can0_rx_data.data, 0U, sizeof(unCAN0_RX_DATA));
//    s_can0_busoff_flg = 0;
//    s_can0_rx_disable = 0;
//    s_can0_tx_disable = 0;
//    common_memset((uint08*)&s_can0_rx_flg, 0U, sizeof(s_can0_rx_flg));
//    common_memset((uint08*)&s_can0_rx_timeout_flg, 0U, sizeof(s_can0_rx_timeout_flg));
//    common_memset((uint08*)&s_can0_rx_timeout_cnt, 0U, sizeof(s_can0_rx_timeout_cnt));
//    common_memset((uint08*)&s_can0_rx_lock, 0U, sizeof(s_can0_rx_lock));
//    common_memset((uint08*)&s_can0_tx_period_cnt, 0U, sizeof(s_can0_tx_period_cnt));
//    #endif

//    common_memset((uint08*)s_can1_rx_data.data, 0U, sizeof(unCAN1_RX_DATA));
    s_can0_busoff_flg = 0;
    s_can0_rx_disable = 0;
    s_can0_tx_disable = 0;
//    common_memset((uint08*)&s_can1_rx_flg, 0U, sizeof(s_can1_rx_flg));
//    common_memset((uint08*)&s_can1_rx_timeout_flg, 0U, sizeof(s_can1_rx_timeout_flg));
//    common_memset((uint08*)&s_can1_rx_timeout_cnt, 0U, sizeof(s_can1_rx_timeout_cnt));
//    common_memset((uint08*)&s_can1_rx_lock, 0U, sizeof(s_can1_rx_lock));
//    common_memset((uint08*)&s_can1_tx_period_cnt, 0U, sizeof(s_can1_tx_period_cnt));
//    g_vehicle_spped = 0;
//    g_vehicle_spped_original = 0;
//
//    can_enable(CAN_CHN_0);
//   can_enable(CAN_CHN_1);
//    can_enable(CAN_CHN_2);
}

/* 10ms period */
void can_user_process(void)
{
//    can_rx_process();
//    can_tx_process();
//    can_user_10ms_period_process();
    if (s_can_drv_tx_repeat)
    {
        s_can_drv_tx_repeat = 0;
        Can_Transmit(s_can_msg_repeat.chn, s_can_msg_repeat.id, s_can_msg_repeat.datalen, s_can_msg_repeat.data, s_can_msg_repeat.canFDFormat);
    }
    else
    {}
}

//void can_enable(eCAN_CHN chn)
//{
//    switch (chn)
//    {
//        case CAN_CHN_0:
////            output_buf_set(OUTPUT_CAN0_EN, 1);
////            output_buf_set(OUTPUT_CAN0_STB, 1);
//            break;
//        case CAN_CHN_1:
////            output_buf_set(OUTPUT_CAN1_EN, 1);
////            output_buf_set(OUTPUT_CAN1_STB, 1);
//            break;
//        case CAN_CHN_2:
////            output_buf_set(OUTPUT_CAN1_EN, 1);
////            output_buf_set(OUTPUT_CAN1_STB, 1);
//            break;
//        default:
//            break;
//    }
//}

//void can_disable(eCAN_CHN chn)
//{
//    switch (chn)
//    {
//        case CAN_CHN_0:
////            output_buf_set(OUTPUT_CAN0_EN, 0);
////            output_buf_set(OUTPUT_CAN0_STB, 0);
//            break;
//        case CAN_CHN_1:
////            output_buf_set(OUTPUT_CAN1_EN, 0);
////            output_buf_set(OUTPUT_CAN1_STB, 0);
//            break;
//        default:
//            break;
//    }
//}

//void can0_user_rx_cbk(uint32 canid, uint08* pdata)
//{
//    uint08 i = 0;
//
//    s_can0_busoff_flg = 0;
//    if (s_can0_rx_disable)
//    {
//        return;
//    }
//    
//    for (i = 0; i < CAN0_RX_NUM; i++)
//    {
//        if (   (CAN_CHN_0 == g_canif_cfg_tbl[i + CANIF_HRH_UDS_NUM].chn)
//            && (canid == g_canif_cfg_tbl[i + CANIF_HRH_UDS_NUM].canid)
//            && (CANIF_RX == g_canif_cfg_tbl[i + CANIF_HRH_UDS_NUM].dir)
//            )
//        {
//            s_can0_rx_timeout_cnt[i] = 0;
//            s_can0_rx_timeout_flg[i] = 0;
//            if ((0 == s_can0_rx_flg[i]) && (CAN_RX_DATA_UNLOCK == s_can0_rx_lock[i]))
//            {
//                s_can0_rx_flg[i] = CAN_RX_DATA_UNUSED;
//                common_memcpy((uint08*)&s_can0_rx_data.data[CAN_DATA_LENGTH * i], pdata, CAN_DATA_LENGTH);
//            }
//            else {}
//            break;
//        }
//        else { /* continue */ }
//    }
//}

//void can1_user_rx_cbk(uint32 canid, uint08* pdata)
//{
//    // #if 0
//    uint08 i = 0;
//    if (s_can0_rx_disable)
//    {
//        return;
//    }
//    s_can1_busoff_flg = 0;
//    for (i = 0; i < CAN1_RX_NUM; i++)
//    {
//        if (   (CAN_CHN_1 == g_canif_cfg_tbl[i + CANIF_HRH0_NUM].chn)
//            && (canid == g_canif_cfg_tbl[i + CANIF_HRH0_NUM].canid)
//            && (CANIF_RX == g_canif_cfg_tbl[i + CANIF_HRH0_NUM].dir)
//            )
//        {
//            s_can1_rx_timeout_cnt[i] = 0;
//            s_can1_rx_timeout_flg[i] = 0;
//            if ((0 == s_can1_rx_flg[i]) && (CAN_RX_DATA_UNLOCK == s_can1_rx_lock[i]))
//            {
//                s_can1_rx_flg[i] = CAN_RX_DATA_UNUSED;
//            }
//            else {}
//            break;
//        }
//        else { /* continue */ }
//    }
//    // #endif
//}

void can0_user_busoff_cbk(void)
{
    s_can0_busoff_flg = 1;
}

//void can1_user_busoff_cbk(void)
//{
//    s_can1_busoff_flg = 1;
//}

void can0_trx_ctrl_by_uds_cbk(uint08 tx_disable, uint08 rx_disable)
{
    s_can0_tx_disable = tx_disable;
    s_can0_rx_disable = rx_disable;
}


void Can_Transmit(cy_pstc_canfd_type_t chn, uint32_t canid, uint8_t dlc, uint8_t * data,bool fd)
{
    cy_stc_canfd_msg_t stcMsg;
    uint8_t i = 0;
    if( chn == CAN0_INSTANCE && 1 == s_can0_tx_disable) return;
    
    stcMsg.canFDFormat = fd;
    stcMsg.idConfig.extended = canid < 0x7ff? false : true ;
    stcMsg.idConfig.identifier = canid;
    stcMsg.dataConfig.dataLengthCode = dlc;
    if(fd)
    {
      if(dlc > 8)
        stcMsg.dataConfig.dataLengthCode = 8 + (dlc - 8)/8 + ((dlc - 8)%8 > 0?1:0);
    }
    memcpy((uint8_t *)&stcMsg.dataConfig.data,data,dlc);
    for(i = 0; i < TX_BUFFER_NUM;i++)
    {
      if(CY_CANFD_TX_BUFFER_IDLE == Cy_CANFD_GetTxBufferStatus(chn,i)
         ||CY_CANFD_TX_BUFFER_TRANSMIT_OCCURRED == Cy_CANFD_GetTxBufferStatus(chn,i) )
         break;                      
    }
    if(i < TX_BUFFER_NUM)
      Cy_CANFD_UpdateAndTransmitMsgBuffer(chn, i, &stcMsg);
    else
    {
      if (0 == s_can_drv_tx_repeat)
        {
            s_can_msg_repeat.chn = chn;
            s_can_msg_repeat.id = canid;
            s_can_msg_repeat.datalen = dlc;
            s_can_msg_repeat.canFDFormat = fd;
            common_memcpy((uint8_t*)s_can_msg_repeat.data, (uint8_t*)data, dlc);
            
            s_can_drv_tx_repeat = 1;    
        }
    }

}


//void can_user_10ms_period_process(void)
//{
//    uint08 i = 0;
//
//    /* can0 timeout process */
//    for (i = 0; i < CAN0_RX_NUM; i++)
//    {
//        s_can0_rx_timeout_cnt[i]++;
//        if (s_can0_rx_timeout_cnt[i] >= s_can0_rx_timeout_cfg[i])
//        {
//            s_can0_rx_timeout_cnt[i] = s_can0_rx_timeout_cfg[i];
//            s_can0_rx_timeout_flg[i] = 1;
//        }
//        else {}
//    }
//    #if 0
//    /* can1 timeout process */
//    for (i = 0; i < CAN1_RX_NUM; i++)
//    {
//        s_can1_rx_timeout_cnt[i]++;
//        if (s_can1_rx_timeout_cnt[i] >= s_can1_rx_timeout_cfg[i])
//        {
//            s_can1_rx_timeout_cnt[i] = s_can1_rx_timeout_cfg[i];
//            s_can1_rx_timeout_flg[i] = 1;
//        }
//        else {}
//    }
//    #endif
//}

//static void can_rx_process(void)
//{
//    can_0cfe6C27_rx_process();
//}

/* 10ms */
//static void can_tx_process(void)
//{
//    #if 0
//    stPDU_INFO pdu;
//    uint08 tx_data[CAN_DATA_LENGTH];
//
//    /* can0 tx */
//    s_can0_tx_period_cnt[CAN_TX_ID_TMC_SYS_ST1]++;
//    if (s_can0_tx_period_cnt[CAN_TX_ID_TMC_SYS_ST1] >= (100 + s_can0_tx_init_cnt[CAN_TX_ID_TMC_SYS_ST1])) /* 1000ms */
//    {
//        s_can0_tx_period_cnt[CAN_TX_ID_TMC_SYS_ST1] = s_can0_tx_init_cnt[CAN_TX_ID_TMC_SYS_ST1];
//        common_memset((uint08*)tx_data, 0U, sizeof(tx_data));
//        pdu.sdu_data = (uint08*)tx_data;
//        pdu.sdu_len = CAN_DATA_LENGTH;
//        
//        canif_transmit(CANIF_HTH0_TMC_SYS_ST1, &pdu);
//    }
//    else {}
//    #endif
//}

//static void can_0cfe6C27_rx_process(void)
//{
//    uint32 temp = 0;
//    
//    if (CAN_RX_DATA_UNUSED == s_can0_rx_flg[CAN_RX_ID_0CFE6C27])
//    {
//        s_can0_rx_flg[CAN_RX_ID_0CFE6C27] = CAN_RX_DATA_USED;
//
//        CAN_USER_DISABLE_IRQ();
//        s_can0_rx_lock[CAN_RX_ID_0CFE6C27] = CAN_RX_DATA_LOCK;
//        CAN_USER_ENABLE_IRQ();
//        /* Data that needs to cross bytes is processed here */
//        temp = s_can0_rx_data.ID_0cfe6c27.vehicle_speed_h;
//        temp <<= 8U;
//        temp += s_can0_rx_data.ID_0cfe6c27.vehicle_speed_l;
//        if (temp > 0xFAFF)
//        {
//            temp = 0xFAFF;
//        }
//        else {}
//        g_vehicle_spped = temp / 256U;
//        g_vehicle_spped_original = temp;
//        CAN_USER_DISABLE_IRQ();
//        s_can0_rx_lock[CAN_RX_ID_0CFE6C27] = CAN_RX_DATA_UNLOCK;
//        CAN_USER_ENABLE_IRQ();
//    }
//}
//


