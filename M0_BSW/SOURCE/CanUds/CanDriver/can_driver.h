/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_driver.h
* Author        : yangming
* Date          : 2024-05-29
* Version       : 1.00
* Description   : Can driver.
* Others        : None
*
****************************************************************************************************/
#ifndef _CAN_DRIVER_H
#define _CAN_DRIVER_H
#include "platform_common_typdef.h"
//#include "cy_project.h"
#include "m0_canfd_cfg.h"
//#if (PLATFORM_CHIP_SELECT == CHIP_S32K144)
//#include "flexcan_driver.h"
////#include "canCom1.h"
////#include "canCom2.h"
////#include "canCom3.h"
//
#define CAN0_INSTANCE                           CY_CANFD0_TYPE
#define CAN1_INSTANCE                           CY_CANFD0_TYPE
#define CAN2_INSTANCE                           CY_CANFD0_TYPE
//
#define CAN0_TX_MB_MIN                          0U /* configurable */
#define CAN1_TX_MB_MIN                          0U /* configurable */
#define CAN2_TX_MB_MIN                          0U /* configurable */
#define CAN0_TX_MB_MAX                          TX_BUFFER_NUM /* fixed */
#define CAN1_TX_MB_MAX                          TX_BUFFER_NUM /* fixed */
#define CAN2_TX_MB_MAX                          TX_BUFFER_NUM /* fixed */
//
//#define CAN_DRV_TX_SYNC_NUM                     8U
//#endif
//
#define CAN_DATA_LENGTH                         8U
//
typedef struct _can_drv_msg                     stCAN_DRV_MSG;
//
enum _can_channel
{
    CAN_CHN_0 , 
    CAN_CHN_1,
    CAN_CHN_2,
    
    CAN_CHN_NUM
};

typedef enum _can_channel                            eCAN_CHN;

struct _can_drv_msg
{
    eCAN_CHN                    chn;
    uint32                      canid;
    uint08                      dlc;
    uint08*                     data;
};
//
//extern void can_drv_init(eCAN_CHN chn);
//extern void can_drv_deinit(eCAN_CHN chn);
//extern void can_transmit(eCAN_CHN chn, uint32 canid, uint08 dlc, uint08* data);
//extern void can_receive(eCAN_CHN chn, uint32 canid, uint08 dlc, uint08* data);
//extern void can_driver_process(void);
//extern stCAN_DRV_MSG* can_drv_tx_msg_ptr_get(void);
//extern stCAN_DRV_MSG g_can_msg_buf[CAN_DRV_TX_SYNC_NUM];
//extern void can_drv_canmsg_buf_clr(void);
#endif /* _CAN_DRIVER_H */

