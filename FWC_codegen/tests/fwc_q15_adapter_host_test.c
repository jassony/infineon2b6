#include "../../ConfigWizard/FocTiming_Cfg.h"
#include "../fwc_q15_adapter.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static Fwc_Q15_VoltageSnapshot makeSnapshot(const float requestUtilization,
                                             const float actualUtilization,
                                             const uint8_t saturated,
                                             const uint32_t saturationStreak,
                                             const uint32_t unsaturationStreak)
{
    Fwc_Q15_VoltageSnapshot snapshot;
    const float dcLinkQ15 = 30000.0F;

    snapshot.sequence = 2u;
    snapshot.saturationStreakFast = saturationStreak;
    snapshot.unsaturationStreakFast = unsaturationStreak;
    snapshot.idAtFloorSaturationStreakFast = saturationStreak;
    snapshot.requestedVoltageQ15 = (Ifx_Math_Fract16)((requestUtilization
        * dcLinkQ15 / 1.7320508F) + 0.5F);
    snapshot.actualVoltageQ15 = (Ifx_Math_Fract16)((actualUtilization
        * dcLinkQ15 / 1.7320508F) + 0.5F);
    snapshot.dcLinkVoltageQ15 = (Ifx_Math_Fract16)dcLinkQ15;
    snapshot.valid = 1u;
    snapshot.saturated = saturated;
    return snapshot;
}

static void restoreCalibration(void)
{
    Cal_FWC_Enable_u8 = 1u;
    Cal_FWC_VutilTgt_PU_f32 = 0.95F;
    Cal_FWC_Kp_PUperPU_f32 = 0.5F;
    Cal_FWC_Ki_PUperPUs_f32 = 40.0F;
    Cal_FWC_IdLo_Q15_s16 = -13107;
    Cal_FWC_IdDnRate_PUps_f32 = 4.0F;
    Cal_FWC_IdUpRate_PUps_f32 = 1.0F;
    Cal_FWC_SatEps_PU_f32 = 0.01F;
    Cal_FWC_CurAwGain_PU_f32 = 1.0F;
    Cal_FWC_LpfTau_ms_f32 = 5.0F;
    Cal_FWC_Hyst_PU_f32 = 0.02F;
    Cal_FWC_Enter_ms_u16 = 2u;
    Cal_FWC_Exit_ms_u16 = 20u;
}

static void executeOne(const Fwc_Q15_VoltageSnapshot *snapshot,
                       const uint8_t eligible,
                       const Ifx_Math_Fract16 baseIdQ15,
                       const Ifx_Math_Fract16 iqReferenceQ15,
                       Fwc_Q15_Output *output)
{
    Fwc_Q15_execute(snapshot, eligible, baseIdQ15, iqReferenceQ15, output);
}

static void testDisabledAndInvalidPaths(void)
{
    Fwc_Q15_Output output;
    Fwc_Q15_VoltageSnapshot snapshot = makeSnapshot(0.96F, 0.96F, 0u, 0u, 1u);

    restoreCalibration();
    Fwc_Q15_initialize();
    Cal_FWC_Enable_u8 = 0u;
    executeOne(&snapshot, 1u, -1000, 2000, &output);
    assert(output.active == 0u);
    assert(output.idReferenceQ15 == -1000);
    assert(output.iqReferenceQ15 == 2000);
    assert(Meas_FWC_Stat_u8 == FWC_Q15_STATUS_OFF);

    Cal_FWC_Enable_u8 = 1u;
    snapshot.valid = 0u;
    snapshot.dcLinkVoltageQ15 = 0;
    executeOne(&snapshot, 1u, -1000, 2000, &output);
    assert(output.active == 0u);
    assert(Meas_FWC_Stat_u8 == FWC_Q15_STATUS_INVALID_VDC);

    snapshot = makeSnapshot(0.96F, 0.96F, 0u, 0u, 1u);
    executeOne(&snapshot, 0u, -1000, 2000, &output);
    assert(output.active == 0u);
    assert(Meas_FWC_Stat_u8 == FWC_Q15_STATUS_INELIGIBLE);
}

static void testWeakeningAndRecoveryRates(void)
{
    Fwc_Q15_Output output;
    Fwc_Q15_VoltageSnapshot snapshot;
    uint16_t sample;
    int16_t idAfterHighVoltage;

    restoreCalibration();
    Fwc_Q15_initialize();
    snapshot = makeSnapshot(0.94F, 0.94F, 0u, 0u, 1u);
    executeOne(&snapshot, 1u, 0, 0, &output);
    assert(output.active == 1u);
    assert(output.idReferenceQ15 == 0);
    assert(Meas_FWC_Stat_u8 == FWC_Q15_STATUS_ACTIVE);

    snapshot = makeSnapshot(1.00F, 1.00F, 0u, 0u, 1u);
    for (sample = 0u; sample < 40u; ++sample)
    {
        executeOne(&snapshot, 1u, 0, 0, &output);
    }
    assert(output.idReferenceQ15 < 0);
    idAfterHighVoltage = output.idReferenceQ15;

    snapshot = makeSnapshot(0.90F, 0.90F, 0u, 0u, 1u);
    executeOne(&snapshot, 1u, 0, 0, &output);
    /* The 1 PU/s recovery rate permits at most 17 Q15 counts per sample. */
    assert(output.idReferenceQ15 <= (int16_t)(idAfterHighVoltage + 17));
}

static void testHardIdLimitAndVoltageRecoveryGovernor(void)
{
    Fwc_Q15_Output output;
    Fwc_Q15_VoltageSnapshot snapshot;
    uint16_t sample;

    restoreCalibration();
    Fwc_Q15_initialize();
    snapshot = makeSnapshot(1.20F, 1.10F, 1u, 40u, 0u);
    /* A long voltage clamp before the Id floor is not enough: recovery must
     * wait for its own 2 ms interval after Id has actually reached -20 A. */
    snapshot.idAtFloorSaturationStreakFast = 0u;
    for (sample = 0u; sample < 260u; ++sample)
    {
        executeOne(&snapshot, 1u, 0, 0, &output);
    }
    assert(output.idReferenceQ15 == -13107);
    assert(output.recoveryActive == 0u);
    assert(Meas_FWC_Stat_u8 == FWC_Q15_STATUS_VOLT_SAT);

    /* Recovery must start at 2 ms at either supported control period. */
    snapshot.idAtFloorSaturationStreakFast = 2000u / FOC_CONTROL_PERIOD_US - 1u;
    executeOne(&snapshot, 1u, 0, 0, &output);
    assert(output.recoveryActive == 0u);
    snapshot.idAtFloorSaturationStreakFast = 2000u / FOC_CONTROL_PERIOD_US;
    executeOne(&snapshot, 1u, 0, 0, &output);
    assert(output.recoveryActive == 1u);
    assert(Meas_FWC_Stat_u8 == FWC_Q15_STATUS_RECOVERY_GOV);
    assert(Meas_FWC_RecoveryCnt_u32 == 1u);

    snapshot = makeSnapshot(0.94F, 0.94F, 0u, 0u, 40u);
    Cal_FWC_LpfTau_ms_f32 = 0.0F;
    for (sample = 0u; sample < 39u; ++sample)
    {
        executeOne(&snapshot, 1u, 0, 0, &output);
        assert(output.recoveryActive == 1u);
    }
    /* One renewed clamp resets the entire release confirmation. */
    snapshot.saturated = 1u;
    executeOne(&snapshot, 1u, 0, 0, &output);
    snapshot = makeSnapshot(1.00F, 0.994F, 0u, 0u, 40u);
    for (sample = 0u; sample < 45u; ++sample)
    {
        executeOne(&snapshot, 1u, 0, 0, &output);
        assert(output.recoveryActive == 1u);
    }
    snapshot = makeSnapshot(0.94F, 0.94F, 0u, 0u, 40u);
    for (sample = 0u; sample < 40u; ++sample)
    {
        executeOne(&snapshot, 1u, 0, 0, &output);
        assert(output.recoveryActive == ((sample < 39u) ? 1u : 0u));
    }
    assert(output.recoveryActive == 0u);
    assert(Meas_FWC_Stat_u8 == FWC_Q15_STATUS_ACTIVE);
}

static void testIdPriorityCurrentCircle(void)
{
    Ifx_Math_CmpFract16 dqCommand;
    const uint32_t currentLimitSquare = 32767u * 32767u;
    uint32_t magnitudeSquare;

    restoreCalibration();
    dqCommand.real = -13107;
    dqCommand.imag = 32767;
    Fwc_Q15_applyCurrentLimit(&dqCommand, -13107, 1u);
    assert(dqCommand.real == -13107);
    magnitudeSquare = (uint32_t)((int32_t)dqCommand.real * (int32_t)dqCommand.real)
        + (uint32_t)((int32_t)dqCommand.imag * (int32_t)dqCommand.imag);
    assert(magnitudeSquare <= currentLimitSquare);
    assert(dqCommand.imag > 0);

    dqCommand.real = -13107;
    dqCommand.imag = -32768;
    Fwc_Q15_applyCurrentLimit(&dqCommand, -13107, 1u);
    magnitudeSquare = (uint32_t)((int32_t)dqCommand.real * (int32_t)dqCommand.real)
        + (uint32_t)((int32_t)dqCommand.imag * (int32_t)dqCommand.imag);
    assert(magnitudeSquare <= currentLimitSquare);
    assert(dqCommand.imag < 0);
}

