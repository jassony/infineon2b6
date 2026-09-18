#define MCU_LOAD_PROFILER_IMPLEMENTATION (1u)
#include "mcu_load_profiler.h"

#if (MCU_LOAD_PROFILER_ENABLE != 0u)

#if defined(MCU_LOAD_PROFILER_HOST_TEST)
#define NO_OPT
static volatile uint32_t s_testCycleCounter;
#define MCU_LOAD_PROFILER_READ_CYCLES() (s_testCycleCounter)
#define MCU_LOAD_PROFILER_ENTER_CRITICAL(mask) do { (mask) = 0u; } while (0)
#define MCU_LOAD_PROFILER_EXIT_CRITICAL(mask)  do { (void)(mask); } while (0)
#else
#include "cy_project.h"
#include "cy_device_headers.h"
#include "no_opt.h"
#define MCU_LOAD_PROFILER_READ_CYCLES() (DWT->CYCCNT)
#define MCU_LOAD_PROFILER_ENTER_CRITICAL(mask) \
    do { (mask) = __get_PRIMASK(); __disable_irq(); } while (0)
#define MCU_LOAD_PROFILER_EXIT_CRITICAL(mask) \
    do { if ((mask) == 0u) { __enable_irq(); } } while (0)
#endif

#define MCU_LOAD_PROFILER_PERCENT_SCALE (100.0F)
#define MCU_LOAD_PROFILER_UINT32_MAX     (0xFFFFFFFFu)

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_FocFastLoad_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_FocFastLastCycles_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_FocFastMaxCycles_u32 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_FocSpdLoad_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_FocSpdLastCycles_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_FocSpdMaxCycles_u32 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_SysTickLoad_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_SysTickLastCycles_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_SysTickMaxCycles_u32 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_GpioIrqLoad_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_GpioIrqLastCycles_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_GpioIrqMaxCycles_u32 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_IpcIrqLoad_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_IpcIrqLastCycles_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_IpcIrqMaxCycles_u32 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_RteTaskLoad_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_RteTaskLastCycles_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_RteTaskMaxCycles_u32 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_Test500msTaskLoad_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_Test500msTaskLastCycles_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_Test500msTaskMaxCycles_u32 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_TotalLoad_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_Reserve_pct_f32 = 100.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_WindowCycles_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_UpdateSeq_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_MCU_Status_u8 = 0u;

#if MCU_FAST_PROFILE_ENABLE
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_FastAdc_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_FastObs_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_FastCtrl_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_FastVPre_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_FastMod_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_FastVPost_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_FastPwm_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_MCU_FastOther_pct_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_MCU_FastSamples_u32 = 0u;

uint32_t McuFastProfile_active;
static McuLoadProfilerToken s_fastCursor;
static uint32_t s_fastDivider;
static uint32_t s_fastParts[MCU_FAST_OTHER];
static uint32_t s_fastTotal;
static uint32_t s_fastSamples;

void McuFastProfile_start(McuLoadProfilerToken token)
{
    McuFastProfile_active = ((s_fastDivider++ & (MCU_FAST_PROFILE_DIVIDER - 1u)) == 0u)
        && (McuLoadProfiler_available != 0u);
    if (McuFastProfile_active != 0u)
    {
        s_fastCursor = token;
    }
}

void McuFastProfile_markInternal(McuFastProfilePart part)
{
    const McuLoadProfilerToken now = McuLoadProfiler_begin();
    const uint32_t raw = now.startCycles - s_fastCursor.startCycles;
    const uint32_t preempt = now.interruptCycles - s_fastCursor.interruptCycles;
    s_fastCursor = now;
    if (preempt > raw)
    {
        McuLoadProfiler_setStatusInternal(MCU_LOAD_PROFILER_STATUS_PREEMPT_CYCLES_INVALID);
    }
    else if ((uint32_t)part < (uint32_t)MCU_FAST_OTHER)
    {
        s_fastParts[part] += raw - preempt;
    }
}

void McuFastProfile_finish(uint32_t exclusiveCycles)
{
    /* Reuse the enclosing fast ISR end timestamp. Never add parts to the
     * interrupt accumulator: they are already included in FOC_FAST. */
    s_fastTotal += exclusiveCycles;
    s_fastSamples++;
    McuFastProfile_active = 0u;
}

static void McuFastProfile_reset(void)
{
    uint32_t index;
    McuFastProfile_active = 0u;
    s_fastCursor.startCycles = 0u;
    s_fastCursor.interruptCycles = 0u;
    s_fastDivider = 0u;
    s_fastTotal = 0u;
    s_fastSamples = 0u;
    for (index = 0u; index < (uint32_t)MCU_FAST_OTHER; index++)
    {
        s_fastParts[index] = 0u;
    }
    Meas_MCU_FastAdc_pct_f32 = 0.0F;
    Meas_MCU_FastObs_pct_f32 = 0.0F;
    Meas_MCU_FastCtrl_pct_f32 = 0.0F;
    Meas_MCU_FastVPre_pct_f32 = 0.0F;
    Meas_MCU_FastMod_pct_f32 = 0.0F;
    Meas_MCU_FastVPost_pct_f32 = 0.0F;
    Meas_MCU_FastPwm_pct_f32 = 0.0F;
    Meas_MCU_FastOther_pct_f32 = 0.0F;
    Meas_MCU_FastSamples_u32 = 0u;
}

static void McuFastProfile_publish(uint32_t *parts, uint32_t total, uint32_t samples)
{
    uint32_t index;
    uint32_t remaining = total;
    float percent[MCU_FAST_PART_COUNT];
    for (index = 0u; index < (uint32_t)MCU_FAST_OTHER; index++)
    {
        if (parts[index] > remaining)
        {
            parts[index] = remaining;
            McuLoadProfiler_setStatusInternal(MCU_LOAD_PROFILER_STATUS_TOTAL_CYCLES_INVALID);
        }
        remaining -= parts[index];
        percent[index] = (total == 0u) ? 0.0F : (float)parts[index] * 100.0F / (float)total;
    }
    percent[MCU_FAST_OTHER] = (total == 0u) ? 0.0F : (float)remaining * 100.0F / (float)total;
    Meas_MCU_FastAdc_pct_f32 = percent[0];
    Meas_MCU_FastObs_pct_f32 = percent[1];
    Meas_MCU_FastCtrl_pct_f32 = percent[2];
    Meas_MCU_FastVPre_pct_f32 = percent[3];
    Meas_MCU_FastMod_pct_f32 = percent[4];
    Meas_MCU_FastVPost_pct_f32 = percent[5];
    Meas_MCU_FastPwm_pct_f32 = percent[6];
    Meas_MCU_FastOther_pct_f32 = percent[7];
    Meas_MCU_FastSamples_u32 = samples;
}
#endif

volatile uint32_t McuLoadProfiler_interruptExclusiveCycles;
uint32_t McuLoadProfiler_windowExclusiveCycles[MCU_LOAD_CONTEXT_COUNT];
uint32_t McuLoadProfiler_lastRawCycles[MCU_LOAD_CONTEXT_COUNT];
uint32_t McuLoadProfiler_maxRawCycles[MCU_LOAD_CONTEXT_COUNT];
static uint32_t s_windowStartCycles;
static uint32_t s_windowPeriodCycles;
volatile uint8_t McuLoadProfiler_available;

void McuLoadProfiler_setStatusInternal(uint8_t status)
{
    uint32_t interruptMask;

    MCU_LOAD_PROFILER_ENTER_CRITICAL(interruptMask);
    Meas_MCU_Status_u8 |= status;
    MCU_LOAD_PROFILER_EXIT_CRITICAL(interruptMask);
}

static void McuLoadProfiler_clearPublishedValues(void)
{
    Meas_MCU_FocFastLoad_pct_f32 = 0.0F;
    Meas_MCU_FocFastLastCycles_u32 = 0u;
    Meas_MCU_FocFastMaxCycles_u32 = 0u;
    Meas_MCU_FocSpdLoad_pct_f32 = 0.0F;
    Meas_MCU_FocSpdLastCycles_u32 = 0u;
    Meas_MCU_FocSpdMaxCycles_u32 = 0u;
    Meas_MCU_SysTickLoad_pct_f32 = 0.0F;
    Meas_MCU_SysTickLastCycles_u32 = 0u;
    Meas_MCU_SysTickMaxCycles_u32 = 0u;
    Meas_MCU_GpioIrqLoad_pct_f32 = 0.0F;
    Meas_MCU_GpioIrqLastCycles_u32 = 0u;
    Meas_MCU_GpioIrqMaxCycles_u32 = 0u;
    Meas_MCU_IpcIrqLoad_pct_f32 = 0.0F;
    Meas_MCU_IpcIrqLastCycles_u32 = 0u;
    Meas_MCU_IpcIrqMaxCycles_u32 = 0u;
    Meas_MCU_RteTaskLoad_pct_f32 = 0.0F;
    Meas_MCU_RteTaskLastCycles_u32 = 0u;
    Meas_MCU_RteTaskMaxCycles_u32 = 0u;
    Meas_MCU_Test500msTaskLoad_pct_f32 = 0.0F;
    Meas_MCU_Test500msTaskLastCycles_u32 = 0u;
    Meas_MCU_Test500msTaskMaxCycles_u32 = 0u;
    Meas_MCU_TotalLoad_pct_f32 = 0.0F;
    Meas_MCU_Reserve_pct_f32 = 100.0F;
    Meas_MCU_WindowCycles_u32 = 0u;
    Meas_MCU_UpdateSeq_u32 = 0u;
    Meas_MCU_Status_u8 = 0u;
}

static void McuLoadProfiler_resetState(uint32_t coreClockHz,
                                       uint32_t startCycles,
                                       uint8_t available)
{
    uint32_t index;

    McuLoadProfiler_available = 0u;
    McuLoadProfiler_interruptExclusiveCycles = 0u;
    s_windowStartCycles = startCycles;
    s_windowPeriodCycles = coreClockHz / 2u;
    if (s_windowPeriodCycles == 0u)
    {
        s_windowPeriodCycles = 1u;
    }

    for (index = 0u; index < (uint32_t)MCU_LOAD_CONTEXT_COUNT; index++)
    {
        McuLoadProfiler_windowExclusiveCycles[index] = 0u;
        McuLoadProfiler_lastRawCycles[index] = 0u;
        McuLoadProfiler_maxRawCycles[index] = 0u;
    }

    McuLoadProfiler_clearPublishedValues();
#if MCU_FAST_PROFILE_ENABLE
    McuFastProfile_reset();
#endif
    if (available == 0u)
    {
        Meas_MCU_Status_u8 = MCU_LOAD_PROFILER_STATUS_DWT_UNAVAILABLE;
    }
    else
    {
        McuLoadProfiler_available = 1u;
    }
}

void McuLoadProfiler_initialize(void)
{
#if defined(MCU_LOAD_PROFILER_HOST_TEST)
    McuLoadProfiler_testInitialize(200u, s_testCycleCounter, 1u);
#else
    uint32_t probeStart;
    uint32_t interruptMask;
    uint8_t available = 1u;

    MCU_LOAD_PROFILER_ENTER_CRITICAL(interruptMask);
    McuLoadProfiler_available = 0u;
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    if ((DWT->CTRL & DWT_CTRL_NOCYCCNT_Msk) != 0u)
    {
        available = 0u;
    }
    else
    {
        DWT->CYCCNT = 0u;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        probeStart = DWT->CYCCNT;
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        if (((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0u)
            || (DWT->CYCCNT == probeStart))
        {
            available = 0u;
        }
    }

    McuLoadProfiler_resetState(SystemCoreClock, DWT->CYCCNT, available);
    MCU_LOAD_PROFILER_EXIT_CRITICAL(interruptMask);
#endif
}

McuLoadProfilerToken McuLoadProfiler_begin(void)
{
    McuLoadProfilerToken token;
    uint32_t interruptCyclesBefore;
    uint32_t interruptCyclesAfter;

    token.startCycles = 0u;
    token.interruptCycles = 0u;

    if (McuLoadProfiler_available == 0u)
    {
        return token;
    }

    do
    {
        interruptCyclesBefore = McuLoadProfiler_interruptExclusiveCycles;
        token.startCycles = MCU_LOAD_PROFILER_READ_CYCLES();
        interruptCyclesAfter = McuLoadProfiler_interruptExclusiveCycles;
    } while (interruptCyclesBefore != interruptCyclesAfter);

    token.interruptCycles = interruptCyclesAfter;
    return token;
}

uint32_t McuLoadProfiler_end(McuLoadProfilerContext context,
                             McuLoadProfilerToken token)
{
    uint32_t endCycles;
    uint32_t interruptCyclesBefore;
    uint32_t interruptCyclesAfter;
    uint32_t rawCycles;
    uint32_t preemptCycles;
    uint32_t exclusiveCycles;

    if ((McuLoadProfiler_available == 0u)
        || ((uint32_t)context >= (uint32_t)MCU_LOAD_CONTEXT_COUNT))
    {
        return 0u;
    }

    do
    {
        interruptCyclesBefore = McuLoadProfiler_interruptExclusiveCycles;
        endCycles = MCU_LOAD_PROFILER_READ_CYCLES();
        interruptCyclesAfter = McuLoadProfiler_interruptExclusiveCycles;
    } while (interruptCyclesBefore != interruptCyclesAfter);

    rawCycles = endCycles - token.startCycles;
    preemptCycles = interruptCyclesAfter - token.interruptCycles;
    if (preemptCycles > rawCycles)
    {
        exclusiveCycles = 0u;
        McuLoadProfiler_setStatusInternal(
            MCU_LOAD_PROFILER_STATUS_PREEMPT_CYCLES_INVALID);
    }
    else
    {
        exclusiveCycles = rawCycles - preemptCycles;
    }

    /* A physical 500 ms window cannot overflow a 32-bit cycle accumulator
     * at the configured 160 MHz clock. Window-total validation is therefore
     * kept in the low-frequency service path. */
#if MCU_FAST_PROFILE_ENABLE
    if ((context == MCU_LOAD_CONTEXT_FOC_FAST) && (McuFastProfile_active != 0u))
    {
        McuFastProfile_finish(exclusiveCycles);
    }
#endif
    McuLoadProfiler_windowExclusiveCycles[context] += exclusiveCycles;

    McuLoadProfiler_lastRawCycles[context] = rawCycles;
    if (rawCycles > McuLoadProfiler_maxRawCycles[context])
    {
        McuLoadProfiler_maxRawCycles[context] = rawCycles;
    }

    if ((uint32_t)context <= (uint32_t)MCU_LOAD_CONTEXT_IPC_IRQ)
    {
        uint32_t interruptMask;

        MCU_LOAD_PROFILER_ENTER_CRITICAL(interruptMask);
        McuLoadProfiler_interruptExclusiveCycles += exclusiveCycles;
        MCU_LOAD_PROFILER_EXIT_CRITICAL(interruptMask);
    }

    return rawCycles;
}

static float McuLoadProfiler_cyclesToPercent(uint32_t cycles,
                                             uint32_t windowCycles)
{
    if (cycles >= windowCycles)
    {
        return MCU_LOAD_PROFILER_PERCENT_SCALE;
    }

    return (((float)cycles * MCU_LOAD_PROFILER_PERCENT_SCALE)
        / (float)windowCycles);
}

static void McuLoadProfiler_publish(const uint32_t *windowCycles,
                                    const uint32_t *lastCycles,
                                    const uint32_t *maxCycles,
                                    uint32_t measuredWindowCycles)
{
    uint32_t index;
    uint32_t totalCycles = 0u;
    float totalLoad;

    Meas_MCU_FocFastLoad_pct_f32 = McuLoadProfiler_cyclesToPercent(
        windowCycles[MCU_LOAD_CONTEXT_FOC_FAST], measuredWindowCycles);
    Meas_MCU_FocSpdLoad_pct_f32 = McuLoadProfiler_cyclesToPercent(
        windowCycles[MCU_LOAD_CONTEXT_FOC_SPD], measuredWindowCycles);
    Meas_MCU_SysTickLoad_pct_f32 = McuLoadProfiler_cyclesToPercent(
        windowCycles[MCU_LOAD_CONTEXT_SYSTICK], measuredWindowCycles);
    Meas_MCU_GpioIrqLoad_pct_f32 = McuLoadProfiler_cyclesToPercent(
        windowCycles[MCU_LOAD_CONTEXT_GPIO_IRQ], measuredWindowCycles);
    Meas_MCU_IpcIrqLoad_pct_f32 = McuLoadProfiler_cyclesToPercent(
        windowCycles[MCU_LOAD_CONTEXT_IPC_IRQ], measuredWindowCycles);
    Meas_MCU_RteTaskLoad_pct_f32 = McuLoadProfiler_cyclesToPercent(
        windowCycles[MCU_LOAD_CONTEXT_RTE_TASK], measuredWindowCycles);
    Meas_MCU_Test500msTaskLoad_pct_f32 = McuLoadProfiler_cyclesToPercent(
        windowCycles[MCU_LOAD_CONTEXT_TEST_500MS_TASK], measuredWindowCycles);

    for (index = 0u; index < (uint32_t)MCU_LOAD_CONTEXT_COUNT; index++)
    {
        if ((MCU_LOAD_PROFILER_UINT32_MAX - totalCycles) < windowCycles[index])
        {
            totalCycles = MCU_LOAD_PROFILER_UINT32_MAX;
            McuLoadProfiler_setStatusInternal(
                MCU_LOAD_PROFILER_STATUS_TOTAL_CYCLES_INVALID);
            break;
        }
        totalCycles += windowCycles[index];
    }

    /* Keep volatile observation reads sequenced for the IAR compiler. */
    totalLoad = Meas_MCU_FocFastLoad_pct_f32;
    totalLoad += Meas_MCU_FocSpdLoad_pct_f32;
    totalLoad += Meas_MCU_SysTickLoad_pct_f32;
    totalLoad += Meas_MCU_GpioIrqLoad_pct_f32;
    totalLoad += Meas_MCU_IpcIrqLoad_pct_f32;
    totalLoad += Meas_MCU_RteTaskLoad_pct_f32;
    totalLoad += Meas_MCU_Test500msTaskLoad_pct_f32;

    if ((totalCycles > measuredWindowCycles)
        || (totalLoad > MCU_LOAD_PROFILER_PERCENT_SCALE))
    {
        totalLoad = MCU_LOAD_PROFILER_PERCENT_SCALE;
        McuLoadProfiler_setStatusInternal(
            MCU_LOAD_PROFILER_STATUS_TOTAL_CYCLES_INVALID);
    }

    Meas_MCU_FocFastLastCycles_u32 = lastCycles[MCU_LOAD_CONTEXT_FOC_FAST];
    Meas_MCU_FocFastMaxCycles_u32 = maxCycles[MCU_LOAD_CONTEXT_FOC_FAST];
    Meas_MCU_FocSpdLastCycles_u32 = lastCycles[MCU_LOAD_CONTEXT_FOC_SPD];
    Meas_MCU_FocSpdMaxCycles_u32 = maxCycles[MCU_LOAD_CONTEXT_FOC_SPD];
    Meas_MCU_SysTickLastCycles_u32 = lastCycles[MCU_LOAD_CONTEXT_SYSTICK];
    Meas_MCU_SysTickMaxCycles_u32 = maxCycles[MCU_LOAD_CONTEXT_SYSTICK];
    Meas_MCU_GpioIrqLastCycles_u32 = lastCycles[MCU_LOAD_CONTEXT_GPIO_IRQ];
    Meas_MCU_GpioIrqMaxCycles_u32 = maxCycles[MCU_LOAD_CONTEXT_GPIO_IRQ];
    Meas_MCU_IpcIrqLastCycles_u32 = lastCycles[MCU_LOAD_CONTEXT_IPC_IRQ];
    Meas_MCU_IpcIrqMaxCycles_u32 = maxCycles[MCU_LOAD_CONTEXT_IPC_IRQ];
    Meas_MCU_RteTaskLastCycles_u32 = lastCycles[MCU_LOAD_CONTEXT_RTE_TASK];
    Meas_MCU_RteTaskMaxCycles_u32 = maxCycles[MCU_LOAD_CONTEXT_RTE_TASK];
    Meas_MCU_Test500msTaskLastCycles_u32 =
        lastCycles[MCU_LOAD_CONTEXT_TEST_500MS_TASK];
    Meas_MCU_Test500msTaskMaxCycles_u32 =
        maxCycles[MCU_LOAD_CONTEXT_TEST_500MS_TASK];

    Meas_MCU_TotalLoad_pct_f32 = totalLoad;
    Meas_MCU_Reserve_pct_f32 = MCU_LOAD_PROFILER_PERCENT_SCALE - totalLoad;
    Meas_MCU_WindowCycles_u32 = measuredWindowCycles;
    Meas_MCU_UpdateSeq_u32++;
}

void McuLoadProfiler_service(void)
{
    uint32_t nowCycles;
    uint32_t measuredWindowCycles;
    uint32_t interruptMask;
    uint32_t index;
    uint32_t windowCycles[MCU_LOAD_CONTEXT_COUNT];
    uint32_t lastCycles[MCU_LOAD_CONTEXT_COUNT];
    uint32_t maxCycles[MCU_LOAD_CONTEXT_COUNT];
#if MCU_FAST_PROFILE_ENABLE
    uint32_t fastParts[MCU_FAST_OTHER];
    uint32_t fastTotal;
    uint32_t fastSamples;
#endif

    if (McuLoadProfiler_available == 0u)
    {
        return;
    }

    nowCycles = MCU_LOAD_PROFILER_READ_CYCLES();
    if ((nowCycles - s_windowStartCycles) < s_windowPeriodCycles)
    {
        return;
    }

    MCU_LOAD_PROFILER_ENTER_CRITICAL(interruptMask);
    nowCycles = MCU_LOAD_PROFILER_READ_CYCLES();
    measuredWindowCycles = nowCycles - s_windowStartCycles;
    if (measuredWindowCycles < s_windowPeriodCycles)
    {
        MCU_LOAD_PROFILER_EXIT_CRITICAL(interruptMask);
        return;
    }

    s_windowStartCycles = nowCycles;
    for (index = 0u; index < (uint32_t)MCU_LOAD_CONTEXT_COUNT; index++)
    {
        windowCycles[index] = McuLoadProfiler_windowExclusiveCycles[index];
        McuLoadProfiler_windowExclusiveCycles[index] = 0u;
        lastCycles[index] = McuLoadProfiler_lastRawCycles[index];
        maxCycles[index] = McuLoadProfiler_maxRawCycles[index];
    }
#if MCU_FAST_PROFILE_ENABLE
    for (index = 0u; index < (uint32_t)MCU_FAST_OTHER; index++)
    {
        fastParts[index] = s_fastParts[index];
        s_fastParts[index] = 0u;
    }
    fastTotal = s_fastTotal;
    fastSamples = s_fastSamples;
    s_fastTotal = 0u;
    s_fastSamples = 0u;
#endif
    MCU_LOAD_PROFILER_EXIT_CRITICAL(interruptMask);

    if (measuredWindowCycles == 0u)
    {
        McuLoadProfiler_setStatusInternal(
            MCU_LOAD_PROFILER_STATUS_TOTAL_CYCLES_INVALID);
        return;
    }

#if MCU_FAST_PROFILE_ENABLE
    McuFastProfile_publish(fastParts, fastTotal, fastSamples);
#endif
    McuLoadProfiler_publish(windowCycles, lastCycles, maxCycles,
                            measuredWindowCycles);
}

#if defined(MCU_LOAD_PROFILER_HOST_TEST)
void McuLoadProfiler_testInitialize(uint32_t coreClockHz,
                                    uint32_t startCycles,
                                    uint8_t dwtAvailable)
{
    s_testCycleCounter = startCycles;
    McuLoadProfiler_resetState(coreClockHz, startCycles, dwtAvailable);
}

void McuLoadProfiler_testSetCycles(uint32_t cycles)
{
    s_testCycleCounter = cycles;
}

void McuLoadProfiler_testSetInterruptCycles(uint32_t cycles)
{
    McuLoadProfiler_interruptExclusiveCycles = cycles;
}
#endif

#endif /* MCU_LOAD_PROFILER_ENABLE */
