#include "../ConfigWizard/FocTiming_Cfg.h"
#if FOC_AUX_ALGORITHMS_ENABLE
#include "apsfsm_torque_compensation_adapter.h"

#include <float.h>
#include <math.h>
#include <stddef.h>

#include "../ConfigWizard/Ifx_MS_FocSolutionF16_Cfg.h"
#include "../Utilities/no_opt.h"

#if (IFX_MS_FOCSOLUTIONF16_CFG_SPEED_LOOP_PERIOD_US != 500u)
#error "APSFSM requires a 500 us speed-loop sample period"
#endif

#if (IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM != 10000u)
#error "APSFSM Q15 scaling requires a 10000 rpm mechanical base speed"
#endif

#define APSFSM_TS_S                  (0.0005F)
#define APSFSM_BASE_OMEGA_RADPS      (1047.197551196597746F)
#define APSFSM_TWO_PI                (6.283185307179586477F)
#define APSFSM_Q15_SCALE             (32768.0F)
#define APSFSM_Q15_MAX               (32767.0F)
#define APSFSM_Q15_MIN               (-32768.0F)
#define APSFSM_TICKS_PER_MS          (2u)
#define APSFSM_CALIBRATION_UNKNOWN    (0u)
#define APSFSM_CALIBRATION_VALID      (1u)
#define APSFSM_CALIBRATION_INVALID    (2u)

typedef struct
{
    float bHatPU;
    float cHatPU;
    float thetaMech_rad;
    float covariance;
    uint32_t settleTicks;
    uint32_t rampTicks;
    uint8_t previousMode;
    uint8_t initialized;
} APSFSM_TorqueCompState;

static APSFSM_TorqueCompState apsfsmTorqueCompState;
/* Last calibration-validation result. Keep this separate from the dynamic
 * observer state so numerical/control resets do not turn a persistent bad
 * calibration into a 2 kHz reset loop. UNKNOWN distinguishes a completed
 * public/mode reset from an invalid-to-valid recovery edge. */
static uint8_t apsfsmCalibrationState;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Cal_APSFSM_Sel_u8 = APSFSM_TORQUE_COMP_MODE_OFF;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Cal_APSFSM_Rst_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_APSFSM_KHat_f32 = 0.25F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_APSFSM_Rho_rad_f32 = -1.570796326794896619F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_APSFSM_Lambda_f32 = 0.98F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_APSFSM_IqHi_Q15_s16 = 1638;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_APSFSM_IqLo_Q15_s16 = -1638;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Cal_APSFSM_SpdLo_rpm_u16 = 800u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Cal_APSFSM_SpdHi_rpm_u16 = 4000u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Cal_APSFSM_Settle_ms_u16 = 500u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Cal_APSFSM_Ramp_ms_u16 = 500u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_APSFSM_Act_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_APSFSM_OutAct_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_APSFSM_Valid_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_APSFSM_Stat_u8 = APSFSM_TORQUE_COMP_STATUS_IDLE;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_APSFSM_Clipped_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_APSFSM_IqRaw_PU_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Meas_APSFSM_IqOut_Q15_s16 = 0;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_APSFSM_BHat_PU_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_APSFSM_CHat_PU_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_APSFSM_Theta_rad_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_APSFSM_SpdErr_PU_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_APSFSM_Cov_f32 = 0.0F;

static uint8_t APSFSM_TorqueComp_isFinite(const float value)
{
    return ((value == value) && (value <= FLT_MAX) && (value >= -FLT_MAX)) ? 1u : 0u;
}

static float APSFSM_TorqueComp_q15ToPU(const Ifx_Math_Fract16 value)
{
    return (float)value / APSFSM_Q15_SCALE;
}

static Ifx_Math_Fract16 APSFSM_TorqueComp_puToQ15(const float value)
{
    float scaledValue;

    if (APSFSM_TorqueComp_isFinite(value) == 0u)
    {
        return (Ifx_Math_Fract16)0;
    }

    scaledValue = value * APSFSM_Q15_SCALE;

    if (scaledValue >= APSFSM_Q15_MAX)
    {
        return (Ifx_Math_Fract16)32767;
    }

    if (scaledValue <= APSFSM_Q15_MIN)
    {
        return (Ifx_Math_Fract16)-32768;
    }

    scaledValue += (scaledValue >= 0.0F) ? 0.5F : -0.5F;
    return (Ifx_Math_Fract16)((int32_t)scaledValue);
}

static Ifx_Math_Fract16 APSFSM_TorqueComp_rpmToQ15(const uint16_t speedRpm)
{
    uint32_t scaledSpeed = ((uint32_t)speedRpm * 32768u)
        + ((uint32_t)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM / 2u);

    scaledSpeed /= (uint32_t)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM;
    if (scaledSpeed > 32767u)
    {
        scaledSpeed = 32767u;
    }

    return (Ifx_Math_Fract16)scaledSpeed;
}

static float APSFSM_TorqueComp_clamp(const float value,
                                     const float lower,
                                     const float upper,
                                     uint8_t *clipped)
{
    if (value > upper)
    {
        *clipped = 1u;
        return upper;
    }

    if (value < lower)
    {
        *clipped = 1u;
        return lower;
    }

    return value;
}

static float APSFSM_TorqueComp_wrapAngle(const float angle_rad)
{
    float wrapped = fmodf(angle_rad, APSFSM_TWO_PI);

    if (wrapped < 0.0F)
    {
        wrapped += APSFSM_TWO_PI;
    }

    if (wrapped >= APSFSM_TWO_PI)
    {
        wrapped = 0.0F;
    }

    return wrapped;
}

static uint32_t APSFSM_TorqueComp_integerSquareRoot(uint32_t value)
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

static float APSFSM_TorqueComp_getQCircleLimitPU(
    const Ifx_Math_Fract16 directCurrentQ15)
{
    int32_t directMagnitudeQ15 = (int32_t)directCurrentQ15;
    const uint32_t maximumCurrentQ15 = 32767u;
    uint32_t directMagnitudeSquared;
    uint32_t maximumMagnitudeSquared;

    if (directMagnitudeQ15 < 0)
    {
        directMagnitudeQ15 = -directMagnitudeQ15;
    }

    if ((uint32_t)directMagnitudeQ15 >= maximumCurrentQ15)
    {
        return 0.0F;
    }

    directMagnitudeSquared = (uint32_t)directMagnitudeQ15
        * (uint32_t)directMagnitudeQ15;
    maximumMagnitudeSquared = maximumCurrentQ15 * maximumCurrentQ15;

    return (float)APSFSM_TorqueComp_integerSquareRoot(
        maximumMagnitudeSquared - directMagnitudeSquared) / APSFSM_Q15_SCALE;
}

static uint8_t APSFSM_TorqueComp_validateCalibration(
    const APSFSM_TorqueCompCalibration *calibration)
{
    float convergenceReal;
    float omegaHigh_radps;
    float convergenceMagnitudeSquared;
    float covarianceMinimum;

    if (calibration == NULL)
    {
        return 0u;
    }

    if ((calibration->selector > APSFSM_TORQUE_COMP_MODE_APPLY)
        || (APSFSM_TorqueComp_isFinite(calibration->kHat) == 0u)
        || (APSFSM_TorqueComp_isFinite(calibration->rho_rad) == 0u)
        || (APSFSM_TorqueComp_isFinite(calibration->lambda) == 0u))
    {
        return 0u;
    }

    if ((calibration->kHat <= 0.0F)
        || (calibration->lambda <= 0.0F)
        || (calibration->lambda >= 1.0F)
        || (calibration->iqLowerLimitQ15 >= 0)
        || (calibration->iqUpperLimitQ15 <= 0)
        || (calibration->iqLowerLimitQ15 >= calibration->iqUpperLimitQ15)
        || (calibration->iqLowerLimitQ15
            < (Ifx_Math_Fract16)IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_OUT_LOW_LIMIT_Q)
        || (calibration->iqUpperLimitQ15
            > (Ifx_Math_Fract16)IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_OUT_UPP_LIMIT_Q)
        || (calibration->speedLowerLimit_rpm == 0u)
        || (calibration->speedLowerLimit_rpm >= calibration->speedUpperLimit_rpm)
        || (calibration->speedUpperLimit_rpm
            > (uint16_t)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM)
        || (calibration->settleTime_ms == 0u)
        || (calibration->rampTime_ms == 0u))
    {
        return 0u;
    }

    covarianceMinimum = 0.5F * calibration->kHat * calibration->kHat;
    omegaHigh_radps = (float)calibration->speedUpperLimit_rpm
        * (APSFSM_TWO_PI / 60.0F);
    convergenceReal = (2.0F * calibration->lambda) - 1.0F;
    convergenceMagnitudeSquared = (convergenceReal * convergenceReal)
        + ((omegaHigh_radps * APSFSM_TS_S) * (omegaHigh_radps * APSFSM_TS_S));

    if ((APSFSM_TorqueComp_isFinite(covarianceMinimum) == 0u)
        || (covarianceMinimum <= 0.0F)
        || (APSFSM_TorqueComp_isFinite(convergenceMagnitudeSquared) == 0u)
        || (convergenceMagnitudeSquared >= 1.0F))
    {
        return 0u;
    }

    return 1u;
}

static void APSFSM_TorqueComp_clearMeasurements(const uint8_t status,
                                                 const Ifx_Math_Fract16 iqOutputQ15)
{
    Meas_APSFSM_Act_u8 = 0u;
    Meas_APSFSM_OutAct_u8 = 0u;
    Meas_APSFSM_Valid_u8 = 0u;
    Meas_APSFSM_Stat_u8 = status;
    Meas_APSFSM_Clipped_u8 = 0u;
    Meas_APSFSM_IqRaw_PU_f32 = 0.0F;
    Meas_APSFSM_IqOut_Q15_s16 = iqOutputQ15;
    Meas_APSFSM_BHat_PU_f32 = 0.0F;
    Meas_APSFSM_CHat_PU_f32 = 0.0F;
    Meas_APSFSM_Theta_rad_f32 = 0.0F;
    Meas_APSFSM_SpdErr_PU_f32 = 0.0F;
    Meas_APSFSM_Cov_f32 = 0.0F;
}

static void APSFSM_TorqueComp_setDiagnostics(
    APSFSM_TorqueCompDiagnostics *diagnostics,
    const float iqRawPU,
    const float iqAppliedPU,
    const float speedErrorPU,
    const Ifx_Math_Fract16 iqOutputQ15,
    const uint8_t active,
    const uint8_t outputActive,
    const uint8_t valid,
    const uint8_t status,
    const uint8_t clipped)
{
    Meas_APSFSM_Act_u8 = active;
    Meas_APSFSM_OutAct_u8 = outputActive;
    Meas_APSFSM_Valid_u8 = valid;
    Meas_APSFSM_Stat_u8 = status;
    Meas_APSFSM_Clipped_u8 = clipped;
    Meas_APSFSM_IqRaw_PU_f32 = iqRawPU;
    Meas_APSFSM_IqOut_Q15_s16 = iqOutputQ15;
    Meas_APSFSM_BHat_PU_f32 = apsfsmTorqueCompState.bHatPU;
    Meas_APSFSM_CHat_PU_f32 = apsfsmTorqueCompState.cHatPU;
    Meas_APSFSM_Theta_rad_f32 = apsfsmTorqueCompState.thetaMech_rad;
    Meas_APSFSM_SpdErr_PU_f32 = speedErrorPU;
    Meas_APSFSM_Cov_f32 = apsfsmTorqueCompState.covariance;

    if (diagnostics != NULL)
    {
        diagnostics->iqRawPU = iqRawPU;
        diagnostics->iqAppliedPU = iqAppliedPU;
        diagnostics->bHatPU = apsfsmTorqueCompState.bHatPU;
        diagnostics->cHatPU = apsfsmTorqueCompState.cHatPU;
        diagnostics->thetaMech_rad = apsfsmTorqueCompState.thetaMech_rad;
        diagnostics->speedErrorPU = speedErrorPU;
        diagnostics->covariance = apsfsmTorqueCompState.covariance;
        diagnostics->iqOutputQ15 = iqOutputQ15;
        diagnostics->active = active;
        diagnostics->outputActive = outputActive;
        diagnostics->valid = valid;
        diagnostics->status = status;
        diagnostics->clipped = clipped;
    }
}

