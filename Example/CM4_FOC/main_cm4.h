/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

/**
 * \file main_cm4.h
 * \brief This is an exemplary main function which integrates the motor control library including the TLE9563 hardware
 * abstraction.
 */

#ifndef MAIN_CM4_H
#define MAIN_CM4_H

extern void Ifx_FOC_speedLoopCallback(void);
extern void Ifx_FOC_periodMatchCallback(void);

/*
 * To use the 3 PWM mode please connect the following pins on the MMEk:
 *
 *  TVII <-> TLE9563
 *  ----------------
 *  P6.3 <-> P6.0
 *  P6.5 <-> P6.2
 *  P6.7 <-> P6.4
 *
 * Additionally, the enum in function Ifx_MHA_BridgeDrv_TLE9563_actionEnable in Ifx_MHA_BridgeDrv_TLE9563 should be
 * changed from BDRV_setAllHbx_hsOn_afwDis_pwmAct to BDRV_setAllHbx_hsOn_afwEn_pwmAct in order to activate the Active
 * Free Wheeling Feature of the TLE9563.
 *
 * To use the 6 PWM mode please connect the following pins on the MMEk:
 *
 *  TVII <-> TLE9563
 *  ----------------
 *  P6.0 <-> P6.0
 *  P6.1 <-> P6.1
 *  P6.2 <-> P6.2
 *  P6.3 <-> P6.3
 *  P6.4 <-> P6.4
 *  P6.5 <-> P6.5
 */

#define MMEK_USE_SIX_PWM 1

#endif /* MAIN_CM4_H */
