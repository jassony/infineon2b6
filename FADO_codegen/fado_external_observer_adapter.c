#include "fado_external_observer_adapter.h"

#include <float.h>

#include "../Utilities/no_opt.h"
#include "fado_external_observer_wrapper_ert_rtw/fado_external_observer_wrapper.h"
#include "../KRE_codegen/kre_external_observer_adapter.h"

#if defined(__ICCARM__)
#define FADO_XCP_SECTION _Pragma("location=\".xcp_cal_m4\"")
#else
#define FADO_XCP_SECTION
#endif

enum
{
    FADO_OUTPUT_ELEC_ANGLE_RAD = 0,
    FADO_OUTPUT_OMEGA_FAST_RADPS,
    FADO_OUTPUT_OMEGA_SLOW_RADPS,
    FADO_OUTPUT_MECH_SPEED_RPM,
    FADO_OUTPUT_LAMBDA_ALPHA1_WB,
    FADO_OUTPUT_LAMBDA_BETA1_WB,
    FADO_OUTPUT_LAMBDA_ALPHA2_WB,
    FADO_OUTPUT_LAMBDA_BETA2_WB,
    FADO_OUTPUT_DHAT_ALPHA_WB,
    FADO_OUTPUT_DHAT_BETA_WB,
    FADO_OUTPUT_ROTOR_FLUX_MAG_WB,
    FADO_OUTPUT_KDF_MODE = 12,
    FADO_OUTPUT_KAF_MODE = 13,
    FADO_OUTPUT_FLUX_LIMITED = 14,
    FADO_OUTPUT_VALID = 15,
    FADO_OUTPUT_STATUS = 16,
    FADO_OUTPUT_ACTIVE = 17
};

enum
{
    FADO_EXTERNAL_OBSERVER_STATUS_INACTIVE = 0u,
    FADO_EXTERNAL_OBSERVER_STATUS_VALID = 1u,
    FADO_EXTERNAL_OBSERVER_STATUS_NUMERICAL_INVALID = 2u,
    FADO_EXTERNAL_OBSERVER_STATUS_PARAMETER_INVALID = 3u
};

FADO_XCP_SECTION
NO_OPT volatile uint8_t Cal_Foc_EstimatorSelector_u8 = FocEstimatorSelector_kre;
FADO_XCP_SECTION
NO_OPT volatile uint8_t Meas_Foc_ActiveEstimator_u8 = FocEstimatorSelector_kre;

FADO_XCP_SECTION
NO_OPT volatile float Cal_Fado_StatorResistance_Ohm_f32 = 0.500F;
FADO_XCP_SECTION
NO_OPT volatile float Cal_Fado_QuadratureInductance_H_f32 = 0.001380F;
FADO_XCP_SECTION
NO_OPT volatile uint8_t Cal_Fado_PolePairs_u8 = 4u;
FADO_XCP_SECTION
NO_OPT volatile float Cal_Fado_FluxLimit_Wb_f32 = 0.052900F;
FADO_XCP_SECTION
NO_OPT volatile float Cal_Fado_Kdf_per_s_f32 = 0.500F;
FADO_XCP_SECTION
NO_OPT volatile float Cal_Fado_LowSpeedThreshold_Hz_f32 = 1.500F;
FADO_XCP_SECTION
NO_OPT volatile float Cal_Fado_Kaf_radps_f32 = 628.31854F;
FADO_XCP_SECTION
NO_OPT volatile float Cal_Fado_FastT2SBandwidth_Hz_f32 = 150.0F;
FADO_XCP_SECTION
NO_OPT volatile float Cal_Fado_SlowT2SBandwidth_Hz_f32 = 35.0F;
FADO_XCP_SECTION
NO_OPT volatile float Cal_Fado_T2SDamping_f32 = 1.0F;

FADO_XCP_SECTION
NO_OPT volatile float Meas_Fado_ElecAngle_rad_f32 = 0.0F;
FADO_XCP_SECTION
NO_OPT volatile float Meas_Fado_OmegaFast_radps_f32 = 0.0F;
FADO_XCP_SECTION
NO_OPT volatile float Meas_Fado_OmegaSlow_radps_f32 = 0.0F;
FADO_XCP_SECTION
NO_OPT volatile float Meas_Fado_MechSpeed_rpm_f32 = 0.0F;
FADO_XCP_SECTION
NO_OPT volatile float Meas_Fado_LambdaAlpha1_Wb_f32 = 0.0F;
FADO_XCP_SECTION
NO_OPT volatile float Meas_Fado_LambdaBeta1_Wb_f32 = 0.0F;
FADO_XCP_SECTION
NO_OPT volatile float Meas_Fado_LambdaAlpha2_Wb_f32 = 0.0F;
FADO_XCP_SECTION
NO_OPT volatile float Meas_Fado_LambdaBeta2_Wb_f32 = 0.0F;
FADO_XCP_SECTION
NO_OPT volatile float Meas_Fado_DhatAlpha_Wb_f32 = 0.0F;
FADO_XCP_SECTION
NO_OPT volatile float Meas_Fado_DhatBeta_Wb_f32 = 0.0F;
FADO_XCP_SECTION
NO_OPT volatile float Meas_Fado_RotorFluxMag_Wb_f32 = 0.0F;
FADO_XCP_SECTION
NO_OPT volatile uint8_t Meas_Fado_KdfMode_u8 = 0u;
FADO_XCP_SECTION
NO_OPT volatile uint8_t Meas_Fado_KafMode_u8 = 0u;
FADO_XCP_SECTION
NO_OPT volatile uint8_t Meas_Fado_FluxLimited_u8 = 0u;
FADO_XCP_SECTION
NO_OPT volatile uint8_t Meas_Fado_Valid_u8 = 0u;
FADO_XCP_SECTION
NO_OPT volatile uint8_t Meas_Fado_Status_u8 = FADO_EXTERNAL_OBSERVER_STATUS_INACTIVE;
FADO_XCP_SECTION
NO_OPT volatile uint8_t Meas_Fado_Active_u8 = 0u;

static volatile FocEstimatorSelector activeFocEstimator = FocEstimatorSelector_kre;

static uint8_t FadoExternalObserver_flag(const float value)
{
    return (value > 0.5F) ? 1u : 0u;
}


static uint8_t FadoExternalObserver_status(const float value)
{
    if (value == (float)FADO_EXTERNAL_OBSERVER_STATUS_VALID)
    {
        return FADO_EXTERNAL_OBSERVER_STATUS_VALID;
    }

    if (value == (float)FADO_EXTERNAL_OBSERVER_STATUS_NUMERICAL_INVALID)
    {
        return FADO_EXTERNAL_OBSERVER_STATUS_NUMERICAL_INVALID;
    }

    if (value == (float)FADO_EXTERNAL_OBSERVER_STATUS_PARAMETER_INVALID)
    {
        return FADO_EXTERNAL_OBSERVER_STATUS_PARAMETER_INVALID;
    }

    return FADO_EXTERNAL_OBSERVER_STATUS_INACTIVE;
}


static uint8_t ExternalObserverManager_isFinite(const float value)
{
    return ((value == value) && (value <= FLT_MAX) && (value >= -FLT_MAX)) ? 1u : 0u;
}

static void FadoExternalObserver_clearMeasurements(void)
{
    Meas_Fado_ElecAngle_rad_f32 = 0.0F;
    Meas_Fado_OmegaFast_radps_f32 = 0.0F;
    Meas_Fado_OmegaSlow_radps_f32 = 0.0F;
    Meas_Fado_MechSpeed_rpm_f32 = 0.0F;
    Meas_Fado_LambdaAlpha1_Wb_f32 = 0.0F;
    Meas_Fado_LambdaBeta1_Wb_f32 = 0.0F;
    Meas_Fado_LambdaAlpha2_Wb_f32 = 0.0F;
    Meas_Fado_LambdaBeta2_Wb_f32 = 0.0F;
    Meas_Fado_DhatAlpha_Wb_f32 = 0.0F;
    Meas_Fado_DhatBeta_Wb_f32 = 0.0F;
    Meas_Fado_RotorFluxMag_Wb_f32 = 0.0F;
    Meas_Fado_KdfMode_u8 = 0u;
    Meas_Fado_KafMode_u8 = 0u;
    Meas_Fado_FluxLimited_u8 = 0u;
    Meas_Fado_Valid_u8 = 0u;
    Meas_Fado_Status_u8 = FADO_EXTERNAL_OBSERVER_STATUS_INACTIVE;
    Meas_Fado_Active_u8 = 0u;
}

void ExternalObserverManager_initialize(void)
{
    fado_external_observer_wrapper_initialize();
    KreExternalObserver_initialize();
    activeFocEstimator = FocEstimatorSelector_kre;
    Meas_Foc_ActiveEstimator_u8 = FocEstimatorSelector_kre;
    ExternalObserverManager_reset();
}

void ExternalObserverManager_reset(void)
{
    uint8_t index;

    for (index = 0u; index < 10u; ++index)
    {
        fado_external_observer_wrapp_DW.fadoState_z[index] = 0.0F;
    }

    fado_external_observer_wrappe_U.fadoEnable = (boolean_T)0;
    for (index = 0u; index < 18u; ++index)
    {
        fado_external_observer_wrappe_Y.fadoOutput[index] = 0.0F;
    }

    KreExternalObserver_reset();
    FadoExternalObserver_clearMeasurements();
}

void ExternalObserverManager_setFocEstimator(const FocEstimatorSelector estimator)
{
    if ((estimator == FocEstimatorSelector_fado)
        || (estimator == FocEstimatorSelector_kre))
    {
        /* FADO is linked for diagnostics, but it is not permitted to own the
         * FOC loop. Keep the legacy selector/API value compatible by mapping
         * it to the validated KRE observer. */
        activeFocEstimator = FocEstimatorSelector_kre;
    }
    else if (estimator == FocEstimatorSelector_flux)
    {
        activeFocEstimator = FocEstimatorSelector_flux;
    }
    else
    {
        /* Preserve an invalid selection; the FOC layer will not run another
         * estimator until a valid selection is written. */
        activeFocEstimator = FocEstimatorSelector_invalid;
    }

    Meas_Foc_ActiveEstimator_u8 = (uint8_t)activeFocEstimator;
}

FocEstimatorSelector ExternalObserverManager_getFocEstimator(void)
{
    return activeFocEstimator;
}

uint8_t ExternalObserverManager_getFocEstimate(float *electricalAngle_rad,
                                                float *mechanicalSpeed_rpm)
{
    if ((electricalAngle_rad == 0) || (mechanicalSpeed_rpm == 0))
    {
        return 0u;
    }

    if (activeFocEstimator == FocEstimatorSelector_fado)
    {
        /* The generated output buffer is cleared during every observer step.
         * Only consume the adapter's completed-sample publication here; the
         * slow state machine must never inspect that working buffer directly. */
        if ((Meas_Fado_Valid_u8 == 0u)
            || (Meas_Fado_Status_u8 != FADO_EXTERNAL_OBSERVER_STATUS_VALID)
            || (Meas_Fado_Active_u8 == 0u)
            || (ExternalObserverManager_isFinite(Meas_Fado_ElecAngle_rad_f32) == 0u)
            || (ExternalObserverManager_isFinite(Meas_Fado_MechSpeed_rpm_f32) == 0u))
        {
            return 0u;
        }

        *electricalAngle_rad = Meas_Fado_ElecAngle_rad_f32;
        *mechanicalSpeed_rpm = Meas_Fado_MechSpeed_rpm_f32;
        return 1u;
    }

    if (activeFocEstimator == FocEstimatorSelector_kre)
    {
        return KreExternalObserver_getFocEstimate(electricalAngle_rad, mechanicalSpeed_rpm);
    }

    return 0u;
}

void ExternalObserverManager_execute(const float voltageAlpha_V,
                                     const float voltageBeta_V,
                                     const float currentAlpha_A,
                                     const float currentBeta_A)
{
    uint8_t fadoStatus;
    uint8_t fadoActive;
    uint8_t fadoValid;

    if (activeFocEstimator == FocEstimatorSelector_kre)
    {
        KreExternalObserver_execute(voltageAlpha_V, voltageBeta_V, currentAlpha_A, currentBeta_A);
        return;
    }

    if (activeFocEstimator != FocEstimatorSelector_fado)
    {
        return;
    }

    fado_external_observer_wrappe_U.fadoEnable = (boolean_T)1;
    fado_external_observer_wrappe_U.fadoVoltageAlpha_V = voltageAlpha_V;
    fado_external_observer_wrappe_U.fadoVoltageBeta_V = voltageBeta_V;
    fado_external_observer_wrappe_U.fadoCurrentAlpha_A = currentAlpha_A;
    fado_external_observer_wrappe_U.fadoCurrentBeta_A = currentBeta_A;
    fado_external_observer_wrappe_U.fadoStatorResistance_Ohm =
        Cal_Fado_StatorResistance_Ohm_f32;
    fado_external_observer_wrappe_U.fadoQuadratureInductance_H =
        Cal_Fado_QuadratureInductance_H_f32;
    fado_external_observer_wrappe_U.fadoPolePairs = Cal_Fado_PolePairs_u8;
    fado_external_observer_wrappe_U.fadoFluxLimit_Wb = Cal_Fado_FluxLimit_Wb_f32;
    fado_external_observer_wrappe_U.fadoKdf_per_s = Cal_Fado_Kdf_per_s_f32;
    fado_external_observer_wrappe_U.fadoLowSpeedThreshold_Hz =
        Cal_Fado_LowSpeedThreshold_Hz_f32;
    fado_external_observer_wrappe_U.fadoKaf_radps = Cal_Fado_Kaf_radps_f32;
    fado_external_observer_wrappe_U.fadoFastT2SBandwidth_Hz =
        Cal_Fado_FastT2SBandwidth_Hz_f32;
    fado_external_observer_wrappe_U.fadoSlowT2SBandwidth_Hz =
        Cal_Fado_SlowT2SBandwidth_Hz_f32;
    fado_external_observer_wrappe_U.fadoT2SDamping = Cal_Fado_T2SDamping_f32;

    /* The generated P-code branch assigns the raw voltages only when this
     * input is true. Zero offsets preserve the paper's unprocessed v_alpha/beta. */
    fado_external_observer_wrappe_U.fadoVoltageAlphaOffset_V = 0.0F;
    fado_external_observer_wrappe_U.fadoVoltageBetaOffset_V = 0.0F;
    fado_external_observer_wrappe_U.fadoVoltagePreprocessEnable = (boolean_T)1;

    fado_external_observer_wrapper_step();

    Meas_Fado_ElecAngle_rad_f32 =
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_ELEC_ANGLE_RAD];
    Meas_Fado_OmegaFast_radps_f32 =
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_OMEGA_FAST_RADPS];
    Meas_Fado_OmegaSlow_radps_f32 =
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_OMEGA_SLOW_RADPS];
    Meas_Fado_MechSpeed_rpm_f32 =
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_MECH_SPEED_RPM];
    Meas_Fado_LambdaAlpha1_Wb_f32 =
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_LAMBDA_ALPHA1_WB];
    Meas_Fado_LambdaBeta1_Wb_f32 =
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_LAMBDA_BETA1_WB];
    Meas_Fado_LambdaAlpha2_Wb_f32 =
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_LAMBDA_ALPHA2_WB];
    Meas_Fado_LambdaBeta2_Wb_f32 =
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_LAMBDA_BETA2_WB];
    Meas_Fado_DhatAlpha_Wb_f32 =
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_DHAT_ALPHA_WB];
    Meas_Fado_DhatBeta_Wb_f32 =
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_DHAT_BETA_WB];
    Meas_Fado_RotorFluxMag_Wb_f32 =
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_ROTOR_FLUX_MAG_WB];
    Meas_Fado_KdfMode_u8 = FadoExternalObserver_flag(
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_KDF_MODE]);
    Meas_Fado_KafMode_u8 = FadoExternalObserver_flag(
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_KAF_MODE]);
    Meas_Fado_FluxLimited_u8 = FadoExternalObserver_flag(
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_FLUX_LIMITED]);

    /* Publish the observer contract only after all sampled outputs have been
     * copied out of the generated working buffer. Valid is the commit flag
     * read by the slow FOC state machine. */
    fadoStatus = FadoExternalObserver_status(
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_STATUS]);
    fadoActive = FadoExternalObserver_flag(
        fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_ACTIVE]);
    fadoValid = ((FadoExternalObserver_flag(
            fado_external_observer_wrappe_Y.fadoOutput[FADO_OUTPUT_VALID]) != 0u)
        && (fadoStatus == FADO_EXTERNAL_OBSERVER_STATUS_VALID)
        && (fadoActive != 0u)
        && (ExternalObserverManager_isFinite(Meas_Fado_ElecAngle_rad_f32) != 0u)
        && (ExternalObserverManager_isFinite(Meas_Fado_MechSpeed_rpm_f32) != 0u)) ? 1u : 0u;

    Meas_Fado_Status_u8 = fadoStatus;
    Meas_Fado_Active_u8 = fadoActive;
    Meas_Fado_Valid_u8 = fadoValid;
}

/* Keep the historical symbols as thin aliases for applications that have not
 * yet switched to the generic observer-manager names. */
void FadoExternalObserver_initialize(void)
{
    ExternalObserverManager_initialize();
}

void FadoExternalObserver_reset(void)
{
    ExternalObserverManager_reset();
}

void FadoExternalObserver_setFocEstimator(const FocEstimatorSelector estimator)
{
    ExternalObserverManager_setFocEstimator(estimator);
}

FocEstimatorSelector FadoExternalObserver_getFocEstimator(void)
{
    return ExternalObserverManager_getFocEstimator();
}

uint8_t FadoExternalObserver_getFocEstimate(float *electricalAngle_rad,
                                            float *mechanicalSpeed_rpm)
{
    return ExternalObserverManager_getFocEstimate(electricalAngle_rad, mechanicalSpeed_rpm);
}

void FadoExternalObserver_execute(const float voltageAlpha_V,
                                  const float voltageBeta_V,
                                  const float currentAlpha_A,
                                  const float currentBeta_A)
{
    ExternalObserverManager_execute(voltageAlpha_V, voltageBeta_V, currentAlpha_A, currentBeta_A);
}
