/*
 * Implements the Motor Control Blockset Dead-Time Compensator equation:
 * VabcComp = Vabc + sign(Iabc) * Vdc * 2 * DeadTime * fc.
 */
#include "Ifx_MAS_DeadTimeCompensatorF16.h"
#include "Ifx_Math_Clarke.h"

#define IFX_MAS_DEADTIMECOMPENSATORF16_Q15_SCALE (32768ULL)
#define IFX_MAS_DEADTIMECOMPENSATORF16_NS_PER_S  (1000000000ULL)

static Ifx_Math_Fract16 Ifx_MAS_DeadTimeCompensatorF16_saturateS16(sint32 value)
{
    if (value > (sint32)INT16_MAX)
    {
        return INT16_MAX;
    }
    if (value < (sint32)INT16_MIN)
    {
        return INT16_MIN;
    }
    return (Ifx_Math_Fract16)value;
}

static Ifx_Math_Fract16 Ifx_MAS_DeadTimeCompensatorF16_getSign(
    Ifx_MAS_DeadTimeCompensatorF16* self, Ifx_Math_Fract16 currentQ15, uint8 index)
{
    Ifx_Math_Fract16 sign;

    if (self->p_enableHysteresis == true)
    {
        if (currentQ15 > self->p_hysteresisBandQ15)
        {
            sign = 1;
        }
        else if (currentQ15 < -self->p_hysteresisBandQ15)
        {
            sign = -1;
        }
        else
        {
            sign = self->p_previousSignUVW[index];
        }
    }
    else
    {
        /* The source block selects +1 at an exact zero crossing. */
        sign = (currentQ15 >= 0) ? 1 : -1;
    }

    self->p_previousSignUVW[index] = sign;
    return sign;
}

void Ifx_MAS_DeadTimeCompensatorF16_init(Ifx_MAS_DeadTimeCompensatorF16* self)
{
    self->p_enable = false;
    self->p_enableHysteresis = false;
    self->p_parametersValid = true;
    self->p_hysteresisBandQ15 = 0;
    self->p_factorQ15 = 0;
    self->p_previousSignUVW[0] = 1;
    self->p_previousSignUVW[1] = 1;
    self->p_previousSignUVW[2] = 1;
    self->p_output.compensationUVW.u = 0;
    self->p_output.compensationUVW.v = 0;
    self->p_output.compensationUVW.w = 0;
    self->p_output.compensationAlphaBeta.real = 0;
    self->p_output.compensationAlphaBeta.imag = 0;
    self->p_output.factorQ15 = 0;
    self->p_output.status = (uint8)Ifx_MAS_DeadTimeCompensatorF16_Status_disabled;
    self->p_output.active = 0u;
}

uint8 Ifx_MAS_DeadTimeCompensatorF16_setParameters(Ifx_MAS_DeadTimeCompensatorF16* self,
    bool enable, bool enableHysteresis, Ifx_Math_Fract16 hysteresisBandQ15,
    uint16 deadTime_ns, uint16 switchingFrequency_Hz)
{
    uint64 factorNumerator;
    uint64 factorQ15;

    factorNumerator = 2ULL * (uint64)deadTime_ns * (uint64)switchingFrequency_Hz
        * IFX_MAS_DEADTIMECOMPENSATORF16_Q15_SCALE;
    factorQ15 = (factorNumerator + (IFX_MAS_DEADTIMECOMPENSATORF16_NS_PER_S / 2ULL))
        / IFX_MAS_DEADTIMECOMPENSATORF16_NS_PER_S;

    self->p_enable = enable;
    self->p_enableHysteresis = enableHysteresis;
    self->p_hysteresisBandQ15 = hysteresisBandQ15;
    self->p_factorQ15 = 0;
    self->p_parametersValid = false;
    self->p_output.factorQ15 = 0;

    if ((hysteresisBandQ15 < 0) || (factorQ15 > (uint64)INT16_MAX))
    {
        self->p_output.status = (uint8)Ifx_MAS_DeadTimeCompensatorF16_Status_invalidParameter;
        self->p_output.active = 0u;
        return 0u;
    }

    self->p_factorQ15 = (Ifx_Math_Fract16)factorQ15;
    self->p_parametersValid = true;
    self->p_output.factorQ15 = self->p_factorQ15;
    /* execute() owns the live status. Keeping it intact here prevents every
     * realtime parameter update from appearing as a disabled PWM sample. */
    return 1u;
}

uint8 Ifx_MAS_DeadTimeCompensatorF16_execute(Ifx_MAS_DeadTimeCompensatorF16* self,
    Ifx_Math_3PhaseFract16 currentsUVW, Ifx_Math_Fract16 dcLinkVoltageQ15,
    Ifx_Math_CmpFract16* compensationAlphaBeta)
{
    Ifx_Math_3PhaseFract16 rawCompensationUVW;
    Ifx_Math_3PhaseFract16 balancedCompensationUVW;
    Ifx_Math_Fract16 compensationMagnitudeQ15;
    sint32 commonModeQ15;

    compensationAlphaBeta->real = 0;
    compensationAlphaBeta->imag = 0;
    self->p_output.compensationUVW.u = 0;
    self->p_output.compensationUVW.v = 0;
    self->p_output.compensationUVW.w = 0;
    self->p_output.compensationAlphaBeta = *compensationAlphaBeta;
    self->p_output.active = 0u;

    if (self->p_parametersValid == false)
    {
        self->p_output.status = (uint8)Ifx_MAS_DeadTimeCompensatorF16_Status_invalidParameter;
        return 0u;
    }

    if (self->p_factorQ15 <= 0)
    {
        self->p_output.status = (uint8)Ifx_MAS_DeadTimeCompensatorF16_Status_disabled;
        return 0u;
    }

    if (dcLinkVoltageQ15 <= 0)
    {
        self->p_output.status = (uint8)Ifx_MAS_DeadTimeCompensatorF16_Status_invalidParameter;
        return 0u;
    }

    compensationMagnitudeQ15 = (Ifx_Math_Fract16)(((sint32)dcLinkVoltageQ15 * self->p_factorQ15) >> 15);
    rawCompensationUVW.u = (Ifx_MAS_DeadTimeCompensatorF16_getSign(self, currentsUVW.u, 0u) > 0)
        ? compensationMagnitudeQ15 : (Ifx_Math_Fract16)-compensationMagnitudeQ15;
    rawCompensationUVW.v = (Ifx_MAS_DeadTimeCompensatorF16_getSign(self, currentsUVW.v, 1u) > 0)
        ? compensationMagnitudeQ15 : (Ifx_Math_Fract16)-compensationMagnitudeQ15;
    rawCompensationUVW.w = (Ifx_MAS_DeadTimeCompensatorF16_getSign(self, currentsUVW.w, 2u) > 0)
        ? compensationMagnitudeQ15 : (Ifx_Math_Fract16)-compensationMagnitudeQ15;

    /* The FOC voltage command has no zero-sequence axis; remove it before Clarke. */
    commonModeQ15 = ((sint32)rawCompensationUVW.u + (sint32)rawCompensationUVW.v
        + (sint32)rawCompensationUVW.w) / 3;
    balancedCompensationUVW.u = Ifx_MAS_DeadTimeCompensatorF16_saturateS16(
        (sint32)rawCompensationUVW.u - commonModeQ15);
    balancedCompensationUVW.v = Ifx_MAS_DeadTimeCompensatorF16_saturateS16(
        (sint32)rawCompensationUVW.v - commonModeQ15);
    balancedCompensationUVW.w = Ifx_MAS_DeadTimeCompensatorF16_saturateS16(
        (sint32)rawCompensationUVW.w - commonModeQ15);

    *compensationAlphaBeta = Ifx_Math_Clarke_F16(balancedCompensationUVW);
    self->p_output.compensationUVW = rawCompensationUVW;
    self->p_output.compensationAlphaBeta = *compensationAlphaBeta;

    /* Keep the calculated voltage available for XCP diagnostics even while
     * PWM compensation is disabled. The return value is the apply gate. */
    if (self->p_enable == false)
    {
        self->p_output.status = (uint8)Ifx_MAS_DeadTimeCompensatorF16_Status_disabled;
        return 0u;
    }

    self->p_output.status = (uint8)Ifx_MAS_DeadTimeCompensatorF16_Status_active;
    self->p_output.active = 1u;
    return 1u;
}
