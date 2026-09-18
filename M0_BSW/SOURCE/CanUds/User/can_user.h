/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_user.h
* Author        : yangming
* Date          : 2024-04-18
* Version       : 1.00
* Description   : Can user communcation.
* Others        : None
*
****************************************************************************************************/
#ifndef _CAN_USER_H
#define _CAN_USER_H
#include "canif.h"
//#include "interrupt_manager.h"

#define CAN_USER_DISABLE_IRQ()                          TASK_DISABLE_IRQ(interruptState)
#define CAN_USER_ENABLE_IRQ()                           TASK_ENABLE_IRQ(interruptState)

/* CAN receive ID mapped to canif */
#define CAN_RX_ID_0CFE6C27                              (CANIF_HRH0_COM_0CFE6C27 - CANIF_HRH_UDS_NUM)
#define CAN0_RX_NUM                                     (CANIF_HRH0_NUM - CANIF_HRH_UDS_NUM)
#define CAN1_RX_NUM                                     1//(CANIF_HRH1_NUM - CANIF_HRH0_NUM)
#define CAN0_RX_DATA_NUM                                (CAN0_RX_NUM * CAN_DATA_LENGTH)
#define CAN1_RX_DATA_NUM                                (CAN1_RX_NUM * CAN_DATA_LENGTH)
#define CAN_RX_DATA_UNLOCK                              0
#define CAN_RX_DATA_LOCK                                1
#define CAN_RX_DATA_USED                                0
#define CAN_RX_DATA_UNUSED                              1

//#define CAN_TX_ID_TMC_SYS_ST1                           (CANIF_HTH0_TMC_SYS_ST1 - CANIF_HTH_UDS_FNUM)
#define CAN0_TX_NUM                                     1//(CANIF_HTH0_NUM - CANIF_HTH_UDS_FNUM)
#define CAN1_TX_NUM                                     1//(CANIF_HTH1_NUM - CANIF_HTH0_NUM)

#define CUSTOM_BOOT_RES_WAIT_MS                         100U
#define CUSTOM_BOOT_RES_PERIOD_MS                       1000U

typedef union _can0_rx_data                             unCAN0_RX_DATA;
typedef union _can1_rx_data                             unCAN1_RX_DATA;

#define BUSOFF_CAN_ID_INDEX                             (CANIF_BUSOFF_CAN - CANIF_HTH_UDS_FNUM)           
//union _can0_rx_data
//{
//    uint08 data[CAN0_RX_DATA_NUM];
//    struct
//    {
//        /* byte[5-0] */
//        uint08                      reserved_0[6];
//        /* byte[6] */
//        uint08                      vehicle_speed_l;
//        /* byte[7] */
//        uint08                      vehicle_speed_h;
//    }ID_0cfe6c27;
//};
//
//union _can1_rx_data
//{
//    uint08 data[CAN1_RX_DATA_NUM];
//    struct
//    {
//        /* byte[7-0] */
//        uint08                      reserved_0[8];
//    }ID_ACP_RSP;
//};

typedef struct
{
  cy_pstc_canfd_type_t    chn;
  bool                    canFDFormat;
  uint32_t                id;
  uint8_t                 data[64];
  uint16_t                datalen;
  
  
}_ST_CAN_TX_MSG;


extern uint16 g_vehicle_speed;
extern uint16 g_vehicle_speed_original;

extern void can_user_init(void);
extern void can_user_process(void);
extern void can_enable(eCAN_CHN chn);
extern void can_disable(eCAN_CHN chn);
extern void can0_user_rx_cbk(uint32 canid, uint08* pdata);
extern void can1_user_rx_cbk(uint32 canid, uint08* pdata);
extern void can0_user_busoff_cbk(void);
extern void can1_user_busoff_cbk(void);
extern void can_user_10ms_period_process(void);
extern uint08 can_user_hcu_timeout_get(void);
extern uint08 can_user_ac_timeout_get(void);
extern uint08 can_user_compressor_timeout_get(void);
extern void Can_Transmit(cy_pstc_canfd_type_t chn, uint32_t canid, uint8_t dlc, uint8_t * data,bool fd);

#endif /* _CAN_USER_H */

