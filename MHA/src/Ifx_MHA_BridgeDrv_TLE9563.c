/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#include "Ifx_MHA_BridgeDrv_TLE9563.h"
#include "Ifx_MHA_BridgeDrv_Cfg.h"

#include "TLE9563_FuncLayer.h"

/* Macros to define the component ID */
#define IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTID_SOURCEID     ((uint8)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTID_LIBRARYID    ((uint16)Ifx_ComponentID_LibraryID_mctrlHardwareAbstraction)
#define IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTID_MODULEID     (0U)
#define IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTID_COMPONENTID1 (4U)

#define IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTID_COMPONENTID2 ((uint8)Ifx_ComponentID_ComponentID2_eco)

/* Macros to define the component version */
#define IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTVERSION_MAJOR   (1U)
#define IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTVERSION_MINOR   (2U)
#define IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTVERSION_PATCH   (0U)
#define IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTVERSION_T       (0U)
#define IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTVERSION_REV     (0U)

/* *INDENT-OFF* */
/* Component ID */
static const Ifx_ComponentID      Ifx_MHA_BridgeDrv_TLE9563_componentID = {
    .sourceID = IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTID_SOURCEID,
    .libraryID = IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTID_LIBRARYID,
    .moduleID = IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTID_MODULEID,
    .componentID1 = IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTID_COMPONENTID1,
    .componentID2 = IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_MHA_BridgeDrv_TLE9563_componentVersion = {
    .major = IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTVERSION_MAJOR,
    .minor = IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTVERSION_MINOR,
    .patch = IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTVERSION_PATCH,
    .t = IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTVERSION_T,
    .rev = IFX_MHA_BRIDGEDRV_TLE9563_COMPONENTVERSION_REV
};
/* *INDENT-ON* */
/* Mask for unveiling all undervoltage status bits within the SUP_STAT register */
#define IFX_MHA_BRIDGEDRV_TLE9563_MASK_SUPSTAT_UV      (0x0D54u)

/* Mask for unveiling all overvoltage status bits within the SUP_STAT register */
#define IFX_MHA_BRIDGEDRV_TLE9563_MASK_SUPSTAT_OV      (0x02A2u)

/* Mask for unveiling all overvoltage status bits within the DSOV register */
#define IFX_MHA_BRIDGEDRV_TLE9563_MASK_DSOV_OV         (0x373Fu)

/* Mask for unveiling all overcurrent status bits within the DSOV register */
#define IFX_MHA_BRIDGEDRV_TLE9563_MASK_DSOV_OC         (0x4000u)

/* Mask for unveiling all overcurrent status bits within the HS_OL_OC_OT_STAT register */
#define IFX_MHA_BRIDGEDRV_TLE9563_MASK_HSOLOCOTSTAT_OC (0x0007u)

/* Number of bridge driver execution cycles until it can safely be assumed that an entire update of all status registers
 * has been performed by the LLD
 * Note: LLD ensures that the status on demand that is not older than the time period of 25 valid cyclic function
 *       calls assuming there is no other setter/clear/state-transition/watchdog requested during these time period.
 *       Potential requests up until this point are setCsaGain, WdgService and potential clrFault requests. This
 *       is why a higher value of 30 is chosen here. */
#define IFX_MHA_BRIDGEDRV_TLE9563_LLD_UPDATE_STAT_CYC  (30u)

/* Checks if any fault occurred and acts accordingly */
static inline bool Ifx_MHA_BridgeDrv_TLE9563_checkFaultStatus(Ifx_MHA_BridgeDrv_TLE9563* self, bool const clearFault);

/* Check HW faults of bridge driver */
static inline bool Ifx_MHA_BridgeDrv_TLE9563_checkHwFaults(bool* faultOverCurrent, bool* faultOverVoltage,
                                                           bool* faultUnderVoltage, bool const clearFault);

/* Check overcurrent status of TLE9563 via LLD */
static inline bool Ifx_MHA_BridgeDrv_TLE9563_checkOvercurrent(void);

/* Check overvoltage status of TLE9563 via LLD */
static inline bool Ifx_MHA_BridgeDrv_TLE9563_checkOvervoltage(void);

/* Check undervoltage status of TLE9563 via LLD */
static inline bool Ifx_MHA_BridgeDrv_TLE9563_checkUndervoltage(void);

/* Bridge driver state machine implementation */
static inline void Ifx_MHA_BridgeDrv_TLE9563_stateMachine(Ifx_MHA_BridgeDrv_TLE9563* self);

/* Bridge driver in init state */
static inline Ifx_MHA_BridgeDrv_TLE9563_State Ifx_MHA_BridgeDrv_TLE9563_stateInit(Ifx_MHA_BridgeDrv_TLE9563* self);

/* Bridge driver in on state */
static inline Ifx_MHA_BridgeDrv_TLE9563_State Ifx_MHA_BridgeDrv_TLE9563_stateOn(Ifx_MHA_BridgeDrv_TLE9563* self, bool
                                                                                const clearFault);

/* Bridge driver in off state */
static inline Ifx_MHA_BridgeDrv_TLE9563_State Ifx_MHA_BridgeDrv_TLE9563_stateOff(Ifx_MHA_BridgeDrv_TLE9563* self, bool
                                                                                 const clearFault);

/* Bridge driver in fault state */
static inline Ifx_MHA_BridgeDrv_TLE9563_State Ifx_MHA_BridgeDrv_TLE9563_stateFault(Ifx_MHA_BridgeDrv_TLE9563* self,
                                                                                   bool const
                                                                                   clearFault);

/* Bridge driver queue handling of LLD requests */
static inline void Ifx_MHA_BridgeDrv_TLE9563_queueHandling(Ifx_MHA_BridgeDrv_TLE9563* self);

/* Mapping of queue requests to LLD function calls */
static inline uint8 Ifx_MHA_BridgeDrv_TLE9563_mapRequestToLLD(Ifx_MHA_BridgeDrv_TLE9563_requestsToLLD const request,
                                                              uint16 const                                  data);

/* Disable and enable the bridge driver */
static inline bool Ifx_MHA_BridgeDrv_TLE9563_actionDisable(Ifx_MHA_BridgeDrv_TLE9563* self);
static inline bool Ifx_MHA_BridgeDrv_TLE9563_actionEnable(Ifx_MHA_BridgeDrv_TLE9563* self);

/* Send all available clear fault requests to the LLD */
static inline void Ifx_MHA_BridgeDrv_TLE9563_clearAllFaultsRequest(Ifx_MHA_BridgeDrv_TLE9563* self);

/* Check the summarized status of all available clear fault request */
static inline bool Ifx_MHA_BridgeDrv_TLE9563_clearAllFaultsCheck(Ifx_MHA_BridgeDrv_TLE9563* self);

/* polyspace-begin MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */

/* Function to get the component ID */
void Ifx_MHA_BridgeDrv_TLE9563_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MHA_BridgeDrv_TLE9563_componentID;
}


/* Function to get the component version */
void Ifx_MHA_BridgeDrv_TLE9563_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MHA_BridgeDrv_TLE9563_componentVersion;
}


void Ifx_MHA_BridgeDrv_TLE9563_execute(Ifx_MHA_BridgeDrv_TLE9563* self)
{
    /* Execute state machine */
    Ifx_MHA_BridgeDrv_TLE9563_stateMachine(self);

    /* Handle queue for LLD requests */
    Ifx_MHA_BridgeDrv_TLE9563_queueHandling(self);

    /* Check whether watchdog must be triggered */
    if (self->p_cycleCounter < self->p_wdTriggerThreshold)
    {
        /* No -> increment bridge driver cycle counter */
        self->p_cycleCounter++;
    }
    else
    {
        /* Yes -> set queue flags to set watchdog serve request */
        self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqServeWD].requested = true;
        self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqServeWD].processed = false;

        /* Reset cycle counter */
        self->p_cycleCounter = 0u;
    }
}


void Ifx_MHA_BridgeDrv_TLE9563_init(Ifx_MHA_BridgeDrv_TLE9563* self)
{
    /* Status of LLD initialization */
    uint8           lldInitSuccess;

    /* Watchdog timer period */
    tWD_timerPeriod wdTimerPeriod;
    uint32          wdTimerPeriodUs;

    /* Initialize internal parameters to 0 */
    self->_Super_Ifx_MHA_BridgeDrv.p_enable     = false;
    self->_Super_Ifx_MHA_BridgeDrv.p_clearFault = false;
    self->p_status.overcurrent                  = false;
    self->p_status.overvoltage                  = false;
    self->p_status.undervoltage                 = false;
    self->p_status.spiFault                     = false;
    self->p_cycleCounter                        = 0u;

    /* Initialize TLE9563 LLD */
    lldInitSuccess = TLE9563_Init();

    /* Was the LLD initialization successful? */
    if (lldInitSuccess)
    {
        self->p_status.TLE9563InitFault = false;
        self->p_status.state            = Ifx_MHA_BridgeDrv_TLE9563_State_init;

        /* Initialize CSA gain */
        Ifx_MHA_BridgeDrv_TLE9563_setCsaGain(self,
            (Ifx_MHA_BridgeDrv_TLE9563_optionCsaGain)IFX_MHA_BRIDGEDRV_CFG_CSA_GAIN);
    }
    else
    {
        /* LLD initialization was not successful, so set non-recoverable fault flag */
        self->p_status.TLE9563InitFault = true;
        self->p_status.state            = Ifx_MHA_BridgeDrv_TLE9563_State_fault;
    }

    /* Get watchdog period from LLD and convert to us */
    wdTimerPeriod = (tWD_timerPeriod)TLE9563_getWdTimerPeriod();

    switch (wdTimerPeriod)
    {
        case WD_timerPeriod_10ms:
            wdTimerPeriodUs = 10000u;
            break;

        case WD_timerPeriod_20ms:
            wdTimerPeriodUs = 20000u;
            break;

        case WD_timerPeriod_50ms:
            wdTimerPeriodUs = 50000u;
            break;

        case WD_timerPeriod_100ms:
            wdTimerPeriodUs = 100000u;
            break;

        case WD_timerPeriod_200ms:
            wdTimerPeriodUs = 200000u;
            break;

        case WD_timerPeriod_500ms:
            wdTimerPeriodUs = 500000u;
            break;

        case WD_timerPeriod_1s:
            wdTimerPeriodUs = 1000000u;
            break;

        case WD_timerPeriod_10s:
            wdTimerPeriodUs = 10000000u;
            break;
    }

    /* Calculate the threshold at which the watchdog must be triggered */
    self->p_wdTriggerThreshold = wdTimerPeriodUs / IFX_MHA_BRIDGEDRV_CFG_EXECUTION_PERIOD_US;
}


/*************************************************************************************************
* Local functions declaration section
*************************************************************************************************/
static inline bool Ifx_MHA_BridgeDrv_TLE9563_faultStatus(bool swFaultStatus, uint32 hwFaultStatus, uint8
                                                         faultConfiguration, bool* faultStatusRet)
{
    /* Return value */
    bool faultOccured = false;

    /* 1.1 Check if this fault is configured as ENABLED */
    if (faultConfiguration >= IFX_MHA_BRIDGEDRV_FAULT_REACTION_ENABLE)
    {
        if (hwFaultStatus != 0u)
        {
            /* Set the fault information status bit */
            faultOccured = true;

            /* 1.2 Check if this fault is configured for REPORTING */
            if (faultConfiguration >= IFX_MHA_BRIDGEDRV_FAULT_REACTION_REPORT_ONLY)
            {
#if (IFX_MHA_BRIDGEDRV_CFG_ENABLE_FAULT_OUT == 1)

                /* Report fault source ONLY the 1st time it occurs */
                if (swFaultStatus == false)
                {
                    /* Report the fault through the user interface */
                    IFX_MHA_BRIDGEDRV_CFG_FAULT_OUT();
                }

#endif

                /* 1.3 Check if this fault is configured for REACTION */
                if (faultConfiguration >= IFX_MHA_BRIDGEDRV_FAULT_REACTION_REPORT_REACT)
                {
                    /* Set the fault status for reacting */
                    *faultStatusRet = true;
                }
            }
        }
    }

    return faultOccured;
}


