#include "hfo_external_observer_adapter.h"

#include <float.h>
#include <math.h>

#include "../ConfigWizard/Ifx_MDA_FluxEstimatorF16_Cfg.h"
#include "../ConfigWizard/Ifx_MS_FocSolutionF16_Cfg.h"
#include "../Math/include/Ifx_Math_Atan2.h"
#include "../Math/include/Ifx_Math_SinCos.h"
#include "../Utilities/no_opt.h"

#if defined(__ICCARM__)
#define HFO_XCP_SECTION _Pragma("location=\".xcp_cal_m4\"")
#else
#define HFO_XCP_SECTION
#endif

#define HFO_TWO_PI_RAD        (6.2831853071795864769F)
#define HFO_Q32_PER_CYCLE     (4294967296.0F)
#define HFO_Q15_SCALE         (32768.0F)
#define HFO_MILLI_TO_BASE     (0.001F)
#define HFO_MICRO_TO_BASE     (0.000001F)
#define HFO_US_TO_SECONDS     (0.000001F)
#define HFO_MIN_ACTIVE_FLUX_WB (0.0000001F)

/* Reuse the existing flux observer's alpha and speed time constants as the
 * default correction and speed-filter bandwidths. */
#define HFO_DEFAULT_KOB_RADPS \
    (1.0F / ((float)IFX_MDA_FLUXESTIMATORF16_CFG_ALPHA_TC_US * HFO_US_TO_SECONDS))
#define HFO_DEFAULT_SPEED_FILTER_HZ \
    (1.0F / (HFO_TWO_PI_RAD * (float)IFX_MDA_FLUXESTIMATORF16_CFG_SPEED_TC_US * HFO_US_TO_SECONDS))

/* Existing project calibration values. They intentionally match the KRE
 * motor-parameter source so an XCP motor update resets both observers. */
extern volatile uint16_t Cal_MotorAppliedPhaseResistance_mOhm_u16;
extern volatile uint16_t Cal_MotorAppliedDirectInductance_uH_u16;
extern volatile uint16_t Cal_MotorAppliedQuadratureInductance_uH_u16;
extern volatile uint8_t Cal_MotorAppliedPolePairs_u8;
extern volatile uint16_t Cal_MTPA_AppliedPermanentMagnetFlux_mWb_u16;

HFO_XCP_SECTION
NO_OPT volatile float Cal_Hfo_Kob_radps_f32 = HFO_DEFAULT_KOB_RADPS;
HFO_XCP_SECTION
NO_OPT volatile float Cal_Hfo_Kinj_f32 = 0.1F;
HFO_XCP_SECTION
NO_OPT volatile float Cal_Hfo_SpeedFilter_Hz_f32 = HFO_DEFAULT_SPEED_FILTER_HZ;

HFO_XCP_SECTION
NO_OPT volatile float Meas_Hfo_ElecAngle_rad_f32 = 0.0F;
HFO_XCP_SECTION
NO_OPT volatile float Meas_Hfo_MechSpeed_rpm_f32 = 0.0F;
/* Detailed virtual-flux diagnostics remain global and non-optimized, but do
 * not consume the fixed 0x100-byte XCP calibration page. */
NO_OPT volatile float Meas_Hfo_Id_A_f32 = 0.0F;
NO_OPT volatile float Meas_Hfo_Iq_A_f32 = 0.0F;
NO_OPT volatile float Meas_Hfo_PsiDInjection_Wb_f32 = 0.0F;
NO_OPT volatile float Meas_Hfo_PsiQInjection_Wb_f32 = 0.0F;
NO_OPT volatile float Meas_Hfo_PsiCurrentAlpha_Wb_f32 = 0.0F;
NO_OPT volatile float Meas_Hfo_PsiCurrentBeta_Wb_f32 = 0.0F;
NO_OPT volatile float Meas_Hfo_PsiVoltageAlpha_Wb_f32 = 0.0F;
NO_OPT volatile float Meas_Hfo_PsiVoltageBeta_Wb_f32 = 0.0F;
NO_OPT volatile float Meas_Hfo_PsiActiveAlpha_Wb_f32 = 0.0F;
NO_OPT volatile float Meas_Hfo_PsiActiveBeta_Wb_f32 = 0.0F;
NO_OPT volatile float Meas_Hfo_ActiveFluxMagnitude_Wb_f32 = 0.0F;
HFO_XCP_SECTION
NO_OPT volatile uint8_t Meas_Hfo_Valid_u8 = 0u;
HFO_XCP_SECTION
NO_OPT volatile uint8_t Meas_Hfo_Status_u8 = HFO_EXTERNAL_OBSERVER_STATUS_IDLE;

static float hfoPsiVoltageAlpha_Wb;
static float hfoPsiVoltageBeta_Wb;
static float hfoFilteredMechanicalSpeed_rpm;
static float hfoSpeedFilterGain;
static float hfoLastSpeedFilterHz;
static uint32_t hfoPreviousAngleIndex;
static uint8_t hfoHasValidAngle;

static uint8_t HfoExternalObserver_isFinite(const float value)
{
    return ((value == value) && (value <= FLT_MAX) && (value >= -FLT_MAX)) ? 1u : 0u;
}

static float HfoExternalObserver_absolute(const float value)
{
    return (value < 0.0F) ? -value : value;
}

static void HfoExternalObserver_clearMeasurements(void)
{
    Meas_Hfo_ElecAngle_rad_f32 = 0.0F;
    Meas_Hfo_MechSpeed_rpm_f32 = 0.0F;
    Meas_Hfo_Id_A_f32 = 0.0F;
    Meas_Hfo_Iq_A_f32 = 0.0F;
    Meas_Hfo_PsiDInjection_Wb_f32 = 0.0F;
    Meas_Hfo_PsiQInjection_Wb_f32 = 0.0F;
    Meas_Hfo_PsiCurrentAlpha_Wb_f32 = 0.0F;
    Meas_Hfo_PsiCurrentBeta_Wb_f32 = 0.0F;
    Meas_Hfo_PsiVoltageAlpha_Wb_f32 = 0.0F;
    Meas_Hfo_PsiVoltageBeta_Wb_f32 = 0.0F;
    Meas_Hfo_PsiActiveAlpha_Wb_f32 = 0.0F;
    Meas_Hfo_PsiActiveBeta_Wb_f32 = 0.0F;
    Meas_Hfo_ActiveFluxMagnitude_Wb_f32 = 0.0F;
    Meas_Hfo_Valid_u8 = 0u;
    Meas_Hfo_Status_u8 = HFO_EXTERNAL_OBSERVER_STATUS_IDLE;
}

static void HfoExternalObserver_clearState(void)
{
    hfoPsiVoltageAlpha_Wb = 0.0F;
    hfoPsiVoltageBeta_Wb = 0.0F;
    hfoFilteredMechanicalSpeed_rpm = 0.0F;
    hfoSpeedFilterGain = 0.0F;
    hfoLastSpeedFilterHz = -1.0F;
    hfoPreviousAngleIndex = 0u;
    hfoHasValidAngle = 0u;
}

static void HfoExternalObserver_invalidate(const uint8_t status)
{
    HfoExternalObserver_clearState();
    HfoExternalObserver_clearMeasurements();
    Meas_Hfo_Status_u8 = status;
}

static uint8_t HfoExternalObserver_parametersAreValid(const float resistance_Ohm,
                                                       const float directInductance_H,
                                                       const float quadratureInductance_H,
                                                       const float permanentMagnetFlux_Wb,
                                                       const float sampleTime_s,
                                                       const uint8_t polePairs,
                                                       const float correctionGain_radps,
                                                       const float injectionGain,
                                                       const float speedFilterHz)
{
    if ((HfoExternalObserver_isFinite(resistance_Ohm) == 0u)
        || (HfoExternalObserver_isFinite(directInductance_H) == 0u)
        || (HfoExternalObserver_isFinite(quadratureInductance_H) == 0u)
        || (HfoExternalObserver_isFinite(permanentMagnetFlux_Wb) == 0u)
        || (HfoExternalObserver_isFinite(sampleTime_s) == 0u)
        || (HfoExternalObserver_isFinite(correctionGain_radps) == 0u)
        || (HfoExternalObserver_isFinite(injectionGain) == 0u)
        || (HfoExternalObserver_isFinite(speedFilterHz) == 0u)
        || (resistance_Ohm < 0.0F)
        || (directInductance_H <= 0.0F)
        || (quadratureInductance_H <= 0.0F)
        || (permanentMagnetFlux_Wb <= 0.0F)
        || (sampleTime_s <= 0.0F)
        || (polePairs == 0u)
        || (correctionGain_radps < 0.0F)
        /* Equation (30) at zero current requires 1 - Kinj > 0. */
        || (injectionGain <= 0.0F)
        || (injectionGain >= 1.0F)
        || (speedFilterHz < 0.0F))
    {
        return 0u;
    }

    return 1u;
}

static void HfoExternalObserver_updateSpeedFilter(const float speedFilterHz,
                                                   const float sampleTime_s)
{
    float exponent;

    if (hfoLastSpeedFilterHz == speedFilterHz)
    {
        return;
    }

    exponent = HFO_TWO_PI_RAD * speedFilterHz * sampleTime_s;
    hfoSpeedFilterGain = 1.0F - expf(-exponent);
    if (hfoSpeedFilterGain < 0.0F)
    {
        hfoSpeedFilterGain = 0.0F;
    }
    else if (hfoSpeedFilterGain > 1.0F)
    {
        hfoSpeedFilterGain = 1.0F;
    }

    hfoLastSpeedFilterHz = speedFilterHz;
}

static Ifx_Math_Fract16 HfoExternalObserver_normalizedToQ15(const float value)
{
    float scaledValue = value * HFO_Q15_SCALE;

    if (scaledValue >= 32767.0F)
    {
        return (Ifx_Math_Fract16)32767;
    }
    if (scaledValue <= -32768.0F)
    {
        return (Ifx_Math_Fract16)-32768;
    }

    return (Ifx_Math_Fract16)scaledValue;
}

static uint32_t HfoExternalObserver_angleFromActiveFlux(const float activeAlpha_Wb,
                                                         const float activeBeta_Wb)
{
    const float alphaAbs = HfoExternalObserver_absolute(activeAlpha_Wb);
    const float betaAbs = HfoExternalObserver_absolute(activeBeta_Wb);
    const float normalization = (alphaAbs > betaAbs) ? alphaAbs : betaAbs;
    const Ifx_Math_Fract16 alphaQ15 = HfoExternalObserver_normalizedToQ15(activeAlpha_Wb / normalization);
    const Ifx_Math_Fract16 betaQ15 = HfoExternalObserver_normalizedToQ15(activeBeta_Wb / normalization);

    return Ifx_Math_Atan2_F16(betaQ15, alphaQ15);
}

void HfoExternalObserver_initialize(void)
{
    HfoExternalObserver_reset();
}

void HfoExternalObserver_reset(void)
{
    HfoExternalObserver_clearState();
    HfoExternalObserver_clearMeasurements();
}

void HfoExternalObserver_execute(const float voltageAlpha_V,
                                 const float voltageBeta_V,
                                 const float currentAlpha_A,
                                 const float currentBeta_A)
{
    const float resistance_Ohm = (float)Cal_MotorAppliedPhaseResistance_mOhm_u16 * HFO_MILLI_TO_BASE;
    const float directInductance_H = (float)Cal_MotorAppliedDirectInductance_uH_u16 * HFO_MICRO_TO_BASE;
    const float quadratureInductance_H =
        (float)Cal_MotorAppliedQuadratureInductance_uH_u16 * HFO_MICRO_TO_BASE;
    const float permanentMagnetFlux_Wb =
        (float)Cal_MTPA_AppliedPermanentMagnetFlux_mWb_u16 * HFO_MILLI_TO_BASE;
    const float sampleTime_s =
        (float)IFX_MS_FOCSOLUTIONF16_CFG_SAMPLING_TIME_US * HFO_US_TO_SECONDS;
    const uint8_t polePairs = Cal_MotorAppliedPolePairs_u8;
    const float correctionGain_radps = Cal_Hfo_Kob_radps_f32;
    const float injectionGain = Cal_Hfo_Kinj_f32;
    const float speedFilterHz = Cal_Hfo_SpeedFilter_Hz_f32;
    Ifx_Math_SinCos_Type sinCos;
    float sine;
    float cosine;
    float directCurrent_A;
    float quadratureCurrent_A;
    float psiCurrentDirect_Wb;
    float psiCurrentQuadrature_Wb;
    float psiCurrentAlpha_Wb;
    float psiCurrentBeta_Wb;
    float psiActiveAlpha_Wb;
    float psiActiveBeta_Wb;
    float activeFluxThreshold_Wb;
    float activeFluxMagnitudeSquared_Wb2;
    uint32_t electricalAngleIndex;
    float rawMechanicalSpeed_rpm;

    if ((HfoExternalObserver_isFinite(voltageAlpha_V) == 0u)
        || (HfoExternalObserver_isFinite(voltageBeta_V) == 0u)
        || (HfoExternalObserver_isFinite(currentAlpha_A) == 0u)
        || (HfoExternalObserver_isFinite(currentBeta_A) == 0u))
    {
        HfoExternalObserver_invalidate(HFO_EXTERNAL_OBSERVER_STATUS_NUMERICAL_INVALID);
        return;
    }

    if (HfoExternalObserver_parametersAreValid(resistance_Ohm, directInductance_H,
            quadratureInductance_H, permanentMagnetFlux_Wb, sampleTime_s, polePairs,
            correctionGain_radps, injectionGain, speedFilterHz) == 0u)
    {
        HfoExternalObserver_invalidate(HFO_EXTERNAL_OBSERVER_STATUS_PARAMETER_INVALID);
        return;
    }

    HfoExternalObserver_updateSpeedFilter(speedFilterHz, sampleTime_s);
    sinCos = Ifx_Math_SinCos_F16(hfoPreviousAngleIndex);
    sine = (float)sinCos.sin / HFO_Q15_SCALE;
    cosine = (float)sinCos.cos / HFO_Q15_SCALE;

    directCurrent_A = (cosine * currentAlpha_A) + (sine * currentBeta_A);
    quadratureCurrent_A = (-sine * currentAlpha_A) + (cosine * currentBeta_A);

    /* Current-model flux with the paper's fixed virtual injection:
     * psi_d_inj = -Kinj*psi_f and psi_q_inj = Kinj*psi_f. */
    psiCurrentDirect_Wb = (directInductance_H * directCurrent_A)
        + permanentMagnetFlux_Wb - (injectionGain * permanentMagnetFlux_Wb);
    psiCurrentQuadrature_Wb = (quadratureInductance_H * quadratureCurrent_A)
        + (injectionGain * permanentMagnetFlux_Wb);
    psiCurrentAlpha_Wb = (cosine * psiCurrentDirect_Wb) - (sine * psiCurrentQuadrature_Wb);
    psiCurrentBeta_Wb = (sine * psiCurrentDirect_Wb) + (cosine * psiCurrentQuadrature_Wb);

    /* P-corrected voltage-model flux. */
    hfoPsiVoltageAlpha_Wb += sampleTime_s * (voltageAlpha_V - (resistance_Ohm * currentAlpha_A)
        + (correctionGain_radps * (psiCurrentAlpha_Wb - hfoPsiVoltageAlpha_Wb)));
    hfoPsiVoltageBeta_Wb += sampleTime_s * (voltageBeta_V - (resistance_Ohm * currentBeta_A)
        + (correctionGain_radps * (psiCurrentBeta_Wb - hfoPsiVoltageBeta_Wb)));

    psiActiveAlpha_Wb = hfoPsiVoltageAlpha_Wb - (quadratureInductance_H * currentAlpha_A);
    psiActiveBeta_Wb = hfoPsiVoltageBeta_Wb - (quadratureInductance_H * currentBeta_A);
    activeFluxThreshold_Wb = HfoExternalObserver_absolute(permanentMagnetFlux_Wb) * 0.0001F;
    if (activeFluxThreshold_Wb < HFO_MIN_ACTIVE_FLUX_WB)
    {
        activeFluxThreshold_Wb = HFO_MIN_ACTIVE_FLUX_WB;
    }
    activeFluxMagnitudeSquared_Wb2 = (psiActiveAlpha_Wb * psiActiveAlpha_Wb)
        + (psiActiveBeta_Wb * psiActiveBeta_Wb);

    if ((HfoExternalObserver_isFinite(directCurrent_A) == 0u)
        || (HfoExternalObserver_isFinite(quadratureCurrent_A) == 0u)
        || (HfoExternalObserver_isFinite(psiCurrentAlpha_Wb) == 0u)
        || (HfoExternalObserver_isFinite(psiCurrentBeta_Wb) == 0u)
        || (HfoExternalObserver_isFinite(hfoPsiVoltageAlpha_Wb) == 0u)
        || (HfoExternalObserver_isFinite(hfoPsiVoltageBeta_Wb) == 0u)
        || (HfoExternalObserver_isFinite(psiActiveAlpha_Wb) == 0u)
        || (HfoExternalObserver_isFinite(psiActiveBeta_Wb) == 0u)
        || (HfoExternalObserver_isFinite(activeFluxMagnitudeSquared_Wb2) == 0u))
    {
        HfoExternalObserver_invalidate(HFO_EXTERNAL_OBSERVER_STATUS_NUMERICAL_INVALID);
        return;
    }

    Meas_Hfo_Id_A_f32 = directCurrent_A;
    Meas_Hfo_Iq_A_f32 = quadratureCurrent_A;
    Meas_Hfo_PsiDInjection_Wb_f32 = -injectionGain * permanentMagnetFlux_Wb;
    Meas_Hfo_PsiQInjection_Wb_f32 = injectionGain * permanentMagnetFlux_Wb;
    Meas_Hfo_PsiCurrentAlpha_Wb_f32 = psiCurrentAlpha_Wb;
    Meas_Hfo_PsiCurrentBeta_Wb_f32 = psiCurrentBeta_Wb;
    Meas_Hfo_PsiVoltageAlpha_Wb_f32 = hfoPsiVoltageAlpha_Wb;
    Meas_Hfo_PsiVoltageBeta_Wb_f32 = hfoPsiVoltageBeta_Wb;
    Meas_Hfo_PsiActiveAlpha_Wb_f32 = psiActiveAlpha_Wb;
    Meas_Hfo_PsiActiveBeta_Wb_f32 = psiActiveBeta_Wb;
    Meas_Hfo_ActiveFluxMagnitude_Wb_f32 = sqrtf(activeFluxMagnitudeSquared_Wb2);

    if (activeFluxMagnitudeSquared_Wb2
        <= (activeFluxThreshold_Wb * activeFluxThreshold_Wb))
    {
        /* Do not differentiate a collapsed active-flux vector. Match the
         * Simulink observer by retaining the last valid angle and speed once
         * initialization has completed. */
        Meas_Hfo_ElecAngle_rad_f32 = (float)hfoPreviousAngleIndex
            * (HFO_TWO_PI_RAD / HFO_Q32_PER_CYCLE);
        Meas_Hfo_MechSpeed_rpm_f32 = (hfoHasValidAngle != 0u)
            ? hfoFilteredMechanicalSpeed_rpm : 0.0F;
        Meas_Hfo_Valid_u8 = hfoHasValidAngle;
        Meas_Hfo_Status_u8 = HFO_EXTERNAL_OBSERVER_STATUS_ACTIVE_FLUX_TOO_SMALL;
        return;
    }

    electricalAngleIndex = HfoExternalObserver_angleFromActiveFlux(psiActiveAlpha_Wb, psiActiveBeta_Wb);
    if (hfoHasValidAngle == 0u)
    {
        rawMechanicalSpeed_rpm = 0.0F;
        hfoHasValidAngle = 1u;
    }
    else
    {
        int64_t angleDifferenceIndex = (int64_t)(uint64_t)electricalAngleIndex
            - (int64_t)(uint64_t)hfoPreviousAngleIndex;

        if (angleDifferenceIndex > 2147483647LL)
        {
            angleDifferenceIndex -= 4294967296LL;
        }
        else if (angleDifferenceIndex < -2147483648LL)
        {
            angleDifferenceIndex += 4294967296LL;
        }

        rawMechanicalSpeed_rpm = ((float)angleDifferenceIndex
            * (HFO_TWO_PI_RAD / HFO_Q32_PER_CYCLE) / sampleTime_s)
            * (60.0F / (HFO_TWO_PI_RAD * (float)polePairs));
    }

    hfoPreviousAngleIndex = electricalAngleIndex;
    hfoFilteredMechanicalSpeed_rpm += hfoSpeedFilterGain
        * (rawMechanicalSpeed_rpm - hfoFilteredMechanicalSpeed_rpm);

    if ((HfoExternalObserver_isFinite(rawMechanicalSpeed_rpm) == 0u)
        || (HfoExternalObserver_isFinite(hfoFilteredMechanicalSpeed_rpm) == 0u))
    {
        HfoExternalObserver_invalidate(HFO_EXTERNAL_OBSERVER_STATUS_NUMERICAL_INVALID);
        return;
    }

    Meas_Hfo_ElecAngle_rad_f32 = (float)electricalAngleIndex
        * (HFO_TWO_PI_RAD / HFO_Q32_PER_CYCLE);
    Meas_Hfo_MechSpeed_rpm_f32 = hfoFilteredMechanicalSpeed_rpm;
    Meas_Hfo_Valid_u8 = 1u;
    Meas_Hfo_Status_u8 = HFO_EXTERNAL_OBSERVER_STATUS_VALID;
}

uint8_t HfoExternalObserver_getFocEstimate(float *electricalAngle_rad,
                                           float *mechanicalSpeed_rpm)
{
    if ((electricalAngle_rad == 0) || (mechanicalSpeed_rpm == 0))
    {
        return 0u;
    }

    if ((Meas_Hfo_Valid_u8 == 0u)
        || ((Meas_Hfo_Status_u8 != HFO_EXTERNAL_OBSERVER_STATUS_VALID)
            && (Meas_Hfo_Status_u8 != HFO_EXTERNAL_OBSERVER_STATUS_ACTIVE_FLUX_TOO_SMALL))
        || (HfoExternalObserver_isFinite(Meas_Hfo_ElecAngle_rad_f32) == 0u)
        || (HfoExternalObserver_isFinite(Meas_Hfo_MechSpeed_rpm_f32) == 0u))
    {
        return 0u;
    }

    *electricalAngle_rad = Meas_Hfo_ElecAngle_rad_f32;
    *mechanicalSpeed_rpm = Meas_Hfo_MechSpeed_rpm_f32;
    return 1u;
}
