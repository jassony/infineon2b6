#ifndef MTPA_Q15_ADAPTER_H
#define MTPA_Q15_ADAPTER_H

#include <stdint.h>

#include "Ifx_Math.h"

typedef struct
{
    Ifx_Math_Fract16 torqueQ15;
    Ifx_Math_Fract16 powerQ15;
} MTPA_Q15_TorqueEstimate;

void MTPA_Q15_initialize(void);

uint8_t MTPA_Q15_setMotorParameters(uint16_t directInductance_uH,
                                    uint16_t quadratureInductance_uH,
                                    uint8_t polePairs,
                                    uint16_t permanentMagnetFlux_mWb);

void MTPA_Q15_estimateTorque(const Ifx_Math_CmpFract16 *currentDq,
                             Ifx_Math_Fract16 speedQ15,
                             MTPA_Q15_TorqueEstimate *estimate);

void MTPA_Q15_computeReference(Ifx_Math_Fract16 torqueQ15,
                               Ifx_Math_Fract16 speedQ15,
                               Ifx_Math_CmpFract16 *referenceDq);

#endif /* MTPA_Q15_ADAPTER_H */
