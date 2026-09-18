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
#ifndef IFX_MDA_FOCCONTROLLERF16_CFG_H
#define IFX_MDA_FOCCONTROLLERF16_CFG_H

#include "FocTiming_Cfg.h"

/* XML Version 1.0.4 */
#define IFX_MDA_FOCCONTROLLERF16_CFG_XML_VERSION (10004)

#define IFX_MDA_FOCCONTROLLERF16_CFG_BASE_CURRENT_A (50.00000)

#define IFX_MDA_FOCCONTROLLERF16_CFG_BASE_VOLTAGE_V (1000.00000)

#define IFX_MDA_FOCCONTROLLERF16_CFG_DIRECT_INDUCTANCE_Q15 (0x22D9) /*decimal 8921*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_DQDECOUPLINGENABLE (0x0) /*decimal 0*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_KAW_TS_Q (81u * FOC_CONTROL_PERIOD_US / 50u)

#define IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_KAW_TS_Q_FORMAT (0xF) /*decimal 15*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_KI_TS_Q (40u * FOC_CONTROL_PERIOD_US / 50u)

#define IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_KI_TS_Q_FORMAT (0xF) /*decimal 15*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_LIMIT_Q_FORMAT (0xF) /*decimal 15*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_OUT_LOW_LIMIT_Q (-16384)

#define IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_OUT_UPP_LIMIT_Q (0x4000) /*decimal 16384*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_PROPGAIN_Q (0x1B55) /*decimal 6997*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_PROPGAIN_Q_FORMAT (0xF) /*decimal 15*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_KAW_TS_Q (81u * FOC_CONTROL_PERIOD_US / 50u)

#define IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_KAW_TS_Q_FORMAT (0xF) /*decimal 15*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_KI_TS_Q (40u * FOC_CONTROL_PERIOD_US / 50u)

#define IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_KI_TS_Q_FORMAT (0xF) /*decimal 15*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_LIMIT_Q_FORMAT (0xF) /*decimal 15*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_OUT_LOW_LIMIT_Q (-16384)

#define IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_OUT_UPP_LIMIT_Q (0x4000) /*decimal 16384*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_PROPGAIN_Q (0x1B55) /*decimal 6997*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_PROPGAIN_Q_FORMAT (0xF) /*decimal 15*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_QUADRATURE_INDUCTANCE_Q15 (0x24FE) /*decimal 9470*/

#define IFX_MDA_FOCCONTROLLERF16_CFG_SAMPLING_TIME_US FOC_CONTROL_PERIOD_US

#endif /* IFX_MDA_FOCCONTROLLERF16_CFG_H */
