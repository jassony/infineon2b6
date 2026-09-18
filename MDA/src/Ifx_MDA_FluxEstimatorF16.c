/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#include "Ifx_MDA_FluxEstimatorF16.h"
#include "Ifx_MDA_FluxEstimatorF16_Cfg.h"

/* Math library includes */
#include "Ifx_Math_Atan2.h"
#include "Ifx_Math_Div.h"
#include "Ifx_Math_DivShL.h"
#include "Ifx_Math_LowPass1stF16.h"
#include "Ifx_Math_Mul.h"
#include "Ifx_Math_MulShRSat.h"
#include "Ifx_Math_PLLF16.h"
#include "Ifx_Math_Sub.h"

/*
 * Adjusted alpha and beta gains.
 * The adjustment factor is defined because usually the filter time constants are relatively large compared to the other
 * time values on the system. Due to this fact, the system base time might not be enough to normalize the filter gains
 *(which are the same as the time constants) and cause overflows.
 * If the flux amplitude needs to be calculated from SW values, this factor should be considered in the scaling.
 */

/* polyspace-begin MISRA2012:D1.1 [Justified:Low] "Behavior verified by unit tests." */
#define IFX_MDA_FLUXESTIMATORF16_CFG_ALPHA_GAIN_ADJUSTED_Q14 \
    ((Ifx_Math_Fract16)((float)                              \
                        IFX_MDA_FLUXESTIMATORF16_CFG_ALPHA_GAIN_Q14 * IFX_MDA_FLUXESTIMATORF16_CFG_ADJUSTMENT_FACTOR))

#define IFX_MDA_FLUXESTIMATORF16_CFG_BETA_GAIN_ADJUSTED_Q14 \
    ((Ifx_Math_Fract16)((float)                             \
                        IFX_MDA_FLUXESTIMATORF16_CFG_BETA_GAIN_Q14 * IFX_MDA_FLUXESTIMATORF16_CFG_ADJUSTMENT_FACTOR))

/* Macros to define the component ID */
#define IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTID_SOURCEID     ((uint8)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTID_LIBRARYID    ((uint16)Ifx_ComponentID_LibraryID_mctrlDriveAlgorithm)
#define IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTID_MODULEID     (0U)
#define IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTID_COMPONENTID1 (1U)

#define IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTID_COMPONENTID2 ((uint8)Ifx_ComponentID_ComponentID2_eco)

/* Macros to define the component version */
#define IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTVERSION_MAJOR   (1U)
#define IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTVERSION_MINOR   (2U)
#define IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTVERSION_PATCH   (0U)
#define IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTVERSION_T       (0U)
#define IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTVERSION_REV     (0U)

/* Component ID */
static const Ifx_ComponentID      Ifx_MDA_FluxEstimator_CYT2B7_componentID = {
    .sourceID                                                                =
        IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTID_SOURCEID, .libraryID        =
        IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTID_LIBRARYID, .moduleID        =
        IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTID_MODULEID,
    .componentID1                                                            =
        IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTID_COMPONENTID1, .componentID2 =
        IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_MDA_FluxEstimator_CYT2B7_componentVersion = {
    .major                                                          =
        IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTVERSION_MAJOR, .minor =
        IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTVERSION_MINOR, .patch =
        IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTVERSION_PATCH,
    .t                                                              = IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTVERSION_T,
    .rev                                                            =
        IFX_MDA_FLUXESTIMATOR_CYT2B7_COMPONENTVERSION_REV
};

/* polyspace-end MISRA2012:D1.1 [Justified:Low] "Behavior verified by unit tests." */
/* Private functions to initialize and configure filters of the FE */
void Ifx_MDA_FluxEstimatorF16_initAlphaFilter(Ifx_MDA_FluxEstimatorF16* self);
void Ifx_MDA_FluxEstimatorF16_initBetaFilter(Ifx_MDA_FluxEstimatorF16* self);
void Ifx_MDA_FluxEstimatorF16_initSpeedFilter(Ifx_MDA_FluxEstimatorF16* self);

/* Private function to calculate a rotor flux axis referred to a stator axis */
static inline Ifx_Math_Fract16 Ifx_MDA_FluxEstimatorF16_calcFlux(Ifx_MDA_FluxEstimatorF16* self,
                                                                 Ifx_Math_LowPass1stF16* filter,
                                                                 Ifx_Math_Fract16 statorVoltage,
                                                                 Ifx_Math_Fract16 statorCurrent);

/* polyspace-begin MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */

/* Function to get the component ID */
void Ifx_MDA_FluxEstimatorF16_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MDA_FluxEstimator_CYT2B7_componentID;
}


/* Function to get the component version */
void Ifx_MDA_FluxEstimatorF16_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MDA_FluxEstimator_CYT2B7_componentVersion;
}


