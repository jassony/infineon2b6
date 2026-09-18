#include "../ConfigWizard/FocTiming_Cfg.h"
#if FOC_RRCDOB_ENABLE
#include "rrc_dob_compensator.h"
#include "rrc_dob_lead.h"

#include <limits.h>
#include <stddef.h>

#include "Ifx_MDA_FocControllerF16_Cfg.h"
#include "Ifx_MS_FocSolutionF16_Cfg.h"
#include "Ifx_Math_Park.h"
#include "no_opt.h"

#if (IFX_MS_FOCSOLUTIONF16_CFG_SAMPLING_TIME_US != FOC_CONTROL_PERIOD_US)
#error "RRC-DOB and FOC sample periods must match"
#endif

#if (IFX_MS_FOCSOLUTIONF16_CFG_CURRENT_LOOP_FACTOR != FOC_PWM_PER_CONTROL)
#error "RRC-DOB must execute once per control step"
#endif

#if (IFX_MDA_FOCCONTROLLERF16_CFG_DQDECOUPLINGENABLE != 0u)
#error "RRC-DOB requires explicit uApplied-uCrossCoupling wiring when d-q decoupling is enabled"
#endif

#define RRCDOB_Q15_ONE                    (32768L)
#define RRCDOB_Q26_SHIFT                  (26u)
#define RRCDOB_Q27_SHIFT                  (27u)
#define RRCDOB_Q29_SHIFT                  (29u)
#define RRCDOB_Q30_SHIFT                  (30u)
#define RRCDOB_Q31_ONE                    (0x80000000UL)
#define RRCDOB_Q30_ONE                    (0x40000000L)
#define RRCDOB_Q29_ONE                    (0x20000000L)
#define RRCDOB_Q15_TO_Q26                 (2048L)
#define RRCDOB_COEFFICIENT_REFRESH_TICKS  (500u / FOC_CONTROL_PERIOD_US)
#define RRCDOB_FAST_TICKS_PER_MS          (1000u / FOC_CONTROL_PERIOD_US)
#define RRCDOB_LUT_INTERVALS              (256u)
#define RRCDOB_MAX_ELECTRICAL_DELTA_Q32   (161061274UL)

typedef struct
{
    int32_t stateQ26[3];
    int32_t aQ27[9];
    int32_t voltageBQ27[3];
    int32_t currentBQ27[3];
    int32_t outputCQ27[3];
    int32_t directDQ27;
    int32_t rQ30;
    int32_t inverseOnePlusRQ30;
    int32_t kvQ27;
    int32_t lambdaQ27;
    int32_t rhoQ27;
    int16_t previousHatQ15;
    uint8_t leadHistoryValid;
} RRCDOB_Axis;

typedef struct
{
    RRCDOB_Parameters parameters;
    RRCDOB_Axis axis[2];
    RRCDOB_Output output;
    RRCDOB_LeadCoefficients lead;
    Ifx_Math_CmpFract16 capturedAppliedVoltageDQ15;
    uint32_t previousElectricalAngleQ32;
    uint32_t minimumAngleDeltaQ32;
    uint32_t maximumAngleDeltaQ32;
    uint32_t rampQ31;
    uint32_t rampStepQ31;
    int32_t previousQ29;
    uint8_t parameterValid;
    uint8_t previousAngleValid;
    uint8_t capturedVoltageFresh;
    uint8_t coefficientsValid;
    uint8_t coefficientAge;
    uint8_t speedInRange;
} RRCDOB_State;

static RRCDOB_State rrcDobState;

/* q = tan(6*pi*angleDeltaCycles). The LUT domain is 0..0.0375
 * electrical cycle/control sample, independent of Ts: 0..750 Hz at
 * 50 us or 0..375 Hz at 100 us. Keep q and q^2 inside signed Q30. */
static const int32_t rrcDobPrewarpQ29[257] = {
    0, 1482393, 2964809, 4447270, 5929799, 7412418, 8895150, 10378018,
    11861044, 13344252, 14827662, 16311299, 17795185, 19279343, 20763795, 22248564,
    23733673, 25219145, 26705002, 28191267, 29677963, 31165113, 32652740, 34140866,
    35629516, 37118711, 38608474, 40098830, 41589800, 43081409, 44573678, 46066632,
    47560293, 49054685, 50549832, 52045756, 53542481, 55040031, 56538429, 58037698,
    59537863, 61038947, 62540973, 64043966, 65547950, 67052948, 68558984, 70066083,
    71574268, 73083565, 74593996, 76105586, 77618361, 79132343, 80647559, 82164032,
    83681787, 85200849, 86721243, 88242994, 89766126, 91290666, 92816638, 94344068,
    95872980, 97403402, 98935357, 100468872, 102003972, 103540685, 105079034, 106619048,
    108160751, 109704170, 111249332, 112796263, 114344991, 115895540, 117447939, 119002215,
    120558395, 122116505, 123676574, 125238629, 126802698, 128368807, 129936986, 131507263,
    133079665, 134654221, 136230959, 137809908, 139391097, 140974555, 142560311, 144148394,
    145738833, 147331658, 148926899, 150524585, 152124748, 153727416, 155332620, 156940391,
    158550760, 160163757, 161779413, 163397761, 165018831, 166642654, 168269264, 169898691,
    171530969, 173166129, 174804204, 176445227, 178089231, 179736250, 181386317, 183039464,
    184695728, 186355141, 188017738, 189683553, 191352622, 193024979, 194700660, 196379701,
    198062136, 199748003, 201437337, 203130174, 204826553, 206526510, 208230081, 209937306,
    211648221, 213362865, 215081276, 216803493, 218529555, 220259501, 221993372, 223731206,
    225473043, 227218926, 228968893, 230722987, 232481249, 234243721, 236010444, 237781462,
    239556816, 241336550, 243120709, 244909334, 246702471, 248500164, 250302458, 252109398,
    253921031, 255737401, 257558555, 259384541, 261215405, 263051195, 264891959, 266737746,
    268588603, 270444582, 272305730, 274172099, 276043739, 277920700, 279803035, 281690796,
    283584034, 285482803, 287387156, 289297147, 291212829, 293134259, 295061491, 296994581,
    298933586, 300878562, 302829567, 304786659, 306749896, 308719337, 310695042, 312677072,
    314665486, 316660347, 318661716, 320669656, 322684230, 324705502, 326733536, 328768398,
    330810153, 332858867, 334914607, 336977442, 339047440, 341124669, 343209199, 345301102,
    347400448, 349507310, 351621759, 353743870, 355873716, 358011373, 360156917, 362310424,
    364471972, 366641639, 368819503, 371005646, 373200147, 375403089, 377614554, 379834625,
    382063388, 384300926, 386547327, 388802677, 391067066, 393340581, 395623314, 397915355,
    400216797, 402527733, 404848257, 407178464, 409518452, 411868317, 414228159, 416598077,
    418978172, 421368547, 423769305, 426180551, 428602390, 431034930, 433478279, 435932547,
    438397845, 440874285, 443361981, 445861048, 448371602, 450893762, 453427647, 455973378,
    458531077
};

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Cal_RRCDOB_Sel_u8 = RRCDOB_SELECTOR_OFF;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Cal_RRCDOB_Rst_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Cal_RRCDOB_Rev_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Cal_RRCDOB_Lead_us_u16 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_RRCDOB_WcRatio_Q15_s16 = 3277;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_RRCDOB_OutHi_Q15_s16 = 3277;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Cal_RRCDOB_SpdLo_rpm_u16 = 800u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Cal_RRCDOB_SpdHi_rpm_u16 = 4000u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Cal_RRCDOB_Ramp_ms_u16 = 10u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_RRCDOB_Sel_u8 = RRCDOB_SELECTOR_OFF;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_RRCDOB_Pend_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Meas_RRCDOB_Lead_us_u16 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_RRCDOB_Act_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_RRCDOB_OutAct_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_RRCDOB_Valid_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_RRCDOB_Stat_u8 = RRCDOB_STATUS_IDLE;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_RRCDOB_Sat_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Meas_RRCDOB_Freq_Hz_u16 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Meas_RRCDOB_VdHat_Q15_s16 = 0;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Meas_RRCDOB_VqHat_Q15_s16 = 0;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Meas_RRCDOB_VdOut_Q15_s16 = 0;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Meas_RRCDOB_VqOut_Q15_s16 = 0;

static uint64_t RrcDobCompensator_divideU64U32(uint64_t dividend, uint32_t divisor)
{
    uint64_t shiftedDivisor;
    uint32_t quotient = 0u;
    int32_t bit;

    if (divisor == 0u)
    {
        return UINT64_MAX;
    }
    if (dividend >= ((uint64_t)divisor << 32u))
    {
        return UINT64_MAX;
    }

    shiftedDivisor = (uint64_t)divisor << 31u;
    for (bit = 31; bit >= 0; bit--)
    {
        if (dividend >= shiftedDivisor)
        {
            dividend -= shiftedDivisor;
            quotient |= (UINT32_C(1) << (uint32_t)bit);
        }
        shiftedDivisor >>= 1u;
    }

    return quotient;
}

static uint64_t RrcDobCompensator_divideRoundU64U32(uint64_t dividend, uint32_t divisor)
{
    if (divisor == 0u)
    {
        return UINT64_MAX;
    }
    if (dividend > (UINT64_MAX - ((uint64_t)divisor >> 1u)))
    {
        return UINT64_MAX;
    }

    return RrcDobCompensator_divideU64U32(dividend + ((uint64_t)divisor >> 1u), divisor);
}

static int32_t RrcDobCompensator_roundShiftS64(int64_t value, uint8_t shift, uint8_t *overflow)
{
    uint64_t magnitude;
    uint64_t roundedMagnitude;
    uint64_t rounding;

    if (shift == 0u)
    {
        if (value > INT32_MAX)
        {
            *overflow = 1u;
            return INT32_MAX;
        }
        if (value < INT32_MIN)
        {
            *overflow = 1u;
            return INT32_MIN;
        }
        return (int32_t)value;
    }

    rounding = UINT64_C(1) << (shift - 1u);
    if (value >= 0)
    {
        magnitude = (uint64_t)value;
        roundedMagnitude = (magnitude + rounding) >> shift;
        if (roundedMagnitude > (uint64_t)INT32_MAX)
        {
            *overflow = 1u;
            return INT32_MAX;
        }
        return (int32_t)roundedMagnitude;
    }

    if (value == INT64_MIN)
    {
        *overflow = 1u;
        return INT32_MIN;
    }

    magnitude = (uint64_t)(-value);
    roundedMagnitude = (magnitude + rounding) >> shift;
    if (roundedMagnitude > (UINT64_C(1) << 31u))
    {
        *overflow = 1u;
        return INT32_MIN;
    }
    if (roundedMagnitude == (UINT64_C(1) << 31u))
    {
        return INT32_MIN;
    }
    return -(int32_t)roundedMagnitude;
}

static int64_t RrcDobCompensator_addSatS64(int64_t accumulator, int64_t addend, uint8_t *overflow)
{
    if ((addend > 0) && (accumulator > (INT64_MAX - addend)))
    {
        *overflow = 1u;
        return INT64_MAX;
    }
    if ((addend < 0) && (accumulator < (INT64_MIN - addend)))
    {
        *overflow = 1u;
        return INT64_MIN;
    }
    return accumulator + addend;
}

#if FOC_RRCDOB_APPLY_ENABLE
static Ifx_Math_Fract16 RrcDobCompensator_saturateQ15(int32_t value, uint8_t *saturated)
{
    if (value > INT16_MAX)
    {
        *saturated = 1u;
        return INT16_MAX;
    }
    if (value < INT16_MIN)
    {
        *saturated = 1u;
        return INT16_MIN;
    }
    return (Ifx_Math_Fract16)value;
}

#endif

static int32_t RrcDobCompensator_multiplyQ27(int32_t firstQ27, int32_t secondQ27, uint8_t *overflow)
{
    return RrcDobCompensator_roundShiftS64((int64_t)firstQ27 * secondQ27,
        RRCDOB_Q27_SHIFT, overflow);
}

static int32_t RrcDobCompensator_multiplyQ30(int32_t firstQ30, int32_t secondQ30, uint8_t *overflow)
{
    return RrcDobCompensator_roundShiftS64((int64_t)firstQ30 * secondQ30,
        RRCDOB_Q30_SHIFT, overflow);
}

static int32_t RrcDobCompensator_scaleQ30ToQ27(int64_t numeratorQ30, uint32_t inverseQ30,
                                                uint8_t *overflow)
{
    return RrcDobCompensator_roundShiftS64(numeratorQ30 * (int64_t)inverseQ30, 33u, overflow);
}

static uint32_t RrcDobCompensator_inverseQ30(uint32_t valueQ30)
{
    uint64_t quotient = RrcDobCompensator_divideRoundU64U32(UINT64_C(1) << 60u, valueQ30);

    if (quotient > UINT32_MAX)
    {
        return UINT32_MAX;
    }
    return (uint32_t)quotient;
}

static uint32_t RrcDobCompensator_speedToAngleDeltaQ32(uint16_t speed_rpm, uint8_t polePairs)
{
    uint64_t numerator = ((uint64_t)speed_rpm * polePairs) << 32u;
    uint64_t result = RrcDobCompensator_divideRoundU64U32(numerator, 60u * FOC_CONTROL_FREQUENCY_HZ);

    return (result > UINT32_MAX) ? UINT32_MAX : (uint32_t)result;
}

static int32_t RrcDobCompensator_lookupQ29(uint32_t electricalAngleDeltaQ32)
{
    uint64_t scaledByFive = (uint64_t)electricalAngleDeltaQ32 * 5u;
    uint32_t dividedBySixteen = (uint32_t)(scaledByFive >> 4u);
    uint32_t positionQ16 = (uint32_t)(((uint64_t)dividedBySixteen * UINT64_C(0xAAAAAAAB)) >> 33u);
    uint32_t index = positionQ16 >> 16u;
    uint32_t fractionQ16 = positionQ16 & 0xFFFFu;
    int32_t lower;
    int32_t difference;

    if (index >= RRCDOB_LUT_INTERVALS)
    {
        return rrcDobPrewarpQ29[RRCDOB_LUT_INTERVALS];
    }

    lower = rrcDobPrewarpQ29[index];
    difference = rrcDobPrewarpQ29[index + 1u] - lower;
    return lower + (int32_t)(((int64_t)difference * fractionQ16 + 32768) >> 16u);
}

static uint16_t RrcDobCompensator_angleDeltaToFrequencyHz(uint32_t angleDeltaQ32)
{
    uint64_t scaled = ((uint64_t)angleDeltaQ32 * FOC_CONTROL_FREQUENCY_HZ) + (UINT64_C(1) << 31u);
    uint32_t frequency = (uint32_t)(scaled >> 32u);

    return (frequency > UINT16_MAX) ? UINT16_MAX : (uint16_t)frequency;
}

static void RrcDobCompensator_clearAxisStates(void)
{
    uint8_t axisIndex;
    uint8_t stateIndex;

    for (axisIndex = 0u; axisIndex < 2u; axisIndex++)
    {
        rrcDobState.axis[axisIndex].previousHatQ15 = 0;
        rrcDobState.axis[axisIndex].leadHistoryValid = 0u;
        for (stateIndex = 0u; stateIndex < 3u; stateIndex++)
        {
            rrcDobState.axis[axisIndex].stateQ26[stateIndex] = 0;
        }
    }
}

static void RrcDobCompensator_clearDynamicState(uint8_t clearCapture)
{
    RrcDobCompensator_clearAxisStates();
    rrcDobState.previousElectricalAngleQ32 = 0u;
    rrcDobState.previousAngleValid = 0u;
    rrcDobState.previousQ29 = 0;
    rrcDobState.coefficientsValid = 0u;
    rrcDobState.coefficientAge = RRCDOB_COEFFICIENT_REFRESH_TICKS;
    rrcDobState.rampQ31 = 0u;
    rrcDobState.speedInRange = 0u;

    if (clearCapture != 0u)
    {
        rrcDobState.capturedAppliedVoltageDQ15.real = 0;
        rrcDobState.capturedAppliedVoltageDQ15.imag = 0;
        rrcDobState.capturedVoltageFresh = 0u;
    }
}

static void RrcDobCompensator_publishOutput(void)
{
    Meas_RRCDOB_Sel_u8 = (rrcDobState.parameterValid != 0u)
        ? rrcDobState.parameters.selector : RRCDOB_SELECTOR_OFF;
    Meas_RRCDOB_Act_u8 = rrcDobState.output.active;
    Meas_RRCDOB_OutAct_u8 = rrcDobState.output.outputActive;
    Meas_RRCDOB_Valid_u8 = rrcDobState.output.valid;
    Meas_RRCDOB_Stat_u8 = rrcDobState.output.status;
    Meas_RRCDOB_Sat_u8 = rrcDobState.output.saturated;
    Meas_RRCDOB_Freq_Hz_u16 = rrcDobState.output.electricalFrequency_Hz;
    Meas_RRCDOB_VdHat_Q15_s16 = rrcDobState.output.voltageErrorHatDQ15.real;
    Meas_RRCDOB_VqHat_Q15_s16 = rrcDobState.output.voltageErrorHatDQ15.imag;
    Meas_RRCDOB_VdOut_Q15_s16 = rrcDobState.output.correctionDQ15.real;
    Meas_RRCDOB_VqOut_Q15_s16 = rrcDobState.output.correctionDQ15.imag;
}

static void RrcDobCompensator_setInactiveOutput(Ifx_Math_CmpFract16 rawVoltageDQ15, uint8_t status)
{
    rrcDobState.output.appliedVoltageDQ15 = rrcDobState.capturedAppliedVoltageDQ15;
    rrcDobState.output.voltageErrorHatDQ15.real = 0;
    rrcDobState.output.voltageErrorHatDQ15.imag = 0;
    rrcDobState.output.correctionDQ15.real = 0;
    rrcDobState.output.correctionDQ15.imag = 0;
    rrcDobState.output.compensatedVoltageDQ15 = rawVoltageDQ15;
    rrcDobState.output.electricalFrequency_Hz = 0u;
    rrcDobState.output.active = 0u;
    rrcDobState.output.outputActive = 0u;
    rrcDobState.output.valid = 0u;
    rrcDobState.output.status = status;
    rrcDobState.output.saturated = 0u;
    RrcDobCompensator_publishOutput();
}

static uint8_t RrcDobCompensator_parametersEqual(const RRCDOB_Parameters *first,
                                                  const RRCDOB_Parameters *second)
{
    return ((first->selector == second->selector)
        && (first->resistance_mOhm == second->resistance_mOhm)
        && (first->inductanceD_uH == second->inductanceD_uH)
        && (first->inductanceQ_uH == second->inductanceQ_uH)
        && (first->polePairs == second->polePairs)
        && (first->cutoffRatioQ15 == second->cutoffRatioQ15)
        && (first->outputLimitQ15 == second->outputLimitQ15)
        && (first->speedLowerLimit_rpm == second->speedLowerLimit_rpm)
        && (first->speedUpperLimit_rpm == second->speedUpperLimit_rpm)
        && (first->rampTime_ms == second->rampTime_ms)
        && (first->reverseOutput == second->reverseOutput)
        && (first->leadTime_us == second->leadTime_us)) ? 1u : 0u;
}

