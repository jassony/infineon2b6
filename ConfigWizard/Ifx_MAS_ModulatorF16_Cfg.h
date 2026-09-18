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
#ifndef IFX_MAS_MODULATORF16_CFG_H
#define IFX_MAS_MODULATORF16_CFG_H

/* XML Version 1.0.6 */
#define IFX_MAS_MODULATORF16_CFG_XML_VERSION (10006)

#define IFX_MAS_MODULATORF16_CFG_BASE_VOLTAGE_V (1000.00000)

#define IFX_MAS_MODULATORF16_CFG_DEADTIME_TICK (0x78) /*decimal 120*/

#define IFX_MAS_MODULATORF16_CFG_DRIVERDELAY_TICK (0x30) /*decimal 48*/

#define IFX_MAS_MODULATORF16_CFG_ENABLE_FAULT_OUT (0x0) /*decimal 0*/

#define IFX_MAS_MODULATORF16_CFG_FAULT_OUT usrFaultCallback

#define IFX_MAS_MODULATORF16_CFG_FAULT_OUT_BEHAVIOR (0x0) /*decimal 0*/

#define IFX_MAS_MODULATORF16_CFG_FAULT_REACTION_MAX_AMPLITUDE 0//(0x1) /*decimal 1*/

#define IFX_MAS_MODULATORF16_CFG_FAULT_REACTION_OVERMODULATION 0//(0x1) /*decimal 1*/

#define IFX_MAS_MODULATORF16_CFG_MAX_AMPLITUDE_Q15 (0x4000) /*decimal 16384*/

#define IFX_MAS_MODULATORF16_CFG_MEASUREMENTTIME_TICK (0x28) /*decimal 40*/

#define IFX_MAS_MODULATORF16_CFG_PERIOD_TICK (0x7CF) /*decimal 1999*/

#define IFX_MAS_MODULATORF16_CFG_RINGINGTIME_TICK (0x18) /*decimal 24*/

#endif /* IFX_MAS_MODULATORF16_CFG_H */
