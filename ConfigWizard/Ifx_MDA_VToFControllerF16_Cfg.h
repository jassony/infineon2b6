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
#ifndef IFX_MDA_VTOFCONTROLLERF16_CFG_H
#define IFX_MDA_VTOFCONTROLLERF16_CFG_H

#include "FocTiming_Cfg.h"

/* XML Version 1.0.3 */
#define IFX_MDA_VTOFCONTROLLERF16_CFG_XML_VERSION (10003)

#define IFX_MDA_VTOFCONTROLLERF16_CFG_ANGLE_INC_Q14 (546u * FOC_CONTROL_PERIOD_US / 50u)

#define IFX_MDA_VTOFCONTROLLERF16_CFG_BASE_ELEC_SPEED_RADPS (0x105C) /*decimal 4188*/

#define IFX_MDA_VTOFCONTROLLERF16_CFG_BASE_VOLTAGE_V (1000.00000)

#define IFX_MDA_VTOFCONTROLLERF16_CFG_CORNER_SPEED (0x289) /*decimal 649*/

#define IFX_MDA_VTOFCONTROLLERF16_CFG_RATED_SPEED (0x1994) /*decimal 6548*/

#define IFX_MDA_VTOFCONTROLLERF16_CFG_SAMPLING_TIME_US FOC_CONTROL_PERIOD_US

#define IFX_MDA_VTOFCONTROLLERF16_CFG_V2CORNER_SPEED (0x312) /*decimal 786*/

#define IFX_MDA_VTOFCONTROLLERF16_CFG_V2MIN_SPEED (0x20C) /*decimal 524*/

#define IFX_MDA_VTOFCONTROLLERF16_CFG_V2RATED_SPEED (0x312) /*decimal 786*/

#endif /* IFX_MDA_VTOFCONTROLLERF16_CFG_H */