static uint8_t RrcDobCompensator_prepareAxis(RRCDOB_Axis *axis, uint16_t resistance_mOhm,
                                             uint16_t inductance_uH)
{
    uint64_t value;
    uint32_t onePlusRQ30;

    value = RrcDobCompensator_divideRoundU64U32(
        (uint64_t)resistance_mOhm * (FOC_CONTROL_PERIOD_US / 2u) * (uint64_t)RRCDOB_Q30_ONE,
        (uint32_t)inductance_uH * 1000u);
    if ((value > (uint64_t)(RRCDOB_Q30_ONE >> 1u)) || (value > INT32_MAX))
    {
        return 0u;
    }
    axis->rQ30 = (int32_t)value;
    onePlusRQ30 = (uint32_t)(RRCDOB_Q30_ONE + axis->rQ30);
    axis->inverseOnePlusRQ30 = (int32_t)RrcDobCompensator_inverseQ30(onePlusRQ30);

    value = RrcDobCompensator_divideRoundU64U32(
        ((uint64_t)10u * FOC_CONTROL_PERIOD_US) << RRCDOB_Q27_SHIFT, inductance_uH);
    if (value > (UINT64_C(15) << RRCDOB_Q27_SHIFT))
    {
        return 0u;
    }
    axis->kvQ27 = (int32_t)value;

    value = RrcDobCompensator_divideRoundU64U32(
        (uint64_t)inductance_uH << RRCDOB_Q27_SHIFT, 20u * FOC_CONTROL_PERIOD_US);
    if (value > (UINT64_C(15) << RRCDOB_Q27_SHIFT))
    {
        return 0u;
    }
    axis->lambdaQ27 = (int32_t)value;

    value = RrcDobCompensator_divideRoundU64U32(
        (uint64_t)resistance_mOhm << RRCDOB_Q27_SHIFT, 20000u);
    if (value > INT32_MAX)
    {
        return 0u;
    }
    axis->rhoQ27 = (int32_t)value;

    return 1u;
}

static uint8_t RrcDobCompensator_buildAxisCoefficients(RRCDOB_Axis *axis, int32_t qQ30,
                                                        int32_t qSquaredQ30, int32_t cQ30,
                                                        int32_t twoCqQ30, uint32_t inverseDQ30,
                                                        uint8_t *overflow)
{
    int32_t oneMinusRQ30 = RRCDOB_Q30_ONE - axis->rQ30;
    int32_t onePlusRQ30 = RRCDOB_Q30_ONE + axis->rQ30;
    int32_t inverseDenominatorQ30 = RrcDobCompensator_multiplyQ30(
        (int32_t)inverseDQ30, axis->inverseOnePlusRQ30, overflow);
    int32_t factorQ27;
    int32_t cTimesDifferenceQ30;
    int32_t twoCqLambdaQ27;
    int32_t cRhoQ27;
    int64_t numeratorQ30;

    numeratorQ30 = (int64_t)RrcDobCompensator_multiplyQ30(oneMinusRQ30,
        RRCDOB_Q30_ONE + qSquaredQ30, overflow)
        - RrcDobCompensator_multiplyQ30(twoCqQ30, onePlusRQ30, overflow);
    axis->aQ27[0] = RrcDobCompensator_scaleQ30ToQ27(numeratorQ30,
        (uint32_t)inverseDenominatorQ30, overflow);
    axis->aQ27[1] = RrcDobCompensator_scaleQ30ToQ27(-((int64_t)twoCqQ30 << 1u),
        inverseDQ30, overflow);
    cTimesDifferenceQ30 = RrcDobCompensator_multiplyQ30(cQ30,
        qSquaredQ30 - axis->rQ30, overflow);
    axis->aQ27[2] = RrcDobCompensator_scaleQ30ToQ27(
        -((int64_t)cTimesDifferenceQ30 * 4),
        (uint32_t)inverseDenominatorQ30, overflow);
    axis->aQ27[3] = RrcDobCompensator_scaleQ30ToQ27(-((int64_t)qSquaredQ30 << 1u),
        (uint32_t)inverseDenominatorQ30, overflow);
    axis->aQ27[4] = RrcDobCompensator_scaleQ30ToQ27(
        (int64_t)RRCDOB_Q30_ONE + twoCqQ30 - qSquaredQ30, inverseDQ30, overflow);
    numeratorQ30 = RrcDobCompensator_multiplyQ30(qQ30,
        RRCDOB_Q30_ONE + axis->rQ30 + twoCqQ30, overflow);
    axis->aQ27[5] = RrcDobCompensator_scaleQ30ToQ27(numeratorQ30 << 1u,
        (uint32_t)inverseDenominatorQ30, overflow);
    axis->aQ27[6] = RrcDobCompensator_scaleQ30ToQ27(-((int64_t)qQ30 << 1u),
        (uint32_t)inverseDenominatorQ30, overflow);
    axis->aQ27[7] = RrcDobCompensator_scaleQ30ToQ27(-((int64_t)qQ30 << 1u),
        inverseDQ30, overflow);
    numeratorQ30 = (int64_t)RrcDobCompensator_multiplyQ30(onePlusRQ30,
        RRCDOB_Q30_ONE - qSquaredQ30, overflow)
        + RrcDobCompensator_multiplyQ30(twoCqQ30, oneMinusRQ30, overflow);
    axis->aQ27[8] = RrcDobCompensator_scaleQ30ToQ27(numeratorQ30,
        (uint32_t)inverseDenominatorQ30, overflow);

    factorQ27 = RrcDobCompensator_scaleQ30ToQ27(
        ((int64_t)RRCDOB_Q30_ONE + qSquaredQ30) << 1u,
        (uint32_t)inverseDenominatorQ30, overflow);
    axis->voltageBQ27[0] = RrcDobCompensator_multiplyQ27(factorQ27, axis->kvQ27, overflow);
    axis->voltageBQ27[1] = RrcDobCompensator_multiplyQ27(axis->aQ27[3], axis->kvQ27, overflow);
    axis->voltageBQ27[2] = RrcDobCompensator_multiplyQ27(axis->aQ27[6], axis->kvQ27, overflow);
    axis->currentBQ27[0] = -axis->aQ27[1];
    axis->currentBQ27[1] = RrcDobCompensator_scaleQ30ToQ27((int64_t)qSquaredQ30 << 1u,
        inverseDQ30, overflow);
    axis->currentBQ27[2] = RrcDobCompensator_scaleQ30ToQ27((int64_t)qQ30 << 1u,
        inverseDQ30, overflow);

    twoCqLambdaQ27 = RrcDobCompensator_roundShiftS64(
        (int64_t)twoCqQ30 * axis->lambdaQ27, RRCDOB_Q30_SHIFT, overflow);
    cRhoQ27 = RrcDobCompensator_roundShiftS64(
        (int64_t)cQ30 * axis->rhoQ27, RRCDOB_Q30_SHIFT, overflow);
    if ((twoCqLambdaQ27 > (INT32_MAX >> 1u)) || (cRhoQ27 > (INT32_MAX >> 1u)))
    {
        *overflow = 1u;
        return 0u;
    }
    axis->outputCQ27[0] = -(twoCqLambdaQ27 << 1u);
    axis->outputCQ27[1] = axis->outputCQ27[0];
    axis->outputCQ27[2] = cRhoQ27 << 1u;
    axis->directDQ27 = -axis->outputCQ27[0];

    return (*overflow == 0u) ? 1u : 0u;
}