void Ifx_MDA_FluxEstimatorF16_init(Ifx_MDA_FluxEstimatorF16* self)
{
    /* Variable to initialize the low pass and PLL gains */
    Ifx_Math_Fract16Q gain;

    /* Initialize filters */
    Ifx_MDA_FluxEstimatorF16_initAlphaFilter(self);
    Ifx_MDA_FluxEstimatorF16_initBetaFilter(self);
    Ifx_MDA_FluxEstimatorF16_initSpeedFilter(self);

    /* Initialize PLL */
    Ifx_Math_PLLF16_init(&self->p_pllFilter);

    /* Set PLL gain */
    gain.value   = IFX_MDA_FLUXESTIMATORF16_CFG_PLL_GAIN_Q;
    gain.qFormat = (Ifx_Math_FractQFormat)IFX_MDA_FLUXESTIMATORF16_CFG_PLL_GAIN_Q_FORMAT;
    Ifx_Math_PLLF16_setPropGain(&self->p_pllFilter, gain);

    self->p_phaseResistanceQ15         = IFX_MDA_FLUXESTIMATORF16_CFG_PHASE_RES_Q15;
    self->p_phaseInductanceQ15         = IFX_MDA_FLUXESTIMATORF16_CFG_PHASE_IND_Q15;
    self->p_phaseInductanceAdjustedQ15 = (Ifx_Math_Fract16)((float)self->p_phaseInductanceQ15
        * (float)IFX_MDA_FLUXESTIMATORF16_CFG_ADJUSTMENT_FACTOR);
    self->p_systemBaseTimeQ30          = IFX_MDA_FLUXESTIMATORF16_CFG_SYSTEM_BASE_TIME_Q30;

    /* Initialize sampling time from Config Wizard */
    Ifx_MDA_FluxEstimatorF16_setSamplingTime_us(self, IFX_MDA_FLUXESTIMATORF16_CFG_SAMPLING_TIME_US);

    /* Reset outputs */
    self->p_output.anglePLL = 0;
    self->p_output.speedQ15 = 0;
}


void Ifx_MDA_FluxEstimatorF16_initAlphaFilter(Ifx_MDA_FluxEstimatorF16* self)
{
    /* Initialize filters */
    Ifx_Math_LowPass1stF16_init(&(self->p_alphaFilter));

    /* Set alpha filter parameters */
    Ifx_Math_LowPass1stF16_setTimeConstant_us(&(self->p_alphaFilter), IFX_MDA_FLUXESTIMATORF16_CFG_ALPHA_TC_US);
    Ifx_Math_LowPass1stF16_setGain(&(self->p_alphaFilter), IFX_MDA_FLUXESTIMATORF16_CFG_ALPHA_GAIN_ADJUSTED_Q14);
}


void Ifx_MDA_FluxEstimatorF16_initBetaFilter(Ifx_MDA_FluxEstimatorF16* self)
{
    /* Initialize filters */
    Ifx_Math_LowPass1stF16_init(&(self->p_betaFilter));

    /* Set beta filter parameters */
    Ifx_Math_LowPass1stF16_setTimeConstant_us(&(self->p_betaFilter), IFX_MDA_FLUXESTIMATORF16_CFG_BETA_TC_US);
    Ifx_Math_LowPass1stF16_setGain(&(self->p_betaFilter), IFX_MDA_FLUXESTIMATORF16_CFG_BETA_GAIN_ADJUSTED_Q14);
}


void Ifx_MDA_FluxEstimatorF16_initSpeedFilter(Ifx_MDA_FluxEstimatorF16* self)
{
    /* Initialize filters */
    Ifx_Math_LowPass1stF16_init(&(self->p_speedFilter));

    /* Set speed filter parameters */
    Ifx_Math_LowPass1stF16_setTimeConstant_us(&(self->p_speedFilter), IFX_MDA_FLUXESTIMATORF16_CFG_SPEED_TC_US);
}


void Ifx_MDA_FluxEstimatorF16_setSamplingTime_us(Ifx_MDA_FluxEstimatorF16* self, uint32 samplingTime_us)
{
    /* Represents the sampling time in Q30 format */
    Ifx_Math_Fract32 samplingTimeQ30;

    /* Convert sampling time from microseconds to seconds, in Ifx_Math_FractQFormat_q30 */

    /* polyspace +4 MISRA2012:10.1 [Justified:Low] "Bitwise operators on signed values are required by fixed point
     * arithmetic." */
    samplingTimeQ30 = (Ifx_Math_Fract32)((((sint64)samplingTime_us) << (sint8)Ifx_Math_FractQFormat_q30) /
                                         IFX_MATH_MICROSECONDS_TO_SECONDS);

    /* Calculate the conversion factor * sampling time, in Q7 */
    self->p_radToRadPerSecondQ7 = (Ifx_Math_Fract16)Ifx_Math_DivShL_F32(
        self->p_systemBaseTimeQ30, samplingTimeQ30, (uint8)Ifx_Math_DivShL_ShiftDiv(
            Ifx_Math_FractQFormat_q30, Ifx_Math_FractQFormat_q30, Ifx_Math_FractQFormat_q7));
    self->p_samplingTime_us = samplingTime_us;

    /* Set sampling time in the dependent modules */
    Ifx_Math_LowPass1stF16_setSamplingTime_us(&(self->p_alphaFilter), samplingTime_us);
    Ifx_Math_LowPass1stF16_setSamplingTime_us(&(self->p_betaFilter), samplingTime_us);
    Ifx_Math_LowPass1stF16_setSamplingTime_us(&(self->p_speedFilter), samplingTime_us);
    Ifx_Math_PLLF16_setSamplingTime_us(&self->p_pllFilter, samplingTime_us);
}


void Ifx_MDA_FluxEstimatorF16_setMotorParameters(Ifx_MDA_FluxEstimatorF16* self,
                                                  Ifx_Math_Fract16 phaseResistanceQ15,
                                                  Ifx_Math_Fract16 phaseInductanceQ15,
                                                  Ifx_Math_Fract32 systemBaseTimeQ30)
{
    self->p_phaseResistanceQ15         = phaseResistanceQ15;
    self->p_phaseInductanceQ15         = phaseInductanceQ15;
    self->p_phaseInductanceAdjustedQ15 = (Ifx_Math_Fract16)((float)phaseInductanceQ15
        * (float)IFX_MDA_FLUXESTIMATORF16_CFG_ADJUSTMENT_FACTOR);
    self->p_systemBaseTimeQ30          = systemBaseTimeQ30;

    /* Recalculate speed normalization only; filter and PLL states remain intact. */
    Ifx_MDA_FluxEstimatorF16_setSamplingTime_us(self, self->p_samplingTime_us);
}


void Ifx_MDA_FluxEstimatorF16_execute(Ifx_MDA_FluxEstimatorF16* self, Ifx_Math_CmpFract16 statorVoltage,
                                      Ifx_Math_CmpFract16 statorCurrent)
{
    /* Local variable declaration */
    Ifx_Math_CmpFract16  rotorFlux;
    uint32               estimatedAngle;
    Ifx_Math_PLLF16_Type pllResult;
    Ifx_Math_Fract16     estimatedElecSpeed;

    /* Calculate the estimated rotor flux */
    rotorFlux.real = Ifx_MDA_FluxEstimatorF16_calcFlux(self, &self->p_alphaFilter, statorVoltage.real,
        statorCurrent.real);
    rotorFlux.imag = Ifx_MDA_FluxEstimatorF16_calcFlux(self, &self->p_betaFilter, statorVoltage.imag,
        statorCurrent.imag);

    /* Get the angle estimation from the rotor flux */
    estimatedAngle = Ifx_Math_Atan2_F16(rotorFlux.imag, rotorFlux.real);

    /* Execute the PLL filter */
    pllResult = Ifx_Math_PLLF16_execute(&self->p_pllFilter, estimatedAngle);

    /* Convert  angle derivative from PLL to normalized rad/s */
    estimatedElecSpeed = Ifx_Math_LowPass1stF16_execute(&(self->p_speedFilter), pllResult.deltaAngle / 2);

    /* Filter and output normalized speed */
    self->p_output.speedQ15 = Ifx_Math_MulShRSat_F16(estimatedElecSpeed, self->p_radToRadPerSecondQ7, 7u);
    self->p_output.anglePLL = pllResult.angle;
}


static inline Ifx_Math_Fract16 Ifx_MDA_FluxEstimatorF16_calcFlux(Ifx_MDA_FluxEstimatorF16* self,
                                                                 Ifx_Math_LowPass1stF16* filter,
                                                                 Ifx_Math_Fract16 statorVoltage,
                                                                 Ifx_Math_Fract16 statorCurrent)
{
    /* Local variable declaration */
    Ifx_Math_Fract16 resVoltage;
    Ifx_Math_Fract16 indVoltageAdjusted;
    Ifx_Math_Fract16 inducedVoltage;
    Ifx_Math_Fract16 statorFluxAdjusted;
    Ifx_Math_Fract16 rotorFluxAdjusted;

    /* U_R = I*R */
    resVoltage = Ifx_Math_Mul_F16(statorCurrent, self->p_phaseResistanceQ15);

    /* U_L = I*L */
    indVoltageAdjusted = Ifx_Math_Mul_F16(statorCurrent, self->p_phaseInductanceAdjustedQ15);

    /* Uind = Us -U_R */
    inducedVoltage = Ifx_Math_Sub_F16(statorVoltage, resVoltage);

    /* Calculate stator flux, Psi_s = integral(Uind) */
    statorFluxAdjusted = Ifx_Math_LowPass1stF16_execute(filter, inducedVoltage);

    /* Calculate the estimated rotor flux, Psi_r = Psi_s -Uind */
    rotorFluxAdjusted = Ifx_Math_Sub_F16(statorFluxAdjusted, indVoltageAdjusted);

    /* Return rotor flux */
    return rotorFluxAdjusted;
}


/* polyspace-end MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */
