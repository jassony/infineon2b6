/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

/**
 * \file Ifx_MHA_BridgeDrv_TLE9563.h
 * \brief A specialized bridge driver module for the TLE9563x devices.
 */

#ifndef IFX_MHA_BRIDGEDRV_TLE9563_H
#define IFX_MHA_BRIDGEDRV_TLE9563_H

#include "Ifx_ComponentID.h"
#include "Ifx_ComponentVersion.h"

#include "Ifx_MHA_BridgeDrv.h"

#include "TLE9563_FuncLayer.h"

/**
 * Bridge driver state options
 */
typedef enum Ifx_MHA_BridgeDrv_TLE9563_State
{
    Ifx_MHA_BridgeDrv_TLE9563_State_init  = 0, /**<Bridge driver is initializing. Drivers are powered off at the end of
                                                * initialization.*/
    Ifx_MHA_BridgeDrv_TLE9563_State_off   = 1, /**<Bridge driver is disabled. Drivers are powered off.*/
    Ifx_MHA_BridgeDrv_TLE9563_State_on    = 2, /**<Bridge driver is enabled. Drivers are controlled by external input.*/
    Ifx_MHA_BridgeDrv_TLE9563_State_fault = 3  /**<Bridge Driver is in fault. Drivers are in the state configured by
                                                * faultOutputBehavior.*/
} Ifx_MHA_BridgeDrv_TLE9563_State;

/**
 * Status of the bridge driver, containing the bit coded errors and the state machine state
 */
typedef struct Ifx_MHA_BridgeDrv_TLE9563_Status
{
    /**
     * State of the bridge driver
     */
    Ifx_MHA_BridgeDrv_TLE9563_State state;

    /**
     * Error information of the TLE9563 device driver
     */
    uint8_t deviceDriverErrorLog;

    /**
     * SPI error status flag
     */
    bool spiFault;

    /**
     * TLE9563 initialization error status flag
     */
    bool TLE9563InitFault;

    /**
     * Overcurrent status flag
     * This flag is set to true if at least one of the following bits in TLE9563 is set:
     * <ul>
     *  <li>register HS_OL_OC_OT_STAT bit 2 (HS3_OC): Overcurrent Detection on HS3;</li>
     *  <li>register HS_OL_OC_OT_STAT bit 1 (HS2_OC): Overcurrent Detection on HS2;</li>
     *  <li>register HS_OL_OC_OT_STAT bit 0 (HS1_OC): Overcurrent Detection on HS1;</li>
     *  <li>register DSOV bit 14 (OC_CSA): CSA Overcurrent detection;</li>
     * </ul>
     */
    bool overcurrent;

    /**
     * Overvoltage status flag
     * This flag is set to true if at least one of the following bits in TLE9563 is set:
     * <ul>
     *  <li>register SUP_STAT bit 9 (HS_OV): HS Supply OV-Detection;</li>
     *  <li>register SUP_STAT bit 7 (VSINT_OV): VSINT OV-Detection;</li>
     *  <li>register SUP_STAT bit 5 (VS_OV): VS Overvoltage Detection;</li>
     *  <li>register SUP_STAT bit 1 (VCC1_OV): VCC1 Overvoltage Detection;</li>
     *  <li>register DSOV bit 13 (VSINTOVBRAKE_ST): VSINT Brake status;</li>
     *  <li>register DSOV bit 12 (VSOVBRAKE_ST): VS Brake status;</li>
     *  <li>register DSOV bit 10 (LS3DSOV_BRK): Drain-source overvoltage on low-side 3 during braking;</li>
     *  <li>register DSOV bit 9 (LS2DSOV_BRK): Drain-source overvoltage on low-side 2 during braking;</li>
     *  <li>register DSOV bit 8 (LS1DSOV_BRK): Drain-source overvoltage on low-side 1 during braking;</li>
     *  <li>register DSOV bit 5 (LS3DSOV): Drain-source overvoltage on low-side 3;</li>
     *  <li>register DSOV bit 4 (LS3DSOV): Drain-source overvoltage on high-side 3;</li>
     *  <li>register DSOV bit 3 (LS3DSOV): Drain-source overvoltage on low-side 2;</li>
     *  <li>register DSOV bit 2 (LS3DSOV): Drain-source overvoltage on high-side 2;</li>
     *  <li>register DSOV bit 1 (LS3DSOV): Drain-source overvoltage on low-side 1;</li>
     *  <li>register DSOV bit 0 (LS3DSOV): Drain-source overvoltage on high-side 1;</li>
     * </ul>
     */
    bool overvoltage;

    /**
     * Undervoltage status flag
     * This flag is set to true if at least one of the following bits in TLE9563 is set:
     * <ul>
     *  <li>register SUP_STAT bit 11 (VCC1_UV_FS): 4th consecutive VCC1 UV-Detection;</li>
     *  <li>register SUP_STAT bit 10 (HS_UV): HS Supply UV-Detection;</li>
     *  <li>register SUP_STAT bit 8 (VSINT_UV): VSINT UV-Detection;</li>
     *  <li>register SUP_STAT bit 6 (VS_UV): VS Undervoltage Detection (VS,uv);</li>
     *  <li>register SUP_STAT bit 4 (CP_UV): CP_UV;</li>
     *  <li>register SUP_STAT bit 2 (VCC1_UV): VCC1 UV-Detection (due to Vrtx reset);</li>
     * </ul>
     */
    bool undervoltage;
} Ifx_MHA_BridgeDrv_TLE9563_Status;