static uint8_t RrcDobCompensator_scaleState(int32_t *stateQ26, uint64_t gain, uint8_t gainShift)
{
    uint64_t magnitude = (*stateQ26 < 0) ? (uint64_t)(-(int64_t)*stateQ26) : (uint64_t)*stateQ26;
    uint64_t scaled = (magnitude * gain) + (UINT64_C(1) << (gainShift - 1u));
    uint64_t result = scaled >> gainShift;

    if (*stateQ26 < 0)
    {
        if (result > (UINT64_C(1) << 31u))
        {
            return 0u;
        }
        *stateQ26 = (result == (UINT64_C(1) << 31u)) ? INT32_MIN : -(int32_t)result;
    }
    else
    {
        if (result > INT32_MAX)
        {
            return 0u;
        }
        *stateQ26 = (int32_t)result;
    }

    return 1u;
}

static uint8_t RrcDobCompensator_rescaleStates(int32_t newQ29, uint8_t *frequencyReset)
{
    uint64_t ratio;
    uint32_t ratioQ30;
    uint8_t axisIndex;

    *frequencyReset = 0u;
    if (rrcDobState.previousQ29 <= 0)
    {
        RrcDobCompensator_clearAxisStates();
        return 1u;
    }

    if (((int64_t)newQ29 > ((int64_t)rrcDobState.previousQ29 << 1u))
        || (((int64_t)newQ29 << 1u) < rrcDobState.previousQ29))
    {
        RrcDobCompensator_clearAxisStates();
        *frequencyReset = 1u;
        return 1u;
    }

    ratio = RrcDobCompensator_divideRoundU64U32(
        (uint64_t)(uint32_t)newQ29 << 30u, (uint32_t)rrcDobState.previousQ29);
    if (ratio > UINT32_MAX)
    {
        RrcDobCompensator_clearAxisStates();
        return 0u;
    }
    ratioQ30 = (uint32_t)ratio;

    for (axisIndex = 0u; axisIndex < 2u; axisIndex++)
    {
        if ((RrcDobCompensator_scaleState(&rrcDobState.axis[axisIndex].stateQ26[1],
                ratioQ30, 30u) == 0u)
            || (RrcDobCompensator_scaleState(&rrcDobState.axis[axisIndex].stateQ26[1],
                ratioQ30, 30u) == 0u)
            || (RrcDobCompensator_scaleState(&rrcDobState.axis[axisIndex].stateQ26[2],
                ratioQ30, 30u) == 0u))
        {
            RrcDobCompensator_clearAxisStates();
            return 0u;
        }
    }

    return 1u;
}

static uint8_t RrcDobCompensator_refreshCoefficients(int32_t qQ29,
                                                      uint8_t *frequencyReset)
{
    int32_t qQ30;
    int32_t qSquaredQ30;
    int32_t cQ30;
    int32_t cTimesQQ30;
    int32_t twoCqQ30;
    uint32_t denominatorQ30;
    uint32_t inverseDQ30;
    uint8_t overflow = 0u;
    uint8_t axisIndex;

    if (RrcDobCompensator_rescaleStates(qQ29, frequencyReset) == 0u)
    {
        return 0u;
    }

    qQ30 = qQ29 << 1u;
    qSquaredQ30 = RrcDobCompensator_multiplyQ30(qQ30, qQ30, &overflow);
    cQ30 = (int32_t)rrcDobState.parameters.cutoffRatioQ15 << 15u;
    cTimesQQ30 = RrcDobCompensator_multiplyQ30(cQ30, qQ30, &overflow);
    if (cTimesQQ30 > (INT32_MAX >> 1u))
    {
        overflow = 1u;
        twoCqQ30 = INT32_MAX;
    }
    else
    {
        twoCqQ30 = cTimesQQ30 << 1u;
    }
    denominatorQ30 = (uint32_t)((uint64_t)RRCDOB_Q30_ONE
        + (uint32_t)twoCqQ30 + (uint32_t)qSquaredQ30);
    inverseDQ30 = RrcDobCompensator_inverseQ30(denominatorQ30);

    for (axisIndex = 0u; axisIndex < 2u; axisIndex++)
    {
        if (RrcDobCompensator_buildAxisCoefficients(&rrcDobState.axis[axisIndex], qQ30,
                qSquaredQ30, cQ30, twoCqQ30, inverseDQ30, &overflow) == 0u)
        {
            overflow = 1u;
        }
    }

    if (overflow != 0u)
    {
        RrcDobCompensator_clearAxisStates();
        rrcDobState.coefficientsValid = 0u;
        return 0u;
    }

    rrcDobState.previousQ29 = qQ29;
    rrcDobState.coefficientsValid = 1u;
    rrcDobState.coefficientAge = 0u;
    return 1u;
}

static uint8_t RrcDobCompensator_executeAxis(RRCDOB_Axis *axis,
                                              Ifx_Math_Fract16 appliedVoltageQ15,
                                              Ifx_Math_Fract16 currentQ15,
                                              Ifx_Math_Fract16 *voltageErrorHatQ15,
                                              Ifx_Math_Fract16 *correctionQ15,
                                              uint8_t *saturated)
{
    int64_t accumulator = 0;
    int64_t inputProduct;
    int32_t voltageErrorHatQ15Value;
    int32_t rampedVoltageErrorQ15;
    int32_t correctionQ15Value;
    int32_t nextStateQ26[3];
    uint8_t overflow = 0u;
    uint8_t row;
    uint8_t column;

    for (column = 0u; column < 3u; column++)
    {
        accumulator = RrcDobCompensator_addSatS64(accumulator,
            (int64_t)axis->outputCQ27[column] * axis->stateQ26[column], &overflow);
    }
    inputProduct = (int64_t)axis->directDQ27 * currentQ15 * RRCDOB_Q15_TO_Q26;
    accumulator = RrcDobCompensator_addSatS64(accumulator, inputProduct, &overflow);
    voltageErrorHatQ15Value = RrcDobCompensator_roundShiftS64(accumulator, 38u, &overflow);
    if ((voltageErrorHatQ15Value > INT16_MAX) || (voltageErrorHatQ15Value < INT16_MIN))
    {
        return 0u;
    }
    *voltageErrorHatQ15 = (Ifx_Math_Fract16)voltageErrorHatQ15Value;

    /* Keep published Hat and the DOB state equations untouched. The predictor
     * history is the raw estimate, never the ramped/limited/signed output. */
    if ((rrcDobState.parameters.leadTime_us != 0u) && (axis->leadHistoryValid != 0u))
    {
        voltageErrorHatQ15Value = RrcDobLead_predict(rrcDobState.lead,
            *voltageErrorHatQ15, axis->previousHatQ15);
    }
    rampedVoltageErrorQ15 = RrcDobCompensator_roundShiftS64(
        (int64_t)voltageErrorHatQ15Value * (int64_t)rrcDobState.rampQ31, 31u, &overflow);
    /* Reverse only the injected correction, never the observer feedback. */
    correctionQ15Value = (rrcDobState.parameters.reverseOutput != 0u)
        ? rampedVoltageErrorQ15 : -rampedVoltageErrorQ15;
    if (correctionQ15Value > rrcDobState.parameters.outputLimitQ15)
    {
        correctionQ15Value = rrcDobState.parameters.outputLimitQ15;
        *saturated = 1u;
    }
    else if (correctionQ15Value < -rrcDobState.parameters.outputLimitQ15)
    {
        correctionQ15Value = -rrcDobState.parameters.outputLimitQ15;
        *saturated = 1u;
    }
    else
    {
        /* Within the configured compensation limit. */
    }
    *correctionQ15 = (Ifx_Math_Fract16)correctionQ15Value;

    for (row = 0u; row < 3u; row++)
    {
        accumulator = 0;
        for (column = 0u; column < 3u; column++)
        {
            accumulator = RrcDobCompensator_addSatS64(accumulator,
                (int64_t)axis->aQ27[(row * 3u) + column] * axis->stateQ26[column], &overflow);
        }
        inputProduct = (int64_t)axis->voltageBQ27[row] * appliedVoltageQ15 * RRCDOB_Q15_TO_Q26;
        accumulator = RrcDobCompensator_addSatS64(accumulator, inputProduct, &overflow);
        inputProduct = (int64_t)axis->currentBQ27[row] * currentQ15 * RRCDOB_Q15_TO_Q26;
        accumulator = RrcDobCompensator_addSatS64(accumulator, inputProduct, &overflow);
        nextStateQ26[row] = RrcDobCompensator_roundShiftS64(accumulator, RRCDOB_Q27_SHIFT, &overflow);
    }

    if (overflow != 0u)
    {
        return 0u;
    }

    for (row = 0u; row < 3u; row++)
    {
        axis->stateQ26[row] = nextStateQ26[row];
    }
    if (rrcDobState.parameters.leadTime_us != 0u)
    {
        axis->previousHatQ15 = *voltageErrorHatQ15;
        axis->leadHistoryValid = 1u;
    }
    return 1u;
}

void RrcDobCompensator_initialize(void)
{
    RRCDOB_Parameters defaults;

    rrcDobState.parameterValid = 0u;
    defaults.selector = Cal_RRCDOB_Sel_u8;
    defaults.reset = 0u;
    defaults.reverseOutput = Cal_RRCDOB_Rev_u8;
    defaults.leadTime_us = Cal_RRCDOB_Lead_us_u16;
    defaults.resistance_mOhm = 500u;
    defaults.inductanceD_uH = 1300u;
    defaults.inductanceQ_uH = 1380u;
    defaults.polePairs = 4u;
    defaults.cutoffRatioQ15 = Cal_RRCDOB_WcRatio_Q15_s16;
    defaults.outputLimitQ15 = Cal_RRCDOB_OutHi_Q15_s16;
    defaults.speedLowerLimit_rpm = Cal_RRCDOB_SpdLo_rpm_u16;
    defaults.speedUpperLimit_rpm = Cal_RRCDOB_SpdHi_rpm_u16;
    defaults.rampTime_ms = Cal_RRCDOB_Ramp_ms_u16;
    (void)RrcDobCompensator_setParameters(&defaults);
    RrcDobCompensator_reset();
}

void RrcDobCompensator_reset(void)
{
    Ifx_Math_CmpFract16 zeroVoltage;

    zeroVoltage.real = 0;
    zeroVoltage.imag = 0;
    RrcDobCompensator_clearDynamicState(1u);
    RrcDobCompensator_setInactiveOutput(zeroVoltage,
        (rrcDobState.parameterValid != 0u) ? RRCDOB_STATUS_IDLE : RRCDOB_STATUS_PARAMETER_INVALID);
}

uint8_t RrcDobCompensator_setParameters(const RRCDOB_Parameters *parameters)
{
    RRCDOB_Parameters nextParameters;
    Ifx_Math_CmpFract16 zeroVoltage;
    uint32_t minimumAngleDeltaQ32;
    uint32_t maximumAngleDeltaQ32;
    uint64_t rampTicks;
    uint64_t rampStep;
    uint8_t algorithmChanged;

    zeroVoltage.real = 0;
    zeroVoltage.imag = 0;
    if (parameters == NULL)
    {
        rrcDobState.parameterValid = 0u;
        RrcDobCompensator_reset();
        rrcDobState.output.status = RRCDOB_STATUS_PARAMETER_INVALID;
        RrcDobCompensator_publishOutput();
        return 0u;
    }

    nextParameters = *parameters;
    nextParameters.reset = 0u;
    if ((rrcDobState.parameterValid != 0u)
        && (parameters->reset == 0u)
        && (RrcDobCompensator_parametersEqual(&rrcDobState.parameters,
                &nextParameters) != 0u))
    {
        if (nextParameters.selector == RRCDOB_SELECTOR_OFF)
        {
            RrcDobCompensator_clearDynamicState(0u);
            RrcDobCompensator_setInactiveOutput(zeroVoltage, RRCDOB_STATUS_IDLE);
        }
        return 1u;
    }

    /* No engineering calibration range gate. Preserve the selector domain,
     * nonnegative Q15 bandwidth (shifted below), and limit magnitude. Numeric
     * coefficient/division checks below remain part of fixed-point math. */
    if ((parameters->selector > RRCDOB_SELECTOR_APPLY)
        || (parameters->cutoffRatioQ15 < 0)
        || (parameters->outputLimitQ15 < 0))
    {
        rrcDobState.parameterValid = 0u;
        RrcDobCompensator_reset();
        rrcDobState.output.status = RRCDOB_STATUS_PARAMETER_INVALID;
        RrcDobCompensator_publishOutput();
        return 0u;
    }

    minimumAngleDeltaQ32 = RrcDobCompensator_speedToAngleDeltaQ32(
        parameters->speedLowerLimit_rpm, parameters->polePairs);
    maximumAngleDeltaQ32 = RrcDobCompensator_speedToAngleDeltaQ32(
        parameters->speedUpperLimit_rpm, parameters->polePairs);
    /* Speed-window qualification and the LUT angle domain are evaluated
     * against actual angle increments in execute(), not at calibration. */

    if ((RrcDobCompensator_prepareAxis(&rrcDobState.axis[0], parameters->resistance_mOhm,
            parameters->inductanceD_uH) == 0u)
        || (RrcDobCompensator_prepareAxis(&rrcDobState.axis[1], parameters->resistance_mOhm,
            parameters->inductanceQ_uH) == 0u))
    {
        rrcDobState.parameterValid = 0u;
        RrcDobCompensator_reset();
        rrcDobState.output.status = RRCDOB_STATUS_PARAMETER_INVALID;
        RrcDobCompensator_publishOutput();
        return 0u;
    }

    rampTicks = (uint64_t)parameters->rampTime_ms * RRCDOB_FAST_TICKS_PER_MS;
    rampStep = RrcDobCompensator_divideU64U32(
        (uint64_t)RRCDOB_Q31_ONE + rampTicks - 1u, (uint32_t)rampTicks);
    if ((rampStep == 0u) || (rampStep > RRCDOB_Q31_ONE))
    {
        rrcDobState.parameterValid = 0u;
        RrcDobCompensator_reset();
        rrcDobState.output.status = RRCDOB_STATUS_PARAMETER_INVALID;
        RrcDobCompensator_publishOutput();
        return 0u;
    }

    algorithmChanged = (rrcDobState.parameterValid == 0u)
        || (RrcDobCompensator_parametersEqual(&rrcDobState.parameters, &nextParameters) == 0u);
    rrcDobState.parameters = nextParameters;
    Meas_RRCDOB_Lead_us_u16 = nextParameters.leadTime_us;
    rrcDobState.minimumAngleDeltaQ32 = minimumAngleDeltaQ32;
    rrcDobState.maximumAngleDeltaQ32 = maximumAngleDeltaQ32;
    rrcDobState.rampStepQ31 = (uint32_t)rampStep;
    rrcDobState.parameterValid = 1u;

    if ((algorithmChanged != 0u) || (parameters->reset != 0u)
        || (parameters->selector == RRCDOB_SELECTOR_OFF))
    {
        RrcDobCompensator_clearDynamicState(0u);
        RrcDobCompensator_setInactiveOutput(zeroVoltage,
            (parameters->selector == RRCDOB_SELECTOR_OFF)
                ? RRCDOB_STATUS_IDLE : RRCDOB_STATUS_INITIALIZING);
    }

    return 1u;
}

void RrcDobCompensator_captureAppliedVoltage(Ifx_Math_CmpFract16 voltageAlphaBetaQ15,
                                              uint32_t electricalAngleQ32)
{
    rrcDobState.capturedAppliedVoltageDQ15 = Ifx_Math_Park_F16(voltageAlphaBetaQ15,
        electricalAngleQ32);
    rrcDobState.capturedVoltageFresh = 1u;
}

uint8_t RrcDobCompensator_execute(Ifx_Math_CmpFract16 rawVoltageDQ_Q15,
                                  Ifx_Math_CmpFract16 currentDQ_Q15,
                                  uint32_t electricalAngleQ32,
                                  Ifx_Math_CmpFract16 *compensatedVoltageDQ_Q15)
{
    uint32_t angleDeltaQ32;
    int32_t qQ29;
    uint8_t saturated = 0u;
    uint8_t outputActive = 0u;
    uint8_t frequencyReset = 0u;
    uint8_t status;

    if (compensatedVoltageDQ_Q15 == NULL)
    {
        RrcDobCompensator_clearDynamicState(1u);
        RrcDobCompensator_setInactiveOutput(rawVoltageDQ_Q15,
            RRCDOB_STATUS_PARAMETER_INVALID);
        return 0u;
    }
    *compensatedVoltageDQ_Q15 = rawVoltageDQ_Q15;

    if ((rrcDobState.parameterValid == 0u)
        || (rrcDobState.parameters.selector > RRCDOB_SELECTOR_APPLY))
    {
        if (rrcDobState.output.status != RRCDOB_STATUS_PARAMETER_INVALID)
        {
            RrcDobCompensator_clearDynamicState(1u);
        }
        RrcDobCompensator_setInactiveOutput(rawVoltageDQ_Q15, RRCDOB_STATUS_PARAMETER_INVALID);
        return 0u;
    }

    if (rrcDobState.parameters.selector == RRCDOB_SELECTOR_OFF)
    {
        RrcDobCompensator_clearDynamicState(1u);
        RrcDobCompensator_setInactiveOutput(rawVoltageDQ_Q15, RRCDOB_STATUS_IDLE);
        return 0u;
    }

    if (rrcDobState.capturedVoltageFresh == 0u)
    {
        if (rrcDobState.output.status != RRCDOB_STATUS_APPLIED_VOLTAGE_STALE)
        {
            RrcDobCompensator_clearDynamicState(1u);
        }
        RrcDobCompensator_setInactiveOutput(rawVoltageDQ_Q15,
            RRCDOB_STATUS_APPLIED_VOLTAGE_STALE);
        return 0u;
    }
    rrcDobState.output.appliedVoltageDQ15 = rrcDobState.capturedAppliedVoltageDQ15;
    rrcDobState.capturedVoltageFresh = 0u;

    if (rrcDobState.previousAngleValid == 0u)
    {
        rrcDobState.previousElectricalAngleQ32 = electricalAngleQ32;
        rrcDobState.previousAngleValid = 1u;
        RrcDobCompensator_clearAxisStates();
        rrcDobState.speedInRange = 0u;
        RrcDobCompensator_setInactiveOutput(rawVoltageDQ_Q15, RRCDOB_STATUS_INITIALIZING);
        return 0u;
    }

    angleDeltaQ32 = electricalAngleQ32 - rrcDobState.previousElectricalAngleQ32;
    rrcDobState.previousElectricalAngleQ32 = electricalAngleQ32;
    if ((angleDeltaQ32 == 0u)
        || (angleDeltaQ32 > INT32_MAX)
        || (angleDeltaQ32 < rrcDobState.minimumAngleDeltaQ32)
        || (angleDeltaQ32 > rrcDobState.maximumAngleDeltaQ32)
        || (angleDeltaQ32 > RRCDOB_MAX_ELECTRICAL_DELTA_Q32))
    {
        if (rrcDobState.speedInRange != 0u)
        {
            /* Clear the dynamic observer only on the speed-qualification
             * falling edge. The angle still advances below, so re-entry is
             * detected without a 20 kHz reset storm. */
            RrcDobCompensator_clearAxisStates();
            rrcDobState.coefficientsValid = 0u;
            rrcDobState.previousQ29 = 0;
            rrcDobState.rampQ31 = 0u;
            rrcDobState.speedInRange = 0u;
        }
        RrcDobCompensator_setInactiveOutput(rawVoltageDQ_Q15,
            RRCDOB_STATUS_SPEED_OUT_OF_RANGE);
        return 0u;
    }

    rrcDobState.speedInRange = 1u;

    qQ29 = RrcDobCompensator_lookupQ29(angleDeltaQ32);
    if ((rrcDobState.coefficientsValid == 0u)
        || (rrcDobState.coefficientAge >= (RRCDOB_COEFFICIENT_REFRESH_TICKS - 1u)))
    {
        if (RrcDobCompensator_refreshCoefficients(qQ29, &frequencyReset) == 0u)
        {
            RrcDobCompensator_clearDynamicState(0u);
            RrcDobCompensator_setInactiveOutput(rawVoltageDQ_Q15,
                RRCDOB_STATUS_ARITHMETIC_OVERFLOW);
            return 0u;
        }
        if (rrcDobState.parameters.leadTime_us != 0u)
        {
            rrcDobState.lead = RrcDobLead_coefficients(angleDeltaQ32,
                rrcDobState.parameters.leadTime_us, FOC_CONTROL_PERIOD_US);
        }
        if (frequencyReset != 0u)
        {
            rrcDobState.rampQ31 = 0u;
            RrcDobCompensator_setInactiveOutput(rawVoltageDQ_Q15,
                RRCDOB_STATUS_INITIALIZING);
            return 0u;
        }
    }
    else
    {
        rrcDobState.coefficientAge++;
    }

    rrcDobState.output.electricalFrequency_Hz =
        RrcDobCompensator_angleDeltaToFrequencyHz(angleDeltaQ32);
    if ((RrcDobCompensator_executeAxis(&rrcDobState.axis[0],
            rrcDobState.output.appliedVoltageDQ15.real, currentDQ_Q15.real,
            &rrcDobState.output.voltageErrorHatDQ15.real,
            &rrcDobState.output.correctionDQ15.real, &saturated) == 0u)
        || (RrcDobCompensator_executeAxis(&rrcDobState.axis[1],
            rrcDobState.output.appliedVoltageDQ15.imag, currentDQ_Q15.imag,
            &rrcDobState.output.voltageErrorHatDQ15.imag,
            &rrcDobState.output.correctionDQ15.imag, &saturated) == 0u))
    {
        RrcDobCompensator_clearDynamicState(0u);
        RrcDobCompensator_setInactiveOutput(rawVoltageDQ_Q15,
            RRCDOB_STATUS_ARITHMETIC_OVERFLOW);
        return 0u;
    }

#if FOC_RRCDOB_APPLY_ENABLE
    if (rrcDobState.parameters.selector == RRCDOB_SELECTOR_APPLY)
    {
        int32_t directVoltage = (int32_t)rawVoltageDQ_Q15.real
            + rrcDobState.output.correctionDQ15.real;
        int32_t quadratureVoltage = (int32_t)rawVoltageDQ_Q15.imag
            + rrcDobState.output.correctionDQ15.imag;

        compensatedVoltageDQ_Q15->real = RrcDobCompensator_saturateQ15(directVoltage, &saturated);
        compensatedVoltageDQ_Q15->imag = RrcDobCompensator_saturateQ15(quadratureVoltage, &saturated);
        outputActive = 1u;
        status = RRCDOB_STATUS_APPLY_VALID;
    }
    else
    {
        status = RRCDOB_STATUS_MONITOR_VALID;
    }
#else
    status = (rrcDobState.parameters.selector == RRCDOB_SELECTOR_APPLY)
        ? RRCDOB_STATUS_APPLY_LOCKED : RRCDOB_STATUS_MONITOR_VALID;
#endif

    rrcDobState.output.compensatedVoltageDQ15 = *compensatedVoltageDQ_Q15;
    rrcDobState.output.active = 1u;
    rrcDobState.output.outputActive = outputActive;
    rrcDobState.output.valid = 1u;
    rrcDobState.output.status = status;
    rrcDobState.output.saturated = saturated;
    RrcDobCompensator_publishOutput();

    if (rrcDobState.rampQ31 < RRCDOB_Q31_ONE)
    {
        uint32_t remaining = RRCDOB_Q31_ONE - rrcDobState.rampQ31;
        rrcDobState.rampQ31 += (rrcDobState.rampStepQ31 >= remaining)
            ? remaining : rrcDobState.rampStepQ31;
    }

    return 1u;
}

void RrcDobCompensator_getOutput(RRCDOB_Output *output)
{
    if (output != NULL)
    {
        *output = rrcDobState.output;
    }
}

#endif /* FOC_RRCDOB_ENABLE */
