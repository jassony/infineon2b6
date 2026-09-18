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
/**
 * \file     TLE9563_CommLayer.h
 *
 * \brief    TLE9563 Communication Layer Functions
 *
 * \version  V0.1.5
 * \date     06 Apr 2022
 *
 * \note 
 */

/*******************************************************************************
**                             Author(s) Identity                             **
********************************************************************************
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** VO           Vanessa Ongaro                                                **
*******************************************************************************/

/*******************************************************************************
**                          Revision Control History                          **
********************************************************************************
** V0.1.0: 2021-12-06, VO:   Initial version                                  **
** V0.1.1: 2022-01-05, VO:   [EP-994] Completed isr DW1_IntHandler            **
** V0.1.2: 2022-01-19, VO:   [EP-994] Removed isr DW1_IntHandler, cyclic task **
**                           polls status of PDMA instead                     **
**                           Moved structs and Spi, Crc buffers to Function   **
**                           Layer                                            **
**                           Deleted commented out lines and added briefs     **
** V0.1.3: 2022-01-27, VO:   Moved SPI configs into Communication layer       **
** V0.1.4: 2022-03-09, VO:   [EP-1038] changed setPeripheFracDiv24_5 from a   **
**                           local to a global function                       **
**                           Removed SPI interrupt configuration              **
**                           Changed SPI instance to SCB 1 on port 18, and    **
**                           DMA channel 21 to channel 11 accordingly         **
**                           Removed commented code                           **
** V0.1.5: 2022-04-06, VO:   [EP-1038] Corrected descriptor config in case no **
**                           CRC is used                                      **
*******************************************************************************/

#ifndef TLE9563_COMMLAYER_H
#define TLE9563_COMMLAYER_H

/** \addtogroup TLE9563_COMMLAYER
 *  @{
 */

/*******************************************************************************
**                                  Includes                                  **
*******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "TLE9563_RegLayer.h"
#include "cy_project.h"
#include "cy_device_headers.h"

/*******************************************************************************
**                          Global Macro Declarations                         **
*******************************************************************************/
/* SPI - Message */
#define TLE9563_SPI_ACTION_WRITE           0x80
#define TLE9563_SPI_ACTION_READ            0x00
#define TLE9563_CRC_STATIC_PATTERN_MOSI    0xA5
#define TLE9563_CRC_STATIC_PATTERN_MISO    0x5A

/* SPI - Device Specific Settings */
#define CY_SPI_SCB_TYPE      SCB1
#define CY_SPI_SCB_MISO_PORT GPIO_PRT18
#define CY_SPI_SCB_MISO_PIN  0ul
#define CY_SPI_SCB_MISO_MUX  P18_0_SCB1_SPI_MISO
#define CY_SPI_SCB_MOSI_PORT GPIO_PRT18
#define CY_SPI_SCB_MOSI_PIN  1ul
#define CY_SPI_SCB_MOSI_MUX  P18_1_SCB1_SPI_MOSI
#define CY_SPI_SCB_CLK_PORT  GPIO_PRT18
#define CY_SPI_SCB_CLK_PIN   2ul
#define CY_SPI_SCB_CLK_MUX   P18_2_SCB1_SPI_CLK
#define CY_SPI_SCB_SEL0_PORT GPIO_PRT18
#define CY_SPI_SCB_SEL0_PIN  3ul
#define CY_SPI_SCB_SEL0_MUX  P18_3_SCB1_SPI_SELECT0
#define CY_SPI_SCB_PCLK      PCLK_SCB1_CLOCK

/* SPI - Master Settings */
#define SCB_MISO_DRIVE_MODE CY_GPIO_DM_HIGHZ
#define SCB_MOSI_DRIVE_MODE CY_GPIO_DM_STRONG_IN_OFF
#define SCB_CLK_DRIVE_MODE  CY_GPIO_DM_STRONG_IN_OFF
#define SCB_SEL0_DRIVE_MODE CY_GPIO_DM_STRONG_IN_OFF

/* SPI - User setting value */
#if (CY_USE_PSVP == 1)  
  #define SOURCE_CLOCK_FRQ 24000000ul
  #define CORE_CLOCK_FRQ   24000000ul
#else
  #define SOURCE_CLOCK_FRQ 80000000ul
  #define CORE_CLOCK_FRQ   80000000ul
#endif

#define SCB_SPI_BAUDRATE     5000000ul
#define SCB_SPI_OVERSAMPLING 16ul
#define SCB_SPI_CLOCK_FREQ (SCB_SPI_BAUDRATE * SCB_SPI_OVERSAMPLING)
#define DIVIDER_NO_1 (1u)

/* DMA */
#define PDMA_INSTANCE                      DW1
#define PDMA_CH1                           1
#define PDMA_CH11                          11

/*******************************************************************************
**                          Global Type Declarations                          **
*******************************************************************************/
   
   
/*******************************************************************************
**                        Global Variable Declarations                        **
*******************************************************************************/


/*******************************************************************************
**                        Global Function Declarations                        **
*******************************************************************************/
void TLE9563_initSpi(void);
void TLE9563_initDma(void);

void SetPeripheFracDiv24_5(uint64_t targetFreq, uint64_t sourceFreq, uint8_t divNum);

void TLE9563_setReg(uint8_t u8_address, uint16_t u16_data);
void TLE9563_getReg(uint8_t u8_address);
void TLE9563_setBankReg(uint8_t u8_address, tDEVICE_regBank e_value, uint16_t u16_data);
void TLE9563_getBankReg(uint8_t u8_address, tDEVICE_regBank e_value);
void TLE9563_sendStaticCrcRecovery(void);

/** @}*/ /* End of group TLE9563_COMMLAYER */

#endif /* TLE9563_COMMLAYER_H */

