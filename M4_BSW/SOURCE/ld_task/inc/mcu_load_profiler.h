#ifndef M4_BSW_SOURCE_LD_TASK_INC_MCU_LOAD_PROFILER_H_
#define M4_BSW_SOURCE_LD_TASK_INC_MCU_LOAD_PROFILER_H_

#include <stdint.h>

#ifndef MCU_LOAD_PROFILER_ENABLE
#define MCU_LOAD_PROFILER_ENABLE (1u)
#endif

/* Diagnostic-only fast-loop breakdown. Power-of-two decimation limits cost. */
#ifndef MCU_FAST_PROFILE_ENABLE
#if defined(USE_M4) || defined(MCU_LOAD_PROFILER_HOST_TEST)
#define MCU_FAST_PROFILE_ENABLE MCU_LOAD_PROFILER_ENABLE
#else
#define MCU_FAST_PROFILE_ENABLE (0u)
#endif
#endif
#ifndef MCU_FAST_PROFILE_DIVIDER
#define MCU_FAST_PROFILE_DIVIDER (16u)
#endif
#if MCU_FAST_PROFILE_ENABLE && !MCU_LOAD_PROFILER_ENABLE
#error "Fast breakdown requires MCU_LOAD_PROFILER_ENABLE"
#endif
#if (MCU_FAST_PROFILE_DIVIDER == 0u) || ((MCU_FAST_PROFILE_DIVIDER & (MCU_FAST_PROFILE_DIVIDER - 1u)) != 0u)
#error "MCU_FAST_PROFILE_DIVIDER must be a nonzero power of two"
#endif

#define MCU_LOAD_PROFILER_STATUS_DWT_UNAVAILABLE       (0x01u)
#define MCU_LOAD_PROFILER_STATUS_PREEMPT_CYCLES_INVALID (0x02u)
#define MCU_LOAD_PROFILER_STATUS_TOTAL_CYCLES_INVALID   (0x04u)

typedef enum
{
    MCU_LOAD_CONTEXT_FOC_FAST = 0,
    MCU_LOAD_CONTEXT_FOC_SPD,
    MCU_LOAD_CONTEXT_SYSTICK,
    MCU_LOAD_CONTEXT_GPIO_IRQ,
    MCU_LOAD_CONTEXT_IPC_IRQ,
    MCU_LOAD_CONTEXT_RTE_TASK,
    MCU_LOAD_CONTEXT_TEST_500MS_TASK,
    MCU_LOAD_CONTEXT_COUNT
} McuLoadProfilerContext;

typedef struct
{
    uint32_t startCycles;
    uint32_t interruptCycles;
} McuLoadProfilerToken;

typedef enum
{
    MCU_FAST_ADC = 0,       /* ADC, reconstruction and Clarke */
    MCU_FAST_OBS,           /* estimator / one-shot observer reset */
    MCU_FAST_CTRL,          /* current regulation or V/f, including RRC */
    MCU_FAST_VPRE,          /* voltage eligibility, HFI apply/reset */
    MCU_FAST_MOD,           /* modulator execute and output retrieval */
    MCU_FAST_VPOST,         /* FWC, voltage feedback, VAFID and RRC capture */
    MCU_FAST_PWM,           /* pattern generator / enable-disable handling */
    MCU_FAST_OTHER,
    MCU_FAST_PART_COUNT
} McuFastProfilePart;

#if MCU_FAST_PROFILE_ENABLE
extern uint32_t McuFastProfile_active;
void McuFastProfile_start(McuLoadProfilerToken token);
void McuFastProfile_markInternal(McuFastProfilePart part);
void McuFastProfile_finish(uint32_t exclusiveCycles);
#define MCU_FAST_MARK(part) do { if (McuFastProfile_active != 0u) { \
    McuFastProfile_markInternal(part); } } while (0)
extern volatile float Meas_MCU_FastAdc_pct_f32;
extern volatile float Meas_MCU_FastObs_pct_f32;
extern volatile float Meas_MCU_FastCtrl_pct_f32;
extern volatile float Meas_MCU_FastVPre_pct_f32;
extern volatile float Meas_MCU_FastMod_pct_f32;
extern volatile float Meas_MCU_FastVPost_pct_f32;
extern volatile float Meas_MCU_FastPwm_pct_f32;
extern volatile float Meas_MCU_FastOther_pct_f32;
extern volatile uint32_t Meas_MCU_FastSamples_u32;
#else
#define McuFastProfile_start(token) ((void)0)
#define MCU_FAST_MARK(part) ((void)0)
#endif

#if (MCU_LOAD_PROFILER_ENABLE != 0u)

void McuLoadProfiler_initialize(void);
void McuLoadProfiler_service(void);

/* Shared hot-path state. These symbols are internal to the profiler contract;
 * they are declared here only so IAR can force-inline begin/end despite the
 * project-wide --no_inline setting. */
extern volatile uint32_t McuLoadProfiler_interruptExclusiveCycles;
extern uint32_t McuLoadProfiler_windowExclusiveCycles[MCU_LOAD_CONTEXT_COUNT];
extern uint32_t McuLoadProfiler_lastRawCycles[MCU_LOAD_CONTEXT_COUNT];
extern uint32_t McuLoadProfiler_maxRawCycles[MCU_LOAD_CONTEXT_COUNT];
extern volatile uint8_t McuLoadProfiler_available;
void McuLoadProfiler_setStatusInternal(uint8_t status);

#if defined(__ICCARM__) \
    && !defined(MCU_LOAD_PROFILER_IMPLEMENTATION) \
    && !defined(MCU_LOAD_PROFILER_HOST_TEST)

#include "cy_project.h"
#include "cy_device_headers.h"

#pragma inline=forced
static inline McuLoadProfilerToken McuLoadProfiler_begin(void)
{
    McuLoadProfilerToken token;
    uint32_t interruptCyclesBefore;
    uint32_t interruptCyclesAfter;

    /* DWT availability is checked during initialize/service. CYCCNT remains a
     * safe read when unsupported, so the hot path needs no repeated branch. */
    do
    {
        interruptCyclesBefore = McuLoadProfiler_interruptExclusiveCycles;
        token.startCycles = DWT->CYCCNT;
        interruptCyclesAfter = McuLoadProfiler_interruptExclusiveCycles;
    } while (interruptCyclesBefore != interruptCyclesAfter);

    token.interruptCycles = interruptCyclesAfter;
    return token;
}

#pragma inline=forced
static inline uint32_t McuLoadProfiler_end(McuLoadProfilerContext context,
                                           McuLoadProfilerToken token)
{
    uint32_t endCycles;
    uint32_t interruptCyclesBefore;
    uint32_t interruptCyclesAfter;
    uint32_t rawCycles;
    uint32_t preemptCycles;
    uint32_t exclusiveCycles;

    if ((uint32_t)context >= (uint32_t)MCU_LOAD_CONTEXT_COUNT)
    {
        return 0u;
    }

    do
    {
        interruptCyclesBefore = McuLoadProfiler_interruptExclusiveCycles;
        endCycles = DWT->CYCCNT;
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
        uint32_t previousCycles;

        /* Exceptions clear the local exclusive monitor. A nested interrupt
         * therefore makes STREX retry without a lost read/modify/write. */
        do
        {
            previousCycles = __LDREXW(
                &McuLoadProfiler_interruptExclusiveCycles);
        } while (__STREXW(previousCycles + exclusiveCycles,
                          &McuLoadProfiler_interruptExclusiveCycles) != 0u);
    }

    return rawCycles;
}

#else

McuLoadProfilerToken McuLoadProfiler_begin(void);
uint32_t McuLoadProfiler_end(McuLoadProfilerContext context,
                             McuLoadProfilerToken token);

#endif

extern volatile float Meas_MCU_FocFastLoad_pct_f32;
extern volatile uint32_t Meas_MCU_FocFastLastCycles_u32;
extern volatile uint32_t Meas_MCU_FocFastMaxCycles_u32;
extern volatile float Meas_MCU_FocSpdLoad_pct_f32;
extern volatile uint32_t Meas_MCU_FocSpdLastCycles_u32;
extern volatile uint32_t Meas_MCU_FocSpdMaxCycles_u32;
extern volatile float Meas_MCU_SysTickLoad_pct_f32;
extern volatile uint32_t Meas_MCU_SysTickLastCycles_u32;
extern volatile uint32_t Meas_MCU_SysTickMaxCycles_u32;
extern volatile float Meas_MCU_GpioIrqLoad_pct_f32;
extern volatile uint32_t Meas_MCU_GpioIrqLastCycles_u32;
extern volatile uint32_t Meas_MCU_GpioIrqMaxCycles_u32;
extern volatile float Meas_MCU_IpcIrqLoad_pct_f32;
extern volatile uint32_t Meas_MCU_IpcIrqLastCycles_u32;
extern volatile uint32_t Meas_MCU_IpcIrqMaxCycles_u32;
extern volatile float Meas_MCU_RteTaskLoad_pct_f32;
extern volatile uint32_t Meas_MCU_RteTaskLastCycles_u32;
extern volatile uint32_t Meas_MCU_RteTaskMaxCycles_u32;
extern volatile float Meas_MCU_Test500msTaskLoad_pct_f32;
extern volatile uint32_t Meas_MCU_Test500msTaskLastCycles_u32;
extern volatile uint32_t Meas_MCU_Test500msTaskMaxCycles_u32;

extern volatile float Meas_MCU_TotalLoad_pct_f32;
extern volatile float Meas_MCU_Reserve_pct_f32;
extern volatile uint32_t Meas_MCU_WindowCycles_u32;
extern volatile uint32_t Meas_MCU_UpdateSeq_u32;
extern volatile uint8_t Meas_MCU_Status_u8;

#if defined(MCU_LOAD_PROFILER_HOST_TEST)
void McuLoadProfiler_testInitialize(uint32_t coreClockHz,
                                    uint32_t startCycles,
                                    uint8_t dwtAvailable);
void McuLoadProfiler_testSetCycles(uint32_t cycles);
void McuLoadProfiler_testSetInterruptCycles(uint32_t cycles);
#endif

#else

#if defined(__ICCARM__)
#pragma inline=forced
#endif
static inline void McuLoadProfiler_initialize(void)
{
}

#if defined(__ICCARM__)
#pragma inline=forced
#endif
static inline McuLoadProfilerToken McuLoadProfiler_begin(void)
{
    McuLoadProfilerToken token = {0u, 0u};
    return token;
}

#if defined(__ICCARM__)
#pragma inline=forced
#endif
static inline uint32_t McuLoadProfiler_end(McuLoadProfilerContext context,
                                           McuLoadProfilerToken token)
{
    (void)context;
    (void)token;
    return 0u;
}

#if defined(__ICCARM__)
#pragma inline=forced
#endif
static inline void McuLoadProfiler_service(void)
{
}

#endif

#endif /* M4_BSW_SOURCE_LD_TASK_INC_MCU_LOAD_PROFILER_H_ */
