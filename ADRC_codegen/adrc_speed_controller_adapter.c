#include "adrc_speed_controller_adapter.h"

#include <float.h>
#include <math.h>

#define ADRC_SPEED_CONTROLLER_TS_S                 (0.0005F)
#define ADRC_SPEED_CONTROLLER_REF_FILTER_GAIN      (0.1F)
#define ADRC_SPEED_CONTROLLER_Q15_SCALE            (32768.0F)
#define ADRC_SPEED_CONTROLLER_Q15_MAX              (32767.0F)
#define ADRC_SPEED_CONTROLLER_Q15_MIN              (-32768.0F)

typedef struct
{
    float observerGain[3];
    float inputGain[2];
    float controlGain;
    float dampingGain;
    float inverseCriticalGain;
    float referenceFilterState;
    float observerState[3];
    ADRC_SpeedControllerCalibration appliedCalibration;
    uint8_t gainsValid;
} ADRC_SpeedControllerState;

static ADRC_SpeedControllerState adrcSpeedControllerState;

static uint8_t ADRC_SpeedController_isFinite(const float value)
{
    return ((value == value) && (value <= FLT_MAX) && (value >= -FLT_MAX)) ? 1u : 0u;
}

static uint8_t ADRC_SpeedController_validateCalibration(
    const ADRC_SpeedControllerCalibration *calibration)
{
    if (calibration == 0)
    {
        return 0u;
    }

    if ((ADRC_SpeedController_isFinite(calibration->criticalGain_PU_per_PU_s2) == 0u)
        || (ADRC_SpeedController_isFinite(calibration->controlBandwidth_radps) == 0u)
        || (ADRC_SpeedController_isFinite(calibration->observerBandwidth_radps) == 0u))
    {
        return 0u;
    }

    if ((calibration->criticalGain_PU_per_PU_s2 <= 0.0F)
        || (calibration->controlBandwidth_radps <= 0.0F)
        || (calibration->observerBandwidth_radps <= 0.0F)
        || (calibration->iqLowerLimitQ15 > calibration->iqUpperLimitQ15))
    {
        return 0u;
    }

    return 1u;
}

static uint8_t ADRC_SpeedController_calibrationChanged(
    const ADRC_SpeedControllerCalibration *calibration)
{
    return ((adrcSpeedControllerState.gainsValid == 0u)
        || (adrcSpeedControllerState.appliedCalibration.criticalGain_PU_per_PU_s2
            != calibration->criticalGain_PU_per_PU_s2)
        || (adrcSpeedControllerState.appliedCalibration.controlBandwidth_radps
            != calibration->controlBandwidth_radps)
        || (adrcSpeedControllerState.appliedCalibration.observerBandwidth_radps
            != calibration->observerBandwidth_radps)
        || (adrcSpeedControllerState.appliedCalibration.iqUpperLimitQ15
            != calibration->iqUpperLimitQ15)
        || (adrcSpeedControllerState.appliedCalibration.iqLowerLimitQ15
            != calibration->iqLowerLimitQ15)) ? 1u : 0u;
}

static uint8_t ADRC_SpeedController_updateGains(
    const ADRC_SpeedControllerCalibration *calibration)
{
    float observerPole;
    float observerPoleMinusOne;
    float observerPoleMinusOneSquared;
    float observerPoleMinusOneCubed;
    float observerGain0;
    float observerGain1;
    float observerGain2;
    float controlGain;
    float dampingGain;
    float inputGain0;
    float inputGain1;

    if (ADRC_SpeedController_calibrationChanged(calibration) == 0u)
    {
        return 1u;
    }

    observerPole = expf(-calibration->observerBandwidth_radps * ADRC_SPEED_CONTROLLER_TS_S);
    observerPoleMinusOne = 1.0F - observerPole;
    observerPoleMinusOneSquared = observerPoleMinusOne * observerPoleMinusOne;
    observerPoleMinusOneCubed = observerPoleMinusOneSquared * observerPoleMinusOne;

    observerGain0 = 1.0F - observerPole * observerPole * observerPole;
    observerGain1 = (1.5F / ADRC_SPEED_CONTROLLER_TS_S)
        * observerPoleMinusOneSquared * (1.0F + observerPole);
    observerGain2 = observerPoleMinusOneCubed
        / (ADRC_SPEED_CONTROLLER_TS_S * ADRC_SPEED_CONTROLLER_TS_S);
    controlGain = calibration->controlBandwidth_radps * calibration->controlBandwidth_radps;
    dampingGain = 2.0F * calibration->controlBandwidth_radps;
    inputGain0 = 0.5F * ADRC_SPEED_CONTROLLER_TS_S * ADRC_SPEED_CONTROLLER_TS_S
        * calibration->criticalGain_PU_per_PU_s2;
    inputGain1 = ADRC_SPEED_CONTROLLER_TS_S * calibration->criticalGain_PU_per_PU_s2;

    if ((ADRC_SpeedController_isFinite(observerPole) == 0u)
        || (ADRC_SpeedController_isFinite(observerGain0) == 0u)
        || (ADRC_SpeedController_isFinite(observerGain1) == 0u)
        || (ADRC_SpeedController_isFinite(observerGain2) == 0u)
        || (ADRC_SpeedController_isFinite(controlGain) == 0u)
        || (ADRC_SpeedController_isFinite(dampingGain) == 0u)
        || (ADRC_SpeedController_isFinite(inputGain0) == 0u)
        || (ADRC_SpeedController_isFinite(inputGain1) == 0u))
    {
        return 0u;
    }

    adrcSpeedControllerState.observerGain[0] = observerGain0;
    adrcSpeedControllerState.observerGain[1] = observerGain1;
    adrcSpeedControllerState.observerGain[2] = observerGain2;
    adrcSpeedControllerState.inputGain[0] = inputGain0;
    adrcSpeedControllerState.inputGain[1] = inputGain1;
    adrcSpeedControllerState.controlGain = controlGain;
    adrcSpeedControllerState.dampingGain = dampingGain;
    adrcSpeedControllerState.inverseCriticalGain =
        1.0F / calibration->criticalGain_PU_per_PU_s2;
    adrcSpeedControllerState.appliedCalibration = *calibration;
    adrcSpeedControllerState.gainsValid = 1u;

    return ADRC_SpeedController_isFinite(adrcSpeedControllerState.inverseCriticalGain);
}

static Ifx_Math_Fract16 ADRC_SpeedController_puToQ15(const float value)
{
    float scaledValue = value * ADRC_SPEED_CONTROLLER_Q15_SCALE;

    if (scaledValue >= ADRC_SPEED_CONTROLLER_Q15_MAX)
    {
        return (Ifx_Math_Fract16)32767;
    }

    if (scaledValue <= ADRC_SPEED_CONTROLLER_Q15_MIN)
    {
        return (Ifx_Math_Fract16)-32768;
    }

    if (scaledValue >= 0.0F)
    {
        scaledValue += 0.5F;
    }
    else
    {
        scaledValue -= 0.5F;
    }

    return (Ifx_Math_Fract16)((int32_t)scaledValue);
}

static void ADRC_SpeedController_setDiagnostics(
    ADRC_SpeedControllerDiagnostics *diagnostics,
    const float referenceFilteredPU,
    const float outputPU,
    const uint8_t status)
{
    if (diagnostics == 0)
    {
        return;
    }

    diagnostics->referenceFilteredPU = referenceFilteredPU;
    diagnostics->outputPU = outputPU;
    diagnostics->estimatedSpeedPU = adrcSpeedControllerState.observerState[0];
    diagnostics->estimatedAccelerationPU_per_s = adrcSpeedControllerState.observerState[1];
    diagnostics->estimatedDisturbancePU_per_s2 = adrcSpeedControllerState.observerState[2];
    diagnostics->status = status;
}

void ADRC_SpeedController_initialize(void)
{
    ADRC_SpeedController_reset();
}

void ADRC_SpeedController_reset(void)
{
    adrcSpeedControllerState.referenceFilterState = 0.0F;
    adrcSpeedControllerState.observerState[0] = 0.0F;
    adrcSpeedControllerState.observerState[1] = 0.0F;
    adrcSpeedControllerState.observerState[2] = 0.0F;
    adrcSpeedControllerState.gainsValid = 0u;
}

uint8_t ADRC_SpeedController_execute(
    const Ifx_Math_Fract16 referenceSpeedQ15,
    const Ifx_Math_Fract16 measuredSpeedQ15,
    const ADRC_SpeedControllerCalibration *calibration,
    Ifx_Math_Fract16 *iqReferenceQ15,
    ADRC_SpeedControllerDiagnostics *diagnostics)
{
    float referencePU;
    float measuredPU;
    float referenceFilteredPU;
    float observerError;
    float correctedState0;
    float correctedState1;
    float correctedState2;
    float outputPU;
    float initialOutputPU;
    float upperLimitPU;
    float lowerLimitPU;
    float nextState0;
    float nextState1;
    float nextState2;
    Ifx_Math_Fract16 initialIqReferenceQ15;
    uint8_t initializeFromHandoff;

    if (iqReferenceQ15 == 0)
    {
        return 0u;
    }

    initialIqReferenceQ15 = *iqReferenceQ15;
    initializeFromHandoff = (adrcSpeedControllerState.gainsValid == 0u) ? 1u : 0u;
    *iqReferenceQ15 = 0;

    if (ADRC_SpeedController_validateCalibration(calibration) == 0u)
    {
        ADRC_SpeedController_reset();
        ADRC_SpeedController_setDiagnostics(diagnostics, 0.0F, 0.0F,
            ADRC_SPEED_CONTROLLER_STATUS_PARAMETER_INVALID);
        return 0u;
    }

    if (ADRC_SpeedController_updateGains(calibration) == 0u)
    {
        ADRC_SpeedController_reset();
        ADRC_SpeedController_setDiagnostics(diagnostics, 0.0F, 0.0F,
            ADRC_SPEED_CONTROLLER_STATUS_NUMERICAL_INVALID);
        return 0u;
    }

    referencePU = (float)referenceSpeedQ15 / ADRC_SPEED_CONTROLLER_Q15_SCALE;
    measuredPU = (float)measuredSpeedQ15 / ADRC_SPEED_CONTROLLER_Q15_SCALE;
    upperLimitPU = (float)calibration->iqUpperLimitQ15 / ADRC_SPEED_CONTROLLER_Q15_SCALE;
    lowerLimitPU = (float)calibration->iqLowerLimitQ15 / ADRC_SPEED_CONTROLLER_Q15_SCALE;
    if ((ADRC_SpeedController_isFinite(referencePU) == 0u)
        || (ADRC_SpeedController_isFinite(measuredPU) == 0u))
    {
        ADRC_SpeedController_reset();
        ADRC_SpeedController_setDiagnostics(diagnostics, 0.0F, 0.0F,
            ADRC_SPEED_CONTROLLER_STATUS_NUMERICAL_INVALID);
        return 0u;
    }

    /* Fixed source-example reference prefilter: y[k] = y[k-1] + 0.1(r-y[k-1]). */
    referenceFilteredPU = adrcSpeedControllerState.referenceFilterState
        + ADRC_SPEED_CONTROLLER_REF_FILTER_GAIN
        * (referencePU - adrcSpeedControllerState.referenceFilterState);
    adrcSpeedControllerState.referenceFilterState = referenceFilteredPU;

    if (initializeFromHandoff != 0u)
    {
        initialOutputPU = (float)initialIqReferenceQ15 / ADRC_SPEED_CONTROLLER_Q15_SCALE;
        if (initialOutputPU > upperLimitPU)
        {
            initialOutputPU = upperLimitPU;
        }
        else if (initialOutputPU < lowerLimitPU)
        {
            initialOutputPU = lowerLimitPU;
        }

        /* Initialize the ESO around the measured speed and outgoing torque.
         * This makes the normal ADRC equation produce initialOutputPU on the
         * first closed-loop sample instead of stepping down from positioning
         * current to the zero-state ADRC output. */
        adrcSpeedControllerState.observerState[0] = measuredPU;
        adrcSpeedControllerState.observerState[1] = 0.0F;
        adrcSpeedControllerState.observerState[2] =
            ((referenceFilteredPU - measuredPU) * adrcSpeedControllerState.controlGain)
            - (initialOutputPU * calibration->criticalGain_PU_per_PU_s2);

        if (ADRC_SpeedController_isFinite(adrcSpeedControllerState.observerState[2]) == 0u)
        {
            ADRC_SpeedController_reset();
            ADRC_SpeedController_setDiagnostics(diagnostics, 0.0F, 0.0F,
                ADRC_SPEED_CONTROLLER_STATUS_NUMERICAL_INVALID);
            return 0u;
        }
    }

    observerError = measuredPU - adrcSpeedControllerState.observerState[0];
    correctedState0 = adrcSpeedControllerState.observerState[0]
        + adrcSpeedControllerState.observerGain[0] * observerError;
    correctedState1 = adrcSpeedControllerState.observerState[1]
        + adrcSpeedControllerState.observerGain[1] * observerError;
    correctedState2 = adrcSpeedControllerState.observerState[2]
        + adrcSpeedControllerState.observerGain[2] * observerError;

    outputPU = (((referenceFilteredPU - correctedState0)
        * adrcSpeedControllerState.controlGain)
        - (adrcSpeedControllerState.dampingGain * correctedState1)
        - correctedState2) * adrcSpeedControllerState.inverseCriticalGain;

    if (outputPU > upperLimitPU)
    {
        outputPU = upperLimitPU;
    }
    else if (outputPU < lowerLimitPU)
    {
        outputPU = lowerLimitPU;
    }

    nextState0 = correctedState0 + ADRC_SPEED_CONTROLLER_TS_S * correctedState1
        + 0.5F * ADRC_SPEED_CONTROLLER_TS_S * ADRC_SPEED_CONTROLLER_TS_S * correctedState2
        + adrcSpeedControllerState.inputGain[0] * outputPU;
    nextState1 = correctedState1 + ADRC_SPEED_CONTROLLER_TS_S * correctedState2
        + adrcSpeedControllerState.inputGain[1] * outputPU;
    nextState2 = correctedState2;

    if ((ADRC_SpeedController_isFinite(referenceFilteredPU) == 0u)
        || (ADRC_SpeedController_isFinite(outputPU) == 0u)
        || (ADRC_SpeedController_isFinite(nextState0) == 0u)
        || (ADRC_SpeedController_isFinite(nextState1) == 0u)
        || (ADRC_SpeedController_isFinite(nextState2) == 0u))
    {
        ADRC_SpeedController_reset();
        ADRC_SpeedController_setDiagnostics(diagnostics, 0.0F, 0.0F,
            ADRC_SPEED_CONTROLLER_STATUS_NUMERICAL_INVALID);
        return 0u;
    }

    adrcSpeedControllerState.observerState[0] = nextState0;
    adrcSpeedControllerState.observerState[1] = nextState1;
    adrcSpeedControllerState.observerState[2] = nextState2;
    *iqReferenceQ15 = ADRC_SpeedController_puToQ15(outputPU);
    ADRC_SpeedController_setDiagnostics(diagnostics, referenceFilteredPU, outputPU,
        ADRC_SPEED_CONTROLLER_STATUS_VALID);

    return 1u;
}
