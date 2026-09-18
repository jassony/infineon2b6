/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#include "Ifx_MDA_FocControllerF16.h"
#include "Ifx_MDA_FocControllerF16_Cfg.h"

#include "Ifx_Math_CartToPolar.h"
#include "Ifx_Math_Park.h"
#include "Ifx_Math_PolarToCart.h"
#include "Ifx_Math_AddSat.h"
#include "Ifx_Math_MulShR.h"
#include "Ifx_Math_ShL.h"
#include "Ifx_Math_SubSat.h"

/* Macros to define the component ID */
#define IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTID_SOURCEID     ((uint8)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTID_LIBRARYID    ((uint16)Ifx_ComponentID_LibraryID_mctrlDriveAlgorithm)
#define IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTID_MODULEID     (1U)
#define IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTID_COMPONENTID1 (1U)

#define IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTID_COMPONENTID2 ((uint8)Ifx_ComponentID_ComponentID2_eco)

/* Macros to define the component version */
#define IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTVERSION_MAJOR   (1U)
#define IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTVERSION_MINOR   (2U)
#define IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTVERSION_PATCH   (0U)
#define IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTVERSION_T       (0U)
#define IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTVERSION_REV     (0U)

/* Component ID */
static const Ifx_ComponentID      Ifx_MDA_FocController_CYT2B7_componentID = {
    .sourceID                                                                =
        IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTID_SOURCEID, .libraryID        =
        IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTID_LIBRARYID, .moduleID        =
        IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTID_MODULEID,
    .componentID1                                                            =
        IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTID_COMPONENTID1, .componentID2 =
        IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_MDA_FocController_CYT2B7_componentVersion = {
    .major                                                          =
        IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTVERSION_MAJOR, .minor =
        IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTVERSION_MINOR, .patch =
        IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTVERSION_PATCH,
    .t                                                              = IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTVERSION_T,
    .rev                                                            =
        IFX_MDA_FOCCONTROLLER_CYT2B7_COMPONENTVERSION_REV
};

#if IFX_MDA_FOCCONTROLLERF16_CFG_DQDECOUPLINGENABLE

/* Local function to execute d-q decoupling */
static inline void Ifx_MDA_FocControllerF16_dqDecoupling(Ifx_MDA_FocControllerF16* self, Ifx_Math_Fract16
                                                         electricalSpeed);
#endif

/* Local functions to initialize d and q PI controllers */
static inline void Ifx_MDA_FocControllerF16_initDPi(Ifx_MDA_FocControllerF16* self);
static inline void Ifx_MDA_FocControllerF16_initQPi(Ifx_MDA_FocControllerF16* self);
static void Ifx_MDA_FocControllerF16_trackOnePi(Ifx_Math_PiF16 *pi,
                                                 Ifx_Math_Fract16 requestedVoltageQ15,
                                                 uint16 appliedOverRequestedQ15,
                                                 uint16 trackingGainQ15);

/* polyspace-begin MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */

/* Function to get the component ID */
void Ifx_MDA_FocControllerF16_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MDA_FocController_CYT2B7_componentID;
}


/* Function to get the component version */
void Ifx_MDA_FocControllerF16_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MDA_FocController_CYT2B7_componentVersion;
}


/**
 *  Initialize the module to the default values.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 */
void Ifx_MDA_FocControllerF16_init(Ifx_MDA_FocControllerF16* self)
{
    /* Initialize internal variables and outputs to 0 */
    self->currentDQ.real                         = 0;
    self->currentDQ.imag                         = 0;
    self->voltageDQ.real                         = 0;
    self->voltageDQ.imag                         = 0;
    self->p_output.voltageCommandPolar.amplitude = 0;
    self->p_output.voltageCommandPolar.angle     = 0;

    /* Initialize Id PI controller */
    Ifx_MDA_FocControllerF16_initDPi(self);

    /* Initialize Iq PI controller */
    Ifx_MDA_FocControllerF16_initQPi(self);

    /* Initialize d-q decoupling */
#if IFX_MDA_FOCCONTROLLERF16_CFG_DQDECOUPLINGENABLE
    Ifx_Math_DqDecouplingF16_init(&(self->dqDecoupling));
    Ifx_Math_DqDecouplingF16_setInductanceD(&(self->dqDecoupling),
        IFX_MDA_FOCCONTROLLERF16_CFG_DIRECT_INDUCTANCE_Q15);
    Ifx_Math_DqDecouplingF16_setInductanceQ(&(self->dqDecoupling),
        IFX_MDA_FOCCONTROLLERF16_CFG_QUADRATURE_INDUCTANCE_Q15);
#endif
}


void Ifx_MDA_FocControllerF16_setInductances(Ifx_MDA_FocControllerF16* self,
                                              Ifx_Math_Fract16 inductanceDQ15,
                                              Ifx_Math_Fract16 inductanceQQ15)
{
#if IFX_MDA_FOCCONTROLLERF16_CFG_DQDECOUPLINGENABLE
    Ifx_Math_DqDecouplingF16_setInductanceD(&(self->dqDecoupling), inductanceDQ15);
    Ifx_Math_DqDecouplingF16_setInductanceQ(&(self->dqDecoupling), inductanceQQ15);
#else
    (void)self;
    (void)inductanceDQ15;
    (void)inductanceQQ15;
#endif
}


