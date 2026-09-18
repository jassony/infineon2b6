/*
 * Dead-time voltage compensation for the PWM voltage-command path.
 */
#ifndef IFX_MAS_DEADTIMECOMPENSATORF16_H
#define IFX_MAS_DEADTIMECOMPENSATORF16_H

#include "Ifx_Math.h"

typedef enum Ifx_MAS_DeadTimeCompensatorF16_Status
{
    Ifx_MAS_DeadTimeCompensatorF16_Status_disabled = 0u,
    Ifx_MAS_DeadTimeCompensatorF16_Status_active = 1u,
    Ifx_MAS_DeadTimeCompensatorF16_Status_invalidParameter = 2u
} Ifx_MAS_DeadTimeCompensatorF16_Status;

typedef struct Ifx_MAS_DeadTimeCompensatorF16_Output
{
    Ifx_Math_3PhaseFract16 compensationUVW;
    Ifx_Math_CmpFract16 compensationAlphaBeta;
    Ifx_Math_Fract16 factorQ15;
    uint8 status;
    uint8 active;
} Ifx_MAS_DeadTimeCompensatorF16_Output;

typedef struct Ifx_MAS_DeadTimeCompensatorF16
{
    bool p_enable;
    bool p_enableHysteresis;
    bool p_parametersValid;
    Ifx_Math_Fract16 p_hysteresisBandQ15;
    Ifx_Math_Fract16 p_factorQ15;
    Ifx_Math_Fract16 p_previousSignUVW[3];
    Ifx_MAS_DeadTimeCompensatorF16_Output p_output;
} Ifx_MAS_DeadTimeCompensatorF16;

void Ifx_MAS_DeadTimeCompensatorF16_init(Ifx_MAS_DeadTimeCompensatorF16* self);
uint8 Ifx_MAS_DeadTimeCompensatorF16_setParameters(Ifx_MAS_DeadTimeCompensatorF16* self,
    bool enable, bool enableHysteresis, Ifx_Math_Fract16 hysteresisBandQ15,
    uint16 deadTime_ns, uint16 switchingFrequency_Hz);
uint8 Ifx_MAS_DeadTimeCompensatorF16_execute(Ifx_MAS_DeadTimeCompensatorF16* self,
    Ifx_Math_3PhaseFract16 currentsUVW, Ifx_Math_Fract16 dcLinkVoltageQ15,
    Ifx_Math_CmpFract16* compensationAlphaBeta);

static inline void Ifx_MAS_DeadTimeCompensatorF16_getOutput(
    Ifx_MAS_DeadTimeCompensatorF16* self, Ifx_MAS_DeadTimeCompensatorF16_Output* output)
{
    *output = self->p_output;
}

#endif /* IFX_MAS_DEADTIMECOMPENSATORF16_H */
