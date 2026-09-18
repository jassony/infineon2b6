/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#include "Ifx_MAS_ModulatorF16.h"
#include "Ifx_Math_All.h"
#include "Ifx_Math_Cfg.h"
#include "no_opt.h"

/* Size of the lookup table used  */
#define IFX_MAS_MODULATORF16_TABLESIN60SQRT3_LUT_SIZE  (12)

/* Maximum index of the lookup table used, 2^(size) -1  */
#define IFX_MAS_MODULATORF16_TABLESIN60SQRT3_MAX_INDEX (4095U)

/* Macro to convert angle value to lookup table index */
#define IFX_MAS_MODULATORF16_ANGLE_TO_INDEX            (15U - IFX_MAS_MODULATORF16_TABLESIN60SQRT3_LUT_SIZE)

/* Modulation index max for linear region, normalized by 2/pi */
#define IFX_MAS_MODULATORF16_CONST_MI_0907_Q15         (18921)

/* Macros to detect individual faults */
#define IFX_MAS_MODULATORF16_MAXAMPLITUDE_FAULT_STS    (0x20U)
#define IFX_MAS_MODULATORF16_OVERMODULATION_FAULT_STS  (0x40U)

/* Macros to define the component ID */
#define IFX_MAS_MODULATORF16_COMPONENTID_SOURCEID      ((uint8)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_MAS_MODULATORF16_COMPONENTID_LIBRARYID     ((uint16)Ifx_ComponentID_LibraryID_mctrlActuatorSensor)
#define IFX_MAS_MODULATORF16_COMPONENTID_MODULEID      (0U)
#define IFX_MAS_MODULATORF16_COMPONENTID_COMPONENTID1  (1U)

#define IFX_MAS_MODULATORF16_COMPONENTID_COMPONENTID2  ((uint8)Ifx_ComponentID_ComponentID2_eco)

/* Macros to define the component version */
#define IFX_MAS_MODULATORF16_COMPONENTVERSION_MAJOR    (1U)
#define IFX_MAS_MODULATORF16_COMPONENTVERSION_MINOR    (2U)
#define IFX_MAS_MODULATORF16_COMPONENTVERSION_PATCH    (0U)
#define IFX_MAS_MODULATORF16_COMPONENTVERSION_T        (0U)
#define IFX_MAS_MODULATORF16_COMPONENTVERSION_REV      (0U)

/* Component ID */
static const Ifx_ComponentID      Ifx_MHA_ModulatorF16_componentID = {
    .sourceID                                                        = IFX_MAS_MODULATORF16_COMPONENTID_SOURCEID,
    .libraryID                                                       = IFX_MAS_MODULATORF16_COMPONENTID_LIBRARYID,
    .moduleID                                                        = IFX_MAS_MODULATORF16_COMPONENTID_MODULEID,
    .componentID1                                                    =
        IFX_MAS_MODULATORF16_COMPONENTID_COMPONENTID1, .componentID2 = IFX_MAS_MODULATORF16_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_MHA_ModulatorF16_componentVersion = {
    .major = IFX_MAS_MODULATORF16_COMPONENTVERSION_MAJOR, .minor = IFX_MAS_MODULATORF16_COMPONENTVERSION_MINOR,
    .patch = IFX_MAS_MODULATORF16_COMPONENTVERSION_PATCH, .t = IFX_MAS_MODULATORF16_COMPONENTVERSION_T, .rev =
        IFX_MAS_MODULATORF16_COMPONENTVERSION_REV
};

/* Current-measurement trigger position, in PWM timer ticks.  Keep this
 * calibration in the XCP section so it can be adjusted without rebuilding. */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16 Cal_MAS_SampleOffset_tick_u16 = 250u;

/* polyspace-begin MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */

/* API to initialize configuration related to current measurement */
static inline void Ifx_MAS_ModulatorF16_initCurrMeasCfg(Ifx_MAS_ModulatorF16* self);

/* API to check the fault status */
static inline bool Ifx_MAS_ModulatorF16_checkFaultStatus(Ifx_MAS_ModulatorF16* self, uint32 faults);

/* APIs to check the fault status of each fault individually */
static inline bool Ifx_MAS_ModulatorF16_maxAmplitudeFaultStatus(Ifx_MAS_ModulatorF16* self, bool* faultStatusRet,
                                                                uint32 faults);
static inline bool Ifx_MAS_ModulatorF16_overmodulationFaultStatus(Ifx_MAS_ModulatorF16* self, bool* faultStatusRet,
                                                                  uint32 faults);

/* API to set the compare values and triggers */
static inline void Ifx_MAS_ModulatorF16_setActiveShort(Ifx_MAS_ModulatorF16* self, sint16 cmprValues);

/* API to check and limit the inputs */
static inline uint8 Ifx_MAS_ModulatorF16_limitInputs(Ifx_MAS_ModulatorF16* self, Ifx_Math_Fract16 refVoltageAmp,
                                                     Ifx_Math_Fract16 dcLinkVoltage,
                                                     Ifx_Math_Fract16* voltageOverDcLinkVoltage);

/* API to calculate compare values based on symmetric switching */
static inline void Ifx_MAS_ModulatorF16_symmetricSwitching(sint16* cmprVal, sint16* switchingTimes, uint8 sector);

/* API to modify the switching times to get a minimum sensing time */
static inline void Ifx_MAS_ModulatorF16_modSwitch(Ifx_MAS_ModulatorF16* self, sint16* compVal,
                                                  sint16* switchingTimes);

/* API to limit the switching times */
static inline void Ifx_MAS_ModulatorF16_limitSwitchTimes(Ifx_MAS_ModulatorF16* self, sint16* cmprVal);

/* API to calculate the triggers based on sector */
static inline void Ifx_MAS_ModulatorF16_calcTriggers(Ifx_MAS_ModulatorF16* self, sint16* cmprVal);

/* API to set the triggers for current measurement */
static inline void Ifx_MAS_ModulatorF16_measBegin(Ifx_MAS_ModulatorF16* self, sint16* cmprVal);

/* API to assign the compare values and triggers */
static inline void Ifx_MAS_ModulatorF16_assignOutputs(Ifx_MAS_ModulatorF16* self, sint16* cmprVal);

/* API to calculate the effective times of the inverter switching states */
static inline void Ifx_MAS_ModulatorF16_updatePolar7SegQ16(Ifx_MAS_ModulatorF16* self, uint32 angle, Ifx_Math_Fract16
                                                           voltageOverDcLinkVoltage);

/* API to calculate the 7 segment switching times based on t_R, t_L, t_0, and sector */
static inline void Ifx_MAS_ModulatorF16_calcSwitchTimes(Ifx_MAS_ModulatorF16* self,
                                                        sint16              * switchingTimes);
static inline Ifx_MAS_ModulatorF16_State Ifx_MAS_ModulatorF16_stateOff(Ifx_MAS_ModulatorF16* self, bool
                                                                       enterFaultState, uint32 angle, Ifx_Math_Fract16
                                                                       voltageOverDcLinkVoltage);
static inline Ifx_MAS_ModulatorF16_State Ifx_MAS_ModulatorF16_stateOn(Ifx_MAS_ModulatorF16* self, bool
                                                                      enterFaultState, uint32 angle, Ifx_Math_Fract16
                                                                      voltageOverDcLinkVoltage);
static inline Ifx_MAS_ModulatorF16_State Ifx_MAS_ModulatorF16_stateFault(Ifx_MAS_ModulatorF16* self);

/* Function to get the component ID */
void Ifx_MAS_ModulatorF16_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MHA_ModulatorF16_componentID;
}


/* Function to get the component version */
void Ifx_MAS_ModulatorF16_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MHA_ModulatorF16_componentVersion;
}


/* Initialize the modulator */
void Ifx_MAS_ModulatorF16_init(Ifx_MAS_ModulatorF16* self)
{
    /* Initialize the period */
    self->p_period_tick = IFX_MAS_MODULATORF16_CFG_PERIOD_TICK;

    /* Initialize the deadtime */
    self->p_deadTime_tick = IFX_MAS_MODULATORF16_CFG_DEADTIME_TICK;

    /* Active low and trigs to 0 */
    Ifx_MAS_ModulatorF16_setActiveShort(self, (2 * self->p_period_tick));

    /* Set the state machine to INIT */
    self->p_status.state              = Ifx_MAS_ModulatorF16_State_init;
    self->p_status.maxAmplitudeFlag   = false;
    self->p_status.overmodulationFlag = false;

    /* Disable module */
    Ifx_MAS_ModulatorF16_enable(self, false);

    /* Initialize current measurement configuration */
    Ifx_MAS_ModulatorF16_initCurrMeasCfg(self);

    /* Minimum sensing time */
    Ifx_MAS_ModulatorF16_p_recalculateMinSenseTime(self);

    /* Maximum amplitude */
    self->p_maxAmplitudeQ15 = IFX_MAS_MODULATORF16_CFG_MAX_AMPLITUDE_Q15;

    /* Initialize internal variables */
    self->p_clearFault = false;
    /** yxs**/
    self->p_forceDutyEnable = false;   // 默认关闭
    self->p_forceDutyU = 1000;         // 默认50%
    self->p_forceDutyV = 1000;
    self->p_forceDutyW = 1000;    // 默认50%
    self->p_forceSampleEnable = false;
    // 采样点1: 周期中心
    self->p_forceSampleTick1 = self->p_period_tick;  
    // 采样点2: 可选(如果不使用可设为0)
    self->p_forceSampleTick2 = self->p_period_tick;
    /** yxs**/
}


static inline void Ifx_MAS_ModulatorF16_initCurrMeasCfg(Ifx_MAS_ModulatorF16* self)
{
    /* Current measurement config */
    self->p_currentMeasurement.p_driverDelay_tick     = IFX_MAS_MODULATORF16_CFG_DRIVERDELAY_TICK;
    self->p_currentMeasurement.p_ringingTime_tick     = IFX_MAS_MODULATORF16_CFG_RINGINGTIME_TICK;
    self->p_currentMeasurement.p_measurementTime_tick = IFX_MAS_MODULATORF16_CFG_MEASUREMENTTIME_TICK;

    /* Delta to be added if measuring from the beginning */
    Ifx_MAS_ModulatorF16_p_recalculateDeltaBegin(self);
}


static inline bool Ifx_MAS_ModulatorF16_checkFaultStatus(Ifx_MAS_ModulatorF16* self, uint32 faults)
{
    /* boolean output fault status initialized to false */
    bool faultStatusRet = false;

    /* Check if any fault occurred */
    if (faults != 0u)
    {
        /* 1. check for maximum amplitude */
        self->p_status.maxAmplitudeFlag = Ifx_MAS_ModulatorF16_maxAmplitudeFaultStatus(self, &faultStatusRet, faults);

        /* 2. check for overmodulation */
        self->p_status.overmodulationFlag = Ifx_MAS_ModulatorF16_overmodulationFaultStatus(self, &faultStatusRet,
            faults);
    }

    /* Check if a fault clear request was placed */
    else if (self->p_clearFault == true)
    {
        /* Clear all faults in status in case a fault clear request was done */
        self->p_status.maxAmplitudeFlag   = false;
        self->p_status.overmodulationFlag = false;
    }
    else
    {
        /* Final else-clause */
    }

    /* return the fault status */
    return faultStatusRet;
}


static inline bool Ifx_MAS_ModulatorF16_maxAmplitudeFaultStatus(Ifx_MAS_ModulatorF16* self, bool* faultStatusRet,
                                                                uint32 faults)
{
    /* Variable to store the max. amplitude fault status */
    bool faultMaxAmplitude = self->p_status.maxAmplitudeFlag;

    /* 1. check individually for maximum amplitude */
    /* 1.1 Check if this fault is configured as ENABLED */
#if IFX_MAS_MODULATORF16_CFG_FAULT_REACTION_MAX_AMPLITUDE >= IFX_MAS_MODULATORF16_FAULT_REACTION_ENABLE

    if ((faults & IFX_MAS_MODULATORF16_MAXAMPLITUDE_FAULT_STS) != 0)
    {
        /* Set the fault information status bit */
        faultMaxAmplitude = true;

        /* 1.2 Check if this fault is configured for REPORTING */
#if IFX_MAS_MODULATORF16_CFG_FAULT_REACTION_MAX_AMPLITUDE >= IFX_MAS_MODULATORF16_FAULT_REACTION_REPORT_ONLY
#if (IFX_MAS_MODULATORF16_CFG_ENABLE_FAULT_OUT == 1)

        /* report fault source ONLY the 1st time it occurs */
        if (self->p_status.maxAmplitudeFlag == false)
        {
            /* report the fault through the user interface */
            IFX_MAS_MODULATORF16_CFG_FAULT_OUT();
        }

#endif

        /* 1.3 Check if this fault is configured for REACTION */
#if IFX_MAS_MODULATORF16_CFG_FAULT_REACTION_MAX_AMPLITUDE >= IFX_MAS_MODULATORF16_FAULT_REACTION_REPORT_REACT

        /* set the fault status for reacting */
        *faultStatusRet = true;
#endif
#endif
    }

#endif

    return faultMaxAmplitude;
}


static inline bool Ifx_MAS_ModulatorF16_overmodulationFaultStatus(Ifx_MAS_ModulatorF16* self, bool* faultStatusRet,
                                                                  uint32 faults)
{
    /* Variable to store the max. amplitude fault status */
    bool faultOvermodulation = self->p_status.overmodulationFlag;

    /* 2 check individually for Overmodulation */
    /* 2.1 Check if this fault is configured as ENABLED */
#if IFX_MAS_MODULATORF16_CFG_FAULT_REACTION_OVERMODULATION >= IFX_MAS_MODULATORF16_FAULT_REACTION_ENABLE

    if ((faults & IFX_MAS_MODULATORF16_OVERMODULATION_FAULT_STS) != 0)
    {
        /* set the fault information status bit */
        faultOvermodulation = true;

#if IFX_MAS_MODULATORF16_CFG_FAULT_REACTION_OVERMODULATION >= IFX_MAS_MODULATORF16_FAULT_REACTION_REPORT_ONLY
#if (IFX_MAS_MODULATORF16_CFG_ENABLE_FAULT_OUT == 1)

        /* report fault source ONLY the 1st time it occurs */
        if (self->p_status.overmodulationFlag == false)
        {
            /* report the fault through the user interface */
            IFX_MAS_MODULATORF16_CFG_FAULT_OUT();
        }

#endif

        /* 2.3 Check if this fault is configured for REACTION */
#if IFX_MAS_MODULATORF16_CFG_FAULT_REACTION_OVERMODULATION >= IFX_MAS_MODULATORF16_FAULT_REACTION_REPORT_REACT

        /* set the fault status for reacting */
        *faultStatusRet = true;
#endif
#endif
    }

#endif

    return faultOvermodulation;
}


/* polyspace-begin CODE-METRIC:CALLING [Justified:Low] "This is a common function expected to be called multiple times."
 * */
static inline void Ifx_MAS_ModulatorF16_setActiveShort(Ifx_MAS_ModulatorF16* self, sint16 cmprValues)
{
    /* Set the compare up values */
    self->p_output.compareValues_tick[0] = (uint16)cmprValues;
    self->p_output.compareValues_tick[1] = (uint16)cmprValues;
    self->p_output.compareValues_tick[2] = (uint16)cmprValues;

    /* Set the compare down values */
    sint16 downCompareValue = (2 * self->p_period_tick) - cmprValues;
    self->p_output.compareValues_tick[3] = (uint16)downCompareValue;
    self->p_output.compareValues_tick[4] = (uint16)downCompareValue;
    self->p_output.compareValues_tick[5] = (uint16)downCompareValue;

    /* Set the current triggers */
    self->p_output.triggerTime_tick[0] = 0;
    self->p_output.triggerTime_tick[1] = 0;
}


/* polyspace-end CODE-METRIC:CALLING [Justified:Low] "This is a common function expected to be called multiple times."
 * */
void Ifx_MAS_ModulatorF16_execute(Ifx_MAS_ModulatorF16* self, Ifx_Math_PolarFract16 refVoltage, Ifx_Math_Fract16
                                  dcLinkVoltageQ15)
{
    /* Stores the occurred faults */
    uint8                      faultStatus;

    /* Stores if the state machine will be set to false */
    bool                       enterFaultState;

    /* Variable to store the state */
    Ifx_MAS_ModulatorF16_State previousState = (self->p_status.state);
    Ifx_MAS_ModulatorF16_State nextState     = previousState;

    /* Stores the ratio between amplitude of needed voltage and dc voltage */
    Ifx_Math_Fract16           voltageOverDcLinkVoltage;

    /* Limit the inputs and check if any fault occurred */
    faultStatus = Ifx_MAS_ModulatorF16_limitInputs(self, refVoltage.amplitude, dcLinkVoltageQ15,
        &voltageOverDcLinkVoltage);

    /* Update the actual output voltage angle to the ref. voltage angle */
    self->p_output.actualVoltage.angle = refVoltage.angle;

    /* Check fault configuration and return if state machine will be set to false */
    enterFaultState = Ifx_MAS_ModulatorF16_checkFaultStatus(self, faultStatus);

    /* Handles the state machine */
    switch (previousState)
    {
        /* Initialize the module */
        case Ifx_MAS_ModulatorF16_State_init:

            /* do state transition to OFF at the 1st state machine execution cycle */
            nextState = Ifx_MAS_ModulatorF16_State_off;
            break;

        /* Modulator not running */
        case Ifx_MAS_ModulatorF16_State_off:

            /* Call API when state is in off */
            nextState = Ifx_MAS_ModulatorF16_stateOff(self, enterFaultState, refVoltage.angle,
                voltageOverDcLinkVoltage);
            break;

        /* Modulator running */
        case Ifx_MAS_ModulatorF16_State_on:
            nextState = Ifx_MAS_ModulatorF16_stateOn(self, enterFaultState, refVoltage.angle,
                voltageOverDcLinkVoltage);
            break;

        /* Modulator in fault */
        case Ifx_MAS_ModulatorF16_State_fault:

            /* Call API when state is in off */
            nextState = Ifx_MAS_ModulatorF16_stateFault(self);
            break;

        /* Invalid state, transition to init */
        default:

            /* Default transition to INIT */
            nextState = Ifx_MAS_ModulatorF16_State_init;

            /* Active low and trigs to 0 */
            Ifx_MAS_ModulatorF16_setActiveShort(self, (2 * self->p_period_tick));
            break;
    }

    /* Clear internal clear fault variable */
    self->p_clearFault = false;

    /* Update state */
    self->p_status.state = nextState;
}


static inline uint8 Ifx_MAS_ModulatorF16_limitInputs(Ifx_MAS_ModulatorF16* self, Ifx_Math_Fract16 refVoltageAmp,
                                                     Ifx_Math_Fract16 dcLinkVoltage,
                                                     Ifx_Math_Fract16* voltageOverDcLinkVoltage)
{
    /* Stores the occurred faults */
    uint8            faultStatus = 0;

    /* Stores actual voltage */
    Ifx_Math_Fract16 limitedAmp;
    Ifx_Math_Fract16 modIndex;
    Ifx_Math_Fract16 dcCheckedAmp;
    Ifx_Math_Fract16 limModIndex;
    Ifx_Math_Fract16 actualVoltageAmp;

    /* Limit amplitude to the maximum amplitude parameter */
    if (refVoltageAmp > self->p_maxAmplitudeQ15)
    {
        limitedAmp   = self->p_maxAmplitudeQ15;
        faultStatus |= (uint8)IFX_MAS_MODULATORF16_MAXAMPLITUDE_FAULT_STS;
    }
    else
    {
        limitedAmp = refVoltageAmp;
    }

    /* Limit DC-Link voltage to positive numbers */
    if (dcLinkVoltage <= 0)
    {
        modIndex     = 0;
        dcCheckedAmp = 0;
    }

    /* Calculate the ratio between amplitude of needed voltage and dc voltage */
    else
    {
        modIndex     = Ifx_Math_DivShLSatNZ_F16(limitedAmp, dcLinkVoltage, 15u);
        dcCheckedAmp = limitedAmp;
    }

    /* Limit modulation index to the Linear Region */
    if (modIndex > IFX_MAS_MODULATORF16_CONST_MI_0907_Q15)
    {
        limModIndex  = IFX_MAS_MODULATORF16_CONST_MI_0907_Q15;
        faultStatus |= (uint8)IFX_MAS_MODULATORF16_OVERMODULATION_FAULT_STS;

        /* Write the actual voltage (DC-Link voltage * linear modulation limit) */
        actualVoltageAmp = Ifx_Math_Mul_F16(dcLinkVoltage, IFX_MAS_MODULATORF16_CONST_MI_0907_Q15);
    }
    else
    {
        limModIndex      = modIndex;
        actualVoltageAmp = dcCheckedAmp;
    }

    /* Write actual voltage output and modulation index */
    *voltageOverDcLinkVoltage              = limModIndex;
    self->p_output.actualVoltage.amplitude = actualVoltageAmp;

    return faultStatus;
}


static inline Ifx_MAS_ModulatorF16_State Ifx_MAS_ModulatorF16_stateOff(Ifx_MAS_ModulatorF16* self, bool
                                                                       enterFaultState, uint32 angle, Ifx_Math_Fract16
                                                                       voltageOverDcLinkVoltage)
{
    Ifx_MAS_ModulatorF16_State nextState;

    /* Transition to FAULT and execute configured software reaction */
    if (enterFaultState == true)
    {
        nextState = Ifx_MAS_ModulatorF16_State_fault;
#if IFX_MAS_MODULATORF16_CFG_FAULT_OUT_BEHAVIOR == IFX_MAS_MODULATORF16_OUTPUT_BEHAVIOR_ACTIVE_SHORT_LOW
        Ifx_MAS_ModulatorF16_setActiveShort(self, (2 * self->p_period_tick));
#elif IFX_MAS_MODULATORF16_CFG_FAULT_OUT_BEHAVIOR == IFX_MAS_MODULATORF16_OUTPUT_BEHAVIOR_ACTIVE_SHORT_HIGH
        Ifx_MAS_ModulatorF16_setActiveShort(self, 0);
#elif IFX_MAS_MODULATORF16_CFG_FAULT_OUT_BEHAVIOR == IFX_MAS_MODULATORF16_OUTPUT_BEHAVIOR_ACTIVE_SHORT_HIGH_LOW
        Ifx_MAS_ModulatorF16_setActiveShort(self, (self->p_period_tick / 2));
#endif
    }

    /* Transition to ON if module enabled on the next execution cycle */
    else if (self->p_enable == true)
    {
        nextState = Ifx_MAS_ModulatorF16_State_on;

        /* Calculate the new compare values */
        Ifx_MAS_ModulatorF16_updatePolar7SegQ16(self, angle, voltageOverDcLinkVoltage);
    }
    else
    {
        /* Stay in off */
        nextState = Ifx_MAS_ModulatorF16_State_off;
    }

    return nextState;
}


static inline Ifx_MAS_ModulatorF16_State Ifx_MAS_ModulatorF16_stateOn(Ifx_MAS_ModulatorF16* self, bool
                                                                      enterFaultState, uint32 angle, Ifx_Math_Fract16
                                                                      voltageOverDcLinkVoltage)
{
    Ifx_MAS_ModulatorF16_State nextState;

    /* Transition to FAULT and execute configured software reaction */
    if (enterFaultState == true)
    {
        nextState = Ifx_MAS_ModulatorF16_State_fault;
#if IFX_MAS_MODULATORF16_CFG_FAULT_OUT_BEHAVIOR == IFX_MAS_MODULATORF16_OUTPUT_BEHAVIOR_ACTIVE_SHORT_LOW
        Ifx_MAS_ModulatorF16_setActiveShort(self, (2 * self->p_period_tick));
#elif IFX_MAS_MODULATORF16_CFG_FAULT_OUT_BEHAVIOR == IFX_MAS_MODULATORF16_OUTPUT_BEHAVIOR_ACTIVE_SHORT_HIGH
        Ifx_MAS_ModulatorF16_setActiveShort(self, 0);
#elif IFX_MAS_MODULATORF16_CFG_FAULT_OUT_BEHAVIOR == IFX_MAS_MODULATORF16_OUTPUT_BEHAVIOR_ACTIVE_SHORT_HIGH_LOW
        Ifx_MAS_ModulatorF16_setActiveShort(self, (self->p_period_tick / 2));
#endif
    }

    /* Modulator enabled */
    else if (self->p_enable == true)
    {
        /**yxs**/
            // === 新增：强制占空比逻辑 ===
    if (self->p_forceDutyEnable == true) {
        // 计算每相的tick值
        uint16 dutyTickU = self->p_forceDutyU;
        uint16 dutyTickV = self->p_forceDutyV;
        uint16 dutyTickW = self->p_forceDutyW;
        
        // 设置比较值 (三相独立)
        self->p_output.compareValues_tick[0] = dutyTickU;
        self->p_output.compareValues_tick[1] = dutyTickV;
        self->p_output.compareValues_tick[2] = dutyTickW;


        self->p_output.compareValues_tick[3] = 2*self->p_period_tick  -dutyTickU;
        self->p_output.compareValues_tick[4] = 2*self->p_period_tick  -dutyTickV;
        self->p_output.compareValues_tick[5] = 2*self->p_period_tick  -dutyTickW;
        
        // 设置采样触发点 (中心点采样)
            // === 修改: 采样点设置 ===
    if (self->p_forceSampleEnable == true) {
        // 使用标定的采样点
        self->p_output.triggerTime_tick[0] = self->p_forceSampleTick1;
        self->p_output.triggerTime_tick[1] = self->p_forceSampleTick2;
    } 
       
        
        nextState = Ifx_MAS_ModulatorF16_State_on;
        return nextState;
    } 
    /**yxs**/
        /* Calculate the new compare values */
        Ifx_MAS_ModulatorF16_updatePolar7SegQ16(self, angle, voltageOverDcLinkVoltage);

        /* Set the state to on */
        nextState = Ifx_MAS_ModulatorF16_State_on;
    }
    else

    /* Modulator disabled */
    {
        /* Set the state to OFF */
        nextState = Ifx_MAS_ModulatorF16_State_off;

        /* Active low and trigs to 0 */
        Ifx_MAS_ModulatorF16_setActiveShort(self, (2 * self->p_period_tick));
    }

    return nextState;
}



static inline Ifx_MAS_ModulatorF16_State Ifx_MAS_ModulatorF16_stateFault(Ifx_MAS_ModulatorF16* self)
{
    Ifx_MAS_ModulatorF16_State nextState;

    if (self->p_clearFault == true)
    {
        /* do state transition to OFF */
        nextState = Ifx_MAS_ModulatorF16_State_off;

        /* Active low and trigs to 0 */
        Ifx_MAS_ModulatorF16_setActiveShort(self, (2 * self->p_period_tick));
    }
    else
    {
        nextState = Ifx_MAS_ModulatorF16_State_fault;
    }

    return nextState;
}


static inline void Ifx_MAS_ModulatorF16_updatePolar7SegQ16(Ifx_MAS_ModulatorF16* self, uint32 angle, Ifx_Math_Fract16
                                                           voltageOverDcLinkVoltage)
{
    /* Sector number [0..5] */
    uint8  sector;

    /* Lookup table index */
    uint16 tableIndex;

    /* Array to store the switching times ={t_Right, t_Left, t_0} */
    sint16 switchingTimes[3];
    uint32 angleTimesSix;

    /* Calculate sector number */
    angleTimesSix = (angle >> 17) * 6;
    sector        = (uint8)(angleTimesSix >> 15);

    /* Assign sector value */
    self->p_output.currentReconstructionInfo.sector = sector;

    /* Calculate angle in sin lookup table: index [0..4095] */
    tableIndex = (uint16)(angleTimesSix >>
                          (IFX_MAS_MODULATORF16_ANGLE_TO_INDEX)&IFX_MAS_MODULATORF16_TABLESIN60SQRT3_MAX_INDEX);

    /* Get sine and cosine values from the lookup table */
    Ifx_Math_Fract16 sinValue = Ifx_MAS_ModulatorF16_lutSin60Sqrt3[tableIndex];
    Ifx_Math_Fract16 cosValue = Ifx_MAS_ModulatorF16_lutSin60Sqrt3[(IFX_MAS_MODULATORF16_TABLESIN60SQRT3_MAX_INDEX -
                                                                    tableIndex)];

    /* Calculate effective switching times, t_R, t_L, and t_0 */
    /* t_Right */
    switchingTimes[0] = Ifx_Math_MulShR_F16(self->p_period_tick, Ifx_Math_Mul_F16(voltageOverDcLinkVoltage, cosValue),
        14u);

    /* t_Left */
    switchingTimes[1] = Ifx_Math_MulShR_F16(self->p_period_tick, Ifx_Math_Mul_F16(voltageOverDcLinkVoltage, sinValue),
        14u);

    /* t_0 */
    switchingTimes[2] = (self->p_period_tick - switchingTimes[0] - switchingTimes[1]) / 2;
    Ifx_MAS_ModulatorF16_calcSwitchTimes(self, switchingTimes);
}


static inline void Ifx_MAS_ModulatorF16_calcSwitchTimes(Ifx_MAS_ModulatorF16* self, sint16* switchingTimes)
{
    /* Temporary values to hold the compare values. The array is organized as following:
     * The three phase pulses have different length: long, middle, short.
     * 0 and 3: switch on/off time of the long pulse.
     * 1 and 4: switch on/off time of the middle pulse.
     * 2 and 5: switch on/off time of the short pulse.
     */
    sint16 cmprVal[6];

    /* Calculate compare values based on symmetric switching */
    Ifx_MAS_ModulatorF16_symmetricSwitching(cmprVal, switchingTimes, self->p_output.currentReconstructionInfo.sector);

    /* Assign compare values symmetrically */
    cmprVal[3] = cmprVal[0];
    cmprVal[4] = cmprVal[1];
    cmprVal[5] = cmprVal[2];
    Ifx_MAS_ModulatorF16_modSwitch(self, cmprVal, switchingTimes);

    /* Limit switching times between the defined limits */
    Ifx_MAS_ModulatorF16_limitSwitchTimes(self, cmprVal);

    /* Calculate the triggers based on sector */
    Ifx_MAS_ModulatorF16_calcTriggers(self, cmprVal);

    /* Assign compare values and triggers */
    Ifx_MAS_ModulatorF16_assignOutputs(self, cmprVal);
}


static inline void Ifx_MAS_ModulatorF16_symmetricSwitching(sint16* cmprVal, sint16* switchingTimes, uint8 sector)
{
    if ((sector & 1u) == 1u)
    {
        /* sector 1,3,5 */
        cmprVal[0] = switchingTimes[2];
        cmprVal[1] = switchingTimes[1] + switchingTimes[2];
        cmprVal[2] = switchingTimes[0] + cmprVal[1];
    }
    else
    {
        /* sector 0,2,4 */
        cmprVal[0] = switchingTimes[2];
        cmprVal[1] = switchingTimes[0] + switchingTimes[2];
        cmprVal[2] = switchingTimes[1] + cmprVal[1];
    }
}


static inline void Ifx_MAS_ModulatorF16_modSwitch(Ifx_MAS_ModulatorF16* self, sint16* compVal, sint16* switchingTimes)
{
    /* Ticks to shift the middle pulse to the right */
    sint16 shiftRight;

    /* Is at least one of the measurement windows (tL and/or tR) too small to be measured? */
    if ((switchingTimes[0] < self->p_minSenseTime_tick)
        || (switchingTimes[1] < self->p_minSenseTime_tick))
    {
        /* Shift the middle pulse to the right by the amount required by the smaller measurement window */
        if (switchingTimes[0] < switchingTimes[1])
        {
            shiftRight = switchingTimes[0] - self->p_minSenseTime_tick;
        }
        else
        {
            shiftRight = switchingTimes[1] - self->p_minSenseTime_tick;
        }

        /* Modify count up */
        compVal[1] = compVal[1] - shiftRight;

        /* Modify count down */
        compVal[4] = compVal[4] + shiftRight;
    }
}


static inline void Ifx_MAS_ModulatorF16_limitSwitchTimes(Ifx_MAS_ModulatorF16* self, sint16* cmprVal)
{
    uint8 i;

    /* Limit compare values */
    for (i = 0; i < 6; i++)
    {
        /* Set input to upper limit */
        if (cmprVal[i] > (self->p_period_tick + 1))
        {
            cmprVal[i] = (self->p_period_tick + 1);
        }

        /* Set input to lower limit */
        else if (cmprVal[i] < 0)
        {
            cmprVal[i] = 0;
        }
        else
        {
            /* Do nothing */
        }
    }
}


static inline void Ifx_MAS_ModulatorF16_calcTriggers(Ifx_MAS_ModulatorF16* self, sint16* cmprVal)
{
    Ifx_MAS_ModulatorF16_measBegin(self, cmprVal);
}


static inline void Ifx_MAS_ModulatorF16_measBegin(Ifx_MAS_ModulatorF16* self, sint16* cmprVal)
{
    uint16 samplePointOffset_tick = Cal_MAS_SampleOffset_tick_u16;

//     self->p_output.triggerTime_tick[0] = 700;//(uint16)((sint16)(self->p_period_tick));
//
//     /* The measurement trigger during up-counting phase*/
//     self->p_output.triggerTime_tick[1] = 800;//(uint16)((sint16)(self->p_period_tick ));


        self->p_output.triggerTime_tick[0] = samplePointOffset_tick;

        /* The measurement trigger during up-counting phase*/
        self->p_output.triggerTime_tick[1] = samplePointOffset_tick;




    // self->p_output.triggerTime_tick[0] = (uint16)((sint16)(cmprVal[0] +
                                                        //    self->p_currentMeasurement.p_deltaBegin_tick));

    /* The measurement trigger during up-counting phase*/
    // self->p_output.triggerTime_tick[1] = (uint16)((sint16)(((sint16)(doublePeriod - cmprVal[5])) +
                                                        //    self->p_currentMeasurement.p_deltaBegin_tick));
}


/* polyspace-begin CODE-METRIC:VOCF [Justified:Low] "For performance reasons, the assignment of the compare values
 * according to the correct sector is implemented as a switch-case" */
static inline void Ifx_MAS_ModulatorF16_assignOutputs(Ifx_MAS_ModulatorF16* self, sint16* cmprVal)
{
    sint16 doublePeriod = 2 * self->p_period_tick;

    switch (self->p_output.currentReconstructionInfo.sector)
    {
        case 0:
            self->p_output.compareValues_tick[0] = (uint16)cmprVal[0];
            self->p_output.compareValues_tick[1] = (uint16)cmprVal[1];
            self->p_output.compareValues_tick[2] = (uint16)cmprVal[2];
            self->p_output.compareValues_tick[3] = (uint16)((sint16)(doublePeriod - cmprVal[3]));
            self->p_output.compareValues_tick[4] = (uint16)((sint16)(doublePeriod - cmprVal[4]));
            self->p_output.compareValues_tick[5] = (uint16)((sint16)(doublePeriod - cmprVal[5]));
            break;

        case 1:
            self->p_output.compareValues_tick[1] = (uint16)cmprVal[0];
            self->p_output.compareValues_tick[0] = (uint16)cmprVal[1];
            self->p_output.compareValues_tick[2] = (uint16)cmprVal[2];
            self->p_output.compareValues_tick[4] = (uint16)((sint16)(doublePeriod - cmprVal[3]));
            self->p_output.compareValues_tick[3] = (uint16)((sint16)(doublePeriod - cmprVal[4]));
            self->p_output.compareValues_tick[5] = (uint16)((sint16)(doublePeriod - cmprVal[5]));
            break;

        case 2:
            self->p_output.compareValues_tick[1] = (uint16)cmprVal[0];
            self->p_output.compareValues_tick[2] = (uint16)cmprVal[1];
            self->p_output.compareValues_tick[0] = (uint16)cmprVal[2];
            self->p_output.compareValues_tick[4] = (uint16)((sint16)(doublePeriod - cmprVal[3]));
            self->p_output.compareValues_tick[5] = (uint16)((sint16)(doublePeriod - cmprVal[4]));
            self->p_output.compareValues_tick[3] = (uint16)((sint16)(doublePeriod - cmprVal[5]));
            break;

        case 3:
            self->p_output.compareValues_tick[2] = (uint16)cmprVal[0];
            self->p_output.compareValues_tick[1] = (uint16)cmprVal[1];
            self->p_output.compareValues_tick[0] = (uint16)cmprVal[2];
            self->p_output.compareValues_tick[5] = (uint16)((sint16)(doublePeriod - cmprVal[3]));
            self->p_output.compareValues_tick[4] = (uint16)((sint16)(doublePeriod - cmprVal[4]));
            self->p_output.compareValues_tick[3] = (uint16)((sint16)(doublePeriod - cmprVal[5]));
            break;

        case 4:
            self->p_output.compareValues_tick[2] = (uint16)cmprVal[0];
            self->p_output.compareValues_tick[0] = (uint16)cmprVal[1];
            self->p_output.compareValues_tick[1] = (uint16)cmprVal[2];
            self->p_output.compareValues_tick[5] = (uint16)((sint16)(doublePeriod - cmprVal[3]));
            self->p_output.compareValues_tick[3] = (uint16)((sint16)(doublePeriod - cmprVal[4]));
            self->p_output.compareValues_tick[4] = (uint16)((sint16)(doublePeriod - cmprVal[5]));
            break;

        default:
            self->p_output.compareValues_tick[0] = (uint16)cmprVal[0];
            self->p_output.compareValues_tick[2] = (uint16)cmprVal[1];
            self->p_output.compareValues_tick[1] = (uint16)cmprVal[2];
            self->p_output.compareValues_tick[3] = (uint16)((sint16)(doublePeriod - cmprVal[3]));
            self->p_output.compareValues_tick[5] = (uint16)((sint16)(doublePeriod - cmprVal[4]));
            self->p_output.compareValues_tick[4] = (uint16)((sint16)(doublePeriod - cmprVal[5]));
            break;
    }
}


/* polyspace-end CODE-METRIC:VOCF [Justified:Low] "For performance reasons, the assignment of the compare values
 * according to the correct sector is implemented as a switch-case" */

/* polyspace-end MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */
