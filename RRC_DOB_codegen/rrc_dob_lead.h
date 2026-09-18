#ifndef RRC_DOB_LEAD_H
#define RRC_DOB_LEAD_H

#include <stdint.h>
#include "Ifx_Math_Sin.h"

/* Output-only narrowband predictor. No DOB state, gain, or dq rotation here.
 * The caller owns history validity and schedules frequency refresh. */
typedef struct
{
    int32_t currentQ16;
    int32_t previousQ16;
} RRCDOB_LeadCoefficients;

static inline int32_t RrcDobLead_sineQ30(uint32_t angleQ32)
{
    const uint8_t fractionBits = 32u - IFX_MATH_CFG_SIN_LUT_SIZE;
    const uint32_t index = angleQ32 >> fractionBits;
    const uint32_t fraction = angleQ32 & ((UINT32_C(1) << fractionBits) - 1u);
    const uint32_t next = (index + 1u) & ((UINT32_C(1) << IFX_MATH_CFG_SIN_LUT_SIZE) - 1u);
    const int32_t first = Ifx_Math_Lut_Sincos_F16_table[index];
    const int32_t difference = (int32_t)Ifx_Math_Lut_Sincos_F16_table[next] - first;
    /* Keep sub-Q15 interpolation precision for very small angle increments.
     * Multiplication, not signed left shift, also defines negative quadrants. */
    return first * 32768L + (int32_t)(((int64_t)difference * fraction * 32768L)
        / (INT64_C(1) << fractionBits));
}

static inline int32_t RrcDobLead_ratioQ16(int32_t numerator, int32_t denominator)
{
    int64_t scaled = (int64_t)numerator * 65536L;
    /* The admitted forward-angle domain is 0 < Omega <= 0.45*pi, hence
     * denominator > 0. Signed round-to-nearest, ties away from zero. */
    scaled += (scaled < 0) ? -(int64_t)(denominator / 2) : denominator / 2;
    return (int32_t)(scaled / denominator);
}

static inline RRCDOB_LeadCoefficients RrcDobLead_coefficients(
    uint32_t electricalDeltaQ32, uint16_t lead_us, uint16_t period_us)
{
    RRCDOB_LeadCoefficients c;
    const uint32_t omegaQ32 = electricalDeltaQ32 * 6u;
    uint32_t phiQ32;
    int32_t denominator;
    if (lead_us == 0u)
    {
        c.currentQ16 = 65536L;
        c.previousQ16 = 0;
        return c;
    }
    if (omegaQ32 == 0u)
    {
        c.previousQ16 = (int32_t)(((uint64_t)lead_us * 65536u + period_us / 2u) / period_us);
        c.currentQ16 = 65536L + c.previousQ16;
        return c;
    }
    phiQ32 = (uint32_t)(((uint64_t)omegaQ32 * lead_us + period_us / 2u) / period_us);
    denominator = RrcDobLead_sineQ30(omegaQ32);
    c.currentQ16 = RrcDobLead_ratioQ16(RrcDobLead_sineQ30(omegaQ32 + phiQ32), denominator);
    c.previousQ16 = RrcDobLead_ratioQ16(RrcDobLead_sineQ30(phiQ32), denominator);
    return c;
}

static inline int32_t RrcDobLead_predict(RRCDOB_LeadCoefficients c, int16_t current, int16_t previous)
{
    int64_t value = (int64_t)c.currentQ16 * current - (int64_t)c.previousQ16 * previous;
    value += (value < 0) ? -32768L : 32768L;
    return (int32_t)(value / 65536L);
}
#endif