static float absoluteValue(float x)
{
    return (x < 0.0F) ? -x : x;
}

static void testFilterAndOnlineCalibration(void)
{
    Fwc_Q15_Output output;
    Fwc_Q15_VoltageSnapshot snapshot;
    float previous;
    float raw;
    union { uint32_t bits; float value; } invalid;
    restoreCalibration();
    Fwc_Q15_reset();
    snapshot = makeSnapshot(0.90F, 0.80F, 0u, 0u, 40u);
    executeOne(&snapshot, 1u, 0, 0, &output);
    previous = Meas_FWC_VreqFlt_PU_f32;
    assert(previous == Meas_FWC_VutilReq_PU_f32);
    raw = Meas_FWC_VutilAct_PU_f32;
    assert(Meas_FWC_VactFlt_PU_f32 == raw);
    snapshot = makeSnapshot(1.10F, 1.00F, 0u, 0u, 40u);
    executeOne(&snapshot, 1u, 0, 0, &output);
    raw = Meas_FWC_VutilReq_PU_f32;
    assert(absoluteValue(Meas_FWC_VreqFlt_PU_f32 - (previous + (raw-previous)/11.0F)) < 1.0e-6F);
    previous = Meas_FWC_VreqFlt_PU_f32;
    Cal_FWC_LpfTau_ms_f32 = 10.0F;
    executeOne(&snapshot, 1u, 0, 0, &output);
    assert(absoluteValue(Meas_FWC_VreqFlt_PU_f32 - (previous + (raw-previous)/21.0F)) < 1.0e-6F);
    Cal_FWC_LpfTau_ms_f32 = 0.0F;
    executeOne(&snapshot, 1u, 0, 0, &output);
    assert(Meas_FWC_VreqFlt_PU_f32 == raw);
    invalid.bits = 0x7FC00000u;
    Cal_FWC_LpfTau_ms_f32 = invalid.value;
    Cal_FWC_Hyst_PU_f32 = invalid.value;
    snapshot = makeSnapshot(0.90F, 0.90F, 0u, 0u, 40u);
    executeOne(&snapshot, 1u, 0, 0, &output);
    previous = Meas_FWC_VutilReq_PU_f32;
    assert(absoluteValue(Meas_FWC_VreqFlt_PU_f32 - (raw + (previous-raw)/11.0F)) < 1.0e-6F);
    Cal_FWC_Enable_u8 = 0u;
    executeOne(&snapshot, 1u, 0, 0, &output);
    assert(Meas_FWC_WeakAct_u8 == 0u && Meas_FWC_IdFw_Q15_s16 == 0);
    assert(Meas_FWC_VreqFlt_PU_f32 == 0.0F);
    Cal_FWC_Enable_u8 = 1u;
    executeOne(&snapshot, 1u, 0, 0, &output);
    raw = Meas_FWC_VutilReq_PU_f32;
    assert(Meas_FWC_VreqFlt_PU_f32 == raw);
    snapshot.valid = 0u;
    executeOne(&snapshot, 1u, 0, 0, &output);
    assert(Meas_FWC_VreqFlt_PU_f32 == 0.0F);
}

static void testHysteresisAndConfirmations(void)
{
    Fwc_Q15_Output output;
    Fwc_Q15_VoltageSnapshot snapshot;
    uint16_t n;
    int16_t heldId;
    restoreCalibration();
    Fwc_Q15_reset();
    Cal_FWC_LpfTau_ms_f32 = 0.0F;
    snapshot = makeSnapshot(0.969F, 0.969F, 0u, 0u, 40u);
    for (n = 0u; n < 100u; ++n)
    {
        executeOne(&snapshot, 1u, 0, 0, &output);
        assert(Meas_FWC_WeakAct_u8 == 0u && output.idReferenceQ15 == 0);
    }
    snapshot = makeSnapshot(0.99F, 0.99F, 0u, 0u, 40u);
    for (n = 0u; n < 3u; ++n)
    {
        executeOne(&snapshot, 1u, 0, 0, &output);
        assert(Meas_FWC_WeakAct_u8 == 0u);
    }
    /* Changing an effective threshold restarts the three accumulated ticks. */
    Cal_FWC_Hyst_PU_f32 = 0.021F;
    executeOne(&snapshot, 1u, 0, 0, &output);
    assert(Meas_FWC_WeakAct_u8 == 0u);
    for (n = 0u; n < 3u; ++n) { executeOne(&snapshot, 1u, 0, 0, &output); }
    assert(Meas_FWC_WeakAct_u8 == 1u && output.idReferenceQ15 < 0);
    snapshot = makeSnapshot(0.95F, 0.95F, 0u, 0u, 40u);
    for (n = 0u; n < 50u; ++n) { executeOne(&snapshot, 1u, 0, 0, &output); }
    heldId = Meas_FWC_IdFw_Q15_s16;
    for (n = 0u; n < 200u; ++n)
    {
        snapshot = makeSnapshot((n & 1u) ? 0.94F : 0.96F, 0.95F, 0u, 0u, 40u);
        executeOne(&snapshot, 1u, 0, 0, &output);
        assert(Meas_FWC_WeakAct_u8 == 1u);
        assert(Meas_FWC_IdFw_Q15_s16 == heldId);
    }
    snapshot = makeSnapshot(0.90F, 0.90F, 0u, 0u, 40u);
    /* Detect release to zero without counting its first eligible exit tick twice. */
    do { executeOne(&snapshot, 1u, -3000, 0, &output); }
    while (Meas_FWC_IdFw_Q15_s16 < -1);
    for (n = 0u; n < 38u; ++n)
    {
        executeOne(&snapshot, 1u, -3000, 0, &output);
        assert(Meas_FWC_WeakAct_u8 == 1u);
    }
    executeOne(&snapshot, 1u, -3000, 0, &output);
    assert(Meas_FWC_WeakAct_u8 == 0u && output.idReferenceQ15 == -3000);
    assert(Meas_FWC_IdFw_Q15_s16 == 0);
    Cal_FWC_Enter_ms_u16 = 0u;
    snapshot = makeSnapshot(1.00F, 1.00F, 0u, 0u, 40u);
    executeOne(&snapshot, 1u, 0, 0, &output);
    assert(Meas_FWC_WeakAct_u8 == 1u);
    executeOne(&snapshot, 0u, 0, 0, &output);
    assert(Meas_FWC_WeakAct_u8 == 0u && Meas_FWC_VreqFlt_PU_f32 == 0.0F);
}

static void testCalibrationBoundsAndImmediateExit(void)
{
    Fwc_Q15_Output output;
    Fwc_Q15_VoltageSnapshot snapshot;
    uint16_t n;
    float initial;
    float raw;
    restoreCalibration();
    Fwc_Q15_reset();
    Cal_FWC_LpfTau_ms_f32 = 500.0F;
    snapshot = makeSnapshot(0.9F, 0.9F, 0u, 0u, 40u);
    executeOne(&snapshot, 1u, 0, 0, &output);
    initial = Meas_FWC_VreqFlt_PU_f32;
    snapshot = makeSnapshot(1.1F, 1.1F, 0u, 0u, 40u);
    executeOne(&snapshot, 1u, 0, 0, &output);
    raw = Meas_FWC_VutilReq_PU_f32;
    assert(absoluteValue(Meas_FWC_VreqFlt_PU_f32 - (initial+(raw-initial)/201.0F)) < 1e-6F);
    Cal_FWC_LpfTau_ms_f32 = -1.0F;
    Cal_FWC_Enter_ms_u16 = 65535u;
    Cal_FWC_Hyst_PU_f32 = 0.5F;
    for (n = 0u; n < 199u; ++n)
    {
        executeOne(&snapshot, 1u, 0, 0, &output);
        assert(Meas_FWC_WeakAct_u8 == 0u);
    }
    executeOne(&snapshot, 1u, 0, 0, &output);
    assert(Meas_FWC_WeakAct_u8 == 1u);
    assert(Meas_FWC_VreqFlt_PU_f32 == raw);
    Cal_FWC_Exit_ms_u16 = 0u;
    snapshot = makeSnapshot(0.8F, 0.8F, 0u, 0u, 40u);
    for (n = 0u; n < 20u; ++n)
    {
        executeOne(&snapshot, 1u, 0, 0, &output);
        if (Meas_FWC_IdFw_Q15_s16 >= -1) { break; }
    }
    assert(Meas_FWC_IdFw_Q15_s16 == 0 && Meas_FWC_WeakAct_u8 == 0u);
    Cal_FWC_Enter_ms_u16 = 0u;
    Cal_FWC_Hyst_PU_f32 = -0.1F;
    snapshot = makeSnapshot(0.96F, 0.96F, 0u, 0u, 40u);
    executeOne(&snapshot, 1u, 0, 0, &output);
    assert(Meas_FWC_WeakAct_u8 == 1u);
}

int main(void)
{
    testDisabledAndInvalidPaths();
    testWeakeningAndRecoveryRates();
    testHardIdLimitAndVoltageRecoveryGovernor();
    testIdPriorityCurrentCircle();
    testFilterAndOnlineCalibration();
    testHysteresisAndConfirmations();
    testCalibrationBoundsAndImmediateExit();
    puts("FWC adapter tests passed");
    return 0;
}
