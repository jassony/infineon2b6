#include <assert.h>
#include <math.h>
#include <stdio.h>

/* Include the adapter in this white-box host test so the parameter-valid
 * latch and dynamic state can be checked without adding a production test
 * API or XCP measurement. */
#include "../apsfsm_torque_compensation_adapter.c"

#define TEST_FLOAT_TOLERANCE (1.0e-7F)

static void assertFloatNear(const float actual, const float expected)
{
    assert(fabsf(actual - expected) <= TEST_FLOAT_TOLERANCE);
}

static Ifx_Math_Fract16 speedRpmToQ15(const uint16_t speedRpm)
{
    uint32_t scaled = ((uint32_t)speedRpm * 32768u)
        + ((uint32_t)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM / 2u);

    scaled /= (uint32_t)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM;
    return (Ifx_Math_Fract16)scaled;
}

static APSFSM_TorqueCompCalibration nominalCalibration(void)
{
    APSFSM_TorqueCompCalibration calibration;

    calibration.selector = APSFSM_TORQUE_COMP_MODE_SHADOW;
    calibration.reset = 0u;
    calibration.kHat = 0.25F;
    calibration.rho_rad = -1.570796326794896619F;
    calibration.lambda = 0.98F;
    calibration.iqUpperLimitQ15 = 1638;
    calibration.iqLowerLimitQ15 = -1638;
    calibration.speedLowerLimit_rpm = 800u;
    calibration.speedUpperLimit_rpm = 4000u;
    calibration.settleTime_ms = 1u;
    calibration.rampTime_ms = 1u;
    return calibration;
}

static void testParameterInvalidResetIsEdgeTriggered(void)
{
    APSFSM_TorqueCompCalibration calibration = nominalCalibration();
    APSFSM_TorqueCompDiagnostics diagnostics;
    Ifx_Math_CmpFract16 baseCommand = {123, 456};
    Ifx_Math_CmpFract16 compensatedCommand = {0, 0};
    const Ifx_Math_Fract16 speedQ15 = speedRpmToQ15(1000u);

    APSFSM_TorqueComp_initialize();
    assert(apsfsmCalibrationState == APSFSM_CALIBRATION_UNKNOWN);
    apsfsmTorqueCompState.rampTicks = 77u;

    /* UNKNOWN means a public reset already cleared dynamic state. The first
     * valid sample must not perform a redundant second reset. It returns in
     * settle before Shadow mode would otherwise clear this ramp sentinel. */
    assert(APSFSM_TorqueComp_execute(speedQ15, speedQ15, &baseCommand, 1u,
        &calibration, &compensatedCommand, &diagnostics) == 0u);
    assert(apsfsmCalibrationState == APSFSM_CALIBRATION_VALID);
    assert(apsfsmTorqueCompState.initialized == 1u);
    assert(apsfsmTorqueCompState.settleTicks == 1u);
    assert(apsfsmTorqueCompState.rampTicks == 77u);
    assert(diagnostics.status == APSFSM_TORQUE_COMP_STATUS_WAIT_SPEED);
    assert(APSFSM_TorqueComp_execute(speedQ15, speedQ15, &baseCommand, 1u,
        &calibration, &compensatedCommand, &diagnostics) == 1u);
    assert(diagnostics.status == APSFSM_TORQUE_COMP_STATUS_SHADOW_VALID);

    /* The valid-to-invalid edge clears the accumulated state once. */
    calibration.lambda = 0.0F;
    assert(APSFSM_TorqueComp_execute(speedQ15, speedQ15, &baseCommand, 1u,
        &calibration, &compensatedCommand, &diagnostics) == 0u);
    assert(apsfsmCalibrationState == APSFSM_CALIBRATION_INVALID);
    assert(apsfsmTorqueCompState.initialized == 0u);
    assert(apsfsmTorqueCompState.settleTicks == 0u);
    assertFloatNear(apsfsmTorqueCompState.bHatPU, 0.0F);
    assert(diagnostics.status == APSFSM_TORQUE_COMP_STATUS_PARAMETER_INVALID);
    assert(compensatedCommand.real == baseCommand.real);
    assert(compensatedCommand.imag == baseCommand.imag);

    /* A persistent invalid snapshot must only republish diagnostics. Sentinels
     * would be erased here if resetState() were still called every sample. */
    apsfsmTorqueCompState.initialized = 1u;
    apsfsmTorqueCompState.settleTicks = 77u;
    apsfsmTorqueCompState.bHatPU = 0.125F;
    calibration.kHat = 0.0F;
    assert(APSFSM_TorqueComp_execute(speedQ15, speedQ15, &baseCommand, 1u,
        &calibration, &compensatedCommand, &diagnostics) == 0u);
    assert(apsfsmTorqueCompState.initialized == 1u);
    assert(apsfsmTorqueCompState.settleTicks == 77u);
    assertFloatNear(apsfsmTorqueCompState.bHatPU, 0.125F);
    assert(diagnostics.status == APSFSM_TORQUE_COMP_STATUS_PARAMETER_INVALID);
    assert(diagnostics.active == 0u);
    assert(diagnostics.outputActive == 0u);
    assert(diagnostics.valid == 0u);
    assert(Meas_APSFSM_Stat_u8 == APSFSM_TORQUE_COMP_STATUS_PARAMETER_INVALID);
    assert(compensatedCommand.real == baseCommand.real);
    assert(compensatedCommand.imag == baseCommand.imag);

    /* Invalid-to-valid recovery clears the stale state once and restarts the
     * configured settle interval from its first tick. */
    calibration = nominalCalibration();
    assert(APSFSM_TorqueComp_execute(speedQ15, speedQ15, &baseCommand, 1u,
        &calibration, &compensatedCommand, &diagnostics) == 0u);
    assert(apsfsmCalibrationState == APSFSM_CALIBRATION_VALID);
    assert(apsfsmTorqueCompState.initialized == 1u);
    assert(apsfsmTorqueCompState.settleTicks == 1u);
    assertFloatNear(apsfsmTorqueCompState.bHatPU, 0.0F);
    assertFloatNear(apsfsmTorqueCompState.covariance, 0.03125F);
    assert(diagnostics.status == APSFSM_TORQUE_COMP_STATUS_WAIT_SPEED);
    assert(APSFSM_TorqueComp_execute(speedQ15, speedQ15, &baseCommand, 1u,
        &calibration, &compensatedCommand, &diagnostics) == 1u);
    assert(diagnostics.status == APSFSM_TORQUE_COMP_STATUS_SHADOW_VALID);

    APSFSM_TorqueComp_reset();
    assert(apsfsmCalibrationState == APSFSM_CALIBRATION_UNKNOWN);
    assert(apsfsmTorqueCompState.initialized == 0u);
}

static void testNullCalibrationUsesTheSameEdges(void)
{
    APSFSM_TorqueCompCalibration calibration = nominalCalibration();
    APSFSM_TorqueCompDiagnostics diagnostics;
    Ifx_Math_CmpFract16 baseCommand = {-123, -456};
    Ifx_Math_CmpFract16 compensatedCommand = {0, 0};
    const Ifx_Math_Fract16 speedQ15 = speedRpmToQ15(1000u);

    APSFSM_TorqueComp_initialize();
    assert(APSFSM_TorqueComp_execute(speedQ15, speedQ15, &baseCommand, 1u,
        &calibration, &compensatedCommand, &diagnostics) == 0u);
    assert(apsfsmCalibrationState == APSFSM_CALIBRATION_VALID);

    assert(APSFSM_TorqueComp_execute(speedQ15, speedQ15, &baseCommand, 1u,
        NULL, &compensatedCommand, &diagnostics) == 0u);
    assert(apsfsmCalibrationState == APSFSM_CALIBRATION_INVALID);
    assert(apsfsmTorqueCompState.initialized == 0u);

    apsfsmTorqueCompState.initialized = 1u;
    apsfsmTorqueCompState.settleTicks = 55u;
    assert(APSFSM_TorqueComp_execute(speedQ15, speedQ15, &baseCommand, 1u,
        NULL, &compensatedCommand, &diagnostics) == 0u);
    assert(apsfsmTorqueCompState.initialized == 1u);
    assert(apsfsmTorqueCompState.settleTicks == 55u);
    assert(diagnostics.status == APSFSM_TORQUE_COMP_STATUS_PARAMETER_INVALID);
    assert(compensatedCommand.real == baseCommand.real);
    assert(compensatedCommand.imag == baseCommand.imag);

    assert(APSFSM_TorqueComp_execute(speedQ15, speedQ15, &baseCommand, 1u,
        &calibration, &compensatedCommand, &diagnostics) == 0u);
    assert(apsfsmCalibrationState == APSFSM_CALIBRATION_VALID);
    assert(apsfsmTorqueCompState.initialized == 1u);
    assert(apsfsmTorqueCompState.settleTicks == 1u);
}

int main(void)
{
    testParameterInvalidResetIsEdgeTriggered();
    testNullCalibrationUsesTheSameEdges();
    puts("APSFSM adapter tests passed");
    return 0;
}
