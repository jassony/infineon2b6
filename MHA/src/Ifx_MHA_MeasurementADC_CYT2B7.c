/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

/* Module includes */
#include "Ifx_MHA_MeasurementADC_CYT2B7.h"
#include "Ifx_MHA_MeasurementADC_Cfg.h"

/* Math library includes */
#include "Ifx_Math_MulShRSat.h"

/* SDL includes */
#include "cy_project.h"
#include "cy_device_headers.h"
#include "SDL_Adc_Cfg.h"

/* Macros to define the component ID */
#define IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTID_SOURCEID     ((uint8)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTID_LIBRARYID \
    ((uint16)Ifx_ComponentID_LibraryID_mctrlHardwareAbstraction)

#define IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTID_MODULEID     (1U)
#define IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTID_COMPONENTID1 (3U)

#define IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTID_COMPONENTID2 ((uint8)Ifx_ComponentID_ComponentID2_eco)

/* Macros to define the component version */
#define IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTVERSION_MAJOR   (1U)
#define IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTVERSION_MINOR   (2U)
#define IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTVERSION_PATCH   (0U)
#define IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTVERSION_T       (0U)
#define IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTVERSION_REV     (0U)
_ST_VDC_PARA g_vdc_para;
      
/* *INDENT-OFF* */
/* Component ID */
static const Ifx_ComponentID Ifx_MHA_MeasurementADC_CYT2B7_componentID =
{
    .sourceID = IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTID_SOURCEID,
    .libraryID = IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTID_LIBRARYID,
    .moduleID = IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTID_MODULEID,
    .componentID1 = IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTID_COMPONENTID1,
    .componentID2 = IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_MHA_MeasurementADC_CYT2B7_componentVersion =
{
    .major = IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTVERSION_MAJOR,
    .minor = IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTVERSION_MINOR,
    .patch = IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTVERSION_PATCH,
    .t = IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTVERSION_T,
    .rev = IFX_MHA_MEASUREMENTADC_CYT2B7_COMPONENTVERSION_REV
};
/* *INDENT-ON* */
/* Sub-function to execute ADC calibration state */
static Ifx_MHA_MeasurementADC_CYT2B7_State Ifx_MHA_MeasurementADC_CYT2B7_stateCalib(
    Ifx_MHA_MeasurementADC_CYT2B7* self);

/* DC link voltage and shunt currents calculation*/
static inline void Ifx_MHA_MeasurementADC_CYT2B7_calc(Ifx_MHA_MeasurementADC_CYT2B7* self);

/* polyspace-begin MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */

/* polyspace-begin MISRA2012:5.1 [Justified:Low] "Violation is justified because all supported compilers can handle
 * notably more significant initial characters in identifiers than required in this case and the readability is
 * ensured." */

/* Function to get the component ID */
void Ifx_MHA_MeasurementADC_CYT2B7_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MHA_MeasurementADC_CYT2B7_componentID;
}


/* Function to get the component version */
void Ifx_MHA_MeasurementADC_CYT2B7_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MHA_MeasurementADC_CYT2B7_componentVersion;
}


/* polyspace-end MISRA2012:5.1 [Justified:Low] "Violation is justified because all supported compilers can handle
 * notably more significant initial characters in identifiers than required in this case and the readability is
 * ensured." */
void Ifx_MHA_MeasurementADC_CYT2B7_init(Ifx_MHA_MeasurementADC_CYT2B7* self)
{
    self->p_output.sampleValid = false;
    /* Reset variables used to store current measurements */
    self->p_rawCurrentMeasurements[0] = 0u;
    self->p_rawCurrentMeasurements[1] = 0u;

    /* Reset outputs */
    self->p_output.dcLinkVoltageQ15    = 0;
    self->p_output.shuntCurrentsQ15[0] = 0;
    self->p_output.shuntCurrentsQ15[1] = 0;

    /* Reset module parameters */
    self->p_currentAccumulator[0]                   = 0u;
    self->p_currentAccumulator[1]                   = 0u;
    self->p_cycleCounter                         = 1u;
    self->p_offset[0]                               = 0u;
    self->p_offset[1]                               = 0u;
    self->_Super_Ifx_MHA_MeasurementADC.p_enable = false;

    /* Initialize state */
    self->p_status.state = Ifx_MHA_MeasurementADC_CYT2B7_State_init;

    /* Initialize current and CSA gains */
    Ifx_MHA_MeasurementADC_CYT2B7_setCsaGain(self,
        (Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain)IFX_MHA_MEASUREMENTADC_CFG_CSA_GAIN);
}


void Ifx_MHA_MeasurementADC_CYT2B7_execute(Ifx_MHA_MeasurementADC_CYT2B7* self)
{
    /* Variable to store the state */
    Ifx_MHA_MeasurementADC_CYT2B7_State previousState = self->p_status.state;
    Ifx_MHA_MeasurementADC_CYT2B7_State nextState     = previousState;
    self->p_output.sampleValid = false;

    switch (previousState)
    {
        /* Initialize ADC and CSA */
        case Ifx_MHA_MeasurementADC_CYT2B7_State_init:

            /* Set the state to OFF */
            nextState = Ifx_MHA_MeasurementADC_CYT2B7_State_off;
            break;

        /* Current sense amplifier powered on and ADC measuring outputs */
        case Ifx_MHA_MeasurementADC_CYT2B7_State_on:

            /* Check if module disabled */
            if (self->_Super_Ifx_MHA_MeasurementADC.p_enable == false)
            {
                /* Set the state to OFF */
                nextState = Ifx_MHA_MeasurementADC_CYT2B7_State_off;
            }
            else
            {
                /* DC link voltage and shunt currents calculation */
                Ifx_MHA_MeasurementADC_CYT2B7_calc(self);
            }

            break;

        /* Current sense amplifier powered off */
        case Ifx_MHA_MeasurementADC_CYT2B7_State_off:

            /* Check if module enabled */
            if (self->_Super_Ifx_MHA_MeasurementADC.p_enable == true)
            {
                /* Reset module parameters for calibration */
                self->p_currentAccumulator[0] = 0u;
                self->p_currentAccumulator[1] = 0u;
                self->p_cycleCounter       = 1u;

                /* Trigger conversion */
                Cy_Adc_Channel_SoftwareTrigger(&ADC_SAR_NUM_CC0->CH[ADC_CHN_NUM_CC0]);
                Cy_Adc_Channel_SoftwareTrigger(&ADC_SAR_NUM_CC1->CH[ADC_CHN_NUM_CC1]);
                nextState = Ifx_MHA_MeasurementADC_CYT2B7_State_calibration;
            }

            break;

        /* Calibration of the current sense amplifier */
        case Ifx_MHA_MeasurementADC_CYT2B7_State_calibration:
            nextState = Ifx_MHA_MeasurementADC_CYT2B7_stateCalib(self);
            break;

        default:

            /* do default transition to INIT */
            nextState = Ifx_MHA_MeasurementADC_CYT2B7_State_init;
            break;
    }

    self->p_status.state = nextState;
}


/* Sub-function to execute ADC calibration state */
static Ifx_MHA_MeasurementADC_CYT2B7_State Ifx_MHA_MeasurementADC_CYT2B7_stateCalib(
    Ifx_MHA_MeasurementADC_CYT2B7* self)
{
    /* Temporary ADC channel status */
    cy_stc_adc_ch_status_t              adcChStatus[2];

    /* Temporary ADC channel result */
    uint16                              adcChResult[2];

    /* Variable to store the state */
    Ifx_MHA_MeasurementADC_CYT2B7_State previousState = self->p_status.state;
    Ifx_MHA_MeasurementADC_CYT2B7_State nextState     = previousState;
    self->p_output.sampleValid = false;

    /* Accumulate the input current */
    Cy_Adc_Channel_GetResult(&ADC_SAR_NUM_CC0->CH[ADC_CHN_NUM_CC0], &adcChResult[0], &adcChStatus[0]);
    Cy_Adc_Channel_GetResult(&ADC_SAR_NUM_CC1->CH[ADC_CHN_NUM_CC1], &adcChResult[1], &adcChStatus[1]);

    if (adcChStatus[0].valid && adcChStatus[1].valid)
    {
        self->p_currentAccumulator[0] += (uint32)adcChResult[0];
        self->p_currentAccumulator[1] += (uint32)adcChResult[1];
    }

    /* Calibration is still ongoing */
    if (self->p_cycleCounter < IFX_MHA_MEASUREMENTADC_CFG_CALIBRATION_CYCLES)
    {
        /* Trigger another conversion */
        Cy_Adc_Channel_SoftwareTrigger(&ADC_SAR_NUM_CC0->CH[ADC_CHN_NUM_CC0]);
        Cy_Adc_Channel_SoftwareTrigger(&ADC_SAR_NUM_CC1->CH[ADC_CHN_NUM_CC1]);
        self->p_cycleCounter++;
    }

    /* Calibration is done, get the average current */
    else
    {
        /* Get the average value */
        self->p_offset[0] = (uint16)(self->p_currentAccumulator[0] / self->p_cycleCounter);
        self->p_offset[1] = (uint16)(self->p_currentAccumulator[1] / self->p_cycleCounter);
        /* Automatic transition to ON */
        nextState = Ifx_MHA_MeasurementADC_CYT2B7_State_on;
    }

    return nextState;
}

//uint32_t vdc;
//uint8_t vdc_underv = 0;
static inline void Ifx_MHA_MeasurementADC_CYT2B7_calc(Ifx_MHA_MeasurementADC_CYT2B7* self)
{
    cy_stc_adc_ch_status_t status[3];
    uint16 raw[3];
    uint8 channel;
    self->p_output.sampleValid = false;
    /* Read before conversion/scaling work. Publish all three together so a
     * missing shunt can never mix a new phase with an old phase. Physical
     * same-cycle trigger/read separation remains a bench timing gate. */
    Cy_Adc_Channel_GetResult(&ADC_SAR_NUM_VDC->CH[ADC_CHN_NUM_VDC], &raw[2], &status[2]);
    Cy_Adc_Channel_GetResult(&ADC_SAR_NUM_CC0->CH[ADC_CHN_NUM_CC0], &raw[0], &status[0]);
    Cy_Adc_Channel_GetResult(&ADC_SAR_NUM_CC1->CH[ADC_CHN_NUM_CC1], &raw[1], &status[1]);
    if (!(status[0].valid && status[1].valid && status[2].valid))
    {
        return;
    }
    self->p_output.dcLinkVoltageQ15 = Ifx_Math_MulShRSat_F16((Ifx_Math_Fract16)raw[2],
        IFX_MHA_MEASUREMENTADC_CFG_CONVERT_VDC_TO_Q15, IFX_MHA_MEASUREMENTADC_CFG_VDC_SHIFT_FACTOR);
    for (channel = 0u; channel < 2u; ++channel)
    {
        const sint16 withoutOffset = (sint16)raw[channel] - (sint16)self->p_offset[channel];
        self->p_rawCurrentMeasurements[channel] = raw[channel];
        self->p_output.shuntCurrentsQ15[channel] = Ifx_Math_MulShRSat_F16(withoutOffset,
            self->p_currentGain.value, (uint8)self->p_currentGain.qFormat);
    }
    self->p_output.sampleValid = true;
}


/* polyspace-end MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */
