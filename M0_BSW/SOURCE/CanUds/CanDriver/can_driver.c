/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_driver.c
* Author        : yangming
* Date          : 2024-05-29
* Version       : 1.00
* Description   : Can driver.
* Others        : None
*
****************************************************************************************************/
#include "can_driver.h"
#include "common_mem_op.h"
//#include "output.h"
#include "can_user.h"
#include "uds_user.h"
#include "canif_cfg.h"
#include "can_busoff.h"

//#if (PLATFORM_CHIP_SELECT == CHIP_S32K144)
//#include"Cpu.h"
//#endif

//static stCAN_DRV_MSG s_can_msg;
//stCAN_DRV_MSG g_can_msg_buf[CAN_DRV_TX_SYNC_NUM];
//static uint08 s_can_msgbuf_index;
//static uint08 s_can_drv_data[CAN_DATA_LENGTH];
//static stCAN_DRV_MSG s_can_msg_repeat;
//static uint08 s_can_drv_data_repeat[CAN_DATA_LENGTH];
//static uint08 s_can_drv_tx_repeat;
//
#if (PLATFORM_CHIP_SELECT == CHIP_S32K144)
//flexcan_msgbuff_t s_can0_rx_msgbuf;
//flexcan_msgbuff_t s_can1_rx_msgbuf;
//flexcan_msgbuff_t s_can2_rx_msgbuf;
//static flexcan_data_info_t s_flexcan_data_info = 
//{
//    .msg_id_type            = FLEXCAN_MSG_ID_EXT,
//    .data_length            = 8U,
//    .is_remote              = false
//};
//
//static const flexcan_id_table_t s_flexcan0_id_table[] =
//{
////    {.id = CAN_PHY_RX_ID,               .isExtendedFrame = true,    .isRemoteFrame = false},
////    {.id = CAN_PHY2_RX_ID,              .isExtendedFrame = true,    .isRemoteFrame = false},
////    {.id = CAN_FUNC_RX_ID,              .isExtendedFrame = true,    .isRemoteFrame = false},
////    {.id = CAN_FUNC2_RX_ID,             .isExtendedFrame = true,    .isRemoteFrame = false},
//    {.id = CAN_COM_0CFE6C27_RX_ID,      .isExtendedFrame = true,    .isRemoteFrame = false},
//};
//
//static const flexcan_id_table_t s_flexcan1_id_table[] =
//{
//    {.id = CAN_COM_7DF_RX_ID,           .isExtendedFrame = false,   .isRemoteFrame = false},
//    {.id = CAN_PHY_RX_ID,               .isExtendedFrame = true,    .isRemoteFrame = false }, /* UDS PHY */
//    {.id = CAN_FUNC_RX_ID,              .isExtendedFrame = true,    .isRemoteFrame = false }, /* UDS FUNC */
//};
//
//#if 1
//static const flexcan_id_table_t s_flexcan2_id_table[] =
//{
//    {.id = CAN_PHY_RX_ID,               .isExtendedFrame = true,    .isRemoteFrame = false }, /* UDS PHY */
//    {.id = CAN_FUNC_RX_ID,              .isExtendedFrame = true,    .isRemoteFrame = false }, /* UDS FUNC */
//};
//#endif

//static void can_rtx_callback(uint08 instance,
//                                   flexcan_event_type_t eventType,
//                                   uint32 buffIdx,
//                                   flexcan_state_t* flexcanState);
//static void can_error_callback(uint08 instance,
//                                      flexcan_event_type_t eventType,
//                                      flexcan_state_t* flexcanState);
//static void s32k144_can0_init(void);
//static void s32k144_can1_init(void);
//static void s32k144_can2_init(void);
//static void s32k144_can_deinit(uint08 instance);
#endif

//void can_drv_init(eCAN_CHN chn)
//{
//    if (CAN_CHN_0 == chn)
//    {
//        #if (PLATFORM_CHIP_SELECT == CHIP_S32K144)
//        can0_rtx_init();
//        s32k144_can0_init();
//        #endif
//    }
//    else if (CAN_CHN_1 == chn)
//    {
//        #if (PLATFORM_CHIP_SELECT == CHIP_S32K144)
//        can1_rtx_init();
//        s32k144_can1_init();
//        #endif
//    }
//    else if (CAN_CHN_2 == chn)
//    {
//        #if (PLATFORM_CHIP_SELECT == CHIP_S32K144)
//        can2_rtx_init();
//        s32k144_can2_init();
//        #endif
//    }
//    else
//    { /* nothing */ }
//
//    s_can_msg.data = (uint08*)s_can_drv_data;
//    s_can_drv_tx_repeat = 0;
//}

//void can_drv_deinit(eCAN_CHN chn)
//{
//    if (CAN_CHN_0 == chn)
//    {
//        s32k144_can_deinit(CAN0_INSTANCE);
//    }
//    else if (CAN_CHN_1 == chn)
//    {
//        s32k144_can_deinit(CAN1_INSTANCE);
//    }
//    else if (CAN_CHN_2 == chn)
//    {
//        s32k144_can_deinit(CAN2_INSTANCE);
//    }
//    else
//    { /* nothing */ } 
//}

//void can_transmit(eCAN_CHN chn, uint32 canid, uint08 dlc, uint08* data)
//{
//    #if (PLATFORM_CHIP_SELECT == CHIP_S32K144)
//    uint08 instance = 0;
//    uint08 mb_idx = 0;
//    uint08 mb_max = 0;
//    status_t sts = STATUS_SUCCESS;
//    
//    instance = (uint08)chn;
//    if (CAN_CHN_0 == instance)
//    {
//        mb_max = CAN0_TX_MB_MAX;
//        mb_idx = CAN0_TX_MB_MIN;
//    }
//    else if (CAN_CHN_1 == instance)
//    {
//        mb_max = CAN1_TX_MB_MAX;
//        mb_idx = CAN1_TX_MB_MIN;
//    }
//    else if (CAN_CHN_2 == instance)
//    {
//        mb_max = CAN2_TX_MB_MAX;
//        mb_idx = CAN2_TX_MB_MIN;
//    }
//    else
//    {
//        /* error */
//    }
//
//    if (canid & CANIF_ID_EXT_MASK)
//    {
//        s_flexcan_data_info.msg_id_type = FLEXCAN_MSG_ID_EXT;
//    }
//    else
//    {
//        s_flexcan_data_info.msg_id_type = FLEXCAN_MSG_ID_STD;
//    }
//    s_flexcan_data_info.data_length = dlc;
//    while (mb_idx < mb_max)
//    {
//        sts = FLEXCAN_DRV_GetTransferStatus(instance, mb_idx);
//        if (STATUS_SUCCESS == sts)
//        {
//            break;
//        }
//        else
//        {
//            mb_idx++;
//        }
//    }
//    if (mb_idx < mb_max)
//    {
//        FLEXCAN_DRV_Send(instance, mb_idx, &s_flexcan_data_info, canid, data);
//        s_can_msg.chn = chn;
//        s_can_msg.canid = canid;
//        s_can_msg.dlc = dlc;
//        common_memcpy((uint08*)s_can_msg.data, (uint08*)data, dlc);
//        
//        if (s_can_msgbuf_index < CAN_DRV_TX_SYNC_NUM)
//        {
//            g_can_msg_buf[s_can_msgbuf_index].chn = chn;
//            g_can_msg_buf[s_can_msgbuf_index].canid = canid;
//            g_can_msg_buf[s_can_msgbuf_index].dlc = dlc;
//            s_can_msgbuf_index++;
//            if (s_can_msgbuf_index >= CAN_DRV_TX_SYNC_NUM)
//            {
//                s_can_msgbuf_index = 0;
//            }
//            else {}
//        }
//        else
//        {
//            s_can_msgbuf_index = 0;
//        }
//        
//        if (DEF_NULL != s_can_msg_repeat.data)
//        {
//            common_memcpy((uint08*)g_can_msg_buf[s_can_msgbuf_index].data, (uint08*)data, dlc);
//        }
//        else {}
//    }
//    else
//    {
//        if (0 == s_can_drv_tx_repeat)
//        {
//            s_can_msg_repeat.chn = chn;
//            s_can_msg_repeat.canid = canid;
//            s_can_msg_repeat.dlc = dlc;
//            common_memcpy((uint08*)s_can_msg_repeat.data, (uint08*)data, dlc);
//            s_can_drv_tx_repeat = 1;    
//        }
//        else {}
////        CAN_DEBUG_PRINT("[CANDRV-T]Transmit failed, canid = %x, mb_idx = %d. \r\n", canid, mb_idx);
//    }
//    #endif
//}

//void can_receive(eCAN_CHN chn, uint32 canid, uint08 dlc, uint08* data)
//{
//    #if (PLATFORM_CHIP_SELECT == CHIP_S32K144)
//    uint08 instance = 0;
//    flexcan_msgbuff_t recv_msg;
//    
//    instance = (uint08)chn;
//    if (CAN0_INSTANCE == instance)
//    {
//        common_memcpy((uint08*)&recv_msg, (uint08*)&s_can0_rx_msgbuf, sizeof(flexcan_msgbuff_t));
//    }
//    else if (CAN1_INSTANCE == instance)
//    {
//        common_memcpy((uint08*)&recv_msg, (uint08*)&s_can1_rx_msgbuf, sizeof(flexcan_msgbuff_t));
//    }
//    else if (CAN2_INSTANCE == instance)
//    {
//        common_memcpy((uint08*)&recv_msg, (uint08*)&s_can2_rx_msgbuf, sizeof(flexcan_msgbuff_t));
//    }
//    else
//    {
//        return;
//    }
//    
//    if (canid == recv_msg.msgId)
//    {
//        common_memcpy(data, (uint08*)recv_msg.data, dlc);
//    }
//    else
//    {
//        /* nothing */
//    }
//    #endif
//}

//void can_driver_process(void)
//{
//    if (s_can_drv_tx_repeat)
//    {
//        s_can_drv_tx_repeat = 0;
//        can_transmit(s_can_msg_repeat.chn, s_can_msg_repeat.canid, s_can_msg_repeat.dlc, s_can_msg_repeat.data);
//    }
//    else
//    {}
//}

//stCAN_DRV_MSG* can_drv_tx_msg_ptr_get(void)
//{
//    return (stCAN_DRV_MSG*)&s_can_msg;
//}

#if (PLATFORM_CHIP_SELECT == CHIP_S32K144)
//static void can_rtx_callback(uint08 instance,
//                                   flexcan_event_type_t eventType,
//                                   uint32 buffIdx,
//                                   flexcan_state_t* flexcanState)
//{
//    (void)buffIdx;
//    (void)flexcanState;
//    
//    if (CAN0_INSTANCE == instance)
//    {
//        if (FLEXCAN_EVENT_RXFIFO_COMPLETE == eventType) /* rx isr */
//        {
//  		    can0_user_rx_cbk(s_can0_rx_msgbuf.msgId, s_can0_rx_msgbuf.data);
//  		    FLEXCAN_DRV_RxFifo(instance, &s_can0_rx_msgbuf);
//        }
//        else {}
//    }
//    else if (CAN1_INSTANCE == instance)
//    {
//        if (FLEXCAN_EVENT_RXFIFO_COMPLETE == eventType) /* rx isr */
//        {
//            can1_uds_rx_cbk(s_can1_rx_msgbuf.msgId,  s_can1_rx_msgbuf.data, s_can1_rx_msgbuf.dataLen);
//            // can1_user_rx_cbk(s_can1_rx_msgbuf.msgId,  s_can1_rx_msgbuf.data);
//  		    FLEXCAN_DRV_RxFifo(instance, &s_can1_rx_msgbuf);
//        }
//        else if (eventType == FLEXCAN_EVENT_TX_COMPLETE)
//        {
//            can1_uds_tx_cbk(can_drv_tx_msg_ptr_get()->canid);
//        }
//    }
//    else if (CAN2_INSTANCE == instance)
//    {
//        if (FLEXCAN_EVENT_RXFIFO_COMPLETE == eventType) /* rx isr */
//        {
//            can2_uds_rx_cbk(s_can2_rx_msgbuf.msgId,  s_can2_rx_msgbuf.data, s_can2_rx_msgbuf.dataLen);
//  		    FLEXCAN_DRV_RxFifo(instance, &s_can2_rx_msgbuf);
//        }
//        else if (eventType == FLEXCAN_EVENT_TX_COMPLETE)
//        {
//            can2_uds_tx_cbk(can_drv_tx_msg_ptr_get()->canid);
//        }
//        else {}
//    }
//    else {}
//}

//static void can_error_callback(uint08 instance,
//                                      flexcan_event_type_t eventType,
//                                      flexcan_state_t* flexcanState)
//{
//    uint32 sts = 0U;
//    
//    if (CAN0_INSTANCE == instance)
//    {
//        if (FLEXCAN_EVENT_ERROR == eventType) /* error isr */
//        {
//            sts = FLEXCAN_DRV_GetErrorStatus(instance);
//            if (CAN_ESR1_BOFFINT(1) & sts)
//            {
//                can0_user_busoff_cbk();
//            }
//            else {}
//        }
//        else {}
//    }
//    else if (CAN1_INSTANCE == instance)
//    {
//        if (FLEXCAN_EVENT_ERROR == eventType) /* error isr */
//        {
//            sts = FLEXCAN_DRV_GetErrorStatus(instance);
//            if (CAN_ESR1_BOFFINT(1) & sts)
//            {
//                can1_user_busoff_cbk();
////                can_busoff_occur_cbk();
//            }
//            else {}
//        }
//        else {}
//    }
//    else if (CAN2_INSTANCE == instance)
//    {
//
//    }
//    else {}
//}

//static void s32k144_can0_init(void)
//{
//    FLEXCAN_DRV_Init(CAN0_INSTANCE, &canCom1_State, &canCom1_InitConfig0);
//	FLEXCAN_DRV_SetRxMaskType(CAN0_INSTANCE, FLEXCAN_RX_MASK_INDIVIDUAL);
//	FLEXCAN_DRV_ConfigRxFifo(CAN0_INSTANCE,FLEXCAN_RX_FIFO_ID_FORMAT_A, s_flexcan0_id_table);
//	FLEXCAN_DRV_InstallEventCallback(CAN0_INSTANCE, can_rtx_callback, NULL);
//	FLEXCAN_DRV_InstallErrorCallback(CAN0_INSTANCE, can_error_callback, NULL);
//	FLEXCAN_DRV_RxFifo(CAN0_INSTANCE, &s_can0_rx_msgbuf);
//}
//
//static void s32k144_can1_init(void)
//{
//    FLEXCAN_DRV_Init(CAN1_INSTANCE, &canCom2_State, &canCom2_InitConfig0);
//	FLEXCAN_DRV_SetRxMaskType(CAN1_INSTANCE, FLEXCAN_RX_MASK_INDIVIDUAL);
//	FLEXCAN_DRV_ConfigRxFifo(CAN1_INSTANCE,FLEXCAN_RX_FIFO_ID_FORMAT_B, s_flexcan1_id_table);
//	FLEXCAN_DRV_InstallEventCallback(CAN1_INSTANCE, can_rtx_callback, NULL);
//	FLEXCAN_DRV_InstallErrorCallback(CAN1_INSTANCE, can_error_callback, NULL);
//	FLEXCAN_DRV_RxFifo(CAN1_INSTANCE, &s_can1_rx_msgbuf);
//}
//
//static void s32k144_can2_init(void)
//{
//    #if 1
//    FLEXCAN_DRV_Init(CAN2_INSTANCE, &canCom3_State, &canCom3_InitConfig0);
//	FLEXCAN_DRV_SetRxMaskType(CAN2_INSTANCE, FLEXCAN_RX_MASK_INDIVIDUAL);
//	FLEXCAN_DRV_ConfigRxFifo(CAN2_INSTANCE,FLEXCAN_RX_FIFO_ID_FORMAT_A, s_flexcan2_id_table);
//	FLEXCAN_DRV_InstallEventCallback(CAN2_INSTANCE, can_rtx_callback, NULL);
//	FLEXCAN_DRV_InstallErrorCallback(CAN2_INSTANCE, can_error_callback, NULL);
//	FLEXCAN_DRV_RxFifo(CAN2_INSTANCE, &s_can2_rx_msgbuf);
//	#endif
//}
//
//static void s32k144_can_deinit(uint08 instance)
//{
//    FLEXCAN_DRV_Deinit(instance);
//}
#endif

