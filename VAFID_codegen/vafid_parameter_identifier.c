#include "../ConfigWizard/FocTiming_Cfg.h"
#if FOC_AUX_ALGORITHMS_ENABLE
#include "vafid_parameter_identifier.h"

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

#if defined(__ICCARM__)
#include <intrinsics.h>
#elif (defined(__ARMCC_VERSION) \
    || (defined(__GNUC__) && (defined(__arm__) || defined(__thumb__))))
#include "cy_device_headers.h"
#endif

#if defined(__ICCARM__) || defined(__GNUC__) || defined(__ARMCC_VERSION)
#include "../Utilities/no_opt.h"
#else
#define NO_OPT
#endif

#if (VAFID_FEEDBACK_ALLOWED_MASK != 0u)
#error "Phase-one VAFID firmware must keep feedback hard locked"
#endif

#define VAFID_ROW_COUNT                  (8u)
#define VAFID_COLUMN_COUNT               (3u)
#define VAFID_BUFFER_COUNT               (2u)
#define VAFID_TWO_PI                     (6.283185307179586477F)
#define VAFID_PI                         (3.141592653589793238F)
#define VAFID_HALF_PI                    (1.570796326794896619F)
#define VAFID_Q15_SCALE                  (32768.0F)
#define VAFID_MAX_D_PROBE_Q15            (492)
#define VAFID_MAX_Q_PROBE_Q15            (328)
#define VAFID_MIN_WINDOW_SAMPLES         (200u)
#define VAFID_MAX_WINDOW_SAMPLES         (20000u)
#define VAFID_MAX_SETTLE_SAMPLES         (200000u)
#define VAFID_MIN_RESISTANCE_OHM         (0.01F)
#define VAFID_MAX_RESISTANCE_OHM         (5.0F)
#define VAFID_MIN_INDUCTANCE_H            (0.00005F)
#define VAFID_MAX_INDUCTANCE_H            (0.02F)
#define VAFID_MIN_FLUX_WB                 (0.001F)
#define VAFID_MAX_FLUX_WB                 (0.5F)
#define VAFID_MIN_PROBE_FREQUENCY_HZ      (10.0F)
#define VAFID_MAX_PROBE_FREQUENCY_HZ      (500.0F)
#define VAFID_MIN_PROBE_SEPARATION_HZ     (5.0F)
#define VAFID_MIN_PROBE_AMPLITUDE_Q15     (4)
#define VAFID_MIN_ELECTRICAL_OMEGA_RADPS  (VAFID_TWO_PI * 5.0F)
#define VAFID_MAX_ELECTRICAL_OMEGA_RADPS  (VAFID_TWO_PI * 1000.0F)
#define VAFID_MAX_KRE_ANGLE_RAD           (VAFID_TWO_PI * 2.0F)
#define VAFID_QR_EPSILON                  (1.0e-9F)
#define VAFID_FIT_EPSILON                 (1.0e-12F)
#define VAFID_PARAMETER_FUSION_ALPHA      (0.20F)
#define VAFID_FLUX_FUSION_ALPHA           (0.35F)
#define VAFID_FLUX_FILTER_HZ              (25.0F)
#define VAFID_OMEGA_FILTER_HZ             (25.0F)
#define VAFID_ANGLE_TRACK_HZ              (8.0F)
#define VAFID_MIN_FLUX_FUSION_WINDOWS     (3u)

#if defined(__ICCARM__)
#define VAFID_XCP_SECTION _Pragma("location=\".xcp_cal_m4\"")
#else
#define VAFID_XCP_SECTION
#endif

typedef struct
{
    float sine;
    float cosine;
    float sineStep;
    float cosineStep;
} VAFID_Oscillator;

typedef struct
{
    float sineD;
    float cosineD;
    float sineQ;
    float cosineQ;
    uint8_t collectEligible;
    uint8_t valid;
} VAFID_ProbeRecord;

typedef struct
{
    float alpha_V;
    float beta_V;
    VAFID_ProbeRecord probe;
    uint8_t fresh;
} VAFID_VoltageRecord;

typedef struct
{
    float a[VAFID_ROW_COUNT][VAFID_COLUMN_COUNT];
    float b[VAFID_ROW_COUNT];
    float voltageD_Cos[2];
    float voltageD_Sin[2];
    float voltageQ_Cos[2];
    float voltageQ_Sin[2];
    float currentD_Cos[2];
    float currentD_Sin[2];
    float currentQ_Cos[2];
    float currentQ_Sin[2];
    float activeFluxSum_Wb;
    float idSum_A;
    float omegaSum_radps;
    uint32_t sampleCount;
    uint32_t completedFastTick;
    uint32_t completedWindow;
    uint32_t collectionGeneration;
    uint8_t allFluxQualified;
    volatile uint8_t ready;
} VAFID_LockinBuffer;

typedef struct
{
    int16_t amplitudeD_Q15;
    int16_t amplitudeQ_Q15;
    float frequencyD_Hz;
    float frequencyQ_Hz;
    uint16_t settle_ms;
    uint16_t window_ms;
    uint8_t consistentWindows;
    uint16_t consistencyTolerance_pct;
    uint16_t maximumFitError_pct;
    float minimumConditionProxy;
    uint16_t stale_ms;
} VAFID_RawCalibration;

typedef struct
{
    int16_t amplitudeD_Q15;
    int16_t amplitudeQ_Q15;
    float frequencyD_Hz;
    float frequencyQ_Hz;
    uint32_t settleSamples;
    uint32_t windowSamples;
    uint32_t staleSamples;
    uint8_t consistentWindows;
    float consistencyTolerance;
    float maximumFitError;
    float minimumConditionProxy;
    float sineStepD;
    float cosineStepD;
    float sineStepQ;
    float cosineStepQ;
    float activeFluxFilterAlpha;
    float omegaFilterAlpha;
    float angleTrackAlpha;
    float envelopeStep;
} VAFID_RuntimeConfiguration;

typedef struct
{
    VAFID_NominalParameters nominal;
    VAFID_RuntimeConfiguration configuration;
    VAFID_RawCalibration appliedCalibration;
    VAFID_FastEligibility eligibility;
    VAFID_Oscillator oscillatorD;
    VAFID_Oscillator oscillatorQ;
    VAFID_ProbeRecord pendingProbe;
    VAFID_VoltageRecord capturedVoltage;
    VAFID_LockinBuffer buffer[VAFID_BUFFER_COUNT];
    VAFID_Estimate candidate;
    VAFID_Estimate previousCandidate;
    VAFID_Estimate published;
    float activeFluxFiltered_Wb;
    float activeFluxFilterAlpha;
    float filteredOmega_radps;
    float omegaFilterAlpha;
    float trackedAngle_rad;
    float angleTrackAlpha;
    float fusedFlux_Wb;
    float envelope;
    float envelopeStep;
    uint32_t settleRemaining;
    uint32_t fastTick;
    uint32_t completedWindows;
    uint32_t lastAcceptedTick;
    uint32_t collectionGeneration;
    uint32_t lastDtcSignature;
    uint16_t lastAdcSampleOffset;
    uint16_t rejectMask;
    uint8_t writeBuffer;
    uint8_t nominalConfigured;
    uint8_t runtimeCalibrationValid;
    uint8_t appliedCalibrationValid;
    uint8_t eligibilitySnapshotValid;
    uint8_t fastEligible;
    uint8_t observerOperatingPointValid;
    uint8_t previousMode;
    uint8_t consecutiveWindows;
    uint8_t previousCandidateValid;
    uint8_t parameterFusionInitialized;
    uint8_t fluxFilterInitialized;
    uint8_t frameTrackerInitialized;
    uint8_t fluxFusionInitialized;
    uint8_t acceptedFluxWindows;
    uint8_t previousResetRequest;
    uint8_t previousApplyRequest;
    uint8_t previousRevertRequest;
    uint8_t captureExpected;
} VAFID_State;

static VAFID_State vafidState;

#if defined(__ICCARM__)
typedef __istate_t VAFID_CriticalState;

static VAFID_CriticalState VAFID_enterCritical(void)
{
    VAFID_CriticalState state = __get_interrupt_state();
    __disable_interrupt();
    return state;
}

static void VAFID_exitCritical(VAFID_CriticalState state)
{
    __set_interrupt_state(state);
}
#elif (defined(__ARMCC_VERSION) \
    || (defined(__GNUC__) && (defined(__arm__) || defined(__thumb__))))
typedef uint32_t VAFID_CriticalState;

static VAFID_CriticalState VAFID_enterCritical(void)
{
    VAFID_CriticalState state = __get_PRIMASK();
    __disable_irq();
    return state;
}

static void VAFID_exitCritical(VAFID_CriticalState state)
{
    if (state == 0u)
    {
        __enable_irq();
    }
}
#else
/* Host replay is single threaded. Production Cortex-M builds use one of the
 * interrupt-preserving implementations above. */
typedef uint32_t VAFID_CriticalState;

static VAFID_CriticalState VAFID_enterCritical(void)
{
    return 0u;
}

static void VAFID_exitCritical(VAFID_CriticalState state)
{
    (void)state;
}
#endif

VAFID_XCP_SECTION NO_OPT volatile uint8_t Cal_VAFID_Mode_u8 = VAFID_MODE_OFF;
VAFID_XCP_SECTION NO_OPT volatile uint8_t Cal_VAFID_Rst_u8 = 0u;
VAFID_XCP_SECTION NO_OPT volatile int16_t Cal_VAFID_Ad_Q15_s16 = 164;
VAFID_XCP_SECTION NO_OPT volatile int16_t Cal_VAFID_Aq_Q15_s16 = 164;
VAFID_XCP_SECTION NO_OPT volatile float Cal_VAFID_Fd_Hz_f32 = 150.0F;
VAFID_XCP_SECTION NO_OPT volatile float Cal_VAFID_Fq_Hz_f32 = 220.0F;
VAFID_XCP_SECTION NO_OPT volatile uint16_t Cal_VAFID_Settle_ms_u16 = 250u;
VAFID_XCP_SECTION NO_OPT volatile uint16_t Cal_VAFID_Window_ms_u16 = 100u;
VAFID_XCP_SECTION NO_OPT volatile uint8_t Cal_VAFID_ConsWin_u8 = 10u;
VAFID_XCP_SECTION NO_OPT volatile uint16_t Cal_VAFID_ConsTol_pct_u16 = 30u;
VAFID_XCP_SECTION NO_OPT volatile uint16_t Cal_VAFID_FitHi_pct_u16 = 65u;
/* Reciprocal 2-norm condition estimate. 1e-4 enforces cond(A)<1e4. */
VAFID_XCP_SECTION NO_OPT volatile float Cal_VAFID_CondLo_f32 = 0.0001F;
VAFID_XCP_SECTION NO_OPT volatile uint16_t Cal_VAFID_Stale_ms_u16 = 300u;
VAFID_XCP_SECTION NO_OPT volatile uint8_t Cal_VAFID_FbMask_u8 = 0u;
VAFID_XCP_SECTION NO_OPT volatile uint8_t Cal_VAFID_Apply_u8 = 0u;
VAFID_XCP_SECTION NO_OPT volatile uint8_t Cal_VAFID_Revert_u8 = 0u;

