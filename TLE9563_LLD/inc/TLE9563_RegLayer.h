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
 * \file     TLE9563.h
 *
 * \brief    TLE9563 low level access library
 *
 * \version  V0.1.6
 * \date     16. Feb 2022
 *
 * \note 
 */

/** \addtogroup TLE9563_api
 *  @{
 */

/*******************************************************************************
**                             Author(s) Identity                             **
********************************************************************************
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** JO           Julia Ott                                                     **
** VO           Vanessa Ongaro                                                **
*******************************************************************************/

/*******************************************************************************
**                          Revision Control History                          **
********************************************************************************
** V0.1.0: 2021-12-06, VO:   Initial version                                  **
** V0.1.1: 2022-01-05, VO:   [EP-994] added return value for clr fkts for use **
**                           in cyclic task                                   **
** V0.1.2: 2022-01-19, VO:   Deleted commented out lines and added briefs     **
** V0.1.3: 2022-01-25, VO:   Added functions to update status registers in    **
**                           RAM copy                                         **
** V0.1.4: 2022-01-26, VO:   Added missing function to update status register **
**                           for SWK Oscillator Calibration Register          **
** V0.1.5: 2022-02-04, VO:   [EP-1003] Updated set register functions to      **
**                           include writing to the RAM copy of the ctrl      **
**                           registers before sending SPI write command       **
** V0.1.6: 2022-02-16, VO:   [EP-1037] Changed TLE9563_setMSCTRL to reset     **
**                           registers in RAM copy in case of SW reset        **
*******************************************************************************/

#ifndef _TLE9563_REGLAYER_H
#define _TLE9563_REGLAYER_H

/*******************************************************************************
**                                  Includes                                  **
*******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tle9563.h"

/*******************************************************************************
**                          Global Macro Declarations                         **
*******************************************************************************/
#define TLE9563_MSCTRL_ADDR          (0x01)
#define TLE9563_HWCTRL_ADDR          (0x02)
#define TLE9563_WDCTRL_ADDR          (0x03)
#define TLE9563_BUSCTRL_ADDR         (0x04)
#define TLE9563_WKCTRLBNK3_ADDR      (0x05)
#define TLE9563_WKCTRLBNK4_ADDR      (0x05)
#define TLE9563_TIMERCTRL_ADDR       (0x06)
#define TLE9563_SWSDCTRL_ADDR        (0x07)
#define TLE9563_HSCTRL_ADDR          (0x08)
#define TLE9563_INTMASK_ADDR         (0x09)
#define TLE9563_PWMCTRLBNK0_ADDR     (0x0A)
#define TLE9563_PWMCTRLBNK1_ADDR     (0x0A)
#define TLE9563_PWMCTRLBNK2_ADDR     (0x0A)
#define TLE9563_PWMCTRLBNK3_ADDR     (0x0A)
#define TLE9563_SYSSTATCTRL_ADDR     (0x0B)
#define TLE9563_GENCTRL_ADDR         (0x10)
#define TLE9563_CSA_ADDR             (0x11)
#define TLE9563_LSVDS_ADDR           (0x12)
#define TLE9563_HSVDS_ADDR           (0x13)
#define TLE9563_CCPBLKBNK0_ADDR      (0x14)
#define TLE9563_CCPBLKBNK1_ADDR      (0x14)
#define TLE9563_CCPBLKBNK2_ADDR      (0x14)
#define TLE9563_CCPBLKBNK4_ADDR      (0x14)
#define TLE9563_CCPBLKBNK5_ADDR      (0x14)
#define TLE9563_CCPBLKBNK6_ADDR      (0x14)
#define TLE9563_HBMODE_ADDR          (0x15)
#define TLE9563_TPRECHGBNK0_ADDR     (0x16)
#define TLE9563_TPRECHGBNK1_ADDR     (0x16)
#define TLE9563_STICHG_ADDR          (0x17)
#define TLE9563_HBICHGBNK0_ADDR      (0x18)
#define TLE9563_HBICHGBNK1_ADDR      (0x18)
#define TLE9563_HBICHGBNK2_ADDR      (0x18)
#define TLE9563_HBICHGBNK4_ADDR      (0x18)
#define TLE9563_HBICHGBNK5_ADDR      (0x18)
#define TLE9563_HBICHGBNK6_ADDR      (0x18)
#define TLE9563_HBICHGMAX_ADDR       (0x19)
#define TLE9563_HBPCHGINITBNK0_ADDR  (0x1A)
#define TLE9563_HBPCHGINITBNK1_ADDR  (0x1A)
#define TLE9563_HBPCHGINITBNK2_ADDR  (0x1A)
#define TLE9563_TDONHBCTRLBNK0_ADDR  (0x1B)
#define TLE9563_TDONHBCTRLBNK1_ADDR  (0x1B)
#define TLE9563_TDONHBCTRLBNK2_ADDR  (0x1B)
#define TLE9563_TDOFFHBCTRLBNK0_ADDR (0x1C)
#define TLE9563_TDOFFHBCTRLBNK1_ADDR (0x1C)
#define TLE9563_TDOFFHBCTRLBNK2_ADDR (0x1C)
#define TLE9563_BRAKE_ADDR           (0x1D)
#define TLE9563_SWKCTRL_ADDR         (0x30)
#define TLE9563_SWKBTL1CTRL_ADDR     (0x31)
#define TLE9563_SWKID1CTRL_ADDR      (0x32)
#define TLE9563_SWKID0CTRL_ADDR      (0x33)
#define TLE9563_SWKMASKID1CTRL_ADDR  (0x34)
#define TLE9563_SWKMASKID0CTRL_ADDR  (0x35)
#define TLE9563_SWKDLCCTRL_ADDR      (0x36)
#define TLE9563_SWKDATA3CTRL_ADDR    (0x37)
#define TLE9563_SWKDATA2CTRL_ADDR    (0x38)
#define TLE9563_SWKDATA1CTRL_ADDR    (0x39)
#define TLE9563_SWKDATA0CTRL_ADDR    (0x3A)
#define TLE9563_SWKCANFDCTRL_ADDR    (0x3B)
#define TLE9563_SWKOSCTRIMCTRL_ADDR  (0x3C)
#define TLE9563_SWKOSCCALSTAT_ADDR   (0x3D)
#define TLE9563_SWKCDRCTRL_ADDR      (0x3E)
#define TLE9563_SWKCDRLIMIT_ADDR     (0x3F)
#define TLE9563_SUPSTAT_ADDR         (0x40)
#define TLE9563_THERMSTAT_ADDR       (0x41)
#define TLE9563_DEVSTAT_ADDR         (0x42)
#define TLE9563_BUSSTAT_ADDR         (0x43)
#define TLE9563_WKSTAT_ADDR          (0x44)
#define TLE9563_WKLVLSTAT_ADDR       (0x45)
#define TLE9563_HSOLOCOTSTAT_ADDR    (0x46)
#define TLE9563_GENSTAT_ADDR         (0x50)
#define TLE9563_TDREG_ADDR           (0x51)
#define TLE9563_DSOV_ADDR            (0x52)
#define TLE9563_EFFTDONOFF1_ADDR     (0x53)
#define TLE9563_EFFTDONOFF2_ADDR     (0x54)
#define TLE9563_EFFTDONOFF3_ADDR     (0x55)
#define TLE9563_TRISEFALL1_ADDR      (0x57)
#define TLE9563_TRISEFALL2_ADDR      (0x58)
#define TLE9563_TRISEFALL3_ADDR      (0x59)
#define TLE9563_SWKSTAT_ADDR         (0x60)
#define TLE9563_SWKECNTSTAT_ADDR     (0x61)
#define TLE9563_SWKCDRSTAT_ADDR      (0x63)
#define TLE9563_FAMPRODSTAT_ADDR     (0x70)

/*******************************************************************************
**                          Global Type Declarations                          **
*******************************************************************************/
/** \enum eDEVICE_regBank
 * 
 *  \brief Enum for the TLE9563 Resgister Banks
 *  \brief In banked registers, the first 3 bits of the payload select the bank that has to be configured
 */
typedef enum _tDEVICE_regBank
{
DEVICE_regBank_000 = 0,                                     /** \brief  */
DEVICE_regBank_001 = 1,                                     /** \brief  */
DEVICE_regBank_010 = 2,                                     /** \brief  */
DEVICE_regBank_011 = 3,                                     /** \brief  */
DEVICE_regBank_100 = 4,                                     /** \brief  */
DEVICE_regBank_101 = 5,                                     /** \brief  */
DEVICE_regBank_110 = 6,                                     /** \brief  */
DEVICE_regBank_111 = 7                                      /** \brief  */
} tDEVICE_regBank;

/*******************************************************************************
**                        Global Variable Declarations                        **
*******************************************************************************/


/*******************************************************************************
**                        Global Function Declarations                        **
*******************************************************************************/
void TLE9563_resetCrc(uint16_t u16_data);
void TLE9563_setMSCTRL(uint16_t u16_data);
void TLE9563_getMSCTRL(void);
void TLE9563_setHWCTRL(uint16_t u16_data);
void TLE9563_getHWCTRL(void);
void TLE9563_setWDCTRL(uint16_t u16_data);
void TLE9563_getWDCTRL(void);
void TLE9563_setBUSCTRL(uint16_t u16_data);
void TLE9563_getBUSCTRL(void);
void TLE9563_setWKCTRLBNK3(uint16_t u16_data);
void TLE9563_getWKCTRLBNK3(void);
void TLE9563_setWKCTRLBNK4(uint16_t u16_data);
void TLE9563_getWKCTRLBNK4(void);
void TLE9563_setTIMERCTRL(uint16_t u16_data);
void TLE9563_getTIMERCTRL(void);
void TLE9563_setSWSDCTRL(uint16_t u16_data);
void TLE9563_getSWSDCTRL(void);
void TLE9563_setHSCTRL(uint16_t u16_data);
void TLE9563_getHSCTRL(void);
void TLE9563_setINTMASK(uint16_t u16_data);
void TLE9563_getINTMASK(void);
void TLE9563_setPWMCTRLBNK0(uint16_t u16_data);
void TLE9563_getPWMCTRLBNK0(void);
void TLE9563_setPWMCTRLBNK1(uint16_t u16_data);
void TLE9563_getPWMCTRLBNK1(void);
void TLE9563_setPWMCTRLBNK2(uint16_t u16_data);
void TLE9563_getPWMCTRLBNK2(void);
void TLE9563_setPWMCTRLBNK3(uint16_t u16_data);
void TLE9563_getPWMCTRLBNK3(void);
void TLE9563_setSYSSTATCTRL(uint16_t u16_data);
void TLE9563_getSYSSTATCTRL(void);
void TLE9563_setGENCTRL(uint16_t u16_data);
void TLE9563_getGENCTRL(void);
void TLE9563_setCSA(uint16_t u16_data);
void TLE9563_getCSA(void);
void TLE9563_setLSVDS(uint16_t u16_data);
void TLE9563_getLSVDS(void);
void TLE9563_setHSVDS(uint16_t u16_data);
void TLE9563_getHSVDS(void);
void TLE9563_setCCPBLKBNK0(uint16_t u16_data);
void TLE9563_getCCPBLKBNK0(void);
void TLE9563_setCCPBLKBNK1(uint16_t u16_data);
void TLE9563_getCCPBLKBNK1(void);
void TLE9563_setCCPBLKBNK2(uint16_t u16_data);
void TLE9563_getCCPBLKBNK2(void);
void TLE9563_setCCPBLKBNK4(uint16_t u16_data);
void TLE9563_getCCPBLKBNK4(void);
void TLE9563_setCCPBLKBNK5(uint16_t u16_data);
void TLE9563_getCCPBLKBNK5(void);
void TLE9563_setCCPBLKBNK6(uint16_t u16_data);
void TLE9563_getCCPBLKBNK6(void);
void TLE9563_setHBMODE(uint16_t u16_data);
void TLE9563_getHBMODE(void);
void TLE9563_setTPRECHGBNK0(uint16_t u16_data);
void TLE9563_getTPRECHGBNK0(void);
void TLE9563_setTPRECHGBNK1(uint16_t u16_data);
void TLE9563_getTPRECHGBNK1(void);
void TLE9563_setSTICHG(uint16_t u16_data);
void TLE9563_getSTICHG(void);
void TLE9563_setHBICHGBNK0(uint16_t u16_data);
void TLE9563_getHBICHGBNK0(void);
void TLE9563_setHBICHGBNK1(uint16_t u16_data);
void TLE9563_getHBICHGBNK1(void);
void TLE9563_setHBICHGBNK2(uint16_t u16_data);
void TLE9563_getHBICHGBNK2(void);
void TLE9563_setHBICHGBNK4(uint16_t u16_data);
void TLE9563_getHBICHGBNK4(void);
void TLE9563_setHBICHGBNK5(uint16_t u16_data);
void TLE9563_getHBICHGBNK5(void);
void TLE9563_setHBICHGBNK6(uint16_t u16_data);
void TLE9563_getHBICHGBNK6(void);
void TLE9563_setHBICHGMAX(uint16_t u16_data);
void TLE9563_getHBICHGMAX(void);
void TLE9563_setHBPCHGINITBNK0(uint16_t u16_data);
void TLE9563_getHBPCHGINITBNK0(void);
void TLE9563_setHBPCHGINITBNK1(uint16_t u16_data);
void TLE9563_getHBPCHGINITBNK1(void);
void TLE9563_setHBPCHGINITBNK2(uint16_t u16_data);
void TLE9563_getHBPCHGINITBNK2(void);
void TLE9563_setTDONHBCTRLBNK0(uint16_t u16_data);
void TLE9563_getTDONHBCTRLBNK0(void);
void TLE9563_setTDONHBCTRLBNK1(uint16_t u16_data);
void TLE9563_getTDONHBCTRLBNK1(void);
void TLE9563_setTDONHBCTRLBNK2(uint16_t u16_data);
void TLE9563_getTDONHBCTRLBNK2(void);
void TLE9563_setTDOFFHBCTRLBNK0(uint16_t u16_data);
void TLE9563_getTDOFFHBCTRLBNK0(void);
void TLE9563_setTDOFFHBCTRLBNK1(uint16_t u16_data);
void TLE9563_getTDOFFHBCTRLBNK1(void);
void TLE9563_setTDOFFHBCTRLBNK2(uint16_t u16_data);
void TLE9563_getTDOFFHBCTRLBNK2(void);
void TLE9563_setBRAKE(uint16_t u16_data);
void TLE9563_getBRAKE(void);
void TLE9563_setSWKCTRL(uint16_t u16_data);
void TLE9563_getSWKCTRL(void);
void TLE9563_setSWKBTL1CTRL(uint16_t u16_data);
void TLE9563_getSWKBTL1CTRL(void);
void TLE9563_setSWKID1CTRL(uint16_t u16_data);
void TLE9563_getSWKID1CTRL(void);
void TLE9563_setSWKID0CTRL(uint16_t u16_data);
void TLE9563_getSWKID0CTRL(void);
void TLE9563_setSWKMASKID1CTRL(uint16_t u16_data);
void TLE9563_getSWKMASKID1CTRL(void);
void TLE9563_setSWKMASKID0CTRL(uint16_t u16_data);
void TLE9563_getSWKMASKID0CTRL(void);
void TLE9563_setSWKDLCCTRL(uint16_t u16_data);
void TLE9563_getSWKDLCCTRL(void);
void TLE9563_setSWKDATA3CTRL(uint16_t u16_data);
void TLE9563_getSWKDATA3CTRL(void);
void TLE9563_setSWKDATA2CTRL(uint16_t u16_data);
void TLE9563_getSWKDATA2CTRL(void);
void TLE9563_setSWKDATA1CTRL(uint16_t u16_data);
void TLE9563_getSWKDATA1CTRL(void);
void TLE9563_setSWKDATA0CTRL(uint16_t u16_data);
void TLE9563_getSWKDATA0CTRL(void);
void TLE9563_setSWKCANFDCTRL(uint16_t u16_data);
void TLE9563_getSWKCANFDCTRL(void);
void TLE9563_setSWKOSCTRIMCTRL(uint16_t u16_data);
void TLE9563_getSWKOSCTRIMCTRL(void);
void TLE9563_getSWKOSCCALSTAT(void);
void TLE9563_updateRamSWKOSCCALSTAT(uint16_t u16_data);
void TLE9563_setSWKCDRCTRL(uint16_t u16_data);
void TLE9563_getSWKCDRCTRL(void);
void TLE9563_setSWKCDRLIMIT(uint16_t u16_data);
void TLE9563_getSWKCDRLIMIT(void);
void TLE9563_getSUPSTAT(void);
void TLE9563_clrSUPSTAT(uint16_t u16_data);
void TLE9563_updateRamSUPSTAT(uint16_t u16_data);
void TLE9563_getTHERMSTAT(void);
void TLE9563_clrTHERMSTAT(uint16_t u16_data);
void TLE9563_updateRamTHERMSTAT(uint16_t u16_data);
void TLE9563_getDEVSTAT(void);
void TLE9563_clrDEVSTAT(uint16_t u16_data);
void TLE9563_updateRamDEVSTAT(uint16_t u16_data);
void TLE9563_getBUSSTAT(void);
void TLE9563_clrBUSSTAT(uint16_t u16_data);
void TLE9563_updateRamBUSSTAT(uint16_t u16_data);
void TLE9563_getWKSTAT(void);
void TLE9563_clrWKSTAT(uint16_t u16_data);
void TLE9563_updateRamWKSTAT(uint16_t u16_data);
void TLE9563_getWKLVLSTAT(void);
void TLE9563_updateRamWKLVLSTAT(uint16_t u16_data);
void TLE9563_getHSOLOCOTSTAT(void);
void TLE9563_clrHSOLOCOTSTAT(uint16_t u16_data);
void TLE9563_updateRamHSOLOCOTSTAT(uint16_t u16_data);
void TLE9563_getGENSTAT(void);
void TLE9563_updateRamGENSTAT(uint16_t u16_data);
void TLE9563_getTDREG(void);
void TLE9563_updateRamTDREG(uint16_t u16_data);
void TLE9563_getDSOV(void);
void TLE9563_clrDSOV(uint16_t u16_data);
void TLE9563_updateRamDSOV(uint16_t u16_data);
void TLE9563_getEFFTDONOFF1(void);
void TLE9563_updateRamEFFTDONOFF1(uint16_t u16_data);
void TLE9563_getEFFTDONOFF2(void);
void TLE9563_updateRamEFFTDONOFF2(uint16_t u16_data);
void TLE9563_getEFFTDONOFF3(void);
void TLE9563_updateRamEFFTDONOFF3(uint16_t u16_data);
void TLE9563_getTRISEFALL1(void);
void TLE9563_updateRamTRISEFALL1(uint16_t u16_data);
void TLE9563_getTRISEFALL2(void);
void TLE9563_updateRamTRISEFALL2(uint16_t u16_data);
void TLE9563_getTRISEFALL3(void);
void TLE9563_updateRamTRISEFALL3(uint16_t u16_data);
void TLE9563_getSWKSTAT(void);
void TLE9563_clrSWKSTAT(uint16_t u16_data);
void TLE9563_updateRamSWKSTAT(uint16_t u16_data);
void TLE9563_getSWKECNTSTAT(void);
void TLE9563_updateRamSWKECNTSTAT(uint16_t u16_data);
void TLE9563_getSWKCDRSTAT(void);
void TLE9563_updateRamSWKCDRSTAT(uint16_t u16_data);
void TLE9563_getFAMPRODSTAT(void);
void TLE9563_updateRamFAMPRODSTAT(uint16_t u16_data);

#endif /* _TLE9563_REGLAYER_H */

