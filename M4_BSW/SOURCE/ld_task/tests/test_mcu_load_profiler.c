#include "mcu_load_profiler.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#define TEST_FLOAT_TOLERANCE (0.001F)

static void assertFloatNear(float actual, float expected)
{
    assert(fabsf(actual - expected) <= TEST_FLOAT_TOLERANCE);
}

static void testNoPreemption(void)
{
    McuLoadProfilerToken token;

    McuLoadProfiler_testInitialize(200u, 0u, 1u);
    McuLoadProfiler_testSetCycles(10u);
    token = McuLoadProfiler_begin();
    McuLoadProfiler_testSetCycles(30u);
    assert(McuLoadProfiler_end(MCU_LOAD_CONTEXT_RTE_TASK, token) == 20u);

    McuLoadProfiler_testSetCycles(100u);
    McuLoadProfiler_service();
    assert(Meas_MCU_UpdateSeq_u32 == 1u);
    assert(Meas_MCU_WindowCycles_u32 == 100u);
    assertFloatNear(Meas_MCU_RteTaskLoad_pct_f32, 20.0F);
    assertFloatNear(Meas_MCU_TotalLoad_pct_f32, 20.0F);
    assertFloatNear(Meas_MCU_Reserve_pct_f32, 80.0F);
    assert(Meas_MCU_RteTaskLastCycles_u32 == 20u);
    assert(Meas_MCU_RteTaskMaxCycles_u32 == 20u);
}

static void testNestedInterruptsAreExclusive(void)
{
    McuLoadProfilerToken rteToken;
    McuLoadProfilerToken speedToken;
    McuLoadProfilerToken fastToken;

    McuLoadProfiler_testInitialize(200u, 0u, 1u);
    rteToken = McuLoadProfiler_begin();
    McuLoadProfiler_testSetCycles(10u);
    speedToken = McuLoadProfiler_begin();
    McuLoadProfiler_testSetCycles(20u);
    fastToken = McuLoadProfiler_begin();
    McuLoadProfiler_testSetCycles(30u);
    assert(McuLoadProfiler_end(MCU_LOAD_CONTEXT_FOC_FAST, fastToken) == 10u);
    McuLoadProfiler_testSetCycles(50u);
    assert(McuLoadProfiler_end(MCU_LOAD_CONTEXT_FOC_SPD, speedToken) == 40u);
    McuLoadProfiler_testSetCycles(100u);
    assert(McuLoadProfiler_end(MCU_LOAD_CONTEXT_RTE_TASK, rteToken) == 100u);

    McuLoadProfiler_service();
    assertFloatNear(Meas_MCU_FocFastLoad_pct_f32, 10.0F);
    assertFloatNear(Meas_MCU_FocSpdLoad_pct_f32, 30.0F);
    assertFloatNear(Meas_MCU_RteTaskLoad_pct_f32, 60.0F);
    assertFloatNear(Meas_MCU_TotalLoad_pct_f32, 100.0F);
    assertFloatNear(Meas_MCU_Reserve_pct_f32, 0.0F);
    assert(Meas_MCU_FocSpdLastCycles_u32 == 40u);
    assert(Meas_MCU_RteTaskLastCycles_u32 == 100u);
    assert(Meas_MCU_Status_u8 == 0u);
}

static void testCounterWrap(void)
{
    McuLoadProfilerToken token;

    McuLoadProfiler_testInitialize(64u, 0xFFFFFFF0u, 1u);
    token = McuLoadProfiler_begin();
    McuLoadProfiler_testSetCycles(0x00000010u);
    assert(McuLoadProfiler_end(MCU_LOAD_CONTEXT_RTE_TASK, token) == 32u);
    McuLoadProfiler_testSetCycles(0x00000020u);
    McuLoadProfiler_service();

    assert(Meas_MCU_WindowCycles_u32 == 48u);
    assert(Meas_MCU_RteTaskLastCycles_u32 == 32u);
    assertFloatNear(Meas_MCU_RteTaskLoad_pct_f32, 66.666664F);
}

static void testWindowAndLifetimeMaximum(void)
{
    McuLoadProfilerToken token;

    McuLoadProfiler_testInitialize(200u, 0u, 1u);
    token = McuLoadProfiler_begin();
    McuLoadProfiler_testSetCycles(20u);
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_TEST_500MS_TASK, token);
    McuLoadProfiler_testSetCycles(99u);
    McuLoadProfiler_service();
    assert(Meas_MCU_UpdateSeq_u32 == 0u);
    McuLoadProfiler_testSetCycles(100u);
    McuLoadProfiler_service();
    assert(Meas_MCU_UpdateSeq_u32 == 1u);
    assert(Meas_MCU_Test500msTaskMaxCycles_u32 == 20u);

    McuLoadProfiler_testSetCycles(110u);
    token = McuLoadProfiler_begin();
    McuLoadProfiler_testSetCycles(120u);
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_TEST_500MS_TASK, token);
    McuLoadProfiler_testSetCycles(199u);
    McuLoadProfiler_service();
    assert(Meas_MCU_UpdateSeq_u32 == 1u);
    McuLoadProfiler_testSetCycles(200u);
    McuLoadProfiler_service();
    assert(Meas_MCU_UpdateSeq_u32 == 2u);
    assert(Meas_MCU_Test500msTaskLastCycles_u32 == 10u);
    assert(Meas_MCU_Test500msTaskMaxCycles_u32 == 20u);
}

static void testInvalidPreemptionIsStickyAndClamped(void)
{
    McuLoadProfilerToken token;

    McuLoadProfiler_testInitialize(200u, 0u, 1u);
    token = McuLoadProfiler_begin();
    McuLoadProfiler_testSetInterruptCycles(50u);
    McuLoadProfiler_testSetCycles(10u);
    assert(McuLoadProfiler_end(MCU_LOAD_CONTEXT_RTE_TASK, token) == 10u);
    McuLoadProfiler_testSetCycles(100u);
    McuLoadProfiler_service();

    assert((Meas_MCU_Status_u8
        & MCU_LOAD_PROFILER_STATUS_PREEMPT_CYCLES_INVALID) != 0u);
    assertFloatNear(Meas_MCU_RteTaskLoad_pct_f32, 0.0F);
    assertFloatNear(Meas_MCU_TotalLoad_pct_f32, 0.0F);
    assertFloatNear(Meas_MCU_Reserve_pct_f32, 100.0F);
}

static void testTotalLoadIsClamped(void)
{
    McuLoadProfilerToken rteToken;
    McuLoadProfilerToken testToken;

    McuLoadProfiler_testInitialize(200u, 0u, 1u);
    rteToken = McuLoadProfiler_begin();
    testToken = McuLoadProfiler_begin();
    McuLoadProfiler_testSetCycles(80u);
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_RTE_TASK, rteToken);
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_TEST_500MS_TASK, testToken);
    McuLoadProfiler_testSetCycles(100u);
    McuLoadProfiler_service();

    assert((Meas_MCU_Status_u8
        & MCU_LOAD_PROFILER_STATUS_TOTAL_CYCLES_INVALID) != 0u);
    assertFloatNear(Meas_MCU_TotalLoad_pct_f32, 100.0F);
    assertFloatNear(Meas_MCU_Reserve_pct_f32, 0.0F);
}

static void testUnavailableCounterIsSafe(void)
{
    McuLoadProfiler_testInitialize(200u, 0u, 0u);
    McuLoadProfiler_testSetCycles(100u);
    McuLoadProfiler_service();

    assert(Meas_MCU_UpdateSeq_u32 == 0u);
    assert((Meas_MCU_Status_u8
        & MCU_LOAD_PROFILER_STATUS_DWT_UNAVAILABLE) != 0u);
    assertFloatNear(Meas_MCU_TotalLoad_pct_f32, 0.0F);
    assertFloatNear(Meas_MCU_Reserve_pct_f32, 100.0F);
}

#if MCU_FAST_PROFILE_ENABLE
static float fastShareSum(void)
{
    float sum = Meas_MCU_FastAdc_pct_f32;
    sum += Meas_MCU_FastObs_pct_f32;
    sum += Meas_MCU_FastCtrl_pct_f32;
    sum += Meas_MCU_FastVPre_pct_f32;
    sum += Meas_MCU_FastMod_pct_f32;
    sum += Meas_MCU_FastVPost_pct_f32;
    sum += Meas_MCU_FastPwm_pct_f32;
    sum += Meas_MCU_FastOther_pct_f32;
    return sum;
}

static void testFastPartsAndPreemption(void)
{
    McuLoadProfilerToken fast, irq;
    uint32_t part;
    McuLoadProfiler_testInitialize(200u, 0u, 1u);
    fast = McuLoadProfiler_begin();
    McuFastProfile_start(fast);
    McuLoadProfiler_testSetCycles(2u);
    MCU_FAST_MARK(MCU_FAST_OTHER);
    McuLoadProfiler_testSetCycles(12u);
    MCU_FAST_MARK(MCU_FAST_ADC);
    irq = McuLoadProfiler_begin();
    McuLoadProfiler_testSetCycles(22u);
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_GPIO_IRQ, irq);
    for (part = MCU_FAST_OBS; part <= MCU_FAST_PWM; part++)
    {
        McuLoadProfiler_testSetCycles(22u + part * 10u);
        MCU_FAST_MARK((McuFastProfilePart)part);
    }
    McuLoadProfiler_testSetCycles(90u);
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_FOC_FAST, fast);
    McuLoadProfiler_testSetCycles(99u);
    McuLoadProfiler_service();
    assert(Meas_MCU_FastSamples_u32 == 0u);
    McuLoadProfiler_testSetCycles(100u);
    McuLoadProfiler_service();
    assert(Meas_MCU_FastSamples_u32 == 1u);
    assertFloatNear(Meas_MCU_FastAdc_pct_f32, 12.5F);
    assertFloatNear(Meas_MCU_FastObs_pct_f32, 12.5F);
    assertFloatNear(Meas_MCU_FastPwm_pct_f32, 12.5F);
    assertFloatNear(Meas_MCU_FastOther_pct_f32, 12.5F);
    assertFloatNear(fastShareSum(), 100.0F);
    /* Parts must never inflate the enclosing fast / system load. */
    assertFloatNear(Meas_MCU_FocFastLoad_pct_f32, 80.0F);
    assertFloatNear(Meas_MCU_TotalLoad_pct_f32, 90.0F);
    assert(Meas_MCU_Status_u8 == 0u);
    McuLoadProfiler_testSetCycles(200u);
    McuLoadProfiler_service();
    assert(Meas_MCU_FastSamples_u32 == 0u);
    assertFloatNear(fastShareSum(), 0.0F);
}

static void testFastDecimationAndStoppedPath(void)
{
    uint32_t call;
    McuLoadProfiler_testInitialize(10000u, 0u, 1u);
    for (call = 0u; call < 2u * MCU_FAST_PROFILE_DIVIDER; call++)
    {
        McuLoadProfilerToken fast;
        McuLoadProfiler_testSetCycles(call * 10u);
        fast = McuLoadProfiler_begin();
        McuFastProfile_start(fast);
        assert(McuFastProfile_active ==
            ((call % MCU_FAST_PROFILE_DIVIDER) == 0u));
        McuLoadProfiler_testSetCycles(call * 10u + 4u);
        MCU_FAST_MARK(MCU_FAST_ADC);
        /* Stopped observer/control have zero injected work. */
        MCU_FAST_MARK(MCU_FAST_OBS);
        MCU_FAST_MARK(MCU_FAST_CTRL);
        McuLoadProfiler_testSetCycles(call * 10u + 8u);
        MCU_FAST_MARK(MCU_FAST_PWM);
        McuLoadProfiler_testSetCycles(call * 10u + 10u);
        (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_FOC_FAST, fast);
    }
    McuLoadProfiler_testSetCycles(5000u);
    McuLoadProfiler_service();
    assert(Meas_MCU_FastSamples_u32 == 2u);
    assertFloatNear(Meas_MCU_FastAdc_pct_f32, 40.0F);
    assertFloatNear(Meas_MCU_FastObs_pct_f32, 0.0F);
    assertFloatNear(Meas_MCU_FastCtrl_pct_f32, 0.0F);
    assertFloatNear(Meas_MCU_FastPwm_pct_f32, 40.0F);
    assertFloatNear(Meas_MCU_FastOther_pct_f32, 20.0F);
    assertFloatNear(fastShareSum(), 100.0F);
}

static void testFastWrapAndInvalidParts(void)
{
    McuLoadProfilerToken fast;
    McuLoadProfiler_testInitialize(64u, 0xFFFFFFF0u, 1u);
    fast = McuLoadProfiler_begin();
    McuFastProfile_start(fast);
    McuLoadProfiler_testSetCycles(0u);
    MCU_FAST_MARK(MCU_FAST_ADC);
    McuLoadProfiler_testSetCycles(16u);
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_FOC_FAST, fast);
    McuLoadProfiler_testSetCycles(32u);
    McuLoadProfiler_service();
    assertFloatNear(Meas_MCU_FastAdc_pct_f32, 50.0F);
    assertFloatNear(Meas_MCU_FastOther_pct_f32, 50.0F);
    assert(Meas_MCU_Status_u8 == 0u);

    McuLoadProfiler_testInitialize(200u, 0u, 1u);
    fast = McuLoadProfiler_begin();
    McuFastProfile_start(fast);
    McuLoadProfiler_testSetCycles(20u);
    MCU_FAST_MARK(MCU_FAST_ADC);
    /* Simulate corrupt/inconsistent total; publishing must not underflow. */
    McuLoadProfiler_testSetCycles(10u);
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_FOC_FAST, fast);
    McuLoadProfiler_testSetCycles(100u);
    McuLoadProfiler_service();
    assertFloatNear(Meas_MCU_FastAdc_pct_f32, 100.0F);
    assertFloatNear(Meas_MCU_FastOther_pct_f32, 0.0F);
    assert((Meas_MCU_Status_u8 & MCU_LOAD_PROFILER_STATUS_TOTAL_CYCLES_INVALID) != 0u);

    McuLoadProfiler_testInitialize(200u, 0u, 0u);
    fast = McuLoadProfiler_begin();
    McuFastProfile_start(fast);
    assert(McuFastProfile_active == 0u);
    assertFloatNear(fastShareSum(), 0.0F);
}
#endif

int main(void)
{
    testNoPreemption();
    testNestedInterruptsAreExclusive();
    testCounterWrap();
    testWindowAndLifetimeMaximum();
    testInvalidPreemptionIsStickyAndClamped();
    testTotalLoadIsClamped();
    testUnavailableCounterIsSafe();
#if MCU_FAST_PROFILE_ENABLE
    testFastPartsAndPreemption();
    testFastDecimationAndStoppedPath();
    testFastWrapAndInvalidParts();
#endif
    puts("McuLoadProfiler tests passed");
    return 0;
}
