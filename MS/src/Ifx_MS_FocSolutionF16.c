#include "../../ConfigWizard/FocTiming_Cfg.h"
/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#include "Ifx_MS_FocSolutionF16.h"
#include "Ifx_MS_FocSolutionF16_Cfg.h"
#include "Ifx_MDA_FocControllerF16_Cfg.h"
#include "Ifx_MDA_IToFControllerF16_Cfg.h"
#include "../../KRE_codegen/external_observer_manager.h"
#if FOC_AUX_ALGORITHMS_ENABLE
#include "../../HFI_codegen/hfi_injection_adapter.h"
#endif
#if FOC_AUX_ALGORITHMS_ENABLE
#include "../../APSFSM_codegen/apsfsm_torque_compensation_adapter.h"
#endif
#if FOC_RRCDOB_ENABLE
#include "../../RRC_DOB_codegen/rrc_dob_compensator.h"
#endif
#if FOC_AUX_ALGORITHMS_ENABLE
#include "../../VAFID_codegen/vafid_parameter_identifier.h"
#endif
#include "../../FWC_codegen/fwc_q15_adapter.h"
#include "../../M4_BSW/SOURCE/ld_task/inc/mcu_load_profiler.h"
#include "../../Utilities/no_opt.h"
#if FOC_AUX_ALGORITHMS_ENABLE
#include "../../HFIPD_codegen/hfipd_injection.h"
#endif

#if FOC_AUX_ALGORITHMS_ENABLE && IFX_MS_FOCSOLUTIONF16_CFG_SAMPLING_TIME_US != 50
#error "HFIPD requires the validated 50 us fast-loop schedule"
#endif

#include <math.h>

#include "Ifx_Math_Abs.h"
#include "Ifx_Math_Mul.h"
#include "Ifx_Math_CartToPolar.h"
#include "Ifx_Math_Clarke.h"
#include "Ifx_Math_Park.h"
#include "Ifx_Math_PolarToCart.h"
#include "Ifx_Math_NegSat.h"
#include "Ifx_Math_Sub.h"
#include "Ifx_Math_ConvSat.h"
#include "Ifx_Math_DivShLSat.h"
#include "Ifx_Math_SubSat.h"

#if (IFX_MDA_FOCCONTROLLERF16_CFG_DQDECOUPLINGENABLE != 0)
#error "RRC-DOB requires explicit uApplied-ucc feedback when d-q decoupling is enabled"
#endif

/* Minimum speed to switch off the module */
#define IFX_MS_FOCSOLUTIONF16_MIN_SPEED                         (16383)

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
/* Feedback vector amplitude in A (not phase RMS). Zero disables the shortcut. */
NO_OPT volatile float Cal_FOC_StopIsHi_A_f32 = 25.0F;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32 Meas_Foc_AdcMissCount_u32 = 0u;

/* Macro to saturate the value with wrapping */
#define IFX_MS_FOCSOLUTIONF16_2MAX                              (-2 * IFX_MATH_FRACT16_MIN)

/* 30 degrees in Q32 */
#define IFX_MS_FOCSOLUTIONF16_PIBY6                             (IFX_MATH_PI_INDEX / 6U)

/* 90 degrees in Q32 */
#define IFX_MS_FOCSOLUTIONF16_PIBY2                             (IFX_MATH_PI_INDEX / 2U)

#define IFX_MS_FOCSOLUTIONF16_EXTERNAL_TWO_PI_RAD               (6.2831855F)
#define IFX_MS_FOCSOLUTIONF16_EXTERNAL_ANGLE_INDEX_PER_RAD      (683565312.0F)
/* Stable VAFID token for a voltage path without an inverter dead-time model. */
#define IFX_MS_FOCSOLUTIONF16_VAFID_NO_DTC_MODEL_SIGNATURE      (0x4E4F4454u)

#if (FOC_DIAG_FLUX_REFERENCE == 0u)
#define IFX_MS_FOCSOLUTIONF16_KRE_RUNTIME_ENABLED               (1u)
#else
#define IFX_MS_FOCSOLUTIONF16_KRE_RUNTIME_ENABLED               (0u)
#endif

/* Macro for transition mode when mode is direct transition */
#define IFX_MS_FOCSOLUTIONF16_TRANSITION_MODE_DIRECT_TRANSITION (0)

/* Macros to define the component ID */
#define IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTID_SOURCEID \
    ((uint8)                                           \
     Ifx_ComponentID_SourceID_infineonTechnologiesAG)

#define IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTID_LIBRARYID         ((uint16)Ifx_ComponentID_LibraryID_mctrlSolution)
#define IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTID_MODULEID          (0U)
#define IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTID_COMPONENTID1      (1U)

#define IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTID_COMPONENTID2      ((uint8)Ifx_ComponentID_ComponentID2_eco)

/* Macros to define the component version */
#define IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTVERSION_MAJOR        (1U)
#define IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTVERSION_MINOR        (2U)
#define IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTVERSION_PATCH        (0U)
#define IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTVERSION_T            (0U)
#define IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTVERSION_REV          (0U)

/* Component ID */
static const Ifx_ComponentID      Ifx_MS_FocSolution_CYT2B7_componentID = {
    .sourceID                                                             =
        IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTID_SOURCEID,
    .libraryID                                                            =
        IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTID_LIBRARYID, .moduleID        =
        IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTID_MODULEID,
    .componentID1                                                         =
        IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTID_COMPONENTID1, .componentID2 =
        IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_MS_FocSolution_CYT2B7_componentVersion = {
    .major                                                       = IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTVERSION_MAJOR,
    .minor                                                       =
        IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTVERSION_MINOR, .patch = IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTVERSION_PATCH,
    .t                                                           =
        IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTVERSION_T, .rev       = IFX_MS_FOCSOLUTION_CYT2B7_COMPONENTVERSION_REV
};

/* polyspace-begin MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */

/* Initialization functions called by Ifx_MS_FocSolutionF16_init() */
void               Ifx_MS_FocSolutionF16_initModules(Ifx_MS_FocSolutionF16* self);
static inline void Ifx_MS_FocSolutionF16_initDriveAlgo(Ifx_MS_FocSolutionF16* self);
static inline void Ifx_MS_FocSolutionF16_initSpeedPi(Ifx_MS_FocSolutionF16* self);
static inline void Ifx_MS_FocSolutionF16_initSpeedAccelerationLimiters(Ifx_MS_FocSolutionF16* self);

/* Functions called by Ifx_MS_FocSolutionF16_executeControlMode() */
static inline Ifx_MHA_MeasurementADC_CYT2B7_Output Ifx_MS_FocSolutionF16_measureAndReconstruct(
    Ifx_MS_FocSolutionF16* self);
static inline uint32                Ifx_MS_FocSolutionF16_estimatePositionAndSpeed(Ifx_MS_FocSolutionF16* self);
static bool Ifx_MS_FocSolutionF16_enterCompiledEstimatorClosedLoopFromAlignment(
    Ifx_MS_FocSolutionF16* self,
    Ifx_Math_Fract16 speedReferenceQ15);
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
static inline uint32                Ifx_MS_FocSolutionF16_externalAngleRadToIndex(float electricalAngle_rad);
static inline Ifx_Math_Fract16      Ifx_MS_FocSolutionF16_externalSpeedRpmToQ15(float mechanicalSpeed_rpm);
#endif
#if (FOC_DIAG_FLUX_REFERENCE != 0u)
static inline uint32                Ifx_MS_FocSolutionF16_executeFluxEstimator(Ifx_MS_FocSolutionF16* self);
static inline bool                  Ifx_MS_FocSolutionF16_isFluxEstimateValid(
    Ifx_MS_FocSolutionF16* self);
#endif
static inline Ifx_Math_PolarFract16 Ifx_MS_FocSolutionF16_regulationLoop(Ifx_MS_FocSolutionF16* self,
                                                                         uint32                 estimatedAngle);
static inline void Ifx_MS_FocSolutionF16_voltageGeneration(
    Ifx_MS_FocSolutionF16* self, Ifx_Math_PolarFract16 voltageCommandPolar, Ifx_MHA_MeasurementADC_CYT2B7_Output
    measurementADCOutput);
#if FOC_RRCDOB_ENABLE
static inline bool Ifx_MS_FocSolutionF16_updateRrcDobEligibility(Ifx_MS_FocSolutionF16* self,
                                                                 bool rawEligibility);
#endif
static inline void Ifx_MS_FocSolutionF16_publishVoltageSaturationSnapshot(
    Ifx_MS_FocSolutionF16 *self,
    Ifx_Math_Fract16 requestedVoltageQ15,
    Ifx_Math_Fract16 actualVoltageQ15,
    Ifx_Math_Fract16 dcLinkVoltageQ15,
    bool valid,
    bool saturated,
    bool idAtFloor);
static void Ifx_MS_FocSolutionF16_stateMachine(Ifx_MS_FocSolutionF16* self, Ifx_Math_Fract16 speedQ15, bool
                                               faultStatus, Ifx_Math_CmpFract16 currentsDqRef,
                                               Ifx_MDA_IToFControllerF16_Output
                                               iToFOutput);

/* State functions called by Ifx_MS_FocSolutionF16_stateMachine function */
static inline Ifx_MS_FocSolutionF16_State Ifx_MS_FocSolutionF16_stateInit(Ifx_MS_FocSolutionF16* self);
static inline Ifx_MS_FocSolutionF16_State Ifx_MS_FocSolutionF16_stateOff(Ifx_MS_FocSolutionF16* self, bool
                                                                         faultStatus);
static inline Ifx_MS_FocSolutionF16_State Ifx_MS_FocSolutionF16_stateStandby(Ifx_MS_FocSolutionF16* self, bool
                                                                             faultStatus);
static inline Ifx_MS_FocSolutionF16_State Ifx_MS_FocSolutionF16_stateFault(Ifx_MS_FocSolutionF16* self);

/* Functions called by Ifx_MS_FocSolutionF16_stateFault() */
/* Requests a clear faul of the underlying modules */
static inline void Ifx_MS_FocSolutionF16_clearModuleFaults(Ifx_MS_FocSolutionF16* self);
static inline void Ifx_MS_FocSolutionF16_clearModuleFaultsMHA(Ifx_MS_FocSolutionF16* self);
static inline void Ifx_MS_FocSolutionF16_clearModuleFaultsMAS(Ifx_MS_FocSolutionF16* self);

/* Returns true if any module has a clear fault pending and false otherwise */
static inline bool Ifx_MS_FocSolutionF16_anyClearFaultIsPending(Ifx_MS_FocSolutionF16* self);

/* Check if all faults of the underlying module are successfully cleared */
static inline bool                        Ifx_MS_FocSolutionF16_faultSuccessfullyCleared(Ifx_MS_FocSolutionF16* self);
/* Called only at the 2 kHz state-machine boundary during a requested stop.
 * Fast and speed callbacks share preemption priority, so this alpha/beta pair
 * is from one completed feedback sample. It also works before Park/I-f handoff
 * and in V/f, where currentDQ need not have been updated. */
static inline bool Ifx_MS_FocSolutionF16_stopCurrentExceeded(const Ifx_MS_FocSolutionF16* self)
{
    const float thresholdA = Cal_FOC_StopIsHi_A_f32;
    const float q15ToA = (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_CURRENT_A / 32768.0F;
    float alphaA;
    float betaA;

    /* Invalid/disabled or unreachable thresholds preserve the original stop.
     * Each Q15 component is at most one base current. The loose upper bound
     * also prevents overflow when squaring an arbitrary XCP float value. */
    if (!(thresholdA > 0.0F)
        || !(thresholdA <= (2.0F * (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_CURRENT_A)))
    {
        return false;
    }

    alphaA = (float)self->currentsAlphaBeta.real * q15ToA;
    betaA = (float)self->currentsAlphaBeta.imag * q15ToA;
    return (alphaA * alphaA + betaA * betaA) > (thresholdA * thresholdA);
}


static inline Ifx_MS_FocSolutionF16_State Ifx_MS_FocSolutionF16_stateRun(Ifx_MS_FocSolutionF16* self, Ifx_Math_Fract16
                                                                         speedQ15, bool faultStatus,
                                                                         Ifx_Math_CmpFract16 currentsDqRef,
                                                                         Ifx_MDA_IToFControllerF16_Output iToFOutput);
static inline Ifx_MS_FocSolutionF16_State Ifx_MS_FocSolutionF16_stateRampDown(Ifx_MS_FocSolutionF16* self, bool
                                                                              faultStatus, Ifx_Math_CmpFract16
                                                                              currentsDqRef,
                                                                              Ifx_MDA_IToFControllerF16_Output
                                                                              iToFOutput);

/* Execute the sub-state machine */
static inline void Ifx_MS_FocSolutionF16_subStateMachine(Ifx_MS_FocSolutionF16* self, Ifx_MDA_IToFControllerF16_Output
                                                         iToFOutput, Ifx_Math_CmpFract16 currentsDqRef);

/* State functions called by Ifx_MS_FocSolutionF16_subStateMachine function */
static inline Ifx_MS_FocSolutionF16_SubState Ifx_MS_FocSolutionF16_subStateOpenLoop(Ifx_MS_FocSolutionF16* self,
                                                                                    Ifx_MDA_IToFControllerF16_Output
                                                                                    iToFOutput, Ifx_Math_CmpFract16
                                                                                    currentsDqRef);
static inline Ifx_MS_FocSolutionF16_SubState Ifx_MS_FocSolutionF16_subStateClosedLoop(Ifx_MS_FocSolutionF16* self,
                                                                                      Ifx_Math_CmpFract16
                                                                                      currentsDqRef);

/* Execute I2f use the I2f angle for Park */
static inline void Ifx_MS_FocSolutionF16_openLoop(Ifx_MS_FocSolutionF16* self);

/* Closed loop implementation */
static inline void Ifx_MS_FocSolutionF16_closedLoop(Ifx_MS_FocSolutionF16* self, uint32 estimatedAngle);

/* Execute the fast loop operations with VToF */
static inline Ifx_Math_PolarFract16 Ifx_MS_FocSolutionF16_vToFLoop(Ifx_MS_FocSolutionF16* self);

/* Reset all the previous values present in the module */
static void Ifx_MS_FocSolutionF16_reset(Ifx_MS_FocSolutionF16* self);

/* Fault check for all the modules */
static bool Ifx_MS_FocSolutionF16_faultStatus(Ifx_MS_FocSolutionF16* self);

/* API to enable / disable all the modules */
static void Ifx_MS_FocSolutionF16_localEnable(Ifx_MS_FocSolutionF16* self, bool enable);

/* Enable/disable measurement ADC */
static void Ifx_MS_FocSolutionF16_enableMeasurementADC(Ifx_MS_FocSolutionF16* self, bool enable);

/* Enable/disable bridge driver */
static void Ifx_MS_FocSolutionF16_enableBridgeDrv(Ifx_MS_FocSolutionF16* self, bool enable);

/* Enable/disable pattern generator */
static void Ifx_MS_FocSolutionF16_enablePatternGen(Ifx_MS_FocSolutionF16* self, bool enable);

/* Speed and acceleration limit*/
static inline void Ifx_MS_FocSolutionF16_limitSpeed(Ifx_MS_FocSolutionF16* self, Ifx_Math_Fract16 speedQ15);

/* Check if all modules are in the on state */
static inline bool Ifx_MS_FocSolutionF16_checkModulesStateOn(Ifx_MS_FocSolutionF16* self);

/* API to calculate the ref. q current in the speed loop or set it equal to the direct interface Q ref. current*/
static inline Ifx_Math_Fract16 Ifx_MS_FocSolutionF16_calcCurrentQRef(Ifx_MS_FocSolutionF16* self, Ifx_Math_Fract16
                                                                     estimatedSpeedQ15, Ifx_Math_Fract16 currentQRef);

/* Check if all modules are in the off state */
static inline bool Ifx_MS_FocSolutionF16_checkModulesStateOff(Ifx_MS_FocSolutionF16* self);

/* API to init. the iToF angle and set the dq ref currents, based on ref. speed */
static inline void Ifx_MS_FocSolutionF16_setDqCommand(Ifx_MS_FocSolutionF16* self, Ifx_MDA_IToFControllerF16_Output
                                                      iToFOutput);

/* API to set the Q command current based on speed */
static inline void Ifx_MS_FoCSolutionF16_setQCommand(Ifx_MS_FocSolutionF16* self, Ifx_MDA_IToFControllerF16_Output
                                                     iToFOutput);

/* API to detect current Q command sign change */
static inline void Ifx_MS_FocSolutionF16_detectQCommandZeroCross(Ifx_MS_FocSolutionF16* self);

/* API to rotate dq ref. system by Pi */
static inline void Ifx_MS_FocSolutionF16_rotateDQRefSystem(Ifx_MS_FocSolutionF16* self);

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/
/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
/* Function to get the component ID */
void Ifx_MS_FocSolutionF16_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MS_FocSolution_CYT2B7_componentID;
}


/* Function to get the component version */
void Ifx_MS_FocSolutionF16_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MS_FocSolution_CYT2B7_componentVersion;
}


void Ifx_MS_FocSolutionF16_init(Ifx_MS_FocSolutionF16* self)
{
    /* Initialize modules from used libraries */
    Ifx_MS_FocSolutionF16_initModules(self);

    /* Initialize speed PI controller */
    Ifx_MS_FocSolutionF16_initSpeedPi(self);

    /* Initialize transition speeds */
    self->transitionSpeedUpQ15   = IFX_MS_FOCSOLUTIONF16_CFG_TRANSITION_SPEED_UP_Q15;
    self->transitionSpeedDownQ15 = IFX_MS_FOCSOLUTIONF16_CFG_TRANSITION_SPEED_DOWN_Q15;

    /* Initialize speed ramp up/down rates */
    self->p_speedRampUpRateOpenLoopQ30     = IFX_MS_FOCSOLUTIONF16_CFG_OPEN_LOOP_RAMP_UP_RATE_Q30;
    self->p_speedRampDownRateOpenLoopQ30   = IFX_MS_FOCSOLUTIONF16_CFG_OPEN_LOOP_RAMP_DOWN_RATE_Q30;
    self->p_speedRampUpRateClosedLoopQ30   = IFX_MS_FOCSOLUTIONF16_CFG_CLOSED_LOOP_RAMP_UP_RATE_Q30;
    self->p_speedRampDownRateClosedLoopQ30 = IFX_MS_FOCSOLUTIONF16_CFG_CLOSED_LOOP_RAMP_DOWN_RATE_Q30;

    /* Initialize speed and acceleration limiters */
    Ifx_MS_FocSolutionF16_initSpeedAccelerationLimiters(self);

    /* Initialize internal variables */
    self->p_status.state            = Ifx_MS_FocSolutionF16_State_init;
    self->p_status.subState         = Ifx_MS_FocSolutionF16_SubState_openLoop;
    self->p_enablePowerStage        = false;
    self->p_enableControl           = false;
    self->p_clearFault              = false;
    self->p_clearFaultIsRequested   = false;
    self->p_externalObserverRunning = false;
    self->p_estimatedAngle          = 0u;
    self->p_qCurrentAtTransitionQ15 = IFX_MS_FOCSOLUTIONF16_CFG_Q_CURRENT_AT_TRANSITION_Q15;
    self->p_qCommandZeroCrossing    = false;
    self->p_openLoopDqReference.real = IFX_MDA_ITOFCONTROLLERF16_CFG_REF_CURRENT_REAL_N;
    self->p_openLoopDqReference.imag = IFX_MDA_ITOFCONTROLLERF16_CFG_REF_CURRENT_IMAG_N;
    self->p_openLoopDqReferenceEnabled = false;
    self->p_directClosedLoopHandoffActive = false;
    Fwc_SpeedRecovery_reset(&self->p_speedRecovery);
    self->p_externalSpeedControllerCallback = 0;
    self->p_externalSpeedControllerContext = 0;
    self->p_externalSpeedControllerEnabled = false;
    self->p_externalSpeedControllerHandoffPending = false;
#if FOC_AUX_ALGORITHMS_ENABLE
    self->p_hfiFastPathEnabled = false;
    self->p_vafidStructuralEligible = false;
#endif
#if FOC_RRCDOB_ENABLE
    self->p_rrcDobFastEligible = false;
#endif
    self->p_voltageSaturationSnapshot.sequence = 0u;
    self->p_voltageSaturationSnapshot.saturationStreakFast = 0u;
    self->p_voltageSaturationSnapshot.unsaturationStreakFast = 0u;
    self->p_voltageSaturationSnapshot.idAtFloorSaturationStreakFast = 0u;
    self->p_voltageSaturationSnapshot.requestedVoltageQ15 = 0;
    self->p_voltageSaturationSnapshot.actualVoltageQ15 = 0;
    self->p_voltageSaturationSnapshot.dcLinkVoltageQ15 = 0;
    self->p_voltageSaturationSnapshot.valid = 0u;
    self->p_voltageSaturationSnapshot.saturated = 0u;

#if FOC_AUX_ALGORITHMS_ENABLE
    HfiInjection_init();
#endif
}

void Ifx_MS_FocSolutionF16_setExternalSpeedControllerCallback(
    Ifx_MS_FocSolutionF16* self,
    Ifx_MS_FocSolutionF16_ExternalSpeedControllerCallback callback,
    void *context)
{
    self->p_externalSpeedControllerCallback = callback;
    self->p_externalSpeedControllerContext = context;
}


bool Ifx_MS_FocSolutionF16_prepareMotorParameters(
                                              Ifx_MS_FocSolutionF16_MotorCoefficients* prepared,
                                              uint16 phaseResistance_mOhm,
                                              uint16 directInductance_uH,
                                              uint16 quadratureInductance_uH,
                                              uint8 polePairs)
{
    const uint32 baseResistance_mOhm = (uint32)(IFX_MDA_FLUXESTIMATORF16_CFG_BASE_RESISTANCE_OHM * 1000.0f);
    const uint32 baseInductance_uH = (uint32)(IFX_MDA_FLUXESTIMATORF16_CFG_BASE_INDUCTANCE_MH * 1000.0f);
    const uint32 averageInductance_uH = ((uint32)directInductance_uH + quadratureInductance_uH) / 2u;
    uint32 baseElecSpeed_radps;
    uint64 qValue;
    Ifx_Math_Fract16 phaseResistanceQ15;
    Ifx_Math_Fract16 directInductanceQ15;
    Ifx_Math_Fract16 quadratureInductanceQ15;
    Ifx_Math_Fract16 phaseInductanceQ15;
    Ifx_Math_Fract16 angleIncrementQ14;
    Ifx_Math_Fract32 systemBaseTimeQ30;

    if ((phaseResistance_mOhm == 0u)
        || (directInductance_uH == 0u)
        || (quadratureInductance_uH == 0u)
        || (polePairs == 0u)
        || (baseResistance_mOhm == 0u)
        || (baseInductance_uH == 0u))
    {
        return false;
    }

    qValue = ((uint64)phaseResistance_mOhm << 15u) / baseResistance_mOhm;

    if (qValue > 32767u)
    {
        return false;
    }

    phaseResistanceQ15 = (Ifx_Math_Fract16)qValue;
    qValue = ((uint64)directInductance_uH << 15u) / baseInductance_uH;

    if (qValue > 32767u)
    {
        return false;
    }

    directInductanceQ15 = (Ifx_Math_Fract16)qValue;
    qValue = ((uint64)quadratureInductance_uH << 15u) / baseInductance_uH;

    if (qValue > 32767u)
    {
        return false;
    }

    quadratureInductanceQ15 = (Ifx_Math_Fract16)qValue;
    qValue = ((uint64)averageInductance_uH << 15u) / baseInductance_uH;

    if (qValue > 32767u)
    {
        return false;
    }

    phaseInductanceQ15 = (Ifx_Math_Fract16)qValue;
    baseElecSpeed_radps = (uint32)(((uint64)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM * polePairs * 6283185u)
        / 60000000u);

    if (baseElecSpeed_radps == 0u)
    {
        return false;
    }

    qValue = ((uint64)IFX_MS_FOCSOLUTIONF16_CFG_SAMPLING_TIME_US * baseElecSpeed_radps * 16384u)
        / 6283185u;

    if (qValue > 32767u)
    {
        return false;
    }

    angleIncrementQ14 = (Ifx_Math_Fract16)qValue;
    systemBaseTimeQ30 = (Ifx_Math_Fract32)(((uint64)1073741824u * 6283185u)
        / ((uint64)baseElecSpeed_radps * 1000000u));

    prepared->phaseResistanceQ15 = phaseResistanceQ15;
    prepared->directInductanceQ15 = directInductanceQ15;
    prepared->quadratureInductanceQ15 = quadratureInductanceQ15;
    prepared->phaseInductanceQ15 = phaseInductanceQ15;
    prepared->phaseInductanceAdjustedQ15 = (Ifx_Math_Fract16)(
        (float)phaseInductanceQ15 * (float)IFX_MDA_FLUXESTIMATORF16_CFG_ADJUSTMENT_FACTOR);
    prepared->angleIncrementQ14 = angleIncrementQ14;
    prepared->systemBaseTimeQ30 = systemBaseTimeQ30;
    return true;
}

void Ifx_MS_FocSolutionF16_applyMotorParameters(Ifx_MS_FocSolutionF16* self,
    const Ifx_MS_FocSolutionF16_MotorCoefficients* prepared)
{
#if (FOC_DIAG_FLUX_REFERENCE != 0u)
    Ifx_MDA_FluxEstimatorF16_setMotorParameters(&self->fluxEstimator,
        prepared->phaseResistanceQ15, prepared->phaseInductanceQ15,
        prepared->systemBaseTimeQ30);
#else
    /* Preserve the public field layout and XCP-facing fixed-point mirrors
     * without linking or scheduling the Flux runtime in production. */
    self->fluxEstimator.p_phaseResistanceQ15 = prepared->phaseResistanceQ15;
    self->fluxEstimator.p_phaseInductanceQ15 = prepared->phaseInductanceQ15;
    self->fluxEstimator.p_phaseInductanceAdjustedQ15 =
        prepared->phaseInductanceAdjustedQ15;
    self->fluxEstimator.p_systemBaseTimeQ30 = prepared->systemBaseTimeQ30;
#endif
    Ifx_MDA_FocControllerF16_setInductances(&self->focController,
        prepared->directInductanceQ15, prepared->quadratureInductanceQ15);
    self->iToF.p_angleIncrementQ14 = prepared->angleIncrementQ14;
    self->vToF.p_angleIncrementQ14 = prepared->angleIncrementQ14;
}


void Ifx_MS_FocSolutionF16_executeSpeedControl(Ifx_MS_FocSolutionF16* self, Ifx_Math_Fract16 speedQ15,
                                               Ifx_Math_CmpFract16 currentsDqRef)
{
    /* Fault status */
    bool                             faultStatus = Ifx_MS_FocSolutionF16_faultStatus(self);

    /* Get current to frequency output */
    Ifx_MDA_IToFControllerF16_Output iToFOutput;
    Ifx_MDA_IToFControllerF16_getOutput(&(self->iToF), &iToFOutput);

    if (self->p_clearFault == true)
    {
        /* Request to clear the underlying modules faults no matter whether FocSolution is in fault state to clear fault
         * bits of the underlying modules */
        Ifx_MS_FocSolutionF16_clearModuleFaults(self);

        if (self->p_status.state == Ifx_MS_FocSolutionF16_State_fault)
        {
            /* Request to clear the fault state of the FocSolution */
            self->p_clearFaultIsRequested = true;
        }

        /* Reset clearFault flag */
        self->p_clearFault = false;
    }

    /* State Machine execute */
    Ifx_MS_FocSolutionF16_stateMachine(self, speedQ15, faultStatus, currentsDqRef, iToFOutput);

    /* Call Bridge Driver (fault handling) */
    Ifx_MHA_BridgeDrv_TLE9563_execute(&self->bridgeDrvTLE9563);
}


bool Ifx_MS_FocSolutionF16_getVoltageSaturationSnapshot(
    const Ifx_MS_FocSolutionF16 *self,
    Ifx_MS_FocSolutionF16_VoltageSaturationSnapshot *snapshot)
{
    uint8 attempt;

    if ((self == 0) || (snapshot == 0))
    {
        return false;
    }

    for (attempt = 0u; attempt < 2u; ++attempt)
    {
        const uint32 sequenceStart = self->p_voltageSaturationSnapshot.sequence;
        uint32 sequenceEnd;

        if ((sequenceStart & 1u) != 0u)
        {
            continue;
        }

        snapshot->saturationStreakFast =
            self->p_voltageSaturationSnapshot.saturationStreakFast;
        snapshot->unsaturationStreakFast =
            self->p_voltageSaturationSnapshot.unsaturationStreakFast;
        snapshot->idAtFloorSaturationStreakFast =
            self->p_voltageSaturationSnapshot.idAtFloorSaturationStreakFast;
        snapshot->requestedVoltageQ15 =
            self->p_voltageSaturationSnapshot.requestedVoltageQ15;
        snapshot->actualVoltageQ15 =
            self->p_voltageSaturationSnapshot.actualVoltageQ15;
        snapshot->dcLinkVoltageQ15 =
            self->p_voltageSaturationSnapshot.dcLinkVoltageQ15;
        snapshot->valid = self->p_voltageSaturationSnapshot.valid;
        snapshot->saturated = self->p_voltageSaturationSnapshot.saturated;
        sequenceEnd = self->p_voltageSaturationSnapshot.sequence;

        if ((sequenceStart == sequenceEnd)
            && ((sequenceEnd & 1u) == 0u))
        {
            snapshot->sequence = sequenceEnd;
            return true;
        }
    }

    return false;
}


void Ifx_MS_FocSolutionF16_setSpeedRecovery(
    Ifx_MS_FocSolutionF16 *self, bool active)
{
    if (self == 0)
    {
        return;
    }
    if ((self->p_status.state != Ifx_MS_FocSolutionF16_State_run)
        || (self->p_status.actualControlMode != Ifx_MS_FocSolutionF16_ControlMode_foc)
        || (self->p_status.subState != Ifx_MS_FocSolutionF16_SubState_closedLoop)
        || (self->p_enablePowerStage == false)
        || (self->p_enableControl == false)
        || (self->p_enableDirectInterface != false)
        || (self->p_externalSpeedControllerEnabled != false))
    {
        active = false;
    }

    Fwc_SpeedRecovery_setActive(&self->p_speedRecovery, active ? 1u : 0u,
        self->rateLimitInSpeedQ15, self->p_output.estimatedSpeedQ15,
        self->dqCommand.imag);
}


bool Ifx_MS_FocSolutionF16_enterClosedLoopFromAlignment(
    Ifx_MS_FocSolutionF16* self,
    Ifx_Math_Fract16 speedReferenceQ15)
{
#if (FOC_DIAG_FLUX_REFERENCE != 0u)
    /* A Flux reference build has no external validity flag. Require a
     * non-trivial reconstructed rotor-flux vector before handoff. */
    if (Ifx_MS_FocSolutionF16_isFluxEstimateValid(self) == false)
    {
        return false;
    }
#else
    float electricalAngle_rad;
    float mechanicalSpeed_rpm;

    /* KRE must already have produced a valid alignment estimate before its
     * angle can take ownership of the current controller. */
    if (ExternalObserverManager_getFocEstimate(&electricalAngle_rad, &mechanicalSpeed_rpm) == 0u)
    {
        return false;
    }
#endif

    return Ifx_MS_FocSolutionF16_enterCompiledEstimatorClosedLoopFromAlignment(
        self, speedReferenceQ15);
}


static bool Ifx_MS_FocSolutionF16_enterCompiledEstimatorClosedLoopFromAlignment(
    Ifx_MS_FocSolutionF16* self,
    const Ifx_Math_Fract16 speedReferenceQ15)
{
    Ifx_Math_Fract16 handoffQCurrentQ15 = 0;

    if ((self->p_status.state != Ifx_MS_FocSolutionF16_State_run)
        || (self->p_status.actualControlMode != Ifx_MS_FocSolutionF16_ControlMode_foc)
        || (self->p_status.subState != Ifx_MS_FocSolutionF16_SubState_openLoop))
    {
        return false;
    }

    /* Preserve the current actually applied by the positioning vector. The
     * regular I/f transition keeps its configured transition-current value. */
    if (self->p_enableDirectInterface == false)
    {
        if (self->p_openLoopDqReferenceEnabled == true)
        {
            handoffQCurrentQ15 = self->dqCommand.imag;
        }
        else if (speedReferenceQ15 > 0)
        {
            handoffQCurrentQ15 = self->p_qCurrentAtTransitionQ15;
        }
        else if (speedReferenceQ15 < 0)
        {
            handoffQCurrentQ15 = Ifx_Math_Neg_F16(self->p_qCurrentAtTransitionQ15);
        }

        Ifx_Math_PiF16_setIntegPreviousValue(&(self->speedPi), handoffQCurrentQ15);
    }
    else
    {
        handoffQCurrentQ15 = self->dqCommand.imag;
    }
    /* Positioning is a stationary-start operation. Do not let an observer
     * speed transient reverse the first limited reference after handoff. */
    Ifx_Math_AccelLimitF16_setSpeedStepPreviousValue(&(self->accelerationLimit), 0);
    Ifx_Math_AccelLimitF16_setSpeedStepUpLimit(&(self->accelerationLimit),
        self->p_speedRampUpRateClosedLoopQ30);
    Ifx_Math_AccelLimitF16_setSpeedStepDownLimit(&(self->accelerationLimit),
        self->p_speedRampDownRateClosedLoopQ30);
    self->p_previousQCommand = handoffQCurrentQ15;
    self->p_qCommandZeroCrossing = false;
    self->p_directClosedLoopHandoffActive = true;
    self->p_externalSpeedControllerHandoffPending = self->p_externalSpeedControllerEnabled;
    self->p_status.subState = Ifx_MS_FocSolutionF16_SubState_closedLoop;

    return true;
}


static inline Ifx_Math_Fract16 Ifx_MS_FocSolutionF16_calcCurrentQRef(Ifx_MS_FocSolutionF16* self, Ifx_Math_Fract16
                                                                     estimatedSpeedQ15, Ifx_Math_Fract16 currentQRef)
{
    Ifx_Math_Fract16 refCurrent;

    /* Check if the direct interface is enabled */
    if (self->p_enableDirectInterface == true)
    {
        /* Set the ref. Q current to the input ref. Q current */
        refCurrent = currentQRef;

        /* Reset speed PI if direct interface is enabled with the input ref. Q current */
        Ifx_Math_PiF16_setIntegPreviousValue(&(self->speedPi), currentQRef);
    }
    else
    {
        if ((self->p_externalSpeedControllerEnabled == true)
            && (self->p_externalSpeedControllerCallback != 0))
        {
            if (self->p_externalSpeedControllerHandoffPending == true)
            {
                refCurrent = self->p_previousQCommand;
                self->p_externalSpeedControllerHandoffPending = false;
            }
            else
            {
                refCurrent = 0;
            }

            if (self->p_externalSpeedControllerCallback(
                    self->p_externalSpeedControllerContext,
                    self->rateLimitInSpeedQ15,
                    estimatedSpeedQ15,
                    &refCurrent) == 0u)
            {
                /* An invalid external result must never revive the PI
                 * integrator or leave a stale torque request active. */
                refCurrent = 0;
            }
        }
        else
        {
            /* Speed error */
            Ifx_Math_Fract16 errSpeed = Ifx_Math_Sub_F16(Ifx_Math_ShR_F16(self->rateLimitInSpeedQ15, 1u),
                Ifx_Math_ShR_F16(estimatedSpeedQ15, 1u));
            const Ifx_Math_Fract32 integralBefore = self->speedPi.p_integPreviousValue;

            /* Execute speed PI */
            refCurrent = Ifx_Math_PiF16_execute(&(self->speedPi), errSpeed);
            if (self->p_speedRecovery.active != 0u)
            {
                /* Track only the next integrator state, with filtered Iq and
                 * bounded Kaw. Never make this pass's output copy feedback. */
                self->speedPi.p_integPreviousValue = Fwc_SpeedRecovery_trackIntegral(
                    &self->p_speedRecovery, errSpeed, refCurrent,
                    self->focController.currentDQ.imag, integralBefore,
                    self->speedPi.p_integPreviousValue,
                    self->speedPi.p_antiWindupGainSamplingTime.value,
                    (uint8)self->speedPi.p_antiWindupGainSamplingTime.qFormat,
                    self->speedPi.p_lowerLimit, self->speedPi.p_upperLimit);
            }
        }
    }

    return refCurrent;
}


/* API to execute the fast loop operations */
void Ifx_MS_FocSolutionF16_executeControlMode(Ifx_MS_FocSolutionF16* self)
{
    /* Local variables */
    Ifx_Math_PolarFract16                voltageCommandPolar;
    Ifx_MHA_MeasurementADC_CYT2B7_Output measurementADCOutput;
    uint32                               estimatedAngle;

    /* Return voltage measurement and perform current measurement and reconstruction */
    MCU_FAST_MARK(MCU_FAST_OTHER);
    measurementADCOutput = Ifx_MS_FocSolutionF16_measureAndReconstruct(self);
    MCU_FAST_MARK(MCU_FAST_ADC);

#if FOC_AUX_ALGORITHMS_ENABLE
    if ((HFIPDInjection_isActive() != 0u)
        && (self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
        && (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc))
    {
        float injectionV;
        uint32 injectionAngle;
        /* Current/voltage SI conversion belongs at this boundary. The voltage
         * budget is the SVPWM linear limit Vdc/sqrt(3). No PI/observer step is
         * run while stationary identification owns the applied voltage. */
        HFIPDInjection_execute(
            (float)self->currentsAlphaBeta.real * ((float)IFX_MS_FOCSOLUTIONF16_BASE_CURRENT_A / 32768.0F),
            (float)self->currentsAlphaBeta.imag * ((float)IFX_MS_FOCSOLUTIONF16_BASE_CURRENT_A / 32768.0F),
            (self->modulator.p_forceDutyEnable != false) ? 0.0F
                : (float)measurementADCOutput.dcLinkVoltageQ15
                    * ((float)IFX_MS_FOCSOLUTIONF16_BASE_VOLTAGE_V / 32768.0F) * 0.577350269F,
            &injectionV, &injectionAngle);
        voltageCommandPolar.amplitude = (Ifx_Math_Fract16)(fabsf(injectionV)
            * (32768.0F / (float)IFX_MS_FOCSOLUTIONF16_BASE_VOLTAGE_V));
        voltageCommandPolar.angle = injectionAngle
            + ((injectionV < 0.0F) ? 0x80000000u : 0u);
        self->dqCommand.real = 0;
        self->dqCommand.imag = 0;
        MCU_FAST_MARK(MCU_FAST_CTRL);
        Ifx_MS_FocSolutionF16_voltageGeneration(self, voltageCommandPolar, measurementADCOutput);
        return;
    }

#endif
    /* No mixed/stale tuple may advance the 100 us observer or current PI.
     * Keep the last PWM command; stop/fault state-machine work is never
     * gated by ADC readiness. Reuse KRE's existing invalid-input handling. */
    if ((measurementADCOutput.sampleValid == false)
        && (self->measurementADCCYT2B7.p_status.state == Ifx_MHA_MeasurementADC_CYT2B7_State_on)
        && ((self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
            || (self->p_status.state == Ifx_MS_FocSolutionF16_State_rampDown)))
    {
        ++Meas_Foc_AdcMissCount_u32;
#if FOC_RRCDOB_ENABLE
        if (self->p_rrcDobFastEligible != false)
        {
            RrcDobCompensator_reset();
            self->p_rrcDobFastEligible = false;
        }
#endif
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
        ExternalObserverManager_execute(NAN, NAN, NAN, NAN);
#endif
        return;
    }

    /* Exactly one selected estimator executes in the fast loop. */
    estimatedAngle = Ifx_MS_FocSolutionF16_estimatePositionAndSpeed(self);
    MCU_FAST_MARK(MCU_FAST_OBS);
    if ((self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
        || (self->p_status.state == Ifx_MS_FocSolutionF16_State_rampDown))
    {
        if (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
        {
            voltageCommandPolar = Ifx_MS_FocSolutionF16_regulationLoop(self, estimatedAngle);
        }

        /* p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_vToF */
        else
        {
            voltageCommandPolar = Ifx_MS_FocSolutionF16_vToFLoop(self);
        }
    }
    else
    {
        /* Modulator output is 0 while in standby, fault, off, startAngleIdent, init */
        voltageCommandPolar.amplitude = 0;
        voltageCommandPolar.angle     = 0;
    }

    /* Generate voltage according to the command */
    MCU_FAST_MARK(MCU_FAST_CTRL);
    Ifx_MS_FocSolutionF16_voltageGeneration(self, voltageCommandPolar, measurementADCOutput);
}


static inline void Ifx_MS_FocSolutionF16_rotateDQRefSystem(Ifx_MS_FocSolutionF16* self)
{
    /* Add 180deg. to the I2f angle */
    Ifx_MDA_IToFControllerF16_addOneEightyDegreeInAngle(&(self->iToF));

    /* Get current controllers previous value */
    Ifx_Math_Fract16 currentDPiPrevValue = Ifx_Math_PiF16_getIntegPreviousValue(&(self->focController.currentDPi));
    Ifx_Math_Fract16 currentQPiPrevValue = Ifx_Math_PiF16_getIntegPreviousValue(&(self->focController.currentQPi));

    /* Reset current PIs with negative of previous value */
    Ifx_Math_PiF16_setIntegPreviousValue(&(self->focController.currentDPi), Ifx_Math_Neg_F16(currentDPiPrevValue));
    Ifx_Math_PiF16_setIntegPreviousValue(&(self->focController.currentQPi), Ifx_Math_Neg_F16(currentQPiPrevValue));
}


static inline void Ifx_MS_FocSolutionF16_clearModuleFaults(Ifx_MS_FocSolutionF16* self)
{
    /* Clear faults of MHA modules */
    Ifx_MS_FocSolutionF16_clearModuleFaultsMHA(self);

    /* Clear faults of MAS modules */
    Ifx_MS_FocSolutionF16_clearModuleFaultsMAS(self);
}


static inline void Ifx_MS_FocSolutionF16_clearModuleFaultsMHA(Ifx_MS_FocSolutionF16* self)
{
    /* Variable declaration */
    Ifx_MHA_BridgeDrv_TLE9563_Status bridgeDrvStatus;
    Ifx_MHA_PatternGen_CYT2B7_Status patternGenStatus;

    /* Get the status of modules */
    bridgeDrvStatus  = Ifx_MHA_BridgeDrv_TLE9563_getStatus(&self->bridgeDrvTLE9563);
    patternGenStatus = Ifx_MHA_PatternGen_CYT2B7_getStatus(&self->patternGenCYT2B7);

    /* Clear faults of modules */
    if ((bridgeDrvStatus.overcurrent != false)
        || (bridgeDrvStatus.overvoltage != false)
        || (bridgeDrvStatus.undervoltage != false)
        || (bridgeDrvStatus.spiFault != false))
    {
        Ifx_MHA_BridgeDrv_TLE9563_clearFault(&self->bridgeDrvTLE9563);
    }
}


static inline void Ifx_MS_FocSolutionF16_clearModuleFaultsMAS(Ifx_MS_FocSolutionF16* self)
{
    /* Variable declaration */
    Ifx_MAS_ModulatorF16_Status modulatorStatus;

    /* Get the status of modules */
    modulatorStatus = Ifx_MAS_ModulatorF16_getStatus(&self->modulator);

    /* Clear faults of modules */
    if ((modulatorStatus.maxAmplitudeFlag != false)
        || (modulatorStatus.overmodulationFlag != false))
    {
        Ifx_MAS_ModulatorF16_clearFault(&self->modulator);
    }
}


static inline Ifx_MHA_MeasurementADC_CYT2B7_Output Ifx_MS_FocSolutionF16_measureAndReconstruct(
    Ifx_MS_FocSolutionF16* self)
{
    /* Get measured shunt currents from previous cycle */
    Ifx_MHA_MeasurementADC_CYT2B7_Output measurementADCOutput;
    Ifx_MHA_MeasurementADC_CYT2B7_execute(&(self->measurementADCCYT2B7));
    Ifx_MHA_MeasurementADC_CYT2B7_getOutput(&(self->measurementADCCYT2B7), &measurementADCOutput);

    if (measurementADCOutput.sampleValid == false)
    {
        return measurementADCOutput;
    }

    /* The held PWM cycle has completed before this 10 kHz read. Its sector
     * is the last submitted one, not the older 20 kHz pipeline sector. */
#if FOC_PWM_PER_CONTROL == 2u
    self->previousCurrentReconstructionInfo = self->p_currentReconstructionInfo;
#endif
    /* Current reconstruction */
    self->currentsUVW = Ifx_Math_CurrentReconstruction_F16(self->previousCurrentReconstructionInfo,
        measurementADCOutput.shuntCurrentsQ15);

    /* Update current reconstuction information for the next state */
    self->previousCurrentReconstructionInfo = self->p_currentReconstructionInfo;

    /* Current Clark transformation (UVW to alpha-beta) */
    self->currentsAlphaBeta = Ifx_Math_Clarke_F16(self->currentsUVW);

    /* Return the measured DC link voltage */
    return measurementADCOutput;
}


#if (FOC_DIAG_FLUX_REFERENCE == 0u)
static inline uint32 Ifx_MS_FocSolutionF16_externalAngleRadToIndex(const float electricalAngle_rad)
{
    float normalizedAngle_rad = electricalAngle_rad;

    if (normalizedAngle_rad < 0.0F)
    {
        normalizedAngle_rad += IFX_MS_FOCSOLUTIONF16_EXTERNAL_TWO_PI_RAD;
    }
    else if (normalizedAngle_rad >= IFX_MS_FOCSOLUTIONF16_EXTERNAL_TWO_PI_RAD)
    {
        normalizedAngle_rad -= IFX_MS_FOCSOLUTIONF16_EXTERNAL_TWO_PI_RAD;
    }

    return (uint32)(normalizedAngle_rad * IFX_MS_FOCSOLUTIONF16_EXTERNAL_ANGLE_INDEX_PER_RAD);
}


static inline Ifx_Math_Fract16 Ifx_MS_FocSolutionF16_externalSpeedRpmToQ15(const float mechanicalSpeed_rpm)
{
    return Ifx_Math_ConvSat_Flt32ToF16(
        mechanicalSpeed_rpm / (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM,
        Ifx_Math_FractQFormat_q15);
}
#endif


#if (FOC_DIAG_FLUX_REFERENCE != 0u)
static inline uint32 Ifx_MS_FocSolutionF16_executeFluxEstimator(Ifx_MS_FocSolutionF16* self)
{
    Ifx_MDA_FluxEstimatorF16_Output fluxEstimatorOutput;

    Ifx_MDA_FluxEstimatorF16_execute(
        &(self->fluxEstimator), self->previousVoltageAlphaBeta, self->currentsAlphaBeta);
    Ifx_MDA_FluxEstimatorF16_getOutput(&(self->fluxEstimator), &fluxEstimatorOutput);
    self->p_output.estimatedSpeedQ15 = fluxEstimatorOutput.speedQ15;

    return fluxEstimatorOutput.anglePLL;
}


static inline bool Ifx_MS_FocSolutionF16_isFluxEstimateValid(Ifx_MS_FocSolutionF16* self)
{
    Ifx_Math_CmpFract16 rotorFlux;
    Ifx_Math_PolarFract16 rotorFluxPolar;

    /* The low-pass states contain stator flux. Remove the estimated L*i term
     * to test the same rotor-flux vector used by the Flux estimator. */
    rotorFlux.real = Ifx_Math_Sub_F16(
        Ifx_Math_LowPass1stF16_getPreviousValue(&(self->fluxEstimator.p_alphaFilter)),
        Ifx_Math_Mul_F16(self->currentsAlphaBeta.real,
            self->fluxEstimator.p_phaseInductanceAdjustedQ15));
    rotorFlux.imag = Ifx_Math_Sub_F16(
        Ifx_Math_LowPass1stF16_getPreviousValue(&(self->fluxEstimator.p_betaFilter)),
        Ifx_Math_Mul_F16(self->currentsAlphaBeta.imag,
            self->fluxEstimator.p_phaseInductanceAdjustedQ15));
    rotorFluxPolar = Ifx_Math_CartToPolar_F16(rotorFlux);

    /* 0.125 PU is a fixed safety floor, not a user calibration. */
    return (rotorFluxPolar.amplitude >= (Ifx_Math_Fract16)4096);
}
#endif


static inline uint32 Ifx_MS_FocSolutionF16_estimatePositionAndSpeed(Ifx_MS_FocSolutionF16* self)
{
    uint32 estimatedAngle = 0u;

#if (FOC_DIAG_FLUX_REFERENCE != 0u)
    self->p_externalObserverRunning = false;
    estimatedAngle = Ifx_MS_FocSolutionF16_executeFluxEstimator(self);
#else
    if ((self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
        || (self->p_status.state == Ifx_MS_FocSolutionF16_State_rampDown))
    {
        float electricalAngle_rad;
        float mechanicalSpeed_rpm;

        if (self->p_externalObserverRunning == false)
        {
            ExternalObserverManager_resetWithReason(ExternalObserverResetReason_runStart);
            self->p_externalObserverRunning = true;
        }

        /* KRE owns its VI_fb delay and uses the latest actual modulator
         * voltage with this cycle's reconstructed current. */
        ExternalObserverManager_execute(
            ((float)self->voltageAlphaBeta.real
                * (float)IFX_MS_FOCSOLUTIONF16_BASE_VOLTAGE_V) / 32768.0F,
            ((float)self->voltageAlphaBeta.imag
                * (float)IFX_MS_FOCSOLUTIONF16_BASE_VOLTAGE_V) / 32768.0F,
            ((float)self->currentsAlphaBeta.real
                * (float)IFX_MS_FOCSOLUTIONF16_BASE_CURRENT_A) / 32768.0F,
            ((float)self->currentsAlphaBeta.imag
                * (float)IFX_MS_FOCSOLUTIONF16_BASE_CURRENT_A) / 32768.0F);

        if (ExternalObserverManager_getFocEstimate(&electricalAngle_rad, &mechanicalSpeed_rpm) != 0u)
        {
            estimatedAngle = Ifx_MS_FocSolutionF16_externalAngleRadToIndex(electricalAngle_rad);
            self->p_output.estimatedSpeedQ15 =
                Ifx_MS_FocSolutionF16_externalSpeedRpmToQ15(mechanicalSpeed_rpm);
        }
        else
        {
            /* Hold the last accepted KRE angle and speed on a transient
             * invalid sample. Clearing speed here would create a false speed
             * error and a corresponding Iq step while the Park angle is held.
             * Flux is not linked or scheduled in this production build. */
            estimatedAngle = self->p_estimatedAngle;
        }
    }
    else
    {
        if (self->p_externalObserverRunning == true)
        {
            ExternalObserverManager_resetWithReason(ExternalObserverResetReason_runStop);
            self->p_externalObserverRunning = false;
        }
        self->p_output.estimatedSpeedQ15 = 0;
    }
#endif

    self->previousVoltageAlphaBeta = self->voltageAlphaBeta;
    self->p_estimatedAngle = estimatedAngle;

    return estimatedAngle;
}


#if FOC_RRCDOB_ENABLE
static inline bool Ifx_MS_FocSolutionF16_updateRrcDobEligibility(Ifx_MS_FocSolutionF16* self,
                                                                 bool rawEligibility)
{
    const uint8 selector = Meas_RRCDOB_Sel_u8;
    const bool selectorEnabled = ((selector > RRCDOB_SELECTOR_OFF)
                                  && (selector <= RRCDOB_SELECTOR_APPLY));
    const bool eligible = ((rawEligibility != false) && (selectorEnabled != false));

    if (eligible == false)
    {
        if (self->p_rrcDobFastEligible != false)
        {
            RrcDobCompensator_reset();
            self->p_rrcDobFastEligible = false;
        }

        return false;
    }

    self->p_rrcDobFastEligible = true;
    return true;
}


#endif
/* Execute the fast loop operations with IToF and FOC */
static inline Ifx_Math_PolarFract16 Ifx_MS_FocSolutionF16_regulationLoop(Ifx_MS_FocSolutionF16* self, uint32
                                                                         estimatedAngle)
{
    Ifx_MDA_FocControllerF16_Output focControllerOutput;
    Ifx_Math_PolarFract16 voltageCommandPolar;
    Ifx_Math_CmpFract16 effectiveDqCommand;
#if FOC_RRCDOB_ENABLE
    Ifx_Math_CmpFract16 compensatedVoltageDQ;
    RRCDOB_Output rrcDobOutput;
#endif
#if FOC_RRCDOB_ENABLE
    /* OFF has no algorithm call, calibration scan or repeated reset. */
    const bool rrcDobEligible = ((Meas_RRCDOB_Sel_u8 != RRCDOB_SELECTOR_OFF)
        || (self->p_rrcDobFastEligible != false))
        && Ifx_MS_FocSolutionF16_updateRrcDobEligibility(self,
            ((self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
                || (self->p_status.state == Ifx_MS_FocSolutionF16_State_rampDown))
            && (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
            && (self->p_status.subState == Ifx_MS_FocSolutionF16_SubState_closedLoop)
#if FOC_AUX_ALGORITHMS_ENABLE
            && (Cal_Hfi_Enable_u8 == 0u)
#endif
            && (self->modulator.p_forceDutyEnable == false));
#endif
#if FOC_AUX_ALGORITHMS_ENABLE
    VAFID_DqQ15 baseDqCommand;
    VAFID_DqQ15 probedDqCommand;
    VAFID_CurrentLimits vafidCurrentLimits;
    const bool hfiEnabled = (Cal_Hfi_Enable_u8 != 0u);
    const bool vafidShadow = (Cal_VAFID_Mode_u8 == VAFID_MODE_SHADOW);
    const bool hfiRequested = ((hfiEnabled != false)
                               && ((self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
                                   || (self->p_status.state == Ifx_MS_FocSolutionF16_State_rampDown))
                               && (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
                               && (IFX_MS_FOCSOLUTIONF16_KRE_RUNTIME_ENABLED != 0u));
    const bool vafidStructuralEligible = ((vafidShadow != false)
        && (self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
        && (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
        && (self->p_status.subState == Ifx_MS_FocSolutionF16_SubState_closedLoop)
        && (self->p_enableDirectInterface == false)
        && (self->p_externalObserverRunning != false)
        && (IFX_MS_FOCSOLUTIONF16_KRE_RUNTIME_ENABLED != 0u)
        && (Cal_APSFSM_Sel_u8 == APSFSM_TORQUE_COMP_MODE_OFF)
        && (Meas_APSFSM_OutAct_u8 == 0u)
        && (hfiRequested == false)
        && ((hfiEnabled == false) || (HfiInjection_isOutputActive() == false))
        && (Cal_RRCDOB_Sel_u8 == RRCDOB_SELECTOR_OFF)
        && (self->modulator.p_forceDutyEnable == false));

#endif
    /* Check if the Q command changes sign */
    if (self->p_qCommandZeroCrossing == true)
    {
        /* Rotate ref. DQ system if Q command changes sign */
        Ifx_MS_FocSolutionF16_rotateDQRefSystem(self);

        /* Set the current zero cross flag to false */
        self->p_qCommandZeroCrossing = false;
    }

    if (self->p_status.subState == Ifx_MS_FocSolutionF16_SubState_closedLoop)
    {
        /* Set the angle for Park transformation from FE */
        Ifx_MS_FocSolutionF16_closedLoop(self, estimatedAngle);
    }
    else
    {
        /* Execute iToF and set the angle for Park transformation from iToF */
        Ifx_MS_FocSolutionF16_openLoop(self);
    }

#if FOC_AUX_ALGORITHMS_ENABLE
    if (vafidStructuralEligible != false)
    {
        self->p_vafidStructuralEligible = true;
    }
    else
    {
        if (self->p_vafidStructuralEligible != false)
        {
            VAFID_abortFast(VAFID_REJECT_ELIGIBILITY);
        }
        self->p_vafidStructuralEligible = false;
    }

#endif
    effectiveDqCommand = self->dqCommand;
#if FOC_AUX_ALGORITHMS_ENABLE
    if (vafidStructuralEligible != false)
    {
        /* Add the identification probe only to this fast-loop copy. The
         * speed/torque/reference-owned command is never rewritten by VAFID. */
        baseDqCommand.d = effectiveDqCommand.real;
        baseDqCommand.q = effectiveDqCommand.imag;
        vafidCurrentLimits.dLowerQ15 = (-32767 - 1);
        vafidCurrentLimits.dUpperQ15 = 32767;
        vafidCurrentLimits.qLowerQ15 = IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_OUT_LOW_LIMIT_Q;
        vafidCurrentLimits.qUpperQ15 = IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_OUT_UPP_LIMIT_Q;
        vafidCurrentLimits.currentMagnitudeMaxQ15 = 32767u;
        VAFID_applyProbe(&baseDqCommand, &vafidCurrentLimits, &probedDqCommand);
        effectiveDqCommand.real = probedDqCommand.d;
        effectiveDqCommand.imag = probedDqCommand.q;
    }

#endif
    /* Field Oriented Controller */
    Ifx_MDA_FocControllerF16_execute(&(self->focController), self->currentsAlphaBeta, effectiveDqCommand, self->angle,
        self->rateLimitInSpeedQ15);
    Ifx_MDA_FocControllerF16_getOutput(&(self->focController), &focControllerOutput);

    /* Preserve the original controller output bit-for-bit unless RRC-DOB has
     * explicit ownership. HFI and RRC-DOB are mutually exclusive because the
     * injected carrier is not part of the RRC-DOB plant-input contract. */
    voltageCommandPolar = focControllerOutput.voltageCommandPolar;
#if FOC_RRCDOB_ENABLE
    if (rrcDobEligible != false)
    {
        (void)RrcDobCompensator_execute(self->focController.voltageDQ,
            self->focController.currentDQ, self->angle, &compensatedVoltageDQ);
        RrcDobCompensator_getOutput(&rrcDobOutput);

        if (rrcDobOutput.outputActive != 0u)
        {
            voltageCommandPolar = Ifx_Math_CartToPolar_F16(compensatedVoltageDQ);
            voltageCommandPolar.angle += self->angle;
        }
    }

#endif
    return voltageCommandPolar;
}


/* Open loop implementation */
static inline void Ifx_MS_FocSolutionF16_openLoop(Ifx_MS_FocSolutionF16* self)
{
    /* Execute I/f block to generate the angle */
    Ifx_MDA_IToFControllerF16_execute(&(self->iToF), self->rateLimitInSpeedQ15);
    Ifx_MDA_IToFControllerF16_Output iToFOutput;
    Ifx_MDA_IToFControllerF16_getOutput(&(self->iToF), &iToFOutput);

    /* Angle is set by iToF */
    self->angle = iToFOutput.currentVecAngle_rad;
}


/* Closed loop implementation */
static inline void Ifx_MS_FocSolutionF16_closedLoop(Ifx_MS_FocSolutionF16* self, uint32 estimatedAngle)
{
    /* Get the angle from the active estimator. */
    self->angle = estimatedAngle;
}


/* Execute the fast loop operations with VToF */
static inline Ifx_Math_PolarFract16 Ifx_MS_FocSolutionF16_vToFLoop(Ifx_MS_FocSolutionF16* self)
{
    Ifx_MDA_VToFControllerF16_Output vToFControllerOutput;
    Ifx_MDA_VToFControllerF16_execute(&(self->vToF), self->rateLimitInSpeedQ15);
    Ifx_MDA_VToFControllerF16_getOutput(&(self->vToF), &vToFControllerOutput);

    return vToFControllerOutput.voltageVector;
}


static inline void Ifx_MS_FocSolutionF16_voltageGeneration(Ifx_MS_FocSolutionF16* self, Ifx_Math_PolarFract16
                                                           voltageCommandPolar, Ifx_MHA_MeasurementADC_CYT2B7_Output
                                                           measurementADCOutput)
{
    /* Private variable to store the modulator output */
    Ifx_MAS_ModulatorF16_Output modulatorOutput;
    Ifx_MAS_ModulatorF16_Status modulatorStatus;
#if FOC_RRCDOB_ENABLE
    /* OFF has no algorithm call, calibration scan or repeated reset. */
    const bool rrcDobEligible = ((Meas_RRCDOB_Sel_u8 != RRCDOB_SELECTOR_OFF)
        || (self->p_rrcDobFastEligible != false))
        && Ifx_MS_FocSolutionF16_updateRrcDobEligibility(self,
            ((self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
                || (self->p_status.state == Ifx_MS_FocSolutionF16_State_rampDown))
            && (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
            && (self->p_status.subState == Ifx_MS_FocSolutionF16_SubState_closedLoop)
#if FOC_AUX_ALGORITHMS_ENABLE
            && (Cal_Hfi_Enable_u8 == 0u)
#endif
            && (self->modulator.p_forceDutyEnable == false));
#endif
#if FOC_AUX_ALGORITHMS_ENABLE && FOC_RRCDOB_ENABLE
    RRCDOB_Output rrcDobOutput;
#endif
#if FOC_AUX_ALGORITHMS_ENABLE
    VAFID_FastEligibility vafidEligibility;
    float voltageQ15ToVolts;
    const bool hfiEnabled = (Cal_Hfi_Enable_u8 != 0u);
    const bool vafidShadow = (Cal_VAFID_Mode_u8 == VAFID_MODE_SHADOW);
    const bool hfiRequested = ((hfiEnabled != false)
                               && ((self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
                                   || (self->p_status.state == Ifx_MS_FocSolutionF16_State_rampDown))
                               && (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
                               && (IFX_MS_FOCSOLUTIONF16_KRE_RUNTIME_ENABLED != 0u));
    const bool vafidStructuralStillEligible = ((vafidShadow != false)
        && (self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
        && (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
        && (self->p_status.subState == Ifx_MS_FocSolutionF16_SubState_closedLoop)
        && (self->p_enableDirectInterface == false)
        && (self->p_externalObserverRunning != false)
        && (IFX_MS_FOCSOLUTIONF16_KRE_RUNTIME_ENABLED != 0u)
        && (Cal_APSFSM_Sel_u8 == APSFSM_TORQUE_COMP_MODE_OFF)
        && (Meas_APSFSM_OutAct_u8 == 0u)
        && (hfiRequested == false)
        && ((hfiEnabled == false) || (HfiInjection_isOutputActive() == false))
        && (Cal_RRCDOB_Sel_u8 == RRCDOB_SELECTOR_OFF)
        && (self->modulator.p_forceDutyEnable == false));
    bool kreFocAllowed = ((HFIPDInjection_isActive() == 0u)
                           && ((self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
                           || (self->p_status.state == Ifx_MS_FocSolutionF16_State_rampDown))
                           && (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
                           && (IFX_MS_FOCSOLUTIONF16_KRE_RUNTIME_ENABLED != 0u));

    if ((self->p_vafidStructuralEligible != false)
        && (vafidStructuralStillEligible == false))
    {
        VAFID_abortFast(VAFID_REJECT_ELIGIBILITY);
        self->p_vafidStructuralEligible = false;
    }

    if (rrcDobEligible != false)
    {
        RrcDobCompensator_getOutput(&rrcDobOutput);
        if (rrcDobOutput.outputActive != 0u)
        {
            /* Close the sub-sample calibration race in favour of the voltage
             * owner already selected by regulationLoop(). */
            kreFocAllowed = false;
        }
    }

    if (hfiEnabled != false)
    {
        self->p_hfiFastPathEnabled = true;
        voltageCommandPolar = HfiInjection_applyPolar(voltageCommandPolar, kreFocAllowed);
    }
    else if (self->p_hfiFastPathEnabled != false)
    {
        HfiInjection_reset();
        self->p_hfiFastPathEnabled = false;
    }

#endif
    Fwc_Q15_FastConfig fwcFastConfig;

    /* Call modulator */
    MCU_FAST_MARK(MCU_FAST_VPRE);
    /* Ensure to call this function in all states, as otherwise clearfault will not be handled */
    Ifx_MAS_ModulatorF16_execute(&(self->modulator), voltageCommandPolar, measurementADCOutput.dcLinkVoltageQ15);
    Ifx_MAS_ModulatorF16_getOutput(&(self->modulator), &modulatorOutput);
    modulatorStatus = Ifx_MAS_ModulatorF16_getStatus(&(self->modulator));
    MCU_FAST_MARK(MCU_FAST_MOD);

    {
        const uint8 fwcFastConfigValid = Fwc_Q15_getFastConfig(&fwcFastConfig);
        const bool bareFocPath = ((fwcFastConfigValid != 0u)
            && (fwcFastConfig.enabled != 0u)
            && (self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
            && (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
            && (self->p_status.subState == Ifx_MS_FocSolutionF16_SubState_closedLoop)
            && (self->p_enablePowerStage != false)
            && (self->p_enableControl != false)
            && (self->p_enableDirectInterface == false)
            && (self->p_externalSpeedControllerEnabled == false)
#if FOC_AUX_ALGORITHMS_ENABLE
            && (Cal_Hfi_Enable_u8 == 0u)
            && (Cal_RRCDOB_Sel_u8 == RRCDOB_SELECTOR_OFF)
            && (Cal_VAFID_Mode_u8 == VAFID_MODE_OFF)
            && (Cal_APSFSM_Sel_u8 == APSFSM_TORQUE_COMP_MODE_OFF)
            && (Meas_APSFSM_OutAct_u8 == 0u)
#endif
            && (self->modulator.p_forceDutyEnable == false)
            && (modulatorStatus.state == Ifx_MAS_ModulatorF16_State_on));
        const bool snapshotValid = ((modulatorStatus.state == Ifx_MAS_ModulatorF16_State_on)
            && (measurementADCOutput.dcLinkVoltageQ15 > 0)
            && (self->modulator.p_forceDutyEnable == false));
        bool voltageLimited = false;
        bool voltageSaturated = false;

        if ((bareFocPath != false)
            && (voltageCommandPolar.amplitude > 0)
            && (modulatorOutput.actualVoltage.amplitude >= 0)
            && (voltageCommandPolar.amplitude
                > modulatorOutput.actualVoltage.amplitude))
        {
            const uint32_t voltageGapQ15 = (uint32_t)((int32_t)voltageCommandPolar.amplitude
                - (int32_t)modulatorOutput.actualVoltage.amplitude);
            const uint32_t voltageUtilGapNumerator = voltageGapQ15
                * (uint32_t)FWC_Q15_SQRT3_Q15;
            const uint32_t saturationEpsilonNumerator =
                (uint32_t)fwcFastConfig.saturationEpsilonQ15
                * (uint32_t)measurementADCOutput.dcLinkVoltageQ15;

            voltageLimited = true;
            /* Compare sqrt(3) * (Vreq - Vact) / Vdc to the PU threshold
             * without a division in the 20 kHz loop. */
            if (voltageUtilGapNumerator > saturationEpsilonNumerator)
            {
                voltageSaturated = true;
            }
        }

        Ifx_MS_FocSolutionF16_publishVoltageSaturationSnapshot(self,
            voltageCommandPolar.amplitude,
            modulatorOutput.actualVoltage.amplitude,
            measurementADCOutput.dcLinkVoltageQ15,
            snapshotValid,
            voltageSaturated,
            ((bareFocPath != false) && (fwcFastConfig.idAtFloor != 0u)));

        /* For a bare FOC sample the modulator preserves the vector angle and
         * only limits amplitude. Therefore the applied d/q voltage is exactly
         * the controller d/q voltage multiplied by actual/requested. */
        if ((bareFocPath != false)
            && (voltageLimited != false)
            && (voltageCommandPolar.amplitude > 0))
        {
            uint32_t appliedScaleQ15 = (((uint32_t)modulatorOutput.actualVoltage.amplitude
                * 32768u) + ((uint32_t)voltageCommandPolar.amplitude / 2u))
                / (uint32_t)voltageCommandPolar.amplitude;

            if (appliedScaleQ15 > 32768u)
            {
                appliedScaleQ15 = 32768u;
            }
            Ifx_MDA_FocControllerF16_trackAppliedVoltageScale(&(self->focController),
                (uint16)appliedScaleQ15, fwcFastConfig.currentAwGainQ15);
            Meas_FWC_CurAw_Act_u8 = 1u;
        }
        else
        {
            Meas_FWC_CurAw_Act_u8 = 0u;
        }
    }

    /* Store current reconstruction information */
    self->p_currentReconstructionInfo = modulatorOutput.currentReconstructionInfo;

    /* Convert actual voltage from modulator to cartesian, to be used by the flux estimator */
    self->voltageAlphaBeta = Ifx_Math_PolarToCart_F16(modulatorOutput.actualVoltage);

#if FOC_AUX_ALGORITHMS_ENABLE
    /* Capture only a real, enabled PWM sample. The eligibility gate excludes
     * forced duty because that mode overrides compare values after computing
     * actualVoltage. The modulator state is checked because actualVoltage is
     * also computed before the normal state-machine reaction. */
    if ((vafidShadow != false)
        && (self->p_vafidStructuralEligible != false))
    {
        /* Qualify the exact completed PWM path for the next identification
         * sample. The legacy dead-time model is not linked into this firmware;
         * VAFID therefore uses a stable no-model token and zero voltage-error
         * input. This keeps VAFID shadow-only but reduces identification
         * accuracy until an RRC-DOB-derived voltage model is validated. */
        vafidEligibility.kreClosedLoop =
            ((self->p_status.state == Ifx_MS_FocSolutionF16_State_run)
            && (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
            && (self->p_status.subState == Ifx_MS_FocSolutionF16_SubState_closedLoop)
            && (self->p_enableDirectInterface == false)
            && (self->p_externalObserverRunning != false)
            && (IFX_MS_FOCSOLUTIONF16_KRE_RUNTIME_ENABLED != 0u)) ? 1u : 0u;
        vafidEligibility.apsfsmOff =
            ((Cal_APSFSM_Sel_u8 == APSFSM_TORQUE_COMP_MODE_OFF)
            && (Meas_APSFSM_OutAct_u8 == 0u)) ? 1u : 0u;
        vafidEligibility.hfiOff = ((hfiEnabled == false)
            || (HfiInjection_isOutputActive() == false)) ? 1u : 0u;
        vafidEligibility.rrcOutputInactive =
            (Cal_RRCDOB_Sel_u8 == RRCDOB_SELECTOR_OFF) ? 1u : 0u;
        vafidEligibility.overmodulationActive =
            ((modulatorStatus.overmodulationFlag != false)
            || (modulatorStatus.maxAmplitudeFlag != false)) ? 1u : 0u;
        vafidEligibility.voltagePathStable =
            ((vafidEligibility.kreClosedLoop != 0u)
            && (modulatorStatus.state == Ifx_MAS_ModulatorF16_State_on)
            && (self->modulator.p_forceDutyEnable == false)
            && (vafidEligibility.overmodulationActive == 0u)) ? 1u : 0u;
        vafidEligibility.deadTimeConfigSignature =
            IFX_MS_FOCSOLUTIONF16_VAFID_NO_DTC_MODEL_SIGNATURE;
        vafidEligibility.adcSampleOffsetTicks = modulatorOutput.triggerTime_tick[0];
        VAFID_setFastEligibility(&vafidEligibility);

        /* actualVoltage is the ideal PWM command. With the legacy model
         * excluded, the estimated dead-time error is explicitly zero; KRE
         * continues to receive self->voltageAlphaBeta unchanged. */
        voltageQ15ToVolts = (float)IFX_MS_FOCSOLUTIONF16_BASE_VOLTAGE_V / 32768.0F;
        VAFID_captureMotorVoltage(
            (float)self->voltageAlphaBeta.real * voltageQ15ToVolts,
            (float)self->voltageAlphaBeta.imag * voltageQ15ToVolts,
            0.0F,
            0.0F,
            vafidEligibility.overmodulationActive);
    }
#endif
    /* Call pattern generator */
    MCU_FAST_MARK(MCU_FAST_VPOST);
    /* Ensure to call this function in all states, as otherwise clearfault will not be handled */
    Ifx_MHA_PatternGen_CYT2B7_execute(&(self->patternGenCYT2B7), modulatorOutput.compareValues_tick,
        modulatorOutput.triggerTime_tick);
    MCU_FAST_MARK(MCU_FAST_PWM);
    /* Capture after PWM submission; consume on the next control sample. */
#if FOC_RRCDOB_ENABLE
    if (rrcDobEligible != false)
    {
        modulatorStatus = Ifx_MAS_ModulatorF16_getStatus(&(self->modulator));
        if ((modulatorStatus.state == Ifx_MAS_ModulatorF16_State_on)
            && (Ifx_MHA_PatternGen_CYT2B7_getStatus(&self->patternGenCYT2B7).state
                == Ifx_MHA_PatternGen_CYT2B7_State_on))
        {
            RrcDobCompensator_captureAppliedVoltage(self->voltageAlphaBeta, self->angle);
        }
    }
#endif
}


void Ifx_MS_FocSolutionF16_initModules(Ifx_MS_FocSolutionF16* self)
{
    /* Initialize hardware abstraction modules */
    Ifx_MHA_MeasurementADC_CYT2B7_init(&(self->measurementADCCYT2B7));
//    Ifx_MHA_BridgeDrv_TLE9563_init(&(self->bridgeDrvTLE9563));
    Ifx_MHA_PatternGen_CYT2B7_init(&(self->patternGenCYT2B7));

    /* Initialize building blocks and drive algorithm modules */
    Ifx_MAS_ModulatorF16_init(&(self->modulator));
    Ifx_MS_FocSolutionF16_initDriveAlgo(self);
}


static inline void Ifx_MS_FocSolutionF16_initDriveAlgo(Ifx_MS_FocSolutionF16* self)
{
#if (FOC_DIAG_FLUX_REFERENCE != 0u)
    Ifx_MDA_FluxEstimatorF16_init(&(self->fluxEstimator));
#else
    static const Ifx_MDA_FluxEstimatorF16 fluxEstimatorPlaceholder = {0};

    self->fluxEstimator = fluxEstimatorPlaceholder;
    self->fluxEstimator.p_samplingTime_us = IFX_MS_FOCSOLUTIONF16_CFG_SAMPLING_TIME_US;
    self->fluxEstimator.p_phaseResistanceQ15 = IFX_MDA_FLUXESTIMATORF16_CFG_PHASE_RES_Q15;
    self->fluxEstimator.p_phaseInductanceQ15 = IFX_MDA_FLUXESTIMATORF16_CFG_PHASE_IND_Q15;
    self->fluxEstimator.p_phaseInductanceAdjustedQ15 =
        (Ifx_Math_Fract16)((float)IFX_MDA_FLUXESTIMATORF16_CFG_PHASE_IND_Q15
        * (float)IFX_MDA_FLUXESTIMATORF16_CFG_ADJUSTMENT_FACTOR);
    self->fluxEstimator.p_systemBaseTimeQ30 = IFX_MDA_FLUXESTIMATORF16_CFG_SYSTEM_BASE_TIME_Q30;
#endif
    Ifx_MDA_IToFControllerF16_init(&(self->iToF));
    Ifx_MDA_FocControllerF16_init(&(self->focController));
    Ifx_MDA_VToFControllerF16_init(&(self->vToF));
}


static inline void Ifx_MS_FocSolutionF16_initSpeedAccelerationLimiters(Ifx_MS_FocSolutionF16* self)
{
    /* Speed limiter */
    Ifx_Math_LimitF16_setLowerLimit(&(self->speedLimit), IFX_MS_FOCSOLUTIONF16_CFG_MINIMUM_SPEED_Q15);
    Ifx_Math_LimitF16_setUpperLimit(&(self->speedLimit), IFX_MS_FOCSOLUTIONF16_CFG_MAXIMUM_SPEED_Q15);

    /* Acceleration limiter */
    Ifx_Math_AccelLimitF16_init(&(self->accelerationLimit));
    Ifx_Math_AccelLimitF16_setSpeedStepUpLimit(&(self->accelerationLimit), self->p_speedRampUpRateOpenLoopQ30);
    Ifx_Math_AccelLimitF16_setSpeedStepDownLimit(&(self->accelerationLimit), self->p_speedRampDownRateOpenLoopQ30);
}


static inline void Ifx_MS_FocSolutionF16_initSpeedPi(Ifx_MS_FocSolutionF16* self)
{
    /* Set Q formats */
    Ifx_Math_PiF16_Qformats piCtrlSpeedQform;
    piCtrlSpeedQform.qFormatPropGain                   =
        (Ifx_Math_FractQFormat)IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_PROPGAIN_Q_FORMAT;
    piCtrlSpeedQform.qFormatIntegGainSamplingTime      =
        (Ifx_Math_FractQFormat)IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_KI_TS_Q_FORMAT;
    piCtrlSpeedQform.qFormatAntiWindupGainSamplingTime =
        (Ifx_Math_FractQFormat)IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_KAW_TS_Q_FORMAT;
    piCtrlSpeedQform.qFormatOutput                     =
        (Ifx_Math_FractQFormat)IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_LIMIT_Q_FORMAT;
    piCtrlSpeedQform.qFormatError                      = Ifx_Math_FractQFormat_q14;

    /* Call init */
    Ifx_Math_PiF16_init(&(self->speedPi), piCtrlSpeedQform);

    /* Call setters */
    Ifx_Math_PiF16_setPropGain(&(self->speedPi), IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_PROPGAIN_Q);
    Ifx_Math_PiF16_setIntegGainSamplingTime(&(self->speedPi), IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_KI_TS_Q);
    Ifx_Math_PiF16_setAntiWindupGainSamplingTime(&(self->speedPi), IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_KAW_TS_Q);
    Ifx_Math_PiF16_setUpperLimit(&(self->speedPi), IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_OUT_UPP_LIMIT_Q);
    Ifx_Math_PiF16_setLowerLimit(&(self->speedPi), IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_OUT_LOW_LIMIT_Q);
}


static void Ifx_MS_FocSolutionF16_enableMeasurementADC(Ifx_MS_FocSolutionF16* self, bool enable)
{
    /* Variables to store the status of the pattern generator and bridge driver */
    Ifx_MHA_BridgeDrv_TLE9563_Status bridgeDrvStatus;
    Ifx_MHA_PatternGen_CYT2B7_Status patternGenStatus;

    /* Get the bridge driver status */
    //bridgeDrvStatus = Ifx_MHA_BridgeDrv_TLE9563_getStatus(&(self->bridgeDrvTLE9563));

    /* Get the pattern generator status */
    patternGenStatus = Ifx_MHA_PatternGen_CYT2B7_getStatus(&self->patternGenCYT2B7);

    if (enable == true)
    {
        Ifx_MHA_MeasurementADC_CYT2B7_enable(&(self->measurementADCCYT2B7), true);
    }
    else if ((bridgeDrvStatus.state == Ifx_MHA_BridgeDrv_TLE9563_State_off)
             && (patternGenStatus.state == Ifx_MHA_PatternGen_CYT2B7_State_off))
    {
        Ifx_MHA_MeasurementADC_CYT2B7_enable(&(self->measurementADCCYT2B7), false);
    }
    else
    {
        /* Do nothing */
    }
}


static void Ifx_MS_FocSolutionF16_enableBridgeDrv(Ifx_MS_FocSolutionF16* self, bool enable)
{
    /* Variable to store the status of bridge driver */
    Ifx_MHA_PatternGen_CYT2B7_Status patternGenStatus;

    /* Get the pattern generator status */
    patternGenStatus = Ifx_MHA_PatternGen_CYT2B7_getStatus(&self->patternGenCYT2B7);

    if ((enable == false)
        && (patternGenStatus.state == Ifx_MHA_PatternGen_CYT2B7_State_off))
    {
        Ifx_MHA_BridgeDrv_TLE9563_enable(&(self->bridgeDrvTLE9563), false);
    }
    else if ((enable == true)
             && (patternGenStatus.state == Ifx_MHA_PatternGen_CYT2B7_State_on))
    {
        Ifx_MHA_BridgeDrv_TLE9563_enable(&(self->bridgeDrvTLE9563), true);
    }
    else
    {
        /* Do nothing */
    }
}


static void Ifx_MS_FocSolutionF16_enablePatternGen(Ifx_MS_FocSolutionF16* self, bool enable)
{
    /* Variables to store the status of the measurement ADC*/
    Ifx_MHA_MeasurementADC_CYT2B7_Status measurementADCStatus;

    /* Get the measurement ADC status */
    measurementADCStatus = Ifx_MHA_MeasurementADC_CYT2B7_getStatus(&self->measurementADCCYT2B7);

    if ((enable == true)
        && (measurementADCStatus.state == Ifx_MHA_MeasurementADC_CYT2B7_State_on))
    {
        Ifx_MHA_PatternGen_CYT2B7_enable(&(self->patternGenCYT2B7), true);
    }
    else if (enable == false)
    {
        Ifx_MHA_PatternGen_CYT2B7_enable(&(self->patternGenCYT2B7), false);
    }
    else
    {
        /* Do nothing */
    }
}


static void Ifx_MS_FocSolutionF16_localEnable(Ifx_MS_FocSolutionF16* self, bool enable)
{
    /* Enable/disable measurement ADC */
    Ifx_MS_FocSolutionF16_enableMeasurementADC(self, enable);

    /* Enable/disable pattern generator */
    Ifx_MS_FocSolutionF16_enablePatternGen(self, enable);

    /* Enable/disable bridge driver */
    Ifx_MS_FocSolutionF16_enableBridgeDrv(self, enable);

    /* Enable/disable modulator */
    Ifx_MAS_ModulatorF16_enable(&(self->modulator), enable);
}


static inline bool Ifx_MS_FocSolutionF16_checkModulesStateOn(Ifx_MS_FocSolutionF16* self)
{
    /* Variable declaration */
    Ifx_MHA_MeasurementADC_CYT2B7_Status measurementADCstatus;
    Ifx_MHA_BridgeDrv_TLE9563_Status     bridgeDrvStatus;
    Ifx_MHA_PatternGen_CYT2B7_Status     patternGenStatus;
    Ifx_MAS_ModulatorF16_Status          modulatorStatus;
    bool                                 returnValue;

    /* Get the status of all modules */
    measurementADCstatus = Ifx_MHA_MeasurementADC_CYT2B7_getStatus(&self->measurementADCCYT2B7);
    bridgeDrvStatus      = Ifx_MHA_BridgeDrv_TLE9563_getStatus(&self->bridgeDrvTLE9563);
    patternGenStatus     = Ifx_MHA_PatternGen_CYT2B7_getStatus(&self->patternGenCYT2B7);
    modulatorStatus      = Ifx_MAS_ModulatorF16_getStatus(&self->modulator);

    /* Check if all modules are in the on state */
    if ((measurementADCstatus.state == Ifx_MHA_MeasurementADC_CYT2B7_State_on)
//        && (bridgeDrvStatus.state == Ifx_MHA_BridgeDrv_TLE9563_State_on)
        && (patternGenStatus.state == Ifx_MHA_PatternGen_CYT2B7_State_on)
        && (modulatorStatus.state == Ifx_MAS_ModulatorF16_State_on))
    {
        returnValue = true;
    }
    else
    {
        returnValue = false;
    }

    return returnValue;
}


static inline bool Ifx_MS_FocSolutionF16_checkModulesStateOff(Ifx_MS_FocSolutionF16* self)
{
    /* Variable declaration */
    Ifx_MHA_MeasurementADC_CYT2B7_Status measurementADCstatus;
    Ifx_MHA_BridgeDrv_TLE9563_Status     bridgeDrvStatus;
    Ifx_MHA_PatternGen_CYT2B7_Status     patternGenStatus;
    Ifx_MAS_ModulatorF16_Status          modulatorStatus;
    bool                                 returnValue;

    /* Get the status of all modules */
    measurementADCstatus = Ifx_MHA_MeasurementADC_CYT2B7_getStatus(&self->measurementADCCYT2B7);
    bridgeDrvStatus      = Ifx_MHA_BridgeDrv_TLE9563_getStatus(&self->bridgeDrvTLE9563);
    patternGenStatus     = Ifx_MHA_PatternGen_CYT2B7_getStatus(&self->patternGenCYT2B7);
    modulatorStatus      = Ifx_MAS_ModulatorF16_getStatus(&self->modulator);

    /* Check if all modules are in the off state */
    if ((measurementADCstatus.state == Ifx_MHA_MeasurementADC_CYT2B7_State_off)
//        && (bridgeDrvStatus.state == Ifx_MHA_BridgeDrv_TLE9563_State_off)if(1)
        && (patternGenStatus.state == Ifx_MHA_PatternGen_CYT2B7_State_off)
        && (modulatorStatus.state == Ifx_MAS_ModulatorF16_State_off))
    {
        returnValue = true;
    }
    else
    {
        returnValue = false;
    }

    return returnValue;
}


/* API to set the fault status of the all used modules */
/* fault check for all the modules */
extern uint8_t IPMFAULT_STATE;
static bool Ifx_MS_FocSolutionF16_faultStatus(Ifx_MS_FocSolutionF16* self)
{
    /* local variable to check fault status */
    bool faultStatusRet = false;

    /* Bridge driver fault */
    //if (Ifx_MHA_BridgeDrv_TLE9563_getStatus(&(self->bridgeDrvTLE9563)).state == Ifx_MHA_BridgeDrv_TLE9563_State_fault || IPMFAULT_STATE == 1)
    if (IPMFAULT_STATE == 1)

    {
        faultStatusRet = true;
    }
    else if (self->p_m0FaultStatus != 0)
    {
        faultStatusRet = true;
    }
    // /* Pattern generator fault */
    // else if (Ifx_MHA_PatternGen_CYT2B7_getStatus(&(self->patternGenCYT2B7)).state ==
    //          Ifx_MHA_PatternGen_CYT2B7_State_fault)
    // {
    //     faultStatusRet = true;
    // }

    // /* Modulator fault */
    // else if (Ifx_MAS_ModulatorF16_getStatus(&(self->modulator)).state == Ifx_MAS_ModulatorF16_State_fault)
    // {
    //     faultStatusRet = true;
    // }
    else
    {
        faultStatusRet = false;
    }

    return faultStatusRet;
}


/* FOC state machine*/
static inline void Ifx_MS_FocSolutionF16_stateMachine(Ifx_MS_FocSolutionF16* self, Ifx_Math_Fract16 speedQ15, bool
                                                      faultStatus, Ifx_Math_CmpFract16 currentsDqRef,
                                                      Ifx_MDA_IToFControllerF16_Output iToFOutput)
{
    /* Variable to store the state */
    Ifx_MS_FocSolutionF16_State previousState = self->p_status.state;
    Ifx_MS_FocSolutionF16_State nextState;

    switch (previousState)
    {
        /* Initialize the module */
        case Ifx_MS_FocSolutionF16_State_init:
            nextState = Ifx_MS_FocSolutionF16_stateInit(self);
            break;

        /* All the modules are enabled */
        case Ifx_MS_FocSolutionF16_State_standBy:
            nextState = Ifx_MS_FocSolutionF16_stateStandby(self, faultStatus);
            break;

        /* Run state */
        case Ifx_MS_FocSolutionF16_State_run:
            nextState = Ifx_MS_FocSolutionF16_stateRun(self, speedQ15, faultStatus, currentsDqRef, iToFOutput);
            break;

        /* FOC not running */
        case Ifx_MS_FocSolutionF16_State_off:
            nextState = Ifx_MS_FocSolutionF16_stateOff(self, faultStatus);
            break;

        /* FOC is in fault */
        case Ifx_MS_FocSolutionF16_State_fault:
            nextState = Ifx_MS_FocSolutionF16_stateFault(self);
            break;

        /*  FOC is in Ramp down state (Open loop I2f) */
        case Ifx_MS_FocSolutionF16_State_rampDown:
            nextState = Ifx_MS_FocSolutionF16_stateRampDown(self, faultStatus, currentsDqRef, iToFOutput);
            break;

        /* do default transition to INIT */
        default:
            nextState = Ifx_MS_FocSolutionF16_State_init;
            break;
    }

    self->p_status.state = nextState;
}


static inline Ifx_MS_FocSolutionF16_State Ifx_MS_FocSolutionF16_stateInit(Ifx_MS_FocSolutionF16* self)
{
    Ifx_MS_FocSolutionF16_State nextState;

    /* Check if all the modules are in OFF state */
    if (Ifx_MS_FocSolutionF16_checkModulesStateOff(self) == true)
//    if(1)
    {
        /* Set the state to OFF */
        nextState = Ifx_MS_FocSolutionF16_State_off;
    }
    else
    {
        nextState = Ifx_MS_FocSolutionF16_State_init;
    }

    return nextState;
}


static inline Ifx_MS_FocSolutionF16_State Ifx_MS_FocSolutionF16_stateOff(Ifx_MS_FocSolutionF16* self, bool
                                                                         faultStatus)
{
    Ifx_MS_FocSolutionF16_State nextState;

    /* Reset previous values */
    Ifx_MS_FocSolutionF16_reset(self);

    if (faultStatus == true)
    {
        nextState = Ifx_MS_FocSolutionF16_State_fault;
    }

    /* Check if power stage is enabled */
    else if (self->p_enablePowerStage == true)
    {
        /* Enable underlying modules */
        Ifx_MS_FocSolutionF16_localEnable(self, true);

        /* Check if underlying modules are in the on state */
        if (Ifx_MS_FocSolutionF16_checkModulesStateOn(self) == true)
        {
            nextState = Ifx_MS_FocSolutionF16_State_standBy;
        }
        else
        {
            nextState = Ifx_MS_FocSolutionF16_State_off;
        }
    }
    else
    {
        nextState = Ifx_MS_FocSolutionF16_State_off;
    }

    return nextState;
}


static inline Ifx_MS_FocSolutionF16_State Ifx_MS_FocSolutionF16_stateStandby(Ifx_MS_FocSolutionF16* self, bool
                                                                             faultStatus)
{
    Ifx_MS_FocSolutionF16_State nextState;

    if (faultStatus == true)
    {
        /* Go into fault state */
        nextState = Ifx_MS_FocSolutionF16_State_fault;
    }
    else if (self->p_enablePowerStage == false)
    {
        /* Disable all underlying modules */
        Ifx_MS_FocSolutionF16_localEnable(self, false);

        /* Check if underlying modules are in the off state */
        if (Ifx_MS_FocSolutionF16_checkModulesStateOff(self) == true)
        {
            nextState = Ifx_MS_FocSolutionF16_State_off;
        }
        else
        {
            nextState = Ifx_MS_FocSolutionF16_State_standBy;
        }
    }
    else if (self->p_enableControl == true)
    {
        /* Update control mode */
        self->p_status.actualControlMode = Ifx_MS_FocSolutionF16_getControlMode(self);

        /* Set open loop ramp up/down rates*/
        Ifx_Math_AccelLimitF16_setSpeedStepUpLimit(&(self->accelerationLimit), self->p_speedRampUpRateOpenLoopQ30);
        Ifx_Math_AccelLimitF16_setSpeedStepDownLimit(&(self->accelerationLimit),
            self->p_speedRampDownRateOpenLoopQ30);

        /* Init. substate machine */
        self->p_status.subState = Ifx_MS_FocSolutionF16_SubState_openLoop;

        /* Go to run state */
        nextState = Ifx_MS_FocSolutionF16_State_run;
    }
    else
    {
        nextState = Ifx_MS_FocSolutionF16_State_standBy;
    }

    /* Always reset FocCtrl module in standby */
    Ifx_MS_FocSolutionF16_reset(self);

    return nextState;
}


static inline Ifx_MS_FocSolutionF16_State Ifx_MS_FocSolutionF16_stateRun(Ifx_MS_FocSolutionF16* self, Ifx_Math_Fract16
                                                                         speedQ15, bool faultStatus,
                                                                         Ifx_Math_CmpFract16 currentsDqRef,
                                                                         Ifx_MDA_IToFControllerF16_Output iToFOutput)
{
    Ifx_MS_FocSolutionF16_State nextState;

    /* Perform speed and acceleration limit */
    Ifx_MS_FocSolutionF16_limitSpeed(self, speedQ15);

    if (faultStatus == true)
    {
        /* Set the module to fault */
        self->p_directClosedLoopHandoffActive = false;
        nextState = Ifx_MS_FocSolutionF16_State_fault;
    }
    else if ((self->p_enableControl == false)
             || (self->p_enablePowerStage == false))
    {
        /* A high-load requested stop bypasses the speed ramp immediately. */
        self->p_directClosedLoopHandoffActive = false;
        nextState = (
#if FOC_AUX_ALGORITHMS_ENABLE
            (HFIPDInjection_isActive() != 0u) ||
#endif
            Ifx_MS_FocSolutionF16_stopCurrentExceeded(self))
            ? Ifx_MS_FocSolutionF16_State_standBy
            : Ifx_MS_FocSolutionF16_State_rampDown;
    }
    else
    {
        /* Execute sub-state machine */
        Ifx_MS_FocSolutionF16_subStateMachine(self, iToFOutput, currentsDqRef);

        /* Stay in run */
        nextState = Ifx_MS_FocSolutionF16_State_run;
    }

    return nextState;
}


static inline void Ifx_MS_FocSolutionF16_limitSpeed(Ifx_MS_FocSolutionF16* self, Ifx_Math_Fract16 speedQ15)
{
    /* Value limited speed */
    Ifx_Math_Fract16 limSpeed;

    /* Limit input speed */
    limSpeed = Ifx_Math_LimitF16_execute(&(self->speedLimit), speedQ15);
    limSpeed = Fwc_SpeedRecovery_limitReference(&self->p_speedRecovery, limSpeed);

    /* Execute accel limited */
    self->rateLimitInSpeedQ15 = Ifx_Math_AccelLimitF16_execute(&(self->accelerationLimit), limSpeed);
}


static inline void Ifx_MS_FocSolutionF16_subStateMachine(Ifx_MS_FocSolutionF16* self, Ifx_MDA_IToFControllerF16_Output
                                                         iToFOutput, Ifx_Math_CmpFract16 currentsDqRef)
{
    /* Variable to store the state */
    Ifx_MS_FocSolutionF16_SubState previousSubState = self->p_status.subState;
    Ifx_MS_FocSolutionF16_SubState nextSubState;

    switch (previousSubState)
    {
        /* Sub state machine run is in open loop */
        case Ifx_MS_FocSolutionF16_SubState_openLoop:
            nextSubState = Ifx_MS_FocSolutionF16_subStateOpenLoop(self, iToFOutput, currentsDqRef);
            break;

        /* Sub state machine run is in closed loop */
        case Ifx_MS_FocSolutionF16_SubState_closedLoop:
            nextSubState = Ifx_MS_FocSolutionF16_subStateClosedLoop(self, currentsDqRef);
            break;

        /* do default transition to open loop */
        default:
            nextSubState = Ifx_MS_FocSolutionF16_SubState_openLoop;
            break;
    }

    self->p_status.subState = nextSubState;
}


static inline Ifx_MS_FocSolutionF16_SubState Ifx_MS_FocSolutionF16_subStateOpenLoop(Ifx_MS_FocSolutionF16* self,
                                                                                    Ifx_MDA_IToFControllerF16_Output
                                                                                    iToFOutput, Ifx_Math_CmpFract16
                                                                                    currentsDqRef)
{
    Ifx_MS_FocSolutionF16_SubState nextSubState;

    /* Check if the direct current interface is enabled or disabled */
    if (self->p_openLoopDqReferenceEnabled == true)
    {
        Ifx_MS_FocSolutionF16_setDqCommand(self, iToFOutput);
    }
    else if (self->p_enableDirectInterface == true)
    {
        self->dqCommand.real = currentsDqRef.real;
        self->dqCommand.imag = currentsDqRef.imag;
    }
    else
    {
        /* Set dq ref current and iToF angle */
        Ifx_MS_FocSolutionF16_setDqCommand(self, iToFOutput);
    }

    /* Check the ref. speed to go to transition up or stay in open loop */
    if ((Ifx_Math_Abs_F16(self->rateLimitInSpeedQ15) > self->transitionSpeedUpQ15)
        && (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc))
    {
#if (IFX_MS_FOCSOLUTIONF16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTIONF16_TRANSITION_MODE_DIRECT_TRANSITION)
#if (FOC_DIAG_FLUX_REFERENCE != 0u)
        bool observerEstimateValid = Ifx_MS_FocSolutionF16_isFluxEstimateValid(self);
#else
        float electricalAngle_rad;
        float mechanicalSpeed_rpm;
        bool observerEstimateValid = (ExternalObserverManager_getFocEstimate(
            &electricalAngle_rad, &mechanicalSpeed_rpm) != 0u);
#endif

        if (observerEstimateValid == true)
        {

        /* Set the integral previous value to the configured transition q current */
        if (self->rateLimitInSpeedQ15 > 0)
        {
            Ifx_Math_PiF16_setIntegPreviousValue(&(self->speedPi), self->p_qCurrentAtTransitionQ15);
        }
        else
        {
            Ifx_Math_PiF16_setIntegPreviousValue(&(self->speedPi), Ifx_Math_Neg_F16(self->p_qCurrentAtTransitionQ15));
        }

        /* Set closed loop  ramp up/down rates*/
        Ifx_Math_AccelLimitF16_setSpeedStepUpLimit(&(self->accelerationLimit), self->p_speedRampUpRateClosedLoopQ30);
        Ifx_Math_AccelLimitF16_setSpeedStepDownLimit(&(self->accelerationLimit),
            self->p_speedRampDownRateClosedLoopQ30);

        self->p_externalSpeedControllerHandoffPending = self->p_externalSpeedControllerEnabled;

        /* Go to closed loop */
        nextSubState = Ifx_MS_FocSolutionF16_SubState_closedLoop;
        }
        else
        {
            nextSubState = Ifx_MS_FocSolutionF16_SubState_openLoop;
        }
#endif
    }
    else
    {
        /* Stay in open loop */
        nextSubState = Ifx_MS_FocSolutionF16_SubState_openLoop;
    }

    return nextSubState;
}


static inline void Ifx_MS_FocSolutionF16_setDqCommand(Ifx_MS_FocSolutionF16* self, Ifx_MDA_IToFControllerF16_Output
                                                      iToFOutput)
{
    if (self->p_openLoopDqReferenceEnabled == true)
    {
        /* Startup calibration owns both axes while I/f owns Park. The Q
         * reference is signed and must not be flipped by ramp direction. */
        self->dqCommand.real = self->p_openLoopDqReference.real;
        self->dqCommand.imag = self->p_openLoopDqReference.imag;

        /* The calibration is a signed d-q vector in the fixed startup frame.
         * Do not apply the normal I/f Q-sign convention, which rotates Park
         * by 180 degrees and would invert a deliberately negative Q command. */
        self->p_qCommandZeroCrossing = false;
    }
    else
    {
        /* Set ref. d current to I/f. */
        self->dqCommand.real = iToFOutput.refCurrent.real;

        /* Set ref. q current to I/f depending on current and previous reference speed. */
        Ifx_MS_FoCSolutionF16_setQCommand(self, iToFOutput);
    }

    /* Update previous ref. current */
    self->p_previousQCommand = self->dqCommand.imag;
}


static inline void Ifx_MS_FoCSolutionF16_setQCommand(Ifx_MS_FocSolutionF16* self, Ifx_MDA_IToFControllerF16_Output
                                                     iToFOutput)
{
    /* effective Q command current */
    Ifx_Math_Fract16 effectiveRefQCurrent;
    effectiveRefQCurrent = iToFOutput.refCurrent.imag;

    /* Set ref. q current to I2f depending on current and previous reference speed */
    if (self->rateLimitInSpeedQ15 >= 0)
    {
        self->dqCommand.imag = effectiveRefQCurrent;
    }
    else
    {
        self->dqCommand.imag = Ifx_Math_Neg_F16(effectiveRefQCurrent);
    }

    /* Detect if the current Q command changed sign */
    Ifx_MS_FocSolutionF16_detectQCommandZeroCross(self);
}


static inline void Ifx_MS_FocSolutionF16_detectQCommandZeroCross(Ifx_MS_FocSolutionF16* self)
{
    if (self->p_previousQCommand < 0)
    {
        if (self->dqCommand.imag > 0)
        {
            self->p_qCommandZeroCrossing = true;
        }
    }

    /* self->p_previousQCommand >= 0 */
    else
    {
        if (self->dqCommand.imag < 0)
        {
            self->p_qCommandZeroCrossing = true;
        }
    }
}


static inline Ifx_MS_FocSolutionF16_SubState Ifx_MS_FocSolutionF16_subStateClosedLoop(Ifx_MS_FocSolutionF16* self,
                                                                                      Ifx_Math_CmpFract16
                                                                                      currentsDqRef)
{
    Ifx_MS_FocSolutionF16_SubState  nextSubState;
    Ifx_Math_Fract16                estimatedSpeedQ15 = self->p_output.estimatedSpeedQ15;

    /* Set the direct real reference current (ready for field weakening) */
    self->dqCommand.real = currentsDqRef.real;

    /* Calculate ref. Q current */
    self->dqCommand.imag = Ifx_MS_FocSolutionF16_calcCurrentQRef(self, estimatedSpeedQ15, currentsDqRef.imag);

#if (FOC_DIAG_FLUX_REFERENCE == 0u)
    /* I/f is selected only as an explicit startup route. Once KRE has passed
     * the handoff gate, keep KRE closed-loop ownership; a later invalid sample
     * is handled in the fast loop by holding the last accepted KRE angle and
     * must not silently change the configured startup route. */
    nextSubState = Ifx_MS_FocSolutionF16_SubState_closedLoop;
#else
    if ((self->p_directClosedLoopHandoffActive == true)
             && (self->p_status.actualControlMode == Ifx_MS_FocSolutionF16_ControlMode_foc)
             && (Ifx_MS_FocSolutionF16_isFluxEstimateValid(self) == true))
    {
        /* Flux normally transitions down at low speed. Keep it in closed loop
         * only for the current run after an explicit alignment handoff. */
        nextSubState = Ifx_MS_FocSolutionF16_SubState_closedLoop;
    }
    else if (Ifx_Math_Abs_F16(estimatedSpeedQ15) <= self->transitionSpeedDownQ15)
    {
        /* Reset the I/f angle to the latest selected-estimator angle. */
        Ifx_MDA_IToFControllerF16_setAnglePreviousValue(&(self->iToF), self->p_estimatedAngle);

        /* Reset the acceleration limit state to the last estimated speed */
        Ifx_Math_AccelLimitF16_setSpeedStepPreviousValue(&(self->accelerationLimit), estimatedSpeedQ15);

        /* Set open loop ramp up/down rates*/
        Ifx_Math_AccelLimitF16_setSpeedStepUpLimit(&(self->accelerationLimit), self->p_speedRampUpRateOpenLoopQ30);
        Ifx_Math_AccelLimitF16_setSpeedStepDownLimit(&(self->accelerationLimit),
            self->p_speedRampDownRateOpenLoopQ30);

        /* Store last ref. q current to calculate the slope */
        self->p_previousQCommand = self->dqCommand.imag;

#if (IFX_MS_FOCSOLUTIONF16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTIONF16_TRANSITION_MODE_DIRECT_TRANSITION)

        /* Go to transition down */
        nextSubState = Ifx_MS_FocSolutionF16_SubState_openLoop;
#endif
    }
    else
    {
        /* Stay in closed loop */
        nextSubState = Ifx_MS_FocSolutionF16_SubState_closedLoop;
    }
#endif

    return nextSubState;
}


static inline Ifx_MS_FocSolutionF16_State Ifx_MS_FocSolutionF16_stateFault(Ifx_MS_FocSolutionF16* self)
{
    Ifx_MS_FocSolutionF16_State nextState;
    bool                        faultCleared;

    self->p_directClosedLoopHandoffActive = false;

    /* Auto-recovery: if M0 fault word cleared, auto-trigger clear fault process */
    if (self->p_clearFaultIsRequested == false)
    {
        if (self->p_m0FaultStatus == 0)
        {
            Ifx_MS_FocSolutionF16_clearModuleFaults(self);
            self->p_clearFaultIsRequested = true;
        }
    }

    /* Check whether clear fault state is requested */
    if (self->p_clearFaultIsRequested == true)
    {
        /* Check whether all faults in the underlying modules have been successfully cleared */
        faultCleared = Ifx_MS_FocSolutionF16_faultSuccessfullyCleared(self);

        if (faultCleared == true)
        {
            /* Disable underlying modules and go to off state */
            Ifx_MS_FocSolutionF16_localEnable(self, false);
            nextState = Ifx_MS_FocSolutionF16_State_off;
        }
        else
        {
            /* Fault of an underlying module has not been successfully cleared, stay in fault state */
            nextState = Ifx_MS_FocSolutionF16_State_fault;
        }
    }
    else
    {
        /* No clear fault requested, stay in fault state */
        nextState = Ifx_MS_FocSolutionF16_State_fault;
    }

    return nextState;
}


static inline Ifx_MS_FocSolutionF16_State Ifx_MS_FocSolutionF16_stateRampDown(Ifx_MS_FocSolutionF16* self, bool
                                                                              faultStatus, Ifx_Math_CmpFract16
                                                                              currentsDqRef,
                                                                              Ifx_MDA_IToFControllerF16_Output
                                                                              iToFOutput)
{
    Ifx_MS_FocSolutionF16_State nextState;

    self->p_directClosedLoopHandoffActive = false;

    /* Perform speed and acceleration limit */
    Ifx_MS_FocSolutionF16_limitSpeed(self, 0);

    if (faultStatus == true)
    {
        nextState = Ifx_MS_FocSolutionF16_State_fault;
    }
    else if (Ifx_MS_FocSolutionF16_stopCurrentExceeded(self)
        || (Ifx_Math_Abs_F16(self->rateLimitInSpeedQ15) <= IFX_MS_FOCSOLUTIONF16_MIN_SPEED))
    {
        nextState = Ifx_MS_FocSolutionF16_State_standBy;
    }
    else
    {
        /* Execute sub-state machine */
        Ifx_MS_FocSolutionF16_subStateMachine(self, iToFOutput, currentsDqRef);
        nextState = Ifx_MS_FocSolutionF16_State_rampDown;
    }

    return nextState;
}


static inline bool Ifx_MS_FocSolutionF16_anyClearFaultIsPending(Ifx_MS_FocSolutionF16* self)
{
    /* Holds the overall result of the underlying modules clearFaultIsPending */
    bool clearFaultIsPending = false;

    /* MHA modules fault is pending */
    if (Ifx_MHA_BridgeDrv_TLE9563_clearFaultIsPending(&(self->bridgeDrvTLE9563)) == true)
    {
        clearFaultIsPending = true;
    }
    else if (Ifx_MHA_PatternGen_CYT2B7_clearFaultIsPending(&(self->patternGenCYT2B7)) == true)
    {
        clearFaultIsPending = true;
    }

    /* Modulator fault is pending */
    else if (Ifx_MAS_ModulatorF16_clearFaultIsPending(&(self->modulator)) == true)
    {
        clearFaultIsPending = true;
    }
    else
    {
        clearFaultIsPending = false;
    }

    return clearFaultIsPending;
}


static inline bool Ifx_MS_FocSolutionF16_faultSuccessfullyCleared(Ifx_MS_FocSolutionF16* self)
{
    bool faultCleared;

    /* Check that none of the clear fault requests in the underlying modules are pending */
    if (Ifx_MS_FocSolutionF16_anyClearFaultIsPending(self) == false)
    {
        /* Check for faults in underlying modules */
        if (Ifx_MS_FocSolutionF16_faultStatus(self) == true)
        {
            faultCleared = false;
        }

        /* No faults detected any more, fault(s) successfully cleared */
        else
        {
            faultCleared = true;
        }

        /* Reset the clear fault requested flag */
        self->p_clearFaultIsRequested = false;
    }
    else
    {
        /* Clear fault has not finished execution in every module yet */
        faultCleared = false;
    }

    return faultCleared;
}


static void Ifx_MS_FocSolutionF16_reset(Ifx_MS_FocSolutionF16* self)
{
#if FOC_RRCDOB_ENABLE
    if (self->p_rrcDobFastEligible != false)
    {
        RrcDobCompensator_reset();
        self->p_rrcDobFastEligible = false;
    }
#endif
#if FOC_AUX_ALGORITHMS_ENABLE
    if (self->p_hfiFastPathEnabled != false)
    {
        HfiInjection_reset();
    }

    if (self->p_vafidStructuralEligible != false)
    {
        VAFID_abortFast(VAFID_REJECT_ELIGIBILITY);
    }

    self->p_hfiFastPathEnabled = false;
    self->p_vafidStructuralEligible = false;
#endif
    /* Startup d-q references must not survive a stop/fault reset. */
    self->p_openLoopDqReferenceEnabled = false;
    self->p_directClosedLoopHandoffActive = false;
    Fwc_SpeedRecovery_reset(&self->p_speedRecovery);
    self->p_externalSpeedControllerHandoffPending = false;

#if (FOC_DIAG_FLUX_REFERENCE != 0u)
    /* Flux estimator is reset only when it exists in a diagnostic build. */
    Ifx_Math_LowPass1stF16_setPreviousValue(&(self->fluxEstimator.p_alphaFilter), 0);
    Ifx_Math_LowPass1stF16_setPreviousValue(&(self->fluxEstimator.p_betaFilter), 0);
    Ifx_Math_LowPass1stF16_setPreviousValue(&(self->fluxEstimator.p_speedFilter), 0);
    Ifx_Math_PLLF16_resetBuffer(&self->fluxEstimator.p_pllFilter);
    Ifx_Math_PLLF16_setPreviousValue(&self->fluxEstimator.p_pllFilter, 0);
#endif

    /* PI controllers */
    Ifx_Math_PiF16_setIntegPreviousValue(&(self->speedPi), 0);

    /* Field oriented controller */
    Ifx_MDA_FocControllerF16_reset(&(self->focController));

    /* Acceleration limiter and I2f */
    Ifx_Math_AccelLimitF16_setSpeedStepPreviousValue(&(self->accelerationLimit), 0);
    Ifx_MDA_IToFControllerF16_setAnglePreviousValue(&(self->iToF), 0u);

    /* Speed, command-sign and sector state */
    self->rateLimitInSpeedQ15                      = 0;
    self->p_previousQCommand                       = 0;
    self->p_qCommandZeroCrossing                   = false;
    self->p_estimatedAngle                          = 0u;
    self->previousCurrentReconstructionInfo.sector = 0;
    self->p_voltageSaturationSnapshot.sequence++;
    self->p_voltageSaturationSnapshot.saturationStreakFast = 0u;
    self->p_voltageSaturationSnapshot.unsaturationStreakFast = 0u;
    self->p_voltageSaturationSnapshot.idAtFloorSaturationStreakFast = 0u;
    self->p_voltageSaturationSnapshot.requestedVoltageQ15 = 0;
    self->p_voltageSaturationSnapshot.actualVoltageQ15 = 0;
    self->p_voltageSaturationSnapshot.dcLinkVoltageQ15 = 0;
    self->p_voltageSaturationSnapshot.valid = 0u;
    self->p_voltageSaturationSnapshot.saturated = 0u;
    self->p_voltageSaturationSnapshot.sequence++;
}


static inline void Ifx_MS_FocSolutionF16_publishVoltageSaturationSnapshot(
    Ifx_MS_FocSolutionF16 *self,
    const Ifx_Math_Fract16 requestedVoltageQ15,
    const Ifx_Math_Fract16 actualVoltageQ15,
    const Ifx_Math_Fract16 dcLinkVoltageQ15,
    const bool valid,
    const bool saturated,
    const bool idAtFloor)
{
    volatile Ifx_MS_FocSolutionF16_VoltageSaturationSnapshot *snapshot =
        &(self->p_voltageSaturationSnapshot);

    snapshot->sequence++;
    snapshot->requestedVoltageQ15 = requestedVoltageQ15;
    snapshot->actualVoltageQ15 = actualVoltageQ15;
    snapshot->dcLinkVoltageQ15 = dcLinkVoltageQ15;
    snapshot->valid = (valid != false) ? 1u : 0u;
    snapshot->saturated = ((valid != false) && (saturated != false)) ? 1u : 0u;

    if (valid == false)
    {
        snapshot->saturationStreakFast = 0u;
        snapshot->unsaturationStreakFast = 0u;
        snapshot->idAtFloorSaturationStreakFast = 0u;
    }
    else if (saturated != false)
    {
        if (snapshot->saturationStreakFast != 0xFFFFFFFFu)
        {
            snapshot->saturationStreakFast++;
        }
        snapshot->unsaturationStreakFast = 0u;
        if (idAtFloor != false)
        {
            if (snapshot->idAtFloorSaturationStreakFast != 0xFFFFFFFFu)
            {
                snapshot->idAtFloorSaturationStreakFast++;
            }
        }
        else
        {
            snapshot->idAtFloorSaturationStreakFast = 0u;
        }
    }
    else
    {
        if (snapshot->unsaturationStreakFast != 0xFFFFFFFFu)
        {
            snapshot->unsaturationStreakFast++;
        }
        snapshot->saturationStreakFast = 0u;
        snapshot->idAtFloorSaturationStreakFast = 0u;
    }

    snapshot->sequence++;
}


/* polyspace-end MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */
