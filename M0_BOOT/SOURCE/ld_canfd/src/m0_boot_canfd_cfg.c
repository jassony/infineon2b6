/*
 * m0_canfd_cfg.c
 *
 *  Created on: 2026年1月22日
 *      Author: hzldy
 */

#include "m0_boot_canfd_cfg.h"
#include "cy_canfd.h"
#include "m0_boot_var.h"
//#include "can_user.h"
#include "uds_user.h"

#define NON_ISO_OPERATION 1
/* CAN in Use */
static void CAN_TxMsgCallback(void);
static void CAN_RxMsgCallback(bool bRxFifoMsg, uint8_t u8MsgBufOrRxFifoNum, cy_stc_canfd_msg_t* pstcCanFDmsg);
static void CAN_RxFifoWithTopCallback(uint8_t u8FifoNum, uint8_t u8BufferSizeInWord, uint32_t* pu32RxBuf);
static void CAN_ErrCallback(cy_en_canfd_bus_error_t enCanFDError);



#if NON_ISO_OPERATION == 1
static void SetISOFormat(cy_pstc_canfd_type_t canfd);
#endif

static const cy_stc_id_filter_t stdIdFilter[] = 
{
    CANFD_CONFIG_STD_ID_FILTER_CLASSIC_RXBUFF(TSET_STD_CAN_ID0, TEST_STD_CAN_INDEX0),      /* ID=0x010, store into RX buffer Idx0 */
    CANFD_CONFIG_STD_ID_FILTER_CLASSIC_RXBUFF(TSET_STD_CAN_ID1, TEST_STD_CAN_INDEX1),      /* ID=0x020, store into RX buffer Idx1 */

    
//id

//    CANFD_CONFIG_STD_ID_FILTER_CLASSIC(TSET_STD_CAN_FID0,TSET_STD_CAN_MASK0,CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO0),
//    CANFD_CONFIG_STD_ID_FILTER_CLASSIC(TSET_STD_CAN_FID1,TSET_STD_CAN_MASK1,CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO0),
////filter
};


///* Extended ID Filter configration */
static const cy_stc_extid_filter_t extIdFilter[] = 
{
    CANFD_CONFIG_EXT_ID_FILTER_CLASSIC_RXBUFF(TSET_EXT_CAN_ID0, TEST_EXT_CAN_INDEX0),    /* ID=0x10010, store into RX buffer Idx2 */
    CANFD_CONFIG_EXT_ID_FILTER_CLASSIC_RXBUFF(TSET_EXT_CAN_ID1, TEST_EXT_CAN_INDEX1),    /* ID=0x10020, store into RX buffer Idx3 */
    CANFD_CONFIG_EXT_ID_FILTER_CLASSIC_RXBUFF(CAN_EXT_PHY_RX_ID, CAN_EXT_PHY_RX_INDEX2),    /* ID=0x10020, store into RX buffer Idx3 */
    CANFD_CONFIG_EXT_ID_FILTER_CLASSIC_RXBUFF(CAN_EXT_FUNC_RX_ID, CAN_EXT_FUNC_RX_INDEX3),    /* ID=0x10020, store into RX buffer Idx3 */

    
//id

    
//    CANFD_CONFIG_EXT_ID_FILTER_CLASSIC(TSET_EXT_CAN_FID0,TSET_EXT_CAN_MASK0,CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO1),
//    CANFD_CONFIG_EXT_ID_FILTER_CLASSIC(TSET_EXT_CAN_FID1,TSET_EXT_CAN_MASK1,CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO1),
////filter

};

static cy_stc_canfd_config_t canCfg = 
{
    .txCallback     = CAN_TxMsgCallback, // Unused.
    .rxCallback     = CAN_RxMsgCallback,
    .rxFifoWithTopCallback = CAN_RxFifoWithTopCallback, //CAN_RxFifoWithTopCallback,
    .statusCallback = NULL, // Un-supported now
    .errorCallback  = CAN_ErrCallback, // Un-supported now

    .canFDMode      = true, // Use CANFD mode

    // 40 MHz
    .bitrate        =       // Nominal bit rate settings (sampling point = 75%)
    {
        .prescaler      = CAN_PRESCALER - 1u,  // cclk/10, When using 500kbps, 1bit = 8tq
        .timeSegment1   = CAN_TSEG1 - 1u,  // tseg1 = 5tq
        .timeSegment2   = CAN_TSEG2 - 1u,  // tseg2 = 2tq
        .syncJumpWidth  = 1u - 1u,  // sjw   = 2tq
    },
    
    .fastBitrate    =       // Fast bit rate settings (sampling point = 75%)
    {
        .prescaler      = CANFD_PRESCALER - 1u,  // cclk/2, When using 2Mbps, 1bit = 10tq
        .timeSegment1   = CANFD_TSEG1 - 1u,  // tseg1 = 5tq,
        .timeSegment2   = CANFD_TSEG2 - 1u,  // tseg2 = 2tq
        .syncJumpWidth  = 1u - 1u,  // sjw   = 2tq
//            .prescaler      = CAN_PRESCALER - 1u,  // cclk/10, When using 500kbps, 1bit = 8tq
//        .timeSegment1   = CAN_TSEG1 - 1u,  // tseg1 = 5tq
//        .timeSegment2   = CAN_TSEG2 - 1u,  // tseg2 = 2tq
//        .syncJumpWidth  = 1u - 1u,  // sjw   = 2tq
    },

    .tdcConfig      =       // Transceiver delay compensation, unused.
    {
        .tdcEnabled     = true,
        .tdcOffset      = 8,
        .tdcFilterWindow= 1,
    },
    .sidFilterConfig    =   // Standard ID filter
    {
        .numberOfSIDFilters = sizeof(stdIdFilter) / sizeof(stdIdFilter[0]),
        .sidFilter          = stdIdFilter,
    },
    .extidFilterConfig  =   // Extended ID filter
    {
        .numberOfEXTIDFilters   = sizeof(extIdFilter) / sizeof(extIdFilter[0]),
        .extidFilter            = extIdFilter,
        .extIDANDMask           = 0x1fffffff,   // No pre filtering.
    },
    .globalFilterConfig =   // Global filter
    {
        .nonMatchingFramesStandard = CY_CANFD_REJECT_NON_MATCHING,  // Reject none match IDs
        .nonMatchingFramesExtended = CY_CANFD_REJECT_NON_MATCHING,  // Reject none match IDs
        .rejectRemoteFramesStandard = true, // No remote frame
        .rejectRemoteFramesExtended = true, // No remote frame
    },
    .rxBufferDataSize = CY_CANFD_BUFFER_DATA_SIZE_64,
    .rxFifo1DataSize  = CY_CANFD_BUFFER_DATA_SIZE_64,
    .rxFifo0DataSize  = CY_CANFD_BUFFER_DATA_SIZE_64,
    .txBufferDataSize = CY_CANFD_BUFFER_DATA_SIZE_64,
    .rxFifo0Config    = // RX FIFO0, unused.
    {
        .mode = CY_CANFD_FIFO_MODE_BLOCKING,
        .watermark = 10u,
        .numberOfFifoElements = 8u,
        .topPointerLogicEnabled = false,
    },
    .rxFifo1Config    = // RX FIFO1, unused.
    {
        .mode = CY_CANFD_FIFO_MODE_BLOCKING,
        .watermark = 10u,
        .numberOfFifoElements = 8u,
        .topPointerLogicEnabled = false, // true,
    },
    .noOfRxBuffers  = CAN_ID_NUM,
    .noOfTxBuffers  = TX_BUFFER_NUM,
};

#if NON_ISO_OPERATION == 1
static void SetISOFormat(cy_pstc_canfd_type_t canfd)
{
    /* Now a ch configured as CANFD is working. */
    canfd->M_TTCAN.unCCCR.stcField.u1INIT = 1;
    while(canfd->M_TTCAN.unCCCR.stcField.u1INIT != 1);
        /* Cancel protection by setting CCE */
    canfd->M_TTCAN.unCCCR.stcField.u1CCE = 1;
    canfd->M_TTCAN.unCCCR.stcField.u1NISO = 1;

    canfd->M_TTCAN.unCCCR.stcField.u1INIT = 0;
    while(canfd->M_TTCAN.unCCCR.stcField.u1INIT != 0);
}
#endif

static void CAN_TxMsgCallback(void)
{
    /* Just loop back to the sender with +1 ID */
//    pstcCanFDmsg->idConfig.identifier += 1u;
//    Cy_CANFD_UpdateAndTransmitMsgBuffer
//    (
//        CY_CANFD0_TYPE,
//        0u,
//        pstcCanFDmsg
//    );
  
}

static void CAN_RxMsgCallback(bool bRxFifoMsg, uint8_t u8MsgBufOrRxFifoNum, cy_stc_canfd_msg_t* pstcCanFDmsg)
{
    /* Just loop back to the sender with +1 ID */
//    can_rx_data[u8MsgBufOrRxFifoNum].rx_flag = 1;
//    can_rx_data[u8MsgBufOrRxFifoNum].canFDFormat = pstcCanFDmsg->canFDFormat;
//    can_rx_data[u8MsgBufOrRxFifoNum].id = pstcCanFDmsg->idConfig.identifier;
//    can_rx_data[u8MsgBufOrRxFifoNum].extended = pstcCanFDmsg->idConfig.extended;
//    can_rx_data[u8MsgBufOrRxFifoNum].datalen = pstcCanFDmsg->dataConfig.dataLengthCode;
//    can_rx_data[u8MsgBufOrRxFifoNum].rx_flag = true;
//    memcpy(can_rx_data[u8MsgBufOrRxFifoNum].data,pstcCanFDmsg->dataConfig.data,pstcCanFDmsg->dataConfig.dataLengthCode);
    can0_uds_rx_cbk(pstcCanFDmsg->idConfig.identifier,(uint8_t*)pstcCanFDmsg->dataConfig.data, pstcCanFDmsg->dataConfig.dataLengthCode);

//    pstcCanFDmsg->idConfig.identifier += 1u;
//    Cy_CANFD_UpdateAndTransmitMsgBuffer
//    (
//        CY_CANFD0_TYPE,
//        0u,
//        pstcCanFDmsg
//    );
  
}

static void CAN_ErrCallback(cy_en_canfd_bus_error_t enCanFDError)//无效
{
      Canfd_Deinit();
      Canfd_Init();
      can0_user_busoff_cbk();

}

static void CAN_RxFifoWithTopCallback(uint8_t u8FifoNum, uint8_t   u8BufferSizeInWord, uint32_t* pu32RxBuf)
{
    /*TODO*/
  
}



void Canfd_Init(void)
{
  if(CAN_ID_NUM > 64)
    while(1)
    {
    
    };
  Cy_CANFD_Init(CY_CANFD0_TYPE, &canCfg);
  CANFD0->CH[0].M_TTCAN.unIE.stcField.u1BOE = 1;//ENABLE BUSOFF
  CANFD0->CH[0].M_TTCAN.unIE.stcField.u1PEDE = 1;//ENABLE PEA
  CANFD0->CH[0].M_TTCAN.unTXBTIE.u32Register = 1; //ENABLE TX INT
  can_user_init();
#if NON_ISO_OPERATION == 1
  SetISOFormat(CY_CANFD0_TYPE);
#endif

}



void Canfd_Deinit(void)
{
  Cy_CANFD_DeInit(CY_CANFD0_TYPE);
  
}



void CanfdInterruptHandler(void)
{
    /* Just invoking */
    Cy_CANFD_IrqHandler(CY_CANFD0_TYPE);
    if(CY_CANFD0_TYPE->M_TTCAN.unIR.stcField.u1BO_ == 1UL)
    {
      CY_CANFD0_TYPE->M_TTCAN.unIR.stcField.u1BO_ = 1UL;
      CAN_ErrCallback(CY_CANFD_BUSOFF);
      
    }
    if(CY_CANFD0_TYPE->M_TTCAN.unIR.stcField.u1PED == 1UL)
    {
      CY_CANFD0_TYPE->M_TTCAN.unIR.stcField.u1PED = 1UL;
//      canCfg.tdcConfig.tdcOffset++;
//      if(canCfg.tdcConfig.tdcOffset == 128)
//      {
//        canCfg.tdcConfig.tdcOffset = 0;
//        canCfg.tdcConfig.tdcFilterWindow ++;
//      }
//      if(canCfg.tdcConfig.tdcFilterWindow == 128)
//          canCfg.tdcConfig.tdcFilterWindow = 0;
//      
    }
}

