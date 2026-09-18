#include "../ConfigWizard/FocTiming_Cfg.h"
#include "kre_external_observer_adapter.h"

#include <float.h>

#include "../Utilities/no_opt.h"
#include "../ConfigWizard/Ifx_MS_FocSolutionF16_Cfg.h"
#if FOC_AUX_ALGORITHMS_ENABLE
#include "../VAFID_codegen/vafid_parameter_identifier.h"
#endif
#include "kre_external_observer_wrapper_ert_rtw/kre_external_observer_wrapper.h"

#if defined(__ICCARM__)
#define KRE_XCP_SECTION _Pragma("location=\".xcp_cal_m4\"")
#else
#define KRE_XCP_SECTION
#endif

#define KRE_TWO_PI_RAD (6.2831853071795864769F)
#define KRE_MILLI_TO_BASE (0.001F)
#define KRE_MICRO_TO_BASE (0.000001F)
#define KRE_US_TO_SEC (0.000001F)
#define KRE_PARAMETER_COUNT (14u)
#define KRE_PLL_PARAMETER_COUNT (2u)

typedef struct
{
    float parameters[KRE_PARAMETER_COUNT];
    float pllParameters[KRE_PLL_PARAMETER_COUNT];
    KreDiscreteCoefficients coefficients;
} KreExternalObserverParameterSnapshot;

/* Existing project calibration values. They are deliberately not duplicated
 * as KRE-specific XCP parameters. */
extern volatile uint16_t Cal_MotorAppliedPhaseResistance_mOhm_u16;
extern volatile uint16_t Cal_MotorAppliedDirectInductance_uH_u16;
extern volatile uint16_t Cal_MotorAppliedQuadratureInductance_uH_u16;
extern volatile uint8_t Cal_MotorAppliedPolePairs_u8;
extern volatile uint16_t Cal_MTPA_AppliedPermanentMagnetFlux_mWb_u16;

KRE_XCP_SECTION
NO_OPT volatile float Cal_Kre_Alpha_radps_f32 = 1256.637061F;
KRE_XCP_SECTION
NO_OPT volatile float Cal_Kre_A_radps_f32 = 125.663704F;
KRE_XCP_SECTION
NO_OPT volatile float Cal_Kre_Gamma_f32 = 1.0F;
KRE_XCP_SECTION
NO_OPT volatile float Cal_Kre_SigmaEpsilon_Wb_f32 = 0.00001F;
KRE_XCP_SECTION
NO_OPT volatile float Cal_Kre_SpeedFilter_Hz_f32 = 150.0F;
KRE_XCP_SECTION
NO_OPT volatile float Cal_Kre_PllBandwidth_Hz_f32 = 100.0F;
KRE_XCP_SECTION
NO_OPT volatile float Cal_Kre_PllDamping_f32 = 0.70710678F;

KRE_XCP_SECTION
NO_OPT volatile float Meas_Kre_ElecAngle_rad_f32 = 0.0F;
KRE_XCP_SECTION
NO_OPT volatile float Meas_Kre_MechSpeed_rpm_f32 = 0.0F;
KRE_XCP_SECTION
NO_OPT volatile float Meas_Kre_RotorFluxMag_Wb_f32 = 0.0F;
KRE_XCP_SECTION
NO_OPT volatile uint8_t Meas_Kre_Valid_u8 = 0u;
KRE_XCP_SECTION
NO_OPT volatile uint8_t Meas_Kre_Status_u8 = 0u;
KRE_XCP_SECTION
NO_OPT volatile float Meas_KRE_ActFlux_Wb_f32 = 0.0F;
KRE_XCP_SECTION
NO_OPT volatile float Meas_KRE_ActFluxA_Wb_f32 = 0.0F;
KRE_XCP_SECTION
NO_OPT volatile float Meas_KRE_ActFluxB_Wb_f32 = 0.0F;
KRE_XCP_SECTION
NO_OPT volatile float Meas_KRE_RawOmega_radps_f32 = 0.0F;
KRE_XCP_SECTION
NO_OPT volatile uint32_t Meas_KRE_RstCount_u32 = 0u;
KRE_XCP_SECTION
NO_OPT volatile uint8_t Meas_KRE_RstReason_u8 = KRE_EXTERNAL_OBSERVER_RST_NONE;
KRE_XCP_SECTION
NO_OPT volatile uint8_t Meas_KRE_ParamValid_u8 = 0u;
KRE_XCP_SECTION
NO_OPT volatile uint32_t Meas_KRE_ParamApplySeq_u32 = 0u;
KRE_XCP_SECTION
NO_OPT volatile uint8_t Meas_KRE_ParamPending_u8 = 0u;

static float kreViFbDelay[4];
static KreExternalObserverParameterSnapshot kreStagedParameters;
static KreExternalObserverParameterSnapshot kreAppliedParameters;
static uint8_t kreHasInput;
static uint8_t kreResetPending;
static uint8_t kreFaultResetLatched;
static uint8_t kreStagedAvailable;
static uint8_t kreStagedValid;

static uint8_t KreExternalObserver_isFinite(const float value)
{
    return ((value == value) && (value <= FLT_MAX) && (value >= -FLT_MAX)) ? 1u : 0u;
}

static void KreExternalObserver_copyParameters(
    KreExternalObserverParameterSnapshot *destination,
    const KreExternalObserverParameterSnapshot *source)
{
    uint8_t index;

    for (index = 0u; index < KRE_PARAMETER_COUNT; ++index)
    {
        destination->parameters[index] = source->parameters[index];
    }
    for (index = 0u; index < KRE_PLL_PARAMETER_COUNT; ++index)
    {
        destination->pllParameters[index] = source->pllParameters[index];
    }
    destination->coefficients = source->coefficients;
}

static uint8_t KreExternalObserver_parametersEqual(
    const KreExternalObserverParameterSnapshot *left,
    const KreExternalObserverParameterSnapshot *right)
{
    uint8_t index;

    for (index = 0u; index < KRE_PARAMETER_COUNT; ++index)
    {
        if (left->parameters[index] != right->parameters[index])
        {
            return 0u;
        }
    }
    for (index = 0u; index < KRE_PLL_PARAMETER_COUNT; ++index)
    {
        if (left->pllParameters[index] != right->pllParameters[index])
        {
            return 0u;
        }
    }
    return 1u;
}

static void KreExternalObserver_captureParameters(
    KreExternalObserverParameterSnapshot *snapshot)
{
    snapshot->parameters[0] =
        (float)Cal_MotorAppliedPhaseResistance_mOhm_u16 * KRE_MILLI_TO_BASE;
    snapshot->parameters[1] =
        (float)Cal_MotorAppliedDirectInductance_uH_u16 * KRE_MICRO_TO_BASE;
    snapshot->parameters[2] =
        (float)Cal_MotorAppliedQuadratureInductance_uH_u16 * KRE_MICRO_TO_BASE;
    snapshot->parameters[3] =
        (float)Cal_MTPA_AppliedPermanentMagnetFlux_mWb_u16 * KRE_MILLI_TO_BASE;
    snapshot->parameters[4] =
        (float)IFX_MS_FOCSOLUTIONF16_CFG_SAMPLING_TIME_US * KRE_US_TO_SEC;
    snapshot->parameters[5] =
        (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_VOLTAGE_V;
    snapshot->parameters[6] =
        (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_CURRENT_A;
    snapshot->parameters[7] =
        (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM;
    snapshot->parameters[8] = (float)Cal_MotorAppliedPolePairs_u8;
    snapshot->parameters[9] = Cal_Kre_Alpha_radps_f32;
    snapshot->parameters[10] = Cal_Kre_A_radps_f32;
    snapshot->parameters[11] = Cal_Kre_Gamma_f32;
    snapshot->parameters[12] = Cal_Kre_SigmaEpsilon_Wb_f32;
    snapshot->parameters[13] = Cal_Kre_SpeedFilter_Hz_f32;
    snapshot->pllParameters[0] = Cal_Kre_PllBandwidth_Hz_f32;
    snapshot->pllParameters[1] = Cal_Kre_PllDamping_f32;
}

static void KreExternalObserver_calculateCoefficients(
    KreExternalObserverParameterSnapshot *snapshot)
{
    const float ts = snapshot->parameters[4];
    const float wn = 6.2831855F * snapshot->pllParameters[0];

    /* Preserve the generated expressions and operation ordering. Do not
     * approximate exp(), change discretization, or add a parameter range gate.
     * This runs only on a changed snapshot, outside the publication lock. */
    snapshot->coefficients.h2Gain =
        1.0F - expf(-snapshot->parameters[9] * ts);
    snapshot->coefficients.speedGain =
        1.0F - expf(-6.2831855F * snapshot->parameters[13] * ts);
    snapshot->coefficients.pllKp =
        2.0F * snapshot->pllParameters[1] * wn;
    snapshot->coefficients.pllKiTs = wn * wn * ts;
    snapshot->coefficients.inductanceDelta =
        snapshot->parameters[1] - snapshot->parameters[2];
}

static void KreExternalObserver_publishParameters(
    const KreExternalObserverParameterSnapshot *snapshot)
{
    uint8_t index;

    for (index = 0u; index < KRE_PARAMETER_COUNT; ++index)
    {
        kre_external_observer_wrapper_U.kreParams[index] =
            snapshot->parameters[index];
    }
    for (index = 0u; index < KRE_PLL_PARAMETER_COUNT; ++index)
    {
        kre_external_observer_wrapper_U.krePllParams[index] =
            snapshot->pllParameters[index];
    }
    kre_external_observer_coefficients = snapshot->coefficients;
}

static void KreExternalObserver_armGeneratedReset(
    const KreExternalObserverResetReason reason)
{
    /* Count logical reset requests. Input/output fault callers are edge
     * latched, so a persistent bad sample contributes only one request. */
    ++Meas_KRE_RstCount_u32;
    Meas_KRE_RstReason_u8 = (uint8_t)reason;
    kre_external_observer_wrapper_U.kreReset = 1u;
    kre_external_observer_w_PrevZCX.KreObserverResettable_Reset_ZCE = NEG_ZCSIG;
    kreResetPending = 1u;
}

static void KreExternalObserver_requestFaultReset(
    const KreExternalObserverResetReason reason)
{
    if (kreFaultResetLatched == 0u)
    {
        kreFaultResetLatched = 1u;
        KreExternalObserver_armGeneratedReset(reason);
    }
}

static void KreExternalObserver_clearMeasurements(void)
{
    Meas_Kre_ElecAngle_rad_f32 = 0.0F;
    Meas_Kre_MechSpeed_rpm_f32 = 0.0F;
    Meas_Kre_RotorFluxMag_Wb_f32 = (Meas_KRE_ParamValid_u8 != 0u)
        ? kreAppliedParameters.parameters[3] : 0.0F;
    Meas_Kre_Valid_u8 = 0u;
    Meas_Kre_Status_u8 = 0u;
    Meas_KRE_ActFlux_Wb_f32 = 0.0F;
    Meas_KRE_ActFluxA_Wb_f32 = 0.0F;
    Meas_KRE_ActFluxB_Wb_f32 = 0.0F;
    Meas_KRE_RawOmega_radps_f32 = 0.0F;
}

static uint8_t KreExternalObserver_stageParametersInternal(const float *motor)
{
    KreExternalObserverParameterSnapshot firstRead;
    KreExternalObserverParameterSnapshot secondRead;
    uint8_t stagedChanged;

    /* A bounded double read rejects a calibration group that changed while
     * it was being sampled. The previously applied snapshot is untouched. */
    KreExternalObserver_captureParameters(&firstRead);
    KreExternalObserver_captureParameters(&secondRead);
    if (motor != 0)
    {
        /* The project supplies the same captured motor set used to prepare
         * the current controller/I-f coefficients for the joint commit. */
        uint8_t index;
        for (index = 0u; index < 4u; ++index)
        {
            firstRead.parameters[index] = motor[index];
            secondRead.parameters[index] = motor[index];
        }
        firstRead.parameters[8] = motor[4];
        secondRead.parameters[8] = motor[4];
    }
    if (KreExternalObserver_parametersEqual(&firstRead, &secondRead) == 0u)
    {
        kreStagedAvailable = 0u;
        kreStagedValid = 0u;
        Meas_KRE_ParamPending_u8 = 1u;
        return 0u;
    }

    stagedChanged = ((kreStagedAvailable == 0u)
        || (KreExternalObserver_parametersEqual(
            &kreStagedParameters, &secondRead) == 0u)) ? 1u : 0u;
    if (stagedChanged != 0u)
    {
        KreExternalObserver_calculateCoefficients(&secondRead);
        KreExternalObserver_copyParameters(&kreStagedParameters, &secondRead);
        kreStagedAvailable = 1u;
        /* The foreground service only captures one coherent parameter group.
         * KRE output valid/status remains the single runtime qualification;
         * do not add a second physical-range gate in the integration layer. */
        kreStagedValid = 1u;
    }

    if ((Meas_KRE_ParamValid_u8 != 0u)
        && (KreExternalObserver_parametersEqual(
            &kreStagedParameters, &kreAppliedParameters) != 0u))
    {
        /* No generated input update is needed when the captured group is the
         * already-applied snapshot. */
        Meas_KRE_ParamPending_u8 = 0u;
    }
    else
    {
        Meas_KRE_ParamPending_u8 = 1u;
    }

    if ((kreStagedValid == 0u) && (Meas_KRE_ParamValid_u8 == 0u))
    {
        Meas_Kre_Valid_u8 = 0u;
        Meas_Kre_Status_u8 = KRE_EXTERNAL_OBSERVER_STATUS_PARAMETER_INVALID;
    }
    return kreStagedValid;
}

uint8_t KreExternalObserver_stageParameters(void)
{
    return KreExternalObserver_stageParametersInternal(0);
}

uint8_t KreExternalObserver_stageMotorParameters(uint16_t resistance_mOhm,
    uint16_t directInductance_uH, uint16_t quadratureInductance_uH,
    uint16_t flux_mWb, uint8_t polePairs)
{
    float motor[5];
    motor[0] = (float)resistance_mOhm * KRE_MILLI_TO_BASE;
    motor[1] = (float)directInductance_uH * KRE_MICRO_TO_BASE;
    motor[2] = (float)quadratureInductance_uH * KRE_MICRO_TO_BASE;
    motor[3] = (float)flux_mWb * KRE_MILLI_TO_BASE;
    motor[4] = (float)polePairs;
    return KreExternalObserver_stageParametersInternal(motor);
}

uint8_t KreExternalObserver_applyStagedParameters(void)
{
    if (Meas_KRE_ParamPending_u8 == 0u)
    {
        return KreExternalObserver_parametersReady();
    }
    if ((kreStagedAvailable == 0u) || (kreStagedValid == 0u))
    {
        if (Meas_KRE_ParamValid_u8 == 0u)
        {
            Meas_Kre_Valid_u8 = 0u;
            Meas_Kre_Status_u8 = KRE_EXTERNAL_OBSERVER_STATUS_PARAMETER_INVALID;
        }
        return 0u;
    }
    /* The foreground caller excludes IRQ execution during this bounded copy.
     * Raw parameters and derived coefficients become visible as one snapshot
     * between observer steps. Online tuning must not clear observer states,
     * the delayed VI sample, PLL integrator, or the published validity. */
    KreExternalObserver_publishParameters(&kreStagedParameters);
    KreExternalObserver_copyParameters(
        &kreAppliedParameters, &kreStagedParameters);
    Meas_KRE_ParamValid_u8 = 1u;
    Meas_KRE_ParamPending_u8 = 0u;
    ++Meas_KRE_ParamApplySeq_u32;
    return 1u;
}

uint8_t KreExternalObserver_parametersReady(void)
{
    return ((Meas_KRE_ParamValid_u8 != 0u)
        && (Meas_KRE_ParamPending_u8 == 0u)) ? 1u : 0u;
}

void KreExternalObserver_initialize(void)
{
    uint8_t index;

    for (index = 0u; index < KRE_PARAMETER_COUNT; ++index)
    {
        kreStagedParameters.parameters[index] = 0.0F;
        kreAppliedParameters.parameters[index] = 0.0F;
    }
    for (index = 0u; index < KRE_PLL_PARAMETER_COUNT; ++index)
    {
        kreStagedParameters.pllParameters[index] = 0.0F;
        kreAppliedParameters.pllParameters[index] = 0.0F;
    }
    kreResetPending = 0u;
    kreFaultResetLatched = 0u;
    kreStagedAvailable = 0u;
    kreStagedValid = 0u;
    Meas_KRE_RstCount_u32 = 0u;
    Meas_KRE_RstReason_u8 = KRE_EXTERNAL_OBSERVER_RST_NONE;
    Meas_KRE_ParamValid_u8 = 0u;
    Meas_KRE_ParamApplySeq_u32 = 0u;
    Meas_KRE_ParamPending_u8 = 0u;
    kre_external_observer_wrapper_initialize();
    KreExternalObserver_resetWithReason(
        KRE_EXTERNAL_OBSERVER_RST_INITIALIZE);
    /* Startup owns the observer exclusively. The application restages after
     * its motor-calibration initialization, before enabling control IRQs. */
    (void)KreExternalObserver_stageParameters();
    (void)KreExternalObserver_applyStagedParameters();
    KreExternalObserver_clearMeasurements();
}

void KreExternalObserver_reset(void)
{
    KreExternalObserver_resetWithReason(
        KRE_EXTERNAL_OBSERVER_RST_EXTERNAL_REQUEST);
}

void KreExternalObserver_resetWithReason(
    const KreExternalObserverResetReason reason)
{
    uint8_t index;

    for (index = 0u; index < 4u; ++index)
    {
        kreViFbDelay[index] = 0.0F;
        kre_external_observer_wrapper_U.kreViFb[index] = 0.0F;
    }

    kre_external_observer_wrapper_U.kreEnable = 0u;
    KreExternalObserver_armGeneratedReset(reason);
    kreHasInput = 0u;
    kreFaultResetLatched = 0u;
    KreExternalObserver_clearMeasurements();
    /* This reset can run in the fast-loop estimator transition. Abort only
     * the ISR-owned collection; the foreground service owns full resets. */
#if FOC_AUX_ALGORITHMS_ENABLE
    if (Cal_VAFID_Mode_u8 == VAFID_MODE_SHADOW)
    {
        VAFID_abortFast(VAFID_REJECT_ELIGIBILITY);
    }
#endif
}

void KreExternalObserver_execute(const float voltageAlpha_V,
                                 const float voltageBeta_V,
                                 const float currentAlpha_A,
                                 const float currentBeta_A)
{
    uint8_t index;
    float activeFluxWb[2];

    if (Meas_KRE_ParamValid_u8 == 0u)
    {
        KreExternalObserver_clearMeasurements();
        Meas_Kre_Status_u8 = KRE_EXTERNAL_OBSERVER_STATUS_PARAMETER_INVALID;
        kreHasInput = 0u;
        return;
    }

    if ((KreExternalObserver_isFinite(voltageAlpha_V) == 0u)
        || (KreExternalObserver_isFinite(voltageBeta_V) == 0u)
        || (KreExternalObserver_isFinite(currentAlpha_A) == 0u)
        || (KreExternalObserver_isFinite(currentBeta_A) == 0u))
    {
        KreExternalObserver_clearMeasurements();
        Meas_Kre_Status_u8 = KRE_EXTERNAL_OBSERVER_STATUS_NUMERICAL_INVALID;
#if FOC_AUX_ALGORITHMS_ENABLE
        if (Cal_VAFID_Mode_u8 == VAFID_MODE_SHADOW)
        {
            VAFID_observerStep(currentAlpha_A, currentBeta_A, 0.0F, 0.0F,
                0.0F, 0u, 0u);
        }
#endif
        kreHasInput = 0u;
        KreExternalObserver_requestFaultReset(
            KRE_EXTERNAL_OBSERVER_RST_INPUT_INVALID);
        return;
    }

    /* Consume the previous complete sample, then retain this sample for the
     * next observer call. This is the VI_fb delay used by the Simulink model. */
    for (index = 0u; index < 4u; ++index)
    {
        kre_external_observer_wrapper_U.kreViFb[index] = kreViFbDelay[index];
    }
    kreViFbDelay[0] = voltageAlpha_V
        / (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_VOLTAGE_V;
    kreViFbDelay[1] = voltageBeta_V
        / (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_VOLTAGE_V;
    kreViFbDelay[2] = currentAlpha_A
        / (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_CURRENT_A;
    kreViFbDelay[3] = currentBeta_A
        / (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_CURRENT_A;

    kre_external_observer_wrapper_U.kreEnable = 1u;
    kre_external_observer_wrapper_U.kreReset = (kreResetPending != 0u) ? 1u : 0u;
    /* Diagnostic-only reconstruction of the core's xHat, before its Euler
     * state update, using the same delayed PU current and applied Lq/base.
     * armGeneratedReset() guarantees a rising reset edge; that step uses
     * zero lambdaHat rather than the old state still present here.
     * Keep the multiplication order identical to the generated core. */
    for (index = 0u; index < 2u; ++index)
    {
        const float currentA = kre_external_observer_wrapper_U.kreViFb[index + 2u]
            * kre_external_observer_wrapper_U.kreParams[6];
        const float lambdaWb = (kreResetPending != 0u) ? 0.0F
            : kre_external_observer_wrappe_DW.lambdaHat[index];
        activeFluxWb[index] = lambdaWb
            - kre_external_observer_wrapper_U.kreParams[2] * currentA;
    }
    kre_external_observer_wrapper_step();
    kre_external_observer_wrapper_U.kreReset = 0u;
    kreResetPending = 0u;

    Meas_Kre_ElecAngle_rad_f32 =
        kre_external_observer_wrapper_Y.krePositionPU * KRE_TWO_PI_RAD;
    Meas_Kre_MechSpeed_rpm_f32 =
        kre_external_observer_wrapper_Y.kreSpeedPU
        * (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM;
    /* Preserve the established Applied-PM-flux measurement semantics. The
     * new uppercase diagnostics below expose KRE's actual internal signals. */
    Meas_Kre_RotorFluxMag_Wb_f32 = kreAppliedParameters.parameters[3];
    Meas_KRE_ActFlux_Wb_f32 =
        kre_external_observer_wrapper_Y.kreActiveFlux_Wb;
    Meas_KRE_RawOmega_radps_f32 =
        kre_external_observer_wrapper_Y.kreRawOmega_radps;
    Meas_Kre_Status_u8 = kre_external_observer_wrapper_Y.kreStatus;
    if ((Meas_Kre_Status_u8 == KRE_EXTERNAL_OBSERVER_STATUS_VALID)
        && (KreExternalObserver_isFinite(Meas_Kre_ElecAngle_rad_f32) != 0u)
        && (KreExternalObserver_isFinite(Meas_Kre_MechSpeed_rpm_f32) != 0u)
        && (KreExternalObserver_isFinite(Meas_KRE_ActFlux_Wb_f32) != 0u)
        && (KreExternalObserver_isFinite(Meas_KRE_RawOmega_radps_f32) != 0u))
    {
        /* The first post-reset sample initializes the angle differentiator;
         * expose it only after the following complete sample. */
        Meas_Kre_Valid_u8 = (kreHasInput != 0u) ? 1u : 0u;
        Meas_KRE_ActFluxA_Wb_f32 = activeFluxWb[0];
        Meas_KRE_ActFluxB_Wb_f32 = activeFluxWb[1];
        kreHasInput = 1u;
        kreFaultResetLatched = 0u;
    }
    else
    {
        Meas_KRE_ActFluxA_Wb_f32 = 0.0F;
        Meas_KRE_ActFluxB_Wb_f32 = 0.0F;
        Meas_Kre_Valid_u8 = 0u;
        Meas_Kre_Status_u8 = KRE_EXTERNAL_OBSERVER_STATUS_NUMERICAL_INVALID;
        kreHasInput = 0u;
        KreExternalObserver_requestFaultReset(
            KRE_EXTERNAL_OBSERVER_RST_OUTPUT_INVALID);
    }

    /* VAFID is a shadow sidecar only. It currently receives the ideal PWM
     * voltage model paired with this ADC current sample; no validated
     * dead-time voltage correction is applied. KRE's active flux remains
     * diagnostic and is not qualified for FluxPM feedback. */
#if FOC_AUX_ALGORITHMS_ENABLE
    if (Cal_VAFID_Mode_u8 == VAFID_MODE_SHADOW)
    {
        float vafidElectricalAngle_rad;
        float vafidRawElectricalOmega_radps;
        float vafidActiveFlux_Wb;
        uint8_t vafidKreValid;

        /* Sequence volatile diagnostic reads before the function call. */
        vafidElectricalAngle_rad = Meas_Kre_ElecAngle_rad_f32;
        vafidRawElectricalOmega_radps = Meas_KRE_RawOmega_radps_f32;
        vafidActiveFlux_Wb = Meas_KRE_ActFlux_Wb_f32;
        vafidKreValid = Meas_Kre_Valid_u8;
        VAFID_observerStep(currentAlpha_A, currentBeta_A,
            vafidElectricalAngle_rad, vafidRawElectricalOmega_radps,
            vafidActiveFlux_Wb, vafidKreValid, 0u);
    }
#endif
}

uint8_t KreExternalObserver_getFocEstimate(float *electricalAngle_rad,
                                           float *mechanicalSpeed_rpm)
{
    if ((electricalAngle_rad == 0) || (mechanicalSpeed_rpm == 0))
    {
        return 0u;
    }

    if ((Meas_Kre_Valid_u8 == 0u)
        || (Meas_Kre_Status_u8 != KRE_EXTERNAL_OBSERVER_STATUS_VALID)
        || (KreExternalObserver_isFinite(Meas_Kre_ElecAngle_rad_f32) == 0u)
        || (KreExternalObserver_isFinite(Meas_Kre_MechSpeed_rpm_f32) == 0u))
    {
        return 0u;
    }

    *electricalAngle_rad = Meas_Kre_ElecAngle_rad_f32;
    *mechanicalSpeed_rpm = Meas_Kre_MechSpeed_rpm_f32;
    return 1u;
}
