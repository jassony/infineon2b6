/**
 * @cond
 ***********************************************************************************************************************
 *
 * Copyright (c) 2018, Infineon Technologies AG
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
#ifndef TLE9563_DEFINES_H
#define TLE9563_DEFINES_H

/* XML Version 2.0.1 */
#define TLE9563_XML_VERSION (20001)

#define TLE9563_BRAKE (0xA0) /*decimal 160*/

#define TLE9563_BUS_CTRL (0x0) /*decimal 0*/

#define TLE9563_CCP_BLK_BNK0 (0xD000) /*decimal 53248*/

#define TLE9563_CCP_BLK_BNK1 (0xD001) /*decimal 53249*/

#define TLE9563_CCP_BLK_BNK2 (0xD002) /*decimal 53250*/

#define TLE9563_CCP_BLK_BNK4 (0xD004) /*decimal 53252*/

#define TLE9563_CCP_BLK_BNK5 (0xD005) /*decimal 53253*/

#define TLE9563_CCP_BLK_BNK6 (0xD006) /*decimal 53254*/

#define TLE9563_CRC_EN (0x1) /*decimal 1*/

#define TLE9563_CRC_REC_INIT (0x1) /*decimal 1*/

#define TLE9563_CSA (0x705) /*decimal 1797*/

#define TLE9563_GENCTRL (0x821) /*decimal 2081*/

#define TLE9563_HBMODE (0x0) /*decimal 0*/

#define TLE9563_HB_ICHG_BNK0 (0x3CD0) /*decimal 15568*/

#define TLE9563_HB_ICHG_BNK1 (0x3CD1) /*decimal 15569*/

#define TLE9563_HB_ICHG_BNK2 (0x3CD2) /*decimal 15570*/

#define TLE9563_HB_ICHG_BNK4 (0x3CD4) /*decimal 15572*/

#define TLE9563_HB_ICHG_BNK5 (0x3CD5) /*decimal 15573*/

#define TLE9563_HB_ICHG_BNK6 (0x3CD6) /*decimal 15574*/

#define TLE9563_HB_ICHG_MAX (0x0) /*decimal 0*/

#define TLE9563_HB_PCHG_INIT_BNK0 (0x3CD0) /*decimal 15568*/

#define TLE9563_HB_PCHG_INIT_BNK1 (0x3CD1) /*decimal 15569*/

#define TLE9563_HB_PCHG_INIT_BNK2 (0x3CD2) /*decimal 15570*/

#define TLE9563_HS_CTRL (0x0) /*decimal 0*/

#define TLE9563_HS_VDS (0x49) /*decimal 73*/

#define TLE9563_HW_CTRL (0x0) /*decimal 0*/

#define TLE9563_INT_MASK (0x140) /*decimal 320*/

#define TLE9563_LS_VDS (0x1049) /*decimal 4169*/

#define TLE9563_M_S_CTRL (0x600) /*decimal 1536*/

#define TLE9563_PWM_CTRL_BNK0 (0x0) /*decimal 0*/

#define TLE9563_PWM_CTRL_BNK1 (0x1) /*decimal 1*/

#define TLE9563_PWM_CTRL_BNK2 (0x2) /*decimal 2*/

#define TLE9563_PWM_CTRL_BNK3 (0x3) /*decimal 3*/

#define TLE9563_ST_ICHG (0x444) /*decimal 1092*/

#define TLE9563_SWK_BTL1_CTRL (0xCC60) /*decimal 52320*/

#define TLE9563_SWK_CAN_FD_CTRL (0x0) /*decimal 0*/

#define TLE9563_SWK_CDR_CTRL (0x4) /*decimal 4*/

#define TLE9563_SWK_CDR_LIMIT (0x9D8F) /*decimal 40335*/

#define TLE9563_SWK_CTRL (0x0) /*decimal 0*/

#define TLE9563_SWK_DATA0_CTRL (0x0) /*decimal 0*/

#define TLE9563_SWK_DATA1_CTRL (0x0) /*decimal 0*/

#define TLE9563_SWK_DATA2_CTRL (0x0) /*decimal 0*/

#define TLE9563_SWK_DATA3_CTRL (0x0) /*decimal 0*/

#define TLE9563_SWK_DLC_CTRL (0x0) /*decimal 0*/

#define TLE9563_SWK_ID0_CTRL (0x0) /*decimal 0*/

#define TLE9563_SWK_ID1_CTRL (0x0) /*decimal 0*/

#define TLE9563_SWK_MASK_ID0_CTRL (0x0) /*decimal 0*/

#define TLE9563_SWK_MASK_ID1_CTRL (0x0) /*decimal 0*/

#define TLE9563_SWK_OSC_CAL_STAT (0x0) /*decimal 0*/

#define TLE9563_SWK_OSC_TRIM_CTRL (0x0) /*decimal 0*/

#define TLE9563_SW_SD_CTRL (0x0) /*decimal 0*/

#define TLE9563_SYS_STAT_CTRL (0x0) /*decimal 0*/

#define TLE9563_TDOFF_HB_CTRL_BNK0 (0xC00) /*decimal 3072*/

#define TLE9563_TDOFF_HB_CTRL_BNK1 (0xC01) /*decimal 3073*/

#define TLE9563_TDOFF_HB_CTRL_BNK2 (0xC02) /*decimal 3074*/

#define TLE9563_TDON_HB_CTRL_BNK0 (0xC00) /*decimal 3072*/

#define TLE9563_TDON_HB_CTRL_BNK1 (0xC01) /*decimal 3073*/

#define TLE9563_TDON_HB_CTRL_BNK2 (0xC02) /*decimal 3074*/

#define TLE9563_TIMER_CTRL (0x46) /*decimal 70*/

#define TLE9563_TPRECHG_BNK0 (0x0) /*decimal 0*/

#define TLE9563_TPRECHG_BNK1 (0x1) /*decimal 1*/

#define TLE9563_WD_CTRL (0x34) /*decimal 52*/

#define TLE9563_WK_CTRL_BNK3 (0x23) /*decimal 35*/

#define TLE9563_WK_CTRL_BNK4 (0x24) /*decimal 36*/

#endif /* TLE9563_DEFINES_H */
