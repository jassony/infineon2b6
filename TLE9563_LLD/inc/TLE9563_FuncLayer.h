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
 * \file     tle9563.h
 *
 * \brief    tle9563 low level access library
 *
 * \version  V0.2.4
 * \date     22. Mar 2022
 *
 * \note 
 */

/*******************************************************************************
**                             Author(s) Identity                             **
********************************************************************************
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** JO           Julia Ott                                                     **
** BG           Blandine Guillot                                              **
** VO           Vanessa Ongaro                                                **
*******************************************************************************/

/*******************************************************************************
**                          Revision Control History                          **
********************************************************************************
** V0.1.0: 2021-12-06, VO:   Initial version                                  **
** V0.1.1: 2022-01-05, VO:   [EP-994] first version of cyclic task fkt, error **
**                           handling fkt and watchdog serve fkt              **
**                           Adjusted bitfield fkts for use in  cyclic task   **
**                           Use local variable to return success             **
**                           Replaced (void *)0 with NULL                     **
**                           Added error codes for pdma and device errors     **
**                           Added return value for init fkt, cyclic task and **
**                           error handling fkt                               **
** V0.1.2: 2022-01-19, VO:   [EP-994] Cyclic task polls status of PDMA instead**
**                           of using an interrupt for PDMA completion        **
**                           Deleted commented out lines and added briefs     **
** V0.1.3: 2022-01-26, VO:   [EP-994] Updated fkts for use in cyclic task     **
**                           with new concept: cyclic update of status        **
**                           registers and return values of get fkts from     **
**                           register RAM copy                                **
**                           Removed Modify and Mask fkts                     **
**                           Added fkts to set complete registers and         **
**                           according fkt pointer type, to be used in init   **
**                           Adjusted device driver struct                    **
**                           Adjusted device driver cyclic task               **
**                           Added arrays with fkts for cyclic update, to be  **
**                           used in device driver cyclic task                **
**                           Adjusted init fkt to use fkt which set complete  **
**                           registers                                        **
** V0.1.4: 2022-01-26, VO:   All watchdog funktions which write to WD_CTRL    **
**                           register adjust parity bit                       **
** V0.1.5: 2022-02-02, VO:   Removed call of cyclic task inside 'TLE9563_set' **
**                           functions                                        **
**                           Moved init of WD Ctrl register to end of init    **
**                           routine                                          **
** V0.1.6: 2022-02-07, VO:   [EP-1003] Added function and enum to configure   **
**                           HB MODE control register with only one single    **
**                           SPI write command                                **
**                           Adjusted set bitfield functions to mask bits     **
**                           Added resetLastSetBitfieldRequest() function     **
**                           Added functions to get individual bits of SIF    **
**                           Updated briefs                                   **
** V0.1.7: 2022-02-08, VO:   [EP-994] Adjusted cyclic task to send out SPI    **
**                           msg on every call, if not busy. Adjusted resturn **
**                           value and error log accordingly                  **
**                           Adjusted Init fct according to changes in cyclic **
**                           task                                             **
** V0.1.8: 2022-02-10, VO:   [EP-994] Added comment about return value of     **
**                           cyclic task                                      **
** V0.1.9: 2022-02-11, VO:   [EP-994] Corrected comments of en/dis functions  **
** V0.2.0: 2022-02-16, VO:   [EP-1036] Added get function to get full status  **
**                           and control regsiters                            **
** V0.2.1: 2022-02-16, VO:   [EP-1037] Added comments to set device mode      **
**                           Changed init order of HS_CTRL and PWM_CTRL       **
**                           according to datasheet recommendation            **
** V0.2.2: 2022-02-18, VO:   [EP-1039] Added first status register read out   **
**                           to init routine, WD init moved to end of init to **
**                           close WD long open window at end of init         **
**                           Updated Init func with meaningful check for SPI  **
**                           errors                                           **
** V0.2.3: 2022-03-09, VO:   [EP-1038] updated main and systick               **
** V0.2.4: 2022-03-22, VO:   [EP-1038] removed TLE9563_setWdChecksumBit       **
*******************************************************************************/

#ifndef _TLE9563_H
#define _TLE9563_H

/** \addtogroup TLE9563_FUNCLAYER_api
 *  @{
 */

/*******************************************************************************
**                                  Includes                                  **
*******************************************************************************/
#include "TLE9563.h"
#include "TLE9563_RegLayer.h"
#include "TLE9563_CommLayer.h"
#include "TLE9563_defines.h"
   
/*******************************************************************************
**                          Global Macro Declarations                         **
*******************************************************************************/
#define TLE9563_DEVICEDRIVER_STATUS_INIT                0
#define TLE9563_DEVICEDRIVER_STATUS_CYCLIC_UPDATE       1

#define TLE9563_DEVICEDRIVER_ERRORLOG_INIT              0
#define TLE9563_DEVICEDRIVER_ERRORLOG_STARTED           1
#define TLE9563_DEVICEDRIVER_ERRORLOG_BUSY              2
#define TLE9563_DEVICEDRIVER_ERRORLOG_SPI_RECEIVE_ERR   3

#define TLE9563_DEVICEDRIVER_CYCLICSTATUPDATENUM       19

/*******************************************************************************
**                          Global Type Declarations                          **
*******************************************************************************/
/** \enum tDEVICE_mode
 *  \brief Enum for the TLE9563 Device Mode Control
 */
typedef enum _tDEVICE_mode
{
  DEVICE_mode_normal = 0,                                   /** \brief  Normal Mode */
  DEVICE_mode_sleep = 1,                                    /** \brief  Sleep Mode */
  DEVICE_mode_stop = 2,                                     /** \brief  Stop Mode */
  DEVICE_mode_reset = 3                                     /** \brief  Device reset Soft reset is executed (configuration of RSTN triggering in bit SOFT_RESET_RO) */
} tDEVICE_mode;

/** \enum tVCC1_overVoltReact
 *  \brief Enum for the TLE9563 Reaction in case of VCC1 Over Voltage
 */
typedef enum _tVCC1_overVoltReact
{
  VCC1_overVoltReact_no = 0,                                /** \brief  no reaction */
  VCC1_overVoltReact_intn = 1,                              /** \brief  INTN event is generated */
  VCC1_overVoltReact_rstn = 2,                              /** \brief  RSTN event is generated */
  VCC1_overVoltReact_failsafe = 3                           /** \brief  Fail-Safe Mode is entered */
} tVCC1_overVoltReact;

/** \enum tVCC1_underVoltResetHys
 *  \brief Enum for the TLE9563 VCC1 Undervoltage Reset Hysteresis Selection (see also Chapter 12.7.1 for more information)
 */
typedef enum _tVCC1_underVoltResetHys
{
  VCC1_underVoltResetHys_default = 0,                       /** \brief  default hysteresis applies as specified in the electrical characteristics table */
  VCC1_underVoltResetHys_highest = 1                        /** \brief  the highest rising threshold (VRT1,R) is always used for the release of the undervoltage reset */
} tVCC1_underVoltResetHys;

/** \enum tVCC1_actPeakThresh
 *  \brief Enum for the TLE9563 VCC1 Active Peak Threshold Selection
 */
typedef enum _tVCC1_actPeakThresh
{
  VCC1_actPeakThresh_low = 0,                               /** \brief  low VCC1 active peak threshold selected */
  VCC1_actPeakThresh_high = 1                               /** \brief  high VCC1 active peak threshold selected */
} tVCC1_actPeakThresh;

/** \enum tVCC1_resetThresh
 *  \brief Enum for the TLE9563 VCC1 Reset Threshold Control
 */
typedef enum _tVCC1_resetThresh
{
  VCC1_resetThresh_vrt1 = 0,                                /** \brief  Vrt1 selected (highest threshold) */
  VCC1_resetThresh_vrt2 = 1,                                /** \brief  Vrt2 selected */
  VCC1_resetThresh_vrt3 = 2,                                /** \brief  Vrt3 selected */
  VCC1_resetThresh_vrt4 = 3                                 /** \brief  Vrt4 selected */
} tVCC1_resetThresh;

/** \enum tTSD2_minWaitTime
 *  \brief Enum for the TLE9563 TSD2 minimum Waiting Time Selection
 */
typedef enum _tTSD2_minWaitTime
{
  TSD2_minWaitTime_1s = 0,                                  /** \brief  Minimum waiting time until TSD2 is released again is always 1 s */
  TSD2_minWaitTime_64s = 1                                  /** \brief  Minimum waiting time until TSD2 is released again is 1 s, after >16 TSD2 consecutive events, it will extended x 64 */
} tTSD2_minWaitTime;

/** \enum tVS_overVoltThresh
 *  \brief Enum for the TLE9563 VS OV comparator threshold change
 */
typedef enum _tVS_overVoltThresh
{
  VS_overVoltThresh_20v = 0,                                /** \brief  Default threshold setting (VS,OVD1) */
  VS_overVoltThresh_30v = 1                                 /** \brief  increased threshold setting (VS,OVD2) */
} tVS_overVoltThresh;

/** \enum tRST_delayTime
 *  \brief Enum for the TLE9563 Reset delay time
 */
typedef enum _tRST_delayTime
{
  RST_delayTime_10ms = 0,                                   /** \brief  Reset delay time 10 ms (tRD1) */
  RST_delayTime_2ms = 1                                     /** \brief  Reset delay time to 2 ms (tRD2) */
} tRST_delayTime;

/** \enum tSOFTRST_config
 *  \brief Enum for the TLE9563 Soft Reset Configuration
 */
typedef enum _tSOFTRST_config
{
  SOFTRST_config_rstn = 0,                                  /** \brief  RSTN will be triggered (pulled low) during a Soft Reset */
  SOFTRST_config_norstn = 1                                 /** \brief  no RSTN trigger during a Soft Reset */
} tSOFTRST_config;

/** \enum tWD_config
 *  \brief Enum for the TLE9563 Watchdog Configuration
 */
typedef enum _tWD_config
{
  WD_config_timeout = 0,                                    /** \brief  Watchdog works as a Time-Out watchdog */
  WD_config_window = 1                                      /** \brief  Watchdog works as a Window watchdog */
} tWD_config;

/** \enum tWD_timerPeriod
 *  \brief Enum for the TLE9563 Watchdog Timer Period
 */
typedef enum _tWD_timerPeriod
{
  WD_timerPeriod_10ms = 0,                                  /** \brief  10ms */
  WD_timerPeriod_20ms = 1,                                  /** \brief  20ms */
  WD_timerPeriod_50ms = 2,                                  /** \brief  50ms */
  WD_timerPeriod_100ms = 3,                                 /** \brief  100ms */
  WD_timerPeriod_200ms = 4,                                 /** \brief  200ms */
  WD_timerPeriod_500ms = 5,                                 /** \brief  500ms */
  WD_timerPeriod_1s = 6,                                    /** \brief  1s */
  WD_timerPeriod_10s = 7                                    /** \brief  10s */
} tWD_timerPeriod;

/** \enum tCAN_mode
 *  \brief Enum for the TLE9563 HS-CAN Module Modes
 */
typedef enum _tCAN_mode
{
  CAN_mode_off = 0,                                         /** \brief  CAN OFF */
  CAN_mode_wake = 1,                                        /** \brief  CAN is wake capable (no SWK) */
  CAN_mode_receive = 2,                                     /** \brief  CAN Receive Only Mode (no SWK) */
  CAN_mode_normal = 3,                                      /** \brief  CAN Normal Mode (no SWK) */
  CAN_mode_wakeswk = 5,                                     /** \brief  CAN is wake capable with SWK */
  CAN_mode_receiveswk = 6,                                  /** \brief  CAN Receive Only Mode with SWK */
  CAN_mode_normalswk = 7                                    /** \brief  CAN Normal Mode with SWK */
} tCAN_mode;

/** \enum tWK_filtTime
 *  \brief Enum for the TLE9563 Wake-up Filter Time Configuration
 */
typedef enum _tWK_filtTime
{
  WK_filtTime_16us = 0,                                     /** \brief  Filter with 16 i.ts filter time (static sensing)  */
  WK_filtTime_64us = 1,                                     /** \brief  Filter with 64 i.ts filter time (static sensing)  */
  WK_filtTime_timer1 = 2,                                   /** \brief  Filtering at the end of the on-time; filter */
  WK_filtTime_timer2 = 3,                                   /** \brief  Filtering at the end of the on-time; filter */
  WK_filtTime_sync = 4,                                     /** \brief  Filter at the end of settle time (80 i.ts), filter time of 16 i.ts (cyclic sensing) is selected, SYNC1)2)  */
} tWK_filtTime;

/** \enum tWK_pullUpPullDown
 *  \brief Enum for the TLE9563 WKx Pull-Up/Pull-Down Configuration
 */
typedef enum _tWK_pullUpPullDown
{
  WK_pullUpPullDown_no = 0,                                 /** \brief  No pull-up/pull-down selected */
  WK_pullUpPullDown_pulldown = 1,                           /** \brief  Pull-down resistor selected */
  WK_pullUpPullDown_pullup = 2,                             /** \brief  Pull-up resistor selected3) */
  WK_pullUpPullDown_auto = 3                                /** \brief  Automatic switching to pull-up or pull-down */
} tWK_pullUpPullDown;

/** \enum tTIMER_onTime
 *  \brief Enum for the TLE9563 Timer2 On-Time Configuration
 */
typedef enum _tTIMER_onTime
{
  TIMER_onTime_offlow = 0,                                  /** \brief  OFF / Low (timer not running, HSx output is low) */
  TIMER_onTime_100us = 1,                                   /** \brief  0.1ms on-time */
  TIMER_onTime_300us = 2,                                   /** \brief  0.3ms on-time */
  TIMER_onTime_1ms = 3,                                     /** \brief  1.0ms on-time */
  TIMER_onTime_10ms = 4,                                    /** \brief  10ms on-time */
  TIMER_onTime_20ms = 5,                                    /** \brief  20ms on-time */
  TIMER_onTime_offhigh = 6,                                 /** \brief  OFF / HIGH (timer not running, HSx */
  TIMER_onTime_ = 7                                         /** \brief  reserved, same behaviour as 110B */
} tTIMER_onTime;

/** \enum tTIMER_period
 *  \brief Enum for the TLE9563 Timer2 Period Configuration
 */
typedef enum _tTIMER_period
{
  TIMER_period_10ms = 0,                                    /** \brief  10ms */
  TIMER_period_20ms = 1,                                    /** \brief  20ms */
  TIMER_period_50ms = 2,                                    /** \brief  50ms */
  TIMER_period_100ms = 3,                                   /** \brief  100ms */
  TIMER_period_200ms = 4,                                   /** \brief  200ms */
  TIMER_period_500ms = 5,                                   /** \brief  500ms */
  TIMER_period_1s = 6,                                      /** \brief  1s */
  TIMER_period_2s = 7                                       /** \brief  2s */
} tTIMER_period;

/** \enum tTIMER_cyclicWk
 *  \brief Enum for the TLE9563 Cyclic Wake Configuration
 */
typedef enum _tTIMER_cyclicWk
{
  TIMER_cyclicWk_disabled = 0,                              /** \brief  Timer1 and Timer2 disabled as wake-up sources */
  TIMER_cyclicWk_timer1 = 1,                                /** \brief  Timer1 is enabled as wake-up source (Cyclic Wake) */
  TIMER_cyclicWk_timer2 = 2,                                /** \brief  Timer2 is enabled as wake-up source (Cyclic Wake) */
  TIMER_cyclicWk_ = 3                                       /** \brief  reserved */
} tTIMER_cyclicWk;

/** \enum tHS_vsOverVoltRecovery
 *  \brief Enum for the TLE9563 Switch recovery after removal of VS Overvoltage for HS3
 */
typedef enum _tHS_vsOverVoltRecovery
{
  HS_vsOverVoltRecovery_disabled = 0,                       /** \brief  Switch recovery is disabled */
  HS_vsOverVoltRecovery_previous = 1                        /** \brief  Previous state before VS Overvoltage is enabled after Overvoltage considtion is removed */
} tHS_vsOverVoltRecovery;

/** \enum tHS_vsUnderVoltRecovery
 *  \brief Enum for the TLE9563 Switch recovery after removal of Undervoltage for HSx
 */
typedef enum _tHS_vsUnderVoltRecovery
{
  HS_vsUnderVoltRecovery_disabled = 0,                      /** \brief  Switch recovery is disabled */
  HS_vsUnderVoltRecovery_previous = 1                       /** \brief  Previous state before VS */
} tHS_vsUnderVoltRecovery;

/** \enum tHS_config
 *  \brief Enum for the TLE9563 HS3 Configuration
 */
typedef enum _tHS_config
{
  HS_config_off = 0,                                        /** \brief  OFF */
  HS_config_on = 1,                                         /** \brief  ON */
  HS_config_timer1 = 2,                                     /** \brief  Controlled by Timer1 */
  HS_config_timer2 = 3,                                     /** \brief  Controlled by Timer2 */
  HS_config_pwm1 = 4,                                       /** \brief  Controlled by PWM1 */
  HS_config_pwm2 = 5,                                       /** \brief  Controlled by PWM2 */
  HS_config_pwm3 = 6,                                       /** \brief  Controlled by PWM3 */
  HS_config_pwm4 = 7,                                       /** \brief  Controlled by PWM4 */
  HS_config_wk4sync = 8                                     /** \brief  Synchronized with WK4/SYNC */
} tHS_config;

/** \enum tPWM_freq
 *  \brief Enum for the TLE9563 PWM generator Frequency Setting
 */
typedef enum _tPWM_freq
{
  PWM_freq_100hz = 0,                                       /** \brief  100Hz is selected */
  PWM_freq_200hz = 1                                        /** \brief  200Hz is selected */
} tPWM_freq;

/** \enum tBDRV_freq
 *  \brief Enum for the TLE9563 Bridge driver synchronization frequency
 */
typedef enum _tBDRV_freq
{
  BDRV_freq_18mhz = 0,                                      /** \brief  typ. 18.75 MHz (default) */
  BDRV_freq_37mhz = 1                                       /** \brief  typ. 37.5 MHz */
} tBDRV_freq;

/** \enum tCP_underVoltThresh
 *  \brief Enum for the TLE9563 Charge pump under voltage (referred to VS)
 */
typedef enum _tCP_underVoltThresh
{
  CP_underVoltThresh_th1 = 0,                               /** \brief  (default) CPUV threshold 1 for FET_LVL = 0, CPUV threshold 1 for FET_LVL = 1 */
  CP_underVoltThresh_th2 = 1                                /** \brief  CPUV threshold 2 for FET_LVL = 0, CPUV threshold 2 for FET_LVL = 1 */
} tCP_underVoltThresh;

/** \enum tBDRV_extMosfetLvl
 *  \brief Enum for the TLE9563 External MOSFET normal / logic level selection
 */
typedef enum _tBDRV_extMosfetLvl
{
  BDRV_extMosfetLvl_logic = 0,                              /** \brief  Logic level MOSFET selected */
  BDRV_extMosfetLvl_normal = 1                              /** \brief  Normal level MOSFET selected(default) */
} tBDRV_extMosfetLvl;

/** \enum tBDRV_preChargePreDischargeCurAdaption
 *  \brief Enum for the TLE9563 Adaptation of the pre-charge and pre-discharge current
 */
typedef enum _tBDRV_preChargePreDischargeCurAdaption
{
  BDRV_preChargePreDischargeCurAdaption_1step = 0,          /** \brief  1 current step (default) */
  BDRV_preChargePreDischargeCurAdaption_2steps = 1          /** \brief  2 current steps */
} tBDRV_preChargePreDischargeCurAdaption;

/** \enum tBDRV_adaptGateCtrl
 *  \brief Enum for the TLE9563 Adaptive gate control
 */
typedef enum _tBDRV_adaptGateCtrl
{
  BDRV_adaptGateCtrl_inactive1 = 0,                         /** \brief  (default) Adaptive gate control disabled, pre-charge and pre-discharge disabled */
  BDRV_adaptGateCtrl_inactive2 = 1,                         /** \brief  Adaptive gate control disabled, precharge is enabled with IPRECHG = IPCHGINIT, predischarge is  enabled with IPREDCHG = IPDCHGINIT */
  BDRV_adaptGateCtrl_active = 2,                            /** \brief  Adaptive gate control enabled, IPRECHG and IPREDCHG are self adapted */
  BDRV_adaptGateCtrl_reserved = 3                           /** \brief  Adaptive gate control enabled, IPRECHG and IPREDCHG are self adapted */
} tBDRV_adaptGateCtrl;

/** \enum tBDRV_holdCur
 *  \brief Enum for the TLE9563 Gate driver hold current IHOLD
 */
typedef enum _tBDRV_holdCur
{
  BDRV_holdCur_th1 = 0,                                     /** \brief  (default) Charge 'CHG15, discharge 'DCHG15. */
  BDRV_holdCur_th2 = 1                                      /** \brief  Charge 'CHG20, discharge 'DCHG20 */
} tBDRV_holdCur;

/** \enum tBDRV_numberOfPwmInputs
 *  \brief Enum for the TLE9563 Selection of 3 or 6 PWM inputs
 */
typedef enum _tBDRV_numberOfPwmInputs
{
  BDRV_numberOfPwmInputs_3pwm = 0,                          /** \brief  3 PWM inputs (default) */
  BDRV_numberOfPwmInputs_6pwm = 1                           /** \brief  6 PWM inputs */
} tBDRV_numberOfPwmInputs;

/** \enum tCSA_cap
 *  \brief Enum for the TLE9563 Capacitance connected to the current sense amplifier output (CCSO), see also Chapter 12.12.4
 */
typedef enum _tCSA_cap
{
  CSA_cap_400pf = 0,                                        /** \brief  CCSO < 400 pF (default) */
  CSA_cap_2nf = 1                                           /** \brief  400 pF < CCSO < 2.2 nF */
} tCSA_cap;

/** \enum tCSA_dir
 *  \brief Enum for the TLE9563 Direction of the current sense amplifier
 */
typedef enum _tCSA_dir
{
  CSA_dir_uni = 0,                                          /** \brief  Unidirectional */
  CSA_dir_bi = 1                                            /** \brief  Bidirectional (default) */
} tCSA_dir;

/** \enum tCSA_overCurFiltTime
 *  \brief Enum for the TLE9563 Overcurrent filter time of CSO
 */
typedef enum _tCSA_overCurFiltTime
{
  CSA_overCurFiltTime_6us = 0,                              /** \brief  6 us (default) */
  CSA_overCurFiltTime_10us = 1,                             /** \brief  10 us */
  CSA_overCurFiltTime_50us = 2,                             /** \brief  50 us */
  CSA_overCurFiltTime_100us = 3                             /** \brief  100 us */
} tCSA_overCurFiltTime;

/** \enum tCSA_overCurThresh
 *  \brief Enum for the TLE9563 Overcurrent detection threshold of CSO
 */
typedef enum _tCSA_overCurThresh
{
  CSA_overCurThresh_th1 = 0,                                /** \brief  VCSO > VCC1/2+2 x VCC1/20 or VCSOx< VCC1/2- 2x VCC1/20 (default) */
  CSA_overCurThresh_th2 = 1,                                /** \brief  VCSO > VCC1/2+ 4x VCC1/20 or VCSOx< VCC1/2- 4x VCC1/20 */
  CSA_overCurThresh_th3 = 2,                                /** \brief  VCSO > VCC1/2+ 5 x VCC1/20 or VCSOx< VCC1/2- 5 xVCC1/20 */
  CSA_overCurThresh_th4 = 3                                 /** \brief  VCSO > VCC1/2+ 6x VCC1/20 or VCSOx< VCC1/2- 6x VCC1/20 */
} tCSA_overCurThresh;

/** \enum tCSA_gain
 *  \brief Enum for the TLE9563 Gain of the current sense amplifier
 */
typedef enum _tCSA_gain
{
  CSA_gain_10vv = 0,                                        /** \brief  GDIFF10 (default) */
  CSA_gain_20vv = 1,                                        /** \brief  GDIFF20 */
  CSA_gain_40vv = 2,                                        /** \brief  GDIFF40 */
  CSA_gain_60vv = 3                                         /** \brief  GDIFF60 */
} tCSA_gain;

/** \enum tVDS_filtTime
 *  \brief Enum for the TLE9563 Filter time of drain-source voltage monitoring
 */
typedef enum _tVDS_filtTime
{
  VDS_filtTime_500ns = 0,                                   /** \brief  0.5 us (default) */
  VDS_filtTime_1us = 1,                                     /** \brief  1 us */
  VDS_filtTime_2us = 2,                                     /** \brief  2 us */
  VDS_filtTime_6us = 3                                      /** \brief  6 us */
} tVDS_filtTime;

/** \enum tVDS_lsOverVoltThresh
 *  \brief Enum for the TLE9563 LS3 drain-source overvoltage threshold
 */
typedef enum _tVDS_lsOverVoltThresh
{
  VDS_lsOverVoltThresh_160mv = 0,                           /** \brief  0.16 V */
  VDS_lsOverVoltThresh_200mv = 1,                           /** \brief  0.20 V (default) */
  VDS_lsOverVoltThresh_300mv = 2,                           /** \brief  0.30 V */
  VDS_lsOverVoltThresh_400mv = 3,                           /** \brief  0.40 V */
  VDS_lsOverVoltThresh_500mv = 4,                           /** \brief  0.50 V */
  VDS_lsOverVoltThresh_600mv = 5,                           /** \brief  0.60 V */
  VDS_lsOverVoltThresh_800mv = 6,                           /** \brief  0.80 V */
  VDS_lsOverVoltThresh_2v = 7                               /** \brief  2.0 V */
} tVDS_lsOverVoltThresh;

/** \enum tVDS_hsOverVoltThresh
 *  \brief Enum for the TLE9563 HS3 drain-source overvoltage threshold
 */
typedef enum _tVDS_hsOverVoltThresh
{
  VDS_hsOverVoltThresh_160mv = 0,                           /** \brief  0.16 V */
  VDS_hsOverVoltThresh_200mv = 1,                           /** \brief  0.20 V (default) */
  VDS_hsOverVoltThresh_300mv = 2,                           /** \brief  0.30 V */
  VDS_hsOverVoltThresh_400mv = 3,                           /** \brief  0.40 V */
  VDS_hsOverVoltThresh_500mv = 4,                           /** \brief  0.50 V */
  VDS_hsOverVoltThresh_600mv = 5,                           /** \brief  0.60 V */
  VDS_hsOverVoltThresh_800mv = 6,                           /** \brief  0.80 V */
  VDS_hsOverVoltThresh_2v = 7                               /** \brief  2.0 V */
} tVDS_hsOverVoltThresh;

/** \enum tBDRV_hb3Mode
 *  \brief Enum for the TLE9563 Half-bridge 3 MODE selection
 */
typedef enum _tBDRV_hb3Mode
{
  BDRV_hb3Mode_passiveoff = 0,                              /** \brief  LS3 and HS3 are off by passive discharge (default) */
  BDRV_hb3Mode_ls3on = 1,                                   /** \brief  LS3 is ON */
  BDRV_hb3Mode_hs3on = 2,                                   /** \brief  HS3 is ON */
  BDRV_hb3Mode_activeoff = 3                                /** \brief  LS3 and HS3 kept off by the active discharge */
} tBDRV_hb3Mode;

/** \enum tBDRV_hb2Mode
 *  \brief Enum for the TLE9563 Half-bridge 2 MODE selection
 */
typedef enum _tBDRV_hb2Mode
{
  BDRV_hb2Mode_passiveoff = 0,                              /** \brief  LS2 and HS2 are off by passive discharge (default) */
  BDRV_hb2Mode_ls2on = 1,                                   /** \brief  LS2 is ON */
  BDRV_hb2Mode_hs2on = 2,                                   /** \brief  HS2 is ON */
  BDRV_hb2Mode_activeoff = 3                                /** \brief  LS2 and HS2 kept off by the active discharge */
} tBDRV_hb2Mode;

/** \enum tBDRV_hb1Mode
 *  \brief Enum for the TLE9563 Half-bridge 1 MODE selection
 */
typedef enum _tBDRV_hb1Mode
{
  BDRV_hb1Mode_passiveoff = 0,                              /** \brief  LS1 and HS1 are off by passive discharge (default) */
  BDRV_hb1Mode_ls1on = 1,                                   /** \brief  LS1 is ON */
  BDRV_hb1Mode_hs1on = 2,                                   /** \brief  HS1 is ON */
  BDRV_hb1Mode_activeoff = 3                                /** \brief  LS1 and HS1 kept off by the active discharge */
} tBDRV_hb1Mode;

/** \enum tBDRV_maxChargeCur
 *  \brief Enum for the TLE9563 Maximum drive current of HB3 during the pre-charge and pre-discharge phases1)
 */
typedef enum _tBDRV_maxChargeCur
{
  BDRV_maxChargeCur_31ma = 0,                               /** \brief  charge ICHG24 typ. 31.6 mA, discharge IDCHG24 typ. 30.9 mA (default) */
  BDRV_maxChargeCur_52ma = 1,                               /** \brief  charge ICHG32 typ. 52.5 mA, discharge IDCHG32 typ. 51.5 mA */
  BDRV_maxChargeCur_112ma = 2,                              /** \brief  charge ICHG52 typ. 112.2mA, discharge IDCHG52 typ. 110.8 mA */
  BDRV_maxChargeCur_150ma = 3                               /** \brief  charge ICHG63 typ. 150 mA, discharge IDCHG63 typ. 150 mA */
} tBDRV_maxChargeCur;

/** \enum tVDS_overVoltThreshLsBrake
 *  \brief Enum for the TLE9563 VDS Overvoltage for LS1-3 during braking
 */
typedef enum _tVDS_overVoltThreshLsBrake
{
  VDS_overVoltThreshLsBrake_800mv = 0,                      /** \brief  VVDSMONTH0_BRAKE, 0.8 V, typ. (default) */
  VDS_overVoltThreshLsBrake_220mv = 1                       /** \brief  VVDSMONTH1_BRAKE, 0.22 V typ. */
} tVDS_overVoltThreshLsBrake;

/** \enum tVDS_overVoltBlankTimeBrake
 *  \brief Enum for the TLE9563 Blank time of VDS overvoltage during braking
 */
typedef enum _tVDS_overVoltBlankTimeBrake
{
  VDS_overVoltBlankTimeBrake_7us = 0,                       /** \brief  tBLK_BRAKE1,7 us typ. */
  VDS_overVoltBlankTimeBrake_11us = 1                       /** \brief  tBLK_BRAKE2, 11 us typ. (default) */
} tVDS_overVoltBlankTimeBrake;

/** \enum tVS_overVoltThreshBrake
 *  \brief Enum for the TLE9563 Overvoltage brake threshold
 */
typedef enum _tVS_overVoltThreshBrake
{
  VS_overVoltThreshBrake_27v = 0,                           /** \brief  typ. 27V (default) */
  VS_overVoltThreshBrake_28v = 1,                           /** \brief  typ. 28V */
  VS_overVoltThreshBrake_29v = 2,                           /** \brief  typ. 29V */
  VS_overVoltThreshBrake_30v = 3,                           /** \brief  typ. 30V */
  VS_overVoltThreshBrake_31v = 4,                           /** \brief  typ. 31V */
  VS_overVoltThreshBrake_32v = 5,                           /** \brief  typ. 32V */
  VS_overVoltThreshBrake_33v = 6,                           /** \brief  typ. 33V */
  VS_overVoltThreshBrake_34v = 7                            /** \brief  typ. 34V */
} tVS_overVoltThreshBrake;

/** \enum tSWK_remoteTransReq
 *  \brief Enum for the TLE9563 Remote Transmission Request Field (acc. ISO11898-22016)
 */
typedef enum _tSWK_remoteTransReq
{
  SWK_remoteTransReq_normal = 0,                            /** \brief  Normal Data Frame */
  SWK_remoteTransReq_remote = 1                             /** \brief  Remote Transmission Request */
} tSWK_remoteTransReq;

/** \enum tSWK_idExtBit
 *  \brief Enum for the TLE9563 Identifier Extension Bit
 */
typedef enum _tSWK_idExtBit
{
  SWK_idExtBit_std = 0,                                     /** \brief  Standard Identifier Length (11 bit) */
  SWK_idExtBit_ext = 1                                      /** \brief  Extended Identifier Length (29 bit) */
} tSWK_idExtBit;

/** \enum tSWK_dataLengthCode
 *  \brief Enum for the TLE9563 Payload length in number of bytes 
 */
typedef enum _tSWK_dataLengthCode
{
  SWK_dataLengthCode_0 = 0,                                 /** \brief  Frame Data Length = 0 or cleared */
  SWK_dataLengthCode_1 = 1,                                 /** \brief  Frame Data Length = 1 */
  SWK_dataLengthCode_2 = 2,                                 /** \brief  Frame Data Length = 2 */
  SWK_dataLengthCode_3 = 3,                                 /** \brief  Frame Data Length = 3 */
  SWK_dataLengthCode_4 = 4,                                 /** \brief  Frame Data Length = 4 */
  SWK_dataLengthCode_5 = 5,                                 /** \brief  Frame Data Length = 5 */
  SWK_dataLengthCode_6 = 6,                                 /** \brief  Frame Data Length = 6 */
  SWK_dataLengthCode_7 = 7,                                 /** \brief  Frame Data Length = 7 */
  SWK_dataLengthCode_8 = 8                                  /** \brief  to 1111B Frame Data Length = 8 */
} tSWK_dataLengthCode;

/** \enum tCANFD_domFiltTime
 *  \brief Enum for the TLE9563 CAN FD Dominant Filter Time
 */
typedef enum _tCANFD_domFiltTime
{
  CANFD_domFiltTime_50ns = 0,                               /** \brief  50 ns */
  CANFD_domFiltTime_100ns = 1,                              /** \brief  100 ns */
  CANFD_domFiltTime_150ns = 2,                              /** \brief  150 ns */
  CANFD_domFiltTime_200ns = 3,                              /** \brief  200 ns */
  CANFD_domFiltTime_250ns = 4,                              /** \brief  250 ns */
  CANFD_domFiltTime_300ns = 5,                              /** \brief  300 ns */
  CANFD_domFiltTime_350ns = 6,                              /** \brief  350 ns */
  CANFD_domFiltTime_775ns = 7                               /** \brief  775 ns */
} tCANFD_domFiltTime;

/** \enum tSWK_receiver
 *  \brief Enum for the TLE9563 SWK Receiver selection (only accessible if TRIM_EN = '11')
 */
typedef enum _tSWK_receiver
{
  SWK_receiver_lowpower = 0,                                /** \brief  Low-Power Receiver selected during SWK */
  SWK_receiver_std = 1                                      /** \brief  Standard Receiver selected during SWK */
} tSWK_receiver;

/** \enum tSWK_clockDataRecoveryInputFreq
 *  \brief Enum for the TLE9563 Select Time Constant of Filter
 */
typedef enum _tSWK_clockDataRecoveryInputFreq
{
  SWK_clockDataRecoveryInputFreq_8 = 0,                     /** \brief  Time constant 8 */
  SWK_clockDataRecoveryInputFreq_16 = 1,                    /** \brief  Time constant 16 (default) */
  SWK_clockDataRecoveryInputFreq_32 = 2,                    /** \brief  Time constant 32 */
  SWK_clockDataRecoveryInputFreq_adaptive = 3               /** \brief  adapt distance between falling edges 2, 3 bit Time constant 32; distance between falling edges 4, 5, 6, 7, 8 bit Time constant 16; distance between falling edges 9, 10 bit Time constant 8 */
} tSWK_clockDataRecoveryInputFreq;

/** \enum tSWK_filtTimeConst
 *  \brief Enum for the TLE9563 Select Time Constant of Filter
 */
typedef enum _tSWK_filtTimeConst
{
  SWK_filtTimeConst_8 = 0,                                  /** \brief  Time constant 8 */
  SWK_filtTimeConst_16 = 1,                                 /** \brief  Time constant 16 (default) */
  SWK_filtTimeConst_32 = 2,                                 /** \brief  Time constant 32 */
  SWK_filtTimeConst_adaptive = 3                            /** \brief  adapt distance between falling edges 2, 3 bit Time constant 32; distance between falling edges 4, 5, 6, 7, 8 bit Time constant 16; distance between falling edges 9, 10 bit Time constant 8 */
} tSWK_filtTimeConst;

/** \enum tBDRV_tBlank
 *  \brief Enum for the TLE9563 Blank time
 */
typedef enum _tBDRV_tBlank
{
  BDRV_tBlank_587ns = 0,                                     /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_853ns = 1,                                     /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_1119ns = 2,                                    /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_1385ns = 3,                                    /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_1651ns = 4,                                    /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_1917ns = 5,                                    /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_2183ns = 6,                                    /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_2449ns = 7,                                    /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% (default) */
  BDRV_tBlank_2715ns = 8,                                    /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_2981ns = 9,                                    /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_3247ns = 10,                                   /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_3513ns = 11,                                   /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_3779ns = 12,                                   /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_4045ns = 13,                                   /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_4311ns = 14,                                   /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
  BDRV_tBlank_4577ns = 15                                    /** \brief  nom. tHBxBLANK = 587 ns + 266 x T[3:0]D, +/- 20% */
} tBDRV_tBlank;

/** \enum tBDRV_tCcp
 *  \brief Enum for the TLE9563 Cross-current protection time
 */
typedef enum _tBDRV_tCcp
{
  BDRV_tCcp_587ns = 0,                                       /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_853ns = 1,                                       /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_1119ns = 2,                                      /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_1385ns = 3,                                      /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_1651ns = 4,                                      /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_1917ns = 5,                                      /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_2183ns = 6,                                      /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_2449ns = 7,                                      /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% (default) */
  BDRV_tCcp_2715ns = 8,                                      /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_2981ns = 9,                                      /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_3247ns = 10,                                     /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_3513ns = 11,                                     /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_3779ns = 12,                                     /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_4045ns = 13,                                     /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_4311ns = 14,                                     /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
  BDRV_tCcp_4577ns = 15                                      /** \brief  nom. tHBxCCP = 587 ns + 266 x TCCP[3:0]D, +/- 20% */
} tBDRV_tCcp;

/** \enum tBDRV_preChargeDischargeTime
 *  \brief Enum for the TLE9563 If TPCHG_BNK=0 precharge time of HB 3, If TPCHG_BNK=1 predischarge time of HB 3
 */
typedef enum _tBDRV_preChargeDischargeTime
{
  BDRV_preChargeDischargeTime_107ns = 0,                     /** \brief  tPCHG000 min =  80 ns, typ = 107 ns, max =  140 ns */
  BDRV_preChargeDischargeTime_160ns = 1,                     /** \brief  tPCHG001 min = 130 ns, typ = 160 ns, max =  190 ns */
  BDRV_preChargeDischargeTime_214ns = 2,                     /** \brief  tPCHG010 min = 170 ns, typ = 214 ns, max =  260 ns */
  BDRV_preChargeDischargeTime_267ns = 3,                     /** \brief  tPCHG011 min = 210 ns, typ = 267 ns, max =  330 ns */
  BDRV_preChargeDischargeTime_320ns = 4,                     /** \brief  tPCHG100 min = 250 ns, typ = 320 ns, max =  390 ns */
  BDRV_preChargeDischargeTime_533ns = 5,                     /** \brief  tPCHG101 min = 420 ns, typ = 533 ns, max =  630 ns */
  BDRV_preChargeDischargeTime_747ns = 6,                     /** \brief  tPCHG110 min = 600 ns, typ = 747 ns, max =  900 ns */
  BDRV_preChargeDischargeTime_1067ns = 7                     /** \brief  tPCHG111 min = 840 ns, typ = 1067 ns, max = 1260 ns */
} tBDRV_preChargeDischargeTime;

/** \enum tBDRV_staticChargeDischargeCur
 *  \brief Enum for the TLE9563 Static charge and discharge currents of HB3
 */
typedef enum _tBDRV_staticChargeDischargeCur
{
  BDRV_staticChargeDischargeCur_0mA5 = 0,                    /** \brief  0000B  0.5   mA (ICHG0) , 0.5   mA (IDCHG0) , +/- 60% */
  BDRV_staticChargeDischargeCur_1mA8 = 1,                    /** \brief  0001B  1.8   mA (ICHG4) , 1.8   mA (IDCHG4) , +/- 60% */
  BDRV_staticChargeDischargeCur_4mA7 = 2,                    /** \brief  0010B  4.7   mA (ICHG8) , 4.7   mA (IDCHG8) , +/- 60% */
  BDRV_staticChargeDischargeCur_9mA4 = 3,                    /** \brief  0011B  9.4   mA (ICHG12), 9.4   mA (IDCHG12), +/- 60% */
  BDRV_staticChargeDischargeCur_15mA3 = 4,                   /** \brief  0100B  15.3  mA (ICHG16), 15.1  mA (IDCHG16), +/- 40% */
  BDRV_staticChargeDischargeCur_23mA = 5,                    /** \brief  0101B  23    mA (ICHG20), 22.5  mA (IDCHG20), +/- 40% */
  BDRV_staticChargeDischargeCur_31mA6 = 6,                   /** \brief  0110B  31.6  mA (ICHG24), 30.9  mA (IDCHG24), +/- 40% */
  BDRV_staticChargeDischargeCur_41mA6 = 7,                   /** \brief  0111B  41.6  mA (ICHG28), 40.8  mA (IDCHG28), +/- 40% */
  BDRV_staticChargeDischargeCur_52mA5 = 8,                   /** \brief  1000B  52.5  mA (ICHG32), 51.5  mA (IDCHG32), +/- 30% */
  BDRV_staticChargeDischargeCur_63mA6 = 9,                   /** \brief  1001B  63.6  mA (ICHG36), 62.4  mA (IDCHG36), +/- 30% */
  BDRV_staticChargeDischargeCur_75mA2 = 10,                  /** \brief  1010B  75.2  mA (ICHG40), 73.7  mA (IDCHG40), +/- 30% */
  BDRV_staticChargeDischargeCur_87mA1 = 11,                  /** \brief  1011B  87.1  mA (ICHG44), 85.5  mA (IDCHG44), +/- 30% */
  BDRV_staticChargeDischargeCur_99mA5 = 12,                  /** \brief  1100B  99.5  mA (ICHG48), 97.7  mA (IDCHG48), +/- 30% */
  BDRV_staticChargeDischargeCur_112mA2 = 13,                 /** \brief  1101B  112.2 mA (ICHG52), 110.8 mA (IDCHG52), +/- 30% */
  BDRV_staticChargeDischargeCur_125mA3 = 14,                 /** \brief  1110B  125.3 mA (ICHG56), 124.5 mA (IDCHG56), +/- 30% */
  BDRV_staticChargeDischargeCur_139mA  = 15                  /** \brief  1111B  139   mA (ICHG60), 138.7 mA (IDCHG60), +/- 30% */
} tBDRV_staticChargeDischargeCur;

/** \enum tBDRV_dischargeCur
 *  \brief Enum for the TLE9563 If ICHG_BNK =0xxB Discharge current of HBx active MOSFET
 */
typedef enum _tBDRV_dischargeCur
{
  BDRV_dischargeCur_0mA5 = 0,                                /** \brief  000000B  IDCHG0  0.5   mA, +/- 60% */
  BDRV_dischargeCur_0mA7 = 1,                                /** \brief  000001B  IDCHG1  0.7   mA, +/- 60% */
  BDRV_dischargeCur_1mA = 2,                                 /** \brief  000010B  IDCHG2  1.0   mA, +/- 60% */
  BDRV_dischargeCur_1mA4 = 3,                                /** \brief  000011B  IDCHG3  1.4   mA, +/- 60% */
  BDRV_dischargeCur_1mA8 = 4,                                /** \brief  000100B  IDCHG4  1.8   mA, +/- 60% */
  BDRV_dischargeCur_2mA4 = 5,                                /** \brief  000101B  IDCHG5  2.4   mA, +/- 60% */
  BDRV_dischargeCur_3mA = 6,                                 /** \brief  000110B  IDCHG6  3.0   mA, +/- 60% */
  BDRV_dischargeCur_3mA8 = 7,                                /** \brief  000111B  IDCHG7  3.8   mA, +/- 60% */
  BDRV_dischargeCur_4mA7 = 8,                                /** \brief  001000B  IDCHG8  4.7   mA, +/- 60% */
  BDRV_dischargeCur_5mA8 = 9,                                /** \brief  001001B  IDCHG9  5.8   mA, +/- 60% */
  BDRV_dischargeCur_6mA9 = 10,                               /** \brief  001010B  IDCHG10 6.9   mA, +/- 60% */
  BDRV_dischargeCur_8mA1 = 11,                               /** \brief  001011B  IDCHG11 8.1   mA, +/- 60% */
  BDRV_dischargeCur_9mA4 = 12,                               /** \brief  001100B  IDCHG12 9.4   mA, +/- 60% */
  BDRV_dischargeCur_10mA7 = 13,                              /** \brief  001101B  IDCHG13 10.7  mA, +/- 60% */
  BDRV_dischargeCur_12mA1 = 14,                              /** \brief  001110B  IDCHG14 12.1  mA, +/- 40% */
  BDRV_dischargeCur_13mA5 = 15,                              /** \brief  001111B  IDCHG15 13.5  mA, +/- 40% */
  BDRV_dischargeCur_15mA1 = 16,                              /** \brief  010000B  IDCHG16 15.1  mA, +/- 40% */
  BDRV_dischargeCur_16mA8 = 17,                              /** \brief  010001B  IDCHG17 16.8  mA, +/- 40% */
  BDRV_dischargeCur_18mA6 = 18,                              /** \brief  010010B  IDCHG18 18.6  mA, +/- 40% */
  BDRV_dischargeCur_20mA5 = 19,                              /** \brief  010011B  IDCHG19 20.5  mA, +/- 40% */
  BDRV_dischargeCur_22mA5 = 20,                              /** \brief  010100B  IDCHG20 22.5  mA, +/- 40% */
  BDRV_dischargeCur_24mA5 = 21,                              /** \brief  010101B  IDCHG21 24.5  mA, +/- 40% */
  BDRV_dischargeCur_26mA5 = 22,                              /** \brief  010110B  IDCHG22 26.5  mA, +/- 40% */
  BDRV_dischargeCur_28mA7 = 23,                              /** \brief  010111B  IDCHG23 28.7  mA, +/- 40% */
  BDRV_dischargeCur_30mA9 = 24,                              /** \brief  011000B  IDCHG24 30.9  mA, +/- 40% */
  BDRV_dischargeCur_33mA2 = 25,                              /** \brief  011001B  IDCHG25 33.2  mA, +/- 40% */
  BDRV_dischargeCur_35mA7 = 26,                              /** \brief  011010B  IDCHG26 35.7  mA, +/- 40% */
  BDRV_dischargeCur_38mA2 = 27,                              /** \brief  011011B  IDCHG27 38.2  mA, +/- 40% */
  BDRV_dischargeCur_40mA8 = 28,                              /** \brief  011100B  IDCHG28 40.8  mA, +/- 40% */
  BDRV_dischargeCur_43mA4 = 29,                              /** \brief  011101B  IDCHG29 43.4  mA, +/- 30% */
  BDRV_dischargeCur_46mA1 = 30,                              /** \brief  011110B  IDCHG30 46.1  mA, +/- 30% */
  BDRV_dischargeCur_48mA8 = 31,                              /** \brief  011111B  IDCHG31 48.8  mA, +/- 30% */
  BDRV_dischargeCur_51mA5 = 32,                              /** \brief  100000B  IDCHG32 51.5  mA, +/- 30% */
  BDRV_dischargeCur_54mA2 = 33,                              /** \brief  100001B  IDCHG33 54.2  mA, +/- 30% */
  BDRV_dischargeCur_56mA9 = 34,                              /** \brief  100010B  IDCHG34 56.9  mA, +/- 30% */
  BDRV_dischargeCur_59mA6 = 35,                              /** \brief  100011B  IDCHG35 59.6  mA, +/- 30% */
  BDRV_dischargeCur_62mA4 = 36,                              /** \brief  100100B  IDCHG36 62.4  mA, +/- 30% */
  BDRV_dischargeCur_65mA2 = 37,                              /** \brief  100101B  IDCHG37 65.2  mA, +/- 30% */
  BDRV_dischargeCur_68mA = 38,                               /** \brief  100110B  IDCHG38 68    mA, +/- 30% */
  BDRV_dischargeCur_70mA8 = 39,                              /** \brief  100111B  IDCHG39 70.8  mA, +/- 30% */
  BDRV_dischargeCur_73mA7 = 40,                              /** \brief  101000B  IDCHG40 73.7  mA, +/- 30% */
  BDRV_dischargeCur_76mA6 = 41,                              /** \brief  101001B  IDCHG41 76.6  mA, +/- 30% */
  BDRV_dischargeCur_79mA5 = 42,                              /** \brief  101010B  IDCHG42 79.5  mA, +/- 30% */
  BDRV_dischargeCur_82mA5 = 43,                              /** \brief  101011B  IDCHG43 82.5  mA, +/- 30% */
  BDRV_dischargeCur_85mA5 = 44,                              /** \brief  101100B  IDCHG44 85.5  mA, +/- 30% */
  BDRV_dischargeCur_88mA5 = 45,                              /** \brief  101101B  IDCHG45 88.5  mA, +/- 30% */
  BDRV_dischargeCur_91mA5 = 46,                              /** \brief  101110B  IDCHG46 91.5  mA, +/- 30% */
  BDRV_dischargeCur_94mA6 = 47,                              /** \brief  101111B  IDCHG47 94.6  mA, +/- 30% */
  BDRV_dischargeCur_97mA7 = 48,                              /** \brief  110000B  IDCHG48 97.7  mA, +/- 30% */
  BDRV_dischargeCur_100mA9 = 49,                             /** \brief  110001B  IDCHG49 100.9 mA, +/- 30% */
  BDRV_dischargeCur_104mA2 = 50,                             /** \brief  110010B  IDCHG50 104.2 mA, +/- 30% */
  BDRV_dischargeCur_107mA5 = 51,                             /** \brief  110011B  IDCHG51 107.5 mA, +/- 30% */
  BDRV_dischargeCur_110mA8 = 52,                             /** \brief  110100B  IDCHG52 110.8 mA, +/- 30% */
  BDRV_dischargeCur_114mA2 = 53,                             /** \brief  110101B  IDCHG53 114.2 mA, +/- 30% */
  BDRV_dischargeCur_117mA6 = 54,                             /** \brief  110110B  IDCHG54 117.6 mA, +/- 30% */
  BDRV_dischargeCur_121mA = 55,                              /** \brief  110111B  IDCHG55 121   mA, +/- 30% */
  BDRV_dischargeCur_124mA5 = 56,                             /** \brief  111000B  IDCHG56 124.5 mA, +/- 30% */
  BDRV_dischargeCur_128mA = 57,                              /** \brief  111001B  IDCHG57 128   mA, +/- 30% */
  BDRV_dischargeCur_131mA5 = 58,                             /** \brief  111010B  IDCHG58 131.5 mA, +/- 30% */
  BDRV_dischargeCur_135mA1 = 59,                             /** \brief  111011B  IDCHG59 135.1 mA, +/- 30% */
  BDRV_dischargeCur_138mA7 = 60,                             /** \brief  111100B  IDCHG60 138.7 mA, +/- 30% */
  BDRV_dischargeCur_142mA3 = 61,                             /** \brief  111101B  IDCHG61 142.3 mA, +/- 30% */
  BDRV_dischargeCur_145mA8 = 62,                             /** \brief  111110B  IDCHG62 145.8 mA, +/- 30% */
  BDRV_dischargeCur_150mA = 63                               /** \brief  111111B  IDCHG63 150   mA, +/- 30% */
} tBDRV_dischargeCur;

/** \enum tBDRV_chargeCur
 *  \brief Enum for the TLE9563 If ICHG_BNK=0xxB Charge current of HBx active MOSFET
 */
typedef enum _tBDRV_chargeCur
{
  BDRV_chargeCur_0mA5 = 0,                                   /** \brief  000000B  ICHG0  0.5   mA, +/- 60% */
  BDRV_chargeCur_0mA7 = 1,                                   /** \brief  000001B  ICHG1  0.7   mA, +/- 60% */
  BDRV_chargeCur_1mA = 2,                                    /** \brief  000010B  ICHG2  1.0   mA, +/- 60% */
  BDRV_chargeCur_1mA4 = 3,                                   /** \brief  000011B  ICHG3  1.4   mA, +/- 60% */
  BDRV_chargeCur_1mA8 = 4,                                   /** \brief  000100B  ICHG4  1.8   mA, +/- 60% */
  BDRV_chargeCur_2mA4 = 5,                                   /** \brief  000101B  ICHG5  2.4   mA, +/- 60% */
  BDRV_chargeCur_3mA = 6,                                    /** \brief  000110B  ICHG6  3.0   mA, +/- 60% */
  BDRV_chargeCur_3mA8 = 7,                                   /** \brief  000111B  ICHG7  3.8   mA, +/- 60% */
  BDRV_chargeCur_4mA7 = 8,                                   /** \brief  001000B  ICHG8  4.7   mA, +/- 55% */
  BDRV_chargeCur_5mA8 = 9,                                   /** \brief  001001B  ICHG9  5.8   mA, +/- 55% */
  BDRV_chargeCur_6mA9 = 10,                                  /** \brief  001010B  ICHG10 6.9   mA, +/- 55% */
  BDRV_chargeCur_8mA1 = 11,                                  /** \brief  001011B  ICHG11 8.1   mA, +/- 55% */
  BDRV_chargeCur_9mA4 = 12,                                  /** \brief  001100B  ICHG12 9.4   mA, +/- 55% */
  BDRV_chargeCur_10mA8 = 13,                                 /** \brief  001101B  ICHG13 10.8  mA, +/- 55% */
  BDRV_chargeCur_12mA2 = 14,                                 /** \brief  001110B  ICHG14 12.2  mA, +/- 40% */
  BDRV_chargeCur_13mA7 = 15,                                 /** \brief  001111B  ICHG15 13.7  mA, +/- 40% */
  BDRV_chargeCur_15mA3 = 16,                                 /** \brief  010000B  ICHG16 15.3  mA, +/- 40% */
  BDRV_chargeCur_17mA1 = 17,                                 /** \brief  010001B  ICHG17 17.1  mA, +/- 40% */
  BDRV_chargeCur_19mA = 18,                                  /** \brief  010010B  ICHG18 19    mA, +/- 40% */
  BDRV_chargeCur_21mA = 19,                                  /** \brief  010011B  ICHG19 21    mA, +/- 40% */
  BDRV_chargeCur_23mA = 20,                                  /** \brief  010100B  ICHG20 23    mA, +/- 40% */
  BDRV_chargeCur_25mA = 21,                                  /** \brief  010101B  ICHG21 25    mA, +/- 40% */
  BDRV_chargeCur_27mA1 = 22,                                 /** \brief  010110B  ICHG22 27.1  mA, +/- 40% */
  BDRV_chargeCur_29mA3 = 23,                                 /** \brief  010111B  ICHG23 29.3  mA, +/- 40% */
  BDRV_chargeCur_31mA6 = 24,                                 /** \brief  011000B  ICHG24 31.6  mA, +/- 40% */
  BDRV_chargeCur_34mA = 25,                                  /** \brief  011001B  ICHG25 34    mA, +/- 40% */
  BDRV_chargeCur_36mA5 = 26,                                 /** \brief  011010B  ICHG26 36.5  mA, +/- 40% */
  BDRV_chargeCur_39mA = 27,                                  /** \brief  011011B  ICHG27 39    mA, +/- 40% */
  BDRV_chargeCur_41mA6 = 28,                                 /** \brief  011100B  ICHG28 41.6  mA, +/- 40% */
  BDRV_chargeCur_44mA2 = 29,                                 /** \brief  011101B  ICHG29 44.2  mA, +/- 30% */
  BDRV_chargeCur_46mA9 = 30,                                 /** \brief  011110B  ICHG30 46.9  mA, +/- 30% */
  BDRV_chargeCur_49mA7 = 31,                                 /** \brief  011111B  ICHG31 49.7  mA, +/- 30% */
  BDRV_chargeCur_52mA5 = 32,                                 /** \brief  100000B  ICHG32 52.5  mA, +/- 30% */
  BDRV_chargeCur_55mA3 = 33,                                 /** \brief  100001B  ICHG33 55.3  mA, +/- 30% */
  BDRV_chargeCur_58mA1 = 34,                                 /** \brief  100010B  ICHG34 58.1  mA, +/- 30% */
  BDRV_chargeCur_60mA8 = 35,                                 /** \brief  100011B  ICHG35 60.8  mA, +/- 30% */
  BDRV_chargeCur_63mA6 = 36,                                 /** \brief  100100B  ICHG36 63.6  mA, +/- 30% */
  BDRV_chargeCur_66mA5 = 37,                                 /** \brief  100101B  ICHG37 66.5  mA, +/- 30% */
  BDRV_chargeCur_69mA4 = 38,                                 /** \brief  100110B  ICHG38 69.4  mA, +/- 30% */
  BDRV_chargeCur_72mA3 = 39,                                 /** \brief  100111B  ICHG39 72.3  mA, +/- 30% */
  BDRV_chargeCur_75mA2 = 40,                                 /** \brief  101000B  ICHG40 75.2  mA, +/- 30% */
  BDRV_chargeCur_78mA1 = 41,                                 /** \brief  101001B  ICHG41 78.1  mA, +/- 30% */
  BDRV_chargeCur_81mA1 = 42,                                 /** \brief  101010B  ICHG42 81.1  mA, +/- 30% */
  BDRV_chargeCur_84mA1 = 43,                                 /** \brief  101011B  ICHG43 84.1  mA, +/- 30% */
  BDRV_chargeCur_87mA1 = 44,                                 /** \brief  101100B  ICHG44 87.1  mA, +/- 30% */
  BDRV_chargeCur_90mA2 = 45,                                 /** \brief  101101B  ICHG45 90.2  mA, +/- 30% */
  BDRV_chargeCur_93mA3 = 46,                                 /** \brief  101110B  ICHG46 93.3  mA, +/- 30% */
  BDRV_chargeCur_96mA4 = 47,                                 /** \brief  101111B  ICHG47 96.4  mA, +/- 30% */
  BDRV_chargeCur_99mA5 = 48,                                 /** \brief  110000B  ICHG48 99.5  mA, +/- 30% */
  BDRV_chargeCur_102mA7 = 49,                                /** \brief  110001B  ICHG49 102.7 mA, +/- 30% */
  BDRV_chargeCur_105mA8 = 50,                                /** \brief  110010B  ICHG50 105.8 mA, +/- 30% */
  BDRV_chargeCur_109mA = 51,                                 /** \brief  110011B  ICHG51 109   mA, +/- 30% */
  BDRV_chargeCur_112mA2 = 52,                                /** \brief  110100B  ICHG52 112.2 mA, +/- 30% */
  BDRV_chargeCur_115mA4 = 53,                                /** \brief  110101B  ICHG53 115.4 mA, +/- 30% */
  BDRV_chargeCur_118mA7 = 54,                                /** \brief  110110B  ICHG54 118.7 mA, +/- 30% */
  BDRV_chargeCur_122mA = 55,                                 /** \brief  110111B  ICHG55 122   mA, +/- 30% */
  BDRV_chargeCur_125mA3 = 56,                                /** \brief  111000B  ICHG56 125.3 mA, +/- 30% */
  BDRV_chargeCur_128mA7 = 57,                                /** \brief  111001B  ICHG57 128.7 mA, +/- 30% */
  BDRV_chargeCur_132mA1 = 58,                                /** \brief  111010B  ICHG58 132.1 mA, +/- 30% */
  BDRV_chargeCur_135mA5 = 59,                                /** \brief  111011B  ICHG59 135.5 mA, +/- 30% */
  BDRV_chargeCur_139mA = 60,                                 /** \brief  111100B  ICHG60 139   mA, +/- 30% */
  BDRV_chargeCur_142mA5 = 61,                                /** \brief  111101B  ICHG61 142.5 mA, +/- 30% */
  BDRV_chargeCur_146mA = 62,                                 /** \brief  111110B  ICHG62 146   mA, +/- 30% */
  BDRV_chargeCur_150mA = 63                                  /** \brief  111111B  ICHG63 150   mA, +/- 30% */
} tBDRV_chargeCur;

/** \enum tBDRV_tdon
 *  \brief Enum for the TLE9563 Turn-on delay time of active MOSFET of HBx
 */
typedef enum _tBDRV_tdon
{
  BDRV_tdon_0ns = 0,                                         /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_53ns = 1,                                        /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_107ns = 2,                                       /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_160ns = 3,                                       /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_213ns = 4,                                       /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_267ns = 5,                                       /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_320ns = 6,                                       /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_373ns = 7,                                       /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_426ns = 8,                                       /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_480ns = 9,                                       /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_533ns = 10,                                      /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_586ns = 11,                                      /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_640ns = 12,                                      /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D (default) */
  BDRV_tdon_693ns = 13,                                      /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_746ns = 14,                                      /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_800ns = 15,                                      /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_853ns = 16,                                      /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_906ns = 17,                                      /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_959ns = 18,                                      /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1013ns = 19,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1066ns = 20,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1119ns = 21,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1173ns = 22,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1226ns = 23,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1279ns = 24,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1333ns = 25,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1386ns = 26,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1439ns = 27,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1492ns = 28,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1546ns = 29,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1599ns = 30,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1652ns = 31,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1706ns = 32,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1759ns = 33,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1812ns = 34,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1866ns = 35,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1919ns = 36,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_1972ns = 37,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2025ns = 38,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2079ns = 39,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2132ns = 40,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2185ns = 41,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2239ns = 42,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2292ns = 43,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2345ns = 44,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2399ns = 45,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2452ns = 46,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2505ns = 47,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2558ns = 48,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2612ns = 49,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2665ns = 50,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2718ns = 51,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2772ns = 52,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2825ns = 53,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2878ns = 54,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2932ns = 55,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_2985ns = 56,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_3038ns = 57,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_3091ns = 58,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_3145ns = 59,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_3198ns = 60,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_3251ns = 61,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_3305ns = 62,                                     /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
  BDRV_tdon_3358ns = 63                                      /** \brief  Nominal tDON = 53.3 ns x TDON[5:0]D */
} tBDRV_tdon;

/** \enum tBDRV_tdoff
 *  \brief Enum for the TLE9563 Turn-off delay time of active MOSFET of HBx The HB_TDOFF_BNK bits selects the turn-off delay time of the active MOSFET of the half-bridge HBx Nominal tDOFF = 53.3 ns x TDOFF[50]D
 */
typedef enum _tBDRV_tdoff
{
  BDRV_tdoff_0ns = 0,                                        /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_53ns = 1,                                       /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_107ns = 2,                                      /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_160ns = 3,                                      /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_213ns = 4,                                      /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_267ns = 5,                                      /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_320ns = 6,                                      /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_373ns = 7,                                      /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_426ns = 8,                                      /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_480ns = 9,                                      /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_533ns = 10,                                     /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_586ns = 11,                                     /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_640ns = 12,                                     /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D (default) */
  BDRV_tdoff_693ns = 13,                                     /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_746ns = 14,                                     /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_800ns = 15,                                     /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_853ns = 16,                                     /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_906ns = 17,                                     /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_959ns = 18,                                     /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1013ns = 19,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1066ns = 20,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1119ns = 21,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1173ns = 22,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1226ns = 23,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1279ns = 24,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1333ns = 25,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1386ns = 26,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1439ns = 27,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1492ns = 28,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1546ns = 29,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1599ns = 30,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1652ns = 31,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1706ns = 32,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1759ns = 33,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1812ns = 34,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1866ns = 35,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1919ns = 36,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_1972ns = 37,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2025ns = 38,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2079ns = 39,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2132ns = 40,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2185ns = 41,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2239ns = 42,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2292ns = 43,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2345ns = 44,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2399ns = 45,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2452ns = 46,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2505ns = 47,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2558ns = 48,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2612ns = 49,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2665ns = 50,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2718ns = 51,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2772ns = 52,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2825ns = 53,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2878ns = 54,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2932ns = 55,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_2985ns = 56,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_3038ns = 57,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_3091ns = 58,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_3145ns = 59,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_3198ns = 60,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_3251ns = 61,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_3305ns = 62,                                    /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
  BDRV_tdoff_3358ns = 63                                     /** \brief  Nominal tDOFF = 53.3 ns x TDOFF[5:0]D */
} tBDRV_tdoff;

/** \enum tBDRV_setAllHbx
 *  \brief Enum for the setting of HBMODE control register. To be used in \ref TLE9563_setAllHbx
 */
typedef enum _tBDRV_setAllHbx
{
  BDRV_setAllHbx_passiveOff_afwEn_pwmInact = 0x0222,         /** \brief  Half-bridge mode = PASSIVE_OFF (default), Active freewheeling = ENABLED (default), PWM mode = INACTIVE (default) */
  BDRV_setAllHbx_activeOff_afwEn_pwmInact = 0x0EEE,          /** \brief  Half-bridge mode = ACTIVE_OFF, Active freewheeling = ENABLED (default), PWM mode = INACTIVE (default) */
  BDRV_setAllHbx_lsOn_afwEn_PwmInact = 0x0666,               /** \brief  Half-bridge mode = LSx_ON, Active freewheeling = ENABLED (default), PWM mode = INACTIVE (default) */
  BDRV_setAllHbx_hsOn_afwEn_PwmInact = 0x0AAA,               /** \brief  Half-bridge mode = HSx_ON, Active freewheeling = ENABLED (default), PWM mode = INACTIVE (default) */
  BDRV_setAllHbx_lsOn_afwEn_pwmAct= 0x0777,                  /** \brief  Half-bridge mode = LSx_ON, Active freewheeling = ENABLED (default), PWM mode = ACTIVE */
  BDRV_setAllHbx_hsOn_afwEn_pwmAct= 0x0BBB,                  /** \brief  Half-bridge mode = HSx_ON, Active freewheeling = ENABLED (default), PWM mode = ACTIVE */
  BDRV_setAllHbx_lsOn_afwDis_pwmAct = 0x0555,                /** \brief  Half-bridge mode = LSx_ON, Active freewheeling = DISABLED, PWM mode = ACTIVE */
  BDRV_setAllHbx_hsOn_afwDis_pwmAct = 0x0999,                /** \brief  Half-bridge mode = HSx_ON, Active freewheeling = DISABLED, PWM mode = ACTIVE */ 
} tBDRV_setAllHbx;

/** \brief function pointer fpDEVICE_setDeviceReg
*/
typedef void (*fpDEVICE_setDeviceReg)(uint16_t);

/** \brief function pointer fpDEVICE_getDeviceReg
*/
typedef void (*fpDEVICE_getDeviceReg)(void);

/** \brief function pointer fpDEVICE_updateRamReg
*/
typedef void (*fpDEVICE_updateRamReg)(uint16_t);

/** \union unDEVICE_spiBuffer
 *  \brief Union for content of last received SPI message
 */
typedef  union _unDEVICE_spiBuffer
{
  uint8_t     au8Bytes[4];
  uint32_t    u32Word;
} unDEVICE_spiBuffer;

/** \union unDEVICE_crcResult
 *  \brief Union for expected result for crc of last received SPI message
 */
typedef  union _unDEVICE_crcResult
{
  uint8_t     au8Bytes[4];
  uint32_t    u32Word;
} unDEVICE_crcResult;

/** \struct sDeviceDriver
 *  \brief Struct for Device Driver cyclic task
 */
typedef struct _sDEVICE_deviceDriver
{
  uint8_t u8_deviceDriverStatus;                              /** \brief  device driver status */
  uint8_t u8_deviceDriverErrorLog;                            /** \brief  device driver error log */
  unDEVICE_spiBuffer un_SpiRx;                                /** \brief  content of last received SPI message */
  unDEVICE_crcResult un_CrcResult;                            /** \brief  expected result for crc of last received SPI message */
  fpDEVICE_setDeviceReg fp_setReg;                            /** \brief  function pointer to next set register function */
  fpDEVICE_getDeviceReg fp_getReg;                            /** \brief  function pointer to next get register function */
  fpDEVICE_updateRamReg fp_updateRamReg;                      /** \brief  function pointer to next update RAM register function */
  uint16_t u16_setBitValue;                                   /** \brief  value to be used in next set register function */
} sDEVICE_deviceDriver;

/*******************************************************************************
**                        Global Variable Declarations                        **
*******************************************************************************/
extern sDEVICE_deviceDriver s_deviceDriver;

/*******************************************************************************
**                        Global Function Declarations                        **
*******************************************************************************/
uint8_t TLE9563_deviceDriverCyclicTask(void);
uint8_t TLE9563_Init(void);
uint8_t TLE9563_serveWatchdog(void);
uint8_t TLE9563_getSif(void);
uint8_t TLE9563_getSifBitSupplyStatus(void);
uint8_t TLE9563_getSifBitTempStatus(void);
uint8_t TLE9563_getSifBitBusStatus(void);
uint8_t TLE9563_getSifBitWakeUp(void);
uint8_t TLE9563_getSifBitHsStatus(void);
uint8_t TLE9563_getSifBitDeviceStatus(void);
uint8_t TLE9563_getSifBitBdrvStatus(void);
uint8_t TLE9563_getSifBitSpiCrcFail(void);
uint8_t TLE9563_recoverCrc(void);
void TLE9563_resetLastSetBitfieldRequest(void);
uint8_t TLE9563_setAllHbx(tBDRV_setAllHbx e_value);

uint16_t TLE9563_getModeSupplyCtrlReg(void);
uint8_t TLE9563_setModeSupplyCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setDeviceMode(tDEVICE_mode e_value);
uint16_t TLE9563_getDeviceMode(void);
uint8_t TLE9563_setVcc1OverVoltReact(tVCC1_overVoltReact e_value);
uint16_t TLE9563_getVcc1OverVoltReact(void);
uint8_t TLE9563_setVcc1UnderVoltResetHys(tVCC1_underVoltResetHys e_value);
uint16_t TLE9563_getVcc1UnderVoltResetHys(void);
uint8_t TLE9563_setVcc1ActPeakThresh(tVCC1_actPeakThresh e_value);
uint16_t TLE9563_getVcc1ActPeakThresh(void);
uint8_t TLE9563_setVcc1ResetThresh(tVCC1_resetThresh e_value);
uint16_t TLE9563_getVcc1ResetThresh(void);
uint16_t TLE9563_getHwCtrlReg(void);
uint8_t TLE9563_setHwCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setTempShutDown2MinWaitTime(tTSD2_minWaitTime e_value);
uint16_t TLE9563_getTempShutDown2MinWaitTime(void);
uint8_t TLE9563_setVsOverVoltThresh(tVS_overVoltThresh e_value);
uint16_t TLE9563_getVsOverVoltThresh(void);
uint8_t TLE9563_enBdrvSampleAndHoldCiruit(void);
uint8_t TLE9563_disBdrvSampleAndHoldCiruit(void);
uint8_t TLE9563_getBdrvSampleAndHoldCiruit(void);
uint8_t TLE9563_setResetDelayTime(tRST_delayTime e_value);
uint16_t TLE9563_getResetDelayTime(void);
uint8_t TLE9563_setSoftResetConfig(tSOFTRST_config e_value);
uint16_t TLE9563_getSoftResetConfig(void);
uint8_t TLE9563_enWdDeactDuringStopModeBit1(void);
uint8_t TLE9563_disWdDeactDuringStopModeBit1(void);
uint8_t TLE9563_getWdDeactDuringStopModeBit1(void);
uint16_t TLE9563_getWdCtrlReg(void);
uint8_t TLE9563_setWdCtrlReg(uint16_t u16_value);
uint16_t TLE9563_getWdChecksumBit(void);
uint8_t TLE9563_enWdDeactDuringStopModeBit0(void);
uint8_t TLE9563_disWdDeactDuringStopModeBit0(void);
uint8_t TLE9563_getWdDeactDuringStopModeBit0(void);
uint8_t TLE9563_setWdConfig(tWD_config e_value);
uint16_t TLE9563_getWdConfig(void);
uint8_t TLE9563_enWdAfterBusWkInStopMode(void);
uint8_t TLE9563_disWdAfterBusWkInStopMode(void);
uint8_t TLE9563_getWdAfterBusWkInStopMode(void);
uint8_t TLE9563_setWdTimerPeriod(tWD_timerPeriod e_value);
uint16_t TLE9563_getWdTimerPeriod(void);
uint16_t TLE9563_getBusCtrlReg(void);
uint8_t TLE9563_setBusCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setCanMode(tCAN_mode e_value);
uint16_t TLE9563_getCanMode(void);
uint16_t TLE9563_getWk4CtrlReg(void);
uint8_t TLE9563_setWk4CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setWk4FiltTime(tWK_filtTime e_value);
uint16_t TLE9563_getWk4FiltTime(void);
uint8_t TLE9563_setWk4PullUpPullDown(tWK_pullUpPullDown e_value);
uint16_t TLE9563_getWk4PullUpPullDown(void);
uint8_t TLE9563_enWk4(void);
uint8_t TLE9563_disWk4(void);
uint8_t TLE9563_getWk4(void);
uint16_t TLE9563_getWk5CtrlReg(void);
uint8_t TLE9563_setWk5CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setWk5FiltTime(tWK_filtTime e_value);
uint16_t TLE9563_getWk5FiltTime(void);
uint8_t TLE9563_setWk5PullUpPullDown(tWK_pullUpPullDown e_value);
uint16_t TLE9563_getWk5PullUpPullDown(void);
uint8_t TLE9563_enWk5(void);
uint8_t TLE9563_disWk5(void);
uint8_t TLE9563_getWk5(void);
uint16_t TLE9563_getTimerCtrlReg(void);
uint8_t TLE9563_setTimerCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setTimer2OnTime(tTIMER_onTime e_value);
uint16_t TLE9563_getTimer2OnTime(void);
uint8_t TLE9563_setTimer2Period(tTIMER_period e_value);
uint16_t TLE9563_getTimer2Period(void);
uint8_t TLE9563_setTimerCyclicWk(tTIMER_cyclicWk e_value);
uint16_t TLE9563_getTimerCyclicWk(void);
uint8_t TLE9563_setTimer1OnTime(tTIMER_onTime e_value);
uint16_t TLE9563_getTimer1OnTime(void);
uint8_t TLE9563_setTimer1Period(tTIMER_period e_value);
uint16_t TLE9563_getTimer1Period(void);
uint16_t TLE9563_getHsSwitchShutdownCtrlReg(void);
uint8_t TLE9563_setHsSwitchShutdownCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setHs3VsOverVoltRecovery(tHS_vsOverVoltRecovery e_value);
uint16_t TLE9563_getHs3VsOverVoltRecovery(void);
uint8_t TLE9563_setHs2VsOverVoltRecovery(tHS_vsOverVoltRecovery e_value);
uint16_t TLE9563_getHs2VsOverVoltRecovery(void);
uint8_t TLE9563_setHs1VsOverVoltRecovery(tHS_vsOverVoltRecovery e_value);
uint16_t TLE9563_getHs1VsOverVoltRecovery(void);
uint8_t TLE9563_enHsxOverTempShutDownAll(void);
uint8_t TLE9563_disHsxOverTempShutDownAll(void);
uint8_t TLE9563_getHsxOverTempShutDownAll(void);
uint8_t TLE9563_enHs3VsOverVoltShutDownNormalMode(void);
uint8_t TLE9563_disHs3VsOverVoltShutDownNormalMode(void);
uint8_t TLE9563_getHs3VsOverVoltShutDownNormalMode(void);
uint8_t TLE9563_enHs2VsOverVoltShutDownNormalMode(void);
uint8_t TLE9563_disHs2VsOverVoltShutDownNormalMode(void);
uint8_t TLE9563_getHs2VsOverVoltShutDownNormalMode(void);
uint8_t TLE9563_enHs1VsOverVoltShutDownNormalMode(void);
uint8_t TLE9563_disHs1VsOverVoltShutDownNormalMode(void);
uint8_t TLE9563_getHs1VsOverVoltShutDownNormalMode(void);
uint8_t TLE9563_enHsxVsOverVoltShutDownStopModeSleepMode(void);
uint8_t TLE9563_disHsxVsOverVoltShutDownStopModeSleepMode(void);
uint8_t TLE9563_getHsxVsOverVoltShutDownStopModeSleepMode(void);
uint8_t TLE9563_enHsxVsUnderVoltShutDown(void);
uint8_t TLE9563_disHsxVsUnderVoltShutDown(void);
uint8_t TLE9563_getHsxVsUnderVoltShutDown(void);
uint8_t TLE9563_setHsxVsUnderVoltRecovery(tHS_vsUnderVoltRecovery e_value);
uint16_t TLE9563_getHsxVsUnderVoltRecovery(void);
uint16_t TLE9563_getHsCtrlReg(void);
uint8_t TLE9563_setHsCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setHs3Config(tHS_config e_value);
uint16_t TLE9563_getHs3Config(void);
uint8_t TLE9563_setHs2Config(tHS_config e_value);
uint16_t TLE9563_getHs2Config(void);
uint8_t TLE9563_setHs1Config(tHS_config e_value);
uint16_t TLE9563_getHs1Config(void);
uint16_t TLE9563_getIntrMaskCtrlReg(void);
uint8_t TLE9563_setIntrMaskCtrlReg(uint16_t u16_value);
uint8_t TLE9563_enIntnPeriodical(void);
uint8_t TLE9563_disIntnPeriodical(void);
uint8_t TLE9563_getIntnPeriodical(void);
uint8_t TLE9563_enWdInSoftDevMode(void);
uint8_t TLE9563_disWdInSoftDevMode(void);
uint8_t TLE9563_getWdInSoftDevMode(void);
uint8_t TLE9563_enIntnWdFail(void);
uint8_t TLE9563_disIntnWdFail(void);
uint8_t TLE9563_getIntnWdFail(void);
uint8_t TLE9563_enIntnSpiFailOrCrcFail(void);
uint8_t TLE9563_disIntnSpiFailOrCrcFail(void);
uint8_t TLE9563_getIntnSpiFailOrCrcFail(void);
uint8_t TLE9563_enIntnBdrvStat(void);
uint8_t TLE9563_disIntnBdrvStat(void);
uint8_t TLE9563_getIntnBdrvStat(void);
uint8_t TLE9563_enIntnHsStat(void);
uint8_t TLE9563_disIntnHsStat(void);
uint8_t TLE9563_getIntnHsStat(void);
uint8_t TLE9563_enIntnBusStat(void);
uint8_t TLE9563_disIntnBusStat(void);
uint8_t TLE9563_getIntnBusStat(void);
uint8_t TLE9563_enIntnTempStat(void);
uint8_t TLE9563_disIntnTempStat(void);
uint8_t TLE9563_getIntnTempStat(void);
uint8_t TLE9563_enIntnSupplyStat(void);
uint8_t TLE9563_disIntnSupplyStat(void);
uint8_t TLE9563_getIntnSupplyStat(void);
uint16_t TLE9563_getPwm1CtrlReg(void);
uint8_t TLE9563_setPwm1CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setPwm1Freq(tPWM_freq e_value);
uint16_t TLE9563_getPwm1Freq(void);
uint8_t TLE9563_setPwm1DutyCycle(uint16_t u16_value);
uint16_t TLE9563_getPwm1DutyCycle(void);
uint16_t TLE9563_getPwm2CtrlReg(void);
uint8_t TLE9563_setPwm2CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setPwm2Freq(tPWM_freq e_value);
uint16_t TLE9563_getPwm2Freq(void);
uint8_t TLE9563_setPwm2DutyCycle(uint16_t u16_value);
uint16_t TLE9563_getPwm2DutyCycle(void);
uint16_t TLE9563_getPwm3CtrlReg(void);
uint8_t TLE9563_setPwm3CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setPwm3Freq(tPWM_freq e_value);
uint16_t TLE9563_getPwm3Freq(void);
uint8_t TLE9563_setPwm3DutyCycle(uint16_t u16_value);
uint16_t TLE9563_getPwm3DutyCycle(void);
uint16_t TLE9563_getPwm4CtrlReg(void);
uint8_t TLE9563_setPwm4CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setPwm4Freq(tPWM_freq e_value);
uint16_t TLE9563_getPwm4Freq(void);
uint8_t TLE9563_setPwm4DutyCycle(uint16_t u16_value);
uint16_t TLE9563_getPwm4DutyCycle(void);
uint8_t TLE9563_setSysStatCtrl(uint16_t u16_value);
uint16_t TLE9563_getSysStatCtrl(void);
uint16_t TLE9563_getGenBridgeCtrlReg(void);
uint8_t TLE9563_setGenBridgeCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvFreq(tBDRV_freq e_value);
uint16_t TLE9563_getBdrvFreq(void);
uint8_t TLE9563_setCpUnderVoltThresh(tCP_underVoltThresh e_value);
uint16_t TLE9563_getCpUnderVoltThresh(void);
uint8_t TLE9563_setBdrvExtMosfetLvl(tBDRV_extMosfetLvl e_value);
uint16_t TLE9563_getBdrvExtMosfetLvl(void);
uint8_t TLE9563_enCpAutoSwitchBetweenDualAndSingleStage(void);
uint8_t TLE9563_disCpAutoSwitchBetweenDualAndSingleStage(void);
uint8_t TLE9563_getCpAutoSwitchBetweenDualAndSingleStage(void);
uint8_t TLE9563_enBdrvVsVsintOverVoltRecovery(void);
uint8_t TLE9563_disBdrvVsVsintOverVoltRecovery(void);
uint8_t TLE9563_getBdrvVsVsintOverVoltRecovery(void);
uint8_t TLE9563_setBdrvPreChargePreDischargeCurAdaption(tBDRV_preChargePreDischargeCurAdaption e_value);
uint16_t TLE9563_getBdrvPreChargePreDischargeCurAdaption(void);
uint8_t TLE9563_setBdrvAdaptGateCtrl(tBDRV_adaptGateCtrl e_value);
uint16_t TLE9563_getBdrvAdaptGateCtrl(void);
uint8_t TLE9563_enCp(void);
uint8_t TLE9563_disCp(void);
uint8_t TLE9563_getCp(void);
uint8_t TLE9563_enBdrvPostCharge(void);
uint8_t TLE9563_disBdrvPostCharge(void);
uint8_t TLE9563_getBdrvPostCharge(void);
uint8_t TLE9563_enBdrvAdaptGateFilt(void);
uint8_t TLE9563_disBdrvAdaptGateFilt(void);
uint8_t TLE9563_getBdrvAdaptGateFilt(void);
uint8_t TLE9563_enBdrvDetectGeneratorMode(void);
uint8_t TLE9563_disBdrvDetectGeneratorMode(void);
uint8_t TLE9563_getBdrvDetectGeneratorMode(void);
uint8_t TLE9563_setBdrvHoldCur(tBDRV_holdCur e_value);
uint16_t TLE9563_getBdrvHoldCur(void);
uint8_t TLE9563_enCpFreqMod(void);
uint8_t TLE9563_disCpFreqMod(void);
uint8_t TLE9563_getCpFreqMod(void);
uint16_t TLE9563_getCsaCtrlReg(void);
uint8_t TLE9563_setCsaCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvNumberOfPwmInputs(tBDRV_numberOfPwmInputs e_value);
uint16_t TLE9563_getBdrvNumberOfPwmInputs(void);
uint8_t TLE9563_setCsaCap(tCSA_cap e_value);
uint16_t TLE9563_getCsaCap(void);
uint8_t TLE9563_setCsaDir(tCSA_dir e_value);
uint16_t TLE9563_getCsaDir(void);
uint8_t TLE9563_setCsaOverCurFiltTime(tCSA_overCurFiltTime e_value);
uint16_t TLE9563_getCsaOverCurFiltTime(void);
uint8_t TLE9563_enCsa(void);
uint8_t TLE9563_disCsa(void);
uint8_t TLE9563_getCsa(void);
uint8_t TLE9563_setCsaOverCurThresh(tCSA_overCurThresh e_value);
uint16_t TLE9563_getCsaOverCurThresh(void);
uint8_t TLE9563_setCsaGain(tCSA_gain e_value);
uint16_t TLE9563_getCsaGain(void);
uint8_t TLE9563_enCsaOverCurShutDown(void);
uint8_t TLE9563_disCsaOverCurShutDown(void);
uint8_t TLE9563_getCsaOverCurShutDown(void);
uint16_t TLE9563_getVdsLsCtrlReg(void);
uint8_t TLE9563_setVdsLsCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setVdsFiltTime(tVDS_filtTime e_value);
uint16_t TLE9563_getVdsFiltTime(void);
uint8_t TLE9563_setVdsLs3OverVoltThresh(tVDS_lsOverVoltThresh e_value);
uint16_t TLE9563_getVdsLs3OverVoltThresh(void);
uint8_t TLE9563_setVdsLs2OverVoltThresh(tVDS_lsOverVoltThresh e_value);
uint16_t TLE9563_getVdsLs2OverVoltThresh(void);
uint8_t TLE9563_setVdsLs1OverVoltThresh(tVDS_lsOverVoltThresh e_value);
uint16_t TLE9563_getVdsLs1OverVoltThresh(void);
uint16_t TLE9563_getVdsHsCtrlReg(void);
uint8_t TLE9563_setVdsHsCtrlReg(uint16_t u16_value);
uint8_t TLE9563_enBdrvDeepAdapt(void);
uint8_t TLE9563_disBdrvDeepAdapt(void);
uint8_t TLE9563_getBdrvDeepAdapt(void);
uint8_t TLE9563_setVdsHs3OverVoltThresh(tVDS_hsOverVoltThresh e_value);
uint16_t TLE9563_getVdsHs3OverVoltThresh(void);
uint8_t TLE9563_setVdsHs2OverVoltThresh(tVDS_hsOverVoltThresh e_value);
uint16_t TLE9563_getVdsHs2OverVoltThresh(void);
uint8_t TLE9563_setVdsHs1OverVoltThresh(tVDS_hsOverVoltThresh e_value);
uint16_t TLE9563_getVdsHs1OverVoltThresh(void);
uint16_t TLE9563_getCcpBlankHb1ActCtrlReg(void);
uint8_t TLE9563_setCcpBlankHb1ActCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb1ActTblank(tBDRV_tBlank e_value);
uint16_t TLE9563_getBdrvHb1ActTblank(void);
uint8_t TLE9563_setBdrvHb1ActTccp(tBDRV_tCcp e_value);
uint16_t TLE9563_getBdrvHb1ActTccp(void);
uint16_t TLE9563_getCcpBlankHb2ActCtrlReg(void);
uint8_t TLE9563_setCcpBlankHb2ActCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb2ActTblank(tBDRV_tBlank e_value);
uint16_t TLE9563_getBdrvHb2ActTblank(void);
uint8_t TLE9563_setBdrvHb2ActTccp(tBDRV_tCcp e_value);
uint16_t TLE9563_getBdrvHb2ActTccp(void);
uint16_t TLE9563_getCcpBlankHb3ActCtrlReg(void);
uint8_t TLE9563_setCcpBlankHb3ActCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb3ActTblank(tBDRV_tBlank e_value);
uint16_t TLE9563_getBdrvHb3ActTblank(void);
uint8_t TLE9563_setBdrvHb3ActTccp(tBDRV_tCcp e_value);
uint16_t TLE9563_getBdrvHb3ActTccp(void);
uint16_t TLE9563_getCcpBlankHb1FwCtrlReg(void);
uint8_t TLE9563_setCcpBlankHb1FwCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb1FwTblank(tBDRV_tBlank e_value);
uint16_t TLE9563_getBdrvHb1FwTblank(void);
uint8_t TLE9563_setBdrvHb1FwTccp(tBDRV_tCcp e_value);
uint16_t TLE9563_getBdrvHb1FwTccp(void);
uint16_t TLE9563_getCcpBlankHb2FwCtrlReg(void);
uint8_t TLE9563_setCcpBlankHb2FwCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb2FwTblank(tBDRV_tBlank e_value);
uint16_t TLE9563_getBdrvHb2FwTblank(void);
uint8_t TLE9563_setBdrvHb2FwTccp(tBDRV_tCcp e_value);
uint16_t TLE9563_getBdrvHb2FwTccp(void);
uint16_t TLE9563_getCcpBlankHb3FwCtrlReg(void);
uint8_t TLE9563_setCcpBlankHb3FwCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb3FwTblank(tBDRV_tBlank e_value);
uint16_t TLE9563_getBdrvHb3FwTblank(void);
uint8_t TLE9563_setBdrvHb3FwTccp(tBDRV_tCcp e_value);
uint16_t TLE9563_getBdrvHb3FwTccp(void);
uint16_t TLE9563_getHbModeCtrlReg(void);
uint8_t TLE9563_setHbModeCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb3Mode(tBDRV_hb3Mode e_value);
uint16_t TLE9563_getBdrvHb3Mode(void);
uint8_t TLE9563_enBdrvHb3ActiveFreeWheeling(void);
uint8_t TLE9563_disBdrvHb3ActiveFreeWheeling(void);
uint8_t TLE9563_getBdrvHb3ActiveFreeWheeling(void);
uint8_t TLE9563_enBdrvHb3PwmMode(void);
uint8_t TLE9563_disBdrvHb3PwmMode(void);
uint8_t TLE9563_getBdrvHb3PwmMode(void);
uint8_t TLE9563_setBdrvHb2Mode(tBDRV_hb2Mode e_value);
uint16_t TLE9563_getBdrvHb2Mode(void);
uint8_t TLE9563_enBdrvHb2ActiveFreeWheeling(void);
uint8_t TLE9563_disBdrvHb2ActiveFreeWheeling(void);
uint8_t TLE9563_getBdrvHb2ActiveFreeWheeling(void);
uint8_t TLE9563_enBdrvHb2PwmMode(void);
uint8_t TLE9563_disBdrvHb2PwmMode(void);
uint8_t TLE9563_getBdrvHb2PwmMode(void);
uint8_t TLE9563_setBdrvHb1Mode(tBDRV_hb1Mode e_value);
uint16_t TLE9563_getBdrvHb1Mode(void);
uint8_t TLE9563_enBdrvHb1ActiveFreeWheeling(void);
uint8_t TLE9563_disBdrvHb1ActiveFreeWheeling(void);
uint8_t TLE9563_getBdrvHb1ActiveFreeWheeling(void);
uint8_t TLE9563_enBdrvHb1PwmMode(void);
uint8_t TLE9563_disBdrvHb1PwmMode(void);
uint8_t TLE9563_getBdrvHb1PwmMode(void);
uint16_t TLE9563_getTpreChgCtrlReg(void);
uint8_t TLE9563_setTpreChgCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb3PreChargeTime(tBDRV_preChargeDischargeTime e_value);
uint16_t TLE9563_getBdrvHb3PreChargeTime(void);
uint8_t TLE9563_setBdrvHb2PreChargeTime(tBDRV_preChargeDischargeTime e_value);
uint16_t TLE9563_getBdrvHb2PreChargeTime(void);
uint8_t TLE9563_setBdrvHb1PreChargeTime(tBDRV_preChargeDischargeTime e_value);
uint16_t TLE9563_getBdrvHb1PreChargeTime(void);
uint16_t TLE9563_getTpreDischgCtrlReg(void);
uint8_t TLE9563_setTpreDischgCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb3PreDischargeTime(tBDRV_preChargeDischargeTime e_value);
uint16_t TLE9563_getBdrvHb3PreDischargeTime(void);
uint8_t TLE9563_setBdrvHb2PreDischargeTime(tBDRV_preChargeDischargeTime e_value);
uint16_t TLE9563_getBdrvHb2PreDischargeTime(void);
uint8_t TLE9563_setBdrvHb1PreDischargeTime(tBDRV_preChargeDischargeTime e_value);
uint16_t TLE9563_getBdrvHb1PreDischargeTime(void);
uint16_t TLE9563_getStIchgCtrlReg(void);
uint8_t TLE9563_setStIchgCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb3StaticChargeDischargeCur(tBDRV_staticChargeDischargeCur e_value);
uint16_t TLE9563_getBdrvHb3StaticChargeDischargeCur(void);
uint8_t TLE9563_setBdrvHb2StaticChargeDischargeCur(tBDRV_staticChargeDischargeCur e_value);
uint16_t TLE9563_getBdrvHb2StaticChargeDischargeCur(void);
uint8_t TLE9563_setBdrvHb1StaticChargeDischargeCur(tBDRV_staticChargeDischargeCur e_value);
uint16_t TLE9563_getBdrvHb1StaticChargeDischargeCur(void);
uint16_t TLE9563_getIchgHb1ActCtrlReg(void);
uint8_t TLE9563_setIchgHb1ActCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb1ActDischargeCur(tBDRV_dischargeCur e_value);
uint16_t TLE9563_getBdrvHb1ActDischargeCur(void);
uint8_t TLE9563_setBdrvHb1ActChargeCur(tBDRV_chargeCur e_value);
uint16_t TLE9563_getBdrvHb1ActChargeCur(void);
uint16_t TLE9563_getIchgHb2ActCtrlReg(void);
uint8_t TLE9563_setIchgHb2ActCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb2ActDischargeCur(tBDRV_dischargeCur e_value);
uint16_t TLE9563_getBdrvHb2ActDischargeCur(void);
uint8_t TLE9563_setBdrvHb2ActChargeCur(tBDRV_chargeCur e_value);
uint16_t TLE9563_getBdrvHb2ActChargeCur(void);
uint16_t TLE9563_getIchgHb3ActCtrlReg(void);
uint8_t TLE9563_setIchgHb3ActCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb3ActDischargeCur(tBDRV_dischargeCur e_value);
uint16_t TLE9563_getBdrvHb3ActDischargeCur(void);
uint8_t TLE9563_setBdrvHb3ActChargeCur(tBDRV_chargeCur e_value);
uint16_t TLE9563_getBdrvHb3ActChargeCur(void);
uint16_t TLE9563_getIchgHb1FwCtrlReg(void);
uint8_t TLE9563_setIchgHb1FwCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb1FwChargeAndDischargeCur(tBDRV_chargeCur e_value);
uint16_t TLE9563_getBdrvHb1FwChargeAndDischargeCur(void);
uint16_t TLE9563_getIchgHb2FwCtrlReg(void);
uint8_t TLE9563_setIchgHb2FwCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb2FwChargeAndDischargeCur(tBDRV_chargeCur e_value);
uint16_t TLE9563_getBdrvHb2FwChargeAndDischargeCur(void);
uint16_t TLE9563_getIchgHb3FwCtrlReg(void);
uint8_t TLE9563_setIchgHb3FwCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb3FwChargeAndDischargeCur(tBDRV_chargeCur e_value);
uint16_t TLE9563_getBdrvHb3FwChargeAndDischargeCur(void);
uint16_t TLE9563_getIchgMaxCtrlReg(void);
uint8_t TLE9563_setIchgMaxCtrlReg(uint16_t u16_value);
uint8_t TLE9563_enBdrvHb3DiagPullDown(void);
uint8_t TLE9563_disBdrvHb3DiagPullDown(void);
uint8_t TLE9563_getBdrvHb3DiagPullDown(void);
uint8_t TLE9563_enBdrvHb2DiagPullDown(void);
uint8_t TLE9563_disBdrvHb2DiagPullDown(void);
uint8_t TLE9563_getBdrvHb2DiagPullDown(void);
uint8_t TLE9563_enBdrvHb1DiagPullDown(void);
uint8_t TLE9563_disBdrvHb1DiagPullDown(void);
uint8_t TLE9563_getBdrvHb1DiagPullDown(void);
uint8_t TLE9563_setBdrvHb3MaxChargeCur(tBDRV_maxChargeCur e_value);
uint16_t TLE9563_getBdrvHb3MaxChargeCur(void);
uint8_t TLE9563_setBdrvHb2MaxChargeCur(tBDRV_maxChargeCur e_value);
uint16_t TLE9563_getBdrvHb2MaxChargeCur(void);
uint8_t TLE9563_setBdrvHb1MaxChargeCur(tBDRV_maxChargeCur e_value);
uint16_t TLE9563_getBdrvHb1MaxChargeCur(void);
uint16_t TLE9563_getPreChgInitHb1CtrlReg(void);
uint8_t TLE9563_setPreChgInitHb1CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb1InitPreDischargeCur(tBDRV_dischargeCur e_value);
uint16_t TLE9563_getBdrvHb1InitPreDischargeCur(void);
uint8_t TLE9563_setBdrvHb1InitPreChargeCur(tBDRV_chargeCur e_value);
uint16_t TLE9563_getBdrvHb1InitPreChargeCur(void);
uint16_t TLE9563_getPreChgInitHb2CtrlReg(void);
uint8_t TLE9563_setPreChgInitHb2CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb2InitPreDischargeCur(tBDRV_dischargeCur e_value);
uint16_t TLE9563_getBdrvHb2InitPreDischargeCur(void);
uint8_t TLE9563_setBdrvHb2InitPreChargeCur(tBDRV_chargeCur e_value);
uint16_t TLE9563_getBdrvHb2InitPreChargeCur(void);
uint16_t TLE9563_getPreChgInitHb3CtrlReg(void);
uint8_t TLE9563_setPreChgInitHb3CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb3InitPreDischargeCur(tBDRV_dischargeCur e_value);
uint16_t TLE9563_getBdrvHb3InitPreDischargeCur(void);
uint8_t TLE9563_setBdrvHb3InitPreChargeCur(tBDRV_chargeCur e_value);
uint16_t TLE9563_getBdrvHb3InitPreChargeCur(void);
uint16_t TLE9563_getTdonHb1CtrlReg(void);
uint8_t TLE9563_setTdonHb1CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb1Tdon(tBDRV_tdon e_value);
uint16_t TLE9563_getBdrvHb1Tdon(void);
uint16_t TLE9563_getTdonHb2CtrlReg(void);
uint8_t TLE9563_setTdonHb2CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb2Tdon(tBDRV_tdon e_value);
uint16_t TLE9563_getBdrvHb2Tdon(void);
uint16_t TLE9563_getTdonHb3CtrlReg(void);
uint8_t TLE9563_setTdonHb3CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb3Tdon(tBDRV_tdon e_value);
uint16_t TLE9563_getBdrvHb3Tdon(void);
uint16_t TLE9563_getTdoffHb1CtrlReg(void);
uint8_t TLE9563_setTdoffHb1CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb1Tdoff(tBDRV_tdoff e_value);
uint16_t TLE9563_getBdrvHb1Tdoff(void);
uint16_t TLE9563_getTdoffHb2CtrlReg(void);
uint8_t TLE9563_setTdoffHb2CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb2Tdoff(tBDRV_tdoff e_value);
uint16_t TLE9563_getBdrvHb2Tdoff(void);
uint16_t TLE9563_getTdoffHb3CtrlReg(void);
uint8_t TLE9563_setTdoffHb3CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setBdrvHb3Tdoff(tBDRV_tdoff e_value);
uint16_t TLE9563_getBdrvHb3Tdoff(void);
uint16_t TLE9563_getBrakeCtrlReg(void);
uint8_t TLE9563_setBrakeCtrlReg(uint16_t u16_value);
uint8_t TLE9563_enLs3CtrlInSlamMode(void);
uint8_t TLE9563_disLs3CtrlInSlamMode(void);
uint8_t TLE9563_getLs3CtrlInSlamMode(void);
uint8_t TLE9563_enLs2CtrlInSlamMode(void);
uint8_t TLE9563_disLs2CtrlInSlamMode(void);
uint8_t TLE9563_getLs2CtrlInSlamMode(void);
uint8_t TLE9563_enLs1CtrlInSlamMode(void);
uint8_t TLE9563_disLs1CtrlInSlamMode(void);
uint8_t TLE9563_getLs1CtrlInSlamMode(void);
uint8_t TLE9563_enSlamMode(void);
uint8_t TLE9563_disSlamMode(void);
uint8_t TLE9563_getSlamMode(void);
uint8_t TLE9563_setVdsOverVoltThreshLsBrake(tVDS_overVoltThreshLsBrake e_value);
uint16_t TLE9563_getVdsOverVoltThreshLsBrake(void);
uint8_t TLE9563_setVdsOverVoltBlankTimeBrake(tVDS_overVoltBlankTimeBrake e_value);
uint16_t TLE9563_getVdsOverVoltBlankTimeBrake(void);
uint8_t TLE9563_enParkingBrake(void);
uint8_t TLE9563_disParkingBrake(void);
uint8_t TLE9563_getParkingBrake(void);
uint8_t TLE9563_enOverVoltBrake(void);
uint8_t TLE9563_disOverVoltBrake(void);
uint8_t TLE9563_getOverVoltBrake(void);
uint8_t TLE9563_setVsOverVoltThreshBrake(tVS_overVoltThreshBrake e_value);
uint16_t TLE9563_getVsOverVoltThreshBrake(void);
uint16_t TLE9563_getCanSwkCtrlReg(void);
uint8_t TLE9563_setCanSwkCtrlReg(uint16_t u16_value);
uint8_t TLE9563_enCanSwkOscCalMode(void);
uint8_t TLE9563_disCanSwkOscCalMode(void);
uint8_t TLE9563_getCanSwkOscCalMode(void);
uint8_t TLE9563_enCanSwkOscRecalLock(void);
uint8_t TLE9563_disCanSwkOscRecalLock(void);
uint8_t TLE9563_getCanSwkOscRecalLock(void);
uint8_t TLE9563_enCanSwkTimeOutMask(void);
uint8_t TLE9563_disCanSwkTimeOutMask(void);
uint8_t TLE9563_getCanSwkTimeOutMask(void);
uint8_t TLE9563_enCanSwkConfigValid(void);
uint8_t TLE9563_disCanSwkConfigValid(void);
uint8_t TLE9563_getCanSwkConfigValid(void);
uint16_t TLE9563_getSwkBtl1CtrlReg(void);
uint8_t TLE9563_setSwkBtl1CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setSwkSamplingPointPos(uint8_t u8_value);
uint16_t TLE9563_getSwkSamplingPointPos(void);
uint8_t TLE9563_setSwkNbTimeQuanta(uint8_t u8_value);
uint16_t TLE9563_getSwkNbTimeQuanta(void);
uint16_t TLE9563_getSwkId1CtrlReg(void);
uint8_t TLE9563_setSwkId1CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setSwkIdBit28(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit28(void);
uint8_t TLE9563_setSwkIdBit27(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit27(void);
uint8_t TLE9563_setSwkIdBit26(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit26(void);
uint8_t TLE9563_setSwkIdBit25(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit25(void);
uint8_t TLE9563_setSwkIdBit24(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit24(void);
uint8_t TLE9563_setSwkIdBit23(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit23(void);
uint8_t TLE9563_setSwkIdBit22(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit22(void);
uint8_t TLE9563_setSwkIdBit21(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit21(void);
uint8_t TLE9563_setSwkIdBit20(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit20(void);
uint8_t TLE9563_setSwkIdBit19(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit19(void);
uint8_t TLE9563_setSwkIdBit18(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit18(void);
uint8_t TLE9563_setSwkIdBit17(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit17(void);
uint8_t TLE9563_setSwkIdBit16(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit16(void);
uint8_t TLE9563_setSwkIdBit15(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit15(void);
uint8_t TLE9563_setSwkIdBit14(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit14(void);
uint8_t TLE9563_setSwkIdBit13(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit13(void);
uint16_t TLE9563_getSwkId0CtrlReg(void);
uint8_t TLE9563_setSwkId0CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setSwkIdBit12(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit12(void);
uint8_t TLE9563_setSwkIdBit11(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit11(void);
uint8_t TLE9563_setSwkIdBit10(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit10(void);
uint8_t TLE9563_setSwkIdBit9(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit9(void);
uint8_t TLE9563_setSwkIdBit8(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit8(void);
uint8_t TLE9563_setSwkIdBit7(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit7(void);
uint8_t TLE9563_setSwkIdBit6(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit6(void);
uint8_t TLE9563_setSwkIdBit5(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit5(void);
uint8_t TLE9563_setSwkIdBit4(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit4(void);
uint8_t TLE9563_setSwkIdBit3(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit3(void);
uint8_t TLE9563_setSwkIdBit2(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit2(void);
uint8_t TLE9563_setSwkIdBit1(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit1(void);
uint8_t TLE9563_setSwkIdBit0(uint8_t u8_value);
uint16_t TLE9563_getSwkIdBit0(void);
uint8_t TLE9563_setSwkRemoteTransReq(tSWK_remoteTransReq e_value);
uint16_t TLE9563_getSwkRemoteTransReq(void);
uint8_t TLE9563_setSwkIdExtBit(tSWK_idExtBit e_value);
uint16_t TLE9563_getSwkIdExtBit(void);
uint16_t TLE9563_getSwkMaskId1CtrlReg(void);
uint8_t TLE9563_setSwkMaskId1CtrlReg(uint16_t u16_value);
uint8_t TLE9563_enSwkIdBit28Mask(void);
uint8_t TLE9563_disSwkIdBit28Mask(void);
uint8_t TLE9563_getSwkIdBit28Mask(void);
uint8_t TLE9563_enSwkIdBit27Mask(void);
uint8_t TLE9563_disSwkIdBit27Mask(void);
uint8_t TLE9563_getSwkIdBit27Mask(void);
uint8_t TLE9563_enSwkIdBit26Mask(void);
uint8_t TLE9563_disSwkIdBit26Mask(void);
uint8_t TLE9563_getSwkIdBit26Mask(void);
uint8_t TLE9563_enSwkIdBit25Mask(void);
uint8_t TLE9563_disSwkIdBit25Mask(void);
uint8_t TLE9563_getSwkIdBit25Mask(void);
uint8_t TLE9563_enSwkIdBit24Mask(void);
uint8_t TLE9563_disSwkIdBit24Mask(void);
uint8_t TLE9563_getSwkIdBit24Mask(void);
uint8_t TLE9563_enSwkIdBit23Mask(void);
uint8_t TLE9563_disSwkIdBit23Mask(void);
uint8_t TLE9563_getSwkIdBit23Mask(void);
uint8_t TLE9563_enSwkIdBit22Mask(void);
uint8_t TLE9563_disSwkIdBit22Mask(void);
uint8_t TLE9563_getSwkIdBit22Mask(void);
uint8_t TLE9563_enSwkIdBit21Mask(void);
uint8_t TLE9563_disSwkIdBit21Mask(void);
uint8_t TLE9563_getSwkIdBit21Mask(void);
uint8_t TLE9563_enSwkIdBit20Mask(void);
uint8_t TLE9563_disSwkIdBit20Mask(void);
uint8_t TLE9563_getSwkIdBit20Mask(void);
uint8_t TLE9563_enSwkIdBit19Mask(void);
uint8_t TLE9563_disSwkIdBit19Mask(void);
uint8_t TLE9563_getSwkIdBit19Mask(void);
uint8_t TLE9563_enSwkIdBit18Mask(void);
uint8_t TLE9563_disSwkIdBit18Mask(void);
uint8_t TLE9563_getSwkIdBit18Mask(void);
uint8_t TLE9563_enSwkIdBit17Mask(void);
uint8_t TLE9563_disSwkIdBit17Mask(void);
uint8_t TLE9563_getSwkIdBit17Mask(void);
uint8_t TLE9563_enSwkIdBit16Mask(void);
uint8_t TLE9563_disSwkIdBit16Mask(void);
uint8_t TLE9563_getSwkIdBit16Mask(void);
uint8_t TLE9563_enSwkIdBit15Mask(void);
uint8_t TLE9563_disSwkIdBit15Mask(void);
uint8_t TLE9563_getSwkIdBit15Mask(void);
uint8_t TLE9563_enSwkIdBit14Mask(void);
uint8_t TLE9563_disSwkIdBit14Mask(void);
uint8_t TLE9563_getSwkIdBit14Mask(void);
uint8_t TLE9563_enSwkIdBit13Mask(void);
uint8_t TLE9563_disSwkIdBit13Mask(void);
uint8_t TLE9563_getSwkIdBit13Mask(void);
uint16_t TLE9563_getSwkMaskId0CtrlReg(void);
uint8_t TLE9563_setSwkMaskId0CtrlReg(uint16_t u16_value);
uint8_t TLE9563_enSwkIdBit12Mask(void);
uint8_t TLE9563_disSwkIdBit12Mask(void);
uint8_t TLE9563_getSwkIdBit12Mask(void);
uint8_t TLE9563_enSwkIdBit11Mask(void);
uint8_t TLE9563_disSwkIdBit11Mask(void);
uint8_t TLE9563_getSwkIdBit11Mask(void);
uint8_t TLE9563_enSwkIdBit10Mask(void);
uint8_t TLE9563_disSwkIdBit10Mask(void);
uint8_t TLE9563_getSwkIdBit10Mask(void);
uint8_t TLE9563_enSwkIdBit9Mask(void);
uint8_t TLE9563_disSwkIdBit9Mask(void);
uint8_t TLE9563_getSwkIdBit9Mask(void);
uint8_t TLE9563_enSwkIdBit8Mask(void);
uint8_t TLE9563_disSwkIdBit8Mask(void);
uint8_t TLE9563_getSwkIdBit8Mask(void);
uint8_t TLE9563_enSwkIdBit7Mask(void);
uint8_t TLE9563_disSwkIdBit7Mask(void);
uint8_t TLE9563_getSwkIdBit7Mask(void);
uint8_t TLE9563_enSwkIdBit6Mask(void);
uint8_t TLE9563_disSwkIdBit6Mask(void);
uint8_t TLE9563_getSwkIdBit6Mask(void);
uint8_t TLE9563_enSwkIdBit5Mask(void);
uint8_t TLE9563_disSwkIdBit5Mask(void);
uint8_t TLE9563_getSwkIdBit5Mask(void);
uint8_t TLE9563_enSwkIdBit4Mask(void);
uint8_t TLE9563_disSwkIdBit4Mask(void);
uint8_t TLE9563_getSwkIdBit4Mask(void);
uint8_t TLE9563_enSwkIdBit3Mask(void);
uint8_t TLE9563_disSwkIdBit3Mask(void);
uint8_t TLE9563_getSwkIdBit3Mask(void);
uint8_t TLE9563_enSwkIdBit2Mask(void);
uint8_t TLE9563_disSwkIdBit2Mask(void);
uint8_t TLE9563_getSwkIdBit2Mask(void);
uint8_t TLE9563_enSwkIdBit1Mask(void);
uint8_t TLE9563_disSwkIdBit1Mask(void);
uint8_t TLE9563_getSwkIdBit1Mask(void);
uint8_t TLE9563_enSwkIdBit0Mask(void);
uint8_t TLE9563_disSwkIdBit0Mask(void);
uint8_t TLE9563_getSwkIdBit0Mask(void);
uint16_t TLE9563_getSwkDataLengthCodeReg(void);
uint8_t TLE9563_setSwkDataLengthCodeReg(uint16_t u16_value);
uint8_t TLE9563_setSwkDataLengthCode(tSWK_dataLengthCode e_value);
uint16_t TLE9563_getSwkDataLengthCode(void);
uint16_t TLE9563_getSwkData3CtrlReg(void);
uint8_t TLE9563_setSwkData3CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setSwkContentData7(uint8_t u8_value);
uint16_t TLE9563_getSwkContentData7(void);
uint8_t TLE9563_setSwkContentData6(uint8_t u8_value);
uint16_t TLE9563_getSwkContentData6(void);
uint16_t TLE9563_getSwkData2CtrlReg(void);
uint8_t TLE9563_setSwkData2CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setSwkContentData5(uint8_t u8_value);
uint16_t TLE9563_getSwkContentData5(void);
uint8_t TLE9563_setSwkContentData4(uint8_t u8_value);
uint16_t TLE9563_getSwkContentData4(void);
uint16_t TLE9563_getSwkData1CtrlReg(void);
uint8_t TLE9563_setSwkData1CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setSwkContentData3(uint8_t u8_value);
uint16_t TLE9563_getSwkContentData3(void);
uint8_t TLE9563_setSwkContentData2(uint8_t u8_value);
uint16_t TLE9563_getSwkContentData2(void);
uint16_t TLE9563_getSwkData0CtrlReg(void);
uint8_t TLE9563_setSwkData0CtrlReg(uint16_t u16_value);
uint8_t TLE9563_setSwkContentData1(uint8_t u8_value);
uint16_t TLE9563_getSwkContentData1(void);
uint8_t TLE9563_setSwkContentData0(uint8_t u8_value);
uint16_t TLE9563_getSwkContentData0(void);
uint16_t TLE9563_getSwkCanFdCtrlReg(void);
uint8_t TLE9563_setSwkCanFdCtrlReg(uint16_t u16_value);
uint8_t TLE9563_enSwkCanFdErrorCount(void);
uint8_t TLE9563_disSwkCanFdErrorCount(void);
uint8_t TLE9563_getSwkCanFdErrorCount(void);
uint8_t TLE9563_setCanFdDomFiltTime(tCANFD_domFiltTime e_value);
uint16_t TLE9563_getCanFdDomFiltTime(void);
uint8_t TLE9563_enCanFdTolerantMode(void);
uint8_t TLE9563_disCanFdTolerantMode(void);
uint8_t TLE9563_getCanFdTolerantMode(void);
uint16_t TLE9563_getSwkOscTrimCtrlReg(void);
uint8_t TLE9563_setSwkOscTrimCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setSwkReceiver(tSWK_receiver e_value);
uint16_t TLE9563_getSwkReceiver(void);
uint8_t TLE9563_setSwkTrimTemp(uint8_t u8_value);
uint16_t TLE9563_getSwkTrimTemp(void);
uint8_t TLE9563_setSwkTrimOsc(uint8_t u8_value);
uint16_t TLE9563_getSwkTrimOsc(void);
uint8_t TLE9563_getSwkOscCalHighRegSts(void);
uint8_t TLE9563_getSwkOscCalLowRegSts(void);
uint16_t TLE9563_getSwkCdrCtrlReg(void);
uint8_t TLE9563_setSwkCdrCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setSwkClockDataRecoveryInputFreq(tSWK_clockDataRecoveryInputFreq e_value);
uint16_t TLE9563_getSwkClockDataRecoveryInputFreq(void);
uint8_t TLE9563_setSwkFiltTimeConst(tSWK_filtTimeConst e_value);
uint16_t TLE9563_getSwkFiltTimeConst(void);
uint8_t TLE9563_enSwkClockDataRecovery(void);
uint8_t TLE9563_disSwkClockDataRecovery(void);
uint8_t TLE9563_getSwkClockDataRecovery(void);
uint16_t TLE9563_getSwkCdrLimitCtrlReg(void);
uint8_t TLE9563_setSwkCdrLimitCtrlReg(uint16_t u16_value);
uint8_t TLE9563_setSwkClockDataRecoveryLimitHigh(uint8_t u8_value);
uint16_t TLE9563_getSwkClockDataRecoveryLimitHigh(void);
uint8_t TLE9563_setSwkClockDataRecoveryLimitLow(uint8_t u8_value);
uint16_t TLE9563_getSwkClockDataRecoveryLimitLow(void);
uint16_t TLE9563_getSupStsReg(void);
uint8_t TLE9563_clrSupSts(void);
uint8_t TLE9563_getPorSts(void);
uint8_t TLE9563_getCpOverTempSts(void);
uint8_t TLE9563_getVcc1UnderVoltFsSts(void);
uint8_t TLE9563_getHsUnderVoltSts(void);
uint8_t TLE9563_getHsOverVoltSts(void);
uint8_t TLE9563_getVsintUnderVoltSts(void);
uint8_t TLE9563_getVsintOverVoltSts(void);
uint8_t TLE9563_getVsUnderVoltSts(void);
uint8_t TLE9563_getVsOverVoltSts(void);
uint8_t TLE9563_getCpUnterVoltSts(void);
uint8_t TLE9563_getVcc1ShortCircuitSts(void);
uint8_t TLE9563_getVcc1UnterVoltSts(void);
uint8_t TLE9563_getVcc1OverVoltSts(void);
uint8_t TLE9563_getVcc1UnderVoltWarnSts(void);
uint16_t TLE9563_getThermStsReg(void);
uint8_t TLE9563_clrThermSts(void);
uint8_t TLE9563_getThermShutDown2SafeStateSts(void);
uint8_t TLE9563_getThermShutDown2Sts(void);
uint8_t TLE9563_getThermShutDown1Sts(void);
uint8_t TLE9563_getThermWarnSts(void);
uint16_t TLE9563_getDevStsReg(void);
uint8_t TLE9563_clrDevSts(void);
uint8_t TLE9563_getCrcSts(void);
uint8_t TLE9563_getCrcFailSts(void);
uint8_t TLE9563_getDeviceSts(void);
uint8_t TLE9563_getOperatingModeSts(void);
uint8_t TLE9563_getWdFailSts(void);
uint8_t TLE9563_getSpiFailSts(void);
uint8_t TLE9563_getFailSts(void);
uint16_t TLE9563_getBusStsReg(void);
uint8_t TLE9563_clrBusSts(void);
uint8_t TLE9563_getCanTimeOutSts(void);
uint8_t TLE9563_getSwkSysErrSts(void);
uint8_t TLE9563_getCanFailSts(void);
uint8_t TLE9563_getVcanUnderVoltSts(void);
uint16_t TLE9563_getWkStsReg(void);
uint8_t TLE9563_clrWkSts(void);
uint8_t TLE9563_getWkCanWuSts(void);
uint8_t TLE9563_getWkTimer2WuSts(void);
uint8_t TLE9563_getWkTimer1WuSts(void);
uint8_t TLE9563_getWk5WuSts(void);
uint8_t TLE9563_getWk4WuSts(void);
uint16_t TLE9563_getWkxLvlStsReg(void);
uint8_t TLE9563_getWk5LvlSts(void);
uint8_t TLE9563_getWk4LvlSts(void);
uint16_t TLE9563_getHsOlOcOtStsReg(void);
uint8_t TLE9563_clrHsOlOcOtSts(void);
uint8_t TLE9563_getHs3OverTempSts(void);
uint8_t TLE9563_getHs2OverTempSts(void);
uint8_t TLE9563_getHs1OverTempSts(void);
uint8_t TLE9563_getHs3OpenLoadSts(void);
uint8_t TLE9563_getHs2OpenLoadSts(void);
uint8_t TLE9563_getHs1OpenLoadSts(void);
uint8_t TLE9563_getHs3OverCurSts(void);
uint8_t TLE9563_getHs2OverCurSts(void);
uint8_t TLE9563_getHs1OverCurSts(void);
uint16_t TLE9563_getGenStsReg(void);
uint8_t TLE9563_getHb3Vsh3VoltLvlSts(void);
uint8_t TLE9563_getHb2Vsh2VoltLvlSts(void);
uint8_t TLE9563_getHb1Vsh1VoltLvlSts(void);
uint8_t TLE9563_getPwm6Sts(void);
uint8_t TLE9563_getPwm5Sts(void);
uint8_t TLE9563_getPwm4Sts(void);
uint8_t TLE9563_getPwm3Sts(void);
uint8_t TLE9563_getPwm2Sts(void);
uint8_t TLE9563_getPwm1Sts(void);
uint16_t TLE9563_getTdregStsReg(void);
uint8_t TLE9563_getHb3PreDischargeCurClampSts(void);
uint8_t TLE9563_getHb2PreDischargeCurClampSts(void);
uint8_t TLE9563_getHb1PreDischargeCurClampSts(void);
uint8_t TLE9563_getHb3PreChargeCurClampSts(void);
uint8_t TLE9563_getHb2PreChargeCurClampSts(void);
uint8_t TLE9563_getHb1PreChargeCurClampSts(void);
uint8_t TLE9563_getHb3TdonTdoffRegSts(void);
uint8_t TLE9563_getHb2TdonTdoffRegSts(void);
uint8_t TLE9563_getHb1TdonTdoffRegSts(void);
uint16_t TLE9563_getDsovStsReg(void);
uint8_t TLE9563_clrDsovSts(void);
uint8_t TLE9563_getCsaOverCurSts(void);
uint8_t TLE9563_getVsintOverVoltBrakeSts(void);
uint8_t TLE9563_getVsOverVoltBrakeSts(void);
uint8_t TLE9563_getVdsOverVoltLs3BrakeSts(void);
uint8_t TLE9563_getVdsOverVoltLs2BrakeSts(void);
uint8_t TLE9563_getVdsOverVoltLs1BrakeSts(void);
uint8_t TLE9563_getVdsOverVoltLs3Sts(void);
uint8_t TLE9563_getVdsOverVoltHs3Sts(void);
uint8_t TLE9563_getVdsOverVoltLs2Sts(void);
uint8_t TLE9563_getVdsOverVoltHs2Sts(void);
uint8_t TLE9563_getVdsOverVoltLs1Sts(void);
uint8_t TLE9563_getVdsOverVoltHs1Sts(void);
uint16_t TLE9563_getEffTdoffTdon1StsReg(void);
uint8_t TLE9563_getBdrvEffTdoff1Sts(void);
uint8_t TLE9563_getBdrvEffTdon1Sts(void);
uint16_t TLE9563_getEffTdoffTdon2StsReg(void);
uint8_t TLE9563_getBdrvEffTdoff2Sts(void);
uint8_t TLE9563_getBdrvEffTdon2Sts(void);
uint16_t TLE9563_getEffTdoffTdon3StsReg(void);
uint8_t TLE9563_getBdrvEffTdoff3Sts(void);
uint8_t TLE9563_getBdrvEffTdon3Sts(void);
uint16_t TLE9563_getEffTfallTrise1StsReg(void);
uint8_t TLE9563_getBdrvEffTfall1Sts(void);
uint8_t TLE9563_getBdrvEffTrise1Sts(void);
uint16_t TLE9563_getEffTfallTrise2StsReg(void);
uint8_t TLE9563_getBdrvEffTfall2Sts(void);
uint8_t TLE9563_getBdrvEffTrise2Sts(void);
uint16_t TLE9563_getEffTfallTrise3StsReg(void);
uint8_t TLE9563_getBdrvEffTfall3Sts(void);
uint8_t TLE9563_getBdrvEffTrise3Sts(void);
uint16_t TLE9563_getSwkStsReg(void);
uint8_t TLE9563_clrSwkSts(void);
uint8_t TLE9563_getSwkSyncSts(void);
uint8_t TLE9563_getSwkWkPatternDetectSts(void);
uint8_t TLE9563_getSwkWkFrameDetectSts(void);
uint8_t TLE9563_getSwkCanSilentTimeSts(void);
uint8_t TLE9563_getSwkActivitySts(void);
uint8_t TLE9563_getSwkCanFrameErrCountSts(void);
uint8_t TLE9563_getSwkClockDataRecoveryOutputSts(void);
uint16_t TLE9563_getFamProdIdStsReg(void);
uint8_t TLE9563_getDeviceFamilyIdSts(void);
uint8_t TLE9563_getDeviceProductIdSts(void);

#endif /* _TLE9563_H */
