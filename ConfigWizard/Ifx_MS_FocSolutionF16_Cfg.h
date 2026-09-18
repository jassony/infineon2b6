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
#ifndef IFX_MS_FOCSOLUTIONF16_CFG_H
#define IFX_MS_FOCSOLUTIONF16_CFG_H

#include "FocTiming_Cfg.h"

/* XML Version 1.0.12 */
#define IFX_MS_FOCSOLUTIONF16_CFG_XML_VERSION (10012)

#define IFX_MS_FOCSOLUTIONF16_CFG_BASE_CURRENT_A (50.00000)

#define IFX_MS_FOCSOLUTIONF16_CFG_BASE_FLUX_WB (0.28)

#define IFX_MS_FOCSOLUTIONF16_CFG_BASE_INDUCTANCE_MH (5.62)

#define IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM (0x2710) /*decimal 10000*/

#define IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_TORQUE_NM (47.76)

#define IFX_MS_FOCSOLUTIONF16_CFG_BASE_POWER_W (0xC350) /*decimal 50000*/

#define IFX_MS_FOCSOLUTIONF16_CFG_BASE_RESISTANCE_OHM (0x14) /*decimal 20*/

#define IFX_MS_FOCSOLUTIONF16_CFG_BASE_VOLTAGE_V (1000.00000)

#define IFX_MS_FOCSOLUTIONF16_CFG_CLOSED_LOOP_RAMP_DOWN_RATE_Q30 (0x20D09) /*decimal 134409*/

#define IFX_MS_FOCSOLUTIONF16_CFG_CLOSED_LOOP_RAMP_UP_RATE_Q30 (0x20D09) /*decimal 134409*/

#define IFX_MS_FOCSOLUTIONF16_CFG_CURRENT_LOOP_FACTOR FOC_PWM_PER_CONTROL

#define IFX_MS_FOCSOLUTIONF16_CFG_FREQUENCY_KHZ (FOC_CONTROL_FREQUENCY_HZ / 1000u)

#define IFX_MS_FOCSOLUTIONF16_CFG_MAXIMUM_SPEED_Q15 (0x7FFC) /*decimal 32764*/

#define IFX_MS_FOCSOLUTIONF16_CFG_MINIMUM_SPEED_Q15 (0x0) /*decimal 0*/

#define IFX_MS_FOCSOLUTIONF16_CFG_OPEN_LOOP_RAMP_DOWN_RATE_Q30 (0x29F1) /*decimal 10737*/

#define IFX_MS_FOCSOLUTIONF16_CFG_OPEN_LOOP_RAMP_UP_RATE_Q30 (0x29F1) /*decimal 10737*/

#define IFX_MS_FOCSOLUTIONF16_CFG_POLE_PAIRS (0x4) /*decimal 4*/

#define IFX_MS_FOCSOLUTIONF16_CFG_Q_CURRENT_AT_TRANSITION_Q15 (0x428F) /*decimal 17039*/

#define IFX_MS_FOCSOLUTIONF16_CFG_SAMPLING_TIME_US FOC_CONTROL_PERIOD_US

#define IFX_MS_FOCSOLUTIONF16_CFG_SPEED_LOOP_FACTOR FOC_CONTROL_PER_SPEED

#define IFX_MS_FOCSOLUTIONF16_CFG_SPEED_LOOP_FREQUENCY_HZ (0x7D0) /*decimal 2000*/

#define IFX_MS_FOCSOLUTIONF16_CFG_SPEED_LOOP_PERIOD_US (0x1F4) /*decimal 500*/

#define IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_KAW_TS_Q (0x100) /*decimal 256*/

#define IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_KAW_TS_Q_FORMAT (0xF) /*decimal 15*/

#define IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_KI_TS_Q (0x2A) /*decimal 42*/

#define IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_KI_TS_Q_FORMAT (0xF) /*decimal 15*/

#define IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_LIMIT_Q_FORMAT (0xF) /*decimal 15*/

#define IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_OUT_LOW_LIMIT_Q (-26215)

#define IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_OUT_UPP_LIMIT_Q (0x6666) /*decimal 26214*/

#define IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_PROPGAIN_Q (0x3F30) /*decimal 16176*/

#define IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_PROPGAIN_Q_FORMAT (0xE) /*decimal 14*/

#define IFX_MS_FOCSOLUTIONF16_CFG_TRANSITION_MODE (0x0) /*decimal 0*/

#define IFX_MS_FOCSOLUTIONF16_CFG_TRANSITION_SPEED_DOWN_Q15 (0x147) /*decimal 327*/

#define IFX_MS_FOCSOLUTIONF16_CFG_TRANSITION_SPEED_UP_Q15 (0x51E) /*decimal 1310*/

#endif /* IFX_MS_FOCSOLUTIONF16_CFG_H */
