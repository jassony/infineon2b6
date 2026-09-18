#include "../vafid_parameter_identifier.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#if defined(MATLAB_MEX_FILE)
#include "mex.h"
#endif

#define TEST_PI          (3.14159265358979323846F)
#define TEST_TWO_PI      (6.28318530717958647692F)
#define TEST_Q15_SCALE   (32768.0F)
#define TEST_TS_S        (0.00005F)
#define TEST_CURRENT_A   (50.0F)

static int testFailureCount;

static void expectTrue(int condition, const char *message)
{
    if (!condition)
    {
        (void)printf("FAIL: %s\n", message);
        testFailureCount++;
    }
}

static float relativeError(float actual, float expected)
{
    float denominator = fabsf(expected);
    if (denominator < 1.0e-9F)
    {
        denominator = 1.0e-9F;
    }
    return fabsf(actual - expected) / denominator;
}

static float wrapAngle(float angle_rad)
{
    while (angle_rad > TEST_PI)
    {
        angle_rad -= TEST_TWO_PI;
    }
    while (angle_rad < -TEST_PI)
    {
        angle_rad += TEST_TWO_PI;
    }
    return angle_rad;
}

static VAFID_NominalParameters defaultNominal(void)
{
    VAFID_NominalParameters nominal;
    nominal.sampleTime_s = TEST_TS_S;
    nominal.currentBase_A = TEST_CURRENT_A;
    nominal.voltageBase_V = 1000.0F;
    nominal.resistance_Ohm = 0.5F;
    nominal.inductanceD_H = 0.0013F;
    nominal.inductanceQ_H = 0.00138F;
    nominal.permanentMagnetFlux_Wb = 0.046F;
    return nominal;
}

static void resetCalibrationDefaults(void)
{
    Cal_VAFID_Mode_u8 = VAFID_MODE_OFF;
    Cal_VAFID_Rst_u8 = 0u;
    Cal_VAFID_Ad_Q15_s16 = 164;
    Cal_VAFID_Aq_Q15_s16 = 164;
    Cal_VAFID_Fd_Hz_f32 = 150.0F;
    Cal_VAFID_Fq_Hz_f32 = 220.0F;
    Cal_VAFID_Settle_ms_u16 = 250u;
    Cal_VAFID_Window_ms_u16 = 100u;
    Cal_VAFID_ConsWin_u8 = 10u;
    Cal_VAFID_ConsTol_pct_u16 = 30u;
    Cal_VAFID_FitHi_pct_u16 = 65u;
    Cal_VAFID_CondLo_f32 = 0.0001F;
    Cal_VAFID_Stale_ms_u16 = 300u;
    Cal_VAFID_FbMask_u8 = 0u;
    Cal_VAFID_Apply_u8 = 0u;
    Cal_VAFID_Revert_u8 = 0u;
}

static void setEligible(uint32_t dtcSignature, uint16_t adcOffset)
{
    VAFID_FastEligibility eligibility;
    eligibility.kreClosedLoop = 1u;
    eligibility.apsfsmOff = 1u;
    eligibility.hfiOff = 1u;
    eligibility.rrcOutputInactive = 1u;
    eligibility.voltagePathStable = 1u;
    eligibility.overmodulationActive = 0u;
    eligibility.deadTimeConfigSignature = dtcSignature;
    eligibility.adcSampleOffsetTicks = adcOffset;
    VAFID_setFastEligibility(&eligibility);
}

static void testOffAndFeedbackLock(void)
{
    VAFID_NominalParameters nominal = defaultNominal();
    VAFID_DqQ15 base = {-123, 456};
    VAFID_DqQ15 output = {0, 0};

    resetCalibrationDefaults();
    VAFID_initialize();
    expectTrue(VAFID_configure(&nominal) != 0u,
        "nominal configuration must be retained while VAFID is OFF");
    VAFID_service();
    VAFID_setFastEligibility(NULL);
    VAFID_applyProbe(&base, NULL, &output);
    expectTrue((output.d == base.d) && (output.q == base.q),
        "mode off must be a bit-exact command pass-through");
    expectTrue((Meas_VAFID_Act_u8 == 0u)
        && (Meas_VAFID_ValidMask_u8 == 0u),
        "mode off must publish inactive and invalid");
    expectTrue(VAFID_requestFeedback(VAFID_FEEDBACK_RS)
        == VAFID_FEEDBACK_RESULT_LOCKED,
        "phase-one feedback request must return LOCKED");
}

static void testOffToShadowPrimeAndMissingCapture(void)
{
    VAFID_NominalParameters nominal = defaultNominal();
    VAFID_CurrentLimits limits = {-12000, 12000, -12000, 12000, 16000u};
    VAFID_DqQ15 base = {0, 2000};
    VAFID_DqQ15 output;

    resetCalibrationDefaults();
    VAFID_initialize();
    expectTrue(VAFID_configure(&nominal) != 0u,
        "OFF configure must accept valid nominal parameters without arming");
    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    VAFID_service();
    VAFID_observerStep(0.0F, 0.0F, -TEST_TS_S * 400.0F,
        400.0F, 0.046F, 1u, 0u);
    setEligible(UINT32_C(0x1001), 120u);
    VAFID_observerStep(0.0F, 0.0F, -TEST_TS_S * 400.0F,
        400.0F, 0.046F, 1u, 0u);
    expectTrue((Meas_VAFID_RejectMask_u16
        & VAFID_REJECT_VOLTAGE_STALE) == 0u,
        "first eligible observer sample must wait for PWM capture");
    VAFID_applyProbe(&base, &limits, &output);
    expectTrue(Meas_VAFID_Act_u8 != 0u,
        "OFF-to-SHADOW transition must arm and start the probe");

    VAFID_observerStep(0.0F, 0.0F, 0.0F,
        400.0F, 0.046F, 1u, 0u);
    expectTrue((Meas_VAFID_RejectMask_u16
        & VAFID_REJECT_VOLTAGE_STALE) != 0u,
        "missing an expected capture must reject as VOLTAGE_STALE");
    setEligible(UINT32_C(0x1001), 120u);
    VAFID_applyProbe(&base, &limits, &output);
    expectTrue((output.d == base.d) && (output.q == base.q)
        && (Meas_VAFID_Act_u8 == 0u),
        "abort must keep eligibility closed until observer recovery");
}

static void testCalibrationBoundsAndRecovery(void)
{
    VAFID_NominalParameters nominal = defaultNominal();
    VAFID_DqQ15 base = {0, 0};
    VAFID_DqQ15 output;

    resetCalibrationDefaults();
    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    Cal_VAFID_Ad_Q15_s16 = 3;
    VAFID_initialize();
    expectTrue(VAFID_configure(&nominal) == 0u,
        "d-axis amplitude below 4 Q15 must be rejected");

    resetCalibrationDefaults();
    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    Cal_VAFID_Aq_Q15_s16 = 3;
    VAFID_initialize();
    expectTrue(VAFID_configure(&nominal) == 0u,
        "q-axis amplitude below 4 Q15 must be rejected");

    resetCalibrationDefaults();
    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    Cal_VAFID_Fd_Hz_f32 = 9.9F;
    VAFID_initialize();
    expectTrue(VAFID_configure(&nominal) == 0u,
        "probe frequency below 10 Hz must be rejected");

    resetCalibrationDefaults();
    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    Cal_VAFID_Fq_Hz_f32 = 500.1F;
    VAFID_initialize();
    expectTrue(VAFID_configure(&nominal) == 0u,
        "probe frequency above 500 Hz must be rejected");

    resetCalibrationDefaults();
    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    Cal_VAFID_Fd_Hz_f32 = 150.0F;
    Cal_VAFID_Fq_Hz_f32 = 154.9F;
    VAFID_initialize();
    expectTrue(VAFID_configure(&nominal) == 0u,
        "probe frequency separation below 5 Hz must be rejected");

    resetCalibrationDefaults();
    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    Cal_VAFID_Ad_Q15_s16 = 4;
    Cal_VAFID_Aq_Q15_s16 = 4;
    Cal_VAFID_Fd_Hz_f32 = 10.0F;
    Cal_VAFID_Fq_Hz_f32 = 500.0F;
    VAFID_initialize();
    expectTrue(VAFID_configure(&nominal) != 0u,
        "inclusive amplitude/frequency boundaries must be accepted");

    resetCalibrationDefaults();
    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    Cal_VAFID_Ad_Q15_s16 = 1000;
    VAFID_initialize();
    expectTrue(VAFID_configure(&nominal) == 0u,
        "unsafe runtime calibration must be rejected");
    VAFID_service();
    Cal_VAFID_Ad_Q15_s16 = 164;
    VAFID_service();
    VAFID_observerStep(0.0F, 0.0F, -TEST_TS_S * 400.0F,
        400.0F, 0.046F, 1u, 0u);
    setEligible(3u, 120u);
    VAFID_applyProbe(&base, NULL, &output);
    expectTrue(Meas_VAFID_Act_u8 != 0u,
        "corrected XCP calibration must recover without reconfigure");
}

static void testSpeedAngleGatesAndAbortIdempotence(void)
{
    VAFID_NominalParameters nominal = defaultNominal();
    VAFID_DqQ15 base = {0, 2000};
    VAFID_DqQ15 output;
    const float minimumOmega = TEST_TWO_PI * 5.0F;
    const float maximumOmega = TEST_TWO_PI * 1000.0F;

    resetCalibrationDefaults();
    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    VAFID_initialize();
    (void)VAFID_configure(&nominal);
    VAFID_observerStep(0.0F, 0.0F, -TEST_TS_S * 400.0F,
        400.0F, 0.046F, 1u, 0u);
    setEligible(4u, 120u);
    VAFID_observerStep(0.0F, 0.0F, 0.0F,
        minimumOmega - 0.01F, 0.046F, 1u, 0u);
    expectTrue((Meas_VAFID_RejectMask_u16
        & VAFID_REJECT_SPEED_RANGE) != 0u,
        "electrical speed below 5 Hz must abort");
    VAFID_abortFast(VAFID_REJECT_SPEED_RANGE);
    VAFID_abortFast(VAFID_REJECT_SPEED_RANGE);
    setEligible(4u, 120u);
    VAFID_applyProbe(&base, NULL, &output);
    expectTrue(Meas_VAFID_Act_u8 == 0u,
        "repeated abort must remain closed and idempotent");

    VAFID_observerStep(0.0F, 0.0F, -TEST_TS_S * minimumOmega,
        minimumOmega, 0.046F, 1u, 0u);
    setEligible(4u, 120u);
    VAFID_observerStep(0.0F, 0.0F, -TEST_TS_S * minimumOmega,
        minimumOmega, 0.046F, 1u, 0u);
    expectTrue((Meas_VAFID_RejectMask_u16
        & VAFID_REJECT_SPEED_RANGE) == 0u,
        "exact 5 Hz boundary must be accepted");

    VAFID_observerStep(0.0F, 0.0F, 0.0F,
        maximumOmega + 0.01F, 0.046F, 1u, 0u);
    expectTrue((Meas_VAFID_RejectMask_u16
        & VAFID_REJECT_SPEED_RANGE) != 0u,
        "electrical speed above 1000 Hz must abort");

    VAFID_observerStep(0.0F, 0.0F, -TEST_TS_S * maximumOmega,
        maximumOmega, 0.046F, 1u, 0u);
    setEligible(4u, 120u);
    VAFID_observerStep(0.0F, 0.0F, -TEST_TS_S * maximumOmega,
        maximumOmega, 0.046F, 1u, 0u);
    expectTrue((Meas_VAFID_RejectMask_u16
        & VAFID_REJECT_SPEED_RANGE) == 0u,
        "exact 1000 Hz boundary must be accepted");

    VAFID_observerStep(0.0F, 0.0F, 1.0e30F,
        400.0F, 0.046F, 1u, 0u);
    expectTrue((Meas_VAFID_RejectMask_u16
        & VAFID_REJECT_KRE_INVALID) != 0u,
        "abnormally ranged finite angle must reject before wrapping");
}

static void testClippingReturnsBase(void)
{
    VAFID_NominalParameters nominal = defaultNominal();
    VAFID_CurrentLimits limits = {-100, 100, -100, 100, 100u};
    VAFID_DqQ15 base = {100, 0};
    VAFID_DqQ15 output = {0, 0};
    uint16_t sample;

    resetCalibrationDefaults();
    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    VAFID_initialize();
    (void)VAFID_configure(&nominal);
    VAFID_observerStep(0.0F, 0.0F, -TEST_TS_S * 400.0F,
        400.0F, 0.046F, 1u, 0u);
    setEligible(1u, 120u);
    for (sample = 0u; sample < 200u; sample++)
    {
        VAFID_applyProbe(&base, &limits, &output);
        if (Meas_VAFID_ProbeClip_u8 != 0u)
        {
            break;
        }
    }
    expectTrue(Meas_VAFID_ProbeClip_u8 != 0u,
        "axis/current-circle clipping must be detected");
    expectTrue((Meas_VAFID_RejectMask_u16
        & VAFID_REJECT_PROBE_CLIPPED) != 0u,
        "probe clipping must discard the window");
    expectTrue((output.d == base.d) && (output.q == base.q),
        "clipped probe must return original base command");
}

static void testExactSettleWindowAndGenerationDiscard(void)
{
    VAFID_NominalParameters nominal = defaultNominal();
    VAFID_CurrentLimits limits = {-12000, 12000, -12000, 12000, 16000u};
    VAFID_DqQ15 base = {0, 2000};
    VAFID_DqQ15 output;
    uint32_t cycle;
    const float omega = 400.0F;

    resetCalibrationDefaults();
    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    Cal_VAFID_Settle_ms_u16 = 1u;
    Cal_VAFID_Window_ms_u16 = 10u;
    Cal_VAFID_Fd_Hz_f32 = 100.0F;
    Cal_VAFID_Fq_Hz_f32 = 200.0F;
    VAFID_initialize();
    expectTrue(VAFID_configure(&nominal) != 0u,
        "short deterministic settle/window must be accepted");

    for (cycle = 0u; cycle <= 220u; cycle++)
    {
        float angle = wrapAngle(omega * (float)cycle * TEST_TS_S);
        float kreAngle = wrapAngle(angle - TEST_TS_S * omega);
        VAFID_observerStep(0.0F, 0.0F, kreAngle,
            omega, 0.046F, 1u, 0u);
        if (cycle == 219u)
        {
            expectTrue(Meas_VAFID_Win_u32 == 0u,
                "last settling tuple must not leak into window");
        }
        if (cycle == 220u)
        {
            expectTrue(Meas_VAFID_Win_u32 == 1u,
                "window must complete at 20 settle + 200 collected probes");
            break;
        }
        setEligible(8u, 120u);
        VAFID_applyProbe(&base, &limits, &output);
        if (Meas_VAFID_Act_u8 != 0u)
        {
            VAFID_captureMotorVoltage(0.0F, 0.0F, 0.0F, 0.0F, 0u);
        }
    }
    VAFID_abortFast(VAFID_REJECT_KRE_INVALID);
    VAFID_service();
    expectTrue((Meas_VAFID_ValidMask_u8 == 0u)
        && ((Meas_VAFID_RejectMask_u16 & VAFID_REJECT_KRE_INVALID) != 0u),
        "abort generation must discard completed unserviced window");
}

static void runSyntheticIdentification(float omega_radps,
                                       uint32_t dtcSignature,
                                       VAFID_Estimate *estimate)
{
    const float resistance_Ohm = 0.5F;
    const float inductanceD_H = 0.0013F;
    const float inductanceQ_H = 0.00138F;
    const float flux_Wb = 0.046F;
    const float fluxBias_Wb = 0.020F;
    VAFID_NominalParameters nominal = defaultNominal();
    VAFID_CurrentLimits limits = {-12000, 12000, -12000, 12000, 16000u};
    VAFID_DqQ15 base = {-1000, 2600};
    VAFID_DqQ15 command;
    float currentAlpha_A = 0.0F;
    float currentBeta_A = 0.0F;
    float activeFlux_Wb = flux_Wb + fluxBias_Wb;
    uint32_t appliedProbeCount = 0u;
    uint32_t sample;

    resetCalibrationDefaults();
    VAFID_initialize();
    expectTrue(VAFID_configure(&nominal) != 0u,
        "valid nominal configuration must be retained while OFF");
    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    VAFID_service();

    for (sample = 0u; sample < 28500u; sample++)
    {
        float angleCurrent = wrapAngle(omega_radps
            * (float)sample * TEST_TS_S);
        float kreDelayedAngle = wrapAngle(angleCurrent
            - (TEST_TS_S * omega_radps));
        float phaseD;
        float phaseQ;
        float idNext_A;
        float iqNext_A;
        float didNext_Aps;
        float diqNext_Aps;
        float vdPrevious_V;
        float vqPrevious_V;
        float angleNext;
        float sineNext;
        float cosineNext;
        float motorAlpha_V;
        float motorBeta_V;
        float dtcAlpha_V;
        float dtcBeta_V;

        VAFID_observerStep(currentAlpha_A, currentBeta_A, kreDelayedAngle,
            omega_radps, activeFlux_Wb, 1u, 0u);
        setEligible(dtcSignature, 120u);
        VAFID_applyProbe(&base, &limits, &command);

        if (Meas_VAFID_Act_u8 != 0u)
        {
            phaseD = TEST_TWO_PI * 150.0F * TEST_TS_S
                * (float)appliedProbeCount;
            phaseQ = TEST_TWO_PI * 220.0F * TEST_TS_S
                * (float)appliedProbeCount;
            idNext_A = ((float)command.d / TEST_Q15_SCALE) * TEST_CURRENT_A;
            iqNext_A = ((float)command.q / TEST_Q15_SCALE) * TEST_CURRENT_A;
            didNext_Aps = ((164.0F / TEST_Q15_SCALE) * TEST_CURRENT_A)
                * (TEST_TWO_PI * 150.0F) * cosf(phaseD);
            diqNext_Aps = ((164.0F / TEST_Q15_SCALE) * TEST_CURRENT_A)
                * (TEST_TWO_PI * 220.0F) * cosf(phaseQ);
            vdPrevious_V = (resistance_Ohm * idNext_A)
                + (inductanceD_H * didNext_Aps)
                - (omega_radps * inductanceQ_H * iqNext_A);
            vqPrevious_V = (resistance_Ohm * iqNext_A)
                + (inductanceQ_H * diqNext_Aps)
                + (omega_radps * ((inductanceD_H * idNext_A) + flux_Wb));
            angleNext = wrapAngle(omega_radps
                * (float)(sample + 1u) * TEST_TS_S);
            sineNext = sinf(angleNext);
            cosineNext = cosf(angleNext);
            currentAlpha_A = (cosineNext * idNext_A)
                - (sineNext * iqNext_A);
            currentBeta_A = (sineNext * idNext_A)
                + (cosineNext * iqNext_A);
            motorAlpha_V = (cosineNext * vdPrevious_V)
                - (sineNext * vqPrevious_V);
            motorBeta_V = (sineNext * vdPrevious_V)
                + (cosineNext * vqPrevious_V);
            dtcAlpha_V = 0.31F * cosineNext;
            dtcBeta_V = 0.31F * sineNext;
            activeFlux_Wb = flux_Wb + fluxBias_Wb
                + ((inductanceD_H - inductanceQ_H) * idNext_A);
            VAFID_captureMotorVoltage(motorAlpha_V + dtcAlpha_V,
                motorBeta_V + dtcBeta_V, dtcAlpha_V, dtcBeta_V, 0u);
            appliedProbeCount++;
        }
        if ((sample % 20u) == 0u)
        {
            VAFID_service();
        }
    }
    VAFID_service();
    (void)VAFID_getEstimate(estimate);
}

static void testIdentificationAndAngleAdvance(void)
{
    VAFID_Estimate positiveEstimate;
    VAFID_Estimate negativeEstimate;

    runSyntheticIdentification(400.0F, UINT32_C(0x5A17D7C0),
        &positiveEstimate);
    expectTrue(positiveEstimate.validMask
        == (VAFID_VALID_RS | VAFID_VALID_LD | VAFID_VALID_LQ),
        "unqualified active flux must suppress only bit 0x08");
    expectTrue(relativeError(positiveEstimate.resistance_Ohm, 0.5F) < 0.04F,
        "positive-speed u[k-1]/i[k] replay must identify Rs");
    expectTrue(relativeError(positiveEstimate.inductanceD_H, 0.0013F) < 0.04F,
        "positive-speed angle advance must identify Ld");
    expectTrue(relativeError(positiveEstimate.inductanceQ_H, 0.00138F) < 0.04F,
        "positive-speed angle advance must identify Lq");
    expectTrue(isfinite(positiveEstimate.fitError)
        && isfinite(positiveEstimate.conditionProxy),
        "positive-speed slow frame and LS must stay finite");
    expectTrue(relativeError(Meas_VAFID_FluxPM_Wb_f32, 0.066F) < 0.04F,
        "unqualified biased FluxPM must remain diagnostic");

    setEligible(UINT32_C(0x5A17D7C1), 120u);
    expectTrue((Meas_VAFID_RejectMask_u16
        & VAFID_REJECT_DTC_CHANGED) != 0u,
        "DTC signature change must invalidate and re-settle");
    setEligible(UINT32_C(0x5A17D7C1), 121u);
    expectTrue((Meas_VAFID_RejectMask_u16
        & VAFID_REJECT_ADC_CHANGED) != 0u,
        "ADC offset change must invalidate and re-settle");

    Cal_VAFID_Mode_u8 = VAFID_MODE_OFF;
    VAFID_service();
    expectTrue((Meas_VAFID_ValidMask_u8 == 0u)
        && (Meas_VAFID_Stat_u8 == VAFID_STATUS_OFF),
        "Mode OFF service must invalidate without fast-loop progress");

    runSyntheticIdentification(-400.0F, UINT32_C(0x5A17D7D0),
        &negativeEstimate);
    expectTrue(negativeEstimate.validMask
        == (VAFID_VALID_RS | VAFID_VALID_LD | VAFID_VALID_LQ),
        "negative-speed replay must reach shadow-valid mask");
    expectTrue(relativeError(negativeEstimate.resistance_Ohm, 0.5F) < 0.04F,
        "negative-speed u[k-1]/i[k] replay must identify Rs");
    expectTrue(relativeError(negativeEstimate.inductanceD_H, 0.0013F) < 0.04F,
        "negative-speed angle advance must identify Ld");
    expectTrue(relativeError(negativeEstimate.inductanceQ_H, 0.00138F) < 0.04F,
        "negative-speed angle advance must identify Lq");
}

static int runAllTests(void)
{
    testFailureCount = 0;
    testOffAndFeedbackLock();
    testOffToShadowPrimeAndMissingCapture();
    testCalibrationBoundsAndRecovery();
    testSpeedAngleGatesAndAbortIdempotence();
    testClippingReturnsBase();
    testExactSettleWindowAndGenerationDiscard();
    testIdentificationAndAngleAdvance();
    if (testFailureCount == 0)
    {
        (void)printf("PASS: VAFID host tests\n");
    }
    return testFailureCount;
}

#if defined(MATLAB_MEX_FILE)
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int failures;
    (void)nlhs;
    (void)plhs;
    (void)nrhs;
    (void)prhs;
    failures = runAllTests();
    if (failures != 0)
    {
        mexErrMsgIdAndTxt("VAFID:HostTestFailed",
            "%d VAFID host tests failed", failures);
    }
}
#else
int main(void)
{
    return runAllTests();
}
#endif
