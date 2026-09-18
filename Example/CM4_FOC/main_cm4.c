#include "../../ConfigWizard/FocTiming_Cfg.h"
/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

/*******************************************************************************
**                      Includes                                              **
*******************************************************************************/

#include "cy_project.h"
#include "cy_device_headers.h"
#include "m4_task.h"
#include "mcu_load_profiler.h"

#include "Ifx_MHA_BridgeDrv_TLE9563.h"
#include "Ifx_MHA_MeasurementADC_CYT2B7.h"
#include "Ifx_MHA_PatternGen_Cfg.h"
#include "Ifx_MS_FocSolutionF16.h"
#include "Ifx_MDA_FluxEstimatorF16_Cfg.h"
#include "Ifx_MDA_IToFControllerF16_Cfg.h"
#include "Ifx_MDA_VToFControllerF16_Cfg.h"
#include "Ifx_Math_DivSat.h"
#include "no_opt.h"
#include "../../xcp/xcp_cal_m4.h"
#include "../../IdMap_codegen/id_map_q15_adapter.h"
#include "../../KRE_codegen/external_observer_manager.h"
#if FOC_AUX_ALGORITHMS_ENABLE
#include "../../HFIPD_codegen/hfipd_injection.h"
#endif
#include "../../ADRC_codegen/adrc_speed_controller_adapter.h"
#if FOC_AUX_ALGORITHMS_ENABLE
#include "../../APSFSM_codegen/apsfsm_torque_compensation_adapter.h"
#endif
#if FOC_RRCDOB_ENABLE
#include "../../RRC_DOB_codegen/rrc_dob_compensator.h"
#endif
#if FOC_AUX_ALGORITHMS_ENABLE
#include "../../VAFID_codegen/vafid_parameter_identifier.h"
#endif
#if FOC_AUX_ALGORITHMS_ENABLE
#include "../../HFI_codegen/hfi_injection_adapter.h"
#endif
#include "../../FWC_codegen/fwc_q15_adapter.h"

#include "SDL_init.h"
#include "SDL_SysInt_Cfg.h"
#include "SDL_Tcpwm_Cfg.h"
#include "TLE9563_FuncLayer.h"

#define FOC_ADRC_SPEED_SELECTOR_PI                 (0u)
#define FOC_ADRC_SPEED_SELECTOR_ADRC               (1u)

/* Api Call Helper Variables */
NO_OPT volatile uint8               execApiId     = 0u;
NO_OPT volatile uint8               execApiLastId = 0u;
NO_OPT volatile uint8               csaGain       = 10u;

/* User input speed which can be set in rpm */
NO_OPT volatile Ifx_Math_Fract16    referenceSpeedQ0 = 2000;

/* Currents in dq frame which can be set when direct interface is enabled */
NO_OPT volatile Ifx_Math_CmpFract16 currentsDqReference = {0, 0};

/* FOC command source: 0 = CAN input, 1 = calibration input */
NO_OPT volatile uint8               Cal_FocCommandSource_u8 = 0u;
NO_OPT volatile uint8               Cal_FocEnable_u8 = 0u;
NO_OPT volatile Ifx_Math_Fract16    Cal_FocSpeedCommand_rpm_s16 = 0;
NO_OPT volatile Ifx_Math_Fract16    Cal_FocCurrentCommandD_Q15_s16 = 0;
NO_OPT volatile Ifx_Math_Fract16    Cal_FocCurrentCommandQ_Q15_s16 = 0;
NO_OPT volatile uint8               Cal_FocControlMode_u8 = 1u;
NO_OPT volatile uint8               Cal_FocDirectInterface_u8 = 0u;

/* Startup mode: 0 = alignment then direct KRE closed loop;
 * 1 = alignment, I/f open loop, then normal closed-loop transition.
 * Other values follow route 1. KRE is the only production estimator owner;
 * I/f is an optional transition route, not a terminal owner or fallback. */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8 Cal_FocStartupMode_u8 = 0u;

/* These legacy symbol names provide the shared permanent-magnet flux
 * calibration consumed by KRE, HFO, and VAFID; they are not an Id source. */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
/* 23.5 V/krpm line-line RMS at four pole pairs is approximately 46 mWb. */
NO_OPT volatile uint16 Cal_MTPA_PermanentMagnetFlux_mWb_u16 = 46u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16 Cal_MTPA_AppliedPermanentMagnetFlux_mWb_u16 = 0u;

/* Speed ADRC uses the R2026a example as a generated-code reference. The
 * selector is latched only while FOC is inactive; all numeric ADRC
 * calibrations are intentionally live while ADRC owns the speed loop. */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8 Cal_ADRC_Speed_Selector_u8 = FOC_ADRC_SPEED_SELECTOR_PI;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8 Cal_ADRC_Speed_Reset_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_ADRC_Speed_CriticalGain_PU_per_PU_s2_f32 = 19817.677368F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_ADRC_Speed_ControlBandwidth_radps_f32 = 104.719757F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_ADRC_Speed_ObserverBandwidth_radps_f32 = 837.758057F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_ADRC_Speed_IqUpperLimit_Q15_s16 =
    IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_OUT_UPP_LIMIT_Q;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_ADRC_Speed_IqLowerLimit_Q15_s16 =
    IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_OUT_LOW_LIMIT_Q;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8 Meas_ADRC_Speed_Active_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8 Meas_ADRC_Speed_Valid_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8 Meas_ADRC_Speed_Status_u8 = ADRC_SPEED_CONTROLLER_STATUS_IDLE;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Meas_ADRC_Speed_IqRef_Q15_s16 = 0;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_ADRC_Speed_ReferenceFiltered_PU_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_ADRC_Speed_Output_PU_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_ADRC_Speed_EstimatedSpeed_PU_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_ADRC_Speed_EstimatedAcceleration_PU_per_s_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_ADRC_Speed_EstimatedDisturbance_PU_per_s2_f32 = 0.0F;

/* FOC instance */
NO_OPT Ifx_MS_FocSolutionF16        FocDemoClosedLoop;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile XcpM4CalibrationType XcpM4Calibration =
{
    1000u, 1000u, 1000u, 0u, 0u, 0u, 0u, 0u
};

/* Positioning d-q current ramp, normalized by base current in Q15.
 * The target vector is held at the end of the ramp and is also the I/f
 * drag current, so positioning and open-loop startup share one target. */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_FocAlignmentCurrentD_Q15_s16 = 0;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_FocAlignmentCurrentQ_Q15_s16 = 0x4000;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_FocAlignmentCurrentStartD_Q15_s16 = 0;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_FocAlignmentCurrentStartQ_Q15_s16 = 0;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16           Cal_FocAlignmentDuration_cnt_u16 = 666u;

/* Runtime motor calibration sources: resistance in mOhm, inductances in uH. */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16 Cal_MotorPhaseResistance_mOhm_u16 = 500u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16 Cal_MotorDirectInductance_uH_u16 = 1300u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16 Cal_MotorQuadratureInductance_uH_u16 = 1380u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8 Cal_MotorPolePairs_u8 = IFX_MS_FOCSOLUTIONF16_CFG_POLE_PAIRS;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8 Cal_MotorParameterStatus_u8 = 0u;

/* Runtime values accepted by the fixed-Q validation and used by FOC. */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16 Cal_MotorAppliedPhaseResistance_mOhm_u16 = 500u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16 Cal_MotorAppliedDirectInductance_uH_u16 = 1300u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16 Cal_MotorAppliedQuadratureInductance_uH_u16 = 1380u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8 Cal_MotorAppliedPolePairs_u8 = IFX_MS_FOCSOLUTIONF16_CFG_POLE_PAIRS;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_MotorAppliedFluxResistance_Q15_s16 =
    IFX_MDA_FLUXESTIMATORF16_CFG_PHASE_RES_Q15;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_MotorAppliedFluxInductance_Q15_s16 =
    IFX_MDA_FLUXESTIMATORF16_CFG_PHASE_IND_Q15;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_MotorAppliedIToFAngleIncrement_Q14_s16 =
    IFX_MDA_ITOFCONTROLLERF16_CFG_ANGLE_INC_Q14;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract16 Cal_MotorAppliedVToFAngleIncrement_Q14_s16 =
    IFX_MDA_VTOFCONTROLLERF16_CFG_ANGLE_INC_Q14;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile Ifx_Math_Fract32 Cal_MotorAppliedSystemBaseTime_Q30_s32 =
    IFX_MDA_FLUXESTIMATORF16_CFG_SYSTEM_BASE_TIME_Q30;

/* User inputs to enable FOC */
/* Variables are defined as uint8 instead of bool because of the error in microinspector */
NO_OPT volatile uint8               enablePowerStage = 0u;
NO_OPT volatile uint8               enableControl    = 0u,enableControl_last = 0;

/* Corresponds to enum Ifx_MS_FocSolutionF16_ControlMode; mode = 1 == IToF and FOC; mode = 0 == VToF */
NO_OPT volatile uint8               controlMode           = 1u;
NO_OPT volatile uint8               enableDirectInterface = 0u;

/* User input to clear FOC fault */
NO_OPT volatile uint8               clrFaultFoc = 0u;

/* Counter for the rotor alignment */
NO_OPT volatile uint16              rotorAlignCounter = 0u;

/* Temporary scheduler diagnostics can be added to the measurement description
 * manually. */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32 Meas_Foc_FastLoopCount_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32 Meas_Foc_SpeedLoopCount_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32 Meas_Foc_IdleSvcCount_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16 Meas_Foc_AlignRemain_tick_u16 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32 Meas_Foc_FastLoopLastCycles_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32 Meas_Foc_FastLoopMaxCycles_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32 Meas_Foc_FastLoopOverrunCount_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32 Meas_Foc_FastLoopBudgetCycles_u32 = 0u;
uint16_t adcChResult[4];
cy_stc_adc_ch_status_t adcChStatus;

static void FocPrepareIdMapReference(Ifx_Math_Fract16 speedQ15,
                                     Ifx_Math_CmpFract16 *currentReference);
static bool FocIdMapControlIsActive(void);
static uint8 FocAdrcSpeedControllerExecute(void *context,
                                           Ifx_Math_Fract16 referenceSpeedQ15,
                                           Ifx_Math_Fract16 estimatedSpeedQ15,
                                           Ifx_Math_Fract16 *currentQReferenceQ15);
static void FocApplyAdrcSpeedController(void);
static void FocClearAdrcSpeedMeasurements(void);
#if FOC_AUX_ALGORITHMS_ENABLE
static bool FocApsfsmSpeedIsEligible(void);
#endif
#if FOC_AUX_ALGORITHMS_ENABLE
static void FocApplyApsfsmTorqueCompensation(void);
#endif
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32 Meas_Foc_PwmIrqCount_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32 Meas_Foc_PwmIrqLastCycles_u32 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32 Meas_Foc_PwmIrqMaxCycles_u32 = 0u;

static bool FocFwcControlIsEligible(void);
static void FocApplyMotorCalibration(bool forceApply);
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
static void FocApplyKreParameters(void);
#endif
#if FOC_RRCDOB_ENABLE
static void FocApplyRrcDobParameters(void);
#endif
#if FOC_AUX_ALGORITHMS_ENABLE
static void FocApplyVafidParameters(void);
#endif
#if FOC_AUX_ALGORITHMS_ENABLE
static void FocServiceVafid(void);
#endif
static void FocLatchCommandAtSpeedBoundary(void);

#if IFX_MHA_PATTERNGEN_CFG_CURRENT_LOOP_FACTOR != FOC_PWM_PER_CONTROL || IFX_MS_FOCSOLUTIONF16_CFG_CURRENT_LOOP_FACTOR != FOC_PWM_PER_CONTROL
#error "PWM and FOC decimation must match"
#endif
#if IFX_MS_FOCSOLUTIONF16_CFG_SPEED_LOOP_PERIOD_US != FOC_SPEED_PERIOD_US || IFX_MDA_FOCCONTROLLERF16_CFG_SAMPLING_TIME_US != FOC_CONTROL_PERIOD_US
#error "Controller periods must match the interrupt schedule"
#endif
#if FOC_DIAG_FLUX_REFERENCE && FOC_CONTROL_PERIOD_US != 50u
#error "Flux diagnostic must use its separate 50 us reference build"
#endif

/* One owner for scheduling and PWM submission; no deferred PendSV work. */
static uint8 focPwmPhase = 0u;

void Ifx_FOC_periodMatchCallback(void)
{
    const uint32 irqStartCycles = DWT->CYCCNT;
#if (MCU_LOAD_PROFILER_ENABLE != 0u)
    const McuLoadProfilerToken irqToken = McuLoadProfiler_begin();
#endif
    uint32 irqElapsedCycles;
    Cy_Tcpwm_Counter_ClearTC_Intr(TCPWMx_GRPx_CNTx_U);
    ++Meas_Foc_PwmIrqCount_u32;
    ++focPwmPhase;
    if (focPwmPhase >= FOC_PWM_PER_CONTROL)
    {
        uint32 controlStartCycles;
        uint32 controlElapsedCycles;
        focPwmPhase = 0u;
        ++Meas_Foc_FastLoopCount_u32;
#if (MCU_LOAD_PROFILER_ENABLE != 0u)
        McuFastProfile_start(irqToken);
#endif
        controlStartCycles = DWT->CYCCNT;
        Ifx_MS_FocSolutionF16_executeControlMode(&FocDemoClosedLoop);
        controlElapsedCycles = DWT->CYCCNT - controlStartCycles;
        Meas_Foc_FastLoopLastCycles_u32 = controlElapsedCycles;
        if (controlElapsedCycles > Meas_Foc_FastLoopMaxCycles_u32)
        {
            Meas_Foc_FastLoopMaxCycles_u32 = controlElapsedCycles;
        }
    }
#if (MCU_LOAD_PROFILER_ENABLE != 0u)
    /* Account every ISR exactly once, including the intervening hold ticks. */
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_FOC_FAST, irqToken);
#endif
    irqElapsedCycles = DWT->CYCCNT - irqStartCycles;
    Meas_Foc_PwmIrqLastCycles_u32 = irqElapsedCycles;
    if (irqElapsedCycles > Meas_Foc_PwmIrqMaxCycles_u32)
    {
        Meas_Foc_PwmIrqMaxCycles_u32 = irqElapsedCycles;
    }
    /* ISR deadline is the PWM period, NOT the decimated control period. */
    if ((Meas_Foc_FastLoopBudgetCycles_u32 != 0u)
        && (irqElapsedCycles >= Meas_Foc_FastLoopBudgetCycles_u32))
    {
        ++Meas_Foc_FastLoopOverrunCount_u32;
    }
}


static Ifx_Math_Fract16 FocCalculateAlignmentCurrentQ15(
    const Ifx_Math_Fract16 startCurrentQ15,
    const Ifx_Math_Fract16 targetCurrentQ15,
    const uint16 duration,
    const uint16 remaining)
{
    int32_t startCurrent;
    int32_t targetCurrent;
    int64_t interpolatedCurrent;

    if ((duration == 0u) || (remaining == 1u))
    {
        return targetCurrentQ15;
    }

    startCurrent = (int32_t)startCurrentQ15;
    targetCurrent = (int32_t)targetCurrentQ15;
    interpolatedCurrent = (int64_t)startCurrent
        + (((int64_t)(targetCurrent - startCurrent) * (duration - remaining)) / duration);

    return (Ifx_Math_Fract16)interpolatedCurrent;
}


/* Slow loop execution call back which is called by timer TCPWM0 */
void Ifx_FOC_speedLoopCallback(void)
{
#if (MCU_LOAD_PROFILER_ENABLE != 0u)
    McuLoadProfilerToken speedLoopProfilerToken;
#endif
    /* Variable to hold the scaled ref. speed */
    Ifx_Math_Fract16 referenceSpeedQ15;
    Ifx_Math_Fract16 requestedSpeedQ15;
    Ifx_Math_CmpFract16 alignmentDqReference;
    Ifx_Math_CmpFract16 effectiveCurrentsDqReference;
    Ifx_MS_FocSolutionF16_VoltageSaturationSnapshot msVoltageSnapshot;
    Fwc_Q15_VoltageSnapshot fwcVoltageSnapshot;
    Fwc_Q15_Output fwcOutput;
    uint8_t fwcSnapshotRead;
    uint8_t fwcControlEligible;
    static uint8 directClosedLoopStartPending = 0u;
    static uint16 startupAlignmentDuration = 0u;
#if FOC_AUX_ALGORITHMS_ENABLE
    Ifx_MS_FocSolutionF16_State stateBeforeSpeed;
#endif

#if (MCU_LOAD_PROFILER_ENABLE != 0u)
    speedLoopProfilerToken = McuLoadProfiler_begin();
#endif
    Meas_Foc_SpeedLoopCount_u32++;

    /* Toggle pin to be able to measure task runtimes */
//    Cy_GPIO_Set(GPIO_PRT20, 0u);

    /* set ref. speed to 0 */
    referenceSpeedQ15 = 0;

    /* Clear peripheral interrupt flag */
    Cy_Tcpwm_Counter_ClearTC_Intr(TCPWM_GRPx_CNTx_SPEEDLOOP);

    /* The FOC state machine advances only in this 2 kHz callback. Consume
     * the selected CAN/XCP command here so start, stop, and speed updates do
     * not depend on foreground service time while KRE is running. */
    FocLatchCommandAtSpeedBoundary();
#if FOC_AUX_ALGORITHMS_ENABLE
    stateBeforeSpeed = FocDemoClosedLoop.p_status.state;
#endif

    /* A startup session is valid only while FOC is actually running. Abort it
     * on every stop, fault/recovery, or standby interval so the next run
     * always starts with a fresh alignment and observer warm-up. */
    if ((enableControl == 0u)
        || (enablePowerStage == 0u)
        || (FocDemoClosedLoop.p_status.state != Ifx_MS_FocSolutionF16_State_run))
    {
        /* Snapshot the startup mode while inactive. Mode 0 requests a direct
         * handoff to the estimator selected by the build after a nonzero
         * positioning interval; all other cases continue with I/f. */
        startupAlignmentDuration = Cal_FocAlignmentDuration_cnt_u16;
        rotorAlignCounter = startupAlignmentDuration;
        directClosedLoopStartPending = ((Cal_FocStartupMode_u8 == 0u)
            && (startupAlignmentDuration != 0u)) ? 1u : 0u;
        alignmentDqReference.real = Cal_FocAlignmentCurrentD_Q15_s16;
        alignmentDqReference.imag = Cal_FocAlignmentCurrentQ_Q15_s16;
        Ifx_MS_FocSolutionF16_setIToFCurrentReference(&FocDemoClosedLoop,
            alignmentDqReference);
        Ifx_MS_FocSolutionF16_enableOpenLoopDqReference(&FocDemoClosedLoop, false);
    }

    /* Start counting alignment only after the FOC core has entered run. */
    else if (FocDemoClosedLoop.p_status.actualControlMode != Ifx_MS_FocSolutionF16_ControlMode_foc)
    {
        /* V/f has no d-q current control. Keep the session armed for a later
         * FOC run rather than consuming positioning time here. */
        Ifx_MS_FocSolutionF16_enableOpenLoopDqReference(&FocDemoClosedLoop, false);
    }
#if FOC_AUX_ALGORITHMS_ENABLE
    else if (HFIPDInjection_isActive() != 0u)
    {
        /* Identification owns voltage before any alignment tick. Consume its
         * result once at 2 kHz. Invalid/disabled results never write an angle.
         * The following speed tick begins the original positioning ramp. */
        alignmentDqReference.real = 0;
        alignmentDqReference.imag = 0;
        Ifx_MS_FocSolutionF16_setOpenLoopDqReference(&FocDemoClosedLoop, alignmentDqReference);
        Ifx_MS_FocSolutionF16_enableOpenLoopDqReference(&FocDemoClosedLoop, true);
        if (HFIPDInjection_isComplete() != 0u)
        {
            uint32 initialAngle;
            if (HFIPDInjection_consume(&initialAngle) != 0u)
            {
                Ifx_MDA_IToFControllerF16_setAnglePreviousValue(
                    &FocDemoClosedLoop.iToF, initialAngle);
                FocDemoClosedLoop.angle = initialAngle;
            }
        }
    }
#endif
    else if (rotorAlignCounter != 0u)
    {
        /* Position with the original start-to-target ramp. This is the only
         * phase that overrides the I/f d-q current reference. */
        alignmentDqReference.real = FocCalculateAlignmentCurrentQ15(
            Cal_FocAlignmentCurrentStartD_Q15_s16,
            Cal_FocAlignmentCurrentD_Q15_s16,
            startupAlignmentDuration,
            rotorAlignCounter);
        alignmentDqReference.imag = FocCalculateAlignmentCurrentQ15(
            Cal_FocAlignmentCurrentStartQ_Q15_s16,
            Cal_FocAlignmentCurrentQ_Q15_s16,
            startupAlignmentDuration,
            rotorAlignCounter);
        Ifx_MS_FocSolutionF16_setOpenLoopDqReference(&FocDemoClosedLoop, alignmentDqReference);
        Ifx_MS_FocSolutionF16_enableOpenLoopDqReference(&FocDemoClosedLoop, true);
        rotorAlignCounter--;
    }
    else if (directClosedLoopStartPending != 0u)
    {
        /* Keep the final positioning vector until the compiled observer has
         * produced the valid estimate required by the common handoff API. */
        alignmentDqReference.real = Cal_FocAlignmentCurrentD_Q15_s16;
        alignmentDqReference.imag = Cal_FocAlignmentCurrentQ_Q15_s16;
        Ifx_MS_FocSolutionF16_setOpenLoopDqReference(&FocDemoClosedLoop, alignmentDqReference);
        Ifx_MS_FocSolutionF16_enableOpenLoopDqReference(&FocDemoClosedLoop, true);

        requestedSpeedQ15 = Ifx_Math_DivSat_F16(
            referenceSpeedQ0, IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM);

        if (Ifx_MS_FocSolutionF16_enterClosedLoopFromAlignment(
                &FocDemoClosedLoop, requestedSpeedQ15) == true)
        {
            directClosedLoopStartPending = 0u;
            referenceSpeedQ15 = requestedSpeedQ15;
            Ifx_MS_FocSolutionF16_enableOpenLoopDqReference(&FocDemoClosedLoop, false);
        }
    }
    else
    {
        /* Open-loop startup has completed positioning. The I/f reference was
         * loaded with the positioning target while stopped; only its speed
         * ramp, direction handling, and transition parameters apply here. */
        Ifx_MS_FocSolutionF16_enableOpenLoopDqReference(&FocDemoClosedLoop, false);
        referenceSpeedQ15 = Ifx_Math_DivSat_F16(referenceSpeedQ0,
            IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM);
    }

    /* Pass IdMap's d-axis value through the state-machine input. Writing
     * dqCommand after this call only creates a short pulse that is replaced
     * by the next speed-loop state-machine execution. */
    effectiveCurrentsDqReference.real = currentsDqReference.real;
    effectiveCurrentsDqReference.imag = currentsDqReference.imag;
    FocPrepareIdMapReference(FocDemoClosedLoop.p_output.estimatedSpeedQ15,
        &effectiveCurrentsDqReference);

    fwcVoltageSnapshot.sequence = 0u;
    fwcVoltageSnapshot.saturationStreakFast = 0u;
    fwcVoltageSnapshot.unsaturationStreakFast = 0u;
    fwcVoltageSnapshot.idAtFloorSaturationStreakFast = 0u;
    fwcVoltageSnapshot.requestedVoltageQ15 = 0;
    fwcVoltageSnapshot.actualVoltageQ15 = 0;
    fwcVoltageSnapshot.dcLinkVoltageQ15 = 0;
    fwcVoltageSnapshot.valid = 0u;
    fwcVoltageSnapshot.saturated = 0u;
    fwcSnapshotRead = Ifx_MS_FocSolutionF16_getVoltageSaturationSnapshot(
        &FocDemoClosedLoop, &msVoltageSnapshot) ? 1u : 0u;
    if (fwcSnapshotRead != 0u)
    {
        fwcVoltageSnapshot.sequence = msVoltageSnapshot.sequence;
        fwcVoltageSnapshot.saturationStreakFast =
            msVoltageSnapshot.saturationStreakFast;
        fwcVoltageSnapshot.unsaturationStreakFast =
            msVoltageSnapshot.unsaturationStreakFast;
        fwcVoltageSnapshot.idAtFloorSaturationStreakFast =
            msVoltageSnapshot.idAtFloorSaturationStreakFast;
        fwcVoltageSnapshot.requestedVoltageQ15 =
            msVoltageSnapshot.requestedVoltageQ15;
        fwcVoltageSnapshot.actualVoltageQ15 = msVoltageSnapshot.actualVoltageQ15;
        fwcVoltageSnapshot.dcLinkVoltageQ15 = msVoltageSnapshot.dcLinkVoltageQ15;
        fwcVoltageSnapshot.valid = msVoltageSnapshot.valid;
        fwcVoltageSnapshot.saturated = msVoltageSnapshot.saturated;
    }

    fwcControlEligible = (FocFwcControlIsEligible() != false) ? 1u : 0u;
    Fwc_Q15_execute(&fwcVoltageSnapshot, fwcControlEligible,
        effectiveCurrentsDqReference.real, FocDemoClosedLoop.dqCommand.imag,
        &fwcOutput);
    if ((fwcOutput.active != 0u) && (FocFwcControlIsEligible() != false))
    {
        effectiveCurrentsDqReference.real = fwcOutput.idReferenceQ15;
    }

    /* Latch a reference cap on entry and release it on every invalid/OFF edge.
     * Keep the raw command and normal ramp; do not copy speed/Iq feedback. */
    Ifx_MS_FocSolutionF16_setSpeedRecovery(&FocDemoClosedLoop,
        (fwcOutput.recoveryActive != 0u) && (FocFwcControlIsEligible() != false));

    /* Execute speed control */
    Ifx_MS_FocSolutionF16_executeSpeedControl(&FocDemoClosedLoop, referenceSpeedQ15,
        effectiveCurrentsDqReference);
#if FOC_AUX_ALGORITHMS_ENABLE
    /* The transition and parameter capture are atomic with respect to fast
     * execution: both callbacks have the same preemption priority. Begin only
     * on the stopped->run edge, before its first fast control sample. */
    if ((FocDemoClosedLoop.p_status.state != Ifx_MS_FocSolutionF16_State_run)
        || (FocDemoClosedLoop.p_status.actualControlMode != Ifx_MS_FocSolutionF16_ControlMode_foc)
        || (enableControl == 0u) || (enablePowerStage == 0u))
    {
        HFIPDInjection_cancel();
    }
    else if (stateBeforeSpeed != Ifx_MS_FocSolutionF16_State_run)
    {
        HFIPDInjection_begin();
    }
    FocApplyApsfsmTorqueCompensation();
#endif
    if ((fwcOutput.active != 0u) && (FocFwcControlIsEligible() != false))
    {
        /* IdMap runs before the speed PI and therefore only sees the prior
         * Iq. Apply the final same-cycle circle limit here with d-axis
        * priority so weak-field Id is never traded away for Iq. */
        FocDemoClosedLoop.dqCommand.real = fwcOutput.idReferenceQ15;
        Fwc_Q15_applyCurrentLimit(&FocDemoClosedLoop.dqCommand,
            fwcOutput.idReferenceQ15, 1u);
    }
    IdMap_Q15_updateIsFeedbackMeasurement(&FocDemoClosedLoop.focController.currentDQ);
    Meas_Foc_AlignRemain_tick_u16 = rotorAlignCounter;
//    Term_Printf("%d,%d,%d\r\n ",FocDemoClosedLoop.measurementADCCYT2B7.p_rawCurrentMeasurements[0]
//                ,FocDemoClosedLoop.measurementADCCYT2B7.p_rawCurrentMeasurements[1],vdc);
    /* Toggle pin to be able to measure task runtimes */
//    Cy_GPIO_Clr(GPIO_PRT20, 0u);
#if (MCU_LOAD_PROFILER_ENABLE != 0u)
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_FOC_SPD,
        speedLoopProfilerToken);
#endif
}

static void FocClearAdrcSpeedMeasurements(void)
{
    Meas_ADRC_Speed_Active_u8 = 0u;
    Meas_ADRC_Speed_Valid_u8 = 0u;
    Meas_ADRC_Speed_Status_u8 = ADRC_SPEED_CONTROLLER_STATUS_IDLE;
    Meas_ADRC_Speed_IqRef_Q15_s16 = 0;
    Meas_ADRC_Speed_ReferenceFiltered_PU_f32 = 0.0F;
    Meas_ADRC_Speed_Output_PU_f32 = 0.0F;
    Meas_ADRC_Speed_EstimatedSpeed_PU_f32 = 0.0F;
    Meas_ADRC_Speed_EstimatedAcceleration_PU_per_s_f32 = 0.0F;
    Meas_ADRC_Speed_EstimatedDisturbance_PU_per_s2_f32 = 0.0F;
}

static uint8 FocAdrcSpeedControllerExecute(void *context,
                                           const Ifx_Math_Fract16 referenceSpeedQ15,
                                           const Ifx_Math_Fract16 estimatedSpeedQ15,
                                           Ifx_Math_Fract16 *currentQReferenceQ15)
{
    ADRC_SpeedControllerCalibration calibration;
    ADRC_SpeedControllerDiagnostics diagnostics;
    uint8 valid;

    (void)context;
    calibration.criticalGain_PU_per_PU_s2 = Cal_ADRC_Speed_CriticalGain_PU_per_PU_s2_f32;
    calibration.controlBandwidth_radps = Cal_ADRC_Speed_ControlBandwidth_radps_f32;
    calibration.observerBandwidth_radps = Cal_ADRC_Speed_ObserverBandwidth_radps_f32;
    calibration.iqUpperLimitQ15 = Cal_ADRC_Speed_IqUpperLimit_Q15_s16;
    calibration.iqLowerLimitQ15 = Cal_ADRC_Speed_IqLowerLimit_Q15_s16;

    valid = ADRC_SpeedController_execute(referenceSpeedQ15, estimatedSpeedQ15,
        &calibration, currentQReferenceQ15, &diagnostics);

    Meas_ADRC_Speed_Active_u8 = 1u;
    Meas_ADRC_Speed_Valid_u8 = valid;
    Meas_ADRC_Speed_Status_u8 = diagnostics.status;
    Meas_ADRC_Speed_IqRef_Q15_s16 = *currentQReferenceQ15;
    Meas_ADRC_Speed_ReferenceFiltered_PU_f32 = diagnostics.referenceFilteredPU;
    Meas_ADRC_Speed_Output_PU_f32 = diagnostics.outputPU;
    Meas_ADRC_Speed_EstimatedSpeed_PU_f32 = diagnostics.estimatedSpeedPU;
    Meas_ADRC_Speed_EstimatedAcceleration_PU_per_s_f32 =
        diagnostics.estimatedAccelerationPU_per_s;
    Meas_ADRC_Speed_EstimatedDisturbance_PU_per_s2_f32 =
        diagnostics.estimatedDisturbancePU_per_s2;

    return valid;
}

static void FocApplyAdrcSpeedController(void)
{
    static uint8 latchedSelector = FOC_ADRC_SPEED_SELECTOR_PI;
    static uint8 previousResetRequest = 0u;
    static uint8 controllerWasActive = 0u;
    uint8 requestedSelector;
    uint8 resetRequest;
    bool focInactive;
    bool controllerActive;

    focInactive = ((FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_off)
        || (FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_standBy));

    if (focInactive)
    {
        requestedSelector = Cal_ADRC_Speed_Selector_u8;
        if (requestedSelector != FOC_ADRC_SPEED_SELECTOR_ADRC)
        {
            requestedSelector = FOC_ADRC_SPEED_SELECTOR_PI;
        }

        if (latchedSelector != requestedSelector)
        {
            latchedSelector = requestedSelector;
            ADRC_SpeedController_reset();
        }

        Ifx_MS_FocSolutionF16_enableExternalSpeedController(&FocDemoClosedLoop,
            (latchedSelector == FOC_ADRC_SPEED_SELECTOR_ADRC));
    }

    resetRequest = (Cal_ADRC_Speed_Reset_u8 != 0u) ? 1u : 0u;
    if ((resetRequest != 0u) && (previousResetRequest == 0u))
    {
        ADRC_SpeedController_reset();
    }
    previousResetRequest = resetRequest;

    controllerActive = ((latchedSelector == FOC_ADRC_SPEED_SELECTOR_ADRC)
        && (FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_run)
        && (FocDemoClosedLoop.p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
        && (FocDemoClosedLoop.p_status.subState == Ifx_MS_FocSolutionF16_SubState_closedLoop)
        && (FocDemoClosedLoop.p_enableDirectInterface == false));

    if (controllerActive == false)
    {
        if (controllerWasActive != 0u)
        {
            ADRC_SpeedController_reset();
        }

        FocClearAdrcSpeedMeasurements();
    }
    else
    {
        Meas_ADRC_Speed_Active_u8 = 1u;
    }

    controllerWasActive = (controllerActive == true) ? 1u : 0u;
}

#if FOC_AUX_ALGORITHMS_ENABLE
static bool FocApsfsmSpeedIsEligible(void)
{
    const uint16_t speedLowerLimitRpm = Cal_APSFSM_SpdLo_rpm_u16;
    const uint16_t speedUpperLimitRpm = Cal_APSFSM_SpdHi_rpm_u16;
    uint32_t speedLowerLimitQ15;
    uint32_t speedUpperLimitQ15;

    if ((speedLowerLimitRpm == 0u)
        || (speedLowerLimitRpm >= speedUpperLimitRpm)
        || (speedUpperLimitRpm
            > (uint16_t)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM))
    {
        return false;
    }

    /* Match the adapter's rounded positive-rpm to Q15 conversion so the
     * call-site qualification edge and the algorithm gate cannot disagree. */
    speedLowerLimitQ15 = ((uint32_t)speedLowerLimitRpm * 32768u)
        + ((uint32_t)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM / 2u);
    speedLowerLimitQ15 /=
        (uint32_t)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM;
    speedUpperLimitQ15 = ((uint32_t)speedUpperLimitRpm * 32768u)
        + ((uint32_t)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM / 2u);
    speedUpperLimitQ15 /=
        (uint32_t)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM;
    if (speedUpperLimitQ15 > 32767u)
    {
        speedUpperLimitQ15 = 32767u;
    }

    return ((FocDemoClosedLoop.rateLimitInSpeedQ15
                >= (Ifx_Math_Fract16)speedLowerLimitQ15)
        && (FocDemoClosedLoop.rateLimitInSpeedQ15
                <= (Ifx_Math_Fract16)speedUpperLimitQ15)
        && (FocDemoClosedLoop.p_output.estimatedSpeedQ15
                >= (Ifx_Math_Fract16)speedLowerLimitQ15)
        && (FocDemoClosedLoop.p_output.estimatedSpeedQ15
                <= (Ifx_Math_Fract16)speedUpperLimitQ15));
}

static void FocApplyApsfsmTorqueCompensation(void)
{
    static uint8_t previousMode = APSFSM_TORQUE_COMP_MODE_OFF;
    static uint8_t previousControlEligible = 0u;
    APSFSM_TorqueCompCalibration calibration;
    Ifx_Math_CmpFract16 baseDqQ15;
    Ifx_Math_CmpFract16 compensatedDqQ15;
    uint8_t requestedMode = Cal_APSFSM_Sel_u8;
    const uint8_t resetRequest = Cal_APSFSM_Rst_u8;
    uint8_t controlEligible = 0u;

    if (requestedMode > APSFSM_TORQUE_COMP_MODE_APPLY)
    {
        requestedMode = APSFSM_TORQUE_COMP_MODE_OFF;
    }

    if (requestedMode != APSFSM_TORQUE_COMP_MODE_OFF)
    {
        controlEligible =
            ((FocDemoClosedLoop.p_enablePowerStage == true)
            && (FocDemoClosedLoop.p_enableControl == true)
            && (FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_run)
            && (FocDemoClosedLoop.p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
            && (FocDemoClosedLoop.p_status.subState == Ifx_MS_FocSolutionF16_SubState_closedLoop)
            && (FocDemoClosedLoop.p_enableDirectInterface == false)
            && (FocApsfsmSpeedIsEligible() != false)) ? 1u : 0u;
    }

    /* The default-off and ineligible paths are edge driven. Speed-window
     * qualification is included here so the adapter is not reset on every
     * 2 kHz sample while waiting to enter its configured operating range. */
    if ((requestedMode == APSFSM_TORQUE_COMP_MODE_OFF)
        || (controlEligible == 0u))
    {
        if ((requestedMode != previousMode)
            || (controlEligible != previousControlEligible)
            || (resetRequest != 0u))
        {
            APSFSM_TorqueComp_reset();
        }
        if (resetRequest != 0u)
        {
            Cal_APSFSM_Rst_u8 = 0u;
        }
        previousMode = requestedMode;
        previousControlEligible = controlEligible;
        return;
    }

    if ((requestedMode != previousMode)
        || (controlEligible != previousControlEligible))
    {
        APSFSM_TorqueComp_reset();
    }

    /* Snapshot the XCP calibration before applying one eligible 2 kHz sample. */
    calibration.selector = requestedMode;
    calibration.reset = Cal_APSFSM_Rst_u8;
    calibration.kHat = Cal_APSFSM_KHat_f32;
    calibration.rho_rad = Cal_APSFSM_Rho_rad_f32;
    calibration.lambda = Cal_APSFSM_Lambda_f32;
    calibration.iqUpperLimitQ15 = Cal_APSFSM_IqHi_Q15_s16;
    calibration.iqLowerLimitQ15 = Cal_APSFSM_IqLo_Q15_s16;
    calibration.speedLowerLimit_rpm = Cal_APSFSM_SpdLo_rpm_u16;
    calibration.speedUpperLimit_rpm = Cal_APSFSM_SpdHi_rpm_u16;
    calibration.settleTime_ms = Cal_APSFSM_Settle_ms_u16;
    calibration.rampTime_ms = Cal_APSFSM_Ramp_ms_u16;

    baseDqQ15 = FocDemoClosedLoop.dqCommand;
    (void)APSFSM_TorqueComp_execute(
        FocDemoClosedLoop.rateLimitInSpeedQ15,
        FocDemoClosedLoop.p_output.estimatedSpeedQ15,
        &baseDqQ15,
        controlEligible,
        &calibration,
        &compensatedDqQ15,
        0);
    if (resetRequest != 0u)
    {
        Cal_APSFSM_Rst_u8 = 0u;
    }
    FocDemoClosedLoop.dqCommand = compensatedDqQ15;
    previousMode = requestedMode;
    previousControlEligible = controlEligible;
}

/* FWC is deliberately a bare internal-PI FOC path. It observes other
 * controllers but never changes their ownership, startup, or power-stage
 * behavior. Any enabled alternative path makes FWC report INELIGIBLE. */
#endif
static bool FocFwcControlIsEligible(void)
{
    return ((FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_run)
        && (FocDemoClosedLoop.p_status.actualControlMode
            == Ifx_MS_FocSolutionF16_ControlMode_foc)
        && (FocDemoClosedLoop.p_status.subState
            == Ifx_MS_FocSolutionF16_SubState_closedLoop)
        && (FocDemoClosedLoop.p_enablePowerStage != false)
        && (FocDemoClosedLoop.p_enableControl != false)
        && (FocDemoClosedLoop.p_enableDirectInterface == false)
        && (FocDemoClosedLoop.p_externalSpeedControllerEnabled == false)
#if FOC_AUX_ALGORITHMS_ENABLE
        && (Cal_Hfi_Enable_u8 == 0u)
        && (Cal_RRCDOB_Sel_u8 == RRCDOB_SELECTOR_OFF)
        && (Cal_VAFID_Mode_u8 == VAFID_MODE_OFF)
        && (Cal_APSFSM_Sel_u8 == APSFSM_TORQUE_COMP_MODE_OFF)
        && (Meas_APSFSM_OutAct_u8 == 0u)
#endif
        && (FocDemoClosedLoop.modulator.p_forceDutyEnable == false));
}

static bool FocIdMapControlIsActive(void)
{
    return (((FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_run)
        || (FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_rampDown))
        && (FocDemoClosedLoop.p_status.subState == Ifx_MS_FocSolutionF16_SubState_closedLoop)
        && (FocDemoClosedLoop.p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
        && (FocDemoClosedLoop.p_enableDirectInterface == false));
}

static void FocPrepareIdMapReference(const Ifx_Math_Fract16 speedQ15,
                                     Ifx_Math_CmpFract16 *currentReference)
{
    uint8 valid;

    if (FocIdMapControlIsActive() == false)
    {
        IdMap_Q15_setActive(0u);
        return;
    }

    IdMap_Q15_setActive(1u);
    valid = IdMap_Q15_computeReference(speedQ15, FocDemoClosedLoop.dqCommand.imag,
        &currentReference->real);
    if (valid == 0u)
    {
        currentReference->real = 0;
    }
}

static void FocLatchCommandAtSpeedBoundary(void)
{
    if (Cal_FocCommandSource_u8 != 0u)
    {
        enableControl = (Cal_FocEnable_u8 != 0u) ? 1u : 0u;
        enablePowerStage = enableControl;
        referenceSpeedQ0 = Cal_FocSpeedCommand_rpm_s16;
        currentsDqReference.real = Cal_FocCurrentCommandD_Q15_s16;
        currentsDqReference.imag = Cal_FocCurrentCommandQ_Q15_s16;
        controlMode = Cal_FocControlMode_u8;
        enableDirectInterface = Cal_FocDirectInterface_u8;
    }
    else
    {
        if (motor_enable_command == 1u)
        {
            enableControl = 1u;
            enablePowerStage = 1u;
        }
        else
        {
            enableControl = 0u;
            enablePowerStage = 0u;
        }

        referenceSpeedQ0 = motorspeedreferenceq10;
    }

    Ifx_MS_FocSolutionF16_enablePowerStage(&FocDemoClosedLoop,
        (enablePowerStage != 0u));
    Ifx_MS_FocSolutionF16_enableControl(&FocDemoClosedLoop,
        (enableControl != 0u));

    /* Mode and direct-interface ownership are fast-loop inputs. Latch them
     * only while inactive; start/stop and speed remain live at 2 kHz. */
    if ((FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_off)
        || (FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_standBy))
    {
        Ifx_MS_FocSolutionF16_setControlMode(&FocDemoClosedLoop,
            (Ifx_MS_FocSolutionF16_ControlMode)controlMode);
        Ifx_MS_FocSolutionF16_enableDirectInterface(&FocDemoClosedLoop,
            (bool)enableDirectInterface);
    }
}

int main(void)
{
    /* Globally enable interrupts */
    __enable_irq();

    /*****************************************************************************
    ** Initialization of the core/peripherals                                   **
    *****************************************************************************/
    SystemInit();
    task_init();

    SDL_Init();

#if (MCU_LOAD_PROFILER_ENABLE != 0u)
    McuLoadProfiler_initialize();
#else
    /* Preserve the existing fast-loop timing diagnostics in profiler-off
     * A/B builds. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0u;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
#endif
    Meas_Foc_FastLoopBudgetCycles_u32 =
        (SystemCoreClock / 1000000u) * FOC_PWM_PERIOD_US;

    focPwmPhase = 0u;

    /*****************************************************************************
    ** Initialization of the FOC application                                    **
    *****************************************************************************/
    Ifx_MS_FocSolutionF16_init(&FocDemoClosedLoop);
    FocDemoClosedLoop.dqCommand.real = 0;
    FocDemoClosedLoop.dqCommand.imag = 0;
    ADRC_SpeedController_initialize();
#if FOC_AUX_ALGORITHMS_ENABLE
    APSFSM_TorqueComp_initialize();
#endif
#if FOC_RRCDOB_ENABLE
    RrcDobCompensator_initialize();
#endif
    Ifx_MS_FocSolutionF16_setExternalSpeedControllerCallback(&FocDemoClosedLoop,
        FocAdrcSpeedControllerExecute, 0);
    IdMap_Q15_initialize();
    Fwc_Q15_initialize();
#if FOC_AUX_ALGORITHMS_ENABLE
    VAFID_initialize();
#endif
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
    ExternalObserverManager_initialize();
#endif
    /* Recalculate all fixed-point motor quantities against the configured
     * 10000 rpm speed base before either control loop can run. */
    FocApplyMotorCalibration(true);
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
    FocApplyKreParameters();
#endif
#if FOC_RRCDOB_ENABLE
    FocApplyRrcDobParameters();
#endif

    /*****************************************************************************
    ** Enable current and speed loop interrupts                                 **
    *****************************************************************************/
    NVIC_EnableIRQ(INTIDX_INPUT_GPIO);
    NVIC_EnableIRQ(INTIDX_FOC_PERIOD);
    /* Use the same preemption priority as the fast FOC IRQ. Pending
     * arbitration lets the speed loop progress without nested access to the
     * shared FOC object. */
    NVIC_SetPriority(INTIDX_SLEEPLOOP, 1u);
    NVIC_EnableIRQ(INTIDX_SLEEPLOOP);

    /*****************************************************************************
    * Main endless loop                                                         *
    *****************************************************************************/
//    Cy_Tcpwm_Pwm_SetCounter(TCPWMx_GRPx_CNTx_THICK, 1000);
//    Cy_Tcpwm_TriggerReloadOrIndex(TCPWMx_GRPx_CNTx_THICK);
//    Cy_Tcpwm_Counter_SetCompare0_Buff(TCPWMx_GRPx_CNTx_V, test[0]);
//    Cy_Tcpwm_Counter_SetCompare0_Buff(TCPWMx_GRPx_CNTx_W, test[0]);
//    Cy_Tcpwm_Counter_SetCompare1_Buff(TCPWMx_GRPx_CNTx_U, test[1]);
//    Cy_Tcpwm_Counter_SetCompare1_Buff(TCPWMx_GRPx_CNTx_V, test[1]);
//    Cy_Tcpwm_Counter_SetCompare1_Buff(TCPWMx_GRPx_CNTx_W, test[1]);
//    Cy_Tcpwm_TriggerCapture0(TCPWMx_GRPx_CNTx_THICK);
//    Cy_Tcpwm_TriggerCapture0(TCPWMx_GRPx_CNTx_V);
//    Cy_Tcpwm_TriggerCapture0(TCPWMx_GRPx_CNTx_W);
//    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_U);
//    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_V);
//    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_W);
//    Cy_Tcpwm_TriggerStart(TCPWMx_GRPx_CNTx_U);
//    Cy_Tcpwm_TriggerStart(TCPWMx_GRPx_CNTx_V);
//    Cy_Tcpwm_TriggerStart(TCPWMx_GRPx_CNTx_W);
    while (true)
    {
        /* Call clear fault API if requested */
        task_process();
        McuLoadProfiler_service();
    }
}

static uint16_t XcpCalM4LimitDuty(const uint16_t duty, const uint16_t maxDuty)
{
    return (duty > maxDuty) ? maxDuty : duty;
}

static void XcpCalM4Apply(void)
{
    static uint8_t lastApplySequence;
    const uint8_t requestSequence = XcpM4Calibration.applySequence;
    uint16_t maxDuty;

    if (requestSequence == lastApplySequence)
    {
        return;
    }

    if (FocDemoClosedLoop.modulator.p_period_tick > 0)
    {
        maxDuty = (uint16_t)FocDemoClosedLoop.modulator.p_period_tick + 1u;
    }
    else
    {
        maxDuty = 0u;
    }

    FocDemoClosedLoop.modulator.p_forceDutyEnable =
        (XcpM4Calibration.forceDutyEnable != 0u);
    FocDemoClosedLoop.modulator.p_forceDutyU =
        XcpCalM4LimitDuty(XcpM4Calibration.forceDutyU, maxDuty);
    FocDemoClosedLoop.modulator.p_forceDutyV =
        XcpCalM4LimitDuty(XcpM4Calibration.forceDutyV, maxDuty);
    FocDemoClosedLoop.modulator.p_forceDutyW =
        XcpCalM4LimitDuty(XcpM4Calibration.forceDutyW, maxDuty);

    XcpM4Calibration.appliedSequence = requestSequence;
    lastApplySequence = requestSequence;
}

static void FocApplyMotorCalibration(const bool forceApply)
{
    static uint16 lastResistance_mOhm = 500u;
    static uint16 lastDirectInductance_uH = 1300u;
    static uint16 lastQuadratureInductance_uH = 1380u;
    static uint16 lastPermanentMagnetFlux_mWb = 46u;
    static uint8 lastPolePairs = IFX_MS_FOCSOLUTIONF16_CFG_POLE_PAIRS;
    uint16 resistance_mOhm;
    uint16 directInductance_uH;
    uint16 quadratureInductance_uH;
    uint16 permanentMagnetFlux_mWb;
    uint8 polePairs;
    uint32 interruptMask;
    Ifx_MS_FocSolutionF16_MotorCoefficients prepared;

    resistance_mOhm = Cal_MotorPhaseResistance_mOhm_u16;
    directInductance_uH = Cal_MotorDirectInductance_uH_u16;
    quadratureInductance_uH = Cal_MotorQuadratureInductance_uH_u16;
    permanentMagnetFlux_mWb = Cal_MTPA_PermanentMagnetFlux_mWb_u16;
    polePairs = Cal_MotorPolePairs_u8;

    /* Detect a write during this bounded capture. This does not turn several
     * separate XCP writes into a transaction; tune coherent groups together. */
    if ((resistance_mOhm != Cal_MotorPhaseResistance_mOhm_u16)
        || (directInductance_uH != Cal_MotorDirectInductance_uH_u16)
        || (quadratureInductance_uH != Cal_MotorQuadratureInductance_uH_u16)
        || (permanentMagnetFlux_mWb != Cal_MTPA_PermanentMagnetFlux_mWb_u16)
        || (polePairs != Cal_MotorPolePairs_u8))
    {
        return;
    }

    /* A failed request must be retried even when the user restores the exact
     * previous valid values; otherwise the stale error status would hold the
     * KRE startup pending gate forever. */
    if ((forceApply == false)
        && (Cal_MotorParameterStatus_u8 == 0u)
        && (resistance_mOhm == lastResistance_mOhm)
        && (directInductance_uH == lastDirectInductance_uH)
        && (quadratureInductance_uH == lastQuadratureInductance_uH)
        && (permanentMagnetFlux_mWb == lastPermanentMagnetFlux_mWb)
        && (polePairs == lastPolePairs))
    {
        return;
    }

    /* Prepare divisions and fixed-point conversions with IRQs enabled.
     * Shared motor parameters still require representable/nonzero values;
     * these checks protect other FOC consumers, not the KRE fast step. */
    if (permanentMagnetFlux_mWb == 0u)
    {
        Cal_MotorParameterStatus_u8 = 1u;
    }
    else
    {
        Cal_MotorParameterStatus_u8 = Ifx_MS_FocSolutionF16_prepareMotorParameters(
            &prepared, resistance_mOhm, directInductance_uH,
            quadratureInductance_uH, polePairs) ? 0u : 1u;
    }

    if (Cal_MotorParameterStatus_u8 != 0u)
    {
        return;
    }
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
    if (ExternalObserverManager_stageMotorParameters(resistance_mOhm,
        directInductance_uH, quadratureInductance_uH,
        permanentMagnetFlux_mWb, polePairs) == 0u)
    {
        return;
    }
#endif

    /* Publish motor derivatives, mirrors, and the matching KRE snapshot in
     * one control-IRQ exclusion. No exp/division in the production commit. */
    interruptMask = __get_PRIMASK();
    __disable_irq();
    {
        Ifx_MS_FocSolutionF16_applyMotorParameters(&FocDemoClosedLoop, &prepared);
        lastResistance_mOhm = resistance_mOhm;
        lastDirectInductance_uH = directInductance_uH;
        lastQuadratureInductance_uH = quadratureInductance_uH;
        lastPermanentMagnetFlux_mWb = permanentMagnetFlux_mWb;
        lastPolePairs = polePairs;
        Cal_MotorAppliedPhaseResistance_mOhm_u16 = resistance_mOhm;
        Cal_MotorAppliedDirectInductance_uH_u16 = directInductance_uH;
        Cal_MotorAppliedQuadratureInductance_uH_u16 = quadratureInductance_uH;
        Cal_MTPA_AppliedPermanentMagnetFlux_mWb_u16 = permanentMagnetFlux_mWb;
        Cal_MotorAppliedPolePairs_u8 = polePairs;
        Cal_MotorAppliedFluxResistance_Q15_s16 = FocDemoClosedLoop.fluxEstimator.p_phaseResistanceQ15;
        Cal_MotorAppliedFluxInductance_Q15_s16 = FocDemoClosedLoop.fluxEstimator.p_phaseInductanceQ15;
        Cal_MotorAppliedIToFAngleIncrement_Q14_s16 = FocDemoClosedLoop.iToF.p_angleIncrementQ14;
        Cal_MotorAppliedVToFAngleIncrement_Q14_s16 = FocDemoClosedLoop.vToF.p_angleIncrementQ14;
        Cal_MotorAppliedSystemBaseTime_Q30_s32 = FocDemoClosedLoop.fluxEstimator.p_systemBaseTimeQ30;
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
        (void)ExternalObserverManager_applyPendingParameters();
#endif
    }

    if (interruptMask == 0u)
    {
        __enable_irq();
    }
}

#if (FOC_DIAG_FLUX_REFERENCE == 0u)
static void FocApplyKreParameters(void)
{
    uint32 interruptMask;
    /* All existing KRE calibrations may be tuned online. Expensive coefficient
     * preparation stays in foreground with control IRQs enabled. */
    if (ExternalObserverManager_stageParameters() == 0u)
    {
        return;
    }

    /* Foreground cannot interrupt an active fast/speed ISR. Excluding IRQs
     * during publication exposes one complete snapshot to the next ISR.
     * No math-library calls or observer resets occur under this lock. */
    interruptMask = __get_PRIMASK();
    __disable_irq();
    (void)ExternalObserverManager_applyPendingParameters();
    if (interruptMask == 0u)
    {
        __enable_irq();
    }
}
#endif

#if FOC_RRCDOB_ENABLE
static void FocApplyRrcDobParameters(void)
{
    static RRCDOB_Parameters previousParameters;
    static uint8 previousParametersValid = 0u;
    RRCDOB_Parameters parameters;
    uint8 parametersChanged;
    const uint8 resetRequest = Cal_RRCDOB_Rst_u8;
    uint32 interruptMask;

    if ((previousParametersValid != 0u)
        && (Cal_RRCDOB_Sel_u8 == RRCDOB_SELECTOR_OFF)
        && (Meas_RRCDOB_Sel_u8 == RRCDOB_SELECTOR_OFF)
        && (resetRequest == 0u))
    {
        Meas_RRCDOB_Pend_u8 = 0u;
        return;
    }

    parameters.selector = Cal_RRCDOB_Sel_u8;
    parameters.reverseOutput = Cal_RRCDOB_Rev_u8;
    parameters.leadTime_us = Cal_RRCDOB_Lead_us_u16;
    parameters.reset = resetRequest;
    parameters.resistance_mOhm = Cal_MotorAppliedPhaseResistance_mOhm_u16;
    parameters.inductanceD_uH = Cal_MotorAppliedDirectInductance_uH_u16;
    parameters.inductanceQ_uH = Cal_MotorAppliedQuadratureInductance_uH_u16;
    parameters.polePairs = Cal_MotorAppliedPolePairs_u8;
    parameters.cutoffRatioQ15 = Cal_RRCDOB_WcRatio_Q15_s16;
    parameters.outputLimitQ15 = Cal_RRCDOB_OutHi_Q15_s16;
    parameters.speedLowerLimit_rpm = Cal_RRCDOB_SpdLo_rpm_u16;
    parameters.speedUpperLimit_rpm = Cal_RRCDOB_SpdHi_rpm_u16;
    parameters.rampTime_ms = Cal_RRCDOB_Ramp_ms_u16;

    parametersChanged = ((previousParametersValid == 0u)
        || (parameters.selector != previousParameters.selector)
        || (parameters.reverseOutput != previousParameters.reverseOutput)
        || (parameters.leadTime_us != previousParameters.leadTime_us)
        || (parameters.resistance_mOhm != previousParameters.resistance_mOhm)
        || (parameters.inductanceD_uH != previousParameters.inductanceD_uH)
        || (parameters.inductanceQ_uH != previousParameters.inductanceQ_uH)
        || (parameters.polePairs != previousParameters.polePairs)
        || (parameters.cutoffRatioQ15 != previousParameters.cutoffRatioQ15)
        || (parameters.outputLimitQ15 != previousParameters.outputLimitQ15)
        || (parameters.speedLowerLimit_rpm != previousParameters.speedLowerLimit_rpm)
        || (parameters.speedUpperLimit_rpm != previousParameters.speedUpperLimit_rpm)
        || (parameters.rampTime_ms != previousParameters.rampTime_ms)) ? 1u : 0u;

    Meas_RRCDOB_Pend_u8 = ((parametersChanged != 0u) || (resetRequest != 0u)) ? 1u : 0u;
    if (Meas_RRCDOB_Pend_u8 == 0u)
    {
        return;
    }
    /* Numeric parameters and selector are a stopped-only snapshot. */
    if ((FocDemoClosedLoop.p_enableControl != false)
        || (FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_run)
        || (FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_rampDown))
    {
        return;
    }

    /* A monitor/apply ownership transition cannot reuse observer state learned
     * under the other voltage path. Force a reset even without an XCP pulse. */
    if ((previousParametersValid != 0u)
        && (parameters.selector != previousParameters.selector))
    {
        parameters.reset = 1u;
    }

    interruptMask = __get_PRIMASK();
    __disable_irq();
    if ((FocDemoClosedLoop.p_enableControl != false)
        || (FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_run)
        || (FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolutionF16_State_rampDown)
        || (parameters.selector != Cal_RRCDOB_Sel_u8)
        || (parameters.reverseOutput != Cal_RRCDOB_Rev_u8)
        || (parameters.leadTime_us != Cal_RRCDOB_Lead_us_u16)
        || (parameters.resistance_mOhm != Cal_MotorAppliedPhaseResistance_mOhm_u16)
        || (parameters.inductanceD_uH != Cal_MotorAppliedDirectInductance_uH_u16)
        || (parameters.inductanceQ_uH != Cal_MotorAppliedQuadratureInductance_uH_u16)
        || (parameters.polePairs != Cal_MotorAppliedPolePairs_u8)
        || (parameters.cutoffRatioQ15 != Cal_RRCDOB_WcRatio_Q15_s16)
        || (parameters.outputLimitQ15 != Cal_RRCDOB_OutHi_Q15_s16)
        || (parameters.speedLowerLimit_rpm != Cal_RRCDOB_SpdLo_rpm_u16)
        || (parameters.speedUpperLimit_rpm != Cal_RRCDOB_SpdHi_rpm_u16)
        || (parameters.rampTime_ms != Cal_RRCDOB_Ramp_ms_u16)
        || (resetRequest != Cal_RRCDOB_Rst_u8))
    {
        if (interruptMask == 0u) { __enable_irq(); }
        return;
    }
    (void)RrcDobCompensator_setParameters(&parameters);
    Meas_RRCDOB_Pend_u8 = 0u;
    if (resetRequest != 0u)
    {
        Cal_RRCDOB_Rst_u8 = 0u;
    }

    if (interruptMask == 0u)
    {
        __enable_irq();
    }

    previousParameters = parameters;
    previousParameters.reset = 0u;
    previousParametersValid = 1u;
}

#endif
#if FOC_AUX_ALGORITHMS_ENABLE
static void FocApplyVafidParameters(void)
{
    static uint16 previousResistance_mOhm = 0u;
    static uint16 previousDirectInductance_uH = 0u;
    static uint16 previousQuadratureInductance_uH = 0u;
    static uint16 previousPermanentMagnetFlux_mWb = 0u;
    static uint8 previousNominalValid = 0u;
    VAFID_NominalParameters nominal;
    uint16 resistance_mOhm;
    uint16 directInductance_uH;
    uint16 quadratureInductance_uH;
    uint16 permanentMagnetFlux_mWb;
    uint8 configurationAccepted;

    resistance_mOhm = Cal_MotorAppliedPhaseResistance_mOhm_u16;
    directInductance_uH = Cal_MotorAppliedDirectInductance_uH_u16;
    quadratureInductance_uH = Cal_MotorAppliedQuadratureInductance_uH_u16;
    permanentMagnetFlux_mWb = Cal_MTPA_AppliedPermanentMagnetFlux_mWb_u16;

    if ((previousNominalValid != 0u)
        && (resistance_mOhm == previousResistance_mOhm)
        && (directInductance_uH == previousDirectInductance_uH)
        && (quadratureInductance_uH == previousQuadratureInductance_uH)
        && (permanentMagnetFlux_mWb == previousPermanentMagnetFlux_mWb))
    {
        return;
    }

    nominal.sampleTime_s = (float)IFX_MS_FOCSOLUTIONF16_CFG_SAMPLING_TIME_US * 1.0e-6F;
    nominal.currentBase_A = (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_CURRENT_A;
    nominal.voltageBase_V = (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_VOLTAGE_V;
    nominal.resistance_Ohm = (float)resistance_mOhm * 1.0e-3F;
    nominal.inductanceD_H = (float)directInductance_uH * 1.0e-6F;
    nominal.inductanceQ_H = (float)quadratureInductance_uH * 1.0e-6F;
    nominal.permanentMagnetFlux_Wb = (float)permanentMagnetFlux_mWb * 1.0e-3F;

    /* configure() derives coefficients outside its internal critical section
     * and atomically commits the new snapshot. Call it only when the applied
     * nominal changes so the 20 kHz ISR is never held off by coefficient work. */
    configurationAccepted = VAFID_configure(&nominal);

    if (configurationAccepted != 0u)
    {
        previousResistance_mOhm = resistance_mOhm;
        previousDirectInductance_uH = directInductance_uH;
        previousQuadratureInductance_uH = quadratureInductance_uH;
        previousPermanentMagnetFlux_mWb = permanentMagnetFlux_mWb;
        previousNominalValid = 1u;
    }
    else
    {
        /* Retry after the operator repairs invalid nominal or VAFID
         * calibration values; never leave a failed first configuration
         * latched as if it had succeeded. */
        previousNominalValid = 0u;
    }
}

static void FocServiceVafid(void)
{
    static uint8 previousMode = 0xFFu;
    const uint8 requestedMode = Cal_VAFID_Mode_u8;

    /* Service continuously only in Shadow. OFF is serviced once on an edge
     * (or an explicit command) so its state is cleared without foreground
     * calibration scans on every idle-loop pass. */
    if ((requestedMode == VAFID_MODE_SHADOW)
        || (requestedMode != previousMode)
        || (Cal_VAFID_Rst_u8 != 0u)
        || (Cal_VAFID_Apply_u8 != 0u)
        || (Cal_VAFID_Revert_u8 != 0u))
    {
        VAFID_service();
    }
    previousMode = requestedMode;
}

#endif
void FOC_Loop_Check(void)
{
    /* 同步M0故障状态到FOC状态机 */
    FocDemoClosedLoop.p_m0FaultStatus = m4_get_data.Word.value;

    /* 处理M0发来的故障清除请求 */
    if (motor_err_Clear != 0)
    {
        Ifx_MS_FocSolutionF16_clearFault(&FocDemoClosedLoop);
        motor_err_Clear = 0;
    }

    if (clrFaultFoc == 1u)
    {
        Ifx_MS_FocSolutionF16_clearFault(&FocDemoClosedLoop);
        clrFaultFoc = 0u;
    }
    // enablePowerStage = enableControl;

    XcpCalM4Apply();
    FocApplyMotorCalibration(false);
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
    FocApplyKreParameters();
#endif
#if FOC_RRCDOB_ENABLE
    FocApplyRrcDobParameters();
#endif
#if FOC_AUX_ALGORITHMS_ENABLE
    FocApplyVafidParameters();
#endif
#if FOC_AUX_ALGORITHMS_ENABLE
    FocServiceVafid();
#endif
    FocApplyAdrcSpeedController();

    switch (execApiId)
    {
        case 8u:
            {
                execApiLastId = execApiId;
                Ifx_MHA_BridgeDrv_TLE9563_setCsaGain(&(FocDemoClosedLoop.bridgeDrvTLE9563),
                    (Ifx_MHA_BridgeDrv_TLE9563_optionCsaGain)csaGain);
                Ifx_MHA_MeasurementADC_CYT2B7_setCsaGain(&(FocDemoClosedLoop.measurementADCCYT2B7),
                    (Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain)csaGain);
                execApiId = 0u;
            } break;

        default:
            {
                /* Do nothing */
            }
    }
    Meas_Foc_IdleSvcCount_u32++;
}
/* [] END OF FILE */
