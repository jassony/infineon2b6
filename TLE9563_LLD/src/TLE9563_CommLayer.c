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

/** \addtogroup TLE9563_COMMLAYER
 *  @{
 */

/*******************************************************************************
**                                  Includes                                  **
*******************************************************************************/
#include "TLE9563_CommLayer.h"
#include "TLE9563_FuncLayer.h"

/*******************************************************************************
**                           Local Macro Definitions                          **
*******************************************************************************/


/*******************************************************************************
**                           Local Type Definitions                           **
*******************************************************************************/


/******************************************************************************/
/**                        Local Variable Definitions                        **/
/******************************************************************************/
/* SPI - transmit buffer */
static  unDEVICE_spiBuffer          au8TxBuffer;

/* SPI port configuration */
static cy_stc_gpio_pin_config_t SPI_port_pin_cfg =
{
    .outVal    = 0ul,
    .driveMode = 0ul,            /* Will be updated in runtime */
    .hsiom     = HSIOM_SEL_GPIO, /* Will be updated in runtime */
    .intEdge   = 0ul,
    .intMask   = 0ul,
    .vtrip     = 0ul,
    .slewRate  = 0ul,
    .driveSel  = 0ul,
};

/* SPI instance configuration */
static const cy_stc_scb_spi_config_t SCB_SPI_cfg =
{
    .spiMode                    = CY_SCB_SPI_MASTER,      /*** Specifies the mode of operation    ***/
    .subMode                    = CY_SCB_SPI_MOTOROLA,    /*** Specifies the sub mode of SPI operation    ***/
    .sclkMode                   = CY_SCB_SPI_CPHA1_CPOL0, /*** Clock is active low, data is changed on trailing edge ***/
    .oversample                 = SCB_SPI_OVERSAMPLING,   /*** SPI_CLOCK divided by SCB_SPI_OVERSAMPLING should be baudrate  ***/
    .rxDataWidth                = 32ul,                   /*** The width of RX data (valid range 4-16). It must be the same as \ref txDataWidth except in National sub-mode. ***/
    .txDataWidth                = 32ul,                   /*** The width of TX data (valid range 4-16). It must be the same as \ref rxDataWidth except in National sub-mode. ***/
    .enableMsbFirst             = false,                  /*** Enables the hardware to shift out the data element MSB first, otherwise, LSB first ***/
    .enableFreeRunSclk          = false,                  /*** Enables the master to generate a continuous SCLK regardless of whether there is data to send  ***/
    .enableInputFilter          = false,                  /*** Enables a digital 3-tap median filter to be applied to the input of the RX FIFO to filter glitches on the line. ***/
    .enableMisoLateSample       = true,                   /*** Enables the master to sample MISO line one half clock later to allow better timings. ***/
    .enableTransferSeperation   = true,                   /*** Enables the master to transmit each data element separated by a de-assertion of the slave select line (only applicable for the master mode) ***/
    .ssPolarity0                = false,                  /*** SS0: active low ***/
    .ssPolarity1                = false,                  /*** SS1: active low ***/
    .ssPolarity2                = false,                  /*** SS2: active low ***/
    .ssPolarity3                = false,                  /*** SS3: active low ***/
    .enableWakeFromSleep        = false,                  /*** When set, the slave will wake the device when the slave select line becomes active. Note that not all SCBs support this mode. Consult the device datasheet to determine which SCBs support wake from deep sleep. ***/
    .rxFifoTriggerLevel         = 0ul,                    /*** Interrupt occurs, when there are more entries of 1 in the RX FIFO */
    .rxFifoIntEnableMask        = 1ul,                    /*** Bits set in this mask will allow events to cause an interrupt  */
    .txFifoTriggerLevel         = 0ul,                    /*** When there are fewer entries in the TX FIFO, then at this level the TX trigger output goes high. This output can be connected to a DMA channel through a trigger mux. Also, it controls the \ref CY_SCB_SPI_TX_TRIGGER interrupt source. */
    .txFifoIntEnableMask        = 0ul,                    /*** Bits set in this mask allow events to cause an interrupt  */
    .masterSlaveIntEnableMask   = 0ul,                    /*** Bits set in this mask allow events to cause an interrupt  */
    .enableSpiDoneInterrupt     = false,
    .enableSpiBusErrorInterrupt = false,
};

/* DMA - descriptors for SPI transfer */
static  cy_stc_pdma_descr_t         stcDescr1e;
static  cy_stc_pdma_descr_t         stcDescr11a;

#if  (TLE9563_CRC_EN == 1)
/* DMA - descriptors for CRC calculation */
static  cy_stc_pdma_descr_t         stcDescr1a;
static  cy_stc_pdma_descr_t         stcDescr1b;
static  cy_stc_pdma_descr_t         stcDescr1c;
static  cy_stc_pdma_descr_t         stcDescr1d;
static  cy_stc_pdma_descr_t         stcDescr11b;
static  cy_stc_pdma_descr_t         stcDescr11c;
static  cy_stc_pdma_descr_t         stcDescr11d;

/* DMA - CRC seed value */
static  uint32_t                    u32CrcSeed      =   0xff000000;

/* DMA - CRC configuration */
const   cy_stc_pdma_crc_config_t    stcCrcConfig    =   {
                                                            .data_reverse = 1,                  
                                                            .rem_reverse  = 1,              
                                                            .data_xor     = 0,                      
                                                            .polynomial   = 0x2f << 24,             
                                                            .lfsr32       = 0xff000000,                     
                                                            .rem_xor      = 0xff000000,                       
                                                        };

/* DMA - Descriptor configurations for CRC */
/* Initialize CRC engine with correct seed value before any CRC calculation */
static  cy_stc_pdma_descr_config_t  stcDmaDescrConfig1a =  {
                                                            .deact          =   0,
                                                            .intrType       =   CY_PDMA_INTR_DESCRCHAIN_CMPLT,
                                                            .trigoutType    =   CY_PDMA_TRIGOUT_DESCRCHAIN_CMPLT,
                                                            .chStateAtCmplt =   CY_PDMA_CH_ENABLED, 
                                                            .triginType     =   CY_PDMA_TRIGIN_DESCRCHAIN,
                                                            .dataSize       =   CY_PDMA_WORD,
                                                            .srcTxfrSize    =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .destTxfrSize   =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .descrType      =   CY_PDMA_SINGLE_TRANSFER,
                                                            .srcAddr        =   (void*) &u32CrcSeed,
                                                            .destAddr       =   (void*) &PDMA_INSTANCE->unCRC_LFSR_CTL.u32Register,
                                                            .srcXincr       =   0, // don't care
                                                            .destXincr      =   0, // don't care
                                                            .xCount         =   0, // don't care
                                                            .srcYincr       =   0, // don't care
                                                            .destYincr      =   0, // don't care
                                                            .yCount         =   0, // don't care
                                                            .descrNext      =   &stcDescr1b,
                                                        };

/* Calculate CRC */
static  cy_stc_pdma_descr_config_t  stcDmaDescrConfig1b =  {
                                                            .deact          =   0,
                                                            .intrType       =   CY_PDMA_INTR_DESCRCHAIN_CMPLT,
                                                            .trigoutType    =   CY_PDMA_TRIGOUT_DESCRCHAIN_CMPLT,
                                                            .chStateAtCmplt =   CY_PDMA_CH_ENABLED, 
                                                            .triginType     =   CY_PDMA_TRIGIN_DESCRCHAIN,
                                                            .dataSize       =   CY_PDMA_BYTE,
                                                            .srcTxfrSize    =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .destTxfrSize   =   CY_PDMA_TXFR_SIZE_WORD,
                                                            .descrType      =   CY_PDMA_CRC_TRANSFER,
                                                            .srcAddr        =   (void*) &au8TxBuffer.u32Word,
                                                            .destAddr       =   (void*) &s_deviceDriver.un_CrcResult.u32Word, // note: destinationwill be overwritten by next descriptor with CRC post-processed result
                                                            .srcXincr       =   1,
                                                            .destXincr      =   0,
                                                            .xCount         =   3,
                                                            .srcYincr       =   0, // don't care
                                                            .destYincr      =   0, // don't care
                                                            .yCount         =   0, // don't care
                                                            .descrNext      =   &stcDescr1c,
                                                        };

/* Copy post-processed CRC result from register to temporary buffer in SRAM (register can only be accessed with 32-bit!) */
static  cy_stc_pdma_descr_config_t  stcDmaDescrConfig1c =  {
                                                            .deact          =   0,
                                                            .intrType       =   CY_PDMA_INTR_DESCRCHAIN_CMPLT,
                                                            .trigoutType    =   CY_PDMA_TRIGOUT_DESCRCHAIN_CMPLT,
                                                            .chStateAtCmplt =   CY_PDMA_CH_ENABLED, 
                                                            .triginType     =   CY_PDMA_TRIGIN_DESCRCHAIN,
                                                            .dataSize       =   CY_PDMA_WORD,
                                                            .srcTxfrSize    =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .destTxfrSize   =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .descrType      =   CY_PDMA_SINGLE_TRANSFER,
                                                            .srcAddr        =   (void*) &PDMA_INSTANCE->unCRC_REM_RESULT.u32Register,
                                                            .destAddr       =   (void*) &s_deviceDriver.un_CrcResult.u32Word,
                                                            .srcXincr       =   0, // don't care
                                                            .destXincr      =   0, // don't care
                                                            .xCount         =   0, // don't care
                                                            .srcYincr       =   0, // don't care
                                                            .destYincr      =   0, // don't care
                                                            .yCount         =   0, // don't care
                                                            .descrNext      =   &stcDescr1d,
                                                        };

/* Copy single CRC byte into correct location of SPI TX buffer */
static  cy_stc_pdma_descr_config_t  stcDmaDescrConfig1d =  {
                                                            .deact          =   0,
                                                            .intrType       =   CY_PDMA_INTR_DESCRCHAIN_CMPLT,
                                                            .trigoutType    =   CY_PDMA_TRIGOUT_DESCRCHAIN_CMPLT,
                                                            .chStateAtCmplt =   CY_PDMA_CH_ENABLED, 
                                                            .triginType     =   CY_PDMA_TRIGIN_DESCRCHAIN,
                                                            .dataSize       =   CY_PDMA_BYTE,
                                                            .srcTxfrSize    =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .destTxfrSize   =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .descrType      =   CY_PDMA_SINGLE_TRANSFER,
                                                            .srcAddr        =   (void*) &s_deviceDriver.un_CrcResult.au8Bytes[0],
                                                            .destAddr       =   (void*) &au8TxBuffer.au8Bytes[3],
                                                            .srcXincr       =   0, // don't care
                                                            .destXincr      =   0, // don't care
                                                            .xCount         =   0, // don't care
                                                            .srcYincr       =   0, // don't care
                                                            .destYincr      =   0, // don't care
                                                            .yCount         =   0, // don't care
                                                            .descrNext      =   &stcDescr1e,
                                                        };

/* Copy SPI TX buffer into SPI TX FIFO */
static  cy_stc_pdma_descr_config_t  stcDmaDescrConfig1e =  {
                                                            .deact          =   0,
                                                            .intrType       =   CY_PDMA_INTR_DESCRCHAIN_CMPLT,
                                                            .trigoutType    =   CY_PDMA_TRIGOUT_DESCRCHAIN_CMPLT,
                                                            .chStateAtCmplt =   CY_PDMA_CH_DISABLED, 
                                                            .triginType     =   CY_PDMA_TRIGIN_DESCRCHAIN,
                                                            .dataSize       =   CY_PDMA_WORD,
                                                            .srcTxfrSize    =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .destTxfrSize   =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .descrType      =   CY_PDMA_SINGLE_TRANSFER,
                                                            .srcAddr        =   (void*) &au8TxBuffer.u32Word,
                                                            .destAddr       =   (void*) &CY_SPI_SCB_TYPE->unTX_FIFO_WR.u32Register,
                                                            .srcXincr       =   0, // don't care
                                                            .destXincr      =   0, // don't care
                                                            .xCount         =   0, // don't care
                                                            .srcYincr       =   0, // don't care
                                                            .destYincr      =   0, // don't care
                                                            .yCount         =   0, // don't care
                                                            .descrNext      =   0,
                                                        };

/* Copy SPI RX FIFO into receive buffer and continue with CRC calculation */
static  cy_stc_pdma_descr_config_t  stcDmaDescrConfig11a =  {
                                                            .deact          =   0,
                                                            .intrType       =   CY_PDMA_INTR_DESCRCHAIN_CMPLT,
                                                            .trigoutType    =   CY_PDMA_TRIGOUT_DESCRCHAIN_CMPLT,
                                                            .chStateAtCmplt =   CY_PDMA_CH_ENABLED, 
                                                            .triginType     =   CY_PDMA_TRIGIN_DESCRCHAIN,
                                                            .dataSize       =   CY_PDMA_WORD,
                                                            .srcTxfrSize    =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .destTxfrSize   =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .descrType      =   CY_PDMA_SINGLE_TRANSFER,
                                                            .srcAddr        =   (void*) &CY_SPI_SCB_TYPE->unRX_FIFO_RD.u32Register,
                                                            .destAddr       =   (void*) &s_deviceDriver.un_SpiRx.u32Word,
                                                            .srcXincr       =   0, // don't care
                                                            .destXincr      =   0, // don't care
                                                            .xCount         =   0, // don't care
                                                            .srcYincr       =   0, // don't care
                                                            .destYincr      =   0, // don't care
                                                            .yCount         =   0, // don't care
                                                            .descrNext      =   &stcDescr11b,
                                                        };

/* Initialize CRC engine with correct seed value before any CRC calculation */
static  cy_stc_pdma_descr_config_t  stcDmaDescrConfig11b =  {
                                                            .deact          =   0,
                                                            .intrType       =   CY_PDMA_INTR_DESCRCHAIN_CMPLT,
                                                            .trigoutType    =   CY_PDMA_TRIGOUT_DESCRCHAIN_CMPLT,
                                                            .chStateAtCmplt =   CY_PDMA_CH_ENABLED, 
                                                            .triginType     =   CY_PDMA_TRIGIN_DESCRCHAIN,
                                                            .dataSize       =   CY_PDMA_WORD,
                                                            .srcTxfrSize    =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .destTxfrSize   =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .descrType      =   CY_PDMA_SINGLE_TRANSFER,
                                                            .srcAddr        =   (void*) &u32CrcSeed,
                                                            .destAddr       =   (void*) &PDMA_INSTANCE->unCRC_LFSR_CTL.u32Register,
                                                            .srcXincr       =   0, // don't care
                                                            .destXincr      =   0, // don't care
                                                            .xCount         =   0, // don't care
                                                            .srcYincr       =   0, // don't care
                                                            .destYincr      =   0, // don't care
                                                            .yCount         =   0, // don't care
                                                            .descrNext      =   &stcDescr11c,
                                                        };

/* Calculate CRC */
/* note: destination will be overwritten by next descriptor with CRC post-processed result */
static  cy_stc_pdma_descr_config_t  stcDmaDescrConfig11c =  {
                                                            .deact          =   0,
                                                            .intrType       =   CY_PDMA_INTR_DESCRCHAIN_CMPLT,
                                                            .trigoutType    =   CY_PDMA_TRIGOUT_DESCRCHAIN_CMPLT,
                                                            .chStateAtCmplt =   CY_PDMA_CH_ENABLED, 
                                                            .triginType     =   CY_PDMA_TRIGIN_DESCRCHAIN,
                                                            .dataSize       =   CY_PDMA_BYTE,
                                                            .srcTxfrSize    =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .destTxfrSize   =   CY_PDMA_TXFR_SIZE_WORD,
                                                            .descrType      =   CY_PDMA_CRC_TRANSFER,
                                                            .srcAddr        =   (void*) &s_deviceDriver.un_SpiRx.u32Word,
                                                            .destAddr       =   (void*) &s_deviceDriver.un_CrcResult.u32Word,
                                                            .srcXincr       =   1,
                                                            .destXincr      =   0,
                                                            .xCount         =   3,
                                                            .srcYincr       =   0, // don't care
                                                            .destYincr      =   0, // don't care
                                                            .yCount         =   0, // don't care
                                                            .descrNext      =   &stcDescr11d,
                                                        };

/* Copy post-processed CRC result from register to temporary buffer in SRAM (register can only be accessed with 32-bit!) */
static  cy_stc_pdma_descr_config_t  stcDmaDescrConfig11d =  {
                                                            .deact          =   0,
                                                            .intrType       =   CY_PDMA_INTR_DESCRCHAIN_CMPLT,
                                                            .trigoutType    =   CY_PDMA_TRIGOUT_DESCRCHAIN_CMPLT,
                                                            .chStateAtCmplt =   CY_PDMA_CH_DISABLED, 
                                                            .triginType     =   CY_PDMA_TRIGIN_DESCRCHAIN,
                                                            .dataSize       =   CY_PDMA_WORD,
                                                            .srcTxfrSize    =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .destTxfrSize   =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .descrType      =   CY_PDMA_SINGLE_TRANSFER,
                                                            .srcAddr        =   (void*) &PDMA_INSTANCE->unCRC_REM_RESULT.u32Register,
                                                            .destAddr       =   (void*) &s_deviceDriver.un_CrcResult.u32Word,
                                                            .srcXincr       =   0, // don't care
                                                            .destXincr      =   0, // don't care
                                                            .xCount         =   0, // don't care
                                                            .srcYincr       =   0, // don't care
                                                            .destYincr      =   0, // don't care
                                                            .yCount         =   0, // don't care
                                                            .descrNext      =   0,
                                                        };

#elif (TLE9563_CRC_EN == 0)
/* Copy SPI TX buffer into SPI TX FIFO */
static  cy_stc_pdma_descr_config_t  stcDmaDescrConfig1e =  {
                                                            .deact          =   0,
                                                            .intrType       =   CY_PDMA_INTR_1ELEMENT_CMPLT,
                                                            .trigoutType    =   CY_PDMA_TRIGOUT_1ELEMENT_CMPLT,
                                                            .chStateAtCmplt =   CY_PDMA_CH_DISABLED, 
                                                            .triginType     =   CY_PDMA_TRIGIN_1ELEMENT,
                                                            .dataSize       =   CY_PDMA_WORD,
                                                            .srcTxfrSize    =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .destTxfrSize   =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .descrType      =   CY_PDMA_SINGLE_TRANSFER,
                                                            .srcAddr        =   (void*) &au8TxBuffer.u32Word,
                                                            .destAddr       =   (void*) &CY_SPI_SCB_TYPE->unTX_FIFO_WR.u32Register,
                                                            .srcXincr       =   0, // don't care
                                                            .destXincr      =   0, // don't care
                                                            .xCount         =   0, // don't care
                                                            .srcYincr       =   0, // don't care
                                                            .destYincr      =   0, // don't care
                                                            .yCount         =   0, // don't care
                                                            .descrNext      =   0,
                                                        };

/* Copy SPI RX FIFO into receive buffer without calculating the CRC afterwards */
static  cy_stc_pdma_descr_config_t  stcDmaDescrConfig11a =  {
                                                            .deact          =   0,
                                                            .intrType       =   CY_PDMA_INTR_1ELEMENT_CMPLT,
                                                            .trigoutType    =   CY_PDMA_TRIGOUT_1ELEMENT_CMPLT,
                                                            .chStateAtCmplt =   CY_PDMA_CH_DISABLED, 
                                                            .triginType     =   CY_PDMA_TRIGIN_1ELEMENT,
                                                            .dataSize       =   CY_PDMA_WORD,
                                                            .srcTxfrSize    =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .destTxfrSize   =   CY_PDMA_TXFR_SIZE_DATA_SIZE,
                                                            .descrType      =   CY_PDMA_SINGLE_TRANSFER,
                                                            .srcAddr        =   (void*) &CY_SPI_SCB_TYPE->unRX_FIFO_RD.u32Register,
                                                            .destAddr       =   (void*) &s_deviceDriver.un_SpiRx.u32Word,
                                                            .srcXincr       =   0, // don't care
                                                            .destXincr      =   0, // don't care
                                                            .xCount         =   0, // don't care
                                                            .srcYincr       =   0, // don't care
                                                            .destYincr      =   0, // don't care
                                                            .yCount         =   0, // don't care
                                                            .descrNext      =   0,
                                                        };
#endif

/* DMA - Channel configuration */
#if  (TLE9563_CRC_EN == 1)
const   cy_stc_pdma_chnl_config_t   chnlConfig1      =   {
                                                            .PDMA_Descriptor=   &stcDescr1a, // start descriptor chain with CRC calculation
                                                            .preemptable    =   0,
                                                            .priority       =   0,
                                                            .enable         =   0,  // keep disabled after initialization
                                                        };
#elif (TLE9563_CRC_EN == 0)
const   cy_stc_pdma_chnl_config_t   chnlConfig1      =   {
                                                            .PDMA_Descriptor=   &stcDescr1e, // start SPI transfer without CRC calculation
                                                            .preemptable    =   0,
                                                            .priority       =   0,
                                                            .enable         =   0,  // keep disabled after initialization
                                                        };
#endif
const   cy_stc_pdma_chnl_config_t   chnlConfig11      =   {
                                                            .PDMA_Descriptor=   &stcDescr11a,
                                                            .preemptable    =   0,
                                                            .priority       =   0,
                                                            .enable         =   0,  // keep disabled after initialization
                                                        };

/******************************************************************************/
/**                        Local Function Definitions                        **/
/******************************************************************************/


/*******************************************************************************
**                        Global Variable Definitions                         **
*******************************************************************************/


/*******************************************************************************
**                         Global Function Definitions                        **
*******************************************************************************/
/** \brief Set Peripheral Fraction Divider 24_5
 *
 */
void SetPeripheFracDiv24_5(uint64_t targetFreq, uint64_t sourceFreq, uint8_t divNum)
{
    uint64_t temp = ((uint64_t)sourceFreq << 5ull);
    uint32_t divSetting;

    divSetting = (uint32_t)(temp / targetFreq);
    Cy_SysClk_PeriphSetFracDivider(CY_SYSCLK_DIV_24_5_BIT, divNum, 
                                   (((divSetting >> 5ul) & 0x00000FFFul) - 1ul), 
                                   (divSetting & 0x0000001Ful));
}

/** \brief Initialize the SPI
 *
 */
void TLE9563_initSpi(void)
{
    /******************************************************/
    /******* Calculate divider setting for the SCB ********/
    /******************************************************/
    Cy_SysClk_PeriphAssignDivider(CY_SPI_SCB_PCLK, CY_SYSCLK_DIV_24_5_BIT, DIVIDER_NO_1);
    SetPeripheFracDiv24_5(SCB_SPI_CLOCK_FREQ, SOURCE_CLOCK_FRQ, DIVIDER_NO_1);
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_24_5_BIT, 1u); 

    /********************************************/
    /*    De-initialization for peripherals     */
    /********************************************/
    Cy_SCB_SPI_DeInit(CY_SPI_SCB_TYPE);

    /**************************************/
    /* Port Setting for SPI communication */
    /**************************************/
    /* According to the HW environment to change SCB CH*/
    SPI_port_pin_cfg.driveMode = SCB_MISO_DRIVE_MODE;
    SPI_port_pin_cfg.hsiom = CY_SPI_SCB_MISO_MUX;
    Cy_GPIO_Pin_Init(CY_SPI_SCB_MISO_PORT, CY_SPI_SCB_MISO_PIN, &SPI_port_pin_cfg);

    SPI_port_pin_cfg.driveMode = SCB_MOSI_DRIVE_MODE;
    SPI_port_pin_cfg.hsiom = CY_SPI_SCB_MOSI_MUX;
    Cy_GPIO_Pin_Init(CY_SPI_SCB_MOSI_PORT, CY_SPI_SCB_MOSI_PIN, &SPI_port_pin_cfg);

    SPI_port_pin_cfg.driveMode = SCB_CLK_DRIVE_MODE;
//    SPI_port_pin_cfg.hsiom = CY_SPI_SCB_CLK_MUX;
    Cy_GPIO_Pin_Init(CY_SPI_SCB_CLK_PORT,CY_SPI_SCB_CLK_PIN, &SPI_port_pin_cfg);
    
    SPI_port_pin_cfg.driveMode = SCB_SEL0_DRIVE_MODE;
    SPI_port_pin_cfg.hsiom = CY_SPI_SCB_SEL0_MUX;
    Cy_GPIO_Pin_Init(CY_SPI_SCB_SEL0_PORT, CY_SPI_SCB_SEL0_PIN, &SPI_port_pin_cfg);

    /********************************************/
    /* SCB initialization for SPI communication */
    /********************************************/
    Cy_SCB_SPI_Init(CY_SPI_SCB_TYPE, &SCB_SPI_cfg, NULL);
    Cy_SCB_SPI_SetActiveSlaveSelect(CY_SPI_SCB_TYPE, 0ul);
    Cy_SCB_SPI_Enable(CY_SPI_SCB_TYPE);
}

/** \brief Initialize the PDMA
 * \brief Descriptors are initialized
 * \brief SPI RX output trigger is connected to trigger input of pdma channel 11
 *
 */
void TLE9563_initDma(void)
{
  /******/
  /* DW */
  /******/  
  Cy_PDMA_Disable(PDMA_INSTANCE);
#if  (TLE9563_CRC_EN == 1)
  Cy_PDMA_CRC_Config(PDMA_INSTANCE, &stcCrcConfig);
  Cy_PDMA_Descr_Init(&stcDescr1a, &stcDmaDescrConfig1a);
  Cy_PDMA_Descr_Init(&stcDescr1b, &stcDmaDescrConfig1b);
  Cy_PDMA_Descr_Init(&stcDescr1c, &stcDmaDescrConfig1c);
  Cy_PDMA_Descr_Init(&stcDescr1d, &stcDmaDescrConfig1d);
  Cy_PDMA_Descr_Init(&stcDescr11b, &stcDmaDescrConfig11b);
  Cy_PDMA_Descr_Init(&stcDescr11c, &stcDmaDescrConfig11c);
  Cy_PDMA_Descr_Init(&stcDescr11d, &stcDmaDescrConfig11d);
#endif
  Cy_PDMA_Descr_Init(&stcDescr1e, &stcDmaDescrConfig1e); 
  Cy_PDMA_Descr_Init(&stcDescr11a, &stcDmaDescrConfig11a);
  Cy_PDMA_Chnl_SetInterruptMask(PDMA_INSTANCE, PDMA_CH11);
  Cy_PDMA_Enable(PDMA_INSTANCE);
  
  /***************/
  /* Trigger MUX */
  /***************/
  Cy_TrigMux_Connect1To1(TRIG_OUT_1TO1_8_SCB_RX_TO_PDMA11, 0, TRIGGER_TYPE_CPUSS_DW1_TR_IN__EDGE, 0);
}

/** \brief Send out SPI read message
 * \brief PDMA channels are initialized and enabled
 * \brief PDMA channel 1 is triggered
 *
 * \param u8_address address of the register
 *
 */
void TLE9563_getReg(uint8_t u8_address)
{
  /************************* Build 32 bit SPI message *************************/
  /** bit[0:6]:       address                                                **/
  /** bit[7]:         ACTION (0: read)                                       **/
  /** bit[8:23]:      0x0000, no data                                        **/
  /** bit[24:31]:     8bit crc or static pattern                             **/

  au8TxBuffer.au8Bytes[0] = u8_address | (uint8_t)TLE9563_SPI_ACTION_READ;
  au8TxBuffer.au8Bytes[1] = 0x00;
  au8TxBuffer.au8Bytes[2] = 0x00;
  // CRC static pattern will be overwriten, if CRC is calculated
  au8TxBuffer.au8Bytes[3] = TLE9563_CRC_STATIC_PATTERN_MOSI;

  // Initialize channel 1 and 11 with according descriptors
  Cy_PDMA_Chnl_Init(PDMA_INSTANCE, PDMA_CH1, (const cy_stc_pdma_chnl_config_t*) &chnlConfig1);
  Cy_PDMA_Chnl_Init(PDMA_INSTANCE, PDMA_CH11, (const cy_stc_pdma_chnl_config_t*) &chnlConfig11);
  // Clear interrupt of channel 1, interrupt for channel 11 is cleared in isr
  Cy_PDMA_Chnl_ClearInterrupt(PDMA_INSTANCE, PDMA_CH1);
  // Enable channel 1 and 11, channels are disabled after descriptor chain completion
  Cy_PDMA_Chnl_Enable(PDMA_INSTANCE, PDMA_CH1);
  Cy_PDMA_Chnl_Enable(PDMA_INSTANCE, PDMA_CH11);
  // Trigger DMA channel 1 by software, channel 11 will be triggered by spi rx
  PDMA_INSTANCE->CH_STRUCT[PDMA_CH1].unTR_CMD.u32Register = 1;
}

/** \brief Send out SPI write message
 * \brief PDMA channels are initialized and enabled
 * \brief PDMA channel 1 is triggered
 *
 * \param u8_address address of the register
 * \param u16_data data to be written
 *
 */
void TLE9563_setReg(uint8_t u8_address, uint16_t u16_data)
{
  /************************* Build 32 bit SPI message *************************/
  /** bit[0:6]:       address                                                **/
  /** bit[7]:         ACTION (1: write)                                      **/
  /** bit[8:23]:      data to be written                                     **/
  /** bit[24:31]:     8bit crc or static pattern                             **/
  
  au8TxBuffer.au8Bytes[0] = u8_address | (uint8_t)TLE9563_SPI_ACTION_WRITE;
  au8TxBuffer.au8Bytes[1] = u16_data;
  au8TxBuffer.au8Bytes[2] = u16_data >> 8;
  // CRC static pattern will be overwriten, if CRC is calculated
  au8TxBuffer.au8Bytes[3] = TLE9563_CRC_STATIC_PATTERN_MOSI;

  // Initialize channel 1 and 11 with according descriptors
  Cy_PDMA_Chnl_Init(PDMA_INSTANCE, PDMA_CH1, (const cy_stc_pdma_chnl_config_t*) &chnlConfig1);
  Cy_PDMA_Chnl_Init(PDMA_INSTANCE, PDMA_CH11, (const cy_stc_pdma_chnl_config_t*) &chnlConfig11);
  // Clear interrupt of channel 1, interrupt for channel 11 is cleared in isr
  Cy_PDMA_Chnl_ClearInterrupt(PDMA_INSTANCE, PDMA_CH1);
  // Enable channel 1 and 11, channels are disabled after descriptor chain completion
  Cy_PDMA_Chnl_Enable(PDMA_INSTANCE, PDMA_CH1);
  Cy_PDMA_Chnl_Enable(PDMA_INSTANCE, PDMA_CH11);
  // Trigger DMA channel 1 by software, channel 11 will be triggered by spi rx
  PDMA_INSTANCE->CH_STRUCT[PDMA_CH1].unTR_CMD.u32Register = 1;
}

/** \brief Send out SPI write message for banked register
 * \brief PDMA channels are initialized and enabled
 * \brief PDMA channel 1 is triggered
 *
 * \param u8_address address of the register
 * \param e_bank register bank
 * \param u16_data data to be written
 *
 */
void TLE9563_setBankReg(uint8_t u8_address, tDEVICE_regBank e_bank, uint16_t u16_data)
{
  /************************* Build 32 bit SPI message *************************/
  /** bit[0:6]:       address                                                **/
  /** bit[7]:         ACTION (1: write)                                      **/
  /** bit[8:10]:      register bank                                          **/
  /** bit[11]:        0, reserved                                            **/
  /** bit[12:23]:     data to be written                                     **/
  /** bit[24:31]:     8bit crc or static pattern                             **/

  au8TxBuffer.au8Bytes[0] = u8_address | (uint8_t)TLE9563_SPI_ACTION_WRITE;
  au8TxBuffer.au8Bytes[1] = u16_data | e_bank;
  au8TxBuffer.au8Bytes[2] = u16_data >> 8;
  // CRC static pattern will be overwriten, if CRC is calculated
  au8TxBuffer.au8Bytes[3] = TLE9563_CRC_STATIC_PATTERN_MOSI;

  // Initialize channel 1 and 11 with according descriptors
  Cy_PDMA_Chnl_Init(PDMA_INSTANCE, PDMA_CH1, (const cy_stc_pdma_chnl_config_t*) &chnlConfig1);
  Cy_PDMA_Chnl_Init(PDMA_INSTANCE, PDMA_CH11, (const cy_stc_pdma_chnl_config_t*) &chnlConfig11);
  // Clear interrupt of channel 1, interrupt for channel 11 is cleared in isr
  Cy_PDMA_Chnl_ClearInterrupt(PDMA_INSTANCE, PDMA_CH1);
  // Enable channel 1 and 11, channels are disabled after descriptor chain completion
  Cy_PDMA_Chnl_Enable(PDMA_INSTANCE, PDMA_CH1);
  Cy_PDMA_Chnl_Enable(PDMA_INSTANCE, PDMA_CH11);
  // Trigger DMA channel 1 by software, channel 11 will be triggered by spi rx
  PDMA_INSTANCE->CH_STRUCT[PDMA_CH1].unTR_CMD.u32Register = 1;
}

/** \brief Send out SPI read message for banked register
 * \brief PDMA channels are initialized and enabled
 * \brief PDMA channel 1 is triggered
 *
 * \param u8_address address of the register
 * \param e_bank register bank
 *
 */
void TLE9563_getBankReg(uint8_t u8_address, tDEVICE_regBank e_bank)
{
  /************************* Build 32 bit SPI message *************************/
  /** bit[0:6]:       address                                                **/
  /** bit[7]:         ACTION (0: read)                                       **/
  /** bit[8:10]:      register bank                                          **/
  /** bit[11]:        0, reserved                                            **/
  /** bit[12:23]:     0x0000, no data                                        **/
  /** bit[24:31]:     8bit crc or static pattern                             **/

  au8TxBuffer.au8Bytes[0] = u8_address | (uint8_t)TLE9563_SPI_ACTION_WRITE;
  au8TxBuffer.au8Bytes[1] = 0x00 | e_bank;
  au8TxBuffer.au8Bytes[2] = 0x00;
  // CRC static pattern will be overwriten, if CRC is calculated
  au8TxBuffer.au8Bytes[3] = TLE9563_CRC_STATIC_PATTERN_MOSI;

  // Initialize channel 1 and 11 with according descriptors
  Cy_PDMA_Chnl_Init(PDMA_INSTANCE, PDMA_CH1, (const cy_stc_pdma_chnl_config_t*) &chnlConfig1);
  Cy_PDMA_Chnl_Init(PDMA_INSTANCE, PDMA_CH11, (const cy_stc_pdma_chnl_config_t*) &chnlConfig11);
  // Clear interrupt of channel 1, interrupt for channel 11 is cleared in isr
  Cy_PDMA_Chnl_ClearInterrupt(PDMA_INSTANCE, PDMA_CH1);
  // Enable channel 1 and 11, channels are disabled after descriptor chain completion
  Cy_PDMA_Chnl_Enable(PDMA_INSTANCE, PDMA_CH1);
  Cy_PDMA_Chnl_Enable(PDMA_INSTANCE, PDMA_CH11);
  // Trigger DMA channel 1 by software, channel 11 will be triggered by spi rx
  PDMA_INSTANCE->CH_STRUCT[PDMA_CH1].unTR_CMD.u32Register = 1;
}

/** \brief Send out static pattern via SPI
 * \brief Static pattern is used for CRC recovery
 * \brief PDMA channel 11 is initialized and enabled
 * \brief Data is written directly into SPI TX FIFO, no crc calculation needed by PDMA
 *
 */
void TLE9563_sendStaticCrcRecovery(void)
{ 
  /************************** Build 32 bit SPI message **************************/
  /** bit[0:7]:   addr + rw_bit (MSB): 0x67 to enable CRC, 0xE7 to disable CRC **/
  /** bit[12:23]: data (MSB):          0xAAAA                                  **/
  /** bit[24:31]: crc (MSB):           0x0E to enable CRC, 0xC3 to disable CRC **/

  au8TxBuffer.au8Bytes[1] = 0x55;
  au8TxBuffer.au8Bytes[2] = 0x55;
  
  #if (TLE9563_CRC_EN == 1)
    au8TxBuffer.au8Bytes[0] = 0xE6;
    au8TxBuffer.au8Bytes[3] = 0x70;
  #else
    au8TxBuffer.au8Bytes[0] = 0xE7;
    au8TxBuffer.au8Bytes[3] = 0xC3;
  #endif
    
  // Initialize channel 11 with according descriptors
  Cy_PDMA_Chnl_Init(PDMA_INSTANCE, PDMA_CH11, (const cy_stc_pdma_chnl_config_t*) &chnlConfig11);
  // Enable channel 11, channels are disabled after descriptor chain completion
  Cy_PDMA_Chnl_Enable(PDMA_INSTANCE, PDMA_CH11);
    
  // Write data directly into SPI TX FIFO
  CY_SPI_SCB_TYPE->unTX_FIFO_WR.u32Register = au8TxBuffer.u32Word;
}

/** @}*/ /* End of group TLE9563_COMMLAYER */

