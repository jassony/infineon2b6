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


/*******************************************************************************
**                                  Includes                                  **
*******************************************************************************/
#include "TLE9563_RegLayer.h"
#include "TLE9563_CommLayer.h"

/*******************************************************************************
**                           Local Macro Definitions                          **
*******************************************************************************/


/*******************************************************************************
**                           Local Type Definitions                           **
*******************************************************************************/


/******************************************************************************/
/**                        Local Variable Definitions                        **/
/******************************************************************************/


/******************************************************************************/
/**                        Local Function Definitions                        **/
/******************************************************************************/


/*******************************************************************************
**                        Global Variable Definitions                         **
*******************************************************************************/


/*******************************************************************************
**                         Global Function Definitions                        **
*******************************************************************************/
/** \brief Recovery SPI command in case of mismatch of CRC setting between the device and µC
 * 
 *  \return void no return value
 */
void TLE9563_resetCrc(uint16_t u16_data)
{
  TLE9563_sendStaticCrcRecovery();
}

/** \brief Sets the device register for Mode and Supply Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setMSCTRL(uint16_t u16_data)
{
  TLE9563->M_S_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_MSCTRL_ADDR, u16_data);
  
  /* in case of software reset, registers are reseted */
  if(TLE9563->M_S_CTRL.bit.MODE == 3)
  {
    TLE9563->M_S_CTRL.reg = 0x0000;
    TLE9563->HW_CTRL.reg = (uint16_t)TLE9563->HW_CTRL.reg & 0x0200;
    TLE9563->WD_CTRL.reg = 0x0014;
    TLE9563->BUS_CTRL.reg = 0x0020;
    TLE9563->WK_CTRL_BNK3.reg = 0x0023;
    TLE9563->WK_CTRL_BNK4.reg = 0x0024;
    TLE9563->TIMER_CTRL.reg = 0x0000;
    TLE9563->SW_SD_CTRL.reg = 0x0000;
    TLE9563->HS_CTRL.reg = 0x0000;
    TLE9563->INT_MASK.reg = 0x0120;
    TLE9563->PWM_CTRL_BNK0.reg = 0x0000;
    TLE9563->PWM_CTRL_BNK1.reg = 0x0001;
    TLE9563->PWM_CTRL_BNK2.reg = 0x0002;
    TLE9563->PWM_CTRL_BNK3.reg = 0x0003;
    TLE9563->SYS_STAT_CTRL.reg = 0x0000;
    TLE9563->GENCTRL.reg = 0x0801;
    TLE9563->CSA.reg = 0x0121;
    TLE9563->LS_VDS.reg = 0x0049;
    TLE9563->HS_VDS.reg = 0x0049;
    TLE9563->CCP_BLK_BNK0.reg = 0x7700;
    TLE9563->CCP_BLK_BNK1.reg = 0x7701;
    TLE9563->CCP_BLK_BNK2.reg = 0x7702;
    TLE9563->CCP_BLK_BNK4.reg = 0x7704;
    TLE9563->CCP_BLK_BNK5.reg = 0x7705;
    TLE9563->CCP_BLK_BNK6.reg = 0x7706;
    TLE9563->HBMODE.reg = 0x0222;
    TLE9563->TPRECHG_BNK0.reg = 0x0000;
    TLE9563->TPRECHG_BNK1.reg = 0x0001;
    TLE9563->ST_ICHG.reg = 0x0444;
    TLE9563->HB_ICHG_BNK0.reg = 0x3CD0;
    TLE9563->HB_ICHG_BNK1.reg = 0x3CD1;
    TLE9563->HB_ICHG_BNK2.reg = 0x3CD2;
    TLE9563->HB_ICHG_BNK4.reg = 0x3CD4;
    TLE9563->HB_ICHG_BNK5.reg = 0x3CD5;
    TLE9563->HB_ICHG_BNK6.reg = 0x3CD6;
    TLE9563->HB_ICHG_MAX.reg = 0x0000;
    TLE9563->HB_PCHG_INIT_BNK0.reg = 0x3CD0;
    TLE9563->HB_PCHG_INIT_BNK1.reg = 0x3CD1;
    TLE9563->HB_PCHG_INIT_BNK2.reg = 0x3CD2;
    TLE9563->TDON_HB_CTRL_BNK0.reg = 0x0C00;
    TLE9563->TDON_HB_CTRL_BNK1.reg = 0x0C01;
    TLE9563->TDON_HB_CTRL_BNK2.reg = 0x0C02;
    TLE9563->TDOFF_HB_CTRL_BNK0.reg = 0x0C00;
    TLE9563->TDOFF_HB_CTRL_BNK1.reg = 0x0C01;
    TLE9563->TDOFF_HB_CTRL_BNK2.reg = 0x0C02;
    TLE9563->BRAKE.reg = 0x00A0;
    TLE9563->SWK_CTRL.reg = 0x0000;
    TLE9563->SWK_BTL1_CTRL.reg = 0xCC96;
    TLE9563->SWK_ID0_CTRL.reg = 0x0000; 
    TLE9563->SWK_ID1_CTRL.reg = 0x0000;
    TLE9563->SWK_MASK_ID1_CTRL.reg = 0x0000;
    TLE9563->SWK_MASK_ID0_CTRL.reg = 0x0000;
    TLE9563->SWK_DATA3_CTRL.reg = 0x0000;
    TLE9563->SWK_DATA2_CTRL.reg = 0x0000; 
    TLE9563->SWK_DATA1_CTRL.reg = 0x0000; 
    TLE9563->SWK_DATA0_CTRL.reg = 0x0000;
    TLE9563->SWK_CAN_FD_CTRL.reg = 0x0000; 
    TLE9563->SWK_OSC_TRIM_CTRL.reg = 0x0000;
    TLE9563->SWK_CDR_CTRL.reg = 0x0004; 
    TLE9563->SWK_CDR_LIMIT.reg = 0x9D8F; 
    TLE9563->SWK_DLC_CTRL.reg = 0x0000; 
  }
}

/** \brief Gets the device register for Mode and Supply Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getMSCTRL(void)
{
  TLE9563_getReg(TLE9563_MSCTRL_ADDR);
}

/** \brief Sets the device register for Hardware Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHWCTRL(uint16_t u16_data)
{
  TLE9563->HW_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_HWCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for Hardware Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getHWCTRL(void)
{
  TLE9563_getReg(TLE9563_HWCTRL_ADDR);
}

/** \brief Sets the device register for Watchdog Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setWDCTRL(uint16_t u16_data)
{
  TLE9563->WD_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_WDCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for Watchdog Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getWDCTRL(void)
{
  TLE9563_getReg(TLE9563_WDCTRL_ADDR);
}

/** \brief Sets the device register for CAN Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setBUSCTRL(uint16_t u16_data)
{
  TLE9563->BUS_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_BUSCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for CAN Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getBUSCTRL(void)
{
  TLE9563_getReg(TLE9563_BUSCTRL_ADDR);
}

/** \brief Sets the device register for Wake-up Control via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setWKCTRLBNK3(uint16_t u16_data)
{
  TLE9563->WK_CTRL_BNK3.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_011);
  TLE9563_setBankReg(TLE9563_WKCTRLBNK3_ADDR, DEVICE_regBank_011, u16_data);
}

/** \brief Gets the device register for Wake-up Control via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getWKCTRLBNK3(void)
{
  TLE9563_getBankReg(TLE9563_WKCTRLBNK3_ADDR, DEVICE_regBank_011);
}

/** \brief Sets the device register for Wake-up Control via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setWKCTRLBNK4(uint16_t u16_data)
{
  TLE9563->WK_CTRL_BNK4.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_100);
  TLE9563_setBankReg(TLE9563_WKCTRLBNK4_ADDR, DEVICE_regBank_100, u16_data);
}

/** \brief Gets the device register for Wake-up Control via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getWKCTRLBNK4(void)
{
  TLE9563_getBankReg(TLE9563_WKCTRLBNK4_ADDR, DEVICE_regBank_100);
}

/** \brief Sets the device register for Timer 1 and Timer2 Control and Selection via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setTIMERCTRL(uint16_t u16_data)
{
  TLE9563->TIMER_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_TIMERCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for Timer 1 and Timer2 Control and Selection via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getTIMERCTRL(void)
{
  TLE9563_getReg(TLE9563_TIMERCTRL_ADDR);
}

/** \brief Sets the device register for High-Side Switch Shutdown Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWSDCTRL(uint16_t u16_data)
{
  TLE9563->SW_SD_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWSDCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for High-Side Switch Shutdown Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWSDCTRL(void)
{
  TLE9563_getReg(TLE9563_SWSDCTRL_ADDR);
}

/** \brief Sets the device register for High-Side Switch Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHSCTRL(uint16_t u16_data)
{
  TLE9563->HS_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_HSCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for High-Side Switch Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getHSCTRL(void)
{
  TLE9563_getReg(TLE9563_HSCTRL_ADDR);
}

/** \brief Sets the device register for Interrupt Mask Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setINTMASK(uint16_t u16_data)
{
  TLE9563->INT_MASK.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_INTMASK_ADDR, u16_data);
}

/** \brief Gets the device register for Interrupt Mask Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getINTMASK(void)
{
  TLE9563_getReg(TLE9563_INTMASK_ADDR);
}

/** \brief Sets the device register for PWM Configuration Control via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setPWMCTRLBNK0(uint16_t u16_data)
{
  TLE9563->PWM_CTRL_BNK0.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_000);
  TLE9563_setBankReg(TLE9563_PWMCTRLBNK0_ADDR, DEVICE_regBank_000, u16_data);
}

/** \brief Gets the device register for PWM Configuration Control via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getPWMCTRLBNK0(void)
{
  TLE9563_getBankReg(TLE9563_PWMCTRLBNK0_ADDR, DEVICE_regBank_000);
}

/** \brief Sets the device register for PWM Configuration Control via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setPWMCTRLBNK1(uint16_t u16_data)
{
  TLE9563->PWM_CTRL_BNK1.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_001);
  TLE9563_setBankReg(TLE9563_PWMCTRLBNK1_ADDR, DEVICE_regBank_001, u16_data);
}

/** \brief Gets the device register for PWM Configuration Control via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getPWMCTRLBNK1(void)
{
  TLE9563_getBankReg(TLE9563_PWMCTRLBNK1_ADDR, DEVICE_regBank_001);
}

/** \brief Sets the device register for PWM Configuration Control via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setPWMCTRLBNK2(uint16_t u16_data)
{
  TLE9563->PWM_CTRL_BNK2.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_010);
  TLE9563_setBankReg(TLE9563_PWMCTRLBNK2_ADDR, DEVICE_regBank_010, u16_data);
}

/** \brief Gets the device register for PWM Configuration Control via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getPWMCTRLBNK2(void)
{
  TLE9563_getBankReg(TLE9563_PWMCTRLBNK2_ADDR, DEVICE_regBank_010);
}

/** \brief Sets the device register for PWM Configuration Control via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setPWMCTRLBNK3(uint16_t u16_data)
{
  TLE9563->PWM_CTRL_BNK3.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_011);
  TLE9563_setBankReg(TLE9563_PWMCTRLBNK3_ADDR, DEVICE_regBank_011, u16_data);
}

/** \brief Gets the device register for PWM Configuration Control via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getPWMCTRLBNK3(void)
{
  TLE9563_getBankReg(TLE9563_PWMCTRLBNK3_ADDR, DEVICE_regBank_011);
}

/** \brief Sets the device register for System Status Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSYSSTATCTRL(uint16_t u16_data)
{
  TLE9563->SYS_STAT_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SYSSTATCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for System Status Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSYSSTATCTRL(void)
{
  TLE9563_getReg(TLE9563_SYSSTATCTRL_ADDR);
}

/** \brief Sets the device register for General Bridge Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setGENCTRL(uint16_t u16_data)
{
  TLE9563->GENCTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_GENCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for General Bridge Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getGENCTRL(void)
{
  TLE9563_getReg(TLE9563_GENCTRL_ADDR);
}

/** \brief Sets the device register for Current sense amplifier via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setCSA(uint16_t u16_data)
{
  TLE9563->CSA.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_CSA_ADDR, u16_data);
}

/** \brief Gets the device register for Current sense amplifier via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getCSA(void)
{
  TLE9563_getReg(TLE9563_CSA_ADDR);
}

/** \brief Sets the device register for VDS monitoring threshold LS1-3 via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setLSVDS(uint16_t u16_data)
{
  TLE9563->LS_VDS.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_LSVDS_ADDR, u16_data);
}

/** \brief Gets the device register for VDS monitoring threshold LS1-3 via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getLSVDS(void)
{
  TLE9563_getReg(TLE9563_LSVDS_ADDR);
}

/** \brief Sets the device register for VDS monitoring threshold HS1-3 via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHSVDS(uint16_t u16_data)
{
  TLE9563->HS_VDS.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_HSVDS_ADDR, u16_data);
}

/** \brief Gets the device register for VDS monitoring threshold HS1-3 via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getHSVDS(void)
{
  TLE9563_getReg(TLE9563_HSVDS_ADDR);
}

/** \brief Sets the device register for CCP and times selection via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setCCPBLKBNK0(uint16_t u16_data)
{
  TLE9563->CCP_BLK_BNK0.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_000);
  TLE9563_setBankReg(TLE9563_CCPBLKBNK0_ADDR, DEVICE_regBank_000, u16_data);
}

/** \brief Gets the device register for CCP and times selection via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getCCPBLKBNK0(void)
{
  TLE9563_getBankReg(TLE9563_CCPBLKBNK0_ADDR, DEVICE_regBank_000);
}

/** \brief Sets the device register for CCP and times selection via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setCCPBLKBNK1(uint16_t u16_data)
{
  TLE9563->CCP_BLK_BNK1.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_001);
  TLE9563_setBankReg(TLE9563_CCPBLKBNK1_ADDR, DEVICE_regBank_001, u16_data);
}

/** \brief Gets the device register for CCP and times selection via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getCCPBLKBNK1(void)
{
  TLE9563_getBankReg(TLE9563_CCPBLKBNK1_ADDR, DEVICE_regBank_001);
}

/** \brief Sets the device register for CCP and times selection via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setCCPBLKBNK2(uint16_t u16_data)
{
  TLE9563->CCP_BLK_BNK2.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_010);
  TLE9563_setBankReg(TLE9563_CCPBLKBNK2_ADDR, DEVICE_regBank_010, u16_data);
}

/** \brief Gets the device register for CCP and times selection via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getCCPBLKBNK2(void)
{
  TLE9563_getBankReg(TLE9563_CCPBLKBNK2_ADDR, DEVICE_regBank_010);
}

/** \brief Sets the device register for CCP and times selection via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setCCPBLKBNK4(uint16_t u16_data)
{
  TLE9563->CCP_BLK_BNK4.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_100);
  TLE9563_setBankReg(TLE9563_CCPBLKBNK4_ADDR, DEVICE_regBank_100, u16_data);
}

/** \brief Gets the device register for CCP and times selection via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getCCPBLKBNK4(void)
{
  TLE9563_getBankReg(TLE9563_CCPBLKBNK4_ADDR, DEVICE_regBank_100);
}

/** \brief Sets the device register for CCP and times selection via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setCCPBLKBNK5(uint16_t u16_data)
{
  TLE9563->CCP_BLK_BNK5.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_101);
  TLE9563_setBankReg(TLE9563_CCPBLKBNK5_ADDR, DEVICE_regBank_101, u16_data);
}

/** \brief Gets the device register for CCP and times selection via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getCCPBLKBNK5(void)
{
  TLE9563_getBankReg(TLE9563_CCPBLKBNK5_ADDR, DEVICE_regBank_101);
}

/** \brief Sets the device register for CCP and times selection via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setCCPBLKBNK6(uint16_t u16_data)
{
  TLE9563->CCP_BLK_BNK6.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_110);
  TLE9563_setBankReg(TLE9563_CCPBLKBNK6_ADDR, DEVICE_regBank_110, u16_data);
}

/** \brief Gets the device register for CCP and times selection via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getCCPBLKBNK6(void)
{
  TLE9563_getBankReg(TLE9563_CCPBLKBNK6_ADDR, DEVICE_regBank_110);
}

/** \brief Sets the device register for Half-Bridge MODE via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHBMODE(uint16_t u16_data)
{
  TLE9563->HBMODE.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_HBMODE_ADDR, u16_data);
}

/** \brief Gets the device register for Half-Bridge MODE via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getHBMODE(void)
{
  TLE9563_getReg(TLE9563_HBMODE_ADDR);
}

/** \brief Sets the device register for HB pre-charge and pre-discharge time via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setTPRECHGBNK0(uint16_t u16_data)
{
  TLE9563->TPRECHG_BNK0.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_000);
  TLE9563_setBankReg(TLE9563_TPRECHGBNK0_ADDR, DEVICE_regBank_000, u16_data);
}

/** \brief Gets the device register for HB pre-charge and pre-discharge time via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getTPRECHGBNK0(void)
{
  TLE9563_getBankReg(TLE9563_TPRECHGBNK0_ADDR, DEVICE_regBank_000);
}

/** \brief Sets the device register for HB pre-charge and pre-discharge time via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setTPRECHGBNK1(uint16_t u16_data)
{
  TLE9563->TPRECHG_BNK1.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_001);
  TLE9563_setBankReg(TLE9563_TPRECHGBNK1_ADDR, DEVICE_regBank_001, u16_data);
}

/** \brief Gets the device register for HB pre-charge and pre-discharge time via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getTPRECHGBNK1(void)
{
  TLE9563_getBankReg(TLE9563_TPRECHGBNK1_ADDR, DEVICE_regBank_001);
}

/** \brief Sets the device register for Static charge/discharge current via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSTICHG(uint16_t u16_data)
{
  TLE9563->ST_ICHG.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_STICHG_ADDR, u16_data);
}

/** \brief Gets the device register for Static charge/discharge current via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSTICHG(void)
{
  TLE9563_getReg(TLE9563_STICHG_ADDR);
}

/** \brief Sets the device register for HB charge/discharge currents for PWM operation via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHBICHGBNK0(uint16_t u16_data)
{
  TLE9563->HB_ICHG_BNK0.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_000);
  TLE9563_setBankReg(TLE9563_HBICHGBNK0_ADDR, DEVICE_regBank_000, u16_data);
}

/** \brief Gets the device register for HB charge/discharge currents for PWM operation via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getHBICHGBNK0(void)
{
  TLE9563_getBankReg(TLE9563_HBICHGBNK0_ADDR, DEVICE_regBank_000);
}

/** \brief Sets the device register for HB charge/discharge currents for PWM operation via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHBICHGBNK1(uint16_t u16_data)
{
  TLE9563->HB_ICHG_BNK1.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_001);
  TLE9563_setBankReg(TLE9563_HBICHGBNK1_ADDR, DEVICE_regBank_001, u16_data);
}

/** \brief Gets the device register for HB charge/discharge currents for PWM operation via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getHBICHGBNK1(void)
{
  TLE9563_getBankReg(TLE9563_HBICHGBNK1_ADDR, DEVICE_regBank_001);
}

/** \brief Sets the device register for HB charge/discharge currents for PWM operation via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHBICHGBNK2(uint16_t u16_data)
{
  TLE9563->HB_ICHG_BNK2.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_010);
  TLE9563_setBankReg(TLE9563_HBICHGBNK2_ADDR, DEVICE_regBank_010, u16_data);
}

/** \brief Gets the device register for HB charge/discharge currents for PWM operation via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getHBICHGBNK2(void)
{
  TLE9563_getBankReg(TLE9563_HBICHGBNK2_ADDR, DEVICE_regBank_010);
}

/** \brief Sets the device register for HB charge/discharge currents for PWM operation via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHBICHGBNK4(uint16_t u16_data)
{
  TLE9563->HB_ICHG_BNK4.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_100);
  TLE9563_setBankReg(TLE9563_HBICHGBNK4_ADDR, DEVICE_regBank_100, u16_data);
}

/** \brief Gets the device register for HB charge/discharge currents for PWM operation via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getHBICHGBNK4(void)
{
  TLE9563_getBankReg(TLE9563_HBICHGBNK4_ADDR, DEVICE_regBank_100);
}

/** \brief Sets the device register for HB charge/discharge currents for PWM operation via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHBICHGBNK5(uint16_t u16_data)
{
  TLE9563->HB_ICHG_BNK5.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_101);
  TLE9563_setBankReg(TLE9563_HBICHGBNK5_ADDR, DEVICE_regBank_101, u16_data);
}

/** \brief Gets the device register for HB charge/discharge currents for PWM operation via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getHBICHGBNK5(void)
{
  TLE9563_getBankReg(TLE9563_HBICHGBNK5_ADDR, DEVICE_regBank_101);
}

/** \brief Sets the device register for HB charge/discharge currents for PWM operation via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHBICHGBNK6(uint16_t u16_data)
{
  TLE9563->HB_ICHG_BNK6.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_110);
  TLE9563_setBankReg(TLE9563_HBICHGBNK6_ADDR, DEVICE_regBank_110, u16_data);
}

/** \brief Gets the device register for HB charge/discharge currents for PWM operation via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getHBICHGBNK6(void)
{
  TLE9563_getBankReg(TLE9563_HBICHGBNK6_ADDR, DEVICE_regBank_110);
}

/** \brief Sets the device register for HB max. pre-charge/pre-discharge in PWM operation current and diagnostic pull-down via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHBICHGMAX(uint16_t u16_data)
{
  TLE9563->HB_ICHG_MAX.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_HBICHGMAX_ADDR, u16_data);
}

/** \brief Gets the device register for HB max. pre-charge/pre-discharge in PWM operation current and diagnostic pull-down via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getHBICHGMAX(void)
{
  TLE9563_getReg(TLE9563_HBICHGMAX_ADDR);
}

/** \brief Sets the device register for HBx pre-charge/pre-discharge initialization configuration in PWM operation via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHBPCHGINITBNK0(uint16_t u16_data)
{
  TLE9563->HB_PCHG_INIT_BNK0.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_000);
  TLE9563_setBankReg(TLE9563_HBPCHGINITBNK0_ADDR, DEVICE_regBank_000, u16_data);
}

/** \brief Gets the device register for HBx pre-charge/pre-discharge initialization configuration in PWM operation via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getHBPCHGINITBNK0(void)
{
  TLE9563_getBankReg(TLE9563_HBPCHGINITBNK0_ADDR, DEVICE_regBank_000);
}

/** \brief Sets the device register for HBx pre-charge/pre-discharge initialization configuration in PWM operation via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHBPCHGINITBNK1(uint16_t u16_data)
{
  TLE9563->HB_PCHG_INIT_BNK1.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_001);
  TLE9563_setBankReg(TLE9563_HBPCHGINITBNK1_ADDR, DEVICE_regBank_001, u16_data);
}

/** \brief Gets the device register for HBx pre-charge/pre-discharge initialization configuration in PWM operation via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getHBPCHGINITBNK1(void)
{
  TLE9563_getBankReg(TLE9563_HBPCHGINITBNK1_ADDR, DEVICE_regBank_001);
}

/** \brief Sets the device register for HBx pre-charge/pre-discharge initialization configuration in PWM operation via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setHBPCHGINITBNK2(uint16_t u16_data)
{
  TLE9563->HB_PCHG_INIT_BNK2.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_010);
  TLE9563_setBankReg(TLE9563_HBPCHGINITBNK2_ADDR, DEVICE_regBank_010, u16_data);
}

/** \brief Gets the device register for HBx pre-charge/pre-discharge initialization configuration in PWM operation via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getHBPCHGINITBNK2(void)
{
  TLE9563_getBankReg(TLE9563_HBPCHGINITBNK2_ADDR, DEVICE_regBank_010);
}

/** \brief Sets the device register for HBx inputs TDON configuration via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setTDONHBCTRLBNK0(uint16_t u16_data)
{
  TLE9563->TDON_HB_CTRL_BNK0.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_000);
  TLE9563_setBankReg(TLE9563_TDONHBCTRLBNK0_ADDR, DEVICE_regBank_000, u16_data);
}

/** \brief Gets the device register for HBx inputs TDON configuration via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getTDONHBCTRLBNK0(void)
{
  TLE9563_getBankReg(TLE9563_TDONHBCTRLBNK0_ADDR, DEVICE_regBank_000);
}

/** \brief Sets the device register for HBx inputs TDON configuration via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setTDONHBCTRLBNK1(uint16_t u16_data)
{
  TLE9563->TDON_HB_CTRL_BNK1.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_001);
  TLE9563_setBankReg(TLE9563_TDONHBCTRLBNK1_ADDR, DEVICE_regBank_001, u16_data);
}

/** \brief Gets the device register for HBx inputs TDON configuration via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getTDONHBCTRLBNK1(void)
{
  TLE9563_getBankReg(TLE9563_TDONHBCTRLBNK1_ADDR, DEVICE_regBank_001);
}

/** \brief Sets the device register for HBx inputs TDON configuration via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setTDONHBCTRLBNK2(uint16_t u16_data)
{
  TLE9563->TDON_HB_CTRL_BNK2.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_010);
  TLE9563_setBankReg(TLE9563_TDONHBCTRLBNK2_ADDR, DEVICE_regBank_010, u16_data);
}

/** \brief Gets the device register for HBx inputs TDON configuration via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getTDONHBCTRLBNK2(void)
{
  TLE9563_getBankReg(TLE9563_TDONHBCTRLBNK2_ADDR, DEVICE_regBank_010);
}

/** \brief Sets the device register for HBx TDOFF configuration via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setTDOFFHBCTRLBNK0(uint16_t u16_data)
{
  TLE9563->TDOFF_HB_CTRL_BNK0.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_000);
  TLE9563_setBankReg(TLE9563_TDOFFHBCTRLBNK0_ADDR, DEVICE_regBank_000, u16_data);
}

/** \brief Gets the device register for HBx TDOFF configuration via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getTDOFFHBCTRLBNK0(void)
{
  TLE9563_getBankReg(TLE9563_TDOFFHBCTRLBNK0_ADDR, DEVICE_regBank_000);
}

/** \brief Sets the device register for HBx TDOFF configuration via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setTDOFFHBCTRLBNK1(uint16_t u16_data)
{
  TLE9563->TDOFF_HB_CTRL_BNK1.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_001);
  TLE9563_setBankReg(TLE9563_TDOFFHBCTRLBNK1_ADDR, DEVICE_regBank_001, u16_data);
}

/** \brief Gets the device register for HBx TDOFF configuration via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getTDOFFHBCTRLBNK1(void)
{
  TLE9563_getBankReg(TLE9563_TDOFFHBCTRLBNK1_ADDR, DEVICE_regBank_001);
}

/** \brief Sets the device register for HBx TDOFF configuration via SPI with according register bank
 * 
 *  \param uint16_t u16_data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setTDOFFHBCTRLBNK2(uint16_t u16_data)
{
  TLE9563->TDOFF_HB_CTRL_BNK2.reg = ((uint16_t)u16_data | (uint16_t)DEVICE_regBank_010);
  TLE9563_setBankReg(TLE9563_TDOFFHBCTRLBNK2_ADDR, DEVICE_regBank_010, u16_data);
}

/** \brief Gets the device register for HBx TDOFF configuration via SPI with according register bank
 * 
 *  \return void no return value
 */
void TLE9563_getTDOFFHBCTRLBNK2(void)
{
  TLE9563_getBankReg(TLE9563_TDOFFHBCTRLBNK2_ADDR, DEVICE_regBank_010);
}

/** \brief Sets the device register for Brake control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setBRAKE(uint16_t u16_data)
{
  TLE9563->BRAKE.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_BRAKE_ADDR, u16_data);
}

/** \brief Gets the device register for Brake control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getBRAKE(void)
{
  TLE9563_getReg(TLE9563_BRAKE_ADDR);
}

/** \brief Sets the device register for CAN Selective Wake Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKCTRL(uint16_t u16_data)
{
  TLE9563->SWK_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for CAN Selective Wake Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKCTRL(void)
{
  TLE9563_getReg(TLE9563_SWKCTRL_ADDR);
}

/** \brief Sets the device register for SWK Bit Timing Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKBTL1CTRL(uint16_t u16_data)
{
  TLE9563->SWK_BTL1_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKBTL1CTRL_ADDR, u16_data);
}

/** \brief Gets the device register for SWK Bit Timing Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKBTL1CTRL(void)
{
  TLE9563_getReg(TLE9563_SWKBTL1CTRL_ADDR);
}

/** \brief Sets the device register for SWK WUF Identifier bits 28...13 via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKID1CTRL(uint16_t u16_data)
{
  TLE9563->SWK_ID1_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKID1CTRL_ADDR, u16_data);
}

/** \brief Gets the device register for SWK WUF Identifier bits 28...13 via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKID1CTRL(void)
{
  TLE9563_getReg(TLE9563_SWKID1CTRL_ADDR);
}

/** \brief Sets the device register for SWK WUF Identifier bits 12...0 via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKID0CTRL(uint16_t u16_data)
{
  TLE9563->SWK_ID0_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKID0CTRL_ADDR, u16_data);
}

/** \brief Gets the device register for SWK WUF Identifier bits 12...0 via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKID0CTRL(void)
{
  TLE9563_getReg(TLE9563_SWKID0CTRL_ADDR);
}

/** \brief Sets the device register for SWK WUF Identifier Mask bits 28...13 via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKMASKID1CTRL(uint16_t u16_data)
{
  TLE9563->SWK_MASK_ID1_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKMASKID1CTRL_ADDR, u16_data);
}

/** \brief Gets the device register for SWK WUF Identifier Mask bits 28...13 via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKMASKID1CTRL(void)
{
  TLE9563_getReg(TLE9563_SWKMASKID1CTRL_ADDR);
}

/** \brief Sets the device register for SWK WUF Identifier Mask bits 12...0 via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKMASKID0CTRL(uint16_t u16_data)
{
  TLE9563->SWK_MASK_ID0_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKMASKID0CTRL_ADDR, u16_data);
}

/** \brief Gets the device register for SWK WUF Identifier Mask bits 12...0 via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKMASKID0CTRL(void)
{
  TLE9563_getReg(TLE9563_SWKMASKID0CTRL_ADDR);
}

/** \brief Sets the device register for SWK Frame Data Length Code Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKDLCCTRL(uint16_t u16_data)
{
  TLE9563->SWK_DLC_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKDLCCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for SWK Frame Data Length Code Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKDLCCTRL(void)
{
  TLE9563_getReg(TLE9563_SWKDLCCTRL_ADDR);
}

/** \brief Sets the device register for SWK Data7-Data6 Register via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKDATA3CTRL(uint16_t u16_data)
{
  TLE9563->SWK_DATA3_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKDATA3CTRL_ADDR, u16_data);
}

/** \brief Gets the device register for SWK Data7-Data6 Register via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKDATA3CTRL(void)
{
  TLE9563_getReg(TLE9563_SWKDATA3CTRL_ADDR);
}

/** \brief Sets the device register for SWK Data5-Data4 Register via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKDATA2CTRL(uint16_t u16_data)
{
  TLE9563->SWK_DATA2_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKDATA2CTRL_ADDR, u16_data);
}

/** \brief Gets the device register for SWK Data5-Data4 Register via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKDATA2CTRL(void)
{
  TLE9563_getReg(TLE9563_SWKDATA2CTRL_ADDR);
}

/** \brief Sets the device register for SWK Data3-Data2 Register via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKDATA1CTRL(uint16_t u16_data)
{
  TLE9563->SWK_DATA1_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKDATA1CTRL_ADDR, u16_data);
}

/** \brief Gets the device register for SWK Data3-Data2 Register via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKDATA1CTRL(void)
{
  TLE9563_getReg(TLE9563_SWKDATA1CTRL_ADDR);
}

/** \brief Sets the device register for SWK Data1-Data0 Register via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKDATA0CTRL(uint16_t u16_data)
{
  TLE9563->SWK_DATA0_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKDATA0CTRL_ADDR, u16_data);
}

/** \brief Gets the device register for SWK Data1-Data0 Register via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKDATA0CTRL(void)
{
  TLE9563_getReg(TLE9563_SWKDATA0CTRL_ADDR);
}

/** \brief Sets the device register for CAN FD Configuration Control Register via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKCANFDCTRL(uint16_t u16_data)
{
  TLE9563->SWK_CAN_FD_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKCANFDCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for CAN FD Configuration Control Register via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKCANFDCTRL(void)
{
  TLE9563_getReg(TLE9563_SWKCANFDCTRL_ADDR);
}

/** \brief Sets the device register for SWK Oscillator Trimming and option Register via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKOSCTRIMCTRL(uint16_t u16_data)
{
  TLE9563->SWK_OSC_TRIM_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKOSCTRIMCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for SWK Oscillator Trimming and option Register via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKOSCTRIMCTRL(void)
{
  TLE9563_getReg(TLE9563_SWKOSCTRIMCTRL_ADDR);
}

/** \brief Gets the device register for SWK Oscillator Calibration Register via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKOSCCALSTAT(void)
{
  TLE9563_getReg(TLE9563_SWKOSCCALSTAT_ADDR);
}

/** \brief Sets the register RAM copy for SWK Oscillator Calibration Register
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamSWKOSCCALSTAT(uint16_t u16_data)
{
  TLE9563->SWK_OSC_CAL_STAT.reg = (uint16_t)u16_data;
}

/** \brief Sets the device register for Clock Data Recovery Control Register via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKCDRCTRL(uint16_t u16_data)
{
  TLE9563->SWK_CDR_CTRL.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKCDRCTRL_ADDR, u16_data);
}

/** \brief Gets the device register for Clock Data Recovery Control Register via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKCDRCTRL(void)
{
  TLE9563_getReg(TLE9563_SWKCDRCTRL_ADDR);
}

/** \brief Sets the device register for SWK Clock Data Recovery Limit Control via SPI
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_setSWKCDRLIMIT(uint16_t u16_data)
{
  TLE9563->SWK_CDR_LIMIT.reg = (uint16_t)u16_data;
  TLE9563_setReg(TLE9563_SWKCDRLIMIT_ADDR, u16_data);
}

/** \brief Gets the device register for SWK Clock Data Recovery Limit Control via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKCDRLIMIT(void)
{
  TLE9563_getReg(TLE9563_SWKCDRLIMIT_ADDR);
}

/** \brief Gets the device register for Supply Voltage Fail Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSUPSTAT(void)
{
  TLE9563_getReg(TLE9563_SUPSTAT_ADDR);
}

/** \brief Clears the device register for Supply Voltage Fail Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_clrSUPSTAT(uint16_t u16_data)
{
  TLE9563_setReg(TLE9563_SUPSTAT_ADDR, 0x0000);
}

/** \brief Sets the register RAM copy for Supply Voltage Fail Status
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamSUPSTAT(uint16_t u16_data)
{
  TLE9563->SUP_STAT.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Thermal Protection Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getTHERMSTAT(void)
{
  TLE9563_getReg(TLE9563_THERMSTAT_ADDR);
}

/** \brief Clears the device register for Thermal Protection Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_clrTHERMSTAT(uint16_t u16_data)
{
  TLE9563_setReg(TLE9563_THERMSTAT_ADDR, 0x0000);
}

/** \brief Sets the register RAM copy for Thermal Protection Status
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamTHERMSTAT(uint16_t u16_data)
{
  TLE9563->THERM_STAT.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Device Information Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getDEVSTAT(void)
{
  TLE9563_getReg(TLE9563_DEVSTAT_ADDR);
}

/** \brief Clears the device register for Device Information Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_clrDEVSTAT(uint16_t u16_data)
{
  TLE9563_setReg(TLE9563_DEVSTAT_ADDR, 0x0000);
}

/** \brief Sets the register RAM copy for Device Information Status
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamDEVSTAT(uint16_t u16_data)
{
  TLE9563->DEV_STAT.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Bus Communication Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getBUSSTAT(void)
{
  TLE9563_getReg(TLE9563_BUSSTAT_ADDR);
}

/** \brief Clears the device register for Bus Communication Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_clrBUSSTAT(uint16_t u16_data)
{
  TLE9563_setReg(TLE9563_BUSSTAT_ADDR, 0x0000);
}

/** \brief Sets the register RAM copy for Bus Communication Status
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamBUSSTAT(uint16_t u16_data)
{
  TLE9563->BUS_STAT.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Wake-up Source and Information Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getWKSTAT(void)
{
  TLE9563_getReg(TLE9563_WKSTAT_ADDR);
}

/** \brief Clears the device register for Wake-up Source and Information Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_clrWKSTAT(uint16_t u16_data)
{
  TLE9563_setReg(TLE9563_WKSTAT_ADDR, 0x0000);
}

/** \brief Sets the register RAM copy for Wake-up Source and Information Status
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamWKSTAT(uint16_t u16_data)
{
  TLE9563->WK_STAT.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for WK Input Level via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getWKLVLSTAT(void)
{
  TLE9563_getReg(TLE9563_WKLVLSTAT_ADDR);
}

/** \brief Sets the register RAM copy for WK Input Level
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamWKLVLSTAT(uint16_t u16_data)
{
  TLE9563->WK_LVL_STAT.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for High-Side Switch Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getHSOLOCOTSTAT(void)
{
  TLE9563_getReg(TLE9563_HSOLOCOTSTAT_ADDR);
}

/** \brief Clears the device register for High-Side Switch Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_clrHSOLOCOTSTAT(uint16_t u16_data)
{
  TLE9563_setReg(TLE9563_HSOLOCOTSTAT_ADDR, 0x0000);
}

/** \brief Sets the register RAM copy for High-Side Switch Status
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamHSOLOCOTSTAT(uint16_t u16_data)
{
  TLE9563->HS_OL_OC_OT_STAT.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for General Status register via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getGENSTAT(void)
{
  TLE9563_getReg(TLE9563_GENSTAT_ADDR);
}

/** \brief Sets the register RAM copy for General Status register
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamGENSTAT(uint16_t u16_data)
{
  TLE9563->GEN_STAT.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Turn-on/off delay regulation register via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getTDREG(void)
{
  TLE9563_getReg(TLE9563_TDREG_ADDR);
}

/** \brief Sets the register RAM copy for Turn-on/off delay regulation register
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamTDREG(uint16_t u16_data)
{
  TLE9563->TDREG.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Drain-source overvoltage via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getDSOV(void)
{
  TLE9563_getReg(TLE9563_DSOV_ADDR);
}

/** \brief Clears the device register for Drain-source overvoltage via SPI
 * 
 *  \return void no return value
 */
void TLE9563_clrDSOV(uint16_t u16_data)
{
  TLE9563_setReg(TLE9563_DSOV_ADDR, 0x0000);
}

/** \brief Sets the register RAM copy for Drain-source overvoltage
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamDSOV(uint16_t u16_data)
{
  TLE9563->DSOV.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Effective MOSFET turn.on/off delay - HB1 via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getEFFTDONOFF1(void)
{
  TLE9563_getReg(TLE9563_EFFTDONOFF1_ADDR);
}

/** \brief Sets the register RAM copy for Effective MOSFET turn.on/off delay - HB1
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamEFFTDONOFF1(uint16_t u16_data)
{
  TLE9563->EFF_TDON_OFF1.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Effective MOSFET turn.on/off delay - HB 2 via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getEFFTDONOFF2(void)
{
  TLE9563_getReg(TLE9563_EFFTDONOFF2_ADDR);
}

/** \brief Sets the register RAM copy for Effective MOSFET turn.on/off delay - HB 2
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamEFFTDONOFF2(uint16_t u16_data)
{
  TLE9563->EFF_TDON_OFF2.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Effective MOSFET turn.on/off delay - HB3 via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getEFFTDONOFF3(void)
{
  TLE9563_getReg(TLE9563_EFFTDONOFF3_ADDR);
}

/** \brief Sets the register RAM copy for Effective MOSFET turn.on/off delay - HB3
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamEFFTDONOFF3(uint16_t u16_data)
{
  TLE9563->EFF_TDON_OFF3.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for MOSFET rise/fall time - HB1 via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getTRISEFALL1(void)
{
  TLE9563_getReg(TLE9563_TRISEFALL1_ADDR);
}

/** \brief Sets the register RAM copy for MOSFET rise/fall time - HB1
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamTRISEFALL1(uint16_t u16_data)
{
  TLE9563->TRISE_FALL1.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for MOSFET rise/fall time - HB2 via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getTRISEFALL2(void)
{
  TLE9563_getReg(TLE9563_TRISEFALL2_ADDR);
}

/** \brief Sets the register RAM copy for MOSFET rise/fall time - HB2
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamTRISEFALL2(uint16_t u16_data)
{
  TLE9563->TRISE_FALL2.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for MOSFET rise/fall time - HB3 via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getTRISEFALL3(void)
{
  TLE9563_getReg(TLE9563_TRISEFALL3_ADDR);
}

/** \brief Sets the register RAM copy for MOSFET rise/fall time - HB3
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamTRISEFALL3(uint16_t u16_data)
{
  TLE9563->TRISE_FALL3.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Selective Wake Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKSTAT(void)
{
  TLE9563_getReg(TLE9563_SWKSTAT_ADDR);
}

/** \brief Clears the device register for Selective Wake Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_clrSWKSTAT(uint16_t u16_data)
{
  TLE9563_setReg(TLE9563_SWKSTAT_ADDR, 0x0000);
}

/** \brief Sets the register RAM copy for Selective Wake Status
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamSWKSTAT(uint16_t u16_data)
{
  TLE9563->SWK_STAT.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Selective Wake ECNT Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKECNTSTAT(void)
{
  TLE9563_getReg(TLE9563_SWKECNTSTAT_ADDR);
}

/** \brief Sets the register RAM copy for Selective Wake ECNT Status
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamSWKECNTSTAT(uint16_t u16_data)
{
  TLE9563->SWK_ECNT_STAT.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Selective Wake CDR Status via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getSWKCDRSTAT(void)
{
  TLE9563_getReg(TLE9563_SWKCDRSTAT_ADDR);
}

/** \brief Sets the register RAM copy for Selective Wake CDR Status
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamSWKCDRSTAT(uint16_t u16_data)
{
  TLE9563->SWK_CDR_STAT.reg = (uint16_t)u16_data;
}

/** \brief Gets the device register for Family and Product Identification Register via SPI
 * 
 *  \return void no return value
 */
void TLE9563_getFAMPRODSTAT(void)
{
  TLE9563_getReg(TLE9563_FAMPRODSTAT_ADDR);
}

/** \brief Sets the register RAM copy for Family and Product Identification Register
 * 
 *  \param uint16_t data to be written
 * 
 *  \return void no return value
 */
void TLE9563_updateRamFAMPRODSTAT(uint16_t u16_data)
{
  TLE9563->FAM_PROD_STAT.reg = (uint16_t)u16_data;
}

