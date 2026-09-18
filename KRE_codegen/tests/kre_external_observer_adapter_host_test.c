#include "../kre_external_observer_adapter.h"
#include "../kre_external_observer_wrapper_ert_rtw/kre_external_observer_wrapper.h"
#include "../../VAFID_codegen/vafid_parameter_identifier.h"
#include "../../MS/include/Ifx_MS_FocSolutionF16.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../../ConfigWizard/FocTiming_Cfg.h"
#if FOC_CONTROL_PERIOD_US == 100u
#include "kre_control10k_vectors.h"
#endif

#define TEST_FLOAT_TOLERANCE (1.0e-7F)

volatile uint16_t Cal_MotorAppliedPhaseResistance_mOhm_u16 = 500u;
volatile uint16_t Cal_MotorAppliedDirectInductance_uH_u16 = 1300u;
volatile uint16_t Cal_MotorAppliedQuadratureInductance_uH_u16 = 1380u;
volatile uint8_t Cal_MotorAppliedPolePairs_u8 = 4u;
volatile uint16_t Cal_MTPA_AppliedPermanentMagnetFlux_mWb_u16 = 46u;
volatile uint8_t Cal_VAFID_Mode_u8 = VAFID_MODE_OFF;

static uint32_t vafidAbortCount;
static uint32_t vafidObserverStepCount;

void VAFID_abortFast(uint16_t rejectMask)
{
    (void)rejectMask;
    ++vafidAbortCount;
}

void VAFID_observerStep(float currentAlpha_A,
                        float currentBeta_A,
                        float kreElectricalAngle_rad,
                        float kreRawElectricalOmega_radps,
                        float kreActiveFlux_Wb,
                        uint8_t kreValid,
                        uint8_t activeFluxQualified)
{
    (void)currentAlpha_A;
    (void)currentBeta_A;
    (void)kreElectricalAngle_rad;
    (void)kreRawElectricalOmega_radps;
    (void)kreActiveFlux_Wb;
    (void)kreValid;
    (void)activeFluxQualified;
    ++vafidObserverStepCount;
}

static void assertFloatNear(float actual, float expected)
{
    assert(fabsf(actual - expected) <= TEST_FLOAT_TOLERANCE);
}

static void restoreNominalCalibrations(void)
{
    Cal_MotorAppliedPhaseResistance_mOhm_u16 = 500u;
    Cal_MotorAppliedDirectInductance_uH_u16 = 1300u;
    Cal_MotorAppliedQuadratureInductance_uH_u16 = 1380u;
    Cal_MotorAppliedPolePairs_u8 = 4u;
    Cal_MTPA_AppliedPermanentMagnetFlux_mWb_u16 = 46u;
    Cal_Kre_Alpha_radps_f32 = 1256.637061F;
    Cal_Kre_A_radps_f32 = 125.663704F;
    Cal_Kre_Gamma_f32 = 1.0F;
    Cal_Kre_SigmaEpsilon_Wb_f32 = 0.00001F;
    Cal_Kre_SpeedFilter_Hz_f32 = 100.0F;
    Cal_Kre_PllBandwidth_Hz_f32 = 50.0F;
    Cal_Kre_PllDamping_f32 = 0.70710678F;
}

static void testInitialSnapshotApply(void)
{
    restoreNominalCalibrations();
    Cal_VAFID_Mode_u8 = VAFID_MODE_OFF;
    vafidAbortCount = 0u;
    vafidObserverStepCount = 0u;

    KreExternalObserver_initialize();
    assert(Meas_KRE_ParamValid_u8 == 1u);
    assert(Meas_KRE_ParamPending_u8 == 0u);
    assert(KreExternalObserver_parametersReady() == 1u);
    assert(Meas_KRE_RstCount_u32 == 1u);
    assert(Meas_KRE_RstReason_u8 == KRE_EXTERNAL_OBSERVER_RST_INITIALIZE);
    assert(vafidAbortCount == 0u);

    assert(KreExternalObserver_stageParameters() == 1u);
    assert(Meas_KRE_ParamPending_u8 == 0u);
    assert(KreExternalObserver_applyStagedParameters() == 1u);
    assert(Meas_KRE_ParamValid_u8 == 1u);
    assert(Meas_KRE_ParamPending_u8 == 0u);
    assert(Meas_KRE_ParamApplySeq_u32 == 1u);
    assert(Meas_KRE_RstCount_u32 == 1u);
    assert(Meas_KRE_RstReason_u8
        == KRE_EXTERNAL_OBSERVER_RST_INITIALIZE);
    assert(KreExternalObserver_parametersReady() == 1u);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[0], 0.5F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[1], 0.0013F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[2], 0.00138F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[3], 0.046F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[8], 4.0F);
    assertFloatNear(Meas_Kre_RotorFluxMag_Wb_f32, 0.046F);
}

static void testChangedSnapshotRemainsPendingUntilApply(void)
{
    float appliedResistance = kre_external_observer_wrapper_U.kreParams[0];
    uint32_t applySequence = Meas_KRE_ParamApplySeq_u32;
    uint32_t resetCount = Meas_KRE_RstCount_u32;

    Cal_MotorAppliedPhaseResistance_mOhm_u16 = 600u;
    assert(KreExternalObserver_stageParameters() == 1u);
    assert(Meas_KRE_ParamValid_u8 == 1u);
    assert(Meas_KRE_ParamPending_u8 == 1u);
    assert(KreExternalObserver_parametersReady() == 0u);
    /* Staging never changes the parameters used by the fast loop. */
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[0],
        appliedResistance);
    assert(KreExternalObserver_applyStagedParameters() == 1u);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[0], 0.6F);
    assert(Meas_KRE_ParamApplySeq_u32 == applySequence + 1u);
    assert(Meas_KRE_RstCount_u32 == resetCount);

    Cal_MotorAppliedPhaseResistance_mOhm_u16 = 500u;
    assert(KreExternalObserver_stageParameters() == 1u);
    assert(KreExternalObserver_applyStagedParameters() == 1u);
    assert(KreExternalObserver_parametersReady() == 1u);
}

static void testSnapshotServiceDoesNotAddRangeGate(void)
{
    float angle;
    float speed;
    uint32_t applySequence = Meas_KRE_ParamApplySeq_u32;
    uint32_t resetCount;

    Cal_MotorAppliedDirectInductance_uH_u16 = 0u;
    Cal_MotorAppliedQuadratureInductance_uH_u16 = 0u;
    Cal_MTPA_AppliedPermanentMagnetFlux_mWb_u16 = 0u;
    Cal_MotorAppliedPolePairs_u8 = 0u;
    Cal_Kre_Gamma_f32 = 0.0F;
    Cal_Kre_SpeedFilter_Hz_f32 = -1.0F;
    assert(KreExternalObserver_stageParameters() == 1u);
    assert(KreExternalObserver_applyStagedParameters() == 1u);
    assert(Meas_KRE_ParamApplySeq_u32 == applySequence + 1u);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[1], 0.0F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[2], 0.0F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[3], 0.0F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[8], 0.0F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[11], 0.0F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[13], -1.0F);

    /* Parameter ranges are not rejected by the snapshot or the fast step.
     * This zero-input case stays finite despite physically invalid settings;
     * VALID is not a guarantee that the calibration is physically usable. */
    resetCount = Meas_KRE_RstCount_u32;
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    assert(Meas_Kre_Valid_u8 == 1u);
    assert(Meas_Kre_Status_u8
        == KRE_EXTERNAL_OBSERVER_STATUS_VALID);
    assert(KreExternalObserver_getFocEstimate(&angle, &speed) == 1u);
    assert(Meas_KRE_RstCount_u32 == resetCount);

    restoreNominalCalibrations();
    assert(KreExternalObserver_stageParameters() == 1u);
    assert(KreExternalObserver_applyStagedParameters() == 1u);
}

static void testChangedValidSnapshotAppliesOnce(void)
{
    float oldAlpha = kre_external_observer_wrapper_U.kreParams[9];
    uint32_t applySequence = Meas_KRE_ParamApplySeq_u32;
    uint32_t resetCount = Meas_KRE_RstCount_u32;

    Cal_Kre_Alpha_radps_f32 = oldAlpha + 10.0F;
    assert(KreExternalObserver_stageParameters() == 1u);
    assert(Meas_KRE_ParamPending_u8 == 1u);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[9], oldAlpha);

    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[9], oldAlpha);
    assert(KreExternalObserver_applyStagedParameters() == 1u);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[9],
        oldAlpha + 10.0F);
    assert(Meas_KRE_ParamApplySeq_u32 == applySequence + 1u);
    assert(Meas_KRE_RstCount_u32 == resetCount);
    assert(KreExternalObserver_applyStagedParameters() == 1u);
    assert(Meas_KRE_RstCount_u32 == resetCount);
}

static void testFaultResetEdgeAndVafidBypass(void)
{
    uint32_t resetCount;
    uint32_t observerCount;
    float appliedResistance;

    /* Consume the parameter-apply reset and establish a valid sample. */
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    resetCount = Meas_KRE_RstCount_u32;
    KreExternalObserver_execute(NAN, 0.0F, 0.0F, 0.0F);
    KreExternalObserver_execute(NAN, 0.0F, 0.0F, 0.0F);
    assert(Meas_KRE_RstCount_u32 == resetCount + 1u);
    assert(Meas_KRE_RstReason_u8
        == KRE_EXTERNAL_OBSERVER_RST_INPUT_INVALID);
    assert(vafidObserverStepCount == 0u);

    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    resetCount = Meas_KRE_RstCount_u32;
    appliedResistance = kre_external_observer_wrapper_U.kreParams[0];
    kre_external_observer_wrapper_U.kreParams[0] = NAN;
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    assert(Meas_KRE_RstCount_u32 == resetCount + 1u);
    assert(Meas_KRE_RstReason_u8
        == KRE_EXTERNAL_OBSERVER_RST_OUTPUT_INVALID);
    kre_external_observer_wrapper_U.kreParams[0] = appliedResistance;

    Cal_VAFID_Mode_u8 = VAFID_MODE_SHADOW;
    KreExternalObserver_reset();
    assert(vafidAbortCount == (FOC_AUX_ALGORITHMS_ENABLE ? 1u : 0u));
    observerCount = vafidObserverStepCount;
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    assert(vafidObserverStepCount == observerCount + (FOC_AUX_ALGORITHMS_ENABLE ? 1u : 0u));

    Cal_VAFID_Mode_u8 = VAFID_MODE_OFF;
    KreExternalObserver_reset();
    assert(vafidAbortCount == (FOC_AUX_ALGORITHMS_ENABLE ? 1u : 0u));
    observerCount = vafidObserverStepCount;
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    assert(vafidObserverStepCount == observerCount);
}

static void testHandoffUsesObserverValidityOnly(void)
{
    float angle;
    float speed;

    Meas_Kre_Valid_u8 = 1u;
    Meas_Kre_Status_u8 = KRE_EXTERNAL_OBSERVER_STATUS_VALID;
    Meas_Kre_ElecAngle_rad_f32 = 1.0F;
    Meas_Kre_MechSpeed_rpm_f32 = 100.0F;
    Meas_KRE_ActFlux_Wb_f32 = Cal_Kre_SigmaEpsilon_Wb_f32 * 0.5F;
    assert(KreExternalObserver_getFocEstimate(&angle, &speed) == 1u);
    assertFloatNear(angle, 1.0F);
    assertFloatNear(speed, 100.0F);

    Meas_Kre_Valid_u8 = 0u;
    assert(KreExternalObserver_getFocEstimate(&angle, &speed) == 0u);
}

static uint32_t replayHashFloat(uint32_t hash, float value)
{
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return (hash ^ bits) * 16777619u;
}

static void assertCachedCoefficients(void)
{
    const float *p = kre_external_observer_wrapper_U.kreParams;
    const float *pll = kre_external_observer_wrapper_U.krePllParams;
    const float wn = 6.2831855F * pll[0];
    assertFloatNear(kre_external_observer_coefficients.h2Gain,
        1.0F - expf(-p[9] * p[4]));
    assertFloatNear(kre_external_observer_coefficients.speedGain,
        1.0F - expf(-6.2831855F * p[13] * p[4]));
    assertFloatNear(kre_external_observer_coefficients.pllKp,
        2.0F * pll[1] * wn);
    assertFloatNear(kre_external_observer_coefficients.pllKiTs,
        wn * wn * p[4]);
    assertFloatNear(kre_external_observer_coefficients.inductanceDelta,
        p[1] - p[2]);
}

static void testEveryCalibrationRefreshesWithoutReset(void)
{
    uint32_t parameter;
    restoreNominalCalibrations();
    KreExternalObserver_initialize();
    assertCachedCoefficients();
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    for (parameter = 0u; parameter < 12u; ++parameter)
    {
        ExtU_kre_external_observer_wr_T beforeInputs;
        DW_kre_external_observer_wrap_T beforeStates;
        KreDiscreteCoefficients beforeCoefficients;
        const uint32_t sequence = Meas_KRE_ParamApplySeq_u32;
        const uint32_t resets = Meas_KRE_RstCount_u32;
        const uint8_t valid = Meas_Kre_Valid_u8;
        /* Model running state, not a fresh all-zero observer. Publication must
         * preserve every state byte, even when motor or PLL gains change. */
        kre_external_observer_wrappe_DW.lambdaHat[0] = 0.04F;
        kre_external_observer_wrappe_DW.lambdaHat[1] = 0.02F;
        kre_external_observer_wrappe_DW.pllTheta = 0.4F;
        kre_external_observer_wrappe_DW.pllOmegaInt = 100.0F;
        kre_external_observer_wrappe_DW.pllInitialized = true;
        beforeInputs = kre_external_observer_wrapper_U;
        beforeStates = kre_external_observer_wrappe_DW;
        beforeCoefficients = kre_external_observer_coefficients;
        switch (parameter)
        {
        case 0u: Cal_Kre_Alpha_radps_f32 += 10.0F; break;
        case 1u: Cal_Kre_A_radps_f32 += 1.0F; break;
        case 2u: Cal_Kre_Gamma_f32 += 0.1F; break;
        case 3u: Cal_Kre_SigmaEpsilon_Wb_f32 *= 2.0F; break;
        case 4u: Cal_Kre_SpeedFilter_Hz_f32 += 20.0F; break;
        case 5u: Cal_Kre_PllBandwidth_Hz_f32 += 10.0F; break;
        case 6u: Cal_Kre_PllDamping_f32 += 0.01F; break;
        case 7u: Cal_MotorAppliedPhaseResistance_mOhm_u16 += 1u; break;
        case 8u: Cal_MotorAppliedDirectInductance_uH_u16 += 1u; break;
        case 9u: Cal_MotorAppliedQuadratureInductance_uH_u16 += 1u; break;
        case 10u: Cal_MTPA_AppliedPermanentMagnetFlux_mWb_u16 += 1u; break;
        default: Cal_MotorAppliedPolePairs_u8 += 1u; break;
        }
        assert(KreExternalObserver_stageParameters() == 1u);
        assert(Meas_KRE_ParamPending_u8 == 1u);
        assert(memcmp(&beforeInputs, &kre_external_observer_wrapper_U,
            sizeof(beforeInputs)) == 0);
        assert(memcmp(&beforeCoefficients, &kre_external_observer_coefficients,
            sizeof(beforeCoefficients)) == 0);
        assert(KreExternalObserver_applyStagedParameters() == 1u);
        assert(Meas_KRE_ParamApplySeq_u32 == sequence + 1u);
        assert(Meas_KRE_RstCount_u32 == resets);
        assert(Meas_Kre_Valid_u8 == valid);
        assert(memcmp(&beforeStates, &kre_external_observer_wrappe_DW,
            sizeof(beforeStates)) == 0);
        assertCachedCoefficients();
        assert(KreExternalObserver_stageParameters() == 1u);
        assert(Meas_KRE_ParamPending_u8 == 0u);
        assert(KreExternalObserver_applyStagedParameters() == 1u);
        assert(Meas_KRE_ParamApplySeq_u32 == sequence + 1u);
        assert(Meas_KRE_RstCount_u32 == resets);
        KreExternalObserver_execute(1.0F, 2.0F, 0.1F, 0.2F);
        assert(Meas_Kre_Valid_u8 == 1u);
        assert(Meas_KRE_RstCount_u32 == resets);
    }
    restoreNominalCalibrations();
}

static void testValidParameterReplay(void)
{
    uint32_t hash = 2166136261u;
    uint32_t index;
    uint32_t scenario;
    restoreNominalCalibrations();
    Cal_VAFID_Mode_u8 = VAFID_MODE_OFF;
    KreExternalObserver_initialize();
    for (scenario = 0u; scenario < 3u; ++scenario)
    {
        Cal_Kre_SpeedFilter_Hz_f32 = (scenario == 0u) ? 0.0F : 150.0F;
        Cal_Kre_PllBandwidth_Hz_f32 = (scenario == 2u) ? 100.0F : 50.0F;
        assert(KreExternalObserver_stageParameters() == 1u);
        assert(KreExternalObserver_applyStagedParameters() == 1u);
        KreExternalObserver_reset();
        for (index = 0u; index < 2000u; ++index)
        {
            const float direction = (scenario == 2u) ? -1.0F : 1.0F;
            const float angle = direction * (float)index * 0.031415927F;
            const float ia = 2.0F * cosf(angle);
            const float ib = 2.0F * sinf(angle);
            if (index == 1000u)
            {
                KreExternalObserver_reset();
            }
            KreExternalObserver_execute(0.5F * ia - 28.0F * sinf(angle),
                0.5F * ib + 28.0F * cosf(angle), ia, ib);
            assert(isfinite(Meas_Kre_ElecAngle_rad_f32));
            assert(isfinite(Meas_Kre_MechSpeed_rpm_f32));
            assert(isfinite(Meas_KRE_ActFlux_Wb_f32));
            assert(isfinite(Meas_KRE_ActFluxA_Wb_f32));
            assert(isfinite(Meas_KRE_ActFluxB_Wb_f32));
            {
                const float fluxA = Meas_KRE_ActFluxA_Wb_f32;
                const float fluxB = Meas_KRE_ActFluxB_Wb_f32;
                assertFloatNear(sqrtf(fluxA * fluxA + fluxB * fluxB),
                    Meas_KRE_ActFlux_Wb_f32);
            }
            assert(isfinite(Meas_KRE_RawOmega_radps_f32));
            hash = replayHashFloat(hash, Meas_Kre_ElecAngle_rad_f32);
            hash = replayHashFloat(hash, Meas_Kre_MechSpeed_rpm_f32);
            hash = replayHashFloat(hash, Meas_KRE_ActFlux_Wb_f32);
            hash = replayHashFloat(hash, Meas_KRE_RawOmega_radps_f32);
            hash = (hash ^ Meas_Kre_Valid_u8) * 16777619u;
            hash = (hash ^ Meas_Kre_Status_u8) * 16777619u;
        }
    }
    printf("KRE valid replay hash: %08lx\n", (unsigned long)hash);
    /* IAR 9.40.1 -Oh, captured before removing parameter checks. */
#if FOC_CONTROL_PERIOD_US == 50u
    assert(hash == 0x7b81d650u);
#endif
}

static void testSharedMotorPreparationAndJointPublication(void)
{
    static Ifx_MS_FocSolutionF16 controller;
    Ifx_MS_FocSolutionF16_MotorCoefficients prepared;
    Ifx_MS_FocSolutionF16_MotorCoefficients saved;
    uint32_t resets;
    restoreNominalCalibrations();
    KreExternalObserver_initialize();
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
    resets = Meas_KRE_RstCount_u32;
    memset(&controller, 0, sizeof(controller));
    controller.iToF.p_anglePreviousValue = 123456u;
    assert(Ifx_MS_FocSolutionF16_prepareMotorParameters(&prepared,
        500u, 1300u, 1380u, 4u));
    saved = prepared;
    /* Failed preparation must not overwrite the accepted snapshot. */
    assert(!Ifx_MS_FocSolutionF16_prepareMotorParameters(&prepared,
        500u, 1300u, 1380u, 0u));
    assert(memcmp(&prepared, &saved, sizeof(prepared)) == 0);
    assert(!Ifx_MS_FocSolutionF16_prepareMotorParameters(&prepared,
        65535u, 1300u, 1380u, 4u));
    assert(memcmp(&prepared, &saved, sizeof(prepared)) == 0);
    assert(Ifx_MS_FocSolutionF16_prepareMotorParameters(&prepared,
        600u, 1400u, 1500u, 5u));
    assert(KreExternalObserver_stageMotorParameters(600u, 1400u,
        1500u, 48u, 5u) == 1u);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[0], 0.5F);
    assert(Cal_MotorAppliedPhaseResistance_mOhm_u16 == 500u);
    /* Simulate the project's single IRQ-excluded publication. */
    Ifx_MS_FocSolutionF16_applyMotorParameters(&controller, &prepared);
    Cal_MotorAppliedPhaseResistance_mOhm_u16 = 600u;
    Cal_MotorAppliedDirectInductance_uH_u16 = 1400u;
    Cal_MotorAppliedQuadratureInductance_uH_u16 = 1500u;
    Cal_MTPA_AppliedPermanentMagnetFlux_mWb_u16 = 48u;
    Cal_MotorAppliedPolePairs_u8 = 5u;
    assert(KreExternalObserver_applyStagedParameters() == 1u);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[0], 0.6F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[1], 0.0014F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[2], 0.0015F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[3], 0.048F);
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[8], 5.0F);
    assertCachedCoefficients();
    assert(controller.iToF.p_anglePreviousValue == 123456u);
    assert(controller.iToF.p_angleIncrementQ14 == prepared.angleIncrementQ14);
    assert(controller.vToF.p_angleIncrementQ14 == prepared.angleIncrementQ14);
    assert(controller.fluxEstimator.p_phaseResistanceQ15 == prepared.phaseResistanceQ15);
    assert(Meas_KRE_RstCount_u32 == resets);
    assert(Meas_Kre_Valid_u8 == 1u);
    assert(KreExternalObserver_stageParameters() == 1u);
    assert(Meas_KRE_ParamPending_u8 == 0u);
    restoreNominalCalibrations();
}

static void assertFluxComponentsClear(void)
{
    assert(Meas_KRE_ActFluxA_Wb_f32 == 0.0F);
    assert(Meas_KRE_ActFluxB_Wb_f32 == 0.0F);
}

static void testActiveFluxComponents(void)
{
    uint32_t quadrant;
    restoreNominalCalibrations();
    Cal_VAFID_Mode_u8 = VAFID_MODE_OFF;
    KreExternalObserver_initialize();
    assertFluxComponentsClear();
    for (quadrant = 0u; quadrant < 4u; ++quadrant)
    {
        const float lambdaA = ((quadrant & 1u) != 0u) ? -0.04F : 0.04F;
        const float lambdaB = ((quadrant & 2u) != 0u) ? -0.03F : 0.03F;
        float fluxA;
        float fluxB;
        KreExternalObserver_reset();
        assertFluxComponentsClear();
        KreExternalObserver_execute(3.0F, -4.0F, 2.0F, -3.0F);
        assertFluxComponentsClear();
        kre_external_observer_wrappe_DW.lambdaHat[0] = lambdaA;
        kre_external_observer_wrappe_DW.lambdaHat[1] = lambdaB;
        /* Different new currents detect accidental use of the current sample;
         * nonzero voltage makes a post-Euler state read observably incorrect. */
        KreExternalObserver_execute(8.0F, -9.0F, -7.0F, 11.0F);
        fluxA = Meas_KRE_ActFluxA_Wb_f32;
        fluxB = Meas_KRE_ActFluxB_Wb_f32;
        assertFloatNear(fluxA, lambdaA - 0.00138F * 2.0F);
        assertFloatNear(fluxB, lambdaB - 0.00138F * -3.0F);
        assertFloatNear(sqrtf(fluxA * fluxA + fluxB * fluxB),
            Meas_KRE_ActFlux_Wb_f32);
        assert(Meas_Kre_Valid_u8 == 1u);
        if (quadrant == 0u)
        {
            KreExternalObserver_execute(NAN, 0.0F, 0.0F, 0.0F);
            assertFluxComponentsClear();
            assert(Meas_Kre_Valid_u8 == 0u);
            /* Fault recovery retains the previously delayed current. The
             * reset clears lambda but does not skip the generated step. */
            KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
            assertFloatNear(Meas_KRE_ActFluxA_Wb_f32, 0.00138F * 7.0F);
            assertFloatNear(Meas_KRE_ActFluxB_Wb_f32, -0.00138F * 11.0F);
            assert(Meas_Kre_Valid_u8 == 0u);
        }
        else if (quadrant == 1u)
        {
            kre_external_observer_wrappe_DW.lambdaHat[0] = NAN;
            KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
            assertFluxComponentsClear();
            assert(Meas_Kre_Valid_u8 == 0u);
        }
        else if (quadrant == 2u)
        {
            Meas_KRE_ParamValid_u8 = 0u;
            KreExternalObserver_execute(0.0F, 0.0F, 0.0F, 0.0F);
            assertFluxComponentsClear();
            assert(Meas_Kre_Status_u8 == KRE_EXTERNAL_OBSERVER_STATUS_PARAMETER_INVALID);
            Meas_KRE_ParamValid_u8 = 1u;
        }
        else
        {
            KreExternalObserver_execute(0.0F, INFINITY, 0.0F, 0.0F);
            assertFluxComponentsClear();
            assert(Meas_Kre_Valid_u8 == 0u);
        }
    }
    KreExternalObserver_reset();
    assertFluxComponentsClear();
}

#if FOC_CONTROL_PERIOD_US == 100u
static void testControl10kMatlabReplay(void)
{
    unsigned n, j;
    float maxima[4] = {0};
    float last[4] = {0};
    const float tol[4] = {REPLAY_ANGLE_TOL, REPLAY_SPEED_TOL,
        REPLAY_FLUX_TOL, REPLAY_OMEGA_TOL};
    restoreNominalCalibrations();
    /* Replay the production filter/PLL defaults, not the legacy test tuning. */
    Cal_Kre_SpeedFilter_Hz_f32 = 150.0F;
    Cal_Kre_PllBandwidth_Hz_f32 = 100.0F;
    KreExternalObserver_initialize();
    assertFloatNear(kre_external_observer_wrapper_U.kreParams[4], 0.0001F);
    for (n = 0; n < sizeof(replay)/sizeof(replay[0]); ++n)
    {
        float actual[4];
        const float *row = replay[n];
        if (row[8] != 0.0F) {
            KreExternalObserver_reset();
            memset(last, 0, sizeof(last));
        }
        KreExternalObserver_execute(row[0], row[1], row[2], row[3]);
        /* Adapter owns exactly one CONTROL-sample delay. */
        for (j=0; j<4; ++j) {
            assertFloatNear(kre_external_observer_wrapper_U.kreViFb[j], last[j]);
            last[j] = row[j] / ((j < 2) ? 1000.0F : 50.0F);
        }
        actual[0] = Meas_Kre_ElecAngle_rad_f32;
        actual[1] = Meas_Kre_MechSpeed_rpm_f32;
        actual[2] = Meas_KRE_ActFlux_Wb_f32;
        actual[3] = Meas_KRE_RawOmega_radps_f32;
        for (j=0; j<4; ++j) {
            float error = fabsf(actual[j]-row[j+4]);
            if (j==0 && error > 3.14159265F) error = fabsf(error-6.283185307F);
            if (error > maxima[j]) maxima[j] = error;
            if (!isfinite(actual[j]) || error > tol[j]) {
                printf("Replay failure row %u output %u error %.9g tol %.9g\n",n,j,(double)error,(double)tol[j]);
                assert(0);
            }
        }
        assert(Meas_Kre_Valid_u8 == ((row[8] != 0.0F) ? 0u : 1u));
    }
    printf("100 us MATLAB replay: %u rows; max errors %.9g %.9g %.9g %.9g\n",
        n,(double)maxima[0],(double)maxima[1],(double)maxima[2],(double)maxima[3]);
}
#endif