/**
 * Structure of one queue element which represents one request to the LLD.
 */
typedef struct Ifx_MHA_BridgeDrv_TLE9563_QueueElement
{
    /**
     * Request-specific data to be sent to the LLD (only used for set requests).
     */
    uint16 data;

    /**
     * Flag which represents that the execution of this queue element is requested.
     */
    bool requested;

    /**
     * Flag which signalizes that the request of this queue element has been processed.
     */
    bool processed;
} Ifx_MHA_BridgeDrv_TLE9563_QueueElement;

/* *INDENT-OFF* */
/**
 * List of possible requests to the LLD sorted by priority (high -> low)
 */
typedef enum
{
    Ifx_MHA_BridgeDrv_TLE9563_reqServeWD  = 0u, /**< Request to LLD for serving the Watchdog. */
    Ifx_MHA_BridgeDrv_TLE9563_reqClrSupSts  = 1u, /**< Request to LLD for clearing register SupSts. */
    Ifx_MHA_BridgeDrv_TLE9563_reqClrThermSts = 2u, /**< Request to LLD for clearing register ThermSts. */
    Ifx_MHA_BridgeDrv_TLE9563_reqClrDevSts  = 3u, /**< Request to LLD for clearing register DevSts. */
    Ifx_MHA_BridgeDrv_TLE9563_reqClrBusSts = 4u, /**< Request to LLD for clearing register BusSts. */
    Ifx_MHA_BridgeDrv_TLE9563_reqClrWkSts   = 5u, /**< Request to LLD for clearing register WkSts. */
    Ifx_MHA_BridgeDrv_TLE9563_reqClrHsOlOcOtSts = 6u, /**< Request to LLD for clearing register HsOlOcOtSts. */
    Ifx_MHA_BridgeDrv_TLE9563_reqClrDsovSts = 7u, /**< Request to LLD for clearing register DsovSts. */
    Ifx_MHA_BridgeDrv_TLE9563_reqClrSwkSts = 8u, /**< Request to LLD for clearing register SwkSts. */
    Ifx_MHA_BridgeDrv_TLE9563_reqSetAllHbx  = 9u, /**< Request to LLD for enabling/disabling all half-bridges. */
    Ifx_MHA_BridgeDrv_TLE9563_reqSetCsaGain = 10u, /**< Request to LLD for setting a new CSA gain. */
    Ifx_MHA_BridgeDrv_TLE9563_reqNumTotal   = 11u  /**< Total number of possible requests to the LLD. */
} Ifx_MHA_BridgeDrv_TLE9563_requestsToLLD;

/**
 * List of steps before a clear fault can be considered as finished
 */
typedef enum
{
    Ifx_MHA_BridgeDrv_TLE9563_clrFaultsWaitProc = 0u,      /**< Wait for the clear fault requests to be processed by
                                                             *   the LLD. */
    Ifx_MHA_BridgeDrv_TLE9563_clrFaultsInitCounter = 1u,   /**< Initialize the clrFaultsCounter. */
    Ifx_MHA_BridgeDrv_TLE9563_clrFaultsWaitCounter = 2u    /**< Wait for the clrFaultsCounter to reach its
                                                             *   threshold. */
} Ifx_MHA_BridgeDrv_TLE9563_clrFaultsSteps;
/* *INDENT-ON* */

/**
 * \brief Data structure that stores all data of module instance.
 *
 */
typedef struct Ifx_MHA_BridgeDrv_TLE9563
{
    /**
     * Structure inherited from Ifx_MHA_BridgeDrv
     */
    Ifx_MHA_BridgeDrv _Super_Ifx_MHA_BridgeDrv;

    /**
     * Status of the bridge driver, containing the bit coded errors and the state machine state.
     */
    Ifx_MHA_BridgeDrv_TLE9563_Status p_status;

    /**
     * List of queue elements (one per possible request to the LLD)
     */
    Ifx_MHA_BridgeDrv_TLE9563_QueueElement p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqNumTotal];

    /**
     * Counter of the bridge driver execution cycles
     */
    uint32 p_cycleCounter;

    /**
     * Threshold of the cycle counter at which the watchdog of TLE9563 must be triggered
     */
    uint32 p_wdTriggerThreshold;

    /**
     * Holds the counter which is used while waiting for all faults to be cleared
     */
    uint8 p_clrFaultsCounter;

    /**
     * Holds the current step while waiting for all faults to be cleared
     */
    Ifx_MHA_BridgeDrv_TLE9563_clrFaultsSteps p_clrFaultsStep;
} Ifx_MHA_BridgeDrv_TLE9563;

/**
 * CSA gain options
 */
typedef enum Ifx_MHA_BridgeDrv_TLE9563_optionCsaGain
{
    Ifx_MHA_BridgeDrv_TLE9563_optionCsaGain_10 = 0, /**<CSA gain 10*/
    Ifx_MHA_BridgeDrv_TLE9563_optionCsaGain_20 = 1, /**<CSA gain 20*/
    Ifx_MHA_BridgeDrv_TLE9563_optionCsaGain_40 = 2, /**<CSA gain 40*/
    Ifx_MHA_BridgeDrv_TLE9563_optionCsaGain_60 = 3  /**<CSA gain 60*/
} Ifx_MHA_BridgeDrv_TLE9563_optionCsaGain;

/**
 *  \brief Advance the state machine of the module.
 *
 *  This API handles the state machine of the module, as described by the available state-diagram. Additionally, it
 * handles the faults as specified per configuration.
 *
 *  Inputs of this API:
 *  <ul>
 *      <li>HW faults, that are read via the SDK function from the bridge driver fault detection status HW register
 * BDRV_IS</li>
 *      <li>Only the faults which are configured for FAULT_REPORT_REACT are considered by fault detection (Bridge
 * drivers overcurrent, overvoltage and undervoltage)</li>
 *      <li>State machine "Enable" input, boolean data that is Enabled/Disabled by a call to the
 * Ifx_BDrv_TLE9563_enable(true/false)</li>
 *  </ul>
 *
 *  Outputs of this API:
 *  <ul>
 *      <li>State variable: encoded on the "Status" output</li>
 *      <li>Error code: encoded on the "Status" output</li>
 *  </ul>
 *
 *  These outputs are set corresponding to the states and state transitions design. A fault, if it is not disabled, will
 * be detected and the configured action will be taken within 3 calls to this function.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 */
void Ifx_MHA_BridgeDrv_TLE9563_execute(Ifx_MHA_BridgeDrv_TLE9563* self);

/**
 *  \brief Initialize the bridge driver to the default settings.
 *
 *  The peripheral initialization should be done by the user before calling this function.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 */
void Ifx_MHA_BridgeDrv_TLE9563_init(Ifx_MHA_BridgeDrv_TLE9563* self);

/**
 *  \brief Returns the component ID
 *
 *
 *  \param [out] *componentID Variable to store the address of the component ID
 *
 */
void Ifx_MHA_BridgeDrv_TLE9563_getID(const Ifx_ComponentID** componentID);

/**
 *  \brief Returns the component version
 *
 *
 *  \param [out] *componentVersion Variable to store the address of the component version
 *
 */
void Ifx_MHA_BridgeDrv_TLE9563_getVersion(const Ifx_ComponentVersion** componentVersion);

/**
 *  Set the gain of the operational amplifier
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] csaGain CSA gain setting
 *
 */
static inline void Ifx_MHA_BridgeDrv_TLE9563_setCsaGain(Ifx_MHA_BridgeDrv_TLE9563                   * self,
                                                        Ifx_MHA_BridgeDrv_TLE9563_optionCsaGain const csaGain)
{
    /* Set request to LLD */
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetCsaGain].requested = true;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetCsaGain].processed = false;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqSetCsaGain].data      = (uint16)csaGain;
}


/**
 *  Get the gain of the operational amplifier
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return CSA gain setting in the hardware
 */
static inline Ifx_MHA_BridgeDrv_TLE9563_optionCsaGain Ifx_MHA_BridgeDrv_TLE9563_getCsaGain(
    Ifx_MHA_BridgeDrv_TLE9563* self)
{
    return (Ifx_MHA_BridgeDrv_TLE9563_optionCsaGain)TLE9563_getCsaGain();
}


/**
 *  \brief Get the status of the bridge driver, containing the state machine state and the bit coded errors.
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Bridge driver status
 */
static inline Ifx_MHA_BridgeDrv_TLE9563_Status Ifx_MHA_BridgeDrv_TLE9563_getStatus(Ifx_MHA_BridgeDrv_TLE9563* self)
{
    return self->p_status;
}


/**
 *  \brief Clears all faults from the module.
 * The module will be set to enable or disable state in the next call to execute() depending on the enable setting.
 * New hardware fault will lead the module to fault state again.
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \note
 *  Inherited from Ifx_MHA_BridgeDrv
 */
static inline void Ifx_MHA_BridgeDrv_TLE9563_clearFault(Ifx_MHA_BridgeDrv_TLE9563* self)
{
    /* Set module internal clear fault */
    Ifx_MHA_BridgeDrv_clearFault(&(self->_Super_Ifx_MHA_BridgeDrv));

    /* Reset the queue requests */
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrSupSts].processed      = false;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrThermSts].processed    = false;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrDevSts].processed      = false;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrBusSts].processed      = false;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrWkSts].processed       = false;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrHsOlOcOtSts].processed = false;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrDsovSts].processed     = false;
    self->p_queueElement[Ifx_MHA_BridgeDrv_TLE9563_reqClrSwkSts].processed      = false;

    /* Start with the first step of the clear fault process */
    self->p_clrFaultsStep = Ifx_MHA_BridgeDrv_TLE9563_clrFaultsWaitProc;
}


/**
 *  \brief Check if a clear fault request is pending.
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return True if a clear fault request was made but still not processed by the module and False otherwise
 *  \note
 *  Inherited from Ifx_MHA_BridgeDrv
 */
static inline bool Ifx_MHA_BridgeDrv_TLE9563_clearFaultIsPending(Ifx_MHA_BridgeDrv_TLE9563* self)
{
    return Ifx_MHA_BridgeDrv_clearFaultIsPending(&(self->_Super_Ifx_MHA_BridgeDrv));
}


/**
 *  \brief This API enables or disables the module based on the input parameter enable.
 * If the input parameter is TRUE and no fault is detected, then the module will be enabled in the next call to
 * execute().
 * This API also set the HW-related members of the internal "self" data structure to the corresponding HW registers
 * according to the user manual.
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] enable Parameter of boolean type to enable or disable the module
 *
 *  \note
 *  Inherited from Ifx_MHA_BridgeDrv
 */
static inline void Ifx_MHA_BridgeDrv_TLE9563_enable(Ifx_MHA_BridgeDrv_TLE9563* self, bool enable)
{
    Ifx_MHA_BridgeDrv_enable(&(self->_Super_Ifx_MHA_BridgeDrv), enable);
}


#endif /*IFX_MHA_BRIDGEDRV_TLE9563_H*/
