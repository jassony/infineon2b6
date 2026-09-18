/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#include "Ifx_MHA_PatternGen_CYT2B7.h"
#include "Ifx_MHA_PatternGen_Cfg.h"
#include "cy_project.h"
#include "SDL_Tcpwm_Cfg.h"
#include "SDL_init.h"
#include "no_opt.h"
#include "Ifx_MS_FocSolutionF16.h"

/* Total number of compare values */
#define IFX_MHA_PATTERNGEN_CYT2B7_N_COMPARE_VALUES (6U)

/* *INDENT-OFF* */
/* Macros to define the component ID */
#define IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTID_SOURCEID     ((uint8)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTID_LIBRARYID    ((uint16)Ifx_ComponentID_LibraryID_mctrlHardwareAbstraction)
#define IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTID_MODULEID     (2U)
#define IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTID_COMPONENTID1 (3U)
#define IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTID_COMPONENTID2 ((uint8)Ifx_ComponentID_ComponentID2_eco)

/* Macros to define the component version */
#define IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTVERSION_MAJOR   (1U)
#define IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTVERSION_MINOR   (2U)
#define IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTVERSION_PATCH   (0U)
#define IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTVERSION_T       (0U)
#define IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTVERSION_REV     (0U)

/* Component ID */
static const Ifx_ComponentID      Ifx_MHA_PatternGen_CYT2B7_componentID = {
    .sourceID = IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTID_SOURCEID,
    .libraryID = IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTID_LIBRARYID,
    .moduleID = IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTID_MODULEID,
    .componentID1 = IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTID_COMPONENTID1,
    .componentID2 = IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_MHA_PatternGen_CYT2B7_componentVersion = {
    .major = IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTVERSION_MAJOR,
    .minor = IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTVERSION_MINOR,
    .patch = IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTVERSION_PATCH,
    .t = IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTVERSION_T,
    .rev = IFX_MHA_PATTERNGEN_CYT2B7_COMPONENTVERSION_REV
};
/* *INDENT-ON* */
/* Checks if any fault occurred and acts accordingly */
static inline bool Ifx_MHA_PatternGen_CYT2B7_checkFaultStatus(Ifx_MHA_PatternGen_CYT2B7* self, bool clearFault);

/* Pattern generator state machine implementation */
static inline void Ifx_MHA_PatternGen_CYT2B7_stateMachine(Ifx_MHA_PatternGen_CYT2B7* self, bool faultStatus, bool
                                                          clearFault);

/* Pattern generator in on state*/
static inline Ifx_MHA_PatternGen_CYT2B7_State Ifx_MHA_PatternGen_CYT2B7_stateOn(Ifx_MHA_PatternGen_CYT2B7* self, bool
                                                                                faultStatus);

/* Pattern generator in off state*/
static inline Ifx_MHA_PatternGen_CYT2B7_State Ifx_MHA_PatternGen_CYT2B7_stateOff(Ifx_MHA_PatternGen_CYT2B7* self, bool
                                                                                 faultStatus);

/* Pattern generator in fault state*/
static inline Ifx_MHA_PatternGen_CYT2B7_State Ifx_MHA_PatternGen_CYT2B7_stateFault(Ifx_MHA_PatternGen_CYT2B7* self,
                                                                                   bool faultStatus, bool clearFault);

/* Update the compare values and trigger times */
static inline void Ifx_MHA_PatternGen_CYT2B7_updateCompareAndTriggers(uint16 compareValues[6], uint16
                                                                      triggerTime_tick[2]);

/* Update the compare values */
static inline void Ifx_MHA_PatternGen_CYT2B7_updateCompareValues(uint16 compareValues[6]);

/* Disable and enable the pattern generator */
static inline void Ifx_MHA_PatternGen_CYT2B7_actionDisable(void);
static inline void Ifx_MHA_PatternGen_CYT2B7_actionEnable(void);

/* polyspace-begin MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */

/* Function to get the component ID */
void Ifx_MHA_PatternGen_CYT2B7_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MHA_PatternGen_CYT2B7_componentID;
}


/* Function to get the component version */
void Ifx_MHA_PatternGen_CYT2B7_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MHA_PatternGen_CYT2B7_componentVersion;
}