static void APSFSM_TorqueComp_setInactiveDiagnostics(
    APSFSM_TorqueCompDiagnostics *diagnostics,
    const Ifx_Math_Fract16 iqOutputQ15,
    const uint8_t status)
{
    APSFSM_TorqueComp_clearMeasurements(status, iqOutputQ15);

    if (diagnostics != NULL)
    {
        diagnostics->iqRawPU = 0.0F;
        diagnostics->iqAppliedPU = 0.0F;
        diagnostics->bHatPU = 0.0F;
        diagnostics->cHatPU = 0.0F;
        diagnostics->thetaMech_rad = 0.0F;
        diagnostics->speedErrorPU = 0.0F;
        diagnostics->covariance = 0.0F;
        diagnostics->iqOutputQ15 = iqOutputQ15;
        diagnostics->active = 0u;
        diagnostics->outputActive = 0u;
        diagnostics->valid = 0u;
        diagnostics->status = status;
        diagnostics->clipped = 0u;
    }
}

static void APSFSM_TorqueComp_resetState(void)
{
    apsfsmTorqueCompState.bHatPU = 0.0F;
    apsfsmTorqueCompState.cHatPU = 0.0F;
    apsfsmTorqueCompState.thetaMech_rad = 0.0F;
    apsfsmTorqueCompState.covariance = 0.0F;
    apsfsmTorqueCompState.settleTicks = 0u;
    apsfsmTorqueCompState.rampTicks = 0u;
    apsfsmTorqueCompState.previousMode = APSFSM_TORQUE_COMP_MODE_OFF;
    apsfsmTorqueCompState.initialized = 0u;
}

static void APSFSM_TorqueComp_projectCoefficients(float *bHatPU,
                                                   float *cHatPU,
                                                   const float radiusPU)
{
    const float magnitudeSquared = (*bHatPU * *bHatPU) + (*cHatPU * *cHatPU);
    const float radiusSquared = radiusPU * radiusPU;

    if ((radiusPU <= 0.0F) && (magnitudeSquared > 0.0F))
    {
        *bHatPU = 0.0F;
        *cHatPU = 0.0F;
    }
    else if (magnitudeSquared > radiusSquared)
    {
        const float scale = radiusPU / sqrtf(magnitudeSquared);
        *bHatPU *= scale;
        *cHatPU *= scale;
    }
    else
    {
        /* No projection is required. */
    }
}

void APSFSM_TorqueComp_initialize(void)
{
    APSFSM_TorqueComp_reset();
}

void APSFSM_TorqueComp_reset(void)
{
    APSFSM_TorqueComp_resetState();
    apsfsmCalibrationState = APSFSM_CALIBRATION_UNKNOWN;
    APSFSM_TorqueComp_clearMeasurements(APSFSM_TORQUE_COMP_STATUS_IDLE, 0);
}

