#include "m4_wdg.h"
#include "cy_sysreset.h"
#include "cy_device_headers.h"
#include "cy_project.h"

cy_stc_mcwdt_config_t mcwdtConfig =
{
    .coreSelect       = CY_MCWDT_PAUSED_BY_DPSLP_CM4_CM7_0,
    .c0LowerLimit     = 0,
    .c0UpperLimit     = 32000,
    .c0WarnLimit      = 0,
    .c0LowerAction    = CY_MCWDT_ACTION_NONE,
    .c0UpperAction    = CY_MCWDT_ACTION_FAULT_THEN_RESET,
    .c0WarnAction     = CY_MCWDT_WARN_ACTION_NONE,
    .c0AutoService    = CY_MCWDT_DISABLE,
    .c0SleepDeepPause = CY_MCWDT_ENABLE,
    .c0DebugRun       = CY_MCWDT_ENABLE,
    .c1LowerLimit     = 100,
    .c1UpperLimit     = 64000,
    .c1WarnLimit      = 0,
    .c1LowerAction    = CY_MCWDT_ACTION_NONE,
    .c1UpperAction    = CY_MCWDT_ACTION_NONE,
    .c1WarnAction     = CY_MCWDT_WARN_ACTION_NONE,
    .c1AutoService    = CY_MCWDT_DISABLE,
    .c1SleepDeepPause = CY_MCWDT_ENABLE,
    .c1DebugRun       = CY_MCWDT_ENABLE,
    .c2ToggleBit      = CY_MCWDT_CNT2_MONITORED_BIT15,
    .c2Action         = CY_MCWDT_CNT2_ACTION_INT,
    .c2SleepDeepPause = CY_MCWDT_ENABLE,
    .c2DebugRun       = CY_MCWDT_ENABLE,
};

void Wdg_Init(void)
{
    Cy_MCWDT_CpuSelectForDpSlpPauseAction(MCWDT1,
        CY_MCWDT_PAUSED_BY_DPSLP_CM4_CM7_0);

    Cy_MCWDT_SetLowerAction(MCWDT1, CY_MCWDT_COUNTER0,
        CY_MCWDT_ACTION_NONE);
    Cy_MCWDT_SetUpperAction(MCWDT1, CY_MCWDT_COUNTER0,
        CY_MCWDT_ACTION_NONE);
    Cy_MCWDT_SetWarnAction(MCWDT1, CY_MCWDT_COUNTER0,
        CY_MCWDT_WARN_ACTION_NONE);

    Cy_MCWDT_SetLowerAction(MCWDT1, CY_MCWDT_COUNTER1,
        CY_MCWDT_ACTION_NONE);
    Cy_MCWDT_SetUpperAction(MCWDT1, CY_MCWDT_COUNTER1,
        CY_MCWDT_ACTION_FAULT_THEN_RESET);
    Cy_MCWDT_SetWarnAction(MCWDT1, CY_MCWDT_COUNTER1,
        CY_MCWDT_WARN_ACTION_NONE);

    Cy_MCWDT_SetSubCounter2Action(MCWDT1, CY_MCWDT_CNT2_ACTION_NONE);

    Cy_MCWDT_SetLowerLimit(MCWDT1, CY_MCWDT_COUNTER0, 0, 0);
    Cy_MCWDT_SetWarnLimit(MCWDT1, CY_MCWDT_COUNTER0, 0, 0);
    Cy_MCWDT_SetUpperLimit(MCWDT1, CY_MCWDT_COUNTER0, 100, 0);

    Cy_MCWDT_SetLowerLimit(MCWDT1, CY_MCWDT_COUNTER1, 0, 0);
    Cy_MCWDT_SetWarnLimit(MCWDT1, CY_MCWDT_COUNTER1, 0, 0);
    Cy_MCWDT_SetUpperLimit(MCWDT1, CY_MCWDT_COUNTER1, 32000, 0);

    Cy_MCWDT_SetToggleBit(MCWDT1, CY_MCWDT_CNT2_MONITORED_BIT15);

    Cy_MCWDT_SetAutoService(MCWDT1, CY_MCWDT_COUNTER0, 0ul);
    Cy_MCWDT_SetAutoService(MCWDT1, CY_MCWDT_COUNTER1, 0ul);

    Cy_MCWDT_SetSleepDeepPause(MCWDT1, CY_MCWDT_COUNTER0, 1ul);
    Cy_MCWDT_SetSleepDeepPause(MCWDT1, CY_MCWDT_COUNTER1, 1ul);
    Cy_MCWDT_SetSleepDeepPause(MCWDT1, CY_MCWDT_COUNTER2, 1ul);

    Cy_MCWDT_SetDebugRun(MCWDT1, CY_MCWDT_COUNTER0, 1ul);
    Cy_MCWDT_SetDebugRun(MCWDT1, CY_MCWDT_COUNTER1, 1ul);
    Cy_MCWDT_SetDebugRun(MCWDT1, CY_MCWDT_COUNTER2, 1ul);

    Cy_MCWDT_Enable(MCWDT1, CY_MCWDT_CTR_Msk, 0);

    while (Cy_MCWDT_GetEnabledStatus(MCWDT1, CY_MCWDT_COUNTER0) != 1ul) {}
    while (Cy_MCWDT_GetEnabledStatus(MCWDT1, CY_MCWDT_COUNTER1) != 1ul) {}
    while (Cy_MCWDT_GetEnabledStatus(MCWDT1, CY_MCWDT_COUNTER2) != 1ul) {}
}

void Feed_Dog(void)
{
    Cy_MCWDT_ClearWatchdog(MCWDT1, CY_MCWDT_COUNTER0);
    Cy_MCWDT_ClearWatchdog(MCWDT1, CY_MCWDT_COUNTER1);
}