void Ifx_MHA_PatternGen_CYT2B7_init(Ifx_MHA_PatternGen_CYT2B7* self)
{
    /* Minimum dead time defined */
    self->_Super_Ifx_MHA_PatternGen.p_deadTimeMin_ns = IFX_MHA_PATTERNGEN_CFG_MIN_DEADTIME_NS;

    /* Initialize internal parameters to 0 */
    self->_Super_Ifx_MHA_PatternGen.p_enable     = false;
    self->_Super_Ifx_MHA_PatternGen.p_clearFault = false;
    self->p_status.state                         = Ifx_MHA_PatternGen_CYT2B7_State_init;

    /* Reset the trigger and the compare values */
    self->p_triggerTime_tick[0] = 0u;
    self->p_triggerTime_tick[1] = 0u;

    for (uint8 i = 0; i < IFX_MHA_PATTERNGEN_CYT2B7_N_COMPARE_VALUES; i++)
    {
        self->p_compareValues_tick[i] = 0u;
    }

}


void Ifx_MHA_PatternGen_CYT2B7_execute(Ifx_MHA_PatternGen_CYT2B7* self, uint16 compareValues[6], uint16
                                       triggerTime_tick[2])
{
    /* Counter variable */
    uint8 i;

    /* Store clearFault variable at the beginning of the module */
    bool  clearFault = self->_Super_Ifx_MHA_PatternGen.p_clearFault;

    /* Variable to store if a fault configured to report and react occurred */
    bool  faultStatus;

    /* Check if a fault occurred and update status flags */
    faultStatus = Ifx_MHA_PatternGen_CYT2B7_checkFaultStatus(self, clearFault);

    /* Assign the trigger and the compare values to private variables */
    self->p_triggerTime_tick[0] = triggerTime_tick[0];
    self->p_triggerTime_tick[1] = triggerTime_tick[1];

    for (i = 0u; i < IFX_MHA_PATTERNGEN_CYT2B7_N_COMPARE_VALUES; i++)
    {
        self->p_compareValues_tick[i] = compareValues[i];
    }

    /* Execute the state machine */
    Ifx_MHA_PatternGen_CYT2B7_stateMachine(self, faultStatus, clearFault);
}


static inline void Ifx_MHA_PatternGen_CYT2B7_stateMachine(Ifx_MHA_PatternGen_CYT2B7* self, bool faultStatus, bool
                                                          clearFault)
{
    /* Variable to store the state */
    Ifx_MHA_PatternGen_CYT2B7_State previousState = self->p_status.state;
    Ifx_MHA_PatternGen_CYT2B7_State nextState     = previousState;

    switch (previousState)
    {
        /* Initialize the module */
        case Ifx_MHA_PatternGen_CYT2B7_State_init:

            /* Set the state to OFF */
            nextState = Ifx_MHA_PatternGen_CYT2B7_State_off;
            break;

        /* Pattern generator is running */
        case Ifx_MHA_PatternGen_CYT2B7_State_on:
            nextState = Ifx_MHA_PatternGen_CYT2B7_stateOn(self, faultStatus);
            break;

        /* Pattern generator not running */
        case Ifx_MHA_PatternGen_CYT2B7_State_off:
            nextState = Ifx_MHA_PatternGen_CYT2B7_stateOff(self, faultStatus);
            break;

        /* Pattern generator is in fault */
        case Ifx_MHA_PatternGen_CYT2B7_State_fault:
            nextState = Ifx_MHA_PatternGen_CYT2B7_stateFault(self, faultStatus, clearFault);
            break;

        default:

            /* do default transition to INIT */
            nextState = Ifx_MHA_PatternGen_CYT2B7_State_init;
            break;
    }

    self->p_status.state = nextState;

    /* Clear the internal variable */
    if (clearFault == true)
    {
        self->_Super_Ifx_MHA_PatternGen.p_clearFault = false;
    }
}

extern uint8 enableControl;
static inline Ifx_MHA_PatternGen_CYT2B7_State Ifx_MHA_PatternGen_CYT2B7_stateOn(Ifx_MHA_PatternGen_CYT2B7* self, bool
                                                                                faultStatus)
{
    /* Variable to store the next state */
    Ifx_MHA_PatternGen_CYT2B7_State nextState;

    if (faultStatus == true)
    {
        /* Set the state to fault */
        nextState = Ifx_MHA_PatternGen_CYT2B7_State_fault;
    }

    /* Check if module disabled */
    else if (self->_Super_Ifx_MHA_PatternGen.p_enable == false)
    {
        /* Set the state to OFF */
        nextState = Ifx_MHA_PatternGen_CYT2B7_State_off;

        /* Disable the pattern generator */
        Ifx_MHA_PatternGen_CYT2B7_actionDisable();
    }
    else
    {
        /* The control ISR submits once per control step. Hardware repeats
         * these compare/trigger values through the intervening PWM cycle. */
        Ifx_MHA_PatternGen_CYT2B7_updateCompareAndTriggers(self->p_compareValues_tick, self->p_triggerTime_tick);


        nextState = Ifx_MHA_PatternGen_CYT2B7_State_on;
    }

    return nextState;
}


static inline Ifx_MHA_PatternGen_CYT2B7_State Ifx_MHA_PatternGen_CYT2B7_stateOff(Ifx_MHA_PatternGen_CYT2B7* self, bool
                                                                                 faultStatus)
{
    /* Variable to store the next state */
    Ifx_MHA_PatternGen_CYT2B7_State nextState;

    /* Check if any fault occurred */
    if (faultStatus == true)
    {
        /* Set the state to fault */
        nextState = Ifx_MHA_PatternGen_CYT2B7_State_fault;
    }

    /* Check if module enabled */
    else if (self->_Super_Ifx_MHA_PatternGen.p_enable == true)
    {
        /* Transition to ON if module enabled on the next execution cycle */
        nextState = Ifx_MHA_PatternGen_CYT2B7_State_on;

        /* Enable the pattern generator */
        Ifx_MHA_PatternGen_CYT2B7_actionEnable();
    }
    else
    {
        nextState = Ifx_MHA_PatternGen_CYT2B7_State_off;
    }

    return nextState;
}


static inline Ifx_MHA_PatternGen_CYT2B7_State Ifx_MHA_PatternGen_CYT2B7_stateFault(Ifx_MHA_PatternGen_CYT2B7* self,
                                                                                   bool faultStatus, bool clearFault)
{
    /* Variable to store the next state */
    Ifx_MHA_PatternGen_CYT2B7_State nextState;

    if ((faultStatus == false)
        && (clearFault == true))
    {
        /* Transition to off if module is in fault in the next execution cycle */
        nextState = Ifx_MHA_PatternGen_CYT2B7_State_off;

        /* Disable the pattern generator*/
        Ifx_MHA_PatternGen_CYT2B7_actionDisable();
    }
    else
    {
        nextState = Ifx_MHA_PatternGen_CYT2B7_State_fault;
    }

    return nextState;
}


static inline bool Ifx_MHA_PatternGen_CYT2B7_checkFaultStatus(Ifx_MHA_PatternGen_CYT2B7* self, bool clearFault)
{
    /* No fault to be checked */
    return false;
}


static inline void Ifx_MHA_PatternGen_CYT2B7_updateCompareValues(uint16 compareValues[6])
{
    /* Update shadow registers if for first half of PWM */
    Cy_Tcpwm_Pwm_SetCompare0_Buff(TCPWMx_GRPx_CNTx_U, compareValues[0]);
    Cy_Tcpwm_Pwm_SetCompare0_Buff(TCPWMx_GRPx_CNTx_V, compareValues[1]);
    Cy_Tcpwm_Pwm_SetCompare0_Buff(TCPWMx_GRPx_CNTx_W, compareValues[2]);

    /* Update shadow registers if for second half of PWM */
    Cy_Tcpwm_Pwm_SetCompare1_Buff(TCPWMx_GRPx_CNTx_U, compareValues[3]);
    Cy_Tcpwm_Pwm_SetCompare1_Buff(TCPWMx_GRPx_CNTx_V, compareValues[4]);
    Cy_Tcpwm_Pwm_SetCompare1_Buff(TCPWMx_GRPx_CNTx_W, compareValues[5]);
//      Cy_Tcpwm_Pwm_SetCompare0_Buff(TCPWMx_GRPx_CNTx_U, 1000);
//    Cy_Tcpwm_Pwm_SetCompare0_Buff(TCPWMx_GRPx_CNTx_V, 1000);
//    Cy_Tcpwm_Pwm_SetCompare0_Buff(TCPWMx_GRPx_CNTx_W, 1000);
//
//    /* Update shadow registers if for second half of PWM */
//    Cy_Tcpwm_Pwm_SetCompare1_Buff(TCPWMx_GRPx_CNTx_U, 3000);
//    Cy_Tcpwm_Pwm_SetCompare1_Buff(TCPWMx_GRPx_CNTx_V, 3000);
//    Cy_Tcpwm_Pwm_SetCompare1_Buff(TCPWMx_GRPx_CNTx_W, 3000);
}

uint16_t test111[2] = {0,0};
static inline void Ifx_MHA_PatternGen_CYT2B7_updateCompareAndTriggers(uint16 compareValues[6], uint16
                                                                      triggerTime_tick[2])
{
    /* Update shadow registers */
    Ifx_MHA_PatternGen_CYT2B7_updateCompareValues(compareValues);

    /* Update trigger times */
    /**yxs**/
    test111[0] = (2000 + (compareValues[0] + compareValues[3])/2 + 120)%4000;
    test111[1] = (2000 + (compareValues[1] + compareValues[4])/2 + 120)%4000;
//    if(test111[0] != 119)
//    {
//      test111[0] = 119;
//    }
//    Cy_Tcpwm_Counter_SetCompare0_Buff(TCPWMx_GRPx_CNTx_CC0, (2000 + (compareValues[0] + compareValues[3])/2 + 120)%4000);
//    Cy_Tcpwm_Counter_SetCompare1_Buff(TCPWMx_GRPx_CNTx_CC1, (2000 + (compareValues[1] + compareValues[4])/2 + 120)%4000);
    /**yxs**/
//    Cy_Tcpwm_Counter_SetCompare0_Buff(TCPWMx_GRPx_CNTx_CC0, 120);
//    Cy_Tcpwm_Counter_SetCompare1_Buff(TCPWMx_GRPx_CNTx_CC1, 120);
    
    /* Update trigger times */
    Cy_Tcpwm_Counter_SetCompare0_Buff(TCPWMx_GRPx_CNTx_CC0, triggerTime_tick[0]);
    Cy_Tcpwm_Counter_SetCompare1_Buff(TCPWMx_GRPx_CNTx_CC1, triggerTime_tick[1]);

    /* Generate the active switch event (capture0) for all channels */
    Cy_Tcpwm_TriggerCapture0(TCPWMx_GRPx_CNTx_U);
    Cy_Tcpwm_TriggerCapture0(TCPWMx_GRPx_CNTx_V);
    Cy_Tcpwm_TriggerCapture0(TCPWMx_GRPx_CNTx_W);
    Cy_Tcpwm_TriggerCapture0(TCPWMx_GRPx_CNTx_CC0);
    Cy_Tcpwm_TriggerCapture0(TCPWMx_GRPx_CNTx_CC1);
}

extern uint8_t vdc_underv;
static inline void Ifx_MHA_PatternGen_CYT2B7_actionDisable(void)
{
    /* Set stop select to 1 */
    TCPWMx_GRPx_CNTx_U->unTR_IN_SEL0.stcField.u8STOP_SEL = 1u;
    TCPWMx_GRPx_CNTx_V->unTR_IN_SEL0.stcField.u8STOP_SEL = 1u;
    TCPWMx_GRPx_CNTx_W->unTR_IN_SEL0.stcField.u8STOP_SEL = 1u;
    /* Do not reinitialize the FOC instance here. This action is reached
     * whenever PWM is disabled; a full init would overwrite online-tuned
     * current and speed PI parameters. */
    
    // if(vdc_underv)
    // {
    //      PortInit_GPIO();
    // }

}


static inline void Ifx_MHA_PatternGen_CYT2B7_actionEnable(void)
{
    /* Set stop select to 0 */
    TCPWMx_GRPx_CNTx_U->unTR_IN_SEL0.stcField.u8STOP_SEL = 0u;
    TCPWMx_GRPx_CNTx_V->unTR_IN_SEL0.stcField.u8STOP_SEL = 0u;
    TCPWMx_GRPx_CNTx_W->unTR_IN_SEL0.stcField.u8STOP_SEL = 0u;
}


/* polyspace-end MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */
