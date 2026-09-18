#include "id_map_q15_adapter.h"

#include <stddef.h>

#include "../ConfigWizard/Ifx_MS_FocSolutionF16_Cfg.h"
#include "../Math/include/Ifx_Math_ConvSat.h"
#include "../Utilities/no_opt.h"

#define ID_MAP_Q15_STATUS_INACTIVE        (0u)
#define ID_MAP_Q15_STATUS_VALID           (1u)
#define ID_MAP_Q15_STATUS_VECTOR_LIMITED  (2u)
#define ID_MAP_Q15_STATUS_INVALID_INPUT   (3u)
#define ID_MAP_Q15_FRACTION_ONE            (32768u)
#define ID_MAP_Q15_HALF_FRACTION           (16384)

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
/* cal0901 bench calibration. Rows are speed breakpoints from 0 to 10000 rpm;
 * columns are Iq breakpoints from 0 to 50 A. The PAR's Iq-row/Speed-column
 * values are transposed here for the production [speed][Iq] table layout. */
NO_OPT volatile int16_t Cal_IdMap_Table_Q15_s16[ID_MAP_Q15_SPEED_POINTS][ID_MAP_Q15_IQ_POINTS] =
{
    {     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 },
    {     0,     0,     0,     0,     0,  -655, -1967, -1967, -1967, -1967, -1967 },
    {     0,     0,     0,     0,  -655, -1310, -2622, -2622, -2622, -2622, -2622 },
    {     0,     0,     0,     0, -1310, -2620, -3277, -3277, -3277, -3277, -3277 },
    {     0,     0,  -655, -1310, -1965, -3275, -3932, -3932, -3932, -3932, -3932 },
    {     0,  -657, -1312, -1967, -2622, -4587, -5242, -5242, -5242, -5242, -5242 },
    {  -655, -1312, -1967, -2622, -3932, -5242, -5897, -5897, -5897, -5897, -5897 },
    { -1969, -3279, -4589, -6554, -6554, -6554, -6554, -6554, -6554, -6554, -6554 },
    { -3280, -4590, -5900, -7210, -7865, -7865, -7865, -7865, -7865, -7865, -7865 },
    { -4592, -5902, -7212, -9177,-11142,-11142,-11142,-11142,-11142,-11142,-11142 },
    { -4592, -5902, -7212, -9177,-11142,-11142,-11142,-11142,-11142,-11142,-11142 }
};

/* Fixed CANape display axes. The direct Q15 lookup uses the same 0:1000:10000
 * rpm and 0:5:50 A grids; these arrays are not intended for online tuning. */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_IdMap_IqAxis_A_f32[ID_MAP_Q15_IQ_POINTS] =
{
    0.0F, 5.0F, 10.0F, 15.0F, 20.0F, 25.0F, 30.0F, 35.0F, 40.0F, 45.0F, 50.0F
};

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_IdMap_SpdAxis_rpm_f32[ID_MAP_Q15_SPEED_POINTS] =
{
    0.0F, 1000.0F, 2000.0F, 3000.0F, 4000.0F, 5000.0F,
    6000.0F, 7000.0F, 8000.0F, 9000.0F, 10000.0F
};

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
/* First-order LPF coefficients, Q15 representation of alpha. */
NO_OPT volatile uint16_t Cal_IdMap_SpdFltAlpha_Q15_u16 = 3277u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Cal_IdMap_IqFltAlpha_Q15_u16 = 3277u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Cal_IdMap_IsFltAlpha_Q15_u16 = 3277u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_IdMap_Act_u8 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_IdMap_Stat_u8 = ID_MAP_Q15_STATUS_INACTIVE;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_IdMap_Valid_u8 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_IdMap_Spd_rpm_f32 = 0.0F;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_IdMap_Iq_A_f32 = 0.0F;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_IdMap_Id_A_f32 = 0.0F;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Meas_IdMap_Id_Q15_s16 = 0;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Meas_IdMap_IsFbkFlt_Q15_s16 = 0;

static uint8_t idMapActive;
static float idMapIsFbkFilteredQ15;
static uint8_t idMapIsFbkFilterInitialized;

static uint16_t IdMap_Q15_absSaturated(const Ifx_Math_Fract16 value)
{
    int32_t magnitude = (int32_t)value;

    if (magnitude < 0)
    {
        magnitude = -magnitude;
    }

    if (magnitude > 32767)
    {
        magnitude = 32767;
    }

    return (uint16_t)magnitude;
}

static void IdMap_Q15_getGridPosition(const uint16_t magnitudeQ15,
                                       const uint8_t pointCount,
                                       uint8_t *lowerIndex,
                                       uint16_t *fractionQ15)
{
    const uint32_t intervalCount = (uint32_t)pointCount - 1u;
    const uint32_t scaledPosition = (uint32_t)magnitudeQ15 * intervalCount;
    uint8_t index;

    if (magnitudeQ15 >= 32767u)
    {
        *lowerIndex = pointCount - 2u;
        *fractionQ15 = ID_MAP_Q15_FRACTION_ONE;
        return;
    }

    index = (uint8_t)(scaledPosition / ID_MAP_Q15_FRACTION_ONE);
    *lowerIndex = index;
    *fractionQ15 = (uint16_t)(scaledPosition
        - ((uint32_t)index * ID_MAP_Q15_FRACTION_ONE));
}

static int32_t IdMap_Q15_interpolate(const int32_t lowerValue,
                                      const int32_t upperValue,
                                      const uint16_t fractionQ15)
{
    int32_t product = (upperValue - lowerValue) * (int32_t)fractionQ15;

    product += (product >= 0) ? ID_MAP_Q15_HALF_FRACTION : -ID_MAP_Q15_HALF_FRACTION;
    return lowerValue + (product / (int32_t)ID_MAP_Q15_FRACTION_ONE);
}

static float IdMap_Q15_alphaToFloat(uint16_t alphaQ15)
{
    if (alphaQ15 > 32767u)
    {
        alphaQ15 = 32767u;
    }

    return (float)alphaQ15 / 32768.0F;
}

static float IdMap_Q15_lowPassSample(const float input,
                                     const float previous,
                                     const uint16_t alphaQ15)
{
    return previous + ((input - previous) * IdMap_Q15_alphaToFloat(alphaQ15));
}

static void IdMap_Q15_filterInputs(const float speedRpm,
                                   const float iqA,
                                   Ifx_Math_Fract16 *filteredSpeedQ15,
                                   Ifx_Math_Fract16 *filteredIqQ15)
{
    float previousSpeedRpm;
    float previousIqA;

    if (Meas_IdMap_Valid_u8 == 0u)
    {
        /* The first sample after activation initializes the filter so IdMap
         * does not create a synthetic startup ramp. */
        Meas_IdMap_Spd_rpm_f32 = speedRpm;
        Meas_IdMap_Iq_A_f32 = iqA;
    }
    else
    {
        previousSpeedRpm = Meas_IdMap_Spd_rpm_f32;
        previousIqA = Meas_IdMap_Iq_A_f32;
        Meas_IdMap_Spd_rpm_f32 = IdMap_Q15_lowPassSample(speedRpm,
            previousSpeedRpm, Cal_IdMap_SpdFltAlpha_Q15_u16);
        Meas_IdMap_Iq_A_f32 = IdMap_Q15_lowPassSample(iqA,
            previousIqA, Cal_IdMap_IqFltAlpha_Q15_u16);
    }

    *filteredSpeedQ15 = Ifx_Math_ConvSat_Flt32ToF16(
        Meas_IdMap_Spd_rpm_f32
            / (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM,
        Ifx_Math_FractQFormat_q15);
    *filteredIqQ15 = Ifx_Math_ConvSat_Flt32ToF16(
        Meas_IdMap_Iq_A_f32 / (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_CURRENT_A,
        Ifx_Math_FractQFormat_q15);
}

static Ifx_Math_Fract16 IdMap_Q15_lookup(const Ifx_Math_Fract16 speedQ15,
                                         const Ifx_Math_Fract16 iqReferenceQ15)
{
    uint8_t speedIndex;
    uint8_t iqIndex;
    uint16_t speedFractionQ15;
    uint16_t iqFractionQ15;
    int32_t lowerSpeedValue;
    int32_t upperSpeedValue;
    int32_t lowerSpeedLowerIq;
    int32_t lowerSpeedUpperIq;
    int32_t upperSpeedLowerIq;
    int32_t upperSpeedUpperIq;
    int32_t result;

    IdMap_Q15_getGridPosition(IdMap_Q15_absSaturated(speedQ15),
        ID_MAP_Q15_SPEED_POINTS, &speedIndex, &speedFractionQ15);
    IdMap_Q15_getGridPosition(IdMap_Q15_absSaturated(iqReferenceQ15),
        ID_MAP_Q15_IQ_POINTS, &iqIndex, &iqFractionQ15);

    /* Each 16-bit table read is live. With no ApplySeq handshake, an XCP
     * update takes effect as soon as CANape writes the respective cell. */
    lowerSpeedLowerIq = (int32_t)Cal_IdMap_Table_Q15_s16[speedIndex][iqIndex];
    lowerSpeedUpperIq = (int32_t)Cal_IdMap_Table_Q15_s16[speedIndex][iqIndex + 1u];
    upperSpeedLowerIq = (int32_t)Cal_IdMap_Table_Q15_s16[speedIndex + 1u][iqIndex];
    upperSpeedUpperIq = (int32_t)Cal_IdMap_Table_Q15_s16[speedIndex + 1u][iqIndex + 1u];

    lowerSpeedValue = IdMap_Q15_interpolate(lowerSpeedLowerIq, lowerSpeedUpperIq,
        iqFractionQ15);
    upperSpeedValue = IdMap_Q15_interpolate(upperSpeedLowerIq, upperSpeedUpperIq,
        iqFractionQ15);
    result = IdMap_Q15_interpolate(lowerSpeedValue, upperSpeedValue,
        speedFractionQ15);

    if (result > 32767)
    {
        result = 32767;
    }
    else if (result < -32768)
    {
        result = -32768;
    }

    return (Ifx_Math_Fract16)result;
}

static uint32_t IdMap_Q15_integerSquareRoot(uint32_t value)
{
    uint32_t result = 0u;
    uint32_t bit = (uint32_t)1u << 30u;

    while (bit > value)
    {
        bit >>= 2u;
    }

    while (bit != 0u)
    {
        if (value >= (result + bit))
        {
            value -= result + bit;
            result = (result >> 1u) + bit;
        }
        else
        {
            result >>= 1u;
        }

        bit >>= 2u;
    }

    return result;
}

static Ifx_Math_Fract16 IdMap_Q15_calculateDqMagnitude(
    const Ifx_Math_CmpFract16 *feedbackDq)
{
    const int32_t directCurrentQ15 = (int32_t)feedbackDq->real;
    const int32_t quadratureCurrentQ15 = (int32_t)feedbackDq->imag;
    const uint32_t squaredMagnitude =
        (uint32_t)(directCurrentQ15 * directCurrentQ15)
        + (uint32_t)(quadratureCurrentQ15 * quadratureCurrentQ15);
    uint32_t magnitudeQ15 = IdMap_Q15_integerSquareRoot(squaredMagnitude);

    if (magnitudeQ15 > 32767u)
    {
        magnitudeQ15 = 32767u;
    }

    return (Ifx_Math_Fract16)magnitudeQ15;
}

static Ifx_Math_Fract16 IdMap_Q15_limitDqMagnitude(Ifx_Math_Fract16 idReferenceQ15,
                                                    Ifx_Math_Fract16 iqReferenceQ15,
                                                    uint8_t *limited)
{
    const int32_t directCurrentQ15 = (int32_t)idReferenceQ15;
    const int32_t quadratureCurrentQ15 = (int32_t)iqReferenceQ15;
    const uint32_t maximumCurrentQ15 = 32767u;
    const uint32_t quadratureMagnitudeQ15 = (quadratureCurrentQ15 < 0)
        ? (uint32_t)(-quadratureCurrentQ15) : (uint32_t)quadratureCurrentQ15;
    const uint32_t quadratureSquare = quadratureMagnitudeQ15 * quadratureMagnitudeQ15;
    const uint32_t maximumSquare = maximumCurrentQ15 * maximumCurrentQ15;
    uint32_t directLimitQ15;

    if (quadratureSquare >= maximumSquare)
    {
        *limited = 1u;
        return 0;
    }

    directLimitQ15 = IdMap_Q15_integerSquareRoot(maximumSquare - quadratureSquare);
    if (directCurrentQ15 > (int32_t)directLimitQ15)
    {
        *limited = 1u;
        return (Ifx_Math_Fract16)directLimitQ15;
    }

    if (directCurrentQ15 < -(int32_t)directLimitQ15)
    {
        *limited = 1u;
        return (Ifx_Math_Fract16)-(int32_t)directLimitQ15;
    }

    *limited = 0u;
    return idReferenceQ15;
}

static void IdMap_Q15_clearMeasurements(void)
{
    Meas_IdMap_Stat_u8 = ID_MAP_Q15_STATUS_INACTIVE;
    Meas_IdMap_Valid_u8 = 0u;
    Meas_IdMap_Spd_rpm_f32 = 0.0F;
    Meas_IdMap_Iq_A_f32 = 0.0F;
    Meas_IdMap_Id_A_f32 = 0.0F;
    Meas_IdMap_Id_Q15_s16 = 0;
    Meas_IdMap_IsFbkFlt_Q15_s16 = 0;
    idMapIsFbkFilteredQ15 = 0.0F;
    idMapIsFbkFilterInitialized = 0u;
}

void IdMap_Q15_initialize(void)
{
    idMapActive = 0u;
    IdMap_Q15_clearMeasurements();
}

uint8_t IdMap_Q15_computeReference(const Ifx_Math_Fract16 speedQ15,
                                   const Ifx_Math_Fract16 iqReferenceQ15,
                                   Ifx_Math_Fract16 *idReferenceQ15)
{
    const float speedRpm = Ifx_Math_ConvSat_F16ToFlt32(speedQ15, Ifx_Math_FractQFormat_q15)
        * (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM;
    const float iqA = Ifx_Math_ConvSat_F16ToFlt32(iqReferenceQ15, Ifx_Math_FractQFormat_q15)
        * (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_CURRENT_A;
    Ifx_Math_Fract16 filteredSpeedQ15;
    Ifx_Math_Fract16 filteredIqQ15;
    uint8_t limited;

    if (idReferenceQ15 == NULL)
    {
        Meas_IdMap_Stat_u8 = ID_MAP_Q15_STATUS_INVALID_INPUT;
        Meas_IdMap_Valid_u8 = 0u;
        return 0u;
    }

    /* Smooth the two lookup coordinates at the speed-loop rate. The
     * current-vector limiter continues to use the unfiltered Iq command. */
    IdMap_Q15_filterInputs(speedRpm, iqA, &filteredSpeedQ15, &filteredIqQ15);
    *idReferenceQ15 = IdMap_Q15_lookup(filteredSpeedQ15, filteredIqQ15);
    *idReferenceQ15 = IdMap_Q15_limitDqMagnitude(*idReferenceQ15, iqReferenceQ15, &limited);

    Meas_IdMap_Id_A_f32 = (float)*idReferenceQ15
        * (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_CURRENT_A / 32768.0F;
    Meas_IdMap_Id_Q15_s16 = *idReferenceQ15;
    Meas_IdMap_Stat_u8 = (limited != 0u) ? ID_MAP_Q15_STATUS_VECTOR_LIMITED : ID_MAP_Q15_STATUS_VALID;
    Meas_IdMap_Valid_u8 = 1u;

    return 1u;
}

void IdMap_Q15_updateIsFeedbackMeasurement(
    const Ifx_Math_CmpFract16 *feedbackDq)
{
    float rawMagnitudeQ15;

    if ((idMapActive == 0u)
        || (Meas_IdMap_Valid_u8 == 0u)
        || (feedbackDq == NULL))
    {
        Meas_IdMap_IsFbkFlt_Q15_s16 = 0;
        idMapIsFbkFilteredQ15 = 0.0F;
        idMapIsFbkFilterInitialized = 0u;
        return;
    }

    rawMagnitudeQ15 = (float)IdMap_Q15_calculateDqMagnitude(feedbackDq);
    if (idMapIsFbkFilterInitialized == 0u)
    {
        idMapIsFbkFilteredQ15 = rawMagnitudeQ15;
        idMapIsFbkFilterInitialized = 1u;
    }
    else
    {
        idMapIsFbkFilteredQ15 = IdMap_Q15_lowPassSample(rawMagnitudeQ15,
            idMapIsFbkFilteredQ15, Cal_IdMap_IsFltAlpha_Q15_u16);
    }

    Meas_IdMap_IsFbkFlt_Q15_s16 =
        (Ifx_Math_Fract16)(idMapIsFbkFilteredQ15 + 0.5F);
}

void IdMap_Q15_setActive(const uint8_t active)
{
    if (active == 0u)
    {
        if (idMapActive != 0u)
        {
            IdMap_Q15_clearMeasurements();
        }
        idMapActive = 0u;
        Meas_IdMap_Act_u8 = 0u;
    }
    else
    {
        idMapActive = 1u;
        Meas_IdMap_Act_u8 = 1u;
    }
}