/**
 *  Execute the current regulation and output the voltage command.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] currentAlphaBeta Measured current, in alpha-beta reference frame
 *  \param [in] dqCommand Current command, in d-q reference frame
 *  \param [in] rotorFluxAngle Rotor flux angle
 *
 */
void Ifx_MDA_FocControllerF16_execute(Ifx_MDA_FocControllerF16* self, Ifx_Math_CmpFract16 currentAlphaBeta,
                                      Ifx_Math_CmpFract16 dqCommand, uint32 rotorFluxAngle, Ifx_Math_Fract16
                                      electricalSpeed)
{
    /* Variable declaration */
    Ifx_Math_CmpFract16 errorCurrentDQ;

    /* Current Park trans. (alpha-beta to d-q) */
    self->currentDQ = Ifx_Math_Park_F16(currentAlphaBeta, rotorFluxAngle);

    /* Calculate current errors */
    errorCurrentDQ.real = Ifx_Math_Sub_F16(Ifx_Math_ShR_F16(dqCommand.real, 1), Ifx_Math_ShR_F16(self->currentDQ.real,
        1));
    errorCurrentDQ.imag = Ifx_Math_Sub_F16(Ifx_Math_ShR_F16(dqCommand.imag, 1), Ifx_Math_ShR_F16(self->currentDQ.imag,
        1));

    /* Execute current PI controllers */
    self->voltageDQ.real = Ifx_Math_PiF16_execute(&(self->currentDPi), errorCurrentDQ.real);
    self->voltageDQ.imag = Ifx_Math_PiF16_execute(&(self->currentQPi), errorCurrentDQ.imag);

    /* Execute d-q decoupling */
#if IFX_MDA_FOCCONTROLLERF16_CFG_DQDECOUPLINGENABLE
    Ifx_MDA_FocControllerF16_dqDecoupling(self, electricalSpeed);
#endif

    /* Voltage cartesian to polar (d-q to angle-amp) */
    self->p_output.voltageCommandPolar = Ifx_Math_CartToPolar_F16(self->voltageDQ);

    /* Add rotor flux angle to convert from d-q to alpha-beta */
    self->p_output.voltageCommandPolar.angle += rotorFluxAngle;
}


void Ifx_MDA_FocControllerF16_trackAppliedVoltageScale(
    Ifx_MDA_FocControllerF16 *self,
    const uint16 appliedOverRequestedQ15,
    const uint16 trackingGainQ15)
{
    if ((appliedOverRequestedQ15 >= 32768u)
        || (trackingGainQ15 == 0u))
    {
        return;
    }

    Ifx_MDA_FocControllerF16_trackOnePi(&(self->currentDPi),
        self->voltageDQ.real, appliedOverRequestedQ15, trackingGainQ15);
    Ifx_MDA_FocControllerF16_trackOnePi(&(self->currentQPi),
        self->voltageDQ.imag, appliedOverRequestedQ15, trackingGainQ15);
}


/* The PI integrator has Q9 guard bits above its Q15 output domain. Keep the
 * external tracking error in that same internal domain and use the exact
 * Kaw*Ts scaling selected by the PI. This makes a DC-link clamp an additive
 * back-calculation term rather than a lossy overwrite of the PI state. */
static void Ifx_MDA_FocControllerF16_trackOnePi(
    Ifx_Math_PiF16 *pi,
    const Ifx_Math_Fract16 requestedVoltageQ15,
    const uint16 appliedOverRequestedQ15,
    const uint16 trackingGainQ15)
{
    const Ifx_Math_Fract16 appliedVoltageQ15 = Ifx_Math_MulShR_F16(
        requestedVoltageQ15, (Ifx_Math_Fract16)appliedOverRequestedQ15, 15u);
    const Ifx_Math_Fract32 voltageErrorQ24 = Ifx_Math_ShL_F32(
        (Ifx_Math_Fract32)appliedVoltageQ15
        - (Ifx_Math_Fract32)requestedVoltageQ15,
        (uint8)Ifx_Math_FractQFormat_q9);
    Ifx_Math_Fract32 correctionQ24;

    if ((voltageErrorQ24 == 0)
        || (pi->p_antiWindupGainSamplingTime.value <= 0))
    {
        return;
    }

    correctionQ24 = Ifx_Math_MulShR_F32(
        (Ifx_Math_Fract32)pi->p_antiWindupGainSamplingTime.value,
        voltageErrorQ24, pi->p_qFormatAntiWindupGain);
    correctionQ24 = Ifx_Math_MulShR_F32(correctionQ24,
        (Ifx_Math_Fract32)trackingGainQ15, 15u);
    pi->p_integPreviousValue = Ifx_Math_AddSat_F32(pi->p_integPreviousValue,
        correctionQ24);
}


void Ifx_MDA_FocControllerF16_reset(Ifx_MDA_FocControllerF16* self)
{
    /* Reset intermediate variables and outputs */
    self->currentDQ.real                         = 0;
    self->currentDQ.imag                         = 0;
    self->voltageDQ.real                         = 0;
    self->voltageDQ.imag                         = 0;
    self->p_output.voltageCommandPolar.amplitude = 0;
    self->p_output.voltageCommandPolar.angle     = 0;

    /* Reset PI controllers previous values */
    Ifx_Math_PiF16_setIntegPreviousValue(&(self->currentDPi), 0);
    Ifx_Math_PiF16_setIntegPreviousValue(&(self->currentQPi), 0);
}


/* Functions called by Ifx_MDA_FocControllerF16_init() */
static inline void Ifx_MDA_FocControllerF16_initDPi(Ifx_MDA_FocControllerF16* self)
{
    /* Set Q formats */
    Ifx_Math_PiF16_Qformats currentDPiQForm;
    currentDPiQForm.qFormatPropGain                   =
        (Ifx_Math_FractQFormat)IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_PROPGAIN_Q_FORMAT;
    currentDPiQForm.qFormatIntegGainSamplingTime      =
        (Ifx_Math_FractQFormat)IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_KI_TS_Q_FORMAT;
    currentDPiQForm.qFormatAntiWindupGainSamplingTime =
        (Ifx_Math_FractQFormat)IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_KAW_TS_Q_FORMAT;
    currentDPiQForm.qFormatOutput                     =
        (Ifx_Math_FractQFormat)IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_LIMIT_Q_FORMAT;
    currentDPiQForm.qFormatError                      = Ifx_Math_FractQFormat_q14;

    /* Call init */
    Ifx_Math_PiF16_init(&(self->currentDPi), currentDPiQForm);

    /* Call setters */
    Ifx_Math_PiF16_setPropGain(&(self->currentDPi), IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_PROPGAIN_Q);
    Ifx_Math_PiF16_setIntegGainSamplingTime(&(self->currentDPi), IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_KI_TS_Q);
    Ifx_Math_PiF16_setAntiWindupGainSamplingTime(&(self->currentDPi), IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_KAW_TS_Q);
    Ifx_Math_PiF16_setUpperLimit(&(self->currentDPi), IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_OUT_UPP_LIMIT_Q);
    Ifx_Math_PiF16_setLowerLimit(&(self->currentDPi), IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_OUT_LOW_LIMIT_Q);
}


static inline void Ifx_MDA_FocControllerF16_initQPi(Ifx_MDA_FocControllerF16* self)
{
    /* Set Q formats */
    Ifx_Math_PiF16_Qformats currentQPiQForm;
    currentQPiQForm.qFormatPropGain                   =
        (Ifx_Math_FractQFormat)IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_PROPGAIN_Q_FORMAT;
    currentQPiQForm.qFormatIntegGainSamplingTime      =
        (Ifx_Math_FractQFormat)IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_KI_TS_Q_FORMAT;
    currentQPiQForm.qFormatAntiWindupGainSamplingTime =
        (Ifx_Math_FractQFormat)IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_KAW_TS_Q_FORMAT;
    currentQPiQForm.qFormatOutput                     =
        (Ifx_Math_FractQFormat)IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_LIMIT_Q_FORMAT;
    currentQPiQForm.qFormatError                      = Ifx_Math_FractQFormat_q14;

    /* Call init */
    Ifx_Math_PiF16_init(&(self->currentQPi), currentQPiQForm);

    /* Call setters */
    Ifx_Math_PiF16_setPropGain(&(self->currentQPi), IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_PROPGAIN_Q);
    Ifx_Math_PiF16_setIntegGainSamplingTime(&(self->currentQPi), IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_KI_TS_Q);
    Ifx_Math_PiF16_setAntiWindupGainSamplingTime(&(self->currentQPi), IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_KAW_TS_Q);
    Ifx_Math_PiF16_setUpperLimit(&(self->currentQPi), IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_OUT_UPP_LIMIT_Q);
    Ifx_Math_PiF16_setLowerLimit(&(self->currentQPi), IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_OUT_LOW_LIMIT_Q);
}


#if IFX_MDA_FOCCONTROLLERF16_CFG_DQDECOUPLINGENABLE

/* Functions called by Ifx_MDA_FocControllerF16_execute() */
static inline void Ifx_MDA_FocControllerF16_dqDecoupling(Ifx_MDA_FocControllerF16* self, Ifx_Math_Fract16
                                                         electricalSpeed)
{
    /* Calculate and apply d-q decoupling */
    Ifx_Math_CmpFract16 compensationVoltageDQ = Ifx_Math_DqDecouplingF16_execute(&(self->dqDecoupling),
        self->currentDQ, electricalSpeed);
    self->voltageDQ.real = Ifx_Math_SubSat_F16(self->voltageDQ.real, compensationVoltageDQ.real);
    self->voltageDQ.imag = Ifx_Math_AddSat_F16(self->voltageDQ.imag, compensationVoltageDQ.imag);
}


#endif

/* polyspace-end MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */
