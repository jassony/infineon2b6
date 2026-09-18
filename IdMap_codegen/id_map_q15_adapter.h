#ifndef ID_MAP_Q15_ADAPTER_H
#define ID_MAP_Q15_ADAPTER_H

#include <stdint.h>

#include "Ifx_Math.h"

#define ID_MAP_Q15_SPEED_POINTS (11u)
#define ID_MAP_Q15_IQ_POINTS    (11u)

/* Table layout is [speed index][Iq index]. The production adapter reads this
 * XCP table directly, using bilinear interpolation and clipped extrapolation. */
extern volatile int16_t Cal_IdMap_Table_Q15_s16[ID_MAP_Q15_SPEED_POINTS][ID_MAP_Q15_IQ_POINTS];
extern volatile uint16_t Cal_IdMap_SpdFltAlpha_Q15_u16;
extern volatile uint16_t Cal_IdMap_IqFltAlpha_Q15_u16;
extern volatile uint16_t Cal_IdMap_IsFltAlpha_Q15_u16;
extern volatile uint8_t Meas_IdMap_Act_u8;
extern volatile uint8_t Meas_IdMap_Stat_u8;
extern volatile uint8_t Meas_IdMap_Valid_u8;
extern volatile float Meas_IdMap_Spd_rpm_f32;
extern volatile float Meas_IdMap_Iq_A_f32;
extern volatile float Meas_IdMap_Id_A_f32;
extern volatile Ifx_Math_Fract16 Meas_IdMap_Id_Q15_s16;
extern volatile Ifx_Math_Fract16 Meas_IdMap_IsFbkFlt_Q15_s16;

void IdMap_Q15_initialize(void);

/* Uses the live Q15 calibration table, limits the complete d-q reference to
 * one per-unit, and publishes the sampled physical values. */
uint8_t IdMap_Q15_computeReference(Ifx_Math_Fract16 speedQ15,
                                   Ifx_Math_Fract16 iqReferenceQ15,
                                   Ifx_Math_Fract16 *idReferenceQ15);

/* Updates the observation-only filtered magnitude from measured d-q current. */
void IdMap_Q15_updateIsFeedbackMeasurement(const Ifx_Math_CmpFract16 *feedbackDq);

void IdMap_Q15_setActive(uint8_t active);

#endif /* ID_MAP_Q15_ADAPTER_H */