VAFID_XCP_SECTION NO_OPT volatile uint8_t Meas_VAFID_Act_u8 = 0u;
VAFID_XCP_SECTION NO_OPT volatile uint8_t Meas_VAFID_Stat_u8 = VAFID_STATUS_OFF;
VAFID_XCP_SECTION NO_OPT volatile uint8_t Meas_VAFID_ValidMask_u8 = 0u;
VAFID_XCP_SECTION NO_OPT volatile uint16_t Meas_VAFID_RejectMask_u16 = 0u;
VAFID_XCP_SECTION NO_OPT volatile int16_t Meas_VAFID_ProbeD_Q15_s16 = 0;
VAFID_XCP_SECTION NO_OPT volatile int16_t Meas_VAFID_ProbeQ_Q15_s16 = 0;
VAFID_XCP_SECTION NO_OPT volatile uint8_t Meas_VAFID_ProbeClip_u8 = 0u;
VAFID_XCP_SECTION NO_OPT volatile float Meas_VAFID_Rs_Ohm_f32 = 0.0F;
VAFID_XCP_SECTION NO_OPT volatile float Meas_VAFID_Ld_H_f32 = 0.0F;
VAFID_XCP_SECTION NO_OPT volatile float Meas_VAFID_Lq_H_f32 = 0.0F;
VAFID_XCP_SECTION NO_OPT volatile float Meas_VAFID_FluxPM_Wb_f32 = 0.0F;
VAFID_XCP_SECTION NO_OPT volatile float Meas_VAFID_ActFluxFlt_Wb_f32 = 0.0F;
VAFID_XCP_SECTION NO_OPT volatile float Meas_VAFID_IdMean_A_f32 = 0.0F;
VAFID_XCP_SECTION NO_OPT volatile float Meas_VAFID_Fit_f32 = 0.0F;
VAFID_XCP_SECTION NO_OPT volatile float Meas_VAFID_Cond_f32 = 0.0F;
VAFID_XCP_SECTION NO_OPT volatile uint32_t Meas_VAFID_Win_u32 = 0u;
VAFID_XCP_SECTION NO_OPT volatile uint8_t Meas_VAFID_ConsWin_u8 = 0u;
VAFID_XCP_SECTION NO_OPT volatile uint8_t Meas_VAFID_FbResult_u8 = VAFID_FEEDBACK_RESULT_NONE;

static uint8_t VAFID_isFinite(float value)
{
    return ((value == value) && (value <= FLT_MAX) && (value >= -FLT_MAX)) ? 1u : 0u;
}

static float VAFID_absolute(float value)
{
    return (value < 0.0F) ? -value : value;
}

static int16_t VAFID_roundSaturateS16(float value)
{
    if (VAFID_isFinite(value) == 0u)
    {
        return 0;
    }
    if (value >= 32767.0F)
    {
        return 32767;
    }
    if (value <= -32768.0F)
    {
        return (int16_t)-32768;
    }
    value += (value >= 0.0F) ? 0.5F : -0.5F;
    return (int16_t)((int32_t)value);
}

static uint32_t VAFID_integerSquareRoot(uint32_t value)
{
    uint32_t result = 0u;
    uint32_t bit = UINT32_C(1) << 30u;

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

static void VAFID_sineCosine(float angle_rad, float *sine, float *cosine)
{
    float x = angle_rad;
    float x2;
    float cosineSign = 1.0F;

    if (x > VAFID_PI)
    {
        x -= VAFID_TWO_PI;
    }
    if (x < -VAFID_PI)
    {
        x += VAFID_TWO_PI;
    }
    if (x > VAFID_HALF_PI)
    {
        x = VAFID_PI - x;
        cosineSign = -1.0F;
    }
    else if (x < -VAFID_HALF_PI)
    {
        x = -VAFID_PI - x;
        cosineSign = -1.0F;
    }

    x2 = x * x;
    *sine = x * (1.0F + x2 * (-0.1666666716F
        + x2 * (0.0083333338F + x2 * (-0.0001984127F))));
    *cosine = cosineSign * (1.0F + x2 * (-0.5F
        + x2 * (0.0416666679F + x2 * (-0.0013888889F
        + x2 * 0.0000248016F))));
}

static float VAFID_wrapAngle(float angle_rad)
{
    /* All fast-path callers validate their source angle to a narrow finite
     * interval before reaching here. Two fixed corrections also cover the
     * one-sample prediction at the maximum supported electrical speed. */
    if (angle_rad > VAFID_PI)
    {
        angle_rad -= VAFID_TWO_PI;
    }
    if (angle_rad > VAFID_PI)
    {
        angle_rad -= VAFID_TWO_PI;
    }
    if (angle_rad < -VAFID_PI)
    {
        angle_rad += VAFID_TWO_PI;
    }
    if (angle_rad < -VAFID_PI)
    {
        angle_rad += VAFID_TWO_PI;
    }
    return angle_rad;
}

static void VAFID_clearBuffer(VAFID_LockinBuffer *buffer)
{
    memset(buffer->a, 0, sizeof(buffer->a));
    memset(buffer->b, 0, sizeof(buffer->b));
    memset(buffer->voltageD_Cos, 0, sizeof(buffer->voltageD_Cos));
    memset(buffer->voltageD_Sin, 0, sizeof(buffer->voltageD_Sin));
    memset(buffer->voltageQ_Cos, 0, sizeof(buffer->voltageQ_Cos));
    memset(buffer->voltageQ_Sin, 0, sizeof(buffer->voltageQ_Sin));
    memset(buffer->currentD_Cos, 0, sizeof(buffer->currentD_Cos));
    memset(buffer->currentD_Sin, 0, sizeof(buffer->currentD_Sin));
    memset(buffer->currentQ_Cos, 0, sizeof(buffer->currentQ_Cos));
    memset(buffer->currentQ_Sin, 0, sizeof(buffer->currentQ_Sin));
    buffer->activeFluxSum_Wb = 0.0F;
    buffer->idSum_A = 0.0F;
    buffer->omegaSum_radps = 0.0F;
    buffer->sampleCount = 0u;
    buffer->completedFastTick = 0u;
    buffer->completedWindow = 0u;
    buffer->collectionGeneration = 0u;
    buffer->allFluxQualified = 1u;
    buffer->ready = 0u;
}

static void VAFID_resetOscillators(void)
{
    vafidState.oscillatorD.sine = 0.0F;
    vafidState.oscillatorD.cosine = 1.0F;
    vafidState.oscillatorQ.sine = 0.0F;
    vafidState.oscillatorQ.cosine = 1.0F;
}

static void VAFID_restartCollectionUnlocked(uint16_t rejectMask)
{
    uint8_t index;

    vafidState.collectionGeneration++;
    for (index = 0u; index < VAFID_BUFFER_COUNT; index++)
    {
        VAFID_clearBuffer(&vafidState.buffer[index]);
    }
    vafidState.writeBuffer = 0u;
    vafidState.settleRemaining = vafidState.configuration.settleSamples;
    vafidState.envelope = 0.0F;
    vafidState.capturedVoltage.fresh = 0u;
    vafidState.pendingProbe.valid = 0u;
    vafidState.captureExpected = 0u;
    vafidState.consecutiveWindows = 0u;
    vafidState.previousCandidateValid = 0u;
    vafidState.fluxFilterInitialized = 0u;
    vafidState.frameTrackerInitialized = 0u;
    vafidState.acceptedFluxWindows = 0u;
    vafidState.published.validMask = 0u;
    vafidState.rejectMask = rejectMask;
    VAFID_resetOscillators();

    Meas_VAFID_ValidMask_u8 = 0u;
    Meas_VAFID_ConsWin_u8 = 0u;
    Meas_VAFID_RejectMask_u16 = rejectMask;
    if (rejectMask != 0u)
    {
        Meas_VAFID_Stat_u8 = VAFID_STATUS_WINDOW_REJECTED;
    }
}

static void VAFID_restartCollection(uint16_t rejectMask)
{
    VAFID_CriticalState criticalState = VAFID_enterCritical();
    VAFID_restartCollectionUnlocked(rejectMask);
    VAFID_exitCritical(criticalState);
}

static uint8_t VAFID_parametersFiniteAndPositive(const VAFID_NominalParameters *nominal)
{
    if ((nominal == NULL)
        || (VAFID_isFinite(nominal->sampleTime_s) == 0u)
        || (VAFID_isFinite(nominal->currentBase_A) == 0u)
        || (VAFID_isFinite(nominal->voltageBase_V) == 0u)
        || (VAFID_isFinite(nominal->resistance_Ohm) == 0u)
        || (VAFID_isFinite(nominal->inductanceD_H) == 0u)
        || (VAFID_isFinite(nominal->inductanceQ_H) == 0u)
        || (VAFID_isFinite(nominal->permanentMagnetFlux_Wb) == 0u))
    {
        return 0u;
    }
    return ((nominal->sampleTime_s > 0.0F)
        && (nominal->currentBase_A > 0.0F)
        && (nominal->voltageBase_V > 0.0F)
        && (nominal->resistance_Ohm > 0.0F)
        && (nominal->inductanceD_H > 0.0F)
        && (nominal->inductanceQ_H > 0.0F)
        && (nominal->permanentMagnetFlux_Wb > 0.0F)) ? 1u : 0u;
}

static void VAFID_readRawCalibration(VAFID_RawCalibration *calibration)
{
    calibration->amplitudeD_Q15 = Cal_VAFID_Ad_Q15_s16;
    calibration->amplitudeQ_Q15 = Cal_VAFID_Aq_Q15_s16;
    calibration->frequencyD_Hz = Cal_VAFID_Fd_Hz_f32;
    calibration->frequencyQ_Hz = Cal_VAFID_Fq_Hz_f32;
    calibration->settle_ms = Cal_VAFID_Settle_ms_u16;
    calibration->window_ms = Cal_VAFID_Window_ms_u16;
    calibration->consistentWindows = Cal_VAFID_ConsWin_u8;
    calibration->consistencyTolerance_pct = Cal_VAFID_ConsTol_pct_u16;
    calibration->maximumFitError_pct = Cal_VAFID_FitHi_pct_u16;
    calibration->minimumConditionProxy = Cal_VAFID_CondLo_f32;
    calibration->stale_ms = Cal_VAFID_Stale_ms_u16;
}

static uint8_t VAFID_rawCalibrationEqual(const VAFID_RawCalibration *left,
                                         const VAFID_RawCalibration *right)
{
    return ((left->amplitudeD_Q15 == right->amplitudeD_Q15)
        && (left->amplitudeQ_Q15 == right->amplitudeQ_Q15)
        && (left->frequencyD_Hz == right->frequencyD_Hz)
        && (left->frequencyQ_Hz == right->frequencyQ_Hz)
        && (left->settle_ms == right->settle_ms)
        && (left->window_ms == right->window_ms)
        && (left->consistentWindows == right->consistentWindows)
        && (left->consistencyTolerance_pct == right->consistencyTolerance_pct)
        && (left->maximumFitError_pct == right->maximumFitError_pct)
        && (left->minimumConditionProxy == right->minimumConditionProxy)
        && (left->stale_ms == right->stale_ms)) ? 1u : 0u;
}

static uint8_t VAFID_deriveConfiguration(VAFID_RuntimeConfiguration *configuration,
                                         const VAFID_RawCalibration *calibration,
                                         const VAFID_NominalParameters *nominal)
{
    float samplesPerMillisecond;
    float nyquistFrequency;
    float angleStepD;
    float angleStepQ;
    float filterAlpha;
    uint32_t sampleCount;

    if ((configuration == NULL) || (calibration == NULL)
        || (VAFID_parametersFiniteAndPositive(nominal) == 0u))
    {
        return 0u;
    }
    if ((calibration->amplitudeD_Q15 < VAFID_MIN_PROBE_AMPLITUDE_Q15)
        || (calibration->amplitudeD_Q15 > VAFID_MAX_D_PROBE_Q15)
        || (calibration->amplitudeQ_Q15 < VAFID_MIN_PROBE_AMPLITUDE_Q15)
        || (calibration->amplitudeQ_Q15 > VAFID_MAX_Q_PROBE_Q15)
        || (VAFID_isFinite(calibration->frequencyD_Hz) == 0u)
        || (VAFID_isFinite(calibration->frequencyQ_Hz) == 0u)
        || (VAFID_isFinite(calibration->minimumConditionProxy) == 0u)
        || (calibration->consistentWindows == 0u)
        || (calibration->consistencyTolerance_pct == 0u)
        || (calibration->maximumFitError_pct == 0u))
    {
        return 0u;
    }

    nyquistFrequency = 0.5F / nominal->sampleTime_s;
    if ((calibration->frequencyD_Hz < VAFID_MIN_PROBE_FREQUENCY_HZ)
        || (calibration->frequencyD_Hz > VAFID_MAX_PROBE_FREQUENCY_HZ)
        || (calibration->frequencyQ_Hz < VAFID_MIN_PROBE_FREQUENCY_HZ)
        || (calibration->frequencyQ_Hz > VAFID_MAX_PROBE_FREQUENCY_HZ)
        || (calibration->frequencyD_Hz >= nyquistFrequency)
        || (calibration->frequencyQ_Hz >= nyquistFrequency)
        || (VAFID_absolute(calibration->frequencyD_Hz
        - calibration->frequencyQ_Hz) < VAFID_MIN_PROBE_SEPARATION_HZ)
        || (calibration->minimumConditionProxy <= 0.0F)
        || (calibration->minimumConditionProxy >= 1.0F))
    {
        return 0u;
    }

    samplesPerMillisecond = 0.001F / nominal->sampleTime_s;
    sampleCount = (uint32_t)((float)calibration->window_ms
        * samplesPerMillisecond + 0.5F);
    if ((sampleCount < VAFID_MIN_WINDOW_SAMPLES)
        || (sampleCount > VAFID_MAX_WINDOW_SAMPLES))
    {
        return 0u;
    }
    configuration->windowSamples = sampleCount;

    sampleCount = (uint32_t)((float)calibration->settle_ms
        * samplesPerMillisecond + 0.5F);
    if (sampleCount > VAFID_MAX_SETTLE_SAMPLES)
    {
        return 0u;
    }
    configuration->settleSamples = sampleCount;
    configuration->staleSamples = (uint32_t)((float)calibration->stale_ms
        * samplesPerMillisecond + 0.5F);
    if (configuration->staleSamples < configuration->windowSamples)
    {
        configuration->staleSamples = configuration->windowSamples;
    }

    configuration->amplitudeD_Q15 = calibration->amplitudeD_Q15;
    configuration->amplitudeQ_Q15 = calibration->amplitudeQ_Q15;
    configuration->frequencyD_Hz = calibration->frequencyD_Hz;
    configuration->frequencyQ_Hz = calibration->frequencyQ_Hz;
    configuration->consistentWindows = calibration->consistentWindows;
    configuration->consistencyTolerance = 0.01F
        * (float)calibration->consistencyTolerance_pct;
    configuration->maximumFitError = 0.01F
        * (float)calibration->maximumFitError_pct;
    configuration->minimumConditionProxy = calibration->minimumConditionProxy;

    angleStepD = VAFID_TWO_PI * configuration->frequencyD_Hz * nominal->sampleTime_s;
    angleStepQ = VAFID_TWO_PI * configuration->frequencyQ_Hz * nominal->sampleTime_s;
    configuration->sineStepD = sinf(angleStepD);
    configuration->cosineStepD = cosf(angleStepD);
    configuration->sineStepQ = sinf(angleStepQ);
    configuration->cosineStepQ = cosf(angleStepQ);
    filterAlpha = VAFID_TWO_PI * VAFID_FLUX_FILTER_HZ * nominal->sampleTime_s;
    configuration->activeFluxFilterAlpha = (filterAlpha < 1.0F)
        ? filterAlpha : 1.0F;
    filterAlpha = VAFID_TWO_PI * VAFID_OMEGA_FILTER_HZ * nominal->sampleTime_s;
    configuration->omegaFilterAlpha = (filterAlpha < 1.0F)
        ? filterAlpha : 1.0F;
    filterAlpha = VAFID_TWO_PI * VAFID_ANGLE_TRACK_HZ * nominal->sampleTime_s;
    configuration->angleTrackAlpha = (filterAlpha < 1.0F)
        ? filterAlpha : 1.0F;
    configuration->envelopeStep = (configuration->settleSamples != 0u)
        ? (1.0F / (float)configuration->settleSamples) : 1.0F;
    return 1u;
}

static void VAFID_seedPublishedFromNominalUnlocked(void)
{
    vafidState.published.resistance_Ohm = vafidState.nominal.resistance_Ohm;
    vafidState.published.inductanceD_H = vafidState.nominal.inductanceD_H;
    vafidState.published.inductanceQ_H = vafidState.nominal.inductanceQ_H;
    vafidState.published.permanentMagnetFlux_Wb =
        vafidState.nominal.permanentMagnetFlux_Wb;
    vafidState.fusedFlux_Wb = vafidState.nominal.permanentMagnetFlux_Wb;
    vafidState.parameterFusionInitialized = 1u;
    vafidState.fluxFusionInitialized = 1u;
}

static void VAFID_commitConfigurationUnlocked(
    const VAFID_RuntimeConfiguration *configuration,
    const VAFID_RawCalibration *calibration,
    uint16_t restartReason)
{
    vafidState.configuration = *configuration;
    vafidState.appliedCalibration = *calibration;
    vafidState.appliedCalibrationValid = 1u;
    vafidState.oscillatorD.sineStep = configuration->sineStepD;
    vafidState.oscillatorD.cosineStep = configuration->cosineStepD;
    vafidState.oscillatorQ.sineStep = configuration->sineStepQ;
    vafidState.oscillatorQ.cosineStep = configuration->cosineStepQ;
    vafidState.activeFluxFilterAlpha = configuration->activeFluxFilterAlpha;
    vafidState.omegaFilterAlpha = configuration->omegaFilterAlpha;
    vafidState.angleTrackAlpha = configuration->angleTrackAlpha;
    vafidState.envelopeStep = configuration->envelopeStep;
    vafidState.runtimeCalibrationValid = 1u;
    VAFID_restartCollectionUnlocked(restartReason);
    if (vafidState.parameterFusionInitialized == 0u)
    {
        VAFID_seedPublishedFromNominalUnlocked();
    }
}

static uint8_t VAFID_allEligibilityTrue(const VAFID_FastEligibility *eligibility)
{
    return ((eligibility->kreClosedLoop != 0u)
        && (eligibility->apsfsmOff != 0u)
        && (eligibility->hfiOff != 0u)
        && (eligibility->rrcOutputInactive != 0u)
        && (eligibility->voltagePathStable != 0u)
        && (eligibility->overmodulationActive == 0u)
        && (vafidState.observerOperatingPointValid != 0u)) ? 1u : 0u;
}

static void VAFID_advanceOscillator(VAFID_Oscillator *oscillator)
{
    float newSine = (oscillator->sine * oscillator->cosineStep)
        + (oscillator->cosine * oscillator->sineStep);
    float newCosine = (oscillator->cosine * oscillator->cosineStep)
        - (oscillator->sine * oscillator->sineStep);
    oscillator->sine = newSine;
    oscillator->cosine = newCosine;
}

static int16_t VAFID_clampS16(int32_t value, int16_t lower, int16_t upper,
                              uint8_t *clipped)
{
    if (value > (int32_t)upper)
    {
        *clipped = 1u;
        return upper;
    }
    if (value < (int32_t)lower)
    {
        *clipped = 1u;
        return lower;
    }
    return (int16_t)value;
}

static void VAFID_accumulatePhasor(VAFID_LockinBuffer *buffer,
                                   uint8_t frequencyIndex,
                                   float cosineReference,
                                   float sineReference,
                                   float voltageD_V,
                                   float voltageQ_V,
                                   float currentD_A,
                                   float currentQ_A)
{
    buffer->voltageD_Cos[frequencyIndex] += voltageD_V * cosineReference;
    buffer->voltageD_Sin[frequencyIndex] += voltageD_V * sineReference;
    buffer->voltageQ_Cos[frequencyIndex] += voltageQ_V * cosineReference;
    buffer->voltageQ_Sin[frequencyIndex] += voltageQ_V * sineReference;
    buffer->currentD_Cos[frequencyIndex] += currentD_A * cosineReference;
    buffer->currentD_Sin[frequencyIndex] += currentD_A * sineReference;
    buffer->currentQ_Cos[frequencyIndex] += currentQ_A * cosineReference;
    buffer->currentQ_Sin[frequencyIndex] += currentQ_A * sineReference;
}

static uint8_t VAFID_buildRegression(VAFID_LockinBuffer *buffer,
                                     const VAFID_RuntimeConfiguration *configuration)
{
    float inverseSamples;
    float phasorScale;
    float omegaMean;
    uint8_t frequencyIndex;

    if (buffer->sampleCount == 0u)
    {
        return 0u;
    }
    inverseSamples = 1.0F / (float)buffer->sampleCount;
    phasorScale = 2.0F * inverseSamples;
    omegaMean = buffer->omegaSum_radps * inverseSamples;
    if (VAFID_isFinite(omegaMean) == 0u)
    {
        return 0u;
    }

    for (frequencyIndex = 0u; frequencyIndex < 2u; frequencyIndex++)
    {
        float injectionOmega = VAFID_TWO_PI
            * ((frequencyIndex == 0u) ? configuration->frequencyD_Hz
                                      : configuration->frequencyQ_Hz);
        float idReal = phasorScale * buffer->currentD_Cos[frequencyIndex];
        float idImag = -phasorScale * buffer->currentD_Sin[frequencyIndex];
        float iqReal = phasorScale * buffer->currentQ_Cos[frequencyIndex];
        float iqImag = -phasorScale * buffer->currentQ_Sin[frequencyIndex];
        float vdReal = phasorScale * buffer->voltageD_Cos[frequencyIndex];
        float vdImag = -phasorScale * buffer->voltageD_Sin[frequencyIndex];
        float vqReal = phasorScale * buffer->voltageQ_Cos[frequencyIndex];
        float vqImag = -phasorScale * buffer->voltageQ_Sin[frequencyIndex];
        uint8_t baseRow = (uint8_t)(frequencyIndex * 4u);

        /* Complex d-axis row:
         * Vd = Rs*Id + j*Omega*Ld*Id - omegaMean*Lq*Iq. */
        buffer->a[baseRow][0] = idReal;
        buffer->a[baseRow][1] = -injectionOmega * idImag;
        buffer->a[baseRow][2] = -omegaMean * iqReal;
        buffer->b[baseRow] = vdReal;
        buffer->a[baseRow + 1u][0] = idImag;
        buffer->a[baseRow + 1u][1] = injectionOmega * idReal;
        buffer->a[baseRow + 1u][2] = -omegaMean * iqImag;
        buffer->b[baseRow + 1u] = vdImag;

        /* Complex q-axis row:
         * Vq = Rs*Iq + omegaMean*Ld*Id + j*Omega*Lq*Iq.
         * The PM/active-flux term is absent from the probe-frequency
         * regression and is used only for the separate FluxPM candidate. */
        buffer->a[baseRow + 2u][0] = iqReal;
        buffer->a[baseRow + 2u][1] = omegaMean * idReal;
        buffer->a[baseRow + 2u][2] = -injectionOmega * iqImag;
        buffer->b[baseRow + 2u] = vqReal;
        buffer->a[baseRow + 3u][0] = iqImag;
        buffer->a[baseRow + 3u][1] = omegaMean * idImag;
        buffer->a[baseRow + 3u][2] = injectionOmega * iqReal;
        buffer->b[baseRow + 3u] = vqImag;
    }
    return 1u;
}

static void VAFID_finishWindow(void)
{
    VAFID_LockinBuffer *completed = &vafidState.buffer[vafidState.writeBuffer];
    uint8_t nextBuffer = (uint8_t)(vafidState.writeBuffer ^ 1u);

    if (vafidState.buffer[nextBuffer].ready != 0u)
    {
        VAFID_restartCollection(VAFID_REJECT_STALE);
        return;
    }

    vafidState.completedWindows++;
    completed->completedFastTick = vafidState.fastTick;
    completed->completedWindow = vafidState.completedWindows;
    completed->collectionGeneration = vafidState.collectionGeneration;
    completed->ready = 1u;
    Meas_VAFID_Win_u32 = vafidState.completedWindows;
    Meas_VAFID_Stat_u8 = VAFID_STATUS_WINDOW_READY;

    vafidState.writeBuffer = nextBuffer;
    VAFID_clearBuffer(&vafidState.buffer[nextBuffer]);
}

static uint8_t VAFID_reciprocalCondition(
    const VAFID_LockinBuffer *buffer, float *conditionProxy)
{
    float gram[VAFID_COLUMN_COUNT][VAFID_COLUMN_COUNT] = {{0.0F}};
    float maximumEntry = 0.0F;
    float minimumEigenvalue;
    float maximumEigenvalue;
    uint8_t row;
    uint8_t column;
    uint8_t otherColumn;
    uint8_t sweep;

    /* A common scale preserves cond2(A) and keeps A'*A in a safe float
     * range. Fixed-sweep Jacobi then estimates the three singular values. */
    for (row = 0u; row < VAFID_ROW_COUNT; row++)
    {
        for (column = 0u; column < VAFID_COLUMN_COUNT; column++)
        {
            float magnitude = VAFID_absolute(buffer->a[row][column]);
            if (magnitude > maximumEntry)
            {
                maximumEntry = magnitude;
            }
        }
    }
    if ((VAFID_isFinite(maximumEntry) == 0u)
        || (maximumEntry <= VAFID_QR_EPSILON))
    {
        return 0u;
    }
    for (row = 0u; row < VAFID_ROW_COUNT; row++)
    {
        for (column = 0u; column < VAFID_COLUMN_COUNT; column++)
        {
            float left = buffer->a[row][column] / maximumEntry;
            for (otherColumn = column; otherColumn < VAFID_COLUMN_COUNT;
                 otherColumn++)
            {
                gram[column][otherColumn] += left
                    * (buffer->a[row][otherColumn] / maximumEntry);
            }
        }
    }
    for (column = 0u; column < VAFID_COLUMN_COUNT; column++)
    {
        for (otherColumn = (uint8_t)(column + 1u);
             otherColumn < VAFID_COLUMN_COUNT; otherColumn++)
        {
            gram[otherColumn][column] = gram[column][otherColumn];
        }
    }

    for (sweep = 0u; sweep < 8u; sweep++)
    {
        static const uint8_t pairP[3] = {0u, 0u, 1u};
        static const uint8_t pairQ[3] = {1u, 2u, 2u};
        uint8_t pair;
        for (pair = 0u; pair < 3u; pair++)
        {
            uint8_t p = pairP[pair];
            uint8_t q = pairQ[pair];
            float apq = gram[p][q];
            if (VAFID_absolute(apq) > 1.0e-12F)
            {
                float app = gram[p][p];
                float aqq = gram[q][q];
                float tau = (aqq - app) / (2.0F * apq);
                float t = ((tau >= 0.0F) ? 1.0F : -1.0F)
                    / (VAFID_absolute(tau) + sqrtf(1.0F + (tau * tau)));
                float c = 1.0F / sqrtf(1.0F + (t * t));
                float s = t * c;
                uint8_t k;

                for (k = 0u; k < VAFID_COLUMN_COUNT; k++)
                {
                    if ((k != p) && (k != q))
                    {
                        float gkp = gram[k][p];
                        float gkq = gram[k][q];
                        gram[k][p] = (c * gkp) - (s * gkq);
                        gram[p][k] = gram[k][p];
                        gram[k][q] = (s * gkp) + (c * gkq);
                        gram[q][k] = gram[k][q];
                    }
                }
                gram[p][p] = app - (t * apq);
                gram[q][q] = aqq + (t * apq);
                gram[p][q] = 0.0F;
                gram[q][p] = 0.0F;
            }
        }
    }

    minimumEigenvalue = gram[0][0];
    maximumEigenvalue = gram[0][0];
    for (column = 1u; column < VAFID_COLUMN_COUNT; column++)
    {
        if (gram[column][column] < minimumEigenvalue)
        {
            minimumEigenvalue = gram[column][column];
        }
        if (gram[column][column] > maximumEigenvalue)
        {
            maximumEigenvalue = gram[column][column];
        }
    }
    if ((VAFID_isFinite(minimumEigenvalue) == 0u)
        || (VAFID_isFinite(maximumEigenvalue) == 0u)
        || (maximumEigenvalue <= VAFID_FIT_EPSILON))
    {
        return 0u;
    }
    if (minimumEigenvalue <= 0.0F)
    {
        *conditionProxy = 0.0F;
    }
    else
    {
        *conditionProxy = sqrtf(minimumEigenvalue / maximumEigenvalue);
    }
    return (VAFID_isFinite(*conditionProxy) != 0u) ? 1u : 0u;
}

static uint8_t VAFID_solveEightByThree(const VAFID_LockinBuffer *buffer,
                                       float result[VAFID_COLUMN_COUNT],
                                       float *fitError,
                                       float *conditionProxy)
{
    float normalized[VAFID_ROW_COUNT][VAFID_COLUMN_COUNT];
    float orthogonal[VAFID_ROW_COUNT][VAFID_COLUMN_COUNT];
    float upper[VAFID_COLUMN_COUNT][VAFID_COLUMN_COUNT] = {{0.0F}};
    float projected[VAFID_COLUMN_COUNT] = {0.0F};
    float scaledResult[VAFID_COLUMN_COUNT] = {0.0F};
    float columnScale[VAFID_COLUMN_COUNT];
    float residualEnergy = 0.0F;
    float outputEnergy = 0.0F;
    uint8_t row;
    uint8_t column;
    uint8_t previous;

    for (column = 0u; column < VAFID_COLUMN_COUNT; column++)
    {
        float scale = 0.0F;
        for (row = 0u; row < VAFID_ROW_COUNT; row++)
        {
            float magnitude = VAFID_absolute(buffer->a[row][column]);
            if (magnitude > scale)
            {
                scale = magnitude;
            }
        }
        if ((VAFID_isFinite(scale) == 0u) || (scale <= VAFID_QR_EPSILON))
        {
            return 0u;
        }
        columnScale[column] = scale;
        for (row = 0u; row < VAFID_ROW_COUNT; row++)
        {
            normalized[row][column] = buffer->a[row][column] / scale;
        }
    }

    for (column = 0u; column < VAFID_COLUMN_COUNT; column++)
    {
        float normSquared = 0.0F;
        for (row = 0u; row < VAFID_ROW_COUNT; row++)
        {
            orthogonal[row][column] = normalized[row][column];
        }
        for (previous = 0u; previous < column; previous++)
        {
            float projection = 0.0F;
            for (row = 0u; row < VAFID_ROW_COUNT; row++)
            {
                projection += orthogonal[row][previous] * orthogonal[row][column];
            }
            upper[previous][column] = projection;
            for (row = 0u; row < VAFID_ROW_COUNT; row++)
            {
                orthogonal[row][column] -= projection * orthogonal[row][previous];
            }
        }
        for (row = 0u; row < VAFID_ROW_COUNT; row++)
        {
            normSquared += orthogonal[row][column] * orthogonal[row][column];
        }
        upper[column][column] = sqrtf(normSquared);
        if ((VAFID_isFinite(upper[column][column]) == 0u)
            || (upper[column][column] <= VAFID_QR_EPSILON))
        {
            return 0u;
        }
        for (row = 0u; row < VAFID_ROW_COUNT; row++)
        {
            orthogonal[row][column] /= upper[column][column];
            projected[column] += orthogonal[row][column] * buffer->b[row];
        }
    }

    for (column = VAFID_COLUMN_COUNT; column > 0u; column--)
    {
        uint8_t solvedColumn = (uint8_t)(column - 1u);
        float value = projected[solvedColumn];
        uint8_t following;
        for (following = (uint8_t)(solvedColumn + 1u);
             following < VAFID_COLUMN_COUNT; following++)
        {
            value -= upper[solvedColumn][following] * scaledResult[following];
        }
        scaledResult[solvedColumn] = value / upper[solvedColumn][solvedColumn];
        result[solvedColumn] = scaledResult[solvedColumn] / columnScale[solvedColumn];
        if (VAFID_isFinite(result[solvedColumn]) == 0u)
        {
            return 0u;
        }
    }

    for (row = 0u; row < VAFID_ROW_COUNT; row++)
    {
        float prediction = 0.0F;
        float residual;
        for (column = 0u; column < VAFID_COLUMN_COUNT; column++)
        {
            prediction += buffer->a[row][column] * result[column];
        }
        residual = buffer->b[row] - prediction;
        residualEnergy += residual * residual;
        outputEnergy += buffer->b[row] * buffer->b[row];
    }
    *fitError = sqrtf(residualEnergy / (outputEnergy + VAFID_FIT_EPSILON));
    if (VAFID_reciprocalCondition(buffer, conditionProxy) == 0u)
    {
        return 0u;
    }
    return ((VAFID_isFinite(*fitError) != 0u)
        && (VAFID_isFinite(*conditionProxy) != 0u)) ? 1u : 0u;
}

static uint8_t VAFID_withinRelativeTolerance(float current,
                                             float previous,
                                             float tolerance,
                                             float floorValue)
{
    float denominator = VAFID_absolute(previous);
    if (denominator < floorValue)
    {
        denominator = floorValue;
    }
    return (VAFID_absolute(current - previous) <= (tolerance * denominator)) ? 1u : 0u;
}

static void VAFID_publishCandidate(const VAFID_LockinBuffer *buffer,
                                   const float solution[VAFID_COLUMN_COUNT],
                                   float fitError,
                                   float conditionProxy)
{
    float inverseSamples = 1.0F / (float)buffer->sampleCount;
    float idMean = buffer->idSum_A * inverseSamples;
    float activeFluxMean = buffer->activeFluxSum_Wb * inverseSamples;
    float resistanceLower = 0.2F * vafidState.nominal.resistance_Ohm;
    float resistanceUpper = 5.0F * vafidState.nominal.resistance_Ohm;
    float inductanceDLower = 0.4F * vafidState.nominal.inductanceD_H;
    float inductanceDUpper = 2.5F * vafidState.nominal.inductanceD_H;
    float inductanceQLower = 0.4F * vafidState.nominal.inductanceQ_H;
    float inductanceQUpper = 2.5F * vafidState.nominal.inductanceQ_H;
    float fluxLower = 0.4F * vafidState.nominal.permanentMagnetFlux_Wb;
    float fluxUpper = 1.6F * vafidState.nominal.permanentMagnetFlux_Wb;
    float fluxCandidate;
    uint8_t consistent = 1u;
    uint8_t rangeValid;
    uint8_t fluxRangeValid;

    if (resistanceLower < VAFID_MIN_RESISTANCE_OHM)
    {
        resistanceLower = VAFID_MIN_RESISTANCE_OHM;
    }
    if (resistanceUpper > VAFID_MAX_RESISTANCE_OHM)
    {
        resistanceUpper = VAFID_MAX_RESISTANCE_OHM;
    }
    if (inductanceDLower < VAFID_MIN_INDUCTANCE_H)
    {
        inductanceDLower = VAFID_MIN_INDUCTANCE_H;
    }
    if (inductanceDUpper > VAFID_MAX_INDUCTANCE_H)
    {
        inductanceDUpper = VAFID_MAX_INDUCTANCE_H;
    }
    if (inductanceQLower < VAFID_MIN_INDUCTANCE_H)
    {
        inductanceQLower = VAFID_MIN_INDUCTANCE_H;
    }
    if (inductanceQUpper > VAFID_MAX_INDUCTANCE_H)
    {
        inductanceQUpper = VAFID_MAX_INDUCTANCE_H;
    }
    if (fluxLower < VAFID_MIN_FLUX_WB)
    {
        fluxLower = VAFID_MIN_FLUX_WB;
    }
    if (fluxUpper > VAFID_MAX_FLUX_WB)
    {
        fluxUpper = VAFID_MAX_FLUX_WB;
    }

    vafidState.candidate.resistance_Ohm = solution[0];
    vafidState.candidate.inductanceD_H = solution[1];
    vafidState.candidate.inductanceQ_H = solution[2];
    vafidState.candidate.fitError = fitError;
    vafidState.candidate.conditionProxy = conditionProxy;
    vafidState.candidate.completedWindow = buffer->completedWindow;
    vafidState.candidate.validMask = 0u;

    Meas_VAFID_ActFluxFlt_Wb_f32 = activeFluxMean;
    Meas_VAFID_IdMean_A_f32 = idMean;
    Meas_VAFID_Fit_f32 = fitError;
    Meas_VAFID_Cond_f32 = conditionProxy;

    /* v5 candidate bounds: nominal-relative bounds intersected with absolute
     * firmware safety bounds. */
    rangeValid = ((solution[0] >= resistanceLower)
        && (solution[0] <= resistanceUpper)
        && (solution[1] >= inductanceDLower)
        && (solution[1] <= inductanceDUpper)
        && (solution[2] >= inductanceQLower)
        && (solution[2] <= inductanceQUpper)) ? 1u : 0u;

    if (fitError >= vafidState.configuration.maximumFitError)
    {
        vafidState.rejectMask = VAFID_REJECT_FIT;
    }
    else if (conditionProxy <= vafidState.configuration.minimumConditionProxy)
    {
        vafidState.rejectMask = VAFID_REJECT_CONDITION;
    }
    else if (rangeValid == 0u)
    {
        vafidState.rejectMask = VAFID_REJECT_RANGE;
    }
    else
    {
        vafidState.rejectMask = 0u;
    }

    if (vafidState.rejectMask != 0u)
    {
        vafidState.consecutiveWindows = 0u;
        vafidState.previousCandidateValid = 0u;
        vafidState.acceptedFluxWindows = 0u;
        vafidState.published.validMask = 0u;
        Meas_VAFID_ValidMask_u8 = 0u;
        Meas_VAFID_ConsWin_u8 = 0u;
        Meas_VAFID_RejectMask_u16 = vafidState.rejectMask;
        Meas_VAFID_Stat_u8 = VAFID_STATUS_WINDOW_REJECTED;
        return;
    }

    if (vafidState.previousCandidateValid != 0u)
    {
        consistent &= VAFID_withinRelativeTolerance(solution[0],
            vafidState.previousCandidate.resistance_Ohm,
            vafidState.configuration.consistencyTolerance, 0.01F);
        consistent &= VAFID_withinRelativeTolerance(solution[1],
            vafidState.previousCandidate.inductanceD_H,
            vafidState.configuration.consistencyTolerance, 0.00005F);
        consistent &= VAFID_withinRelativeTolerance(solution[2],
            vafidState.previousCandidate.inductanceQ_H,
            vafidState.configuration.consistencyTolerance, 0.00005F);
    }

    if (consistent == 0u)
    {
        vafidState.consecutiveWindows = 0u;
        vafidState.acceptedFluxWindows = 0u;
        vafidState.rejectMask = VAFID_REJECT_CONSISTENCY;
        vafidState.published.validMask = 0u;
        Meas_VAFID_ValidMask_u8 = 0u;
        Meas_VAFID_RejectMask_u16 = vafidState.rejectMask;
        Meas_VAFID_Stat_u8 = VAFID_STATUS_WINDOW_REJECTED;
        vafidState.previousCandidate = vafidState.candidate;
        vafidState.previousCandidateValid = 1u;
        Meas_VAFID_ConsWin_u8 = 0u;
        return;
    }
    else if (vafidState.consecutiveWindows < 255u)
    {
        vafidState.consecutiveWindows++;
    }

    /* Every quality-accepted window updates the parameter candidates using
     * the v5 alpha=0.20 fusion, seeded from the configured nominal values. */
    if (vafidState.parameterFusionInitialized == 0u)
    {
        vafidState.published.resistance_Ohm = solution[0];
        vafidState.published.inductanceD_H = solution[1];
        vafidState.published.inductanceQ_H = solution[2];
        vafidState.parameterFusionInitialized = 1u;
    }
    else
    {
        vafidState.published.resistance_Ohm += VAFID_PARAMETER_FUSION_ALPHA
            * (solution[0] - vafidState.published.resistance_Ohm);
        vafidState.published.inductanceD_H += VAFID_PARAMETER_FUSION_ALPHA
            * (solution[1] - vafidState.published.inductanceD_H);
        vafidState.published.inductanceQ_H += VAFID_PARAMETER_FUSION_ALPHA
            * (solution[2] - vafidState.published.inductanceQ_H);
    }

    fluxCandidate = activeFluxMean
        - ((vafidState.published.inductanceD_H
        - vafidState.published.inductanceQ_H) * idMean);
    vafidState.candidate.permanentMagnetFlux_Wb = fluxCandidate;
    fluxRangeValid = ((VAFID_isFinite(fluxCandidate) != 0u)
        && (fluxCandidate >= fluxLower)
        && (fluxCandidate <= fluxUpper)) ? 1u : 0u;
    if (fluxRangeValid != 0u)
    {
        if (vafidState.fluxFusionInitialized == 0u)
        {
            vafidState.fusedFlux_Wb = vafidState.nominal.permanentMagnetFlux_Wb;
            vafidState.fluxFusionInitialized = 1u;
        }
        if (vafidState.acceptedFluxWindows < 255u)
        {
            vafidState.acceptedFluxWindows++;
        }
        if (vafidState.acceptedFluxWindows >= VAFID_MIN_FLUX_FUSION_WINDOWS)
        {
            vafidState.fusedFlux_Wb += VAFID_FLUX_FUSION_ALPHA
                * (fluxCandidate - vafidState.fusedFlux_Wb);
        }
    }
    else
    {
        vafidState.acceptedFluxWindows = 0u;
    }

    vafidState.published.permanentMagnetFlux_Wb = vafidState.fusedFlux_Wb;
    vafidState.published.fitError = fitError;
    vafidState.published.conditionProxy = conditionProxy;
    vafidState.published.completedWindow = buffer->completedWindow;
    vafidState.published.validMask = 0u;
    Meas_VAFID_Rs_Ohm_f32 = vafidState.published.resistance_Ohm;
    Meas_VAFID_Ld_H_f32 = vafidState.published.inductanceD_H;
    Meas_VAFID_Lq_H_f32 = vafidState.published.inductanceQ_H;
    Meas_VAFID_FluxPM_Wb_f32 = vafidState.published.permanentMagnetFlux_Wb;

    vafidState.previousCandidate = vafidState.candidate;
    vafidState.previousCandidateValid = 1u;
    Meas_VAFID_ConsWin_u8 = vafidState.consecutiveWindows;

    if (vafidState.consecutiveWindows >= vafidState.configuration.consistentWindows)
    {
        vafidState.published.validMask = VAFID_VALID_RS | VAFID_VALID_LD | VAFID_VALID_LQ;
        if ((buffer->allFluxQualified != 0u)
            && (fluxRangeValid != 0u)
            && (vafidState.acceptedFluxWindows >= VAFID_MIN_FLUX_FUSION_WINDOWS))
        {
            vafidState.published.validMask |= VAFID_VALID_FLUX_PM;
        }
        vafidState.lastAcceptedTick = buffer->completedFastTick;
        Meas_VAFID_ValidMask_u8 = vafidState.published.validMask;
        Meas_VAFID_RejectMask_u16 = 0u;
        Meas_VAFID_Stat_u8 = VAFID_STATUS_SHADOW_VALID;
    }
    else if (consistent != 0u)
    {
        Meas_VAFID_RejectMask_u16 = 0u;
        Meas_VAFID_Stat_u8 = VAFID_STATUS_COLLECTING;
    }
}

static void VAFID_initializeUnlocked(uint32_t collectionGeneration)
{
    memset(&vafidState, 0, sizeof(vafidState));
    vafidState.collectionGeneration = collectionGeneration;
    VAFID_clearBuffer(&vafidState.buffer[0]);
    VAFID_clearBuffer(&vafidState.buffer[1]);
    VAFID_resetOscillators();

    Meas_VAFID_Act_u8 = 0u;
    Meas_VAFID_Stat_u8 = VAFID_STATUS_OFF;
    Meas_VAFID_ValidMask_u8 = 0u;
    Meas_VAFID_RejectMask_u16 = 0u;
    Meas_VAFID_ProbeD_Q15_s16 = 0;
    Meas_VAFID_ProbeQ_Q15_s16 = 0;
    Meas_VAFID_ProbeClip_u8 = 0u;
    Meas_VAFID_Rs_Ohm_f32 = 0.0F;
    Meas_VAFID_Ld_H_f32 = 0.0F;
    Meas_VAFID_Lq_H_f32 = 0.0F;
    Meas_VAFID_FluxPM_Wb_f32 = 0.0F;
    Meas_VAFID_ActFluxFlt_Wb_f32 = 0.0F;
    Meas_VAFID_IdMean_A_f32 = 0.0F;
    Meas_VAFID_Fit_f32 = 0.0F;
    Meas_VAFID_Cond_f32 = 0.0F;
    Meas_VAFID_Win_u32 = 0u;
    Meas_VAFID_ConsWin_u8 = 0u;
    Meas_VAFID_FbResult_u8 = VAFID_FEEDBACK_RESULT_NONE;
}

void VAFID_initialize(void)
{
    VAFID_CriticalState criticalState = VAFID_enterCritical();
    VAFID_initializeUnlocked(1u);
    VAFID_exitCritical(criticalState);
}

void VAFID_reset(void)
{
    VAFID_NominalParameters nominal;
    VAFID_RuntimeConfiguration configuration;
    VAFID_RawCalibration appliedCalibration;
    uint32_t nextGeneration;
    uint8_t nominalConfigured;
    uint8_t runtimeCalibrationValid;
    uint8_t appliedCalibrationValid;
    VAFID_CriticalState criticalState = VAFID_enterCritical();

    nominal = vafidState.nominal;
    configuration = vafidState.configuration;
    appliedCalibration = vafidState.appliedCalibration;
    nominalConfigured = vafidState.nominalConfigured;
    runtimeCalibrationValid = vafidState.runtimeCalibrationValid;
    appliedCalibrationValid = vafidState.appliedCalibrationValid;
    nextGeneration = vafidState.collectionGeneration + 1u;
    VAFID_initializeUnlocked(nextGeneration);
    vafidState.nominal = nominal;
    vafidState.configuration = configuration;
    vafidState.appliedCalibration = appliedCalibration;
    vafidState.nominalConfigured = nominalConfigured;
    vafidState.runtimeCalibrationValid = runtimeCalibrationValid;
    vafidState.appliedCalibrationValid = appliedCalibrationValid;
    vafidState.oscillatorD.sineStep = configuration.sineStepD;
    vafidState.oscillatorD.cosineStep = configuration.cosineStepD;
    vafidState.oscillatorQ.sineStep = configuration.sineStepQ;
    vafidState.oscillatorQ.cosineStep = configuration.cosineStepQ;
    vafidState.activeFluxFilterAlpha = configuration.activeFluxFilterAlpha;
    vafidState.omegaFilterAlpha = configuration.omegaFilterAlpha;
    vafidState.angleTrackAlpha = configuration.angleTrackAlpha;
    vafidState.envelopeStep = configuration.envelopeStep;
    if (nominalConfigured != 0u)
    {
        VAFID_seedPublishedFromNominalUnlocked();
    }
    vafidState.settleRemaining = configuration.settleSamples;
    Meas_VAFID_Stat_u8 = (Cal_VAFID_Mode_u8 == VAFID_MODE_SHADOW)
        ? VAFID_STATUS_WAIT_ELIGIBILITY : VAFID_STATUS_OFF;
    VAFID_exitCritical(criticalState);
}

void VAFID_abortFast(uint16_t rejectMask)
{
    uint8_t collectionActive;
    VAFID_CriticalState criticalState = VAFID_enterCritical();

    collectionActive = ((vafidState.fastEligible != 0u)
        || (vafidState.pendingProbe.valid != 0u)
        || (vafidState.capturedVoltage.fresh != 0u)
        || (vafidState.captureExpected != 0u)
        || (vafidState.buffer[0].ready != 0u)
        || (vafidState.buffer[1].ready != 0u)
        || (vafidState.buffer[0].sampleCount != 0u)
        || (vafidState.buffer[1].sampleCount != 0u)
        || (vafidState.published.validMask != 0u)
        || (vafidState.consecutiveWindows != 0u)
        || (vafidState.previousCandidateValid != 0u)) ? 1u : 0u;
    vafidState.fastEligible = 0u;
    vafidState.observerOperatingPointValid = 0u;
    Meas_VAFID_Act_u8 = 0u;
    if (collectionActive != 0u)
    {
        VAFID_restartCollectionUnlocked(rejectMask);
    }
    else
    {
        vafidState.published.validMask = 0u;
        vafidState.rejectMask = rejectMask;
        Meas_VAFID_ValidMask_u8 = 0u;
        Meas_VAFID_RejectMask_u16 = rejectMask;
        if (rejectMask != 0u)
        {
            Meas_VAFID_Stat_u8 = VAFID_STATUS_WINDOW_REJECTED;
        }
    }
    VAFID_exitCritical(criticalState);
}

uint8_t VAFID_configure(const VAFID_NominalParameters *nominal)
{
    VAFID_RuntimeConfiguration configuration;
    VAFID_RawCalibration calibration;
    uint8_t mode = Cal_VAFID_Mode_u8;
    uint8_t configurationValid = 0u;
    VAFID_CriticalState criticalState;

    if (VAFID_parametersFiniteAndPositive(nominal) == 0u)
    {
        criticalState = VAFID_enterCritical();
        vafidState.nominalConfigured = 0u;
        vafidState.runtimeCalibrationValid = 0u;
        vafidState.appliedCalibrationValid = 0u;
        vafidState.fastEligible = 0u;
        vafidState.observerOperatingPointValid = 0u;
        VAFID_restartCollectionUnlocked(VAFID_REJECT_PARAMETER);
        Meas_VAFID_Stat_u8 = VAFID_STATUS_PARAMETER_INVALID;
        Meas_VAFID_RejectMask_u16 = VAFID_REJECT_PARAMETER;
        VAFID_exitCritical(criticalState);
        return 0u;
    }

    VAFID_readRawCalibration(&calibration);
    if (mode == VAFID_MODE_SHADOW)
    {
        configurationValid = VAFID_deriveConfiguration(&configuration,
            &calibration, nominal);
    }

    criticalState = VAFID_enterCritical();
    vafidState.nominal = *nominal;
    vafidState.nominalConfigured = 1u;
    VAFID_seedPublishedFromNominalUnlocked();
    if (mode == VAFID_MODE_OFF)
    {
        vafidState.runtimeCalibrationValid = 0u;
        vafidState.appliedCalibrationValid = 0u;
        vafidState.fastEligible = 0u;
        vafidState.observerOperatingPointValid = 0u;
        VAFID_restartCollectionUnlocked(0u);
        Meas_VAFID_Stat_u8 = VAFID_STATUS_OFF;
        VAFID_exitCritical(criticalState);
        return 1u;
    }
    if ((mode != VAFID_MODE_SHADOW) || (configurationValid == 0u))
    {
        vafidState.runtimeCalibrationValid = 0u;
        vafidState.appliedCalibrationValid = 0u;
        vafidState.fastEligible = 0u;
        vafidState.observerOperatingPointValid = 0u;
        VAFID_restartCollectionUnlocked(VAFID_REJECT_PARAMETER);
        Meas_VAFID_Stat_u8 = VAFID_STATUS_PARAMETER_INVALID;
        Meas_VAFID_RejectMask_u16 = VAFID_REJECT_PARAMETER;
        VAFID_exitCritical(criticalState);
        return 0u;
    }

    VAFID_commitConfigurationUnlocked(&configuration, &calibration, 0u);
    Meas_VAFID_Stat_u8 = VAFID_STATUS_WAIT_ELIGIBILITY;
    VAFID_exitCritical(criticalState);
    return 1u;
}

void VAFID_setFastEligibility(const VAFID_FastEligibility *eligibility)
{
    uint16_t rejectMask = 0u;
    uint8_t eligibleNow;

    if (Cal_VAFID_Mode_u8 == VAFID_MODE_OFF)
    {
        if ((vafidState.previousMode != VAFID_MODE_OFF)
            || (vafidState.fastEligible != 0u)
            || (vafidState.observerOperatingPointValid != 0u)
            || (vafidState.published.validMask != 0u))
        {
            VAFID_abortFast(0u);
        }
        else
        {
            vafidState.fastEligible = 0u;
        }
        vafidState.eligibilitySnapshotValid = 0u;
        vafidState.previousMode = VAFID_MODE_OFF;
        Meas_VAFID_Act_u8 = 0u;
        Meas_VAFID_ValidMask_u8 = 0u;
        Meas_VAFID_RejectMask_u16 = 0u;
        Meas_VAFID_Stat_u8 = VAFID_STATUS_OFF;
        return;
    }
    if (eligibility == NULL)
    {
        memset(&vafidState.eligibility, 0, sizeof(vafidState.eligibility));
        VAFID_abortFast(VAFID_REJECT_ELIGIBILITY);
        return;
    }

    eligibleNow = ((VAFID_allEligibilityTrue(eligibility) != 0u)
        && (vafidState.observerOperatingPointValid != 0u)) ? 1u : 0u;
    if (vafidState.eligibilitySnapshotValid != 0u)
    {
        if (eligibility->deadTimeConfigSignature != vafidState.lastDtcSignature)
        {
            rejectMask |= VAFID_REJECT_DTC_CHANGED;
        }
        if (eligibility->adcSampleOffsetTicks != vafidState.lastAdcSampleOffset)
        {
            rejectMask |= VAFID_REJECT_ADC_CHANGED;
        }
    }
    vafidState.lastDtcSignature = eligibility->deadTimeConfigSignature;
    vafidState.lastAdcSampleOffset = eligibility->adcSampleOffsetTicks;
    vafidState.eligibilitySnapshotValid = 1u;
    vafidState.eligibility = *eligibility;

    if (eligibility->overmodulationActive != 0u)
    {
        rejectMask |= VAFID_REJECT_OVERMODULATION;
    }
    if (eligibleNow == 0u)
    {
        rejectMask |= VAFID_REJECT_ELIGIBILITY;
    }
    if (eligibleNow == 0u)
    {
        if (vafidState.fastEligible != 0u)
        {
            VAFID_abortFast(rejectMask);
        }
        else
        {
            /* Structural or observer eligibility is still absent. Publish
             * the wait reason without repeatedly entering the fast-path
             * critical section or clearing an already-empty collection. */
            vafidState.rejectMask = rejectMask;
            Meas_VAFID_Act_u8 = 0u;
            Meas_VAFID_ValidMask_u8 = 0u;
            Meas_VAFID_RejectMask_u16 = rejectMask;
            Meas_VAFID_Stat_u8 = VAFID_STATUS_WAIT_ELIGIBILITY;
        }
        return;
    }
    if ((rejectMask
        & (VAFID_REJECT_DTC_CHANGED | VAFID_REJECT_ADC_CHANGED)) != 0u)
    {
        VAFID_restartCollection(rejectMask);
    }
    else if (vafidState.fastEligible == 0u)
    {
        VAFID_restartCollection(0u);
    }
    vafidState.fastEligible = 1u;
}

void VAFID_applyProbe(const VAFID_DqQ15 *baseCommand,
                      const VAFID_CurrentLimits *limits,
                      VAFID_DqQ15 *probedCommand)
{
    VAFID_CurrentLimits safeLimits;
    int16_t probeD;
    int16_t probeQ;
    int32_t commandD;
    int32_t commandQ;
    uint8_t clipped = 0u;
    uint8_t collectEligible;
    uint8_t mode = Cal_VAFID_Mode_u8;

    if ((baseCommand == NULL) || (probedCommand == NULL))
    {
        return;
    }
    *probedCommand = *baseCommand;
    vafidState.pendingProbe.valid = 0u;
    Meas_VAFID_ProbeD_Q15_s16 = 0;
    Meas_VAFID_ProbeQ_Q15_s16 = 0;
    Meas_VAFID_ProbeClip_u8 = 0u;

    if (mode == VAFID_MODE_OFF)
    {
        if ((vafidState.previousMode != VAFID_MODE_OFF)
            || (vafidState.fastEligible != 0u)
            || (vafidState.observerOperatingPointValid != 0u)
            || (vafidState.published.validMask != 0u))
        {
            VAFID_abortFast(0u);
        }
        vafidState.previousMode = mode;
        Meas_VAFID_Act_u8 = 0u;
        Meas_VAFID_Stat_u8 = VAFID_STATUS_OFF;
        return;
    }
    if ((mode != VAFID_MODE_SHADOW)
        || (vafidState.nominalConfigured == 0u)
        || (vafidState.runtimeCalibrationValid == 0u))
    {
        Meas_VAFID_Act_u8 = 0u;
        Meas_VAFID_Stat_u8 = VAFID_STATUS_PARAMETER_INVALID;
        return;
    }
    if (vafidState.previousMode != mode)
    {
        VAFID_restartCollection(0u);
    }
    vafidState.previousMode = mode;
    if (vafidState.fastEligible == 0u)
    {
        Meas_VAFID_Act_u8 = 0u;
        Meas_VAFID_Stat_u8 = VAFID_STATUS_WAIT_ELIGIBILITY;
        return;
    }

    safeLimits.dLowerQ15 = -32768;
    safeLimits.dUpperQ15 = 32767;
    safeLimits.qLowerQ15 = -32768;
    safeLimits.qUpperQ15 = 32767;
    safeLimits.currentMagnitudeMaxQ15 = 32767u;
    if (limits != NULL)
    {
        safeLimits = *limits;
    }
    if ((safeLimits.dLowerQ15 > safeLimits.dUpperQ15)
        || (safeLimits.qLowerQ15 > safeLimits.qUpperQ15)
        || (safeLimits.currentMagnitudeMaxQ15 == 0u)
        || (safeLimits.currentMagnitudeMaxQ15 > 32767u))
    {
        VAFID_restartCollection(VAFID_REJECT_PARAMETER);
        return;
    }

    collectEligible = (vafidState.settleRemaining == 0u) ? 1u : 0u;
    if (vafidState.settleRemaining != 0u)
    {
        if (vafidState.envelope < 1.0F)
        {
            vafidState.envelope += vafidState.envelopeStep;
            if (vafidState.envelope > 1.0F)
            {
                vafidState.envelope = 1.0F;
            }
        }
        vafidState.settleRemaining--;
        Meas_VAFID_Stat_u8 = VAFID_STATUS_SETTLING;
    }
    else
    {
        vafidState.envelope = 1.0F;
        Meas_VAFID_Stat_u8 = VAFID_STATUS_COLLECTING;
    }

    vafidState.pendingProbe.sineD = vafidState.oscillatorD.sine;
    vafidState.pendingProbe.cosineD = vafidState.oscillatorD.cosine;
    vafidState.pendingProbe.sineQ = vafidState.oscillatorQ.sine;
    vafidState.pendingProbe.cosineQ = vafidState.oscillatorQ.cosine;
    vafidState.pendingProbe.collectEligible = collectEligible;
    vafidState.pendingProbe.valid = 1u;

    probeD = VAFID_roundSaturateS16((float)vafidState.configuration.amplitudeD_Q15
        * vafidState.envelope * vafidState.oscillatorD.sine);
    probeQ = VAFID_roundSaturateS16((float)vafidState.configuration.amplitudeQ_Q15
        * vafidState.envelope * vafidState.oscillatorQ.sine);
    commandD = (int32_t)baseCommand->d + (int32_t)probeD;
    commandQ = (int32_t)baseCommand->q + (int32_t)probeQ;
    probedCommand->d = VAFID_clampS16(commandD, safeLimits.dLowerQ15,
        safeLimits.dUpperQ15, &clipped);
    probedCommand->q = VAFID_clampS16(commandQ, safeLimits.qLowerQ15,
        safeLimits.qUpperQ15, &clipped);

    {
        uint32_t magnitudeLimit = safeLimits.currentMagnitudeMaxQ15;
        uint32_t dMagnitude = (probedCommand->d < 0)
            ? (uint32_t)(-(int32_t)probedCommand->d) : (uint32_t)probedCommand->d;
        uint32_t qMagnitude = (probedCommand->q < 0)
            ? (uint32_t)(-(int32_t)probedCommand->q) : (uint32_t)probedCommand->q;
        uint32_t limitSquared = magnitudeLimit * magnitudeLimit;
        uint32_t dSquared = dMagnitude * dMagnitude;
        uint32_t qSquared = qMagnitude * qMagnitude;
        if ((dSquared + qSquared) > limitSquared)
        {
            uint32_t qLimit;
            clipped = 1u;
            if (dMagnitude >= magnitudeLimit)
            {
                probedCommand->d = (probedCommand->d < 0)
                    ? -(int16_t)magnitudeLimit : (int16_t)magnitudeLimit;
                probedCommand->q = 0;
            }
            else
            {
                qLimit = VAFID_integerSquareRoot(limitSquared - dSquared);
                if (qMagnitude > qLimit)
                {
                    probedCommand->q = (probedCommand->q < 0)
                        ? -(int16_t)qLimit : (int16_t)qLimit;
                }
            }
        }
    }

    Meas_VAFID_Act_u8 = 1u;
    Meas_VAFID_ProbeD_Q15_s16 = probeD;
    Meas_VAFID_ProbeQ_Q15_s16 = probeQ;
    Meas_VAFID_ProbeClip_u8 = clipped;
    VAFID_advanceOscillator(&vafidState.oscillatorD);
    VAFID_advanceOscillator(&vafidState.oscillatorQ);

    if (clipped != 0u)
    {
        *probedCommand = *baseCommand;
        vafidState.pendingProbe.valid = 0u;
        VAFID_restartCollection(VAFID_REJECT_PROBE_CLIPPED);
    }
    else
    {
        vafidState.captureExpected = 1u;
    }
}

void VAFID_captureMotorVoltage(float modulatorActualAlpha_V,
                               float modulatorActualBeta_V,
                               float deadTimeModelAlpha_V,
                               float deadTimeModelBeta_V,
                               uint8_t overmodulationActive)
{
    if ((Cal_VAFID_Mode_u8 != VAFID_MODE_SHADOW)
        || (vafidState.nominalConfigured == 0u)
        || (vafidState.runtimeCalibrationValid == 0u)
        || (vafidState.fastEligible == 0u)
        || (vafidState.pendingProbe.valid == 0u))
    {
        return;
    }
    if (overmodulationActive != 0u)
    {
        VAFID_restartCollection(VAFID_REJECT_OVERMODULATION);
        return;
    }
    if ((VAFID_isFinite(modulatorActualAlpha_V) == 0u)
        || (VAFID_isFinite(modulatorActualBeta_V) == 0u)
        || (VAFID_isFinite(deadTimeModelAlpha_V) == 0u)
        || (VAFID_isFinite(deadTimeModelBeta_V) == 0u))
    {
        VAFID_restartCollection(VAFID_REJECT_VOLTAGE_STALE);
        return;
    }
    if (vafidState.capturedVoltage.fresh != 0u)
    {
        VAFID_restartCollection(VAFID_REJECT_VOLTAGE_STALE);
        return;
    }

    vafidState.capturedVoltage.alpha_V = modulatorActualAlpha_V
        - deadTimeModelAlpha_V;
    vafidState.capturedVoltage.beta_V = modulatorActualBeta_V
        - deadTimeModelBeta_V;
    vafidState.capturedVoltage.probe = vafidState.pendingProbe;
    vafidState.capturedVoltage.fresh = 1u;
    vafidState.pendingProbe.valid = 0u;
}

void VAFID_observerStep(float currentAlpha_A,
                        float currentBeta_A,
                        float kreElectricalAngle_rad,
                        float kreRawElectricalOmega_radps,
                        float kreActiveFlux_Wb,
                        uint8_t kreValid,
                        uint8_t activeFluxQualified)
{
    float absoluteOmega;
    float thetaForCurrent;
    float sine;
    float cosine;
    float id_A;
    float iq_A;
    float vd_V;
    float vq_V;
    VAFID_VoltageRecord voltageRecord;
    VAFID_LockinBuffer *buffer;

    if ((Cal_VAFID_Mode_u8 != VAFID_MODE_SHADOW)
        || (vafidState.nominalConfigured == 0u)
        || (vafidState.runtimeCalibrationValid == 0u))
    {
        return;
    }

    if ((kreValid == 0u)
        || (VAFID_isFinite(currentAlpha_A) == 0u)
        || (VAFID_isFinite(currentBeta_A) == 0u)
        || (VAFID_isFinite(kreElectricalAngle_rad) == 0u)
        || (VAFID_isFinite(kreRawElectricalOmega_radps) == 0u)
        || (VAFID_isFinite(kreActiveFlux_Wb) == 0u)
        || (VAFID_absolute(kreElectricalAngle_rad)
        > VAFID_MAX_KRE_ANGLE_RAD))
    {
        if ((vafidState.observerOperatingPointValid != 0u)
            || (vafidState.fastEligible != 0u))
        {
            VAFID_abortFast(VAFID_REJECT_KRE_INVALID);
        }
        return;
    }

    absoluteOmega = VAFID_absolute(kreRawElectricalOmega_radps);
    if ((absoluteOmega < VAFID_MIN_ELECTRICAL_OMEGA_RADPS)
        || (absoluteOmega > VAFID_MAX_ELECTRICAL_OMEGA_RADPS))
    {
        if ((vafidState.observerOperatingPointValid != 0u)
            || (vafidState.fastEligible != 0u))
        {
            VAFID_abortFast(VAFID_REJECT_SPEED_RANGE);
        }
        return;
    }

    /* KRE angle is produced from its internal delayed V/I tuple. Advance it
     * exactly one fast sample so this frame corresponds to current i[k]. */
    thetaForCurrent = kreElectricalAngle_rad
        + (vafidState.nominal.sampleTime_s * kreRawElectricalOmega_radps);
    if ((VAFID_isFinite(thetaForCurrent) == 0u)
        || (VAFID_absolute(thetaForCurrent) > VAFID_MAX_KRE_ANGLE_RAD))
    {
        if ((vafidState.observerOperatingPointValid != 0u)
            || (vafidState.fastEligible != 0u))
        {
            VAFID_abortFast(VAFID_REJECT_KRE_INVALID);
        }
        return;
    }
    thetaForCurrent = VAFID_wrapAngle(thetaForCurrent);
    vafidState.observerOperatingPointValid = 1u;

    if (vafidState.fastEligible == 0u)
    {
        return;
    }
    vafidState.fastTick++;

    if ((vafidState.capturedVoltage.fresh == 0u)
        || (vafidState.capturedVoltage.probe.valid == 0u))
    {
        /* OFF->SHADOW startup reaches the observer before the first PWM
         * apply/capture. That first absence is an expected priming wait. */
        if (vafidState.captureExpected != 0u)
        {
            VAFID_abortFast(VAFID_REJECT_VOLTAGE_STALE);
        }
        return;
    }

    voltageRecord = vafidState.capturedVoltage;
    vafidState.capturedVoltage.fresh = 0u;
    vafidState.captureExpected = 0u;

    if (vafidState.frameTrackerInitialized == 0u)
    {
        vafidState.filteredOmega_radps = kreRawElectricalOmega_radps;
        vafidState.trackedAngle_rad = thetaForCurrent;
        vafidState.frameTrackerInitialized = 1u;
    }
    else
    {
        float predictedAngle;
        float angleError;
        vafidState.filteredOmega_radps += vafidState.omegaFilterAlpha
            * (kreRawElectricalOmega_radps - vafidState.filteredOmega_radps);
        predictedAngle = VAFID_wrapAngle(vafidState.trackedAngle_rad
            + (vafidState.nominal.sampleTime_s
            * vafidState.filteredOmega_radps));
        angleError = VAFID_wrapAngle(thetaForCurrent - predictedAngle);
        vafidState.trackedAngle_rad = VAFID_wrapAngle(predictedAngle
            + (vafidState.angleTrackAlpha * angleError));
    }

    VAFID_sineCosine(vafidState.trackedAngle_rad, &sine, &cosine);
    id_A = (cosine * currentAlpha_A) + (sine * currentBeta_A);
    iq_A = (-sine * currentAlpha_A) + (cosine * currentBeta_A);
    vd_V = (cosine * voltageRecord.alpha_V)
        + (sine * voltageRecord.beta_V);
    vq_V = (-sine * voltageRecord.alpha_V)
        + (cosine * voltageRecord.beta_V);

    if (vafidState.fluxFilterInitialized == 0u)
    {
        vafidState.activeFluxFiltered_Wb = kreActiveFlux_Wb;
        vafidState.fluxFilterInitialized = 1u;
    }
    else
    {
        vafidState.activeFluxFiltered_Wb += vafidState.activeFluxFilterAlpha
            * (kreActiveFlux_Wb - vafidState.activeFluxFiltered_Wb);
    }

    if (voltageRecord.probe.collectEligible != 0u)
    {
        const VAFID_ProbeRecord *probe = &voltageRecord.probe;
        buffer = &vafidState.buffer[vafidState.writeBuffer];
        VAFID_accumulatePhasor(buffer, 0u, probe->cosineD, probe->sineD,
            vd_V, vq_V, id_A, iq_A);
        VAFID_accumulatePhasor(buffer, 1u, probe->cosineQ, probe->sineQ,
            vd_V, vq_V, id_A, iq_A);
        buffer->activeFluxSum_Wb += vafidState.activeFluxFiltered_Wb;
        buffer->idSum_A += id_A;
        buffer->omegaSum_radps += vafidState.filteredOmega_radps;
        buffer->allFluxQualified &= activeFluxQualified;
        buffer->sampleCount++;

        if (buffer->sampleCount >= vafidState.configuration.windowSamples)
        {
            VAFID_finishWindow();
        }
    }
}

void VAFID_service(void)
{
    VAFID_LockinBuffer localBuffer;
    VAFID_RuntimeConfiguration localConfiguration;
    VAFID_RawCalibration calibration;
    VAFID_RawCalibration calibrationConfirm;
    VAFID_RawCalibration appliedCalibration;
    VAFID_NominalParameters nominal;
    uint32_t snapshotGeneration = 0u;
    uint32_t snapshotFastTick = 0u;
    uint8_t appliedCalibrationValid;
    uint8_t runtimeCalibrationValid;
    uint8_t haveWindow = 0u;
    uint8_t index;
    uint8_t resetRequest = (Cal_VAFID_Rst_u8 != 0u) ? 1u : 0u;
    uint8_t applyRequest = (Cal_VAFID_Apply_u8 != 0u) ? 1u : 0u;
    uint8_t revertRequest = (Cal_VAFID_Revert_u8 != 0u) ? 1u : 0u;
    VAFID_CriticalState criticalState;

    if ((resetRequest != 0u) && (vafidState.previousResetRequest == 0u))
    {
        VAFID_reset();
    }
    vafidState.previousResetRequest = resetRequest;

    if ((applyRequest != 0u) && (vafidState.previousApplyRequest == 0u))
    {
        (void)VAFID_requestFeedback(Cal_VAFID_FbMask_u8);
    }
    if ((revertRequest != 0u) && (vafidState.previousRevertRequest == 0u))
    {
        VAFID_revertFeedback();
    }
    vafidState.previousApplyRequest = applyRequest;
    vafidState.previousRevertRequest = revertRequest;

    /* The default OFF foreground path performs no raw calibration scan and
     * no trigonometric or filter-coefficient work. */
    if (Cal_VAFID_Mode_u8 == VAFID_MODE_OFF)
    {
        if ((vafidState.previousMode != VAFID_MODE_OFF)
            || (vafidState.fastEligible != 0u)
            || (vafidState.observerOperatingPointValid != 0u)
            || (vafidState.published.validMask != 0u))
        {
            VAFID_abortFast(0u);
        }
        vafidState.previousMode = VAFID_MODE_OFF;
        Meas_VAFID_Act_u8 = 0u;
        Meas_VAFID_ValidMask_u8 = 0u;
        Meas_VAFID_RejectMask_u16 = 0u;
        Meas_VAFID_Stat_u8 = VAFID_STATUS_OFF;
        return;
    }
    if (vafidState.nominalConfigured == 0u)
    {
        return;
    }

    VAFID_readRawCalibration(&calibration);
    criticalState = VAFID_enterCritical();
    appliedCalibration = vafidState.appliedCalibration;
    appliedCalibrationValid = vafidState.appliedCalibrationValid;
    runtimeCalibrationValid = vafidState.runtimeCalibrationValid;
    nominal = vafidState.nominal;
    VAFID_exitCritical(criticalState);

    if ((appliedCalibrationValid == 0u)
        || (VAFID_rawCalibrationEqual(&calibration,
        &appliedCalibration) == 0u))
    {
        VAFID_RuntimeConfiguration configuration;
        uint8_t validConfiguration = VAFID_deriveConfiguration(&configuration,
            &calibration, &nominal);

        /* Do not commit a derived mixture while XCP is updating more than
         * one scalar. A stable snapshot is retried on the next service call. */
        VAFID_readRawCalibration(&calibrationConfirm);
        if (VAFID_rawCalibrationEqual(&calibration,
            &calibrationConfirm) == 0u)
        {
            return;
        }

        criticalState = VAFID_enterCritical();
        if ((Cal_VAFID_Mode_u8 != VAFID_MODE_SHADOW)
            || (vafidState.nominalConfigured == 0u))
        {
            VAFID_exitCritical(criticalState);
            return;
        }
        if (validConfiguration == 0u)
        {
            vafidState.appliedCalibration = calibration;
            vafidState.appliedCalibrationValid = 1u;
            vafidState.runtimeCalibrationValid = 0u;
            vafidState.fastEligible = 0u;
            vafidState.observerOperatingPointValid = 0u;
            VAFID_restartCollectionUnlocked(VAFID_REJECT_PARAMETER);
            Meas_VAFID_Stat_u8 = VAFID_STATUS_PARAMETER_INVALID;
            Meas_VAFID_RejectMask_u16 = VAFID_REJECT_PARAMETER;
            VAFID_exitCritical(criticalState);
            return;
        }

        VAFID_commitConfigurationUnlocked(&configuration, &calibration,
            (runtimeCalibrationValid != 0u) ? VAFID_REJECT_PARAMETER : 0u);
        Meas_VAFID_Stat_u8 = VAFID_STATUS_WAIT_ELIGIBILITY;
        VAFID_exitCritical(criticalState);
        runtimeCalibrationValid = 1u;
    }
    else if (runtimeCalibrationValid == 0u)
    {
        /* Remembering an invalid raw snapshot prevents repeated foreground
         * derivation. Any corrected scalar changes the raw snapshot and
         * automatically reaches the derivation path above. */
        Meas_VAFID_Stat_u8 = VAFID_STATUS_PARAMETER_INVALID;
        Meas_VAFID_RejectMask_u16 = VAFID_REJECT_PARAMETER;
        return;
    }

    /* Copy the complete ready payload while the ISR is excluded, then solve
     * the local copy. ISR abort/restart is never blocked by QR work. */
    criticalState = VAFID_enterCritical();
    for (index = 0u; index < VAFID_BUFFER_COUNT; index++)
    {
        if (vafidState.buffer[index].ready != 0u)
        {
            if (vafidState.buffer[index].collectionGeneration
                == vafidState.collectionGeneration)
            {
                localBuffer = vafidState.buffer[index];
                localConfiguration = vafidState.configuration;
                snapshotGeneration = vafidState.collectionGeneration;
                snapshotFastTick = vafidState.fastTick;
                haveWindow = 1u;
            }
            vafidState.buffer[index].ready = 0u;
            break;
        }
    }
    VAFID_exitCritical(criticalState);

    if (haveWindow != 0u)
    {
        float solution[VAFID_COLUMN_COUNT];
        float fitError = 0.0F;
        float conditionProxy = 0.0F;
        uint8_t staleSnapshot = ((snapshotFastTick
            - localBuffer.completedFastTick) > localConfiguration.staleSamples)
            ? 1u : 0u;
        uint8_t solved = 0u;

        if ((staleSnapshot == 0u)
            && (VAFID_buildRegression(&localBuffer,
            &localConfiguration) != 0u)
            && (VAFID_solveEightByThree(&localBuffer, solution,
            &fitError, &conditionProxy) != 0u))
        {
            solved = 1u;
        }

        criticalState = VAFID_enterCritical();
        if ((vafidState.collectionGeneration == snapshotGeneration)
            && (Cal_VAFID_Mode_u8 == VAFID_MODE_SHADOW)
            && (vafidState.runtimeCalibrationValid != 0u)
            && (vafidState.fastEligible != 0u))
        {
            if ((staleSnapshot != 0u)
                || ((vafidState.fastTick - localBuffer.completedFastTick)
                > vafidState.configuration.staleSamples))
            {
                vafidState.published.validMask = 0u;
                vafidState.consecutiveWindows = 0u;
                vafidState.previousCandidateValid = 0u;
                vafidState.acceptedFluxWindows = 0u;
                vafidState.rejectMask = VAFID_REJECT_STALE;
                Meas_VAFID_ValidMask_u8 = 0u;
                Meas_VAFID_ConsWin_u8 = 0u;
                Meas_VAFID_RejectMask_u16 = VAFID_REJECT_STALE;
                Meas_VAFID_Stat_u8 = VAFID_STATUS_RESULT_STALE;
            }
            else if (solved == 0u)
            {
                vafidState.published.validMask = 0u;
                vafidState.consecutiveWindows = 0u;
                vafidState.previousCandidateValid = 0u;
                vafidState.acceptedFluxWindows = 0u;
                vafidState.rejectMask = VAFID_REJECT_SOLVER;
                Meas_VAFID_ValidMask_u8 = 0u;
                Meas_VAFID_ConsWin_u8 = 0u;
                Meas_VAFID_RejectMask_u16 = VAFID_REJECT_SOLVER;
                Meas_VAFID_Stat_u8 = VAFID_STATUS_WINDOW_REJECTED;
            }
            else
            {
                VAFID_publishCandidate(&localBuffer, solution, fitError,
                    conditionProxy);
            }
        }
        VAFID_exitCritical(criticalState);
    }

    criticalState = VAFID_enterCritical();
    if ((vafidState.published.validMask != 0u)
        && ((vafidState.fastTick - vafidState.lastAcceptedTick)
        > vafidState.configuration.staleSamples))
    {
        vafidState.published.validMask = 0u;
        vafidState.consecutiveWindows = 0u;
        vafidState.previousCandidateValid = 0u;
        vafidState.acceptedFluxWindows = 0u;
        Meas_VAFID_ValidMask_u8 = 0u;
        Meas_VAFID_ConsWin_u8 = 0u;
        Meas_VAFID_RejectMask_u16 = VAFID_REJECT_STALE;
        Meas_VAFID_Stat_u8 = VAFID_STATUS_RESULT_STALE;
    }
    VAFID_exitCritical(criticalState);
}

uint8_t VAFID_getEstimate(VAFID_Estimate *estimate)
{
    VAFID_CriticalState criticalState;

    if (estimate == NULL)
    {
        return 0u;
    }
    criticalState = VAFID_enterCritical();
    *estimate = vafidState.published;
    VAFID_exitCritical(criticalState);
    return (estimate->validMask != 0u) ? 1u : 0u;
}

uint8_t VAFID_requestFeedback(uint8_t requestedMask)
{
    if (requestedMask == 0u)
    {
        Meas_VAFID_FbResult_u8 = VAFID_FEEDBACK_RESULT_NONE;
        return VAFID_FEEDBACK_RESULT_NONE;
    }
    Meas_VAFID_FbResult_u8 = VAFID_FEEDBACK_RESULT_LOCKED;
    return VAFID_FEEDBACK_RESULT_LOCKED;
}

void VAFID_revertFeedback(void)
{
    Meas_VAFID_FbResult_u8 = VAFID_FEEDBACK_RESULT_REVERTED;
}

#endif /* FOC_AUX_ALGORITHMS_ENABLE */