static inline bool Ifx_MHA_BridgeDrv_TLE9563_checkHwFaults(bool* faultOverCurrent, bool* faultOverVoltage,
                                                           bool* faultUnderVoltage, bool clearFault)
{
    /* Boolean output fault status */
    bool faultStatusRet;

    /* Local fault status */
    bool overCurrentStatus;
    bool overVoltageStatus;
    bool underVoltageStatus;

    /* Initialize output fault status to false */
    faultStatusRet = false;

    /* Check for overcurrent */
    overCurrentStatus = Ifx_MHA_BridgeDrv_TLE9563_checkOvercurrent();
    *faultOverCurrent = Ifx_MHA_BridgeDrv_TLE9563_faultStatus(*faultOverCurrent, overCurrentStatus,
        IFX_MHA_BRIDGEDRV_CFG_FAULT_REACTION_OVERCURRENT, &faultStatusRet);

    /* Check for overvoltage */
    overVoltageStatus = Ifx_MHA_BridgeDrv_TLE9563_checkOvervoltage();
    *faultOverVoltage = Ifx_MHA_BridgeDrv_TLE9563_faultStatus(*faultUnderVoltage, overVoltageStatus,
        IFX_MHA_BRIDGEDRV_CFG_FAULT_REACTION_OVERVOLT, &faultStatusRet);

    /* Check for undervoltage */
    underVoltageStatus = Ifx_MHA_BridgeDrv_TLE9563_checkUndervoltage();
    *faultUnderVoltage = Ifx_MHA_BridgeDrv_TLE9563_faultStatus(*faultUnderVoltage, underVoltageStatus,
        IFX_MHA_BRIDGEDRV_CFG_FAULT_REACTION_UNDERVOLT, &faultStatusRet);

    return faultStatusRet;
}


static inline bool Ifx_MHA_BridgeDrv_TLE9563_checkFaultStatus(Ifx_MHA_BridgeDrv_TLE9563* self, bool const clearFault)
{
    /* TLE9563 SPI/CRC fail status */
    uint8 sifSpiCrcFail;

    /* Initialize boolean output fault status to false */
    bool  faultStatusRet = false;

    /* Save status of bridge driver faults in local variables */
    bool  faultOverCurrent  = self->p_status.overcurrent;
    bool  faultOverVoltage  = self->p_status.overvoltage;
    bool  faultUnderVoltage = self->p_status.undervoltage;

    /* Check if a clear fault was requested */
    if (clearFault == true)
    {
        /* Send clear request for all hardware faults via LLD */
        Ifx_MHA_BridgeDrv_TLE9563_clearAllFaultsRequest(self);

        /* Reset SPI fault status */
        self->p_status.spiFault = false;
    }

    /* Check HW faults */
    faultStatusRet = Ifx_MHA_BridgeDrv_TLE9563_checkHwFaults(&faultOverCurrent, &faultOverVoltage, &faultUnderVoltage,
        clearFault);

    /* Update bdrv faults from local variables */
    self->p_status.overcurrent  = faultOverCurrent;
    self->p_status.overvoltage  = faultOverVoltage;
    self->p_status.undervoltage = faultUnderVoltage;

    /* Get TLE9563 SPI/CRC fail status from the status information field */
    sifSpiCrcFail = TLE9563_getSifBitSpiCrcFail();

    /* Set SPI fault status in case TLE9563 has signalized an SPI/CRC error or if device driver signals an SPI receive
     * error */
    if ((sifSpiCrcFail != 0u)
        || (self->p_status.deviceDriverErrorLog == TLE9563_DEVICEDRIVER_ERRORLOG_SPI_RECEIVE_ERR))
    {
        self->p_status.spiFault = true;
    }

    /* Bridge Driver component should go to fail in case any of the voltage, current, SPI or Init faults is present */
    faultStatusRet = faultStatusRet
                     || self->p_status.spiFault
                     || self->p_status.TLE9563InitFault;

    /* Return the fault status */
    return faultStatusRet;
}


static inline bool Ifx_MHA_BridgeDrv_TLE9563_checkOvercurrent(void)
{
    /* Local fault status */
    bool   overcurrentStatus;

    /* TLE9563 Drain-source overvoltage HBVOUT register value */
    uint16 dsov;

    /* TLE9563 High-Side Switch Status register value */
    uint16 hsOlOcOtStat;

    /* Get status registers of TLE9563 from the LLD */
    dsov         = TLE9563_getDsovStsReg();
    hsOlOcOtStat = TLE9563_getHsOlOcOtStsReg();

    /* Check for overcurrent */
    overcurrentStatus = ((dsov & IFX_MHA_BRIDGEDRV_TLE9563_MASK_DSOV_OC) > 0u);
    overcurrentStatus = overcurrentStatus | ((hsOlOcOtStat & IFX_MHA_BRIDGEDRV_TLE9563_MASK_HSOLOCOTSTAT_OC) > 0u);

    return overcurrentStatus;
}


static inline bool Ifx_MHA_BridgeDrv_TLE9563_checkOvervoltage(void)
{
    /* Local fault status */
    bool   overvoltageStatus;

    /* TLE9563 Supply Voltage Fail Status register value */
    uint16 supStat;

    /* TLE9563 Drain-source overvoltage HBVOUT register value */
    uint16 dsov;

    /* Get status registers of TLE9563 from the LLD */
    supStat = TLE9563_getSupStsReg();
    dsov    = TLE9563_getDsovStsReg();

    /* Check for overvoltage */
    overvoltageStatus = ((supStat & IFX_MHA_BRIDGEDRV_TLE9563_MASK_SUPSTAT_OV) > 0u);
    overvoltageStatus = overvoltageStatus | ((dsov & IFX_MHA_BRIDGEDRV_TLE9563_MASK_DSOV_OV) > 0u);

    return overvoltageStatus;
}


static inline bool Ifx_MHA_BridgeDrv_TLE9563_checkUndervoltage(void)
{
    /* Local fault status */
    bool   undervoltageStatus;

    /* TLE9563 Supply Voltage Fail Status register value */
    uint16 supStat;

    /* Get status register of TLE9563 from the LLD */
    supStat = TLE9563_getSupStsReg();

    /* Check for undervoltage */
    undervoltageStatus = ((supStat & IFX_MHA_BRIDGEDRV_TLE9563_MASK_SUPSTAT_UV) > 0u);

    return undervoltageStatus;
}


static inline Ifx_MHA_BridgeDrv_TLE9563_State Ifx_MHA_BridgeDrv_TLE9563_stateInit(Ifx_MHA_BridgeDrv_TLE9563* self)
{
    /* Variable to store the next state */
    Ifx_MHA_BridgeDrv_TLE9563_State nextState;

    /* Check for undervoltage (initial readout of the status registers of TLE9563 is done in LLD init) */
    self->p_status.undervoltage = Ifx_MHA_BridgeDrv_TLE9563_checkUndervoltage();

    /* Undervoltage pending, i.e. charge pump has not ramped up yet? */
    if (!self->p_status.undervoltage)
    {
        /* No, go to state OFF */
        nextState = Ifx_MHA_BridgeDrv_TLE9563_State_off;
    }
    else
    {
        /* Yes -> clear supply status register until no more undervoltage fault is detected */
        self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrSupSts].requested = true;

        if (self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrSupSts].processed)
        {
            self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrSupSts].processed = false;
        }
    }

    /* Have all status register of TLE9563 already been updated at least once after the clear request? */
    if (self->p_cycleCounter > IFX_MHA_BRIDGEDRV_TLE9563_LLD_UPDATE_STAT_CYC)
    {
        /* Charge pump is not ramping up, go to state FAULT via state OFF */
        nextState = Ifx_MHA_BridgeDrv_TLE9563_State_off;
    }
    else
    {
        /* Stay in INIT as long as charge pump has not ramped up yet */
        nextState = Ifx_MHA_BridgeDrv_TLE9563_State_init;
    }

    return nextState;
}


static inline Ifx_MHA_BridgeDrv_TLE9563_State Ifx_MHA_BridgeDrv_TLE9563_stateOff(Ifx_MHA_BridgeDrv_TLE9563* self, bool
                                                                                 const clearFault)
{
    /* Variable to store the next state */
    Ifx_MHA_BridgeDrv_TLE9563_State nextState;

    /* Variable to store fault status of bridge driver */
    bool                            faultStatus;

    /* Flag whether the enable action has finished */
    bool                            finished;

    /* Check fault status of bridge driver */
    faultStatus = Ifx_MHA_BridgeDrv_TLE9563_checkFaultStatus(self, clearFault);

    /* check condition for T1 */
    if ((self->_Super_Ifx_MHA_BridgeDrv.p_enable == true)
        && (faultStatus == false))
    {
        /* do Action BridgeDriver ENABLED */
        finished = Ifx_MHA_BridgeDrv_TLE9563_actionEnable(self);

        if (finished)
        {
            /* do state transition to ON */
            nextState = Ifx_MHA_BridgeDrv_TLE9563_State_on;
        }
        else
        {
            /* stay in OFF state */
            nextState = Ifx_MHA_BridgeDrv_TLE9563_State_off;
        }
    }

    /* check condition for T2 */
    else if (faultStatus == true)
    {
        /* do state transition to FAULT */
        nextState = Ifx_MHA_BridgeDrv_TLE9563_State_fault;
    }
    else
    {
        /* stay in OFF state */
        nextState = Ifx_MHA_BridgeDrv_TLE9563_State_off;
    }

    return nextState;
}


static inline Ifx_MHA_BridgeDrv_TLE9563_State Ifx_MHA_BridgeDrv_TLE9563_stateOn(Ifx_MHA_BridgeDrv_TLE9563* self, bool
                                                                                const clearFault)
{
    /* Variable to store the next state */
    Ifx_MHA_BridgeDrv_TLE9563_State nextState;

    /* Variable to store fault status of bridge driver */
    bool                            faultStatus;

    /* Flag whether the disable action has finished */
    bool                            finished;

    /* Check fault status of bridge driver */
    faultStatus = Ifx_MHA_BridgeDrv_TLE9563_checkFaultStatus(self, clearFault);

    /* check condition for T2 */
    if (faultStatus == true)
    {
        /* do state transition to FAULT */
        nextState = Ifx_MHA_BridgeDrv_TLE9563_State_fault;
    }

    /* check condition for T1 */
    else if (self->_Super_Ifx_MHA_BridgeDrv.p_enable == false)
    {
        /* do Action BridgeDriver DISABLED */
        finished = Ifx_MHA_BridgeDrv_TLE9563_actionDisable(self);

        if (finished)
        {
            /* do state transition to OFF */
            nextState = Ifx_MHA_BridgeDrv_TLE9563_State_off;
        }
        else
        {
            nextState = Ifx_MHA_BridgeDrv_TLE9563_State_on;
        }
    }
    else
    {
        nextState = Ifx_MHA_BridgeDrv_TLE9563_State_on;
    }

    return nextState;
}


static inline Ifx_MHA_BridgeDrv_TLE9563_State Ifx_MHA_BridgeDrv_TLE9563_stateFault(Ifx_MHA_BridgeDrv_TLE9563* self,
                                                                                   bool const
                                                                                   clearFault)
{
    /* Variable to store the next state */
    Ifx_MHA_BridgeDrv_TLE9563_State nextState;

    /* Variable to store fault status of bridge driver */
    bool                            faultStatus;

    /* Check fault status of bridge driver */
    faultStatus = Ifx_MHA_BridgeDrv_TLE9563_checkFaultStatus(self, clearFault);

    /* check condition for T1 */
    if ((self->_Super_Ifx_MHA_BridgeDrv.p_enable == true)
        && (faultStatus == false))
    {
        /* do state transition to ON */
        nextState = Ifx_MHA_BridgeDrv_TLE9563_State_on;

        /* do Action BridgeDriver ENABLED */
        Ifx_MHA_BridgeDrv_TLE9563_actionEnable(self);
    }
    else if ((self->_Super_Ifx_MHA_BridgeDrv.p_enable == false)
             && (faultStatus == false))
    {
        /* do state transition to OFF */
        nextState = Ifx_MHA_BridgeDrv_TLE9563_State_off;

        /* do Action BridgeDriver DISABLED */
        Ifx_MHA_BridgeDrv_TLE9563_actionDisable(self);
    }
    else
    {
        /* stay in FAULT */
        nextState = Ifx_MHA_BridgeDrv_TLE9563_State_fault;
    }

    return nextState;
}


static inline void Ifx_MHA_BridgeDrv_TLE9563_stateMachine(Ifx_MHA_BridgeDrv_TLE9563* self)
{
    /* Variable to store the previous state */
    Ifx_MHA_BridgeDrv_TLE9563_State previousState = self->p_status.state;

    /* Variable to store the next state */
    Ifx_MHA_BridgeDrv_TLE9563_State nextState;

    /* Variable to store clear fault */
    bool                            clearFault = self->_Super_Ifx_MHA_BridgeDrv.p_clearFault;

    switch (previousState)
    {
        /* Bridge driver is in init state */
        case Ifx_MHA_BridgeDrv_TLE9563_State_init:
            nextState = Ifx_MHA_BridgeDrv_TLE9563_stateInit(self);
            break;

        /* Bridge driver is in off state */
        case Ifx_MHA_BridgeDrv_TLE9563_State_off:
            nextState = Ifx_MHA_BridgeDrv_TLE9563_stateOff(self, clearFault);
            break;

        /* Bridge driver is in on state */
        case Ifx_MHA_BridgeDrv_TLE9563_State_on:
            nextState = Ifx_MHA_BridgeDrv_TLE9563_stateOn(self, clearFault);
            break;

        /* Bridge driver is in fault state (Ifx_MHA_BridgeDrv_TLE9563_State_fault, default) */
        default:
            nextState = Ifx_MHA_BridgeDrv_TLE9563_stateFault(self, clearFault);
            break;
    }

    /* Update state and reset clear fault */
    self->p_status.state = nextState;

    /* All clear fault requests have been processed by the LLD? */
    if (Ifx_MHA_BridgeDrv_TLE9563_clearAllFaultsCheck(self))
    {
        /* Yes -> clear fault request has finished */
        self->_Super_Ifx_MHA_BridgeDrv.p_clearFault = false;
    }
}


static inline void Ifx_MHA_BridgeDrv_TLE9563_queueHandling(Ifx_MHA_BridgeDrv_TLE9563* self)
{
    /* Temporary variable to hold the request with the highest priority */
    Ifx_MHA_BridgeDrv_TLE9563_requestsToLLD requestLLD;

    /* Counter variable */
    uint32                                  i;

    /* Index of the request with the highest priority */
    uint32                                  requestHighestPriority;

    /* Status returned by the LLD */
    uint8                                   success;

    /* No pending request has been passed on successfully to the LLD yet */
    success = 0u;

    /* Find the pending request with the highest priority */
    for (i = 0u; i < Ifx_MHA_BridgeDrv_TLE9563_reqNumTotal; i++)
    {
        if (self->p_queueElement[i].requested
            && (!self->p_queueElement[i].processed))
        {
            /* Store the number of the request with the highest priority */
            requestHighestPriority = i;

            /* Convert the request number to the corresponding enum */
            requestLLD = (Ifx_MHA_BridgeDrv_TLE9563_requestsToLLD)requestHighestPriority;

            /* Pass the request on to the LLD */
            success = Ifx_MHA_BridgeDrv_TLE9563_mapRequestToLLD(requestLLD, self->p_queueElement[i].data);

            /* Only one request can be passed on per cyclic task call */
            break;
        }
    }

    /* Call TLE9563 LLD cyclic function to process any wd/set/clear requests and to update status registers */
    self->p_status.deviceDriverErrorLog = TLE9563_deviceDriverCyclicTask();

    /* Did the LLD successfully start processing a request? */
    if (success
        && (self->p_status.deviceDriverErrorLog == TLE9563_DEVICEDRIVER_ERRORLOG_STARTED))
    {
        /* Yes -> update the queue element with the highest priority */
        self->p_queueElement[requestHighestPriority].processed = true;
    }
}


static inline uint8 Ifx_MHA_BridgeDrv_TLE9563_mapRequestToLLD(Ifx_MHA_BridgeDrv_TLE9563_requestsToLLD const request,
                                                              uint16 const                                  data)
{
    /* Status returned from LLD */
    uint8 success;

    /* Pass the request to the correct LLD function */
    switch (request)
    {
        case Ifx_MHA_BridgeDrv_TLE9563_reqServeWD:
            {
                success = TLE9563_serveWatchdog();
            } break;

        case Ifx_MHA_BridgeDrv_TLE9563_reqClrSupSts:
            {
                success = TLE9563_clrSupSts();
            } break;

        case Ifx_MHA_BridgeDrv_TLE9563_reqClrThermSts:
            {
                success = TLE9563_clrThermSts();
            } break;

        case Ifx_MHA_BridgeDrv_TLE9563_reqClrDevSts:
            {
                success = TLE9563_clrDevSts();
            } break;

        case Ifx_MHA_BridgeDrv_TLE9563_reqClrBusSts:
            {
                success = TLE9563_clrBusSts();
            } break;

        case Ifx_MHA_BridgeDrv_TLE9563_reqClrWkSts:
            {
                success = TLE9563_clrWkSts();
            } break;

        case Ifx_MHA_BridgeDrv_TLE9563_reqClrHsOlOcOtSts:
            {
                success = TLE9563_clrHsOlOcOtSts();
            } break;

        case Ifx_MHA_BridgeDrv_TLE9563_reqClrDsovSts:
            {
                success = TLE9563_clrDsovSts();
            } break;

        case Ifx_MHA_BridgeDrv_TLE9563_reqClrSwkSts:
            {
                success = TLE9563_clrSwkSts();
            } break;

        case Ifx_MHA_BridgeDrv_TLE9563_reqSetAllHbx:
            {
                success = TLE9563_setAllHbx((tBDRV_setAllHbx)data);
            } break;

        case Ifx_MHA_BridgeDrv_TLE9563_reqSetCsaGain:
            {
                success = TLE9563_setCsaGain((tCSA_gain)data);
            } break;

        default:
            {
                success = 0u;
            } break;
    }

    return success;
}


static inline bool Ifx_MHA_BridgeDrv_TLE9563_actionDisable(Ifx_MHA_BridgeDrv_TLE9563* self)
{
    /* The return value represents whether the action has finished */
    bool finished;

    /* Bridge Driver not disabled yet */
    finished = false;

    /* LLD request already processed? */
    if (self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetAllHbx].processed)
    {
        /* Yes -> bridge driver can go to state off */
        finished = true;

        /* Reset queue flags */
        self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetAllHbx].requested = false;
        self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetAllHbx].processed = false;
    }
    else
    {
        /* No -> send set request to LLD */
        self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetAllHbx].requested = true;
        self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetAllHbx].data      =
            (uint16)BDRV_setAllHbx_passiveOff_afwEn_pwmInact;
    }

    return finished;
}


static inline bool Ifx_MHA_BridgeDrv_TLE9563_actionEnable(Ifx_MHA_BridgeDrv_TLE9563* self)
{
    /* The return value represents whether the action has finished */
    bool finished;

    /* Bridge Driver not disabled yet */
    finished = false;

    /* LLD request already processed? */
    if (self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetAllHbx].processed)
    {
        /* Yes -> bridge driver can go to state off */
        finished = true;

        /* Reset queue flags */
        self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetAllHbx].requested = false;
        self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetAllHbx].processed = false;
    }
    else
    {
        /* No -> set request to LLD */
        self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetAllHbx].requested = true;
        self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetAllHbx].data      =
            (uint16)BDRV_setAllHbx_hsOn_afwDis_pwmAct;
    }

    return finished;
}


static inline void Ifx_MHA_BridgeDrv_TLE9563_clearAllFaultsRequest(Ifx_MHA_BridgeDrv_TLE9563* self)
{
    /* Request clear fault from LLD for every status register */
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrSupSts].requested      = true;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrThermSts].requested    = true;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrDevSts].requested      = true;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrBusSts].requested      = true;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrWkSts].requested       = true;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrHsOlOcOtSts].requested = true;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrDsovSts].requested     = true;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrSwkSts].requested      = true;
}


static inline bool Ifx_MHA_BridgeDrv_TLE9563_clearAllFaultsCheck(Ifx_MHA_BridgeDrv_TLE9563* self)
{
    /* Status flag whether all clear fault steps are finished */
    bool clearFaultsFinished;

    /* Status flag whether all clear fault requests have been processed by the LLD */
    bool clearFaultsProcessed;
    clearFaultsFinished = false;

    if (self->p_clrFaultsStep == Ifx_MHA_BridgeDrv_TLE9563_clrFaultsWaitProc)
    {
        /* Step 1: Check all clear requests in the queue */
        clearFaultsProcessed  = true;
        clearFaultsProcessed &= self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrSupSts].processed;
        clearFaultsProcessed &= self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrThermSts].processed;
        clearFaultsProcessed &= self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrDevSts].processed;
        clearFaultsProcessed &= self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrBusSts].processed;
        clearFaultsProcessed &= self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrWkSts].processed;
        clearFaultsProcessed &= self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrHsOlOcOtSts].processed;
        clearFaultsProcessed &= self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrDsovSts].processed;
        clearFaultsProcessed &= self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrSwkSts].processed;

        if (clearFaultsProcessed)
        {
            /* Goto next step */
            self->p_clrFaultsStep = Ifx_MHA_BridgeDrv_TLE9563_clrFaultsInitCounter;
        }
    }

    if (self->p_clrFaultsStep == Ifx_MHA_BridgeDrv_TLE9563_clrFaultsInitCounter)
    {
        /* Step 2: Initialize counter */
        self->p_clrFaultsCounter = 0u;

        /* Goto next step */
        self->p_clrFaultsStep = Ifx_MHA_BridgeDrv_TLE9563_clrFaultsWaitCounter;
    }

    if (self->p_clrFaultsStep == Ifx_MHA_BridgeDrv_TLE9563_clrFaultsWaitCounter)
    {
        /* Step 3: Wait for update of all status registers */
        if (self->p_clrFaultsCounter < IFX_MHA_BRIDGEDRV_TLE9563_LLD_UPDATE_STAT_CYC)
        {
            self->p_clrFaultsCounter++;
        }
        else
        {
            clearFaultsFinished = true;
        }
    }

    return clearFaultsFinished;
}


/* polyspace-end MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */
