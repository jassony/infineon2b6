/*
 ***********************************************************************************************************************
 *
 * Copyright (c) Infineon Technologies AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,are permitted provided that the
 * following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this list of conditions and the  following
 *   disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 *   following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 *   Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote
 *   products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT  OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **********************************************************************************************************************/


/** \addtogroup TLE9563_FUNCLAYER_api
 *  @{
 */

/*******************************************************************************
**                                  Includes                                  **
*******************************************************************************/
#include "TLE9563_FuncLayer.h"

/*******************************************************************************
**                           Local Macro Definitions                          **
*******************************************************************************/


/*******************************************************************************
**                           Local Type Definitions                           **
*******************************************************************************/
/** \brief function pointer fpDEVICE_initDeviceReg
*/
typedef uint8_t (*fpDEVICE_initDeviceReg)(uint16_t);

/******************************************************************************/
/**                        Local Variable Definitions                        **/
/******************************************************************************/
static uint8_t u8_cyclicStatUpdateCount = 0;
static fpDEVICE_getDeviceReg fp_cyclicStatGetFuncs[] =  {  &TLE9563_getSUPSTAT,
                                                           &TLE9563_getTHERMSTAT,
                                                           &TLE9563_getDEVSTAT,
                                                           &TLE9563_getBUSSTAT,
                                                           &TLE9563_getWKSTAT,
                                                           &TLE9563_getWKLVLSTAT,
                                                           &TLE9563_getHSOLOCOTSTAT,
                                                           &TLE9563_getGENSTAT,
                                                           &TLE9563_getTDREG,
                                                           &TLE9563_getDSOV,
                                                           &TLE9563_getEFFTDONOFF1,
                                                           &TLE9563_getEFFTDONOFF2,
                                                           &TLE9563_getEFFTDONOFF3,
                                                           &TLE9563_getTRISEFALL1,
                                                           &TLE9563_getTRISEFALL2,
                                                           &TLE9563_getTRISEFALL3,
                                                           &TLE9563_getSWKSTAT,
                                                           &TLE9563_getSWKECNTSTAT,
                                                           &TLE9563_getSWKCDRSTAT,
                                                           &TLE9563_getFAMPRODSTAT,
                                                           &TLE9563_getSWKOSCCALSTAT,
                                                        };

static fpDEVICE_updateRamReg fp_cyclicStatUpdateFuncs[] = {  &TLE9563_updateRamSUPSTAT,
                                                             &TLE9563_updateRamTHERMSTAT,
                                                             &TLE9563_updateRamDEVSTAT,
                                                             &TLE9563_updateRamBUSSTAT,
                                                             &TLE9563_updateRamWKSTAT,
                                                             &TLE9563_updateRamWKLVLSTAT,
                                                             &TLE9563_updateRamHSOLOCOTSTAT,
                                                             &TLE9563_updateRamGENSTAT,
                                                             &TLE9563_updateRamTDREG,
                                                             &TLE9563_updateRamDSOV,
                                                             &TLE9563_updateRamEFFTDONOFF1,
                                                             &TLE9563_updateRamEFFTDONOFF2,
                                                             &TLE9563_updateRamEFFTDONOFF3,
                                                             &TLE9563_updateRamTRISEFALL1,
                                                             &TLE9563_updateRamTRISEFALL2,
                                                             &TLE9563_updateRamTRISEFALL3,
                                                             &TLE9563_updateRamSWKSTAT,
                                                             &TLE9563_updateRamSWKECNTSTAT,
                                                             &TLE9563_updateRamSWKCDRSTAT,
                                                             &TLE9563_updateRamFAMPRODSTAT,
                                                             &TLE9563_updateRamSWKOSCCALSTAT,
                                                          };

/******************************************************************************/
/**                        Local Function Definitions                        **/
/******************************************************************************/


/*******************************************************************************
**                        Global Variable Definitions                         **
*******************************************************************************/
extern sDEVICE_deviceDriver s_deviceDriver = { TLE9563_DEVICEDRIVER_STATUS_INIT,
                                               TLE9563_DEVICEDRIVER_ERRORLOG_INIT,
                                               0x0000,
                                               0x0000,
                                               NULL,
                                               NULL,
                                               NULL,
                                               0x0000,
                                             };

/*******************************************************************************
**                         Global Function Definitions                        **
*******************************************************************************/
/** \brief Device Driver Cyclic Task
 *  \brief This function needs to be called continuously by the application
 * 
 *  \return uint8_t Device Driver ErrorLog (0: INIT, 1: STARTED, 2: BUSY, 3: SPI_RECEIVE_ERR)
 */
uint8_t TLE9563_deviceDriverCyclicTask(void)
{ 
  uint32_t u32_pdmaInterruptStatusMasked;
  uint32_t u32_spiRxFifoStatus;
  uint16_t u16_tempRegData;
  
  switch(s_deviceDriver.u8_deviceDriverStatus)
  {
  case TLE9563_DEVICEDRIVER_STATUS_INIT:
    /* dynamic set register requests to write to control registers have highest priority */
    if(s_deviceDriver.fp_setReg)
    {
      s_deviceDriver.u8_deviceDriverStatus = TLE9563_DEVICEDRIVER_STATUS_CYCLIC_UPDATE;
      s_deviceDriver.fp_setReg(s_deviceDriver.u16_setBitValue);
      s_deviceDriver.fp_setReg = NULL;
      s_deviceDriver.u8_deviceDriverErrorLog = TLE9563_DEVICEDRIVER_ERRORLOG_STARTED;
      break;
    }
    /* cyclic get register requests to update status registers have lowest priority */
    else if(s_deviceDriver.fp_getReg)
    {
      s_deviceDriver.u8_deviceDriverStatus = TLE9563_DEVICEDRIVER_STATUS_CYCLIC_UPDATE;
      s_deviceDriver.fp_getReg = fp_cyclicStatGetFuncs[u8_cyclicStatUpdateCount];
      s_deviceDriver.fp_updateRamReg = fp_cyclicStatUpdateFuncs[u8_cyclicStatUpdateCount];
      s_deviceDriver.fp_getReg();
      if(u8_cyclicStatUpdateCount < TLE9563_DEVICEDRIVER_CYCLICSTATUPDATENUM)
      {
        u8_cyclicStatUpdateCount ++;
      }
      else
      {
        u8_cyclicStatUpdateCount = 0;
      }
      s_deviceDriver.u8_deviceDriverErrorLog = TLE9563_DEVICEDRIVER_ERRORLOG_STARTED;
      break;
    }
    else
    {
      s_deviceDriver.u8_deviceDriverErrorLog = TLE9563_DEVICEDRIVER_ERRORLOG_INIT;
      s_deviceDriver.u8_deviceDriverStatus = TLE9563_DEVICEDRIVER_STATUS_INIT;
      break;
    }
    break;
    
  case TLE9563_DEVICEDRIVER_STATUS_CYCLIC_UPDATE:
    /* check, if PDMA and SPI are done */
    u32_pdmaInterruptStatusMasked = Cy_PDMA_Chnl_GetInterruptStatusMasked(PDMA_INSTANCE, PDMA_CH11);
    u32_spiRxFifoStatus = Cy_SCB_SPI_GetRxFifoStatus(CY_SPI_SCB_TYPE);
    if (((u32_pdmaInterruptStatusMasked & CY_PDMA_INTRCAUSE_COMPLETION) != 0u) && ((u32_spiRxFifoStatus & CY_SCB_SPI_RX_TRIGGER) != 0u))
    {
      /* clear device driver error log */
      s_deviceDriver.u8_deviceDriverErrorLog = TLE9563_DEVICEDRIVER_ERRORLOG_INIT;
      /* check CRC of received SPI message*/
    #if (TLE9563_CRC_EN == 1)
      if(s_deviceDriver.un_SpiRx.au8Bytes[3] == s_deviceDriver.un_CrcResult.au8Bytes[0])
    #elif (TLE9563_CRC_EN == 0)
      if(s_deviceDriver.un_SpiRx.au8Bytes[3] == TLE9563_CRC_STATIC_PATTERN_MISO)
    #endif
      {
        /* update SIF */
        TLE9563->SIF.byte = (uint8_t)(s_deviceDriver.un_SpiRx.u32Word & 0x000000FF);
        /* update status register, if provided */
        if(s_deviceDriver.fp_updateRamReg)
        {
          u16_tempRegData = (uint16_t)((s_deviceDriver.un_SpiRx.u32Word & 0x00FFFF00) >> 8);
          s_deviceDriver.fp_updateRamReg(u16_tempRegData);
          s_deviceDriver.fp_updateRamReg = NULL;
        }
      }
      else
      {
        s_deviceDriver.u8_deviceDriverErrorLog = TLE9563_DEVICEDRIVER_ERRORLOG_SPI_RECEIVE_ERR;
      }
    
      /* Complete SPI */
      Cy_SCB_SPI_ClearRxFifoStatus(CY_SPI_SCB_TYPE, CY_SCB_SPI_RX_TRIGGER);
      /* Complete PDMA */
      Cy_PDMA_Chnl_ClearInterrupt(PDMA_INSTANCE, PDMA_CH11);

      /* Start next SPI command */
      /* dynamic set register requests to write to control registers have highest priority */
      if(s_deviceDriver.fp_setReg)
      {
        s_deviceDriver.u8_deviceDriverStatus = TLE9563_DEVICEDRIVER_STATUS_CYCLIC_UPDATE;
        s_deviceDriver.fp_setReg(s_deviceDriver.u16_setBitValue);
        s_deviceDriver.fp_setReg = NULL;
        s_deviceDriver.u8_deviceDriverErrorLog = s_deviceDriver.u8_deviceDriverErrorLog | TLE9563_DEVICEDRIVER_ERRORLOG_STARTED;
        break;
      }
      /* cyclic get register requests to update status registers have lowest priority */
      else if(s_deviceDriver.fp_getReg)
      {
        s_deviceDriver.u8_deviceDriverStatus = TLE9563_DEVICEDRIVER_STATUS_CYCLIC_UPDATE;
        s_deviceDriver.fp_getReg = fp_cyclicStatGetFuncs[u8_cyclicStatUpdateCount];
        s_deviceDriver.fp_updateRamReg = fp_cyclicStatUpdateFuncs[u8_cyclicStatUpdateCount];
        s_deviceDriver.fp_getReg();
        if(u8_cyclicStatUpdateCount < TLE9563_DEVICEDRIVER_CYCLICSTATUPDATENUM)
        {
          u8_cyclicStatUpdateCount ++;
        }
        else
        {
          u8_cyclicStatUpdateCount = 0;
        }
        s_deviceDriver.u8_deviceDriverErrorLog = s_deviceDriver.u8_deviceDriverErrorLog | TLE9563_DEVICEDRIVER_ERRORLOG_STARTED;
        break;
      }
      else
      {
        s_deviceDriver.u8_deviceDriverStatus = TLE9563_DEVICEDRIVER_STATUS_INIT;
        break;
      }
    }
    else
    {
     s_deviceDriver.u8_deviceDriverErrorLog = TLE9563_DEVICEDRIVER_ERRORLOG_BUSY;
    }
    break;
    
  default:
    /* invalid device driver status */
    break;
  }
  
  return s_deviceDriver.u8_deviceDriverErrorLog;
}

/** \brief Initialize the Device Modules based on users configuration
 *  \note This function is blocking while Spi write commands are sent out to all device registers
 *  \note In case the BDRV needs to remain disabled after initialization, put HBx mode = 0x11 (ACTIVE_OFF) as users configuration. 
 *  \note Use \ref TLE9563_setAllHbx() at desired time to enable the BDRV.
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_Init(void)
{
  uint8_t u8_success = 0;
  fpDEVICE_initDeviceReg fp_initRegTemp;
  uint16_t u16_initRegValueTemp;
  uint8_t u8_initRegisterCounter;
  uint8_t u8_initRegisterNum;
  uint8_t u8_initStatUpdateCount = 0;
  
  fpDEVICE_initDeviceReg fp_initRegAll[] = {  &TLE9563_setModeSupplyCtrlReg,
                                              &TLE9563_setHwCtrlReg,
                                              &TLE9563_setBusCtrlReg,
                                              &TLE9563_setWk4CtrlReg,
                                              &TLE9563_setWk5CtrlReg,
                                              &TLE9563_setTimerCtrlReg,
                                              &TLE9563_setHsSwitchShutdownCtrlReg,
                                              &TLE9563_setIntrMaskCtrlReg,
                                              &TLE9563_setPwm1CtrlReg,
                                              &TLE9563_setPwm2CtrlReg,
                                              &TLE9563_setPwm3CtrlReg,
                                              &TLE9563_setPwm4CtrlReg,
                                              &TLE9563_setHsCtrlReg,
                                              &TLE9563_setGenBridgeCtrlReg,
                                              &TLE9563_setCsaCtrlReg,
                                              &TLE9563_setVdsLsCtrlReg,
                                              &TLE9563_setVdsHsCtrlReg,
                                              &TLE9563_setCcpBlankHb1ActCtrlReg,
                                              &TLE9563_setCcpBlankHb2ActCtrlReg,
                                              &TLE9563_setCcpBlankHb3ActCtrlReg,
                                              &TLE9563_setCcpBlankHb1FwCtrlReg,
                                              &TLE9563_setCcpBlankHb2FwCtrlReg,
                                              &TLE9563_setCcpBlankHb3FwCtrlReg,
                                              &TLE9563_setHbModeCtrlReg,
                                              &TLE9563_setTpreChgCtrlReg,
                                              &TLE9563_setTpreDischgCtrlReg,
                                              &TLE9563_setStIchgCtrlReg,
                                              &TLE9563_setIchgHb1ActCtrlReg,
                                              &TLE9563_setIchgHb2ActCtrlReg,
                                              &TLE9563_setIchgHb3ActCtrlReg,
                                              &TLE9563_setIchgHb1FwCtrlReg,
                                              &TLE9563_setIchgHb2FwCtrlReg,
                                              &TLE9563_setIchgHb3FwCtrlReg,
                                              &TLE9563_setIchgMaxCtrlReg,
                                              &TLE9563_setPreChgInitHb1CtrlReg,
                                              &TLE9563_setPreChgInitHb2CtrlReg,
                                              &TLE9563_setPreChgInitHb3CtrlReg,
                                              &TLE9563_setTdonHb1CtrlReg,
                                              &TLE9563_setTdonHb2CtrlReg,
                                              &TLE9563_setTdonHb3CtrlReg,
                                              &TLE9563_setTdoffHb1CtrlReg,
                                              &TLE9563_setTdoffHb2CtrlReg,
                                              &TLE9563_setTdoffHb3CtrlReg,
                                              &TLE9563_setBrakeCtrlReg,
                                              &TLE9563_setCanSwkCtrlReg,
                                              &TLE9563_setSwkBtl1CtrlReg,
                                              &TLE9563_setSwkId1CtrlReg,
                                              &TLE9563_setSwkId0CtrlReg,
                                              &TLE9563_setSwkMaskId1CtrlReg,
                                              &TLE9563_setSwkMaskId0CtrlReg,
                                              &TLE9563_setSwkData3CtrlReg,
                                              &TLE9563_setSwkData2CtrlReg,
                                              &TLE9563_setSwkData1CtrlReg,
                                              &TLE9563_setSwkData0CtrlReg,
                                              &TLE9563_setSwkCanFdCtrlReg,
                                              &TLE9563_setSwkOscTrimCtrlReg,
                                              &TLE9563_setSwkCdrCtrlReg,
                                              &TLE9563_setSwkCdrLimitCtrlReg,
                                              &TLE9563_setSwkDataLengthCodeReg,
                                              NULL,
                                           };

  uint16_t u16_initRegValueAll[] = { (uint16_t)TLE9563_M_S_CTRL,
                                     (uint16_t)TLE9563_HW_CTRL,
                                     (uint16_t)TLE9563_BUS_CTRL,
                                     (uint16_t)TLE9563_WK_CTRL_BNK3,
                                     (uint16_t)TLE9563_WK_CTRL_BNK4,
                                     (uint16_t)TLE9563_TIMER_CTRL,
                                     (uint16_t)TLE9563_SW_SD_CTRL,
                                     (uint16_t)TLE9563_INT_MASK,
                                     (uint16_t)TLE9563_PWM_CTRL_BNK0,
                                     (uint16_t)TLE9563_PWM_CTRL_BNK1,
                                     (uint16_t)TLE9563_PWM_CTRL_BNK2,
                                     (uint16_t)TLE9563_PWM_CTRL_BNK3,
                                     (uint16_t)TLE9563_HS_CTRL,
                                     (uint16_t)TLE9563_GENCTRL,
                                     (uint16_t)TLE9563_CSA,
                                     (uint16_t)TLE9563_LS_VDS,
                                     (uint16_t)TLE9563_HS_VDS,
                                     (uint16_t)TLE9563_CCP_BLK_BNK0,
                                     (uint16_t)TLE9563_CCP_BLK_BNK1,
                                     (uint16_t)TLE9563_CCP_BLK_BNK2,
                                     (uint16_t)TLE9563_CCP_BLK_BNK4,
                                     (uint16_t)TLE9563_CCP_BLK_BNK5,
                                     (uint16_t)TLE9563_CCP_BLK_BNK6,
                                     (uint16_t)TLE9563_HBMODE,
                                     (uint16_t)TLE9563_TPRECHG_BNK0,
                                     (uint16_t)TLE9563_TPRECHG_BNK1,
                                     (uint16_t)TLE9563_ST_ICHG,
                                     (uint16_t)TLE9563_HB_ICHG_BNK0,
                                     (uint16_t)TLE9563_HB_ICHG_BNK1,
                                     (uint16_t)TLE9563_HB_ICHG_BNK2,
                                     (uint16_t)TLE9563_HB_ICHG_BNK4,
                                     (uint16_t)TLE9563_HB_ICHG_BNK5,
                                     (uint16_t)TLE9563_HB_ICHG_BNK6,
                                     (uint16_t)TLE9563_HB_ICHG_MAX,
                                     (uint16_t)TLE9563_HB_PCHG_INIT_BNK0,
                                     (uint16_t)TLE9563_HB_PCHG_INIT_BNK1,
                                     (uint16_t)TLE9563_HB_PCHG_INIT_BNK2,
                                     (uint16_t)TLE9563_TDON_HB_CTRL_BNK0,
                                     (uint16_t)TLE9563_TDON_HB_CTRL_BNK1,
                                     (uint16_t)TLE9563_TDON_HB_CTRL_BNK2,
                                     (uint16_t)TLE9563_TDOFF_HB_CTRL_BNK0,
                                     (uint16_t)TLE9563_TDOFF_HB_CTRL_BNK1,
                                     (uint16_t)TLE9563_TDOFF_HB_CTRL_BNK2,
                                     (uint16_t)TLE9563_BRAKE,
                                     (uint16_t)TLE9563_SWK_CTRL,
                                     (uint16_t)TLE9563_SWK_BTL1_CTRL,
                                     (uint16_t)TLE9563_SWK_ID0_CTRL, 
                                     (uint16_t)TLE9563_SWK_ID1_CTRL,
                                     (uint16_t)TLE9563_SWK_MASK_ID1_CTRL,
                                     (uint16_t)TLE9563_SWK_MASK_ID0_CTRL,
                                     (uint16_t)TLE9563_SWK_DATA3_CTRL,
                                     (uint16_t)TLE9563_SWK_DATA2_CTRL, 
                                     (uint16_t)TLE9563_SWK_DATA1_CTRL, 
                                     (uint16_t)TLE9563_SWK_DATA0_CTRL,
                                     (uint16_t)TLE9563_SWK_CAN_FD_CTRL, 
                                     (uint16_t)TLE9563_SWK_OSC_TRIM_CTRL,
                                     (uint16_t)TLE9563_SWK_CDR_CTRL, 
                                     (uint16_t)TLE9563_SWK_CDR_LIMIT, 
                                     (uint16_t)TLE9563_SWK_DLC_CTRL,
                                     (uint16_t)0x0000,
                                  };

  TLE9563_initSpi();
  TLE9563_initDma();
  
  /* Clear POR */
  TLE9563_clrSupSts();
  TLE9563_deviceDriverCyclicTask();
  while(TLE9563_deviceDriverCyclicTask() == TLE9563_DEVICEDRIVER_ERRORLOG_BUSY);
  
#if (TLE9563_CRC_REC_INIT == 1)
  /* set CRC according to users settings */
  TLE9563_recoverCrc();
  TLE9563_deviceDriverCyclicTask();
  while(TLE9563_deviceDriverCyclicTask() == TLE9563_DEVICEDRIVER_ERRORLOG_BUSY);
  /* clear DEV_STAT in case there was a CRC mismatch before */
  TLE9563_clrDevSts();
  TLE9563_deviceDriverCyclicTask();
  while(TLE9563_deviceDriverCyclicTask() == TLE9563_DEVICEDRIVER_ERRORLOG_BUSY);
  /* Clear POR in case there was a CRC mismatch before and first POR clear cmd was ignored */
  TLE9563_clrSupSts();
  TLE9563_deviceDriverCyclicTask();
  while(TLE9563_deviceDriverCyclicTask() == TLE9563_DEVICEDRIVER_ERRORLOG_BUSY);
#endif
  
  /* Initialize all control registers */
  u8_initRegisterNum = (uint8_t)(sizeof(u16_initRegValueAll)/sizeof(u16_initRegValueAll[0])) - 1;
  for(u8_initRegisterCounter = 0; u8_initRegisterCounter < u8_initRegisterNum; u8_initRegisterCounter++)
  {
    fp_initRegTemp = fp_initRegAll[u8_initRegisterCounter];
    u16_initRegValueTemp = u16_initRegValueAll[u8_initRegisterCounter];
    fp_initRegTemp(u16_initRegValueTemp);
    TLE9563_deviceDriverCyclicTask();
    while(TLE9563_deviceDriverCyclicTask() == TLE9563_DEVICEDRIVER_ERRORLOG_BUSY);
    if((s_deviceDriver.u8_deviceDriverErrorLog == TLE9563_DEVICEDRIVER_ERRORLOG_SPI_RECEIVE_ERR) || (TLE9563->SIF.bit.SPI_CRC_FAIL == 1))
    {
      u8_success = 0;
      return u8_success;
    }
  }
  /* Initialize first get-register functionpointer */
  s_deviceDriver.fp_getReg = &TLE9563_getSUPSTAT;
  s_deviceDriver.fp_updateRamReg = &TLE9563_updateRamSUPSTAT;
  /* Read out all status registers */
  for(u8_initStatUpdateCount = 0; u8_initStatUpdateCount < TLE9563_DEVICEDRIVER_CYCLICSTATUPDATENUM; u8_initStatUpdateCount++)
  {
    TLE9563_deviceDriverCyclicTask();
    while(TLE9563_deviceDriverCyclicTask() == TLE9563_DEVICEDRIVER_ERRORLOG_BUSY);
    if((s_deviceDriver.u8_deviceDriverErrorLog == TLE9563_DEVICEDRIVER_ERRORLOG_SPI_RECEIVE_ERR) || (TLE9563->SIF.bit.SPI_CRC_FAIL == 1))
    {
      u8_success = 0;
      return u8_success;
    }
  } 
  /* Init Watchdog to close long open window after start up */
  TLE9563_setWdCtrlReg((uint16_t)TLE9563_WD_CTRL);
  TLE9563_deviceDriverCyclicTask();
  while(TLE9563_deviceDriverCyclicTask() == TLE9563_DEVICEDRIVER_ERRORLOG_BUSY);
  /* Final check for SPI error */
  if((s_deviceDriver.u8_deviceDriverErrorLog == TLE9563_DEVICEDRIVER_ERRORLOG_STARTED) && (TLE9563->SIF.bit.SPI_CRC_FAIL == 0))
  {
    u8_success = 1;
  }
  
  return u8_success;
}

/** \brief Get the Status Information Field of last SPI message received from the device
 * 
 *  \return uint8_t SIF byte
 */
uint8_t TLE9563_getSif(void)
{ 
  return TLE9563->SIF.byte;
}

/** \brief Get the Supply Status bit of the Status Information Field of last SPI message received from the device
 * 
 *  \return uint8_t Supply Status bit of SIF
 */
uint8_t TLE9563_getSifBitSupplyStatus(void)
{
  return TLE9563->SIF.bit.SUPPLY_STAT;
}

/** \brief Get the Temperature Status bit of the Status Information Field of last SPI message received from the device
 * 
 *  \return uint8_t Temperature Status bit of SIF
 */
uint8_t TLE9563_getSifBitTempStatus(void)
{
  return TLE9563->SIF.bit.TEMP_STAT;
}

/** \brief Get the Bus Status bit of the Status Information Field of last SPI message received from the device
 * 
 *  \return uint8_t Bus Status bit of SIF
 */
uint8_t TLE9563_getSifBitBusStatus(void)
{
  return TLE9563->SIF.bit.BUS_STAT;
}

/** \brief Get the Wake Up bit of the Status Information Field of last SPI message received from the device
 * 
 *  \return uint8_t Wake Up bit of SIF
 */
uint8_t TLE9563_getSifBitWakeUp(void)
{
  return TLE9563->SIF.bit.WAKE_UP;
}

/** \brief Get the High Side Status bit of the Status Information Field of last SPI message received from the device
 * 
 *  \return uint8_t High Side Status bit of SIF
 */
uint8_t TLE9563_getSifBitHsStatus(void)
{
  return TLE9563->SIF.bit.HS_STAT;
}

/** \brief Get the Device Status bit of the Status Information Field of last SPI message received from the device
 * 
 *  \return uint8_t Device Status bit of SIF
 */
uint8_t TLE9563_getSifBitDeviceStatus(void)
{
  return TLE9563->SIF.bit.DEV_STAT;
}

/** \brief Get the Bridge Driver Status bit of the Status Information Field of last SPI message received from the device
 * 
 *  \return uint8_t Bridge Driver Status bit of SIF
 */
uint8_t TLE9563_getSifBitBdrvStatus(void)
{
  return TLE9563->SIF.bit.BD_STAT;
}

/** \brief Get the SPI or CRC Fail bit of the Status Information Field of last SPI message received from the device
 * 
 *  \return uint8_t SPI or CRC Fail bit of SIF
 */
uint8_t TLE9563_getSifBitSpiCrcFail(void)
{
  return TLE9563->SIF.bit.SPI_CRC_FAIL;
}

/** \brief Serve Watchdog
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_serveWatchdog(void)
{
  uint8_t u8_success = 0;
  /* if no set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = TLE9563->WD_CTRL.reg;
    s_deviceDriver.fp_setReg = &TLE9563_setWDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Recovery SPI command in case of mismatch of CRC setting between the device and µC
 *  \brief Can only be used, if device is in Normal Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_recoverCrc(void)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.fp_setReg = &TLE9563_resetCrc;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Reset the last call of a set bitfield function
 * 
 *  \return void no return value
 */
void TLE9563_resetLastSetBitfieldRequest(void)
{
  s_deviceDriver.u16_setBitValue = (uint16_t)0;
  s_deviceDriver.fp_setReg = NULL;
}

/** \brief Set all HBx with one SPI command
 * 
 *  \param e_value HB Mode Register content \ref tBDRV_setAllHbx
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setAllHbx(tBDRV_setAllHbx e_value)
{
  uint8_t u8_success = 0;
  /* if no set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = (uint16_t)e_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Mode and Supply Control Register
 * 
 *  \return uint16_t Mode and Supply Control Register
 */
uint16_t TLE9563_getModeSupplyCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->M_S_CTRL.reg;
}

/** \brief Set Mode and Supply Control Register
 * 
 *  \param u16_value Mode and Supply Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setModeSupplyCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setMSCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Device Mode Control
 *  \brief Use this function for device mode transitions.
 *  \brief In order to go to Normal Mode, use: TLE9563_setDeviceMode(DEVICE_mode_normal)
 *  \brief In order to go to Stop Mode, use: TLE9563_setDeviceMode(DEVICE_mode_stop)
 *  \brief In order to go to Sleep Mode, use: TLE9563_setDeviceMode(DEVICE_mode_sleep)
 *  \brief In order to perform a software reset (go to Init Mode), use: TLE9563_setDeviceMode(DEVICE_mode_reset)
 *  \brief In case of a software reset, the device driver also resets the control register RAM copy \ref TLE9563_setMSCTRL
 *  
 *  \warning VCC1 of the TLE9563 is turned off in Sleep mode. If VCC1 is used to supply the uC, going to Sleep Mode will turn off the uC.
 *  \warning Going to Sleep Mode can only be done, if a wake up source of the TLE9563 is enabled.
 *  \warning The mode transition Stop -> Sleep is not possible.
 *  \warning The mode transitions Sleep -> Normal or return from Fail Safe are not possible via SPI.
 *  \warning Some control registers are modified by the device after certain mode transistions and under certain conditions, e.g. HW_CTRL, BUS_CTRL, WK_CTRL, WD_CTRL.
 *  \warning Please refer to device user manual and re-initialize according registers, if needed.
 * 
 *  \param e_value Device Mode Control
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setDeviceMode(tDEVICE_mode e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->M_S_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_M_S_CTRL_MODE_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_M_S_CTRL_MODE_Pos) & TLE9563_M_S_CTRL_MODE_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setMSCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Device Mode Control
 * 
 *  \return uint16_t Device Mode Control
 */
uint16_t TLE9563_getDeviceMode(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->M_S_CTRL.bit.MODE;
}

/** \brief Set Reaction in case of VCC1 Over Voltage
 * 
 *  \param e_value Reaction in case of VCC1 Over Voltage
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVcc1OverVoltReact(tVCC1_overVoltReact e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->M_S_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_M_S_CTRL_VCC1_OV_MOD_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_M_S_CTRL_VCC1_OV_MOD_Pos) & TLE9563_M_S_CTRL_VCC1_OV_MOD_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setMSCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Reaction in case of VCC1 Over Voltage
 * 
 *  \return uint16_t Reaction in case of VCC1 Over Voltage
 */
uint16_t TLE9563_getVcc1OverVoltReact(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->M_S_CTRL.bit.VCC1_OV_MOD;
}

/** \brief Set VCC1 Undervoltage Reset Hysteresis Selection (see also Chapter 12.7.1 for more information)
 * 
 *  \param e_value VCC1 Undervoltage Reset Hysteresis Selection (see also Chapter 12.7.1 for more information)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVcc1UnderVoltResetHys(tVCC1_underVoltResetHys e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->M_S_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_M_S_CTRL_RSTN_HYS_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_M_S_CTRL_RSTN_HYS_Pos) & TLE9563_M_S_CTRL_RSTN_HYS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setMSCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get VCC1 Undervoltage Reset Hysteresis Selection (see also Chapter 12.7.1 for more information)
 * 
 *  \return uint16_t VCC1 Undervoltage Reset Hysteresis Selection (see also Chapter 12.7.1 for more information)
 */
uint16_t TLE9563_getVcc1UnderVoltResetHys(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->M_S_CTRL.bit.RSTN_HYS;
}

/** \brief Set VCC1 Active Peak Threshold Selection
 * 
 *  \param e_value VCC1 Active Peak Threshold Selection
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVcc1ActPeakThresh(tVCC1_actPeakThresh e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->M_S_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_M_S_CTRL_I_PEAK_TH_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_M_S_CTRL_I_PEAK_TH_Pos) & TLE9563_M_S_CTRL_I_PEAK_TH_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setMSCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get VCC1 Active Peak Threshold Selection
 * 
 *  \return uint16_t VCC1 Active Peak Threshold Selection
 */
uint16_t TLE9563_getVcc1ActPeakThresh(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->M_S_CTRL.bit.I_PEAK_TH;
}

/** \brief Set VCC1 Reset Threshold Control
 * 
 *  \param e_value VCC1 Reset Threshold Control
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVcc1ResetThresh(tVCC1_resetThresh e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->M_S_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_M_S_CTRL_VCC1_RT_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_M_S_CTRL_VCC1_RT_Pos) & TLE9563_M_S_CTRL_VCC1_RT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setMSCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get VCC1 Reset Threshold Control
 * 
 *  \return uint16_t VCC1 Reset Threshold Control
 */
uint16_t TLE9563_getVcc1ResetThresh(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->M_S_CTRL.bit.VCC1_RT;
}

/** \brief Get Hardware Control Register
 * 
 *  \return uint16_t Hardware Control Register
 */
uint16_t TLE9563_getHwCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HW_CTRL.reg;
}

/** \brief Set Hardware Control Register
 * 
 *  \param u16_value Hardware Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setHwCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHWCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set TSD2 minimum Waiting Time Selection
 * 
 *  \param e_value TSD2 minimum Waiting Time Selection
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTempShutDown2MinWaitTime(tTSD2_minWaitTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HW_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HW_CTRL_TSD2_DEL_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HW_CTRL_TSD2_DEL_Pos) & TLE9563_HW_CTRL_TSD2_DEL_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHWCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get TSD2 minimum Waiting Time Selection
 * 
 *  \return uint16_t TSD2 minimum Waiting Time Selection
 */
uint16_t TLE9563_getTempShutDown2MinWaitTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HW_CTRL.bit.TSD2_DEL;
}

/** \brief Set VS OV comparator threshold change
 * 
 *  \param e_value VS OV comparator threshold change
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVsOverVoltThresh(tVS_overVoltThresh e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HW_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HW_CTRL_VS_OV_SEL_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HW_CTRL_VS_OV_SEL_Pos) & TLE9563_HW_CTRL_VS_OV_SEL_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHWCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get VS OV comparator threshold change
 * 
 *  \return uint16_t VS OV comparator threshold change
 */
uint16_t TLE9563_getVsOverVoltThresh(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HW_CTRL.bit.VS_OV_SEL;
}

/** \brief Enable Sample and hold circuitry disable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvSampleAndHoldCiruit(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HW_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HW_CTRL_SH_DISABLE_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_HW_CTRL_SH_DISABLE_Pos) & TLE9563_HW_CTRL_SH_DISABLE_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHWCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Sample and hold circuitry disable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvSampleAndHoldCiruit(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HW_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HW_CTRL_SH_DISABLE_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_HW_CTRL_SH_DISABLE_Pos) & TLE9563_HW_CTRL_SH_DISABLE_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHWCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Sample and hold circuitry disable Enable/Disable Setting
 * 
 *  \return  Sample and hold circuitry disable Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getBdrvSampleAndHoldCiruit(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HW_CTRL.bit.SH_DISABLE;
}

/** \brief Set Reset delay time
 * 
 *  \param e_value Reset delay time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setResetDelayTime(tRST_delayTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HW_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HW_CTRL_RSTN_DEL_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HW_CTRL_RSTN_DEL_Pos) & TLE9563_HW_CTRL_RSTN_DEL_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHWCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Reset delay time
 * 
 *  \return uint16_t Reset delay time
 */
uint16_t TLE9563_getResetDelayTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HW_CTRL.bit.RSTN_DEL;
}

/** \brief Set Soft Reset Configuration
 * 
 *  \param e_value Soft Reset Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSoftResetConfig(tSOFTRST_config e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HW_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HW_CTRL_SOFT_RESET_RO_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HW_CTRL_SOFT_RESET_RO_Pos) & TLE9563_HW_CTRL_SOFT_RESET_RO_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHWCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Soft Reset Configuration
 * 
 *  \return uint16_t Soft Reset Configuration
 */
uint16_t TLE9563_getSoftResetConfig(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HW_CTRL.bit.SOFT_RESET_RO;
}

/** \brief Enable Watchdog Deactivation during Stop Mode, bit1
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enWdDeactDuringStopModeBit1(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HW_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HW_CTRL_WD_STM_EN_1_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_HW_CTRL_WD_STM_EN_1_Pos) & TLE9563_HW_CTRL_WD_STM_EN_1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHWCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Watchdog Deactivation during Stop Mode, bit1
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disWdDeactDuringStopModeBit1(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HW_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HW_CTRL_WD_STM_EN_1_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_HW_CTRL_WD_STM_EN_1_Pos) & TLE9563_HW_CTRL_WD_STM_EN_1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHWCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Watchdog Deactivation during Stop Mode, bit1 Enable/Disable Setting
 * 
 *  \return  Watchdog Deactivation during Stop Mode, bit1 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getWdDeactDuringStopModeBit1(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HW_CTRL.bit.WD_STM_EN_1;
}

/** \brief Get Watchdog Control Register
 * 
 *  \return uint16_t Watchdog Control Register
 */
uint16_t TLE9563_getWdCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->WD_CTRL.reg;
}

/** \brief Set Watchdog Control Register
 * 
 *  \param u16_value Watchdog Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setWdCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_paritybit = 0;
  uint16_t u16_wdCtrl_temp;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* calculate checksum bit (even parity) */
    u16_wdCtrl_temp = u16_value;
    while(u16_wdCtrl_temp > (uint16_t)0)
    {
      /* Check if LSB is set*/
      if((u16_wdCtrl_temp & (uint16_t)0x0001) == (uint16_t)0x0001)
      {
        /* Toggle parity bit */
        u16_paritybit = u16_paritybit ^ (uint16_t)0x8000;
      }
      u16_wdCtrl_temp = u16_wdCtrl_temp >> 1;
    }
    /* modify checksum bit */
    if(u16_paritybit)
    {
      u16_value = (uint16_t)((u16_value & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    else
    {
      u16_value = (uint16_t)((u16_value & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setWDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Watchdog Setting Check Sum Bit
 * 
 *  \return uint16_t Watchdog Setting Check Sum Bit
 */
uint16_t TLE9563_getWdChecksumBit(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->WD_CTRL.bit.CHECKSUM;
}

/** \brief Enable Watchdog Deactivation during Stop Mode, bit0
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enWdDeactDuringStopModeBit0(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  uint16_t u16_paritybit = 0;
  uint16_t u16_wdCtrl_temp;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_WD_STM_EN_0_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_WD_CTRL_WD_STM_EN_0_Pos) & TLE9563_WD_CTRL_WD_STM_EN_0_Msk));
    /* calculate checksum bit (even parity) */
    u16_wdCtrl_temp = u16_reg_temp;
    while(u16_wdCtrl_temp > (uint16_t)0)
    {
      /* Check if LSB is set*/
      if((u16_wdCtrl_temp & (uint16_t)0x0001) == (uint16_t)0x0001)
      {
        /* Toggle parity bit */
        u16_paritybit = u16_paritybit ^ (uint16_t)0x8000;
      }
      u16_wdCtrl_temp = u16_wdCtrl_temp >> 1;
    }
    /* modify checksum bit */
    if(u16_paritybit)
    {
      u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    else
    {
      u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Watchdog Deactivation during Stop Mode, bit0
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disWdDeactDuringStopModeBit0(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  uint16_t u16_paritybit = 0;
  uint16_t u16_wdCtrl_temp;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_WD_STM_EN_0_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_WD_CTRL_WD_STM_EN_0_Pos) & TLE9563_WD_CTRL_WD_STM_EN_0_Msk));
    /* calculate checksum bit (even parity) */
    u16_wdCtrl_temp = u16_reg_temp;
    while(u16_wdCtrl_temp > (uint16_t)0)
    {
      /* Check if LSB is set*/
      if((u16_wdCtrl_temp & (uint16_t)0x0001) == (uint16_t)0x0001)
      {
        /* Toggle parity bit */
        u16_paritybit = u16_paritybit ^ (uint16_t)0x8000;
      }
      u16_wdCtrl_temp = u16_wdCtrl_temp >> 1;
    }
    /* modify checksum bit */
    if(u16_paritybit)
    {
      u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    else
    {
      u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Watchdog Deactivation during Stop Mode, bit0 Enable/Disable Setting
 * 
 *  \return  Watchdog Deactivation during Stop Mode, bit0 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getWdDeactDuringStopModeBit0(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->WD_CTRL.bit.WD_STM_EN_0;
}

/** \brief Set Watchdog Configuration
 * 
 *  \param e_value Watchdog Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setWdConfig(tWD_config e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  uint16_t u16_paritybit = 0;
  uint16_t u16_wdCtrl_temp;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_WD_CFG_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_WD_CTRL_WD_CFG_Pos) & TLE9563_WD_CTRL_WD_CFG_Msk));
    /* calculate checksum bit (even parity) */
    u16_wdCtrl_temp = u16_reg_temp;
    while(u16_wdCtrl_temp > (uint16_t)0)
    {
      /* Check if LSB is set*/
      if((u16_wdCtrl_temp & (uint16_t)0x0001) == (uint16_t)0x0001)
      {
        /* Toggle parity bit */
        u16_paritybit = u16_paritybit ^ (uint16_t)0x8000;
      }
      u16_wdCtrl_temp = u16_wdCtrl_temp >> 1;
    }
    /* modify checksum bit */
    if(u16_paritybit)
    {
      u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    else
    {
      u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Watchdog Configuration
 * 
 *  \return uint16_t Watchdog Configuration
 */
uint16_t TLE9563_getWdConfig(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->WD_CTRL.bit.WD_CFG;
}

/** \brief Enable Watchdog Enable after Bus Wake in Stop Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enWdAfterBusWkInStopMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  uint16_t u16_paritybit = 0;
  uint16_t u16_wdCtrl_temp;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_WD_EN_WK_BUS_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_WD_CTRL_WD_EN_WK_BUS_Pos) & TLE9563_WD_CTRL_WD_EN_WK_BUS_Msk));
    /* calculate checksum bit (even parity) */
    u16_wdCtrl_temp = u16_reg_temp;
    while(u16_wdCtrl_temp > (uint16_t)0)
    {
      /* Check if LSB is set*/
      if((u16_wdCtrl_temp & (uint16_t)0x0001) == (uint16_t)0x0001)
      {
        /* Toggle parity bit */
        u16_paritybit = u16_paritybit ^ (uint16_t)0x8000;
      }
      u16_wdCtrl_temp = u16_wdCtrl_temp >> 1;
    }
    /* modify checksum bit */
    if(u16_paritybit)
    {
      u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    else
    {
      u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Watchdog Enable after Bus Wake in Stop Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disWdAfterBusWkInStopMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  uint16_t u16_paritybit = 0;
  uint16_t u16_wdCtrl_temp;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_WD_EN_WK_BUS_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_WD_CTRL_WD_EN_WK_BUS_Pos) & TLE9563_WD_CTRL_WD_EN_WK_BUS_Msk));
    /* calculate checksum bit (even parity) */
    u16_wdCtrl_temp = u16_reg_temp;
    while(u16_wdCtrl_temp > (uint16_t)0)
    {
      /* Check if LSB is set*/
      if((u16_wdCtrl_temp & (uint16_t)0x0001) == (uint16_t)0x0001)
      {
        /* Toggle parity bit */
        u16_paritybit = u16_paritybit ^ (uint16_t)0x8000;
      }
      u16_wdCtrl_temp = u16_wdCtrl_temp >> 1;
    }
    /* modify checksum bit */
    if(u16_paritybit)
    {
      u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    else
    {
      u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Watchdog Enable after Bus Wake in Stop Mode Enable/Disable Setting
 * 
 *  \return  Watchdog Enable after Bus Wake in Stop Mode Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getWdAfterBusWkInStopMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->WD_CTRL.bit.WD_EN_WK_BUS;
}

/** \brief Set Watchdog Timer Period
 * 
 *  \param e_value Watchdog Timer Period
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setWdTimerPeriod(tWD_timerPeriod e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  uint16_t u16_paritybit = 0;
  uint16_t u16_wdCtrl_temp;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_WD_TIMER_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_WD_CTRL_WD_TIMER_Pos) & TLE9563_WD_CTRL_WD_TIMER_Msk));
    /* calculate checksum bit (even parity) */
    u16_wdCtrl_temp = u16_reg_temp;
    while(u16_wdCtrl_temp > (uint16_t)0)
    {
      /* Check if LSB is set*/
      if((u16_wdCtrl_temp & (uint16_t)0x0001) == (uint16_t)0x0001)
      {
        /* Toggle parity bit */
        u16_paritybit = u16_paritybit ^ (uint16_t)0x8000;
      }
      u16_wdCtrl_temp = u16_wdCtrl_temp >> 1;
    }
    /* modify checksum bit */
    if(u16_paritybit)
    {
      u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    else
    {
      u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WD_CTRL_CHECKSUM_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_WD_CTRL_CHECKSUM_Pos) & TLE9563_WD_CTRL_CHECKSUM_Msk));
    }
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Watchdog Timer Period
 * 
 *  \return uint16_t Watchdog Timer Period
 */
uint16_t TLE9563_getWdTimerPeriod(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->WD_CTRL.bit.WD_TIMER;
}

/** \brief Get CAN Control Register
 * 
 *  \return uint16_t CAN Control Register
 */
uint16_t TLE9563_getBusCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->BUS_CTRL.reg;
}

/** \brief Set CAN Control Register
 * 
 *  \param u16_value CAN Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBusCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setBUSCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set HS-CAN Module Modes
 * 
 *  \param e_value HS-CAN Module Modes
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCanMode(tCAN_mode e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BUS_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BUS_CTRL_CAN_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_BUS_CTRL_CAN_Pos) & TLE9563_BUS_CTRL_CAN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBUSCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get HS-CAN Module Modes
 * 
 *  \return uint16_t HS-CAN Module Modes
 */
uint16_t TLE9563_getCanMode(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->BUS_CTRL.bit.CAN;
}

/** \brief Get Wake-up Control Register
 * 
 *  \return uint16_t Wake-up Control Register
 */
uint16_t TLE9563_getWk4CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->WK_CTRL_BNK3.reg;
}

/** \brief Set Wake-up Control Register
 * 
 *  \param u16_value Wake-up Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setWk4CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setWKCTRLBNK3;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Wake-up Filter Time Configuration
 * 
 *  \param e_value Wake-up Filter Time Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setWk4FiltTime(tWK_filtTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WK_CTRL_BNK3.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WK_CTRL_BNK3_WK_FILT_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_WK_CTRL_BNK3_WK_FILT_Pos) & TLE9563_WK_CTRL_BNK3_WK_FILT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWKCTRLBNK3;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Wake-up Filter Time Configuration
 * 
 *  \return uint16_t Wake-up Filter Time Configuration
 */
uint16_t TLE9563_getWk4FiltTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->WK_CTRL_BNK3.bit.WK_FILT;
}

/** \brief Set WKx Pull-Up/Pull-Down Configuration
 * 
 *  \param e_value WKx Pull-Up/Pull-Down Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setWk4PullUpPullDown(tWK_pullUpPullDown e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WK_CTRL_BNK3.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WK_CTRL_BNK3_WK_PUPD_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_WK_CTRL_BNK3_WK_PUPD_Pos) & TLE9563_WK_CTRL_BNK3_WK_PUPD_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWKCTRLBNK3;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WKx Pull-Up/Pull-Down Configuration
 * 
 *  \return uint16_t WKx Pull-Up/Pull-Down Configuration
 */
uint16_t TLE9563_getWk4PullUpPullDown(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->WK_CTRL_BNK3.bit.WK_PUPD;
}

/** \brief Enable WKx Enable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enWk4(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WK_CTRL_BNK3.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WK_CTRL_BNK3_WK_EN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_WK_CTRL_BNK3_WK_EN_Pos) & TLE9563_WK_CTRL_BNK3_WK_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWKCTRLBNK3;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WKx Enable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disWk4(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WK_CTRL_BNK3.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WK_CTRL_BNK3_WK_EN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_WK_CTRL_BNK3_WK_EN_Pos) & TLE9563_WK_CTRL_BNK3_WK_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWKCTRLBNK3;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WKx Enable Enable/Disable Setting
 * 
 *  \return  WKx Enable Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getWk4(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->WK_CTRL_BNK3.bit.WK_EN;
}

/** \brief Get Wake-up Control Register
 * 
 *  \return uint16_t Wake-up Control Register
 */
uint16_t TLE9563_getWk5CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->WK_CTRL_BNK4.reg;
}

/** \brief Set Wake-up Control Register
 * 
 *  \param u16_value Wake-up Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setWk5CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setWKCTRLBNK4;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Wake-up Filter Time Configuration
 * 
 *  \param e_value Wake-up Filter Time Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setWk5FiltTime(tWK_filtTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WK_CTRL_BNK4.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WK_CTRL_BNK4_WK_FILT_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_WK_CTRL_BNK4_WK_FILT_Pos) & TLE9563_WK_CTRL_BNK4_WK_FILT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWKCTRLBNK4;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Wake-up Filter Time Configuration
 * 
 *  \return uint16_t Wake-up Filter Time Configuration
 */
uint16_t TLE9563_getWk5FiltTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->WK_CTRL_BNK4.bit.WK_FILT;
}

/** \brief Set WKx Pull-Up/Pull-Down Configuration
 * 
 *  \param e_value WKx Pull-Up/Pull-Down Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setWk5PullUpPullDown(tWK_pullUpPullDown e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WK_CTRL_BNK4.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WK_CTRL_BNK4_WK_PUPD_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_WK_CTRL_BNK4_WK_PUPD_Pos) & TLE9563_WK_CTRL_BNK4_WK_PUPD_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWKCTRLBNK4;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WKx Pull-Up/Pull-Down Configuration
 * 
 *  \return uint16_t WKx Pull-Up/Pull-Down Configuration
 */
uint16_t TLE9563_getWk5PullUpPullDown(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->WK_CTRL_BNK4.bit.WK_PUPD;
}

/** \brief Enable WKx Enable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enWk5(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WK_CTRL_BNK4.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WK_CTRL_BNK4_WK_EN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_WK_CTRL_BNK4_WK_EN_Pos) & TLE9563_WK_CTRL_BNK4_WK_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWKCTRLBNK4;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WKx Enable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disWk5(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->WK_CTRL_BNK4.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_WK_CTRL_BNK4_WK_EN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_WK_CTRL_BNK4_WK_EN_Pos) & TLE9563_WK_CTRL_BNK4_WK_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setWKCTRLBNK4;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WKx Enable Enable/Disable Setting
 * 
 *  \return  WKx Enable Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getWk5(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->WK_CTRL_BNK4.bit.WK_EN;
}

/** \brief Get Timer 1 and Timer2 Control and Selection Register
 * 
 *  \return uint16_t Timer 1 and Timer2 Control and Selection Register
 */
uint16_t TLE9563_getTimerCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TIMER_CTRL.reg;
}

/** \brief Set Timer 1 and Timer2 Control and Selection Register
 * 
 *  \param u16_value Timer 1 and Timer2 Control and Selection Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTimerCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setTIMERCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Timer2 On-Time Configuration
 * 
 *  \param e_value Timer2 On-Time Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTimer2OnTime(tTIMER_onTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TIMER_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TIMER_CTRL_TIMER2_ON_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TIMER_CTRL_TIMER2_ON_Pos) & TLE9563_TIMER_CTRL_TIMER2_ON_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTIMERCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Timer2 On-Time Configuration
 * 
 *  \return uint16_t Timer2 On-Time Configuration
 */
uint16_t TLE9563_getTimer2OnTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TIMER_CTRL.bit.TIMER2_ON;
}

/** \brief Set Timer2 Period Configuration
 * 
 *  \param e_value Timer2 Period Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTimer2Period(tTIMER_period e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TIMER_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TIMER_CTRL_TIMER2_PER_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TIMER_CTRL_TIMER2_PER_Pos) & TLE9563_TIMER_CTRL_TIMER2_PER_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTIMERCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Timer2 Period Configuration
 * 
 *  \return uint16_t Timer2 Period Configuration
 */
uint16_t TLE9563_getTimer2Period(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TIMER_CTRL.bit.TIMER2_PER;
}

/** \brief Set Cyclic Wake Configuration
 * 
 *  \param e_value Cyclic Wake Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTimerCyclicWk(tTIMER_cyclicWk e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TIMER_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TIMER_CTRL_CYCWK_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TIMER_CTRL_CYCWK_Pos) & TLE9563_TIMER_CTRL_CYCWK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTIMERCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Cyclic Wake Configuration
 * 
 *  \return uint16_t Cyclic Wake Configuration
 */
uint16_t TLE9563_getTimerCyclicWk(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TIMER_CTRL.bit.CYCWK;
}

/** \brief Set Timer1 On-Time Configuration
 * 
 *  \param e_value Timer1 On-Time Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTimer1OnTime(tTIMER_onTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TIMER_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TIMER_CTRL_TIMER1_ON_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TIMER_CTRL_TIMER1_ON_Pos) & TLE9563_TIMER_CTRL_TIMER1_ON_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTIMERCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Timer1 On-Time Configuration
 * 
 *  \return uint16_t Timer1 On-Time Configuration
 */
uint16_t TLE9563_getTimer1OnTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TIMER_CTRL.bit.TIMER1_ON;
}

/** \brief Set Timer1 Period Configuration
 * 
 *  \param e_value Timer1 Period Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTimer1Period(tTIMER_period e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TIMER_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TIMER_CTRL_TIMER1_PER_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TIMER_CTRL_TIMER1_PER_Pos) & TLE9563_TIMER_CTRL_TIMER1_PER_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTIMERCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Timer1 Period Configuration
 * 
 *  \return uint16_t Timer1 Period Configuration
 */
uint16_t TLE9563_getTimer1Period(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TIMER_CTRL.bit.TIMER1_PER;
}

/** \brief Get High-Side Switch Shutdown Control Register
 * 
 *  \return uint16_t High-Side Switch Shutdown Control Register
 */
uint16_t TLE9563_getHsSwitchShutdownCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SW_SD_CTRL.reg;
}

/** \brief Set High-Side Switch Shutdown Control Register
 * 
 *  \param u16_value High-Side Switch Shutdown Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setHsSwitchShutdownCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Switch recovery after removal of VS Overvoltage for HS3
 * 
 *  \param e_value Switch recovery after removal of VS Overvoltage for HS3
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setHs3VsOverVoltRecovery(tHS_vsOverVoltRecovery e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS3_OV_REC_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_SW_SD_CTRL_HS3_OV_REC_Pos) & TLE9563_SW_SD_CTRL_HS3_OV_REC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Switch recovery after removal of VS Overvoltage for HS3
 * 
 *  \return uint16_t Switch recovery after removal of VS Overvoltage for HS3
 */
uint16_t TLE9563_getHs3VsOverVoltRecovery(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SW_SD_CTRL.bit.HS3_OV_REC;
}

/** \brief Set Switch recovery after removal of VS Overvoltage for HS2
 * 
 *  \param e_value Switch recovery after removal of VS Overvoltage for HS2
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setHs2VsOverVoltRecovery(tHS_vsOverVoltRecovery e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS2_OV_REC_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_SW_SD_CTRL_HS2_OV_REC_Pos) & TLE9563_SW_SD_CTRL_HS2_OV_REC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Switch recovery after removal of VS Overvoltage for HS2
 * 
 *  \return uint16_t Switch recovery after removal of VS Overvoltage for HS2
 */
uint16_t TLE9563_getHs2VsOverVoltRecovery(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SW_SD_CTRL.bit.HS2_OV_REC;
}

/** \brief Set Switch recovery after removal of VS Overvoltage for HS1
 * 
 *  \param e_value Switch recovery after removal of VS Overvoltage for HS1
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setHs1VsOverVoltRecovery(tHS_vsOverVoltRecovery e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS1_OV_REC_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_SW_SD_CTRL_HS1_OV_REC_Pos) & TLE9563_SW_SD_CTRL_HS1_OV_REC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Switch recovery after removal of VS Overvoltage for HS1
 * 
 *  \return uint16_t Switch recovery after removal of VS Overvoltage for HS1
 */
uint16_t TLE9563_getHs1VsOverVoltRecovery(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SW_SD_CTRL.bit.HS1_OV_REC;
}

/** \brief Enable Shutdown Disabling of all HS in case of Overtemperature event
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enHsxOverTempShutDownAll(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS_OT_SD_DIS_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SW_SD_CTRL_HS_OT_SD_DIS_Pos) & TLE9563_SW_SD_CTRL_HS_OT_SD_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Shutdown Disabling of all HS in case of Overtemperature event
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disHsxOverTempShutDownAll(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS_OT_SD_DIS_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SW_SD_CTRL_HS_OT_SD_DIS_Pos) & TLE9563_SW_SD_CTRL_HS_OT_SD_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Shutdown Disabling of all HS in case of Overtemperature event Enable/Disable Setting
 * 
 *  \return  Shutdown Disabling of all HS in case of Overtemperature event Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getHsxOverTempShutDownAll(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SW_SD_CTRL.bit.HS_OT_SD_DIS;
}

/** \brief Enable Shutdown Disabling of HS3 in case of input supply overvoltage in Normal Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enHs3VsOverVoltShutDownNormalMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS3_OV_SDN_DIS_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SW_SD_CTRL_HS3_OV_SDN_DIS_Pos) & TLE9563_SW_SD_CTRL_HS3_OV_SDN_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Shutdown Disabling of HS3 in case of input supply overvoltage in Normal Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disHs3VsOverVoltShutDownNormalMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS3_OV_SDN_DIS_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SW_SD_CTRL_HS3_OV_SDN_DIS_Pos) & TLE9563_SW_SD_CTRL_HS3_OV_SDN_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Shutdown Disabling of HS3 in case of input supply overvoltage in Normal Mode Enable/Disable Setting
 * 
 *  \return  Shutdown Disabling of HS3 in case of input supply overvoltage in Normal Mode Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getHs3VsOverVoltShutDownNormalMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SW_SD_CTRL.bit.HS3_OV_SDN_DIS;
}

/** \brief Enable Shutdown Disabling of HS2 in case of input supply overvoltage in Normal Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enHs2VsOverVoltShutDownNormalMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS2_OV_SDN_DIS_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SW_SD_CTRL_HS2_OV_SDN_DIS_Pos) & TLE9563_SW_SD_CTRL_HS2_OV_SDN_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Shutdown Disabling of HS2 in case of input supply overvoltage in Normal Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disHs2VsOverVoltShutDownNormalMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS2_OV_SDN_DIS_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SW_SD_CTRL_HS2_OV_SDN_DIS_Pos) & TLE9563_SW_SD_CTRL_HS2_OV_SDN_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Shutdown Disabling of HS2 in case of input supply overvoltage in Normal Mode Enable/Disable Setting
 * 
 *  \return  Shutdown Disabling of HS2 in case of input supply overvoltage in Normal Mode Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getHs2VsOverVoltShutDownNormalMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SW_SD_CTRL.bit.HS2_OV_SDN_DIS;
}

/** \brief Enable Shutdown Disabling of HS1 in case of input supply overvoltage in Normal Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enHs1VsOverVoltShutDownNormalMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS1_OV_SDN_DIS_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SW_SD_CTRL_HS1_OV_SDN_DIS_Pos) & TLE9563_SW_SD_CTRL_HS1_OV_SDN_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Shutdown Disabling of HS1 in case of input supply overvoltage in Normal Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disHs1VsOverVoltShutDownNormalMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS1_OV_SDN_DIS_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SW_SD_CTRL_HS1_OV_SDN_DIS_Pos) & TLE9563_SW_SD_CTRL_HS1_OV_SDN_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Shutdown Disabling of HS1 in case of input supply overvoltage in Normal Mode Enable/Disable Setting
 * 
 *  \return  Shutdown Disabling of HS1 in case of input supply overvoltage in Normal Mode Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getHs1VsOverVoltShutDownNormalMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SW_SD_CTRL.bit.HS1_OV_SDN_DIS;
}

/** \brief Enable Shutdown Disabling of HSx in case of input supply overvoltage in Stop Mode or Sleep Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enHsxVsOverVoltShutDownStopModeSleepMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS_OV_SDS_DIS_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SW_SD_CTRL_HS_OV_SDS_DIS_Pos) & TLE9563_SW_SD_CTRL_HS_OV_SDS_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Shutdown Disabling of HSx in case of input supply overvoltage in Stop Mode or Sleep Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disHsxVsOverVoltShutDownStopModeSleepMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS_OV_SDS_DIS_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SW_SD_CTRL_HS_OV_SDS_DIS_Pos) & TLE9563_SW_SD_CTRL_HS_OV_SDS_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Shutdown Disabling of HSx in case of input supply overvoltage in Stop Mode or Sleep Mode Enable/Disable Setting
 * 
 *  \return  Shutdown Disabling of HSx in case of input supply overvoltage in Stop Mode or Sleep Mode Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getHsxVsOverVoltShutDownStopModeSleepMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SW_SD_CTRL.bit.HS_OV_SDS_DIS;
}

/** \brief Enable Shutdown Disabling of HSx in case of input supply undervoltage
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enHsxVsUnderVoltShutDown(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS_UV_SD_DIS_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SW_SD_CTRL_HS_UV_SD_DIS_Pos) & TLE9563_SW_SD_CTRL_HS_UV_SD_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Shutdown Disabling of HSx in case of input supply undervoltage
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disHsxVsUnderVoltShutDown(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS_UV_SD_DIS_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SW_SD_CTRL_HS_UV_SD_DIS_Pos) & TLE9563_SW_SD_CTRL_HS_UV_SD_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Shutdown Disabling of HSx in case of input supply undervoltage Enable/Disable Setting
 * 
 *  \return  Shutdown Disabling of HSx in case of input supply undervoltage Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getHsxVsUnderVoltShutDown(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SW_SD_CTRL.bit.HS_UV_SD_DIS;
}

/** \brief Set Switch recovery after removal of Undervoltage for HSx
 * 
 *  \param e_value Switch recovery after removal of Undervoltage for HSx
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setHsxVsUnderVoltRecovery(tHS_vsUnderVoltRecovery e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SW_SD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SW_SD_CTRL_HS_UV_REC_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_SW_SD_CTRL_HS_UV_REC_Pos) & TLE9563_SW_SD_CTRL_HS_UV_REC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWSDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Switch recovery after removal of Undervoltage for HSx
 * 
 *  \return uint16_t Switch recovery after removal of Undervoltage for HSx
 */
uint16_t TLE9563_getHsxVsUnderVoltRecovery(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SW_SD_CTRL.bit.HS_UV_REC;
}

/** \brief Get High-Side Switch Control Register
 * 
 *  \return uint16_t High-Side Switch Control Register
 */
uint16_t TLE9563_getHsCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HS_CTRL.reg;
}

/** \brief Set High-Side Switch Control Register
 *  \brief The desired duty cycle should be set first in according PWM_CTRL register before the HSx is enabled as PWM.
 * 
 *  \param u16_value High-Side Switch Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setHsCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHSCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set HS3 Configuration
 *  \brief The desired duty cycle should be set first in according PWM_CTRL register before the HSx is enabled as PWM.
 * 
 *  \param e_value HS3 Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setHs3Config(tHS_config e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HS_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HS_CTRL_HS3_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HS_CTRL_HS3_Pos) & TLE9563_HS_CTRL_HS3_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHSCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get HS3 Configuration
 * 
 *  \return uint16_t HS3 Configuration
 */
uint16_t TLE9563_getHs3Config(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HS_CTRL.bit.HS3;
}

/** \brief Set HS2 Configuration
 *  \brief The desired duty cycle should be set first in according PWM_CTRL register before the HSx is enabled as PWM.
 * 
 *  \param e_value HS2 Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setHs2Config(tHS_config e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HS_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HS_CTRL_HS2_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HS_CTRL_HS2_Pos) & TLE9563_HS_CTRL_HS2_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHSCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get HS2 Configuration
 * 
 *  \return uint16_t HS2 Configuration
 */
uint16_t TLE9563_getHs2Config(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HS_CTRL.bit.HS2;
}

/** \brief Set HS1 Configuration
 *  \brief The desired duty cycle should be set first in according PWM_CTRL register before the HSx is enabled as PWM.
 * 
 *  \param e_value HS1 Configuration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setHs1Config(tHS_config e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HS_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HS_CTRL_HS1_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HS_CTRL_HS1_Pos) & TLE9563_HS_CTRL_HS1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHSCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get HS1 Configuration
 * 
 *  \return uint16_t HS1 Configuration
 */
uint16_t TLE9563_getHs1Config(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HS_CTRL.bit.HS1;
}

/** \brief Get Interrupt Mask Control Register
 * 
 *  \return uint16_t Interrupt Mask Control Register
 */
uint16_t TLE9563_getIntrMaskCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->INT_MASK.reg;
}

/** \brief Set Interrupt Mask Control Register
 * 
 *  \param u16_value Interrupt Mask Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setIntrMaskCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Enable Periodical INTN generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enIntnPeriodical(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_INTN_CYC_EN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_INT_MASK_INTN_CYC_EN_Pos) & TLE9563_INT_MASK_INTN_CYC_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Periodical INTN generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disIntnPeriodical(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_INTN_CYC_EN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_INT_MASK_INTN_CYC_EN_Pos) & TLE9563_INT_MASK_INTN_CYC_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Periodical INTN generation Enable/Disable Setting
 * 
 *  \return  Periodical INTN generation Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getIntnPeriodical(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->INT_MASK.bit.INTN_CYC_EN;
}

/** \brief Enable Disable Watchdog in Software Development Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enWdInSoftDevMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_WD_SDM_DISABLE_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_INT_MASK_WD_SDM_DISABLE_Pos) & TLE9563_INT_MASK_WD_SDM_DISABLE_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Disable Watchdog in Software Development Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disWdInSoftDevMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_WD_SDM_DISABLE_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_INT_MASK_WD_SDM_DISABLE_Pos) & TLE9563_INT_MASK_WD_SDM_DISABLE_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Disable Watchdog in Software Development Mode Enable/Disable Setting
 * 
 *  \return  Disable Watchdog in Software Development Mode Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getWdInSoftDevMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->INT_MASK.bit.WD_SDM_DISABLE;
}

/** \brief Enable Watchdog failure in Software Development Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enIntnWdFail(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_WD_SDM_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_INT_MASK_WD_SDM_Pos) & TLE9563_INT_MASK_WD_SDM_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Watchdog failure in Software Development Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disIntnWdFail(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_WD_SDM_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_INT_MASK_WD_SDM_Pos) & TLE9563_INT_MASK_WD_SDM_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Watchdog failure in Software Development Mode Enable/Disable Setting
 * 
 *  \return  Watchdog failure in Software Development Mode Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getIntnWdFail(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->INT_MASK.bit.WD_SDM;
}

/** \brief Enable SPI and CRC interrupt generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enIntnSpiFailOrCrcFail(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_SPI_CRC_FAIL_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_INT_MASK_SPI_CRC_FAIL_Pos) & TLE9563_INT_MASK_SPI_CRC_FAIL_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable SPI and CRC interrupt generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disIntnSpiFailOrCrcFail(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_SPI_CRC_FAIL_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_INT_MASK_SPI_CRC_FAIL_Pos) & TLE9563_INT_MASK_SPI_CRC_FAIL_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get SPI and CRC interrupt generation Enable/Disable Setting
 * 
 *  \return  SPI and CRC interrupt generation Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getIntnSpiFailOrCrcFail(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->INT_MASK.bit.SPI_CRC_FAIL;
}

/** \brief Enable Bridge Driver Interrupt generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enIntnBdrvStat(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_BD_STAT_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_INT_MASK_BD_STAT_Pos) & TLE9563_INT_MASK_BD_STAT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Bridge Driver Interrupt generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disIntnBdrvStat(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_BD_STAT_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_INT_MASK_BD_STAT_Pos) & TLE9563_INT_MASK_BD_STAT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Bridge Driver Interrupt generation Enable/Disable Setting
 * 
 *  \return  Bridge Driver Interrupt generation Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getIntnBdrvStat(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->INT_MASK.bit.BD_STAT;
}

/** \brief Enable High Side Interrupt generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enIntnHsStat(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_HS_STAT_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_INT_MASK_HS_STAT_Pos) & TLE9563_INT_MASK_HS_STAT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable High Side Interrupt generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disIntnHsStat(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_HS_STAT_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_INT_MASK_HS_STAT_Pos) & TLE9563_INT_MASK_HS_STAT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get High Side Interrupt generation Enable/Disable Setting
 * 
 *  \return  High Side Interrupt generation Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getIntnHsStat(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->INT_MASK.bit.HS_STAT;
}

/** \brief Enable BUS Interrupt generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enIntnBusStat(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_BUS_STAT_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_INT_MASK_BUS_STAT_Pos) & TLE9563_INT_MASK_BUS_STAT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable BUS Interrupt generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disIntnBusStat(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_BUS_STAT_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_INT_MASK_BUS_STAT_Pos) & TLE9563_INT_MASK_BUS_STAT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get BUS Interrupt generation Enable/Disable Setting
 * 
 *  \return  BUS Interrupt generation Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getIntnBusStat(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->INT_MASK.bit.BUS_STAT;
}

/** \brief Enable Temperature Interrupt generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enIntnTempStat(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_TEMP_STAT_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_INT_MASK_TEMP_STAT_Pos) & TLE9563_INT_MASK_TEMP_STAT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Temperature Interrupt generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disIntnTempStat(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_TEMP_STAT_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_INT_MASK_TEMP_STAT_Pos) & TLE9563_INT_MASK_TEMP_STAT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Temperature Interrupt generation Enable/Disable Setting
 * 
 *  \return  Temperature Interrupt generation Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getIntnTempStat(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->INT_MASK.bit.TEMP_STAT;
}

/** \brief Enable SUPPLY Status Interrupt generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enIntnSupplyStat(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_SUPPLY_STAT_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_INT_MASK_SUPPLY_STAT_Pos) & TLE9563_INT_MASK_SUPPLY_STAT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable SUPPLY Status Interrupt generation
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disIntnSupplyStat(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->INT_MASK.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_INT_MASK_SUPPLY_STAT_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_INT_MASK_SUPPLY_STAT_Pos) & TLE9563_INT_MASK_SUPPLY_STAT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setINTMASK;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get SUPPLY Status Interrupt generation Enable/Disable Setting
 * 
 *  \return  SUPPLY Status Interrupt generation Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getIntnSupplyStat(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->INT_MASK.bit.SUPPLY_STAT;
}

/** \brief Get PWM Configuration Control Register
 * 
 *  \return uint16_t PWM Configuration Control Register
 */
uint16_t TLE9563_getPwm1CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->PWM_CTRL_BNK0.reg;
}

/** \brief Set PWM Configuration Control Register
 * 
 *  \param u16_value PWM Configuration Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPwm1CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setPWMCTRLBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set PWM generator Frequency Setting
 * 
 *  \param e_value PWM generator Frequency Setting
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPwm1Freq(tPWM_freq e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->PWM_CTRL_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_PWM_CTRL_BNK0_PWM_FREQ_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_PWM_CTRL_BNK0_PWM_FREQ_Pos) & TLE9563_PWM_CTRL_BNK0_PWM_FREQ_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setPWMCTRLBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get PWM generator Frequency Setting
 * 
 *  \return uint16_t PWM generator Frequency Setting
 */
uint16_t TLE9563_getPwm1Freq(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->PWM_CTRL_BNK0.bit.PWM_FREQ;
}

/** \brief Set PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 * 
 *  \param u16_value PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPwm1DutyCycle(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->PWM_CTRL_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_PWM_CTRL_BNK0_PWM_DC_Msk)) | (((uint16_t)u16_value << (uint16_t)TLE9563_PWM_CTRL_BNK0_PWM_DC_Pos) & TLE9563_PWM_CTRL_BNK0_PWM_DC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setPWMCTRLBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 * 
 *  \return uint16_t PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 */
uint16_t TLE9563_getPwm1DutyCycle(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->PWM_CTRL_BNK0.bit.PWM_DC;
}

/** \brief Get PWM Configuration Control Register
 * 
 *  \return uint16_t PWM Configuration Control Register
 */
uint16_t TLE9563_getPwm2CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->PWM_CTRL_BNK1.reg;
}

/** \brief Set PWM Configuration Control Register
 * 
 *  \param u16_value PWM Configuration Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPwm2CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setPWMCTRLBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set PWM generator Frequency Setting
 * 
 *  \param e_value PWM generator Frequency Setting
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPwm2Freq(tPWM_freq e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->PWM_CTRL_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_PWM_CTRL_BNK1_PWM_FREQ_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_PWM_CTRL_BNK1_PWM_FREQ_Pos) & TLE9563_PWM_CTRL_BNK1_PWM_FREQ_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setPWMCTRLBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get PWM generator Frequency Setting
 * 
 *  \return uint16_t PWM generator Frequency Setting
 */
uint16_t TLE9563_getPwm2Freq(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->PWM_CTRL_BNK1.bit.PWM_FREQ;
}

/** \brief Set PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 * 
 *  \param u16_value PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPwm2DutyCycle(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->PWM_CTRL_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_PWM_CTRL_BNK1_PWM_DC_Msk)) | (((uint16_t)u16_value << (uint16_t)TLE9563_PWM_CTRL_BNK1_PWM_DC_Pos) & TLE9563_PWM_CTRL_BNK1_PWM_DC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setPWMCTRLBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 * 
 *  \return uint16_t PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 */
uint16_t TLE9563_getPwm2DutyCycle(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->PWM_CTRL_BNK1.bit.PWM_DC;
}

/** \brief Get PWM Configuration Control Register
 * 
 *  \return uint16_t PWM Configuration Control Register
 */
uint16_t TLE9563_getPwm3CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->PWM_CTRL_BNK2.reg;
}

/** \brief Set PWM Configuration Control Register
 * 
 *  \param u16_value PWM Configuration Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPwm3CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setPWMCTRLBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set PWM generator Frequency Setting
 * 
 *  \param e_value PWM generator Frequency Setting
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPwm3Freq(tPWM_freq e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->PWM_CTRL_BNK2.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_PWM_CTRL_BNK2_PWM_FREQ_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_PWM_CTRL_BNK2_PWM_FREQ_Pos) & TLE9563_PWM_CTRL_BNK2_PWM_FREQ_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setPWMCTRLBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get PWM generator Frequency Setting
 * 
 *  \return uint16_t PWM generator Frequency Setting
 */
uint16_t TLE9563_getPwm3Freq(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->PWM_CTRL_BNK2.bit.PWM_FREQ;
}

/** \brief Set PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 * 
 *  \param u16_value PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPwm3DutyCycle(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->PWM_CTRL_BNK2.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_PWM_CTRL_BNK2_PWM_DC_Msk)) | (((uint16_t)u16_value << (uint16_t)TLE9563_PWM_CTRL_BNK2_PWM_DC_Pos) & TLE9563_PWM_CTRL_BNK2_PWM_DC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setPWMCTRLBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 * 
 *  \return uint16_t PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 */
uint16_t TLE9563_getPwm3DutyCycle(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->PWM_CTRL_BNK2.bit.PWM_DC;
}

/** \brief Get PWM Configuration Control Register
 * 
 *  \return uint16_t PWM Configuration Control Register
 */
uint16_t TLE9563_getPwm4CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->PWM_CTRL_BNK3.reg;
}

/** \brief Set PWM Configuration Control Register
 * 
 *  \param u16_value PWM Configuration Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPwm4CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setPWMCTRLBNK3;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set PWM generator Frequency Setting
 * 
 *  \param e_value PWM generator Frequency Setting
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPwm4Freq(tPWM_freq e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->PWM_CTRL_BNK3.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_PWM_CTRL_BNK3_PWM_FREQ_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_PWM_CTRL_BNK3_PWM_FREQ_Pos) & TLE9563_PWM_CTRL_BNK3_PWM_FREQ_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setPWMCTRLBNK3;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get PWM generator Frequency Setting
 * 
 *  \return uint16_t PWM generator Frequency Setting
 */
uint16_t TLE9563_getPwm4Freq(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->PWM_CTRL_BNK3.bit.PWM_FREQ;
}

/** \brief Set PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 * 
 *  \param u16_value PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPwm4DutyCycle(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->PWM_CTRL_BNK3.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_PWM_CTRL_BNK3_PWM_DC_Msk)) | (((uint16_t)u16_value << (uint16_t)TLE9563_PWM_CTRL_BNK3_PWM_DC_Pos) & TLE9563_PWM_CTRL_BNK3_PWM_DC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setPWMCTRLBNK3;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 * 
 *  \return uint16_t PWM Duty Cycle Setting (bit4 = LSB; bit13 = MSB) 
 */
uint16_t TLE9563_getPwm4DutyCycle(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->PWM_CTRL_BNK3.bit.PWM_DC;
}

/** \brief Set System Status Control (bit0=LSB; bit15=MSB) Dedicated bytes for system configuration, access only by microcontroller. Cleared after power up and soft reset.
 * 
 *  \param u16_value System Status Control (bit0=LSB; bit15=MSB) Dedicated bytes for system configuration, access only by microcontroller. Cleared after power up and soft reset.
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSysStatCtrl(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SYS_STAT_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SYS_STAT_CTRL_SYS_STAT_Msk)) | (((uint16_t)u16_value << (uint16_t)TLE9563_SYS_STAT_CTRL_SYS_STAT_Pos) & TLE9563_SYS_STAT_CTRL_SYS_STAT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSYSSTATCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get System Status Control (bit0=LSB; bit15=MSB) Dedicated bytes for system configuration, access only by microcontroller. Cleared after power up and soft reset.
 * 
 *  \return uint16_t System Status Control (bit0=LSB; bit15=MSB) Dedicated bytes for system configuration, access only by microcontroller. Cleared after power up and soft reset.
 */
uint16_t TLE9563_getSysStatCtrl(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SYS_STAT_CTRL.bit.SYS_STAT;
}

/** \brief Get General Bridge Control Register
 * 
 *  \return uint16_t General Bridge Control Register
 */
uint16_t TLE9563_getGenBridgeCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->GENCTRL.reg;
}

/** \brief Set General Bridge Control Register
 * 
 *  \param u16_value General Bridge Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setGenBridgeCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Bridge driver synchronization frequency
 * 
 *  \param e_value Bridge driver synchronization frequency
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvFreq(tBDRV_freq e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_BDFREQ_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_GENCTRL_BDFREQ_Pos) & TLE9563_GENCTRL_BDFREQ_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Bridge driver synchronization frequency
 * 
 *  \return uint16_t Bridge driver synchronization frequency
 */
uint16_t TLE9563_getBdrvFreq(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->GENCTRL.bit.BDFREQ;
}

/** \brief Set Charge pump under voltage (referred to VS)
 * 
 *  \param e_value Charge pump under voltage (referred to VS)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCpUnderVoltThresh(tCP_underVoltThresh e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_CPUVTH_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_GENCTRL_CPUVTH_Pos) & TLE9563_GENCTRL_CPUVTH_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Charge pump under voltage (referred to VS)
 * 
 *  \return uint16_t Charge pump under voltage (referred to VS)
 */
uint16_t TLE9563_getCpUnderVoltThresh(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->GENCTRL.bit.CPUVTH;
}

/** \brief Set External MOSFET normal / logic level selection
 * 
 *  \param e_value External MOSFET normal / logic level selection
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvExtMosfetLvl(tBDRV_extMosfetLvl e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_FET_LVL_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_GENCTRL_FET_LVL_Pos) & TLE9563_GENCTRL_FET_LVL_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get External MOSFET normal / logic level selection
 * 
 *  \return uint16_t External MOSFET normal / logic level selection
 */
uint16_t TLE9563_getBdrvExtMosfetLvl(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->GENCTRL.bit.FET_LVL;
}

/** \brief Enable Automatic switchover between dual and single charge pump stage
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enCpAutoSwitchBetweenDualAndSingleStage(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_CPSTGA_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_GENCTRL_CPSTGA_Pos) & TLE9563_GENCTRL_CPSTGA_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Automatic switchover between dual and single charge pump stage
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disCpAutoSwitchBetweenDualAndSingleStage(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_CPSTGA_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_GENCTRL_CPSTGA_Pos) & TLE9563_GENCTRL_CPSTGA_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Automatic switchover between dual and single charge pump stage Enable/Disable Setting
 * 
 *  \return  Automatic switchover between dual and single charge pump stage Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getCpAutoSwitchBetweenDualAndSingleStage(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GENCTRL.bit.CPSTGA;
}

/** \brief Enable Bridge driver recover from VS and VSINT Overvoltage
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvVsVsintOverVoltRecovery(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_BDOV_REC_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_GENCTRL_BDOV_REC_Pos) & TLE9563_GENCTRL_BDOV_REC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Bridge driver recover from VS and VSINT Overvoltage
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvVsVsintOverVoltRecovery(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_BDOV_REC_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_GENCTRL_BDOV_REC_Pos) & TLE9563_GENCTRL_BDOV_REC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Bridge driver recover from VS and VSINT Overvoltage Enable/Disable Setting
 * 
 *  \return  Bridge driver recover from VS and VSINT Overvoltage Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvVsVsintOverVoltRecovery(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GENCTRL.bit.BDOV_REC;
}

/** \brief Set Adaptation of the pre-charge and pre-discharge current
 * 
 *  \param e_value Adaptation of the pre-charge and pre-discharge current
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvPreChargePreDischargeCurAdaption(tBDRV_preChargePreDischargeCurAdaption e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_IPCHGADT_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_GENCTRL_IPCHGADT_Pos) & TLE9563_GENCTRL_IPCHGADT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Adaptation of the pre-charge and pre-discharge current
 * 
 *  \return uint16_t Adaptation of the pre-charge and pre-discharge current
 */
uint16_t TLE9563_getBdrvPreChargePreDischargeCurAdaption(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->GENCTRL.bit.IPCHGADT;
}

/** \brief Set Adaptive gate control
 * 
 *  \param e_value Adaptive gate control
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvAdaptGateCtrl(tBDRV_adaptGateCtrl e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_AGC_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_GENCTRL_AGC_Pos) & TLE9563_GENCTRL_AGC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Adaptive gate control
 * 
 *  \return uint16_t Adaptive gate control
 */
uint16_t TLE9563_getBdrvAdaptGateCtrl(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->GENCTRL.bit.AGC;
}

/** \brief Enable CPEN
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enCp(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_CPEN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_GENCTRL_CPEN_Pos) & TLE9563_GENCTRL_CPEN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable CPEN
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disCp(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_CPEN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_GENCTRL_CPEN_Pos) & TLE9563_GENCTRL_CPEN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get CPEN Enable/Disable Setting
 * 
 *  \return  CPEN Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getCp(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GENCTRL.bit.CPEN;
}

/** \brief Enable Postcharge disable bit
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvPostCharge(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_POCHGDIS_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_GENCTRL_POCHGDIS_Pos) & TLE9563_GENCTRL_POCHGDIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Postcharge disable bit
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvPostCharge(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_POCHGDIS_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_GENCTRL_POCHGDIS_Pos) & TLE9563_GENCTRL_POCHGDIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Postcharge disable bit Enable/Disable Setting
 * 
 *  \return  Postcharge disable bit Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getBdrvPostCharge(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GENCTRL.bit.POCHGDIS;
}

/** \brief Enable Filter for adaptive gate control
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvAdaptGateFilt(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_AGCFILT_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_GENCTRL_AGCFILT_Pos) & TLE9563_GENCTRL_AGCFILT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Filter for adaptive gate control
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvAdaptGateFilt(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_AGCFILT_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_GENCTRL_AGCFILT_Pos) & TLE9563_GENCTRL_AGCFILT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Filter for adaptive gate control Enable/Disable Setting
 * 
 *  \return  Filter for adaptive gate control Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvAdaptGateFilt(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GENCTRL.bit.AGCFILT;
}

/** \brief Enable Detection of active / FW MOSFET
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvDetectGeneratorMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_EN_GEN_CHECK_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_GENCTRL_EN_GEN_CHECK_Pos) & TLE9563_GENCTRL_EN_GEN_CHECK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Detection of active / FW MOSFET
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvDetectGeneratorMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_EN_GEN_CHECK_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_GENCTRL_EN_GEN_CHECK_Pos) & TLE9563_GENCTRL_EN_GEN_CHECK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Detection of active / FW MOSFET Enable/Disable Setting
 * 
 *  \return  Detection of active / FW MOSFET Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvDetectGeneratorMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GENCTRL.bit.EN_GEN_CHECK;
}

/** \brief Set Gate driver hold current IHOLD
 * 
 *  \param e_value Gate driver hold current IHOLD
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHoldCur(tBDRV_holdCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_IHOLD_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_GENCTRL_IHOLD_Pos) & TLE9563_GENCTRL_IHOLD_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Gate driver hold current IHOLD
 * 
 *  \return uint16_t Gate driver hold current IHOLD
 */
uint16_t TLE9563_getBdrvHoldCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->GENCTRL.bit.IHOLD;
}

/** \brief Enable Frequency modulation of the charge pump
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enCpFreqMod(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_FMODE_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_GENCTRL_FMODE_Pos) & TLE9563_GENCTRL_FMODE_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Frequency modulation of the charge pump
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disCpFreqMod(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->GENCTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_GENCTRL_FMODE_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_GENCTRL_FMODE_Pos) & TLE9563_GENCTRL_FMODE_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setGENCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Frequency modulation of the charge pump Enable/Disable Setting
 * 
 *  \return  Frequency modulation of the charge pump Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getCpFreqMod(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GENCTRL.bit.FMODE;
}

/** \brief Get Current sense amplifier Register
 * 
 *  \return uint16_t Current sense amplifier Register
 */
uint16_t TLE9563_getCsaCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CSA.reg;
}

/** \brief Set Current sense amplifier Register
 * 
 *  \param u16_value Current sense amplifier Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCsaCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setCSA;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Selection of 3 or 6 PWM inputs
 * 
 *  \param e_value Selection of 3 or 6 PWM inputs
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvNumberOfPwmInputs(tBDRV_numberOfPwmInputs e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CSA.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CSA_PWM_NB_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CSA_PWM_NB_Pos) & TLE9563_CSA_PWM_NB_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCSA;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Selection of 3 or 6 PWM inputs
 * 
 *  \return uint16_t Selection of 3 or 6 PWM inputs
 */
uint16_t TLE9563_getBdrvNumberOfPwmInputs(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CSA.bit.PWM_NB;
}

/** \brief Set Capacitance connected to the current sense amplifier output (CCSO), see also Chapter 12.12.4
 * 
 *  \param e_value Capacitance connected to the current sense amplifier output (CCSO), see also Chapter 12.12.4
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCsaCap(tCSA_cap e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CSA.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CSA_CSO_CAP_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CSA_CSO_CAP_Pos) & TLE9563_CSA_CSO_CAP_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCSA;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Capacitance connected to the current sense amplifier output (CCSO), see also Chapter 12.12.4
 * 
 *  \return uint16_t Capacitance connected to the current sense amplifier output (CCSO), see also Chapter 12.12.4
 */
uint16_t TLE9563_getCsaCap(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CSA.bit.CSO_CAP;
}

/** \brief Set Direction of the current sense amplifier
 * 
 *  \param e_value Direction of the current sense amplifier
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCsaDir(tCSA_dir e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CSA.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CSA_CSD_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CSA_CSD_Pos) & TLE9563_CSA_CSD_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCSA;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Direction of the current sense amplifier
 * 
 *  \return uint16_t Direction of the current sense amplifier
 */
uint16_t TLE9563_getCsaDir(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CSA.bit.CSD;
}

/** \brief Set Overcurrent filter time of CSO
 * 
 *  \param e_value Overcurrent filter time of CSO
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCsaOverCurFiltTime(tCSA_overCurFiltTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CSA.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CSA_OCFILT_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CSA_OCFILT_Pos) & TLE9563_CSA_OCFILT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCSA;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Overcurrent filter time of CSO
 * 
 *  \return uint16_t Overcurrent filter time of CSO
 */
uint16_t TLE9563_getCsaOverCurFiltTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CSA.bit.OCFILT;
}

/** \brief Enable CSA OFF
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enCsa(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CSA.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CSA_CSA_OFF_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_CSA_CSA_OFF_Pos) & TLE9563_CSA_CSA_OFF_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCSA;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable CSA OFF
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disCsa(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CSA.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CSA_CSA_OFF_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_CSA_CSA_OFF_Pos) & TLE9563_CSA_CSA_OFF_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCSA;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get CSA OFF Enable/Disable Setting
 * 
 *  \return  CSA OFF Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getCsa(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->CSA.bit.CSA_OFF;
}

/** \brief Set Overcurrent detection threshold of CSO
 * 
 *  \param e_value Overcurrent detection threshold of CSO
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCsaOverCurThresh(tCSA_overCurThresh e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CSA.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CSA_OCTH_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CSA_OCTH_Pos) & TLE9563_CSA_OCTH_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCSA;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Overcurrent detection threshold of CSO
 * 
 *  \return uint16_t Overcurrent detection threshold of CSO
 */
uint16_t TLE9563_getCsaOverCurThresh(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CSA.bit.OCTH;
}

/** \brief Set Gain of the current sense amplifier
 * 
 *  \param e_value Gain of the current sense amplifier
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCsaGain(tCSA_gain e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CSA.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CSA_CSAG_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CSA_CSAG_Pos) & TLE9563_CSA_CSAG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCSA;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Gain of the current sense amplifier
 * 
 *  \return uint16_t Gain of the current sense amplifier
 */
uint16_t TLE9563_getCsaGain(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CSA.bit.CSAG;
}

/** \brief Enable Overcurrent shutdown Enable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enCsaOverCurShutDown(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CSA.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CSA_OCEN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_CSA_OCEN_Pos) & TLE9563_CSA_OCEN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCSA;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Overcurrent shutdown Enable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disCsaOverCurShutDown(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CSA.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CSA_OCEN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_CSA_OCEN_Pos) & TLE9563_CSA_OCEN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCSA;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Overcurrent shutdown Enable Enable/Disable Setting
 * 
 *  \return  Overcurrent shutdown Enable Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getCsaOverCurShutDown(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->CSA.bit.OCEN;
}

/** \brief Get VDS monitoring threshold LS1-3 Register
 * 
 *  \return uint16_t VDS monitoring threshold LS1-3 Register
 */
uint16_t TLE9563_getVdsLsCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->LS_VDS.reg;
}

/** \brief Set VDS monitoring threshold LS1-3 Register
 * 
 *  \param u16_value VDS monitoring threshold LS1-3 Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVdsLsCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setLSVDS;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Filter time of drain-source voltage monitoring
 * 
 *  \param e_value Filter time of drain-source voltage monitoring
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVdsFiltTime(tVDS_filtTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->LS_VDS.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_LS_VDS_TFVDS_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_LS_VDS_TFVDS_Pos) & TLE9563_LS_VDS_TFVDS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setLSVDS;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Filter time of drain-source voltage monitoring
 * 
 *  \return uint16_t Filter time of drain-source voltage monitoring
 */
uint16_t TLE9563_getVdsFiltTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->LS_VDS.bit.TFVDS;
}

/** \brief Set LS3 drain-source overvoltage threshold
 * 
 *  \param e_value LS3 drain-source overvoltage threshold
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVdsLs3OverVoltThresh(tVDS_lsOverVoltThresh e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->LS_VDS.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_LS_VDS_LS3VDSTH_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_LS_VDS_LS3VDSTH_Pos) & TLE9563_LS_VDS_LS3VDSTH_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setLSVDS;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get LS3 drain-source overvoltage threshold
 * 
 *  \return uint16_t LS3 drain-source overvoltage threshold
 */
uint16_t TLE9563_getVdsLs3OverVoltThresh(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->LS_VDS.bit.LS3VDSTH;
}

/** \brief Set LS2 drain-source overvoltage threshold
 * 
 *  \param e_value LS2 drain-source overvoltage threshold
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVdsLs2OverVoltThresh(tVDS_lsOverVoltThresh e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->LS_VDS.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_LS_VDS_LS2VDSTH_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_LS_VDS_LS2VDSTH_Pos) & TLE9563_LS_VDS_LS2VDSTH_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setLSVDS;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get LS2 drain-source overvoltage threshold
 * 
 *  \return uint16_t LS2 drain-source overvoltage threshold
 */
uint16_t TLE9563_getVdsLs2OverVoltThresh(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->LS_VDS.bit.LS2VDSTH;
}

/** \brief Set LS1 drain-source overvoltage threshold
 * 
 *  \param e_value LS1 drain-source overvoltage threshold
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVdsLs1OverVoltThresh(tVDS_lsOverVoltThresh e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->LS_VDS.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_LS_VDS_LS1VDSTH_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_LS_VDS_LS1VDSTH_Pos) & TLE9563_LS_VDS_LS1VDSTH_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setLSVDS;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get LS1 drain-source overvoltage threshold
 * 
 *  \return uint16_t LS1 drain-source overvoltage threshold
 */
uint16_t TLE9563_getVdsLs1OverVoltThresh(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->LS_VDS.bit.LS1VDSTH;
}

/** \brief Get VDS monitoring threshold HS1-3 Register
 * 
 *  \return uint16_t VDS monitoring threshold HS1-3 Register
 */
uint16_t TLE9563_getVdsHsCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HS_VDS.reg;
}

/** \brief Set VDS monitoring threshold HS1-3 Register
 * 
 *  \param u16_value VDS monitoring threshold HS1-3 Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVdsHsCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHSVDS;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Enable Deep adaptation enable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvDeepAdapt(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HS_VDS.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HS_VDS_DEEP_ADAP_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_HS_VDS_DEEP_ADAP_Pos) & TLE9563_HS_VDS_DEEP_ADAP_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHSVDS;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Deep adaptation enable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvDeepAdapt(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HS_VDS.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HS_VDS_DEEP_ADAP_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_HS_VDS_DEEP_ADAP_Pos) & TLE9563_HS_VDS_DEEP_ADAP_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHSVDS;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Deep adaptation enable Enable/Disable Setting
 * 
 *  \return  Deep adaptation enable Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvDeepAdapt(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HS_VDS.bit.DEEP_ADAP;
}

/** \brief Set HS3 drain-source overvoltage threshold
 * 
 *  \param e_value HS3 drain-source overvoltage threshold
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVdsHs3OverVoltThresh(tVDS_hsOverVoltThresh e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HS_VDS.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HS_VDS_HS3VDSTH_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HS_VDS_HS3VDSTH_Pos) & TLE9563_HS_VDS_HS3VDSTH_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHSVDS;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get HS3 drain-source overvoltage threshold
 * 
 *  \return uint16_t HS3 drain-source overvoltage threshold
 */
uint16_t TLE9563_getVdsHs3OverVoltThresh(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HS_VDS.bit.HS3VDSTH;
}

/** \brief Set HS2 drain-source overvoltage threshold
 * 
 *  \param e_value HS2 drain-source overvoltage threshold
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVdsHs2OverVoltThresh(tVDS_hsOverVoltThresh e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HS_VDS.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HS_VDS_HS2VDSTH_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HS_VDS_HS2VDSTH_Pos) & TLE9563_HS_VDS_HS2VDSTH_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHSVDS;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get HS2 drain-source overvoltage threshold
 * 
 *  \return uint16_t HS2 drain-source overvoltage threshold
 */
uint16_t TLE9563_getVdsHs2OverVoltThresh(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HS_VDS.bit.HS2VDSTH;
}

/** \brief Set HS1 drain-source overvoltage threshold
 * 
 *  \param e_value HS1 drain-source overvoltage threshold
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVdsHs1OverVoltThresh(tVDS_hsOverVoltThresh e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HS_VDS.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HS_VDS_HS1VDSTH_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HS_VDS_HS1VDSTH_Pos) & TLE9563_HS_VDS_HS1VDSTH_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHSVDS;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get HS1 drain-source overvoltage threshold
 * 
 *  \return uint16_t HS1 drain-source overvoltage threshold
 */
uint16_t TLE9563_getVdsHs1OverVoltThresh(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HS_VDS.bit.HS1VDSTH;
}

/** \brief Get CCP and times selection Register
 * 
 *  \return uint16_t CCP and times selection Register
 */
uint16_t TLE9563_getCcpBlankHb1ActCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK0.reg;
}

/** \brief Set CCP and times selection Register
 * 
 *  \param u16_value CCP and times selection Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCcpBlankHb1ActCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Blank time
 * 
 *  \param e_value Blank time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1ActTblank(tBDRV_tBlank e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CCP_BLK_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CCP_BLK_BNK0_TBLANK_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CCP_BLK_BNK0_TBLANK_Pos) & TLE9563_CCP_BLK_BNK0_TBLANK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Blank time
 * 
 *  \return uint16_t Blank time
 */
uint16_t TLE9563_getBdrvHb1ActTblank(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK0.bit.TBLANK;
}

/** \brief Set Cross-current protection time
 * 
 *  \param e_value Cross-current protection time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1ActTccp(tBDRV_tCcp e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CCP_BLK_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CCP_BLK_BNK0_TCCP_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CCP_BLK_BNK0_TCCP_Pos) & TLE9563_CCP_BLK_BNK0_TCCP_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Cross-current protection time
 * 
 *  \return uint16_t Cross-current protection time
 */
uint16_t TLE9563_getBdrvHb1ActTccp(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK0.bit.TCCP;
}

/** \brief Get CCP and times selection Register
 * 
 *  \return uint16_t CCP and times selection Register
 */
uint16_t TLE9563_getCcpBlankHb2ActCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK1.reg;
}

/** \brief Set CCP and times selection Register
 * 
 *  \param u16_value CCP and times selection Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCcpBlankHb2ActCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Blank time
 * 
 *  \param e_value Blank time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2ActTblank(tBDRV_tBlank e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CCP_BLK_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CCP_BLK_BNK1_TBLANK_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CCP_BLK_BNK1_TBLANK_Pos) & TLE9563_CCP_BLK_BNK1_TBLANK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Blank time
 * 
 *  \return uint16_t Blank time
 */
uint16_t TLE9563_getBdrvHb2ActTblank(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK1.bit.TBLANK;
}

/** \brief Set Cross-current protection time
 * 
 *  \param e_value Cross-current protection time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2ActTccp(tBDRV_tCcp e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CCP_BLK_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CCP_BLK_BNK1_TCCP_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CCP_BLK_BNK1_TCCP_Pos) & TLE9563_CCP_BLK_BNK1_TCCP_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Cross-current protection time
 * 
 *  \return uint16_t Cross-current protection time
 */
uint16_t TLE9563_getBdrvHb2ActTccp(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK1.bit.TCCP;
}

/** \brief Get CCP and times selection Register
 * 
 *  \return uint16_t CCP and times selection Register
 */
uint16_t TLE9563_getCcpBlankHb3ActCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK2.reg;
}

/** \brief Set CCP and times selection Register
 * 
 *  \param u16_value CCP and times selection Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCcpBlankHb3ActCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Blank time
 * 
 *  \param e_value Blank time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3ActTblank(tBDRV_tBlank e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CCP_BLK_BNK2.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CCP_BLK_BNK2_TBLANK_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CCP_BLK_BNK2_TBLANK_Pos) & TLE9563_CCP_BLK_BNK2_TBLANK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Blank time
 * 
 *  \return uint16_t Blank time
 */
uint16_t TLE9563_getBdrvHb3ActTblank(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK2.bit.TBLANK;
}

/** \brief Set Cross-current protection time
 * 
 *  \param e_value Cross-current protection time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3ActTccp(tBDRV_tCcp e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CCP_BLK_BNK2.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CCP_BLK_BNK2_TCCP_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CCP_BLK_BNK2_TCCP_Pos) & TLE9563_CCP_BLK_BNK2_TCCP_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Cross-current protection time
 * 
 *  \return uint16_t Cross-current protection time
 */
uint16_t TLE9563_getBdrvHb3ActTccp(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK2.bit.TCCP;
}

/** \brief Get CCP and times selection Register
 * 
 *  \return uint16_t CCP and times selection Register
 */
uint16_t TLE9563_getCcpBlankHb1FwCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK4.reg;
}

/** \brief Set CCP and times selection Register
 * 
 *  \param u16_value CCP and times selection Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCcpBlankHb1FwCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK4;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Blank time
 * 
 *  \param e_value Blank time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1FwTblank(tBDRV_tBlank e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CCP_BLK_BNK4.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CCP_BLK_BNK4_TBLANK_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CCP_BLK_BNK4_TBLANK_Pos) & TLE9563_CCP_BLK_BNK4_TBLANK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK4;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Blank time
 * 
 *  \return uint16_t Blank time
 */
uint16_t TLE9563_getBdrvHb1FwTblank(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK4.bit.TBLANK;
}

/** \brief Set Cross-current protection time
 * 
 *  \param e_value Cross-current protection time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1FwTccp(tBDRV_tCcp e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CCP_BLK_BNK4.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CCP_BLK_BNK4_TCCP_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CCP_BLK_BNK4_TCCP_Pos) & TLE9563_CCP_BLK_BNK4_TCCP_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK4;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Cross-current protection time
 * 
 *  \return uint16_t Cross-current protection time
 */
uint16_t TLE9563_getBdrvHb1FwTccp(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK4.bit.TCCP;
}

/** \brief Get CCP and times selection Register
 * 
 *  \return uint16_t CCP and times selection Register
 */
uint16_t TLE9563_getCcpBlankHb2FwCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK5.reg;
}

/** \brief Set CCP and times selection Register
 * 
 *  \param u16_value CCP and times selection Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCcpBlankHb2FwCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK5;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Blank time
 * 
 *  \param e_value Blank time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2FwTblank(tBDRV_tBlank e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CCP_BLK_BNK5.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CCP_BLK_BNK5_TBLANK_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CCP_BLK_BNK5_TBLANK_Pos) & TLE9563_CCP_BLK_BNK5_TBLANK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK5;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Blank time
 * 
 *  \return uint16_t Blank time
 */
uint16_t TLE9563_getBdrvHb2FwTblank(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK5.bit.TBLANK;
}

/** \brief Set Cross-current protection time
 * 
 *  \param e_value Cross-current protection time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2FwTccp(tBDRV_tCcp e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CCP_BLK_BNK5.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CCP_BLK_BNK5_TCCP_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CCP_BLK_BNK5_TCCP_Pos) & TLE9563_CCP_BLK_BNK5_TCCP_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK5;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Cross-current protection time
 * 
 *  \return uint16_t Cross-current protection time
 */
uint16_t TLE9563_getBdrvHb2FwTccp(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK5.bit.TCCP;
}

/** \brief Get CCP and times selection Register
 * 
 *  \return uint16_t CCP and times selection Register
 */
uint16_t TLE9563_getCcpBlankHb3FwCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK6.reg;
}

/** \brief Set CCP and times selection Register
 * 
 *  \param u16_value CCP and times selection Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCcpBlankHb3FwCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK6;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Blank time
 * 
 *  \param e_value Blank time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3FwTblank(tBDRV_tBlank e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CCP_BLK_BNK6.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CCP_BLK_BNK6_TBLANK_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CCP_BLK_BNK6_TBLANK_Pos) & TLE9563_CCP_BLK_BNK6_TBLANK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK6;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Blank time
 * 
 *  \return uint16_t Blank time
 */
uint16_t TLE9563_getBdrvHb3FwTblank(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK6.bit.TBLANK;
}

/** \brief Set Cross-current protection time
 * 
 *  \param e_value Cross-current protection time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3FwTccp(tBDRV_tCcp e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->CCP_BLK_BNK6.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_CCP_BLK_BNK6_TCCP_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_CCP_BLK_BNK6_TCCP_Pos) & TLE9563_CCP_BLK_BNK6_TCCP_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setCCPBLKBNK6;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Cross-current protection time
 * 
 *  \return uint16_t Cross-current protection time
 */
uint16_t TLE9563_getBdrvHb3FwTccp(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->CCP_BLK_BNK6.bit.TCCP;
}

/** \brief Get Half-Bridge MODE Register
 * 
 *  \return uint16_t Half-Bridge MODE Register
 */
uint16_t TLE9563_getHbModeCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HBMODE.reg;
}

/** \brief Set Half-Bridge MODE Register
 * 
 *  \param u16_value Half-Bridge MODE Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setHbModeCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Half-bridge 3 MODE selection
 * 
 *  \param e_value Half-bridge 3 MODE selection
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3Mode(tBDRV_hb3Mode e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_HB3MODE_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HBMODE_HB3MODE_Pos) & TLE9563_HBMODE_HB3MODE_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Half-bridge 3 MODE selection
 * 
 *  \return uint16_t Half-bridge 3 MODE selection
 */
uint16_t TLE9563_getBdrvHb3Mode(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HBMODE.bit.HB3MODE;
}

/** \brief Enable Active freewheeling for half-bridge 3 during PWM
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvHb3ActiveFreeWheeling(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_AFW3_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_HBMODE_AFW3_Pos) & TLE9563_HBMODE_AFW3_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Active freewheeling for half-bridge 3 during PWM
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvHb3ActiveFreeWheeling(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_AFW3_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_HBMODE_AFW3_Pos) & TLE9563_HBMODE_AFW3_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Active freewheeling for half-bridge 3 during PWM Enable/Disable Setting
 * 
 *  \return  Active freewheeling for half-bridge 3 during PWM Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvHb3ActiveFreeWheeling(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HBMODE.bit.AFW3;
}

/** \brief Enable PWM mode for half-bridge 3
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvHb3PwmMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_HB3_PWM_EN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_HBMODE_HB3_PWM_EN_Pos) & TLE9563_HBMODE_HB3_PWM_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable PWM mode for half-bridge 3
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvHb3PwmMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_HB3_PWM_EN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_HBMODE_HB3_PWM_EN_Pos) & TLE9563_HBMODE_HB3_PWM_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get PWM mode for half-bridge 3 Enable/Disable Setting
 * 
 *  \return  PWM mode for half-bridge 3 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvHb3PwmMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HBMODE.bit.HB3_PWM_EN;
}

/** \brief Set Half-bridge 2 MODE selection
 * 
 *  \param e_value Half-bridge 2 MODE selection
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2Mode(tBDRV_hb2Mode e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_HB2MODE_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HBMODE_HB2MODE_Pos) & TLE9563_HBMODE_HB2MODE_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Half-bridge 2 MODE selection
 * 
 *  \return uint16_t Half-bridge 2 MODE selection
 */
uint16_t TLE9563_getBdrvHb2Mode(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HBMODE.bit.HB2MODE;
}

/** \brief Enable Active freewheeling for half-bridge 2 during PWM
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvHb2ActiveFreeWheeling(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_AFW2_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_HBMODE_AFW2_Pos) & TLE9563_HBMODE_AFW2_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Active freewheeling for half-bridge 2 during PWM
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvHb2ActiveFreeWheeling(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_AFW2_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_HBMODE_AFW2_Pos) & TLE9563_HBMODE_AFW2_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Active freewheeling for half-bridge 2 during PWM Enable/Disable Setting
 * 
 *  \return  Active freewheeling for half-bridge 2 during PWM Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvHb2ActiveFreeWheeling(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HBMODE.bit.AFW2;
}

/** \brief Enable PWM mode for half-bridge 2
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvHb2PwmMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_HB2_PWM_EN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_HBMODE_HB2_PWM_EN_Pos) & TLE9563_HBMODE_HB2_PWM_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable PWM mode for half-bridge 2
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvHb2PwmMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_HB2_PWM_EN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_HBMODE_HB2_PWM_EN_Pos) & TLE9563_HBMODE_HB2_PWM_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get PWM mode for half-bridge 2 Enable/Disable Setting
 * 
 *  \return  PWM mode for half-bridge 2 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvHb2PwmMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HBMODE.bit.HB2_PWM_EN;
}

/** \brief Set Half-bridge 1 MODE selection
 * 
 *  \param e_value Half-bridge 1 MODE selection
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1Mode(tBDRV_hb1Mode e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_HB1MODE_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HBMODE_HB1MODE_Pos) & TLE9563_HBMODE_HB1MODE_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Half-bridge 1 MODE selection
 * 
 *  \return uint16_t Half-bridge 1 MODE selection
 */
uint16_t TLE9563_getBdrvHb1Mode(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HBMODE.bit.HB1MODE;
}

/** \brief Enable Active freewheeling for half-bridge 1 during PWM
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvHb1ActiveFreeWheeling(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_AFW1_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_HBMODE_AFW1_Pos) & TLE9563_HBMODE_AFW1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Active freewheeling for half-bridge 1 during PWM
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvHb1ActiveFreeWheeling(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_AFW1_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_HBMODE_AFW1_Pos) & TLE9563_HBMODE_AFW1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Active freewheeling for half-bridge 1 during PWM Enable/Disable Setting
 * 
 *  \return  Active freewheeling for half-bridge 1 during PWM Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvHb1ActiveFreeWheeling(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HBMODE.bit.AFW1;
}

/** \brief Enable PWM mode for half-bridge 1
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvHb1PwmMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_HB1_PWM_EN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_HBMODE_HB1_PWM_EN_Pos) & TLE9563_HBMODE_HB1_PWM_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable PWM mode for half-bridge 1
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvHb1PwmMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HBMODE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HBMODE_HB1_PWM_EN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_HBMODE_HB1_PWM_EN_Pos) & TLE9563_HBMODE_HB1_PWM_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBMODE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get PWM mode for half-bridge 1 Enable/Disable Setting
 * 
 *  \return  PWM mode for half-bridge 1 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvHb1PwmMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HBMODE.bit.HB1_PWM_EN;
}

/** \brief Get HB pre-charge and pre-discharge time Register
 * 
 *  \return uint16_t HB pre-charge and pre-discharge time Register
 */
uint16_t TLE9563_getTpreChgCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TPRECHG_BNK0.reg;
}

/** \brief Set HB pre-charge and pre-discharge time Register
 * 
 *  \param u16_value HB pre-charge and pre-discharge time Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTpreChgCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setTPRECHGBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set If TPCHG_BNK=0 precharge time of HB 3, If TPCHG_BNK=1 predischarge time of HB 3
 * 
 *  \param e_value If TPCHG_BNK=0 precharge time of HB 3, If TPCHG_BNK=1 predischarge time of HB 3
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3PreChargeTime(tBDRV_preChargeDischargeTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TPRECHG_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TPRECHG_BNK0_TPCHG3_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TPRECHG_BNK0_TPCHG3_Pos) & TLE9563_TPRECHG_BNK0_TPCHG3_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTPRECHGBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If TPCHG_BNK=0 precharge time of HB 3, If TPCHG_BNK=1 predischarge time of HB 3
 * 
 *  \return uint16_t If TPCHG_BNK=0 precharge time of HB 3, If TPCHG_BNK=1 predischarge time of HB 3
 */
uint16_t TLE9563_getBdrvHb3PreChargeTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TPRECHG_BNK0.bit.TPCHG3;
}

/** \brief Set If TPCHG_BNK=0 precharge time of HB 2, If TPCHG_BNK=1 predischarge time of HB 2
 * 
 *  \param e_value If TPCHG_BNK=0 precharge time of HB 2, If TPCHG_BNK=1 predischarge time of HB 2
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2PreChargeTime(tBDRV_preChargeDischargeTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TPRECHG_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TPRECHG_BNK0_TPCHG2_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TPRECHG_BNK0_TPCHG2_Pos) & TLE9563_TPRECHG_BNK0_TPCHG2_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTPRECHGBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If TPCHG_BNK=0 precharge time of HB 2, If TPCHG_BNK=1 predischarge time of HB 2
 * 
 *  \return uint16_t If TPCHG_BNK=0 precharge time of HB 2, If TPCHG_BNK=1 predischarge time of HB 2
 */
uint16_t TLE9563_getBdrvHb2PreChargeTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TPRECHG_BNK0.bit.TPCHG2;
}

/** \brief Set If TPCHG_BNK=0 precharge time of HB 1, If TPCHG_BNK=1 predischarge time of HB 1
 * 
 *  \param e_value If TPCHG_BNK=0 precharge time of HB 1, If TPCHG_BNK=1 predischarge time of HB 1
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1PreChargeTime(tBDRV_preChargeDischargeTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TPRECHG_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TPRECHG_BNK0_TPCHG1_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TPRECHG_BNK0_TPCHG1_Pos) & TLE9563_TPRECHG_BNK0_TPCHG1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTPRECHGBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If TPCHG_BNK=0 precharge time of HB 1, If TPCHG_BNK=1 predischarge time of HB 1
 * 
 *  \return uint16_t If TPCHG_BNK=0 precharge time of HB 1, If TPCHG_BNK=1 predischarge time of HB 1
 */
uint16_t TLE9563_getBdrvHb1PreChargeTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TPRECHG_BNK0.bit.TPCHG1;
}

/** \brief Get HB pre-charge and pre-discharge time Register
 * 
 *  \return uint16_t HB pre-charge and pre-discharge time Register
 */
uint16_t TLE9563_getTpreDischgCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TPRECHG_BNK1.reg;
}

/** \brief Set HB pre-charge and pre-discharge time Register
 * 
 *  \param u16_value HB pre-charge and pre-discharge time Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTpreDischgCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setTPRECHGBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set If TPCHG_BNK=0 precharge time of HB 3, If TPCHG_BNK=1 predischarge time of HB 3
 * 
 *  \param e_value If TPCHG_BNK=0 precharge time of HB 3, If TPCHG_BNK=1 predischarge time of HB 3
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3PreDischargeTime(tBDRV_preChargeDischargeTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TPRECHG_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TPRECHG_BNK1_TPCHG3_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TPRECHG_BNK1_TPCHG3_Pos) & TLE9563_TPRECHG_BNK1_TPCHG3_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTPRECHGBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If TPCHG_BNK=0 precharge time of HB 3, If TPCHG_BNK=1 predischarge time of HB 3
 * 
 *  \return uint16_t If TPCHG_BNK=0 precharge time of HB 3, If TPCHG_BNK=1 predischarge time of HB 3
 */
uint16_t TLE9563_getBdrvHb3PreDischargeTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TPRECHG_BNK1.bit.TPCHG3;
}

/** \brief Set If TPCHG_BNK=0 precharge time of HB 2, If TPCHG_BNK=1 predischarge time of HB 2
 * 
 *  \param e_value If TPCHG_BNK=0 precharge time of HB 2, If TPCHG_BNK=1 predischarge time of HB 2
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2PreDischargeTime(tBDRV_preChargeDischargeTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TPRECHG_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TPRECHG_BNK1_TPCHG2_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TPRECHG_BNK1_TPCHG2_Pos) & TLE9563_TPRECHG_BNK1_TPCHG2_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTPRECHGBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If TPCHG_BNK=0 precharge time of HB 2, If TPCHG_BNK=1 predischarge time of HB 2
 * 
 *  \return uint16_t If TPCHG_BNK=0 precharge time of HB 2, If TPCHG_BNK=1 predischarge time of HB 2
 */
uint16_t TLE9563_getBdrvHb2PreDischargeTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TPRECHG_BNK1.bit.TPCHG2;
}

/** \brief Set If TPCHG_BNK=0 precharge time of HB 1, If TPCHG_BNK=1 predischarge time of HB 1
 * 
 *  \param e_value If TPCHG_BNK=0 precharge time of HB 1, If TPCHG_BNK=1 predischarge time of HB 1
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1PreDischargeTime(tBDRV_preChargeDischargeTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TPRECHG_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TPRECHG_BNK1_TPCHG1_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TPRECHG_BNK1_TPCHG1_Pos) & TLE9563_TPRECHG_BNK1_TPCHG1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTPRECHGBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If TPCHG_BNK=0 precharge time of HB 1, If TPCHG_BNK=1 predischarge time of HB 1
 * 
 *  \return uint16_t If TPCHG_BNK=0 precharge time of HB 1, If TPCHG_BNK=1 predischarge time of HB 1
 */
uint16_t TLE9563_getBdrvHb1PreDischargeTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TPRECHG_BNK1.bit.TPCHG1;
}

/** \brief Get Static charge/discharge current Register
 * 
 *  \return uint16_t Static charge/discharge current Register
 */
uint16_t TLE9563_getStIchgCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->ST_ICHG.reg;
}

/** \brief Set Static charge/discharge current Register
 * 
 *  \param u16_value Static charge/discharge current Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setStIchgCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSTICHG;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Static charge and discharge currents of HB3
 * 
 *  \param e_value Static charge and discharge currents of HB3
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3StaticChargeDischargeCur(tBDRV_staticChargeDischargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->ST_ICHG.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_ST_ICHG_ICHGST3_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_ST_ICHG_ICHGST3_Pos) & TLE9563_ST_ICHG_ICHGST3_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSTICHG;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Static charge and discharge currents of HB3
 * 
 *  \return uint16_t Static charge and discharge currents of HB3
 */
uint16_t TLE9563_getBdrvHb3StaticChargeDischargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->ST_ICHG.bit.ICHGST3;
}

/** \brief Set Static charge and discharge currents of HB2
 * 
 *  \param e_value Static charge and discharge currents of HB2
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2StaticChargeDischargeCur(tBDRV_staticChargeDischargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->ST_ICHG.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_ST_ICHG_ICHGST2_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_ST_ICHG_ICHGST2_Pos) & TLE9563_ST_ICHG_ICHGST2_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSTICHG;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Static charge and discharge currents of HB2
 * 
 *  \return uint16_t Static charge and discharge currents of HB2
 */
uint16_t TLE9563_getBdrvHb2StaticChargeDischargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->ST_ICHG.bit.ICHGST2;
}

/** \brief Set Static charge and discharge currents of HB1
 * 
 *  \param e_value Static charge and discharge currents of HB1
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1StaticChargeDischargeCur(tBDRV_staticChargeDischargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->ST_ICHG.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_ST_ICHG_ICHGST1_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_ST_ICHG_ICHGST1_Pos) & TLE9563_ST_ICHG_ICHGST1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSTICHG;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Static charge and discharge currents of HB1
 * 
 *  \return uint16_t Static charge and discharge currents of HB1
 */
uint16_t TLE9563_getBdrvHb1StaticChargeDischargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->ST_ICHG.bit.ICHGST1;
}

/** \brief Get HB charge/discharge currents for PWM operation Register
 * 
 *  \return uint16_t HB charge/discharge currents for PWM operation Register
 */
uint16_t TLE9563_getIchgHb1ActCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK0.reg;
}

/** \brief Set HB charge/discharge currents for PWM operation Register
 * 
 *  \param u16_value HB charge/discharge currents for PWM operation Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setIchgHb1ActCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 * 
 *  \param e_value If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1ActDischargeCur(tBDRV_dischargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_BNK0_IDCHG_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_ICHG_BNK0_IDCHG_Pos) & TLE9563_HB_ICHG_BNK0_IDCHG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 * 
 *  \return uint16_t If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 */
uint16_t TLE9563_getBdrvHb1ActDischargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK0.bit.IDCHG;
}

/** \brief Set If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \param e_value If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1ActChargeCur(tBDRV_chargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_BNK0_ICHG_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_ICHG_BNK0_ICHG_Pos) & TLE9563_HB_ICHG_BNK0_ICHG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \return uint16_t If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 */
uint16_t TLE9563_getBdrvHb1ActChargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK0.bit.ICHG;
}

/** \brief Get HB charge/discharge currents for PWM operation Register
 * 
 *  \return uint16_t HB charge/discharge currents for PWM operation Register
 */
uint16_t TLE9563_getIchgHb2ActCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK1.reg;
}

/** \brief Set HB charge/discharge currents for PWM operation Register
 * 
 *  \param u16_value HB charge/discharge currents for PWM operation Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setIchgHb2ActCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 * 
 *  \param e_value If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2ActDischargeCur(tBDRV_dischargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_BNK1_IDCHG_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_ICHG_BNK1_IDCHG_Pos) & TLE9563_HB_ICHG_BNK1_IDCHG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 * 
 *  \return uint16_t If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 */
uint16_t TLE9563_getBdrvHb2ActDischargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK1.bit.IDCHG;
}

/** \brief Set If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \param e_value If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2ActChargeCur(tBDRV_chargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_BNK1_ICHG_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_ICHG_BNK1_ICHG_Pos) & TLE9563_HB_ICHG_BNK1_ICHG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \return uint16_t If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 */
uint16_t TLE9563_getBdrvHb2ActChargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK1.bit.ICHG;
}

/** \brief Get HB charge/discharge currents for PWM operation Register
 * 
 *  \return uint16_t HB charge/discharge currents for PWM operation Register
 */
uint16_t TLE9563_getIchgHb3ActCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK2.reg;
}

/** \brief Set HB charge/discharge currents for PWM operation Register
 * 
 *  \param u16_value HB charge/discharge currents for PWM operation Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setIchgHb3ActCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 * 
 *  \param e_value If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3ActDischargeCur(tBDRV_dischargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_BNK2.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_BNK2_IDCHG_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_ICHG_BNK2_IDCHG_Pos) & TLE9563_HB_ICHG_BNK2_IDCHG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 * 
 *  \return uint16_t If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 */
uint16_t TLE9563_getBdrvHb3ActDischargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK2.bit.IDCHG;
}

/** \brief Set If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \param e_value If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3ActChargeCur(tBDRV_chargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_BNK2.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_BNK2_ICHG_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_ICHG_BNK2_ICHG_Pos) & TLE9563_HB_ICHG_BNK2_ICHG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \return uint16_t If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 */
uint16_t TLE9563_getBdrvHb3ActChargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK2.bit.ICHG;
}

/** \brief Get HB charge/discharge currents for PWM operation Register
 * 
 *  \return uint16_t HB charge/discharge currents for PWM operation Register
 */
uint16_t TLE9563_getIchgHb1FwCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK4.reg;
}

/** \brief Set HB charge/discharge currents for PWM operation Register
 * 
 *  \param u16_value HB charge/discharge currents for PWM operation Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setIchgHb1FwCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK4;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \param e_value If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1FwChargeAndDischargeCur(tBDRV_chargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_BNK4.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_BNK4_ICHG_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_ICHG_BNK4_ICHG_Pos) & TLE9563_HB_ICHG_BNK4_ICHG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK4;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \return uint16_t If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 */
uint16_t TLE9563_getBdrvHb1FwChargeAndDischargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK4.bit.ICHG;
}

/** \brief Get HB charge/discharge currents for PWM operation Register
 * 
 *  \return uint16_t HB charge/discharge currents for PWM operation Register
 */
uint16_t TLE9563_getIchgHb2FwCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK5.reg;
}

/** \brief Set HB charge/discharge currents for PWM operation Register
 * 
 *  \param u16_value HB charge/discharge currents for PWM operation Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setIchgHb2FwCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK5;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \param e_value If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2FwChargeAndDischargeCur(tBDRV_chargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_BNK5.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_BNK5_ICHG_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_ICHG_BNK5_ICHG_Pos) & TLE9563_HB_ICHG_BNK5_ICHG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK5;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \return uint16_t If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 */
uint16_t TLE9563_getBdrvHb2FwChargeAndDischargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK5.bit.ICHG;
}

/** \brief Get HB charge/discharge currents for PWM operation Register
 * 
 *  \return uint16_t HB charge/discharge currents for PWM operation Register
 */
uint16_t TLE9563_getIchgHb3FwCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK6.reg;
}

/** \brief Set HB charge/discharge currents for PWM operation Register
 * 
 *  \param u16_value HB charge/discharge currents for PWM operation Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setIchgHb3FwCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK6;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \param e_value If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3FwChargeAndDischargeCur(tBDRV_chargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_BNK6.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_BNK6_ICHG_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_ICHG_BNK6_ICHG_Pos) & TLE9563_HB_ICHG_BNK6_ICHG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGBNK6;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 * 
 *  \return uint16_t If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 */
uint16_t TLE9563_getBdrvHb3FwChargeAndDischargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_BNK6.bit.ICHG;
}

/** \brief Get HB max. pre-charge/pre-discharge in PWM operation current and diagnostic pull-down Register
 * 
 *  \return uint16_t HB max. pre-charge/pre-discharge in PWM operation current and diagnostic pull-down Register
 */
uint16_t TLE9563_getIchgMaxCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_MAX.reg;
}

/** \brief Set HB max. pre-charge/pre-discharge in PWM operation current and diagnostic pull-down Register
 * 
 *  \param u16_value HB max. pre-charge/pre-discharge in PWM operation current and diagnostic pull-down Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setIchgMaxCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGMAX;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Enable Control of HB3 off-state current source and current sink
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvHb3DiagPullDown(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_MAX.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_MAX_HB3IDIAG_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_HB_ICHG_MAX_HB3IDIAG_Pos) & TLE9563_HB_ICHG_MAX_HB3IDIAG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGMAX;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Control of HB3 off-state current source and current sink
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvHb3DiagPullDown(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_MAX.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_MAX_HB3IDIAG_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_HB_ICHG_MAX_HB3IDIAG_Pos) & TLE9563_HB_ICHG_MAX_HB3IDIAG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGMAX;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Control of HB3 off-state current source and current sink Enable/Disable Setting
 * 
 *  \return  Control of HB3 off-state current source and current sink Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvHb3DiagPullDown(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HB_ICHG_MAX.bit.HB3IDIAG;
}

/** \brief Enable Control of HB2 pull-down for off-state diagnostic
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvHb2DiagPullDown(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_MAX.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_MAX_HB2IDIAG_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_HB_ICHG_MAX_HB2IDIAG_Pos) & TLE9563_HB_ICHG_MAX_HB2IDIAG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGMAX;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Control of HB2 pull-down for off-state diagnostic
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvHb2DiagPullDown(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_MAX.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_MAX_HB2IDIAG_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_HB_ICHG_MAX_HB2IDIAG_Pos) & TLE9563_HB_ICHG_MAX_HB2IDIAG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGMAX;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Control of HB2 pull-down for off-state diagnostic Enable/Disable Setting
 * 
 *  \return  Control of HB2 pull-down for off-state diagnostic Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvHb2DiagPullDown(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HB_ICHG_MAX.bit.HB2IDIAG;
}

/** \brief Enable Control of HB1 pull-down for off-state diagnostic
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enBdrvHb1DiagPullDown(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_MAX.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_MAX_HB1IDIAG_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_HB_ICHG_MAX_HB1IDIAG_Pos) & TLE9563_HB_ICHG_MAX_HB1IDIAG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGMAX;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Control of HB1 pull-down for off-state diagnostic
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disBdrvHb1DiagPullDown(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_MAX.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_MAX_HB1IDIAG_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_HB_ICHG_MAX_HB1IDIAG_Pos) & TLE9563_HB_ICHG_MAX_HB1IDIAG_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGMAX;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Control of HB1 pull-down for off-state diagnostic Enable/Disable Setting
 * 
 *  \return  Control of HB1 pull-down for off-state diagnostic Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getBdrvHb1DiagPullDown(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HB_ICHG_MAX.bit.HB1IDIAG;
}

/** \brief Set Maximum drive current of HB3 during the pre-charge and pre-discharge phases1)
 * 
 *  \param e_value Maximum drive current of HB3 during the pre-charge and pre-discharge phases1)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3MaxChargeCur(tBDRV_maxChargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_MAX.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_MAX_ICHGMAX3_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_ICHG_MAX_ICHGMAX3_Pos) & TLE9563_HB_ICHG_MAX_ICHGMAX3_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGMAX;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Maximum drive current of HB3 during the pre-charge and pre-discharge phases1)
 * 
 *  \return uint16_t Maximum drive current of HB3 during the pre-charge and pre-discharge phases1)
 */
uint16_t TLE9563_getBdrvHb3MaxChargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_MAX.bit.ICHGMAX3;
}

/** \brief Set Maximum drive current of HB2 during the pre-charge phase and pre-discharge phases1)
 * 
 *  \param e_value Maximum drive current of HB2 during the pre-charge phase and pre-discharge phases1)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2MaxChargeCur(tBDRV_maxChargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_MAX.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_MAX_ICHGMAX2_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_ICHG_MAX_ICHGMAX2_Pos) & TLE9563_HB_ICHG_MAX_ICHGMAX2_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGMAX;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Maximum drive current of HB2 during the pre-charge phase and pre-discharge phases1)
 * 
 *  \return uint16_t Maximum drive current of HB2 during the pre-charge phase and pre-discharge phases1)
 */
uint16_t TLE9563_getBdrvHb2MaxChargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_MAX.bit.ICHGMAX2;
}

/** \brief Set Maximum drive current of HB1 during the pre-charge and pre-discharge phases1)
 * 
 *  \param e_value Maximum drive current of HB1 during the pre-charge and pre-discharge phases1)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1MaxChargeCur(tBDRV_maxChargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_ICHG_MAX.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_ICHG_MAX_ICHGMAX1_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_ICHG_MAX_ICHGMAX1_Pos) & TLE9563_HB_ICHG_MAX_ICHGMAX1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBICHGMAX;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Maximum drive current of HB1 during the pre-charge and pre-discharge phases1)
 * 
 *  \return uint16_t Maximum drive current of HB1 during the pre-charge and pre-discharge phases1)
 */
uint16_t TLE9563_getBdrvHb1MaxChargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_ICHG_MAX.bit.ICHGMAX1;
}

/** \brief Get HBx pre-charge/pre-discharge initialization configuration in PWM operation Register
 * 
 *  \return uint16_t HBx pre-charge/pre-discharge initialization configuration in PWM operation Register
 */
uint16_t TLE9563_getPreChgInitHb1CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_PCHG_INIT_BNK0.reg;
}

/** \brief Set HBx pre-charge/pre-discharge initialization configuration in PWM operation Register
 * 
 *  \param u16_value HBx pre-charge/pre-discharge initialization configuration in PWM operation Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPreChgInitHb1CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHBPCHGINITBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Initial predischarge current of HBx, IPDCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001111B
 * 
 *  \param e_value Initial predischarge current of HBx, IPDCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001111B
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1InitPreDischargeCur(tBDRV_dischargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_PCHG_INIT_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_PCHG_INIT_BNK0_PDCHGINIT_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_PCHG_INIT_BNK0_PDCHGINIT_Pos) & TLE9563_HB_PCHG_INIT_BNK0_PDCHGINIT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBPCHGINITBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Initial predischarge current of HBx, IPDCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001111B
 * 
 *  \return uint16_t Initial predischarge current of HBx, IPDCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001111B
 */
uint16_t TLE9563_getBdrvHb1InitPreDischargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_PCHG_INIT_BNK0.bit.PDCHGINIT;
}

/** \brief Set Initial precharge current of HBx, IPCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001101B Refer to Table 34
 * 
 *  \param e_value Initial precharge current of HBx, IPCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001101B Refer to Table 34
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1InitPreChargeCur(tBDRV_chargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_PCHG_INIT_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_PCHG_INIT_BNK0_PCHGINIT_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_PCHG_INIT_BNK0_PCHGINIT_Pos) & TLE9563_HB_PCHG_INIT_BNK0_PCHGINIT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBPCHGINITBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Initial precharge current of HBx, IPCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001101B Refer to Table 34
 * 
 *  \return uint16_t Initial precharge current of HBx, IPCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001101B Refer to Table 34
 */
uint16_t TLE9563_getBdrvHb1InitPreChargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_PCHG_INIT_BNK0.bit.PCHGINIT;
}

/** \brief Get HBx pre-charge/pre-discharge initialization configuration in PWM operation Register
 * 
 *  \return uint16_t HBx pre-charge/pre-discharge initialization configuration in PWM operation Register
 */
uint16_t TLE9563_getPreChgInitHb2CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_PCHG_INIT_BNK1.reg;
}

/** \brief Set HBx pre-charge/pre-discharge initialization configuration in PWM operation Register
 * 
 *  \param u16_value HBx pre-charge/pre-discharge initialization configuration in PWM operation Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPreChgInitHb2CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHBPCHGINITBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Initial predischarge current of HBx, IPDCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001111B
 * 
 *  \param e_value Initial predischarge current of HBx, IPDCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001111B
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2InitPreDischargeCur(tBDRV_dischargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_PCHG_INIT_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_PCHG_INIT_BNK1_PDCHGINIT_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_PCHG_INIT_BNK1_PDCHGINIT_Pos) & TLE9563_HB_PCHG_INIT_BNK1_PDCHGINIT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBPCHGINITBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Initial predischarge current of HBx, IPDCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001111B
 * 
 *  \return uint16_t Initial predischarge current of HBx, IPDCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001111B
 */
uint16_t TLE9563_getBdrvHb2InitPreDischargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_PCHG_INIT_BNK1.bit.PDCHGINIT;
}

/** \brief Set Initial precharge current of HBx, IPCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001101B Refer to Table 34
 * 
 *  \param e_value Initial precharge current of HBx, IPCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001101B Refer to Table 34
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2InitPreChargeCur(tBDRV_chargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_PCHG_INIT_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_PCHG_INIT_BNK1_PCHGINIT_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_PCHG_INIT_BNK1_PCHGINIT_Pos) & TLE9563_HB_PCHG_INIT_BNK1_PCHGINIT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBPCHGINITBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Initial precharge current of HBx, IPCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001101B Refer to Table 34
 * 
 *  \return uint16_t Initial precharge current of HBx, IPCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001101B Refer to Table 34
 */
uint16_t TLE9563_getBdrvHb2InitPreChargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_PCHG_INIT_BNK1.bit.PCHGINIT;
}

/** \brief Get HBx pre-charge/pre-discharge initialization configuration in PWM operation Register
 * 
 *  \return uint16_t HBx pre-charge/pre-discharge initialization configuration in PWM operation Register
 */
uint16_t TLE9563_getPreChgInitHb3CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_PCHG_INIT_BNK2.reg;
}

/** \brief Set HBx pre-charge/pre-discharge initialization configuration in PWM operation Register
 * 
 *  \param u16_value HBx pre-charge/pre-discharge initialization configuration in PWM operation Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setPreChgInitHb3CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setHBPCHGINITBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Initial predischarge current of HBx, IPDCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001111B
 * 
 *  \param e_value Initial predischarge current of HBx, IPDCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001111B
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3InitPreDischargeCur(tBDRV_dischargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_PCHG_INIT_BNK2.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_PCHG_INIT_BNK2_PDCHGINIT_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_PCHG_INIT_BNK2_PDCHGINIT_Pos) & TLE9563_HB_PCHG_INIT_BNK2_PDCHGINIT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBPCHGINITBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Initial predischarge current of HBx, IPDCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001111B
 * 
 *  \return uint16_t Initial predischarge current of HBx, IPDCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001111B
 */
uint16_t TLE9563_getBdrvHb3InitPreDischargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_PCHG_INIT_BNK2.bit.PDCHGINIT;
}

/** \brief Set Initial precharge current of HBx, IPCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001101B Refer to Table 34
 * 
 *  \param e_value Initial precharge current of HBx, IPCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001101B Refer to Table 34
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3InitPreChargeCur(tBDRV_chargeCur e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->HB_PCHG_INIT_BNK2.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_HB_PCHG_INIT_BNK2_PCHGINIT_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_HB_PCHG_INIT_BNK2_PCHGINIT_Pos) & TLE9563_HB_PCHG_INIT_BNK2_PCHGINIT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setHBPCHGINITBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Initial precharge current of HBx, IPCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001101B Refer to Table 34
 * 
 *  \return uint16_t Initial precharge current of HBx, IPCHGINITx The INIT_BNK bits select the addressed half-bridge Default 001101B Refer to Table 34
 */
uint16_t TLE9563_getBdrvHb3InitPreChargeCur(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HB_PCHG_INIT_BNK2.bit.PCHGINIT;
}

/** \brief Get HBx inputs TDON configuration Register
 * 
 *  \return uint16_t HBx inputs TDON configuration Register
 */
uint16_t TLE9563_getTdonHb1CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDON_HB_CTRL_BNK0.reg;
}

/** \brief Set HBx inputs TDON configuration Register
 * 
 *  \param u16_value HBx inputs TDON configuration Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTdonHb1CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setTDONHBCTRLBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Turn-on delay time of active MOSFET of HBx
 * 
 *  \param e_value Turn-on delay time of active MOSFET of HBx
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1Tdon(tBDRV_tdon e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TDON_HB_CTRL_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TDON_HB_CTRL_BNK0_TDON_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TDON_HB_CTRL_BNK0_TDON_Pos) & TLE9563_TDON_HB_CTRL_BNK0_TDON_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTDONHBCTRLBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Turn-on delay time of active MOSFET of HBx
 * 
 *  \return uint16_t Turn-on delay time of active MOSFET of HBx
 */
uint16_t TLE9563_getBdrvHb1Tdon(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDON_HB_CTRL_BNK0.bit.TDON;
}

/** \brief Get HBx inputs TDON configuration Register
 * 
 *  \return uint16_t HBx inputs TDON configuration Register
 */
uint16_t TLE9563_getTdonHb2CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDON_HB_CTRL_BNK1.reg;
}

/** \brief Set HBx inputs TDON configuration Register
 * 
 *  \param u16_value HBx inputs TDON configuration Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTdonHb2CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setTDONHBCTRLBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Turn-on delay time of active MOSFET of HBx
 * 
 *  \param e_value Turn-on delay time of active MOSFET of HBx
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2Tdon(tBDRV_tdon e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TDON_HB_CTRL_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TDON_HB_CTRL_BNK1_TDON_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TDON_HB_CTRL_BNK1_TDON_Pos) & TLE9563_TDON_HB_CTRL_BNK1_TDON_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTDONHBCTRLBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Turn-on delay time of active MOSFET of HBx
 * 
 *  \return uint16_t Turn-on delay time of active MOSFET of HBx
 */
uint16_t TLE9563_getBdrvHb2Tdon(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDON_HB_CTRL_BNK1.bit.TDON;
}

/** \brief Get HBx inputs TDON configuration Register
 * 
 *  \return uint16_t HBx inputs TDON configuration Register
 */
uint16_t TLE9563_getTdonHb3CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDON_HB_CTRL_BNK2.reg;
}

/** \brief Set HBx inputs TDON configuration Register
 * 
 *  \param u16_value HBx inputs TDON configuration Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTdonHb3CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setTDONHBCTRLBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Turn-on delay time of active MOSFET of HBx
 * 
 *  \param e_value Turn-on delay time of active MOSFET of HBx
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3Tdon(tBDRV_tdon e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TDON_HB_CTRL_BNK2.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TDON_HB_CTRL_BNK2_TDON_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TDON_HB_CTRL_BNK2_TDON_Pos) & TLE9563_TDON_HB_CTRL_BNK2_TDON_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTDONHBCTRLBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Turn-on delay time of active MOSFET of HBx
 * 
 *  \return uint16_t Turn-on delay time of active MOSFET of HBx
 */
uint16_t TLE9563_getBdrvHb3Tdon(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDON_HB_CTRL_BNK2.bit.TDON;
}

/** \brief Get HBx TDOFF configuration Register
 * 
 *  \return uint16_t HBx TDOFF configuration Register
 */
uint16_t TLE9563_getTdoffHb1CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDOFF_HB_CTRL_BNK0.reg;
}

/** \brief Set HBx TDOFF configuration Register
 * 
 *  \param u16_value HBx TDOFF configuration Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTdoffHb1CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setTDOFFHBCTRLBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 * 
 *  \param e_value Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb1Tdoff(tBDRV_tdoff e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TDOFF_HB_CTRL_BNK0.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TDOFF_HB_CTRL_BNK0_TDOFF_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TDOFF_HB_CTRL_BNK0_TDOFF_Pos) & TLE9563_TDOFF_HB_CTRL_BNK0_TDOFF_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTDOFFHBCTRLBNK0;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 * 
 *  \return uint16_t Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 */
uint16_t TLE9563_getBdrvHb1Tdoff(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDOFF_HB_CTRL_BNK0.bit.TDOFF;
}

/** \brief Get HBx TDOFF configuration Register
 * 
 *  \return uint16_t HBx TDOFF configuration Register
 */
uint16_t TLE9563_getTdoffHb2CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDOFF_HB_CTRL_BNK1.reg;
}

/** \brief Set HBx TDOFF configuration Register
 * 
 *  \param u16_value HBx TDOFF configuration Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTdoffHb2CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setTDOFFHBCTRLBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 * 
 *  \param e_value Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb2Tdoff(tBDRV_tdoff e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TDOFF_HB_CTRL_BNK1.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TDOFF_HB_CTRL_BNK1_TDOFF_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TDOFF_HB_CTRL_BNK1_TDOFF_Pos) & TLE9563_TDOFF_HB_CTRL_BNK1_TDOFF_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTDOFFHBCTRLBNK1;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 * 
 *  \return uint16_t Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 */
uint16_t TLE9563_getBdrvHb2Tdoff(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDOFF_HB_CTRL_BNK1.bit.TDOFF;
}

/** \brief Get HBx TDOFF configuration Register
 * 
 *  \return uint16_t HBx TDOFF configuration Register
 */
uint16_t TLE9563_getTdoffHb3CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDOFF_HB_CTRL_BNK2.reg;
}

/** \brief Set HBx TDOFF configuration Register
 * 
 *  \param u16_value HBx TDOFF configuration Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setTdoffHb3CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setTDOFFHBCTRLBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 * 
 *  \param e_value Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBdrvHb3Tdoff(tBDRV_tdoff e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->TDOFF_HB_CTRL_BNK2.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_TDOFF_HB_CTRL_BNK2_TDOFF_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_TDOFF_HB_CTRL_BNK2_TDOFF_Pos) & TLE9563_TDOFF_HB_CTRL_BNK2_TDOFF_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setTDOFFHBCTRLBNK2;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 * 
 *  \return uint16_t Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 */
uint16_t TLE9563_getBdrvHb3Tdoff(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDOFF_HB_CTRL_BNK2.bit.TDOFF;
}

/** \brief Get Brake control Register
 * 
 *  \return uint16_t Brake control Register
 */
uint16_t TLE9563_getBrakeCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->BRAKE.reg;
}

/** \brief Set Brake control Register
 * 
 *  \param u16_value Brake control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setBrakeCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Enable LS3 output disable during SLAM mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enLs3CtrlInSlamMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_SLAM_LS3_DIS_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_BRAKE_SLAM_LS3_DIS_Pos) & TLE9563_BRAKE_SLAM_LS3_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable LS3 output disable during SLAM mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disLs3CtrlInSlamMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_SLAM_LS3_DIS_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_BRAKE_SLAM_LS3_DIS_Pos) & TLE9563_BRAKE_SLAM_LS3_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get LS3 output disable during SLAM mode Enable/Disable Setting
 * 
 *  \return  LS3 output disable during SLAM mode Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getLs3CtrlInSlamMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->BRAKE.bit.SLAM_LS3_DIS;
}

/** \brief Enable LS2 output disable during SLAM mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enLs2CtrlInSlamMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_SLAM_LS2_DIS_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_BRAKE_SLAM_LS2_DIS_Pos) & TLE9563_BRAKE_SLAM_LS2_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable LS2 output disable during SLAM mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disLs2CtrlInSlamMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_SLAM_LS2_DIS_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_BRAKE_SLAM_LS2_DIS_Pos) & TLE9563_BRAKE_SLAM_LS2_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get LS2 output disable during SLAM mode Enable/Disable Setting
 * 
 *  \return  LS2 output disable during SLAM mode Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getLs2CtrlInSlamMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->BRAKE.bit.SLAM_LS2_DIS;
}

/** \brief Enable LS1 output disable during SLAM mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enLs1CtrlInSlamMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_SLAM_LS1_DIS_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_BRAKE_SLAM_LS1_DIS_Pos) & TLE9563_BRAKE_SLAM_LS1_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable LS1 output disable during SLAM mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disLs1CtrlInSlamMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_SLAM_LS1_DIS_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_BRAKE_SLAM_LS1_DIS_Pos) & TLE9563_BRAKE_SLAM_LS1_DIS_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get LS1 output disable during SLAM mode Enable/Disable Setting
 * 
 *  \return  LS1 output disable during SLAM mode Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getLs1CtrlInSlamMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->BRAKE.bit.SLAM_LS1_DIS;
}

/** \brief Enable Slam mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSlamMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_SLAM_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_BRAKE_SLAM_Pos) & TLE9563_BRAKE_SLAM_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Slam mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSlamMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_SLAM_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_BRAKE_SLAM_Pos) & TLE9563_BRAKE_SLAM_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Slam mode Enable/Disable Setting
 * 
 *  \return  Slam mode Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSlamMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->BRAKE.bit.SLAM;
}

/** \brief Set VDS Overvoltage for LS1-3 during braking
 * 
 *  \param e_value VDS Overvoltage for LS1-3 during braking
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVdsOverVoltThreshLsBrake(tVDS_overVoltThreshLsBrake e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_VDSTH_BRK_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_BRAKE_VDSTH_BRK_Pos) & TLE9563_BRAKE_VDSTH_BRK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get VDS Overvoltage for LS1-3 during braking
 * 
 *  \return uint16_t VDS Overvoltage for LS1-3 during braking
 */
uint16_t TLE9563_getVdsOverVoltThreshLsBrake(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->BRAKE.bit.VDSTH_BRK;
}

/** \brief Set Blank time of VDS overvoltage during braking
 * 
 *  \param e_value Blank time of VDS overvoltage during braking
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVdsOverVoltBlankTimeBrake(tVDS_overVoltBlankTimeBrake e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_TBLK_BRK_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_BRAKE_TBLK_BRK_Pos) & TLE9563_BRAKE_TBLK_BRK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Blank time of VDS overvoltage during braking
 * 
 *  \return uint16_t Blank time of VDS overvoltage during braking
 */
uint16_t TLE9563_getVdsOverVoltBlankTimeBrake(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->BRAKE.bit.TBLK_BRK;
}

/** \brief Enable Parking brake enable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enParkingBrake(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_PARK_BRK_EN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_BRAKE_PARK_BRK_EN_Pos) & TLE9563_BRAKE_PARK_BRK_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Parking brake enable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disParkingBrake(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_PARK_BRK_EN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_BRAKE_PARK_BRK_EN_Pos) & TLE9563_BRAKE_PARK_BRK_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Parking brake enable Enable/Disable Setting
 * 
 *  \return  Parking brake enable Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getParkingBrake(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->BRAKE.bit.PARK_BRK_EN;
}

/** \brief Enable Overvoltage brake enable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enOverVoltBrake(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_OV_BRK_EN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_BRAKE_OV_BRK_EN_Pos) & TLE9563_BRAKE_OV_BRK_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Overvoltage brake enable
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disOverVoltBrake(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_OV_BRK_EN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_BRAKE_OV_BRK_EN_Pos) & TLE9563_BRAKE_OV_BRK_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Overvoltage brake enable Enable/Disable Setting
 * 
 *  \return  Overvoltage brake enable Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getOverVoltBrake(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->BRAKE.bit.OV_BRK_EN;
}

/** \brief Set Overvoltage brake threshold
 * 
 *  \param e_value Overvoltage brake threshold
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setVsOverVoltThreshBrake(tVS_overVoltThreshBrake e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->BRAKE.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_BRAKE_OV_BRK_TH_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_BRAKE_OV_BRK_TH_Pos) & TLE9563_BRAKE_OV_BRK_TH_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setBRAKE;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Overvoltage brake threshold
 * 
 *  \return uint16_t Overvoltage brake threshold
 */
uint16_t TLE9563_getVsOverVoltThreshBrake(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->BRAKE.bit.OV_BRK_TH;
}

/** \brief Get CAN Selective Wake Control Register
 * 
 *  \return uint16_t CAN Selective Wake Control Register
 */
uint16_t TLE9563_getCanSwkCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_CTRL.reg;
}

/** \brief Set CAN Selective Wake Control Register
 * 
 *  \param u16_value CAN Selective Wake Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCanSwkCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Enable Oscillator Calibration Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enCanSwkOscCalMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CTRL_OSC_CAL_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_CTRL_OSC_CAL_Pos) & TLE9563_SWK_CTRL_OSC_CAL_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Oscillator Calibration Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disCanSwkOscCalMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CTRL_OSC_CAL_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_CTRL_OSC_CAL_Pos) & TLE9563_SWK_CTRL_OSC_CAL_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Oscillator Calibration Mode Enable/Disable Setting
 * 
 *  \return  Oscillator Calibration Mode Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getCanSwkOscCalMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_CTRL.bit.OSC_CAL;
}

/** \brief Enable (Un)locking mechanism of oscillator recalibration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enCanSwkOscRecalLock(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CTRL_TRIM_EN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_CTRL_TRIM_EN_Pos) & TLE9563_SWK_CTRL_TRIM_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable (Un)locking mechanism of oscillator recalibration
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disCanSwkOscRecalLock(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CTRL_TRIM_EN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_CTRL_TRIM_EN_Pos) & TLE9563_SWK_CTRL_TRIM_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get (Un)locking mechanism of oscillator recalibration Enable/Disable Setting
 * 
 *  \return  (Un)locking mechanism of oscillator recalibration Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getCanSwkOscRecalLock(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_CTRL.bit.TRIM_EN;
}

/** \brief Enable CAN Time Out Masking
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enCanSwkTimeOutMask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CTRL_CANTO_MASK_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_CTRL_CANTO_MASK_Pos) & TLE9563_SWK_CTRL_CANTO_MASK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable CAN Time Out Masking
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disCanSwkTimeOutMask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CTRL_CANTO_MASK_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_CTRL_CANTO_MASK_Pos) & TLE9563_SWK_CTRL_CANTO_MASK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get CAN Time Out Masking Enable/Disable Setting
 * 
 *  \return  CAN Time Out Masking Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getCanSwkTimeOutMask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_CTRL.bit.CANTO_MASK;
}

/** \brief Enable SWK Configuration valid
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enCanSwkConfigValid(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CTRL_CFG_VAL_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_CTRL_CFG_VAL_Pos) & TLE9563_SWK_CTRL_CFG_VAL_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable SWK Configuration valid
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disCanSwkConfigValid(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CTRL_CFG_VAL_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_CTRL_CFG_VAL_Pos) & TLE9563_SWK_CTRL_CFG_VAL_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get SWK Configuration valid Enable/Disable Setting
 * 
 *  \return  SWK Configuration valid Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getCanSwkConfigValid(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_CTRL.bit.CFG_VAL;
}

/** \brief Get SWK Bit Timing Control Register
 * 
 *  \return uint16_t SWK Bit Timing Control Register
 */
uint16_t TLE9563_getSwkBtl1CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_BTL1_CTRL.reg;
}

/** \brief Set SWK Bit Timing Control Register
 * 
 *  \param u16_value SWK Bit Timing Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkBtl1CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKBTL1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Sampling Point Position
 * 
 *  \param u8_value Sampling Point Position
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkSamplingPointPos(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_BTL1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_BTL1_CTRL_SP_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_BTL1_CTRL_SP_Pos) & TLE9563_SWK_BTL1_CTRL_SP_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKBTL1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Sampling Point Position
 * 
 *  \return uint16_t Sampling Point Position
 */
uint16_t TLE9563_getSwkSamplingPointPos(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_BTL1_CTRL.bit.SP;
}

/** \brief Set Number of Time Quanta in a Bit Time
 * 
 *  \param u8_value Number of Time Quanta in a Bit Time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkNbTimeQuanta(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_BTL1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_BTL1_CTRL_TBIT_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_BTL1_CTRL_TBIT_Pos) & TLE9563_SWK_BTL1_CTRL_TBIT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKBTL1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Number of Time Quanta in a Bit Time
 * 
 *  \return uint16_t Number of Time Quanta in a Bit Time
 */
uint16_t TLE9563_getSwkNbTimeQuanta(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_BTL1_CTRL.bit.TBIT;
}

/** \brief Get SWK WUF Identifier bits 28...13 Register
 * 
 *  \return uint16_t SWK WUF Identifier bits 28...13 Register
 */
uint16_t TLE9563_getSwkId1CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
}

/** \brief Set SWK WUF Identifier bits 28...13 Register
 * 
 *  \param u16_value SWK WUF Identifier bits 28...13 Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkId1CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set WUF Identifier Bit 28
 * 
 *  \param u8_value WUF Identifier Bit 28
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit28(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID28_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID28_Pos) & TLE9563_SWK_ID1_CTRL_ID28_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 28
 * 
 *  \return uint16_t WUF Identifier Bit 28
 */
uint16_t TLE9563_getSwkIdBit28(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID28;
}

/** \brief Set WUF Identifier Bit 27
 * 
 *  \param u8_value WUF Identifier Bit 27
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit27(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID27_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID27_Pos) & TLE9563_SWK_ID1_CTRL_ID27_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 27
 * 
 *  \return uint16_t WUF Identifier Bit 27
 */
uint16_t TLE9563_getSwkIdBit27(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID27;
}

/** \brief Set WUF Identifier Bit 26
 * 
 *  \param u8_value WUF Identifier Bit 26
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit26(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID26_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID26_Pos) & TLE9563_SWK_ID1_CTRL_ID26_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 26
 * 
 *  \return uint16_t WUF Identifier Bit 26
 */
uint16_t TLE9563_getSwkIdBit26(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID26;
}

/** \brief Set WUF Identifier Bit 25
 * 
 *  \param u8_value WUF Identifier Bit 25
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit25(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID25_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID25_Pos) & TLE9563_SWK_ID1_CTRL_ID25_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 25
 * 
 *  \return uint16_t WUF Identifier Bit 25
 */
uint16_t TLE9563_getSwkIdBit25(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID25;
}

/** \brief Set WUF Identifier Bit 24
 * 
 *  \param u8_value WUF Identifier Bit 24
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit24(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID24_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID24_Pos) & TLE9563_SWK_ID1_CTRL_ID24_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 24
 * 
 *  \return uint16_t WUF Identifier Bit 24
 */
uint16_t TLE9563_getSwkIdBit24(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID24;
}

/** \brief Set WUF Identifier Bit 23
 * 
 *  \param u8_value WUF Identifier Bit 23
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit23(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID23_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID23_Pos) & TLE9563_SWK_ID1_CTRL_ID23_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 23
 * 
 *  \return uint16_t WUF Identifier Bit 23
 */
uint16_t TLE9563_getSwkIdBit23(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID23;
}

/** \brief Set WUF Identifier Bit 22
 * 
 *  \param u8_value WUF Identifier Bit 22
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit22(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID22_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID22_Pos) & TLE9563_SWK_ID1_CTRL_ID22_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 22
 * 
 *  \return uint16_t WUF Identifier Bit 22
 */
uint16_t TLE9563_getSwkIdBit22(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID22;
}

/** \brief Set WUF Identifier Bit 21
 * 
 *  \param u8_value WUF Identifier Bit 21
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit21(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID21_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID21_Pos) & TLE9563_SWK_ID1_CTRL_ID21_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 21
 * 
 *  \return uint16_t WUF Identifier Bit 21
 */
uint16_t TLE9563_getSwkIdBit21(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID21;
}

/** \brief Set WUF Identifier Bit 20
 * 
 *  \param u8_value WUF Identifier Bit 20
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit20(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID20_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID20_Pos) & TLE9563_SWK_ID1_CTRL_ID20_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 20
 * 
 *  \return uint16_t WUF Identifier Bit 20
 */
uint16_t TLE9563_getSwkIdBit20(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID20;
}

/** \brief Set WUF Identifier Bit 19
 * 
 *  \param u8_value WUF Identifier Bit 19
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit19(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID19_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID19_Pos) & TLE9563_SWK_ID1_CTRL_ID19_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 19
 * 
 *  \return uint16_t WUF Identifier Bit 19
 */
uint16_t TLE9563_getSwkIdBit19(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID19;
}

/** \brief Set WUF Identifier Bit 18
 * 
 *  \param u8_value WUF Identifier Bit 18
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit18(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID18_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID18_Pos) & TLE9563_SWK_ID1_CTRL_ID18_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 18
 * 
 *  \return uint16_t WUF Identifier Bit 18
 */
uint16_t TLE9563_getSwkIdBit18(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID18;
}

/** \brief Set WUF Identifier Bit 17
 * 
 *  \param u8_value WUF Identifier Bit 17
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit17(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID17_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID17_Pos) & TLE9563_SWK_ID1_CTRL_ID17_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 17
 * 
 *  \return uint16_t WUF Identifier Bit 17
 */
uint16_t TLE9563_getSwkIdBit17(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID17;
}

/** \brief Set WUF Identifier Bit 16
 * 
 *  \param u8_value WUF Identifier Bit 16
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit16(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID16_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID16_Pos) & TLE9563_SWK_ID1_CTRL_ID16_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 16
 * 
 *  \return uint16_t WUF Identifier Bit 16
 */
uint16_t TLE9563_getSwkIdBit16(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID16;
}

/** \brief Set WUF Identifier Bit 15
 * 
 *  \param u8_value WUF Identifier Bit 15
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit15(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID15_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID15_Pos) & TLE9563_SWK_ID1_CTRL_ID15_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 15
 * 
 *  \return uint16_t WUF Identifier Bit 15
 */
uint16_t TLE9563_getSwkIdBit15(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID15;
}

/** \brief Set WUF Identifier Bit 14
 * 
 *  \param u8_value WUF Identifier Bit 14
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit14(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID14_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID14_Pos) & TLE9563_SWK_ID1_CTRL_ID14_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 14
 * 
 *  \return uint16_t WUF Identifier Bit 14
 */
uint16_t TLE9563_getSwkIdBit14(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID14;
}

/** \brief Set WUF Identifier Bit 13
 * 
 *  \param u8_value WUF Identifier Bit 13
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit13(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID1_CTRL_ID13_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID1_CTRL_ID13_Pos) & TLE9563_SWK_ID1_CTRL_ID13_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 13
 * 
 *  \return uint16_t WUF Identifier Bit 13
 */
uint16_t TLE9563_getSwkIdBit13(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID1_CTRL.bit.ID13;
}

/** \brief Get SWK WUF Identifier bits 12...0 Register
 * 
 *  \return uint16_t SWK WUF Identifier bits 12...0 Register
 */
uint16_t TLE9563_getSwkId0CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
}

/** \brief Set SWK WUF Identifier bits 12...0 Register
 * 
 *  \param u16_value SWK WUF Identifier bits 12...0 Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkId0CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set WUF Identifier Bit 12
 * 
 *  \param u8_value WUF Identifier Bit 12
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit12(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID12_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID12_Pos) & TLE9563_SWK_ID0_CTRL_ID12_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 12
 * 
 *  \return uint16_t WUF Identifier Bit 12
 */
uint16_t TLE9563_getSwkIdBit12(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID12;
}

/** \brief Set WUF Identifier Bit 11
 * 
 *  \param u8_value WUF Identifier Bit 11
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit11(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID11_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID11_Pos) & TLE9563_SWK_ID0_CTRL_ID11_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 11
 * 
 *  \return uint16_t WUF Identifier Bit 11
 */
uint16_t TLE9563_getSwkIdBit11(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID11;
}

/** \brief Set WUF Identifier Bit 10
 * 
 *  \param u8_value WUF Identifier Bit 10
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit10(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID10_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID10_Pos) & TLE9563_SWK_ID0_CTRL_ID10_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 10
 * 
 *  \return uint16_t WUF Identifier Bit 10
 */
uint16_t TLE9563_getSwkIdBit10(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID10;
}

/** \brief Set WUF Identifier Bit 9
 * 
 *  \param u8_value WUF Identifier Bit 9
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit9(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID9_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID9_Pos) & TLE9563_SWK_ID0_CTRL_ID9_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 9
 * 
 *  \return uint16_t WUF Identifier Bit 9
 */
uint16_t TLE9563_getSwkIdBit9(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID9;
}

/** \brief Set WUF Identifier Bit 8
 * 
 *  \param u8_value WUF Identifier Bit 8
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit8(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID8_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID8_Pos) & TLE9563_SWK_ID0_CTRL_ID8_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 8
 * 
 *  \return uint16_t WUF Identifier Bit 8
 */
uint16_t TLE9563_getSwkIdBit8(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID8;
}

/** \brief Set WUF Identifier Bit 7
 * 
 *  \param u8_value WUF Identifier Bit 7
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit7(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID7_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID7_Pos) & TLE9563_SWK_ID0_CTRL_ID7_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 7
 * 
 *  \return uint16_t WUF Identifier Bit 7
 */
uint16_t TLE9563_getSwkIdBit7(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID7;
}

/** \brief Set WUF Identifier Bit 6
 * 
 *  \param u8_value WUF Identifier Bit 6
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit6(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID6_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID6_Pos) & TLE9563_SWK_ID0_CTRL_ID6_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 6
 * 
 *  \return uint16_t WUF Identifier Bit 6
 */
uint16_t TLE9563_getSwkIdBit6(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID6;
}

/** \brief Set WUF Identifier Bit 5
 * 
 *  \param u8_value WUF Identifier Bit 5
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit5(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID5_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID5_Pos) & TLE9563_SWK_ID0_CTRL_ID5_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 5
 * 
 *  \return uint16_t WUF Identifier Bit 5
 */
uint16_t TLE9563_getSwkIdBit5(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID5;
}

/** \brief Set WUF Identifier Bit 4
 * 
 *  \param u8_value WUF Identifier Bit 4
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit4(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID4_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID4_Pos) & TLE9563_SWK_ID0_CTRL_ID4_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 4
 * 
 *  \return uint16_t WUF Identifier Bit 4
 */
uint16_t TLE9563_getSwkIdBit4(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID4;
}

/** \brief Set WUF Identifier Bit 3
 * 
 *  \param u8_value WUF Identifier Bit 3
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit3(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID3_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID3_Pos) & TLE9563_SWK_ID0_CTRL_ID3_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 3
 * 
 *  \return uint16_t WUF Identifier Bit 3
 */
uint16_t TLE9563_getSwkIdBit3(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID3;
}

/** \brief Set WUF Identifier Bit 2
 * 
 *  \param u8_value WUF Identifier Bit 2
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit2(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID2_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID2_Pos) & TLE9563_SWK_ID0_CTRL_ID2_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 2
 * 
 *  \return uint16_t WUF Identifier Bit 2
 */
uint16_t TLE9563_getSwkIdBit2(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID2;
}

/** \brief Set WUF Identifier Bit 1
 * 
 *  \param u8_value WUF Identifier Bit 1
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit1(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID1_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID1_Pos) & TLE9563_SWK_ID0_CTRL_ID1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 1
 * 
 *  \return uint16_t WUF Identifier Bit 1
 */
uint16_t TLE9563_getSwkIdBit1(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID1;
}

/** \brief Set WUF Identifier Bit 0
 * 
 *  \param u8_value WUF Identifier Bit 0
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdBit0(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_ID0_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_ID0_CTRL_ID0_Pos) & TLE9563_SWK_ID0_CTRL_ID0_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get WUF Identifier Bit 0
 * 
 *  \return uint16_t WUF Identifier Bit 0
 */
uint16_t TLE9563_getSwkIdBit0(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.ID0;
}

/** \brief Set Remote Transmission Request Field (acc. ISO11898-22016)
 * 
 *  \param e_value Remote Transmission Request Field (acc. ISO11898-22016)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkRemoteTransReq(tSWK_remoteTransReq e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_RTR_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_SWK_ID0_CTRL_RTR_Pos) & TLE9563_SWK_ID0_CTRL_RTR_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Remote Transmission Request Field (acc. ISO11898-22016)
 * 
 *  \return uint16_t Remote Transmission Request Field (acc. ISO11898-22016)
 */
uint16_t TLE9563_getSwkRemoteTransReq(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.RTR;
}

/** \brief Set Identifier Extension Bit
 * 
 *  \param e_value Identifier Extension Bit
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkIdExtBit(tSWK_idExtBit e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_ID0_CTRL_IDE_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_SWK_ID0_CTRL_IDE_Pos) & TLE9563_SWK_ID0_CTRL_IDE_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Identifier Extension Bit
 * 
 *  \return uint16_t Identifier Extension Bit
 */
uint16_t TLE9563_getSwkIdExtBit(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_ID0_CTRL.bit.IDE;
}

/** \brief Get SWK WUF Identifier Mask bits 28...13 Register
 * 
 *  \return uint16_t SWK WUF Identifier Mask bits 28...13 Register
 */
uint16_t TLE9563_getSwkMaskId1CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
}

/** \brief Set SWK WUF Identifier Mask bits 28...13 Register
 * 
 *  \param u16_value SWK WUF Identifier Mask bits 28...13 Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkMaskId1CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Enable WUF Identifier Mask Bit 28
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit28Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID28_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID28_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID28_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 28
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit28Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID28_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID28_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID28_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 28 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 28 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit28Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID28;
}

/** \brief Enable WUF Identifier Mask Bit 27
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit27Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID27_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID27_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID27_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 27
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit27Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID27_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID27_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID27_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 27 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 27 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit27Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID27;
}

/** \brief Enable WUF Identifier Mask Bit 26
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit26Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID26_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID26_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID26_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 26
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit26Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID26_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID26_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID26_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 26 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 26 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit26Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID26;
}

/** \brief Enable WUF Identifier Mask Bit 25
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit25Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID25_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID25_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID25_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 25
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit25Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID25_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID25_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID25_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 25 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 25 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit25Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID25;
}

/** \brief Enable WUF Identifier Mask Bit 24
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit24Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID24_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID24_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID24_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 24
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit24Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID24_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID24_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID24_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 24 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 24 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit24Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID24;
}

/** \brief Enable WUF Identifier Mask Bit 23
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit23Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID23_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID23_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID23_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 23
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit23Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID23_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID23_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID23_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 23 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 23 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit23Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID23;
}

/** \brief Enable WUF Identifier Mask Bit 22
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit22Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID22_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID22_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID22_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 22
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit22Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID22_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID22_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID22_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 22 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 22 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit22Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID22;
}

/** \brief Enable WUF Identifier Mask Bit 21
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit21Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID21_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID21_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID21_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 21
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit21Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID21_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID21_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID21_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 21 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 21 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit21Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID21;
}

/** \brief Enable WUF Identifier Mask Bit 20
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit20Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID20_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID20_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID20_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 20
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit20Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID20_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID20_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID20_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 20 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 20 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit20Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID20;
}

/** \brief Enable WUF Identifier Mask Bit 19
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit19Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID19_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID19_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID19_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 19
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit19Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID19_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID19_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID19_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 19 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 19 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit19Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID19;
}

/** \brief Enable WUF Identifier Mask Bit 18
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit18Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID18_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID18_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID18_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 18
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit18Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID18_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID18_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID18_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 18 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 18 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit18Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID18;
}

/** \brief Enable WUF Identifier Mask Bit 17
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit17Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID17_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID17_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID17_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 17
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit17Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID17_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID17_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID17_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 17 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 17 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit17Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID17;
}

/** \brief Enable WUF Identifier Mask Bit 16
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit16Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID16_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID16_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID16_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 16
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit16Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID16_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID16_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID16_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 16 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 16 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit16Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID16;
}

/** \brief Enable WUF Identifier Mask Bit 15
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit15Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID15_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID15_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID15_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 15
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit15Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID15_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID15_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID15_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 15 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 15 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit15Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID15;
}

/** \brief Enable WUF Identifier Mask Bit 14
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit14Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID14_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID14_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID14_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 14
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit14Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID14_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID14_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID14_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 14 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 14 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit14Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID14;
}

/** \brief Enable WUF Identifier Mask Bit 13
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit13Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID13_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID13_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID13_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 13
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit13Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID1_CTRL_MASK_ID13_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID1_CTRL_MASK_ID13_Pos) & TLE9563_SWK_MASK_ID1_CTRL_MASK_ID13_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 13 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 13 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit13Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID1_CTRL.bit.MASK_ID13;
}

/** \brief Get SWK WUF Identifier Mask bits 12...0 Register
 * 
 *  \return uint16_t SWK WUF Identifier Mask bits 12...0 Register
 */
uint16_t TLE9563_getSwkMaskId0CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
}

/** \brief Set SWK WUF Identifier Mask bits 12...0 Register
 * 
 *  \param u16_value SWK WUF Identifier Mask bits 12...0 Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkMaskId0CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Enable WUF Identifier Mask Bit 12
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit12Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID12_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID12_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID12_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 12
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit12Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID12_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID12_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID12_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 12 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 12 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit12Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID12;
}

/** \brief Enable WUF Identifier Mask Bit 11
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit11Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID11_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID11_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID11_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 11
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit11Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID11_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID11_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID11_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 11 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 11 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit11Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID11;
}

/** \brief Enable WUF Identifier Mask Bit 10
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit10Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID10_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID10_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID10_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 10
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit10Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID10_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID10_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID10_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 10 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 10 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit10Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID10;
}

/** \brief Enable WUF Identifier Mask Bit 9
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit9Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID9_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID9_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID9_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 9
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit9Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID9_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID9_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID9_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 9 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 9 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit9Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID9;
}

/** \brief Enable WUF Identifier Mask Bit 8
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit8Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID8_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID8_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID8_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 8
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit8Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID8_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID8_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID8_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 8 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 8 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit8Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID8;
}

/** \brief Enable WUF Identifier Mask Bit 7
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit7Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID7_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID7_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID7_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 7
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit7Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID7_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID7_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID7_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 7 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 7 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit7Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID7;
}

/** \brief Enable WUF Identifier Mask Bit 6
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit6Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID6_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID6_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID6_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 6
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit6Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID6_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID6_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID6_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 6 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 6 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit6Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID6;
}

/** \brief Enable WUF Identifier Mask Bit 5
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit5Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID5_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID5_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID5_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 5
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit5Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID5_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID5_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID5_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 5 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 5 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit5Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID5;
}

/** \brief Enable WUF Identifier Mask Bit 4
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit4Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID4_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID4_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID4_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 4
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit4Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID4_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID4_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID4_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 4 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 4 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit4Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID4;
}

/** \brief Enable WUF Identifier Mask Bit 3
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit3Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID3_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID3_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID3_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 3
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit3Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID3_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID3_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID3_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 3 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 3 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit3Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID3;
}

/** \brief Enable WUF Identifier Mask Bit 2
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit2Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID2_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID2_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID2_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 2
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit2Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID2_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID2_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID2_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 2 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 2 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit2Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID2;
}

/** \brief Enable WUF Identifier Mask Bit 1
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit1Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID1_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID1_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 1
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit1Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID1_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID1_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 1 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 1 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit1Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID1;
}

/** \brief Enable WUF Identifier Mask Bit 0
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkIdBit0Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID0_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID0_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID0_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable WUF Identifier Mask Bit 0
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkIdBit0Mask(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_MASK_ID0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_MASK_ID0_CTRL_MASK_ID0_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_MASK_ID0_CTRL_MASK_ID0_Pos) & TLE9563_SWK_MASK_ID0_CTRL_MASK_ID0_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKMASKID0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get WUF Identifier Mask Bit 0 Enable/Disable Setting
 * 
 *  \return  WUF Identifier Mask Bit 0 Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkIdBit0Mask(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_MASK_ID0_CTRL.bit.MASK_ID0;
}

/** \brief Get SWK Frame Data Length Code Control Register
 * 
 *  \return uint16_t SWK Frame Data Length Code Control Register
 */
uint16_t TLE9563_getSwkDataLengthCodeReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DLC_CTRL.reg;
}

/** \brief Set SWK Frame Data Length Code Control Register
 * 
 *  \param u16_value SWK Frame Data Length Code Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkDataLengthCodeReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDLCCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Payload length in number of bytes 
 * 
 *  \param e_value Payload length in number of bytes 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkDataLengthCode(tSWK_dataLengthCode e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_DLC_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_DLC_CTRL_DLC_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_SWK_DLC_CTRL_DLC_Pos) & TLE9563_SWK_DLC_CTRL_DLC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDLCCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Payload length in number of bytes 
 * 
 *  \return uint16_t Payload length in number of bytes 
 */
uint16_t TLE9563_getSwkDataLengthCode(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DLC_CTRL.bit.DLC;
}

/** \brief Get SWK Data7-Data6 Register Register
 * 
 *  \return uint16_t SWK Data7-Data6 Register Register
 */
uint16_t TLE9563_getSwkData3CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DATA3_CTRL.reg;
}

/** \brief Set SWK Data7-Data6 Register Register
 * 
 *  \param u16_value SWK Data7-Data6 Register Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkData3CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDATA3CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Data7 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \param u8_value Data7 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkContentData7(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_DATA3_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_DATA3_CTRL_DATA7_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_DATA3_CTRL_DATA7_Pos) & TLE9563_SWK_DATA3_CTRL_DATA7_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDATA3CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Data7 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint16_t Data7 byte content(bit0=LSB; bit7=MSB)
 */
uint16_t TLE9563_getSwkContentData7(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DATA3_CTRL.bit.DATA7;
}

/** \brief Set Data6 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \param u8_value Data6 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkContentData6(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_DATA3_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_DATA3_CTRL_DATA6_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_DATA3_CTRL_DATA6_Pos) & TLE9563_SWK_DATA3_CTRL_DATA6_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDATA3CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Data6 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint16_t Data6 byte content(bit0=LSB; bit7=MSB)
 */
uint16_t TLE9563_getSwkContentData6(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DATA3_CTRL.bit.DATA6;
}

/** \brief Get SWK Data5-Data4 Register Register
 * 
 *  \return uint16_t SWK Data5-Data4 Register Register
 */
uint16_t TLE9563_getSwkData2CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DATA2_CTRL.reg;
}

/** \brief Set SWK Data5-Data4 Register Register
 * 
 *  \param u16_value SWK Data5-Data4 Register Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkData2CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDATA2CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Data5 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \param u8_value Data5 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkContentData5(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_DATA2_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_DATA2_CTRL_DATA5_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_DATA2_CTRL_DATA5_Pos) & TLE9563_SWK_DATA2_CTRL_DATA5_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDATA2CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Data5 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint16_t Data5 byte content(bit0=LSB; bit7=MSB)
 */
uint16_t TLE9563_getSwkContentData5(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DATA2_CTRL.bit.DATA5;
}

/** \brief Set Data4 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \param u8_value Data4 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkContentData4(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_DATA2_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_DATA2_CTRL_DATA4_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_DATA2_CTRL_DATA4_Pos) & TLE9563_SWK_DATA2_CTRL_DATA4_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDATA2CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Data4 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint16_t Data4 byte content(bit0=LSB; bit7=MSB)
 */
uint16_t TLE9563_getSwkContentData4(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DATA2_CTRL.bit.DATA4;
}

/** \brief Get SWK Data3-Data2 Register Register
 * 
 *  \return uint16_t SWK Data3-Data2 Register Register
 */
uint16_t TLE9563_getSwkData1CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DATA1_CTRL.reg;
}

/** \brief Set SWK Data3-Data2 Register Register
 * 
 *  \param u16_value SWK Data3-Data2 Register Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkData1CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDATA1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Data3 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \param u8_value Data3 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkContentData3(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_DATA1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_DATA1_CTRL_DATA3_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_DATA1_CTRL_DATA3_Pos) & TLE9563_SWK_DATA1_CTRL_DATA3_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDATA1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Data3 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint16_t Data3 byte content(bit0=LSB; bit7=MSB)
 */
uint16_t TLE9563_getSwkContentData3(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DATA1_CTRL.bit.DATA3;
}

/** \brief Set Data2 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \param u8_value Data2 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkContentData2(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_DATA1_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_DATA1_CTRL_DATA2_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_DATA1_CTRL_DATA2_Pos) & TLE9563_SWK_DATA1_CTRL_DATA2_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDATA1CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Data2 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint16_t Data2 byte content(bit0=LSB; bit7=MSB)
 */
uint16_t TLE9563_getSwkContentData2(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DATA1_CTRL.bit.DATA2;
}

/** \brief Get SWK Data1-Data0 Register Register
 * 
 *  \return uint16_t SWK Data1-Data0 Register Register
 */
uint16_t TLE9563_getSwkData0CtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DATA0_CTRL.reg;
}

/** \brief Set SWK Data1-Data0 Register Register
 * 
 *  \param u16_value SWK Data1-Data0 Register Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkData0CtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDATA0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Data1 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \param u8_value Data1 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkContentData1(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_DATA0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_DATA0_CTRL_DATA1_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_DATA0_CTRL_DATA1_Pos) & TLE9563_SWK_DATA0_CTRL_DATA1_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDATA0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Data1 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint16_t Data1 byte content(bit0=LSB; bit7=MSB)
 */
uint16_t TLE9563_getSwkContentData1(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DATA0_CTRL.bit.DATA1;
}

/** \brief Set Data0 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \param u8_value Data0 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkContentData0(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_DATA0_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_DATA0_CTRL_DATA0_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_DATA0_CTRL_DATA0_Pos) & TLE9563_SWK_DATA0_CTRL_DATA0_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKDATA0CTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Data0 byte content(bit0=LSB; bit7=MSB)
 * 
 *  \return uint16_t Data0 byte content(bit0=LSB; bit7=MSB)
 */
uint16_t TLE9563_getSwkContentData0(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_DATA0_CTRL.bit.DATA0;
}

/** \brief Get CAN FD Configuration Control Register Register
 * 
 *  \return uint16_t CAN FD Configuration Control Register Register
 */
uint16_t TLE9563_getSwkCanFdCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_CAN_FD_CTRL.reg;
}

/** \brief Set CAN FD Configuration Control Register Register
 * 
 *  \param u16_value CAN FD Configuration Control Register Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkCanFdCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCANFDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Enable Error Counter Disable Function
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkCanFdErrorCount(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CAN_FD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CAN_FD_CTRL_DIS_ERR_CNT_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_CAN_FD_CTRL_DIS_ERR_CNT_Pos) & TLE9563_SWK_CAN_FD_CTRL_DIS_ERR_CNT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCANFDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Error Counter Disable Function
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkCanFdErrorCount(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CAN_FD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CAN_FD_CTRL_DIS_ERR_CNT_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_CAN_FD_CTRL_DIS_ERR_CNT_Pos) & TLE9563_SWK_CAN_FD_CTRL_DIS_ERR_CNT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCANFDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Error Counter Disable Function Enable/Disable Setting
 * 
 *  \return  Error Counter Disable Function Status (0: feature enabled, 1: feature disabled)
 */
uint8_t TLE9563_getSwkCanFdErrorCount(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_CAN_FD_CTRL.bit.DIS_ERR_CNT;
}

/** \brief Set CAN FD Dominant Filter Time
 * 
 *  \param e_value CAN FD Dominant Filter Time
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setCanFdDomFiltTime(tCANFD_domFiltTime e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CAN_FD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CAN_FD_CTRL_FD_FILTER_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_SWK_CAN_FD_CTRL_FD_FILTER_Pos) & TLE9563_SWK_CAN_FD_CTRL_FD_FILTER_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCANFDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get CAN FD Dominant Filter Time
 * 
 *  \return uint16_t CAN FD Dominant Filter Time
 */
uint16_t TLE9563_getCanFdDomFiltTime(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_CAN_FD_CTRL.bit.FD_FILTER;
}

/** \brief Enable Enable CAN FD Tolerant Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enCanFdTolerantMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CAN_FD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CAN_FD_CTRL_CAN_FD_EN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_CAN_FD_CTRL_CAN_FD_EN_Pos) & TLE9563_SWK_CAN_FD_CTRL_CAN_FD_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCANFDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Enable CAN FD Tolerant Mode
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disCanFdTolerantMode(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CAN_FD_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CAN_FD_CTRL_CAN_FD_EN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_CAN_FD_CTRL_CAN_FD_EN_Pos) & TLE9563_SWK_CAN_FD_CTRL_CAN_FD_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCANFDCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Enable CAN FD Tolerant Mode Enable/Disable Setting
 * 
 *  \return  Enable CAN FD Tolerant Mode Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getCanFdTolerantMode(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_CAN_FD_CTRL.bit.CAN_FD_EN;
}

/** \brief Get SWK Oscillator Trimming and option Register Register
 * 
 *  \return uint16_t SWK Oscillator Trimming and option Register Register
 */
uint16_t TLE9563_getSwkOscTrimCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_OSC_TRIM_CTRL.reg;
}

/** \brief Set SWK Oscillator Trimming and option Register Register
 * 
 *  \param u16_value SWK Oscillator Trimming and option Register Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkOscTrimCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKOSCTRIMCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set SWK Receiver selection (only accessible if TRIM_EN = '11')
 * 
 *  \param e_value SWK Receiver selection (only accessible if TRIM_EN = '11')
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkReceiver(tSWK_receiver e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_OSC_TRIM_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_OSC_TRIM_CTRL_RX_WK_SEL_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_SWK_OSC_TRIM_CTRL_RX_WK_SEL_Pos) & TLE9563_SWK_OSC_TRIM_CTRL_RX_WK_SEL_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKOSCTRIMCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get SWK Receiver selection (only accessible if TRIM_EN = '11')
 * 
 *  \return uint16_t SWK Receiver selection (only accessible if TRIM_EN = '11')
 */
uint16_t TLE9563_getSwkReceiver(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_OSC_TRIM_CTRL.bit.RX_WK_SEL;
}

/** \brief Set Trimming of temp_coef (only writable if TRIM_EN = '11')
 * 
 *  \param u8_value Trimming of temp_coef (only writable if TRIM_EN = '11')
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkTrimTemp(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_OSC_TRIM_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_OSC_TRIM_CTRL_TEMP_COEF_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_OSC_TRIM_CTRL_TEMP_COEF_Pos) & TLE9563_SWK_OSC_TRIM_CTRL_TEMP_COEF_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKOSCTRIMCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Trimming of temp_coef (only writable if TRIM_EN = '11')
 * 
 *  \return uint16_t Trimming of temp_coef (only writable if TRIM_EN = '11')
 */
uint16_t TLE9563_getSwkTrimTemp(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_OSC_TRIM_CTRL.bit.TEMP_COEF;
}

/** \brief Set Trimming of oscillator (only writable if
 * 
 *  \param u8_value Trimming of oscillator (only writable if
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkTrimOsc(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_OSC_TRIM_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_OSC_TRIM_CTRL_TRIM_OSC_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_OSC_TRIM_CTRL_TRIM_OSC_Pos) & TLE9563_SWK_OSC_TRIM_CTRL_TRIM_OSC_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKOSCTRIMCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Trimming of oscillator (only writable if
 * 
 *  \return uint16_t Trimming of oscillator (only writable if
 */
uint16_t TLE9563_getSwkTrimOsc(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_OSC_TRIM_CTRL.bit.TRIM_OSC;
}

/** \brief Get Oscillator Calibration High Register
 * 
 *  \return  Oscillator Calibration High Register Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getSwkOscCalHighRegSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_OSC_CAL_STAT.bit.OSC_CAL_H;
}

/** \brief Get Oscillator Calibration Low Register
 * 
 *  \return  Oscillator Calibration Low Register Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getSwkOscCalLowRegSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_OSC_CAL_STAT.bit.OSC_CAL_L;
}

/** \brief Get Clock Data Recovery Control Register Register
 * 
 *  \return uint16_t Clock Data Recovery Control Register Register
 */
uint16_t TLE9563_getSwkCdrCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_CDR_CTRL.reg;
}

/** \brief Set Clock Data Recovery Control Register Register
 * 
 *  \param u16_value Clock Data Recovery Control Register Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkCdrCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCDRCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Input Frequency for CDR module
 * 
 *  \param e_value Input Frequency for CDR module
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkClockDataRecoveryInputFreq(tSWK_clockDataRecoveryInputFreq e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CDR_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CDR_CTRL_SEL_OSC_CLK_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_SWK_CDR_CTRL_SEL_OSC_CLK_Pos) & TLE9563_SWK_CDR_CTRL_SEL_OSC_CLK_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCDRCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Input Frequency for CDR module
 * 
 *  \return uint16_t Input Frequency for CDR module
 */
uint16_t TLE9563_getSwkClockDataRecoveryInputFreq(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_CDR_CTRL.bit.SEL_OSC_CLK;
}

/** \brief Set Select Time Constant of Filter
 * 
 *  \param e_value Select Time Constant of Filter
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkFiltTimeConst(tSWK_filtTimeConst e_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CDR_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CDR_CTRL_SELFILT_Msk)) | (((uint16_t)e_value << (uint16_t)TLE9563_SWK_CDR_CTRL_SELFILT_Pos) & TLE9563_SWK_CDR_CTRL_SELFILT_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCDRCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Select Time Constant of Filter
 * 
 *  \return uint16_t Select Time Constant of Filter
 */
uint16_t TLE9563_getSwkFiltTimeConst(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_CDR_CTRL.bit.SELFILT;
}

/** \brief Enable Enable CDR
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_enSwkClockDataRecovery(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CDR_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CDR_CTRL_CDR_EN_Msk)) | (((uint16_t)1 << (uint16_t)TLE9563_SWK_CDR_CTRL_CDR_EN_Pos) & TLE9563_SWK_CDR_CTRL_CDR_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCDRCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Disable Enable CDR
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_disSwkClockDataRecovery(void)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CDR_CTRL.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CDR_CTRL_CDR_EN_Msk)) | (((uint16_t)0 << (uint16_t)TLE9563_SWK_CDR_CTRL_CDR_EN_Pos) & TLE9563_SWK_CDR_CTRL_CDR_EN_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCDRCTRL;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief get Enable CDR Enable/Disable Setting
 * 
 *  \return  Enable CDR Status (1: feature enabled, 0: feature disabled)
 */
uint8_t TLE9563_getSwkClockDataRecovery(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_CDR_CTRL.bit.CDR_EN;
}

/** \brief Get SWK Clock Data Recovery Limit Control Register
 * 
 *  \return uint16_t SWK Clock Data Recovery Limit Control Register
 */
uint16_t TLE9563_getSwkCdrLimitCtrlReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_CDR_LIMIT.reg;
}

/** \brief Set SWK Clock Data Recovery Limit Control Register
 * 
 *  \param u16_value SWK Clock Data Recovery Limit Control Register content
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkCdrLimitCtrlReg(uint16_t u16_value)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = u16_value;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCDRLIMIT;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Set Upper Bit Time Detection Range of Clock and Data
 * 
 *  \param u8_value Upper Bit Time Detection Range of Clock and Data
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkClockDataRecoveryLimitHigh(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CDR_LIMIT.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CDR_LIMIT_CDR_LIM_H_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_CDR_LIMIT_CDR_LIM_H_Pos) & TLE9563_SWK_CDR_LIMIT_CDR_LIM_H_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCDRLIMIT;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Upper Bit Time Detection Range of Clock and Data
 * 
 *  \return uint16_t Upper Bit Time Detection Range of Clock and Data
 */
uint16_t TLE9563_getSwkClockDataRecoveryLimitHigh(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_CDR_LIMIT.bit.CDR_LIM_H;
}

/** \brief Set Lower Bit Time Detection Range of Clock and Data
 * 
 *  \param u8_value Lower Bit Time Detection Range of Clock and Data
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_setSwkClockDataRecoveryLimitLow(uint8_t u8_value)
{
  uint8_t u8_success = 0;
  uint16_t u16_reg_temp = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    /* get register content */
    u16_reg_temp = (uint16_t)TLE9563->SWK_CDR_LIMIT.reg;
    /* modify requested bits */
    u16_reg_temp = (uint16_t)((u16_reg_temp & (~TLE9563_SWK_CDR_LIMIT_CDR_LIM_L_Msk)) | (((uint16_t)u8_value << (uint16_t)TLE9563_SWK_CDR_LIMIT_CDR_LIM_L_Pos) & TLE9563_SWK_CDR_LIMIT_CDR_LIM_L_Msk));
    s_deviceDriver.u16_setBitValue = u16_reg_temp;
    s_deviceDriver.fp_setReg = &TLE9563_setSWKCDRLIMIT;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Lower Bit Time Detection Range of Clock and Data
 * 
 *  \return uint16_t Lower Bit Time Detection Range of Clock and Data
 */
uint16_t TLE9563_getSwkClockDataRecoveryLimitLow(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_CDR_LIMIT.bit.CDR_LIM_L;
}

/** \brief Get Supply Voltage Fail Status Register
 * 
 *  \return uint16_t Supply Voltage Fail Status Register
 */
uint16_t TLE9563_getSupStsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SUP_STAT.reg;
}

/** \brief Clear Supply Voltage Fail Status Status
 * 
 *  \warning Status clearing can only be done word-wise, so this function clears the complete status register.
 *  \warning This function will not update the regsiter copy in RAM. 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_clrSupSts(void)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = 0x0000;
    s_deviceDriver.fp_setReg = &TLE9563_clrSUPSTAT;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Power-On reset detection
 * 
 *  \return  Power-On reset detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getPorSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.POR;
}

/** \brief Get Charge pump overtemperature
 * 
 *  \return  Charge pump overtemperature Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getCpOverTempSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.CP_OT;
}

/** \brief Get 4th consecutive VCC1 UV-Detection
 * 
 *  \return  4th consecutive VCC1 UV-Detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVcc1UnderVoltFsSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.VCC1_UV_FS;
}

/** \brief Get HS Supply UV-Detection
 * 
 *  \return  HS Supply UV-Detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHsUnderVoltSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.HS_UV;
}

/** \brief Get HS Supply OV-Detection
 * 
 *  \return  HS Supply OV-Detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHsOverVoltSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.HS_OV;
}

/** \brief Get VSINT UV-Detection
 * 
 *  \return  VSINT UV-Detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVsintUnderVoltSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.VSINT_UV;
}

/** \brief Get VSINT OV-Detection
 * 
 *  \return  VSINT OV-Detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVsintOverVoltSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.VSINT_OV;
}

/** \brief Get VS Undervoltage Detection (VS,UV)
 * 
 *  \return  VS Undervoltage Detection (VS,UV) Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVsUnderVoltSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.VS_UV;
}

/** \brief Get VS Overvoltage Detection (VS,OV)
 * 
 *  \return  VS Overvoltage Detection (VS,OV) Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVsOverVoltSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.VS_OV;
}

/** \brief Get CP_UV
 * 
 *  \return  CP_UV Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getCpUnterVoltSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.CP_UV;
}

/** \brief Get VCC1 SC
 * 
 *  \return  VCC1 SC Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVcc1ShortCircuitSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.VCC1_SC;
}

/** \brief Get VCC1 UV-Detection (due to Vrtx reset)
 * 
 *  \return  VCC1 UV-Detection (due to Vrtx reset) Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVcc1UnterVoltSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.VCC1_UV;
}

/** \brief Get VCC1 Overvoltage Detection
 * 
 *  \return  VCC1 Overvoltage Detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVcc1OverVoltSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.VCC1_OV;
}

/** \brief Get VCC1 Undervoltage Prewarning
 * 
 *  \return  VCC1 Undervoltage Prewarning Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVcc1UnderVoltWarnSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SUP_STAT.bit.VCC1_WARN;
}

/** \brief Get Thermal Protection Status Register
 * 
 *  \return uint16_t Thermal Protection Status Register
 */
uint16_t TLE9563_getThermStsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->THERM_STAT.reg;
}

/** \brief Clear Thermal Protection Status Status
 * 
 *  \warning Status clearing can only be done word-wise, so this function clears the complete status register.
 *  \warning This function will not update the regsiter copy in RAM. 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_clrThermSts(void)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = 0x0000;
    s_deviceDriver.fp_setReg = &TLE9563_clrTHERMSTAT;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get TSD2 Thermal Shut-Down Safe State Detection
 * 
 *  \return  TSD2 Thermal Shut-Down Safe State Detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getThermShutDown2SafeStateSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->THERM_STAT.bit.TSD2_SAFE;
}

/** \brief Get TSD2 Thermal Shut-Down Detection
 * 
 *  \return  TSD2 Thermal Shut-Down Detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getThermShutDown2Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->THERM_STAT.bit.TSD2;
}

/** \brief Get TSD1 Thermal Shut-Down Detection
 * 
 *  \return  TSD1 Thermal Shut-Down Detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getThermShutDown1Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->THERM_STAT.bit.TSD1;
}

/** \brief Get Thermal Pre Warning
 * 
 *  \return  Thermal Pre Warning Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getThermWarnSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->THERM_STAT.bit.TPW;
}

/** \brief Get Device Information Status Register
 * 
 *  \return uint16_t Device Information Status Register
 */
uint16_t TLE9563_getDevStsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->DEV_STAT.reg;
}

/** \brief Clear Device Information Status Status
 * 
 *  \warning Status clearing can only be done word-wise, so this function clears the complete status register.
 *  \warning This function will not update the regsiter copy in RAM. 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_clrDevSts(void)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = 0x0000;
    s_deviceDriver.fp_setReg = &TLE9563_clrDEVSTAT;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get CRC STAT Information
 * 
 *  \return  CRC STAT Information Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getCrcSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DEV_STAT.bit.CRC_STAT;
}

/** \brief Get CRC Fail Information1)
 * 
 *  \return  CRC Fail Information1) Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getCrcFailSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DEV_STAT.bit.CRC_FAIL;
}

/** \brief Get Device Status before Restart Mode
 * 
 *  \return  Device Status before Restart Mode Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getDeviceSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DEV_STAT.bit.DEV_STAT;
}

/** \brief Get Status of Operating Mode
 * 
 *  \return  Status of Operating Mode Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getOperatingModeSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DEV_STAT.bit.SW_DEV;
}

/** \brief Get Number of WD-Failure Events
 * 
 *  \return  Number of WD-Failure Events Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getWdFailSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DEV_STAT.bit.WD_FAIL;
}

/** \brief Get SPI Fail Information
 * 
 *  \return  SPI Fail Information Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getSpiFailSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DEV_STAT.bit.SPI_FAIL;
}

/** \brief Get Failure detection
 * 
 *  \return  Failure detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getFailSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DEV_STAT.bit.FAILURE;
}

/** \brief Get Bus Communication Status Register
 * 
 *  \return uint16_t Bus Communication Status Register
 */
uint16_t TLE9563_getBusStsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->BUS_STAT.reg;
}

/** \brief Clear Bus Communication Status Status
 * 
 *  \warning Status clearing can only be done word-wise, so this function clears the complete status register.
 *  \warning This function will not update the regsiter copy in RAM. 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_clrBusSts(void)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = 0x0000;
    s_deviceDriver.fp_setReg = &TLE9563_clrBUSSTAT;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get CAN Time Out Detection
 * 
 *  \return  CAN Time Out Detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getCanTimeOutSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->BUS_STAT.bit.CANTO;
}

/** \brief Get SWK System Error
 * 
 *  \return  SWK System Error Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getSwkSysErrSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->BUS_STAT.bit.SYSERR;
}

/** \brief Get CAN failure status
 * 
 *  \return  CAN failure status Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getCanFailSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->BUS_STAT.bit.CAN_FAIL;
}

/** \brief Get Under Voltage CAN Bus Supply
 * 
 *  \return  Under Voltage CAN Bus Supply Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVcanUnderVoltSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->BUS_STAT.bit.VCAN_UV;
}

/** \brief Get Wake-up Source and Information Status Register
 * 
 *  \return uint16_t Wake-up Source and Information Status Register
 */
uint16_t TLE9563_getWkStsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->WK_STAT.reg;
}

/** \brief Clear Wake-up Source and Information Status Status
 * 
 *  \warning Status clearing can only be done word-wise, so this function clears the complete status register.
 *  \warning This function will not update the regsiter copy in RAM. 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_clrWkSts(void)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = 0x0000;
    s_deviceDriver.fp_setReg = &TLE9563_clrWKSTAT;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Wake up via CAN Bus
 * 
 *  \return  Wake up via CAN Bus Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getWkCanWuSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->WK_STAT.bit.CAN_WU;
}

/** \brief Get Wake up via Timer2
 * 
 *  \return  Wake up via Timer2 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getWkTimer2WuSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->WK_STAT.bit.TIMER2_WU;
}

/** \brief Get Wake up via Timer1
 * 
 *  \return  Wake up via Timer1 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getWkTimer1WuSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->WK_STAT.bit.TIMER1_WU;
}

/** \brief Get Wake up via WK5
 * 
 *  \return  Wake up via WK5 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getWk5WuSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->WK_STAT.bit.WK5_WU;
}

/** \brief Get Wake up via WK4
 * 
 *  \return  Wake up via WK4 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getWk4WuSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->WK_STAT.bit.WK4_WU;
}

/** \brief Get WK Input Level Register
 * 
 *  \return uint16_t WK Input Level Register
 */
uint16_t TLE9563_getWkxLvlStsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->WK_LVL_STAT.reg;
}

/** \brief Get Status of WK5
 * 
 *  \return  Status of WK5 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getWk5LvlSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->WK_LVL_STAT.bit.WK5_LVL;
}

/** \brief Get Status of WK4
 * 
 *  \return  Status of WK4 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getWk4LvlSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->WK_LVL_STAT.bit.WK4_LVL;
}

/** \brief Get High-Side Switch Status Register
 * 
 *  \return uint16_t High-Side Switch Status Register
 */
uint16_t TLE9563_getHsOlOcOtStsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->HS_OL_OC_OT_STAT.reg;
}

/** \brief Clear High-Side Switch Status Status
 * 
 *  \warning Status clearing can only be done word-wise, so this function clears the complete status register.
 *  \warning This function will not update the regsiter copy in RAM. 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_clrHsOlOcOtSts(void)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = 0x0000;
    s_deviceDriver.fp_setReg = &TLE9563_clrHSOLOCOTSTAT;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Overtemperature Detection on HS3
 * 
 *  \return  Overtemperature Detection on HS3 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHs3OverTempSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HS_OL_OC_OT_STAT.bit.HS3_OT;
}

/** \brief Get Overtemperature Detection on HS2
 * 
 *  \return  Overtemperature Detection on HS2 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHs2OverTempSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HS_OL_OC_OT_STAT.bit.HS2_OT;
}

/** \brief Get Overtemperature Detection on HS1
 * 
 *  \return  Overtemperature Detection on HS1 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHs1OverTempSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HS_OL_OC_OT_STAT.bit.HS1_OT;
}

/** \brief Get Open-Load Detection on HS3
 * 
 *  \return  Open-Load Detection on HS3 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHs3OpenLoadSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HS_OL_OC_OT_STAT.bit.HS3_OL;
}

/** \brief Get Open-Load Detection on HS2
 * 
 *  \return  Open-Load Detection on HS2 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHs2OpenLoadSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HS_OL_OC_OT_STAT.bit.HS2_OL;
}

/** \brief Get Open-Load Detection on HS1
 * 
 *  \return  Open-Load Detection on HS1 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHs1OpenLoadSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HS_OL_OC_OT_STAT.bit.HS1_OL;
}

/** \brief Get Overcurrent Detection on HS3
 * 
 *  \return  Overcurrent Detection on HS3 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHs3OverCurSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HS_OL_OC_OT_STAT.bit.HS3_OC;
}

/** \brief Get Overcurrent Detection on HS2
 * 
 *  \return  Overcurrent Detection on HS2 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHs2OverCurSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HS_OL_OC_OT_STAT.bit.HS2_OC;
}

/** \brief Get Overcurrent Detection on HS1
 * 
 *  \return  Overcurrent Detection on HS1 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHs1OverCurSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->HS_OL_OC_OT_STAT.bit.HS1_OC;
}

/** \brief Get General Status register Register
 * 
 *  \return uint16_t General Status register Register
 */
uint16_t TLE9563_getGenStsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->GEN_STAT.reg;
}

/** \brief Get Voltage level at VSH3 when HB3MODE[10] = 11 and CPEN=1
 * 
 *  \return  Voltage level at VSH3 when HB3MODE[10] = 11 and CPEN=1 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHb3Vsh3VoltLvlSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GEN_STAT.bit.HB3VOUT;
}

/** \brief Get Voltage level at VSH2 when HB2MODE[10] = 11 and CPEN=1
 * 
 *  \return  Voltage level at VSH2 when HB2MODE[10] = 11 and CPEN=1 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHb2Vsh2VoltLvlSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GEN_STAT.bit.HB2VOUT;
}

/** \brief Get Voltage level at VSH1 when HB1MODE[10] = 11 and CPEN=1
 * 
 *  \return  Voltage level at VSH1 when HB1MODE[10] = 11 and CPEN=1 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHb1Vsh1VoltLvlSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GEN_STAT.bit.HB1VOUT;
}

/** \brief Get PWM6 status
 * 
 *  \return  PWM6 status Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getPwm6Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GEN_STAT.bit.PWM6STAT;
}

/** \brief Get PWM5 status
 * 
 *  \return  PWM5 status Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getPwm5Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GEN_STAT.bit.PWM5STAT;
}

/** \brief Get PWM4 Status
 * 
 *  \return  PWM4 Status Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getPwm4Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GEN_STAT.bit.PWM4STAT;
}

/** \brief Get PWM3 status
 * 
 *  \return  PWM3 status Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getPwm3Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GEN_STAT.bit.PWM3STAT;
}

/** \brief Get PWM2 Status
 * 
 *  \return  PWM2 Status Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getPwm2Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GEN_STAT.bit.PWM2STAT;
}

/** \brief Get PWM1/CRC status
 * 
 *  \return  PWM1/CRC status Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getPwm1Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->GEN_STAT.bit.PWM1STAT;
}

/** \brief Get Turn-on/off delay regulation register Register
 * 
 *  \return uint16_t Turn-on/off delay regulation register Register
 */
uint16_t TLE9563_getTdregStsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TDREG.reg;
}

/** \brief Get HB3 predischarge status Status
 * 
 *  \return  HB3 predischarge status Status (0: Status set, 1: Status not set)
 */
uint8_t TLE9563_getHb3PreDischargeCurClamp(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TDREG.bit.IPDCHG3_ST;
}

/** \brief Get HB2 predischarge status Status
 * 
 *  \return  HB2 predischarge status Status (0: Status set, 1: Status not set)
 */
uint8_t TLE9563_getHb2PreDischargeCurClamp(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TDREG.bit.IPDCHG2_ST;
}

/** \brief Get HB1 predischarge status Status
 * 
 *  \return  HB1 predischarge status Status (0: Status set, 1: Status not set)
 */
uint8_t TLE9563_getHb1PreDischargeCurClamp(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TDREG.bit.IPDCHG1_ST;
}

/** \brief Get HB3 precharge status Status
 * 
 *  \return  HB3 precharge status Status (0: Status set, 1: Status not set)
 */
uint8_t TLE9563_getHb3PreChargeCurClamp(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TDREG.bit.IPCHG3_ST;
}

/** \brief Get HB2 precharge status Status
 * 
 *  \return  HB2 precharge status Status (0: Status set, 1: Status not set)
 */
uint8_t TLE9563_getHb2PreChargeCurClamp(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TDREG.bit.IPCHG2_ST;
}

/** \brief Get HB1 precharge status Status
 * 
 *  \return  HB1 precharge status Status (0: Status set, 1: Status not set)
 */
uint8_t TLE9563_getHb1PreChargeCurClamp(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TDREG.bit.IPCHG1_ST;
}

/** \brief Get HB3 Regulation of turn-on/off delay
 * 
 *  \return  HB3 Regulation of turn-on/off delay Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHb3TdonTdoffRegSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TDREG.bit.TDREG3;
}

/** \brief Get HB2 Regulation of turn-on/off delay
 * 
 *  \return  HB2 Regulation of turn-on/off delay Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHb2TdonTdoffRegSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TDREG.bit.TDREG2;
}

/** \brief Get HB1 Regulation of turn-on/off delay
 * 
 *  \return  HB1 Regulation of turn-on/off delay Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getHb1TdonTdoffRegSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TDREG.bit.TDREG1;
}

/** \brief Get Drain-source overvoltage Register
 * 
 *  \return uint16_t Drain-source overvoltage Register
 */
uint16_t TLE9563_getDsovStsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->DSOV.reg;
}

/** \brief Clear Drain-source overvoltage Status
 * 
 *  \warning Status clearing can only be done word-wise, so this function clears the complete status register.
 *  \warning This function will not update the regsiter copy in RAM. 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_clrDsovSts(void)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = 0x0000;
    s_deviceDriver.fp_setReg = &TLE9563_clrDSOV;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get CSA Overcurrent detection
 * 
 *  \return  CSA Overcurrent detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getCsaOverCurSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DSOV.bit.OC_CSA;
}

/** \brief Get VSINT Brake status
 * 
 *  \return  VSINT Brake status Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVsintOverVoltBrakeSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DSOV.bit.VSINTOVBRAKE_ST;
}

/** \brief Get VS Brake status
 * 
 *  \return  VS Brake status Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVsOverVoltBrakeSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DSOV.bit.VSOVBRAKE_ST;
}

/** \brief Get Drain-source overvoltage on low-side 3 during braking
 * 
 *  \return  Drain-source overvoltage on low-side 3 during braking Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVdsOverVoltLs3BrakeSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DSOV.bit.LS3DSOV_BRK;
}

/** \brief Get Drain-source overvoltage on low-side 2 during braking
 * 
 *  \return  Drain-source overvoltage on low-side 2 during braking Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVdsOverVoltLs2BrakeSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DSOV.bit.LS2DSOV_BRK;
}

/** \brief Get Drain-source overvoltage on low-side 1 during braking
 * 
 *  \return  Drain-source overvoltage on low-side 1 during braking Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVdsOverVoltLs1BrakeSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DSOV.bit.LS1DSOV_BRK;
}

/** \brief Get Drain-source overvoltage on low-side 3
 * 
 *  \return  Drain-source overvoltage on low-side 3 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVdsOverVoltLs3Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DSOV.bit.LS3DSOV;
}

/** \brief Get Drain-source overvoltage on high-side 3
 * 
 *  \return  Drain-source overvoltage on high-side 3 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVdsOverVoltHs3Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DSOV.bit.HS3DSOV;
}

/** \brief Get Drain-source overvoltage on low-side 2
 * 
 *  \return  Drain-source overvoltage on low-side 2 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVdsOverVoltLs2Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DSOV.bit.LS2DSOV;
}

/** \brief Get Drain-source overvoltage on high-side 2
 * 
 *  \return  Drain-source overvoltage on high-side 2 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVdsOverVoltHs2Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DSOV.bit.HS2DSOV;
}

/** \brief Get Drain-source overvoltage on low-side 1
 * 
 *  \return  Drain-source overvoltage on low-side 1 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVdsOverVoltLs1Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DSOV.bit.LS1DSOV;
}

/** \brief Get Drain-source overvoltage on high-side 1
 * 
 *  \return  Drain-source overvoltage on high-side 1 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getVdsOverVoltHs1Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->DSOV.bit.HS1DSOV;
}

/** \brief Get Effective MOSFET turn.on/off delay - HB1 Register
 * 
 *  \return uint16_t Effective MOSFET turn.on/off delay - HB1 Register
 */
uint16_t TLE9563_getEffTdoffTdon1StsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->EFF_TDON_OFF1.reg;
}

/** \brief Get Effective active MOSFET turn-off delay HB1
 * 
 *  \return  Effective active MOSFET turn-off delay HB1 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getBdrvEffTdoff1Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->EFF_TDON_OFF1.bit.TDOFF1EFF;
}

/** \brief Get Effective active MOSFET turn-on delay HB1 Nominal effective tDON1 = 53.3 ns x TDON1EFF[50]D
 * 
 *  \return  Effective active MOSFET turn-on delay HB1 Nominal effective tDON1 = 53.3 ns x TDON1EFF[50]D Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getBdrvEffTdon1Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->EFF_TDON_OFF1.bit.TDON1EFF;
}

/** \brief Get Effective MOSFET turn.on/off delay - HB 2 Register
 * 
 *  \return uint16_t Effective MOSFET turn.on/off delay - HB 2 Register
 */
uint16_t TLE9563_getEffTdoffTdon2StsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->EFF_TDON_OFF2.reg;
}

/** \brief Get Effective active MOSFET turn-off delay HB2
 * 
 *  \return  Effective active MOSFET turn-off delay HB2 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getBdrvEffTdoff2Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->EFF_TDON_OFF2.bit.TDOFF2EFF;
}

/** \brief Get Effective active MOSFET turn-on delay HB2 Nominal effective tDON2 = 53.3 ns x TDON2EFF[50]D
 * 
 *  \return  Effective active MOSFET turn-on delay HB2 Nominal effective tDON2 = 53.3 ns x TDON2EFF[50]D Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getBdrvEffTdon2Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->EFF_TDON_OFF2.bit.TDON2EFF;
}

/** \brief Get Effective MOSFET turn.on/off delay - HB3 Register
 * 
 *  \return uint16_t Effective MOSFET turn.on/off delay - HB3 Register
 */
uint16_t TLE9563_getEffTdoffTdon3StsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->EFF_TDON_OFF3.reg;
}

/** \brief Get Effective active MOSFET turn-off delay HB3 Nominal effective tDOFF3 = 53.3 ns x TDO3EFF[138]D
 * 
 *  \return  Effective active MOSFET turn-off delay HB3 Nominal effective tDOFF3 = 53.3 ns x TDO3EFF[138]D Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getBdrvEffTdoff3Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->EFF_TDON_OFF3.bit.TDOFF3EFF;
}

/** \brief Get Effective active MOSFET turn-on delay HB3 Nominal effective tDON3 = 53.3 ns x TDON3EFF[50]D
 * 
 *  \return  Effective active MOSFET turn-on delay HB3 Nominal effective tDON3 = 53.3 ns x TDON3EFF[50]D Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getBdrvEffTdon3Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->EFF_TDON_OFF3.bit.TDON3EFF;
}

/** \brief Get MOSFET rise/fall time - HB1 Register
 * 
 *  \return uint16_t MOSFET rise/fall time - HB1 Register
 */
uint16_t TLE9563_getEffTfallTrise1StsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TRISE_FALL1.reg;
}

/** \brief Get Active MOSFET fall time HB1
 * 
 *  \return  Active MOSFET fall time HB1 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getBdrvEffTfall1Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TRISE_FALL1.bit.TFALL1;
}

/** \brief Get Active MOSFET rise time HB1
 * 
 *  \return  Active MOSFET rise time HB1 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getBdrvEffTrise1Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TRISE_FALL1.bit.TRISE1;
}

/** \brief Get MOSFET rise/fall time - HB2 Register
 * 
 *  \return uint16_t MOSFET rise/fall time - HB2 Register
 */
uint16_t TLE9563_getEffTfallTrise2StsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TRISE_FALL2.reg;
}

/** \brief Get Active MOSFET fall time HB2
 * 
 *  \return  Active MOSFET fall time HB2 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getBdrvEffTfall2Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TRISE_FALL2.bit.TFALL2;
}

/** \brief Get Active MOSFET rise time HB2
 * 
 *  \return  Active MOSFET rise time HB2 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getBdrvEffTrise2Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TRISE_FALL2.bit.TRISE2;
}

/** \brief Get MOSFET rise/fall time - HB3 Register
 * 
 *  \return uint16_t MOSFET rise/fall time - HB3 Register
 */
uint16_t TLE9563_getEffTfallTrise3StsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->TRISE_FALL3.reg;
}

/** \brief Get Active MOSFET fall time HB3
 * 
 *  \return  Active MOSFET fall time HB3 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getBdrvEffTfall3Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TRISE_FALL3.bit.TFALL3;
}

/** \brief Get Active MOSFET rise time HB3
 * 
 *  \return  Active MOSFET rise time HB3 Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getBdrvEffTrise3Sts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->TRISE_FALL3.bit.TRISE3;
}

/** \brief Get Selective Wake Status Register
 * 
 *  \return uint16_t Selective Wake Status Register
 */
uint16_t TLE9563_getSwkStsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->SWK_STAT.reg;
}

/** \brief Clear Selective Wake Status Status
 * 
 *  \warning Status clearing can only be done word-wise, so this function clears the complete status register.
 *  \warning This function will not update the regsiter copy in RAM. 
 * 
 *  \return uint8_t Success (0: request not successful, 1: request successful)
 */
uint8_t TLE9563_clrSwkSts(void)
{
  uint8_t u8_success = 0;
  /* if no current set-register request is set, start set-register request */
  if(s_deviceDriver.fp_setReg == NULL)
  {
    s_deviceDriver.u16_setBitValue = 0x0000;
    s_deviceDriver.fp_setReg = &TLE9563_clrSWKSTAT;
    u8_success = 1;
  }
  return u8_success;
}

/** \brief Get Synchronisation (at least one CAN frame without fail must have been received)
 * 
 *  \return  Synchronisation (at least one CAN frame without fail must have been received) Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getSwkSyncSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_STAT.bit.SYNC;
}

/** \brief Get Wake-up Pattern Detection
 * 
 *  \return  Wake-up Pattern Detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getSwkWkPatternDetectSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_STAT.bit.WUP;
}

/** \brief Get SWK Wake-up Frame Detection
 * 
 *  \return  SWK Wake-up Frame Detection Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getSwkWkFrameDetectSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_STAT.bit.WUF;
}

/** \brief Get CAN Silent Time during SWK operation
 * 
 *  \return  CAN Silent Time during SWK operation Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getSwkCanSilentTimeSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_STAT.bit.CANSIL;
}

/** \brief Get Selective Wake Activity
 * 
 *  \return  Selective Wake Activity Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getSwkActivitySts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_STAT.bit.SWK_SET;
}

/** \brief Get SWK CAN Frame Error Counter
 * 
 *  \return  SWK CAN Frame Error Counter Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getSwkCanFrameErrCountSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_ECNT_STAT.bit.ECNT;
}

/** \brief Get Output Value from Filter Block
 * 
 *  \return  Output Value from Filter Block Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getSwkClockDataRecoveryOutputSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->SWK_CDR_STAT.bit.N_AVG;
}

/** \brief Get Family and Product Identification Register Register
 * 
 *  \return uint16_t Family and Product Identification Register Register
 */
uint16_t TLE9563_getFamProdIdStsReg(void)
{
  /* return value from register copy */
  return (uint16_t)TLE9563->FAM_PROD_STAT.reg;
}

/** \brief Get Device Family Identifier
 * 
 *  \return  Device Family Identifier Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getDeviceFamilyIdSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->FAM_PROD_STAT.bit.FAM;
}

/** \brief Get Device Product Identifier
 * 
 *  \return  Device Product Identifier Status (1: Status set, 0: Status not set)
 */
uint8_t TLE9563_getDeviceProductIdSts(void)
{
  /* return value from register copy */
  return (uint8_t)TLE9563->FAM_PROD_STAT.bit.PROD;
}

