#ifndef FWC_SPEED_RECOVERY_H
#define FWC_SPEED_RECOVERY_H

#include <stdint.h>

/* 2 kHz integration helper, independent of the estimator and hardware.
 * Current filter units are Q15 counts (not PU); PI integral units are Q24.
 * No fast-loop calls. No runtime calibration scan on the inactive path. */
typedef struct
{
    uint8_t active;
    int16_t speedCapQ15;
    float iqFilteredQ15;
} Fwc_SpeedRecovery;

static inline void Fwc_SpeedRecovery_reset(Fwc_SpeedRecovery *state)
{
    state->active = 0u;
    state->speedCapQ15 = 0;
    state->iqFilteredQ15 = 0.0F;
}

static inline void Fwc_SpeedRecovery_setActive(Fwc_SpeedRecovery *state,
    uint8_t active, int16_t referenceQ15, int16_t estimatedQ15,
    int16_t previousIqCommandQ15)
{
    if (active == 0u)
    {
        if (state->active != 0u)
        {
            Fwc_SpeedRecovery_reset(state);
        }
    }
    else if (state->active == 0u)
    {
        int32_t referenceMagnitude = referenceQ15;
        int32_t estimatedMagnitude = estimatedQ15;
        if (referenceMagnitude < 0) { referenceMagnitude = -referenceMagnitude; }
        if (estimatedMagnitude < 0) { estimatedMagnitude = -estimatedMagnitude; }
        if (referenceMagnitude > estimatedMagnitude)
        {
            referenceMagnitude = estimatedMagnitude;
        }
        if (referenceMagnitude > 32767) { referenceMagnitude = 32767; }
        state->speedCapQ15 = (int16_t)referenceMagnitude;
        /* Seed from the command, not an instantaneous ripple/noise sample. */
        state->iqFilteredQ15 = (float)previousIqCommandQ15;
        state->active = 1u;
    }
}

/* Apply AFTER normal speed-input limits and BEFORE the existing ramp.
 * This value is a target, never a direct write of the ramp output. */
static inline int16_t Fwc_SpeedRecovery_limitReference(
    const Fwc_SpeedRecovery *state, int16_t requestedQ15)
{
    if (state->active != 0u)
    {
        if (requestedQ15 > state->speedCapQ15) { return state->speedCapQ15; }
        if (requestedQ15 < -state->speedCapQ15) { return (int16_t)-state->speedCapQ15; }
    }
    return requestedQ15;
}

/* Called AFTER the normal speed PI, retaining its proportional action and
 * static back calculation. Only change NEXT-cycle integration, never Iq_out.
 * Opposing error must always be allowed to unwind the previous saturation. */
static inline int32_t Fwc_SpeedRecovery_trackIntegral(Fwc_SpeedRecovery *state,
    int16_t errorQ14, int16_t requestedIqQ15, int16_t measuredIqQ15,
    int32_t integralBeforeQ24, int32_t integralAfterQ24,
    int16_t kawTs, uint8_t kawQFormat, int16_t iqLoQ15, int16_t iqHiQ15)
{
    float mismatch;
    float integral;
    float gain;
    const float alpha = 0.5F / (20.0F + 0.5F);
    if (state->active == 0u) { return integralAfterQ24; }
    state->iqFilteredQ15 += alpha * ((float)measuredIqQ15 - state->iqFilteredQ15);
    mismatch = (float)requestedIqQ15 - state->iqFilteredQ15;
    if (!(((errorQ14 > 0) && (mismatch > 0.0F))
        || ((errorQ14 < 0) && (mismatch < 0.0F))))
    {
        return integralAfterQ24;
    }
    integral = (float)integralAfterQ24;
    if (((errorQ14 > 0) && (integralAfterQ24 > integralBeforeQ24))
        || ((errorQ14 < 0) && (integralAfterQ24 < integralBeforeQ24)))
    {
        integral = (float)integralBeforeQ24;
    }
    gain = 0.0F;
    if ((kawTs > 0) && (kawQFormat <= 30u))
    {
        gain = (float)kawTs / (float)((uint32_t)1u << kawQFormat);
        if (gain > 1.0F) { gain = 1.0F; }
    }
    integral -= gain * mismatch * 512.0F;
    if (integral > (float)iqHiQ15 * 512.0F) { integral = (float)iqHiQ15 * 512.0F; }
    if (integral < (float)iqLoQ15 * 512.0F) { integral = (float)iqLoQ15 * 512.0F; }
    return (int32_t)integral;
}

#endif