uint8_t APSFSM_TorqueComp_execute(
    const Ifx_Math_Fract16 referenceSpeedQ15,
    const Ifx_Math_Fract16 measuredSpeedQ15,
    const Ifx_Math_CmpFract16 *baseDqQ15,
    const uint8_t controlEligible,
    const APSFSM_TorqueCompCalibration *calibration,
    Ifx_Math_CmpFract16 *compensatedDqQ15,
    APSFSM_TorqueCompDiagnostics *diagnostics)
{
    float referenceSpeedPU;
    float measuredSpeedPU;
    float speedErrorPU;
    float iqRawPU;
    float iqBoundedPU;
    float iqAppliedPU = 0.0F;
    float baseDPU;
    float baseQPU;
    float outputQPU;
    float qSystemLowerPU;
    float qSystemUpperPU;
    float qCircleLimitPU;
    float thetaNext_rad;
    float covarianceMinimum;
    float covarianceCandidate;
    float bCandidatePU;
    float cCandidatePU;
    float coefficientRadiusPU;
    float rampGain;
    float adaptationAngle_rad;
    uint32_t settleTicksRequired;
    uint32_t rampTicksRequired;
    Ifx_Math_Fract16 speedLowerLimitQ15;
    Ifx_Math_Fract16 speedUpperLimitQ15;
    uint8_t clipped = 0u;
    uint8_t externalClipped = 0u;
    uint8_t temporaryClipped = 0u;
    uint8_t outputActive;
    uint8_t status;
    Ifx_Math_CmpFract16 baseCommandQ15;

    if ((baseDqQ15 == NULL) || (compensatedDqQ15 == NULL))
    {
        APSFSM_TorqueComp_resetState();
        APSFSM_TorqueComp_setInactiveDiagnostics(diagnostics, 0,
            APSFSM_TORQUE_COMP_STATUS_NUMERICAL_INVALID);
        return 0u;
    }

    baseCommandQ15 = *baseDqQ15;
    *compensatedDqQ15 = baseCommandQ15;

    if (calibration == NULL)
    {
        if (apsfsmCalibrationState == APSFSM_CALIBRATION_VALID)
        {
            APSFSM_TorqueComp_resetState();
        }
        apsfsmCalibrationState = APSFSM_CALIBRATION_INVALID;
        APSFSM_TorqueComp_setInactiveDiagnostics(diagnostics, baseCommandQ15.imag,
            APSFSM_TORQUE_COMP_STATUS_PARAMETER_INVALID);
        return 0u;
    }

    if ((calibration->selector == APSFSM_TORQUE_COMP_MODE_OFF)
        || (calibration->reset != 0u))
    {
        APSFSM_TorqueComp_resetState();
        apsfsmCalibrationState = APSFSM_CALIBRATION_UNKNOWN;
        APSFSM_TorqueComp_setInactiveDiagnostics(diagnostics, baseCommandQ15.imag,
            APSFSM_TORQUE_COMP_STATUS_IDLE);
        return 0u;
    }

    if (APSFSM_TorqueComp_validateCalibration(calibration) == 0u)
    {
        if (apsfsmCalibrationState == APSFSM_CALIBRATION_VALID)
        {
            APSFSM_TorqueComp_resetState();
        }
        apsfsmCalibrationState = APSFSM_CALIBRATION_INVALID;
        APSFSM_TorqueComp_setInactiveDiagnostics(diagnostics, baseCommandQ15.imag,
            APSFSM_TORQUE_COMP_STATUS_PARAMETER_INVALID);
        return 0u;
    }

    if (apsfsmCalibrationState == APSFSM_CALIBRATION_INVALID)
    {
        /* A repaired calibration starts from a clean observer state once.
         * Subsequent valid samples retain the accumulated state. */
        APSFSM_TorqueComp_resetState();
    }
    apsfsmCalibrationState = APSFSM_CALIBRATION_VALID;

    referenceSpeedPU = APSFSM_TorqueComp_q15ToPU(referenceSpeedQ15);
    measuredSpeedPU = APSFSM_TorqueComp_q15ToPU(measuredSpeedQ15);
    baseDPU = APSFSM_TorqueComp_q15ToPU(baseCommandQ15.real);
    baseQPU = APSFSM_TorqueComp_q15ToPU(baseCommandQ15.imag);
    speedLowerLimitQ15 = APSFSM_TorqueComp_rpmToQ15(
        calibration->speedLowerLimit_rpm);
    speedUpperLimitQ15 = APSFSM_TorqueComp_rpmToQ15(
        calibration->speedUpperLimit_rpm);

    if (apsfsmTorqueCompState.initialized == 0u)
    {
        /* The source initial condition is c[0] = KHat^2 / 2. The adapter
         * cannot establish it in reset() because KHat is a live calibration. */
        apsfsmTorqueCompState.covariance = 0.5F * calibration->kHat
            * calibration->kHat;
        apsfsmTorqueCompState.initialized = 1u;
    }

    if ((APSFSM_TorqueComp_isFinite(referenceSpeedPU) == 0u)
        || (APSFSM_TorqueComp_isFinite(measuredSpeedPU) == 0u)
        || (APSFSM_TorqueComp_isFinite(baseDPU) == 0u)
        || (APSFSM_TorqueComp_isFinite(baseQPU) == 0u))
    {
        APSFSM_TorqueComp_resetState();
        APSFSM_TorqueComp_setInactiveDiagnostics(diagnostics, baseCommandQ15.imag,
            APSFSM_TORQUE_COMP_STATUS_NUMERICAL_INVALID);
        return 0u;
    }

    if ((controlEligible == 0u)
        || (referenceSpeedQ15 < speedLowerLimitQ15)
        || (referenceSpeedQ15 > speedUpperLimitQ15)
        || (measuredSpeedQ15 < speedLowerLimitQ15)
        || (measuredSpeedQ15 > speedUpperLimitQ15))
    {
        const uint8_t inactiveStatus = (controlEligible == 0u)
            ? APSFSM_TORQUE_COMP_STATUS_IDLE : APSFSM_TORQUE_COMP_STATUS_WAIT_SPEED;

        APSFSM_TorqueComp_resetState();
        APSFSM_TorqueComp_setInactiveDiagnostics(diagnostics, baseCommandQ15.imag,
            inactiveStatus);
        return 0u;
    }

    settleTicksRequired = (uint32_t)calibration->settleTime_ms * APSFSM_TICKS_PER_MS;
    if (apsfsmTorqueCompState.settleTicks < settleTicksRequired)
    {
        apsfsmTorqueCompState.settleTicks++;
    }

    if (apsfsmTorqueCompState.settleTicks < settleTicksRequired)
    {
        apsfsmTorqueCompState.previousMode = calibration->selector;
        APSFSM_TorqueComp_setDiagnostics(diagnostics, 0.0F, 0.0F, 0.0F,
            baseCommandQ15.imag, 0u, 0u, 0u,
            APSFSM_TORQUE_COMP_STATUS_WAIT_SPEED, 0u);
        return 0u;
    }

    speedErrorPU = referenceSpeedPU - measuredSpeedPU;

    /* The source S-function publishes with B[k], C[k], and theta[k]. */
    iqRawPU = (apsfsmTorqueCompState.bHatPU * sinf(apsfsmTorqueCompState.thetaMech_rad))
        + (apsfsmTorqueCompState.cHatPU * cosf(apsfsmTorqueCompState.thetaMech_rad));

    if ((APSFSM_TorqueComp_isFinite(speedErrorPU) == 0u)
        || (APSFSM_TorqueComp_isFinite(iqRawPU) == 0u))
    {
        APSFSM_TorqueComp_resetState();
        APSFSM_TorqueComp_setInactiveDiagnostics(diagnostics, baseCommandQ15.imag,
            APSFSM_TORQUE_COMP_STATUS_NUMERICAL_INVALID);
        return 0u;
    }

    iqBoundedPU = APSFSM_TorqueComp_clamp(iqRawPU,
        APSFSM_TorqueComp_q15ToPU(calibration->iqLowerLimitQ15),
        APSFSM_TorqueComp_q15ToPU(calibration->iqUpperLimitQ15), &clipped);
    outputQPU = baseQPU;
    outputActive = (calibration->selector == APSFSM_TORQUE_COMP_MODE_APPLY) ? 1u : 0u;

    if (outputActive != 0u)
    {
        rampTicksRequired = (uint32_t)calibration->rampTime_ms * APSFSM_TICKS_PER_MS;
        if (apsfsmTorqueCompState.previousMode != APSFSM_TORQUE_COMP_MODE_APPLY)
        {
            apsfsmTorqueCompState.rampTicks = 0u;
        }

        if (apsfsmTorqueCompState.rampTicks < rampTicksRequired)
        {
            apsfsmTorqueCompState.rampTicks++;
        }

        rampGain = (float)apsfsmTorqueCompState.rampTicks / (float)rampTicksRequired;
        if (rampGain > 1.0F)
        {
            rampGain = 1.0F;
        }

        iqAppliedPU = iqBoundedPU * rampGain;
        outputQPU = baseQPU + iqAppliedPU;
        qSystemLowerPU = (float)IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_OUT_LOW_LIMIT_Q
            / APSFSM_Q15_SCALE;
        qSystemUpperPU = (float)IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_OUT_UPP_LIMIT_Q
            / APSFSM_Q15_SCALE;
        temporaryClipped = 0u;
        outputQPU = APSFSM_TorqueComp_clamp(outputQPU, qSystemLowerPU,
            qSystemUpperPU, &temporaryClipped);
        if (temporaryClipped != 0u)
        {
            clipped = 1u;
            externalClipped = 1u;
        }

        temporaryClipped = 0u;
        qCircleLimitPU = APSFSM_TorqueComp_getQCircleLimitPU(baseCommandQ15.real);

        outputQPU = APSFSM_TorqueComp_clamp(outputQPU, -qCircleLimitPU,
            qCircleLimitPU, &temporaryClipped);
        if (temporaryClipped != 0u)
        {
            clipped = 1u;
            externalClipped = 1u;
        }

        iqAppliedPU = outputQPU - baseQPU;
        compensatedDqQ15->imag = APSFSM_TorqueComp_puToQ15(outputQPU);
    }
    else
    {
        apsfsmTorqueCompState.rampTicks = 0u;
    }

    /* The state update starts only after the current sample's output and
     * clipping decision are known. Theta always advances on an active sample. */
    thetaNext_rad = APSFSM_TorqueComp_wrapAngle(apsfsmTorqueCompState.thetaMech_rad
        + (APSFSM_TS_S * APSFSM_BASE_OMEGA_RADPS * measuredSpeedPU));
    covarianceMinimum = 0.5F * calibration->kHat * calibration->kHat;
    covarianceCandidate = (calibration->lambda * apsfsmTorqueCompState.covariance)
        + covarianceMinimum;
    if (covarianceCandidate < covarianceMinimum)
    {
        covarianceCandidate = covarianceMinimum;
    }

    adaptationAngle_rad = APSFSM_TorqueComp_wrapAngle(thetaNext_rad
        + calibration->rho_rad);
    bCandidatePU = apsfsmTorqueCompState.bHatPU
        + (calibration->kHat * sinf(adaptationAngle_rad) * speedErrorPU
            / covarianceCandidate);
    cCandidatePU = apsfsmTorqueCompState.cHatPU
        + (calibration->kHat * cosf(adaptationAngle_rad) * speedErrorPU
            / covarianceCandidate);

    coefficientRadiusPU = APSFSM_TorqueComp_q15ToPU(calibration->iqUpperLimitQ15);
    if (-APSFSM_TorqueComp_q15ToPU(calibration->iqLowerLimitQ15)
        < coefficientRadiusPU)
    {
        coefficientRadiusPU = -APSFSM_TorqueComp_q15ToPU(calibration->iqLowerLimitQ15);
    }
    APSFSM_TorqueComp_projectCoefficients(&bCandidatePU, &cCandidatePU,
        coefficientRadiusPU);

    if ((APSFSM_TorqueComp_isFinite(thetaNext_rad) == 0u)
        || (APSFSM_TorqueComp_isFinite(covarianceCandidate) == 0u)
        || (APSFSM_TorqueComp_isFinite(bCandidatePU) == 0u)
        || (APSFSM_TorqueComp_isFinite(cCandidatePU) == 0u)
        || (APSFSM_TorqueComp_isFinite(outputQPU) == 0u)
        || (APSFSM_TorqueComp_isFinite(iqAppliedPU) == 0u))
    {
        *compensatedDqQ15 = baseCommandQ15;
        APSFSM_TorqueComp_resetState();
        APSFSM_TorqueComp_setInactiveDiagnostics(diagnostics, baseCommandQ15.imag,
            APSFSM_TORQUE_COMP_STATUS_NUMERICAL_INVALID);
        return 0u;
    }

    apsfsmTorqueCompState.thetaMech_rad = thetaNext_rad;
    if (externalClipped == 0u)
    {
        apsfsmTorqueCompState.bHatPU = bCandidatePU;
        apsfsmTorqueCompState.cHatPU = cCandidatePU;
        apsfsmTorqueCompState.covariance = covarianceCandidate;
    }

    apsfsmTorqueCompState.previousMode = calibration->selector;
    status = (outputActive != 0u) ? APSFSM_TORQUE_COMP_STATUS_APPLY_VALID
        : APSFSM_TORQUE_COMP_STATUS_SHADOW_VALID;
    APSFSM_TorqueComp_setDiagnostics(diagnostics, iqRawPU, iqAppliedPU,
        speedErrorPU, compensatedDqQ15->imag, 1u, outputActive, 1u, status, clipped);

    return 1u;
}

#endif /* FOC_AUX_ALGORITHMS_ENABLE */