static void testCurrentPiControlPeriod(void)
{
    Ifx_MDA_FocControllerF16 controller, reference;
    Ifx_Math_PiF16 *current[2], *old[2];
    unsigned axis, n, sub;
    const unsigned ratio = FOC_CONTROL_PERIOD_US / 50u;
    Ifx_MDA_FocControllerF16_init(&controller);
    reference = controller;
    current[0] = &controller.currentDPi; current[1] = &controller.currentQPi;
    old[0] = &reference.currentDPi; old[1] = &reference.currentQPi;
    for (axis=0; axis<2; ++axis) {
        int32_t initial, delta;
        assert(current[axis]->p_integGainSamplingTime.value == 40*ratio);
        assert(current[axis]->p_antiWindupGainSamplingTime.value == 81*ratio);
        assert(current[axis]->p_propGain.value == 6997);
        Ifx_Math_PiF16_setIntegGainSamplingTime(old[axis],40);
        Ifx_Math_PiF16_setAntiWindupGainSamplingTime(old[axis],81);
        for (n=0; n<100; ++n) {
            const int16_t error = axis ? -2048 : 2048;
            (void)Ifx_Math_PiF16_execute(current[axis],error);
            for(sub=0; sub<ratio; ++sub) (void)Ifx_Math_PiF16_execute(old[axis],error);
            /* Equal physical time, exactly representable unsaturated error. */
            assert(current[axis]->p_integPreviousValue == old[axis]->p_integPreviousValue);
        }
        initial = (int32_t)current[axis]->p_upperLimit*512 + 1048576;
        current[axis]->p_integPreviousValue = old[axis]->p_integPreviousValue = initial;
        (void)Ifx_Math_PiF16_execute(old[axis],0);
        (void)Ifx_Math_PiF16_execute(current[axis],0);
        delta = current[axis]->p_integPreviousValue - initial;
        assert(delta < 0);
        assert(delta == (old[axis]->p_integPreviousValue - initial)*(int32_t)ratio);
    }
    Ifx_MDA_FocControllerF16_reset(&controller);
    assert(controller.currentDPi.p_integPreviousValue == 0);
    assert(controller.currentQPi.p_integPreviousValue == 0);
    puts("Current PI period scaling and anti-windup tests passed");
}

int main(void)
{
    testCurrentPiControlPeriod();
    testInitialSnapshotApply();
    testChangedSnapshotRemainsPendingUntilApply();
    testSnapshotServiceDoesNotAddRangeGate();
    testChangedValidSnapshotAppliesOnce();
    testFaultResetEdgeAndVafidBypass();
    testHandoffUsesObserverValidityOnly();
    testValidParameterReplay();
    testEveryCalibrationRefreshesWithoutReset();
    testSharedMotorPreparationAndJointPublication();
    testActiveFluxComponents();
#if FOC_CONTROL_PERIOD_US == 100u
    testControl10kMatlabReplay();
#endif
    puts("KRE adapter tests passed");
    return 0;
}
