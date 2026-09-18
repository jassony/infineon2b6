/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

/**
 * \file Ifx_MHA_MeasurementADC_CYT2B7.h
 * \brief A specialized measurement ADC module for CYT2B7x devices.
 * This module provides the initialization, configuration, calibration and measurement routines for the measurement ADC,
 * including the  current sense amplifier.
 *
 * After an enable request, the offset calibration is started and the ADC is configured in software mode. This means
 * that the sequencer is stopped and the ADC cannot be used to measure other channels.
 * The calibration routine averages the current measurements performed in the number of configured calibration cycles.
 * During the calibration, the current measurement received by the module should be measured when there is no current
 * flowing through the shunt (0V differential voltage in the amplifier input).
 * When the calibration is finished, the module transitions automatically to ON and the ADC runs in the sequencer mode.
 *
 * While in the ON state, the outputs are the DC link voltage and the two shunt current measurements, which can be used
 * by other modules to reconstruct the phase current values. Both of the inputs are represented in Q15 and normalized by
 * the base voltage/current.
 */

#ifndef IFX_MHA_MEASUREMENTADC_CYT2B7_H
#define IFX_MHA_MEASUREMENTADC_CYT2B7_H

#include "Ifx_ComponentID.h"
#include "Ifx_ComponentVersion.h"
#include "Ifx_MHA_MeasurementADC.h"
#include "Ifx_MHA_MeasurementADC_Cfg.h"
#include "Ifx_Math.h"

/**
 * Base current, in A
 * <table>
 *  <caption>Parameter Representation</caption>
 *      <tr>
 *          <th>Name</th>
 *          <th>Attribute</th>
 *          <th>Value</th>
 *      </tr>
 *      <tr>
 *          <td rowspan="6">BASE_CURRENT_A</td>
 *          <td>Fractional bits</td>
 *          <td>0</td>
 *      </tr>
 *      <tr>
 *          <td>Gain</td>
 *          <td>1</td>
 *      </tr>
 *      <tr>
 *          <td>Offset</td>
 *          <td>0</td>
 *      </tr>
 *      <tr>
 *          <td>Accuracy</td>
 *          <td>-</td>
 *      </tr>
 *      <tr>
 *          <td>Range</td>
 *          <td>[-3.40282347E+38, 3.40282347E+38]</td>
 *      </tr>
 *      <tr>
 *          <td>Unit</td>
 *          <td>A</td>
 *      </tr>
 * </table>
 *
 */
#define IFX_MHA_MEASUREMENTADC_CYT2B7_BASE_CURRENT_A (IFX_MEASUREMENTADC_CFG_BASE_CURRENT_A)

/**
 * Base voltage, in V
 * <table>
 *  <caption>Parameter Representation</caption>
 *      <tr>
 *          <th>Name</th>
 *          <th>Attribute</th>
 *          <th>Value</th>
 *      </tr>
 *      <tr>
 *          <td rowspan="6">BASE_VOLTAGE_V</td>
 *          <td>Fractional bits</td>
 *          <td>0</td>
 *      </tr>
 *      <tr>
 *          <td>Gain</td>
 *          <td>1</td>
 *      </tr>
 *      <tr>
 *          <td>Offset</td>
 *          <td>0</td>
 *      </tr>
 *      <tr>
 *          <td>Accuracy</td>
 *          <td>-</td>
 *      </tr>
 *      <tr>
 *          <td>Range</td>
 *          <td>[-3.40282347E+38, 3.40282347E+38]</td>
 *      </tr>
 *      <tr>
 *          <td>Unit</td>
 *          <td>V</td>
 *      </tr>
 * </table>
 *
 */
#define IFX_MHA_MEASUREMENTADC_CYT2B7_BASE_VOLTAGE_V (IFX_MEASUREMENTADC_CFG_BASE_VOLTAGE_V)

/**
 * Contains the module output variables.
 */
typedef struct Ifx_MHA_MeasurementADC_CYT2B7_Output
{
    /**
     * DC Link voltage, scaled by the base voltage, represented in Q15
     * <table>
     *  <caption>Parameter Representation</caption>
     *      <tr>
     *          <th>Name</th>
     *          <th>Attribute</th>
     *          <th>Value</th>
     *      </tr>
     *      <tr>
     *          <td rowspan="6">dcLinkVoltageQ15</td>
     *          <td>Fractional bits</td>
     *          <td>15</td>
     *      </tr>
     *      <tr>
     *          <td>Gain</td>
     *          <td>IFX_MHA_MEASUREMENTADC_CYT2B7_BASE_VOLTAGE_V</td>
     *      </tr>
     *      <tr>
     *          <td>Offset</td>
     *          <td>0</td>
     *      </tr>
     *      <tr>
     *          <td>Accuracy</td>
     *          <td>-</td>
     *      </tr>
     *      <tr>
     *          <td>Range</td>
     *          <td>[-32768, 32767]</td>
     *      </tr>
     *      <tr>
     *          <td>Unit</td>
     *          <td>V</td>
     *      </tr>
     * </table>
     *
     */
    Ifx_Math_Fract16 dcLinkVoltageQ15;

    /**
     * Shunt current measurements, scaled by the base current, represented in Q15
     * <table>
     *  <caption>Parameter Representation</caption>
     *      <tr>
     *          <th>Name</th>
     *          <th>Attribute</th>
     *          <th>Value</th>
     *      </tr>
     *      <tr>
     *          <td rowspan="6">shuntCurrentsQ15</td>
     *          <td>Fractional bits</td>
     *          <td>15</td>
     *      </tr>
     *      <tr>
     *          <td>Gain</td>
     *          <td>IFX_MHA_MEASUREMENTADC_CYT2B7_BASE_CURRENT_A</td>
     *      </tr>
     *      <tr>
     *          <td>Offset</td>
     *          <td>0</td>
     *      </tr>
     *      <tr>
     *          <td>Accuracy</td>
     *          <td>-</td>
     *      </tr>
     *      <tr>
     *          <td>Range</td>
     *          <td>[-32768, 32767]</td>
     *      </tr>
     *      <tr>
     *          <td>Unit</td>
     *          <td>A</td>
     *      </tr>
     * </table>
     *
     */
    Ifx_Math_Fract16 shuntCurrentsQ15[2];
    /* One complete Vdc/shunt tuple was read on this execute call. */
    bool sampleValid;
} Ifx_MHA_MeasurementADC_CYT2B7_Output;

/**
 * Measurement ADC states
 */
typedef enum Ifx_MHA_MeasurementADC_CYT2B7_State
{
    Ifx_MHA_MeasurementADC_CYT2B7_State_init        = 0, /**<Current sense amplifier is initializing. Amplifier is
                                                          * powered off at the end of the initialization.*/
    Ifx_MHA_MeasurementADC_CYT2B7_State_off         = 1, /**<Current sense amplifier is disabled and powered off.*/
    Ifx_MHA_MeasurementADC_CYT2B7_State_on          = 2, /**<Current sense amplifier is enabled and powered on.*/
    Ifx_MHA_MeasurementADC_CYT2B7_State_calibration = 3  /**<Current sense amplifier is calibrating the current
                                                          * offset.*/
} Ifx_MHA_MeasurementADC_CYT2B7_State;

/**
 * CSA gain options
 */
typedef enum Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain
{
    Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain_10 = 0, /**<CSA gain 10*/
    Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain_20 = 1, /**<CSA gain 20*/
    Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain_40 = 2, /**<CSA gain 40*/
    Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain_60 = 3  /**<CSA gain 60*/
} Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain;

/**
 * Status of the measurement ADC, containing the state machine state
 */
typedef struct Ifx_MHA_MeasurementADC_CYT2B7_Status
{
    /**
     * State of the current sense amplifier
     */
    Ifx_MHA_MeasurementADC_CYT2B7_State state;
} Ifx_MHA_MeasurementADC_CYT2B7_Status;

/**
 * \brief Data structure that stores all data of module instance.
 *
 */
typedef struct Ifx_MHA_MeasurementADC_CYT2B7
{
    /**
     * Structure inherited from Ifx_MHA_MeasurementADC
     */
    Ifx_MHA_MeasurementADC _Super_Ifx_MHA_MeasurementADC;

    /**
     * Output variables of the module
     */
    Ifx_MHA_MeasurementADC_CYT2B7_Output p_output;

    /**
     * Buffer (two-dimensional array) to write and read raw current measurement values.
     *
     * - 0 -&gt; 1st current measurement of a PWM cycle (up counting)
     * - 1 -&gt; 2nd current measurement of a PWM cycle (down counting)
     */
    uint16 p_rawCurrentMeasurements[2];

    /**
     * Status of the current sense amplifier, containing the state machine state
     */
    Ifx_MHA_MeasurementADC_CYT2B7_Status p_status;

    /**
     * Gain to calculate the real current from the ADC measurement
     */
    Ifx_Math_Fract16Q p_currentGain;

    /**
     * Gain option which was set by the set-function
     */
    Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain p_currentGainOption;

    /**
     * Accumulator for the current offset calibration
     */
    uint32 p_currentAccumulator[2];

    /**
     * Counter for the current calibration cycle
     */
    uint16 p_cycleCounter;

    /**
     * Offset of the amplifier
     */
    uint16 p_offset[2];
} Ifx_MHA_MeasurementADC_CYT2B7;

/**
 *  \brief Initialize the measurement ADC to the default settings.
 *
 *  The peripheral initialization should be done by the user before calling this function.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 */
void Ifx_MHA_MeasurementADC_CYT2B7_init(Ifx_MHA_MeasurementADC_CYT2B7* self);

/**
 *  \brief Handles the state machine of the module.
 *
 *  This API Handles the state machine of the module. In the calibration state the function reads the current
 * measurement from the current amplifier and performs a calibration routine for the offset. In the on state the DC link
 * voltage is read and  the previously read currents are written to the outputs.
 *
 *  Inputs of this API:
 *  <ul>
 *      <li>State machine "Enable" input, boolean data that is Enabled/Disabled by a call to the
 * Ifx_PatternGen_CYT2B7_enable(true/false)</li>
 *  </ul>
 *
 *  Outputs of this API:
 *  <ul>
 *      <li>State variable: encoded on the "Status" output</li>
 *      <li>Measured currents and DC voltage, normalized by IFX_MEASUREMENTADC_CFG_BASE_CURRENT_A and
 * IFX_MEASUREMENTADC_CFG_BASE_VOLTAGE_V respectively, in Q15 format</li>
 *  </ul>
 *
 *  These outputs are set corresponding to the states and state transitions design.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 */
void Ifx_MHA_MeasurementADC_CYT2B7_execute(Ifx_MHA_MeasurementADC_CYT2B7* self);

/**
 *  \brief Get  the status of the measurement ADC, containing the state machine state.
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Measurement ADC status
 */
static inline Ifx_MHA_MeasurementADC_CYT2B7_Status Ifx_MHA_MeasurementADC_CYT2B7_getStatus(
    Ifx_MHA_MeasurementADC_CYT2B7* self)
{
    return self->p_status;
}


/**
 *  \brief Returns the component ID
 *
 *
 *  \param [out] *componentID Variable to store the address of the component ID
 *
 */
void Ifx_MHA_MeasurementADC_CYT2B7_getID(const Ifx_ComponentID** componentID);

/**
 *  \brief Get the module output variables.
 * The output contains the shunt current measurements and the measured DC-link voltage, both normalized in Q15.
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [out] output Structure containing the module output
 *
 */
static inline void Ifx_MHA_MeasurementADC_CYT2B7_getOutput(Ifx_MHA_MeasurementADC_CYT2B7       * self,
                                                           Ifx_MHA_MeasurementADC_CYT2B7_Output* output)
{
    *output = self->p_output;
}


/**
 *  \brief Returns the component version
 *
 *
 *  \param [out] *componentVersion Variable to store the address of the component version
 *
 */
void Ifx_MHA_MeasurementADC_CYT2B7_getVersion(const Ifx_ComponentVersion** componentVersion);

/**
 *  Set the gain of the operational amplifier
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] csaGain CSA gain setting
 *
 */
static inline void Ifx_MHA_MeasurementADC_CYT2B7_setCsaGain(Ifx_MHA_MeasurementADC_CYT2B7             * self,
                                                            Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain csaGain)
{
    /* Temporary parameters */
    Ifx_Math_Fract16 gainValue;
    Ifx_Math_Fract16 gainQformat;

    /* Set internal current gain parameter depending on CSA gain */
    if (csaGain == Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain_10)
    {
        gainValue   = IFX_MHA_MEASUREMENTADC_CFG_CURRENT_GAIN_Q_10;
        gainQformat = IFX_MHA_MEASUREMENTADC_CFG_CURRENT_GAIN_SHIFT_10;
    }
    else if (csaGain == Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain_20)
    {
        gainValue   = IFX_MHA_MEASUREMENTADC_CFG_CURRENT_GAIN_Q_20;
        gainQformat = IFX_MHA_MEASUREMENTADC_CFG_CURRENT_GAIN_SHIFT_20;
    }
    else if (csaGain == Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain_40)
    {
        gainValue   = IFX_MHA_MEASUREMENTADC_CFG_CURRENT_GAIN_Q_40;
        gainQformat = IFX_MHA_MEASUREMENTADC_CFG_CURRENT_GAIN_SHIFT_40;
    }
    else
    {
        /* Default case */
        gainValue   = IFX_MHA_MEASUREMENTADC_CFG_CURRENT_GAIN_Q_60;
        gainQformat = IFX_MHA_MEASUREMENTADC_CFG_CURRENT_GAIN_SHIFT_60;
    }

    /* Set internal parameters */
    self->p_currentGain.value   = gainValue;
    self->p_currentGain.qFormat = (Ifx_Math_FractQFormat)gainQformat;
    self->p_currentGainOption   = csaGain;
}


/**
 *  Get the gain of the operational amplifier
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return CSA gain setting in the hardware
 */
static inline Ifx_MHA_MeasurementADC_CYT2B7_optionCsaGain Ifx_MHA_MeasurementADC_CYT2B7_getCsaGain(
    Ifx_MHA_MeasurementADC_CYT2B7* self)
{
    return self->p_currentGainOption;
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
 *  Inherited from Ifx_MHA_MeasurementADC
 */
static inline void Ifx_MHA_MeasurementADC_CYT2B7_enable(Ifx_MHA_MeasurementADC_CYT2B7* self, bool enable)
{
    Ifx_MHA_MeasurementADC_enable(&(self->_Super_Ifx_MHA_MeasurementADC), enable);
}


#endif /* IFX_MHA_MEASUREMENTADC_CYT2B7_H */
