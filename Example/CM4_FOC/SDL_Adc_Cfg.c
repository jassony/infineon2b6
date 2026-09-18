/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#include "cy_project.h"
#include "cy_device_headers.h"

/* *INDENT-OFF* */

/* ADC configuration */
cy_stc_adc_config_t const adcCfg =
{
  .preconditionTime = 0u,
  .powerupTime = 0u,
  .enableIdlePowerDown = false,
  .msbStretchMode = CY_ADC_MSB_STRETCH_MODE_1CYCLE,
  .enableHalfLsbConv = 0u,
  .sarMuxEnable = true,
  .adcEnable = true,
  .sarIpEnable = true,
};

/* Configuration of ADC channel for phase current 0 */
cy_stc_adc_channel_config_t adcChCC0Cfg =
{
  .triggerSelection = CY_ADC_TRIGGER_GENERIC0,
  .channelPriority = 0u,
  .preenptionType = CY_ADC_PREEMPTION_FINISH_RESUME,
  .isGroupEnd = true,
  .doneLevel = CY_ADC_DONE_LEVEL_PULSE,
  .pinAddress = CY_ADC_PIN_ADDRESS_AN0,
  .portAddress = CY_ADC_PORT_ADDRESS_SARMUX0,
  .extMuxSelect = 0u,
  .extMuxEnable = true,
  .preconditionMode = CY_ADC_PRECONDITION_MODE_OFF,
  .overlapDiagMode = CY_ADC_OVERLAP_DIAG_MODE_OFF,
  .sampleTime = 0u, /* Will be set in init function */
  .calibrationValueSelect = CY_ADC_CALIBRATION_VALUE_REGULAR,
  .postProcessingMode = CY_ADC_POST_PROCESSING_MODE_NONE,
  .resultAlignment = CY_ADC_RESULT_ALIGNMENT_RIGHT,
  .signExtention = CY_ADC_SIGN_EXTENTION_UNSIGNED,
  .averageCount = 0u,
  .rightShift = 0u,
  .rangeDetectionMode = CY_ADC_RANGE_DETECTION_MODE_INSIDE_RANGE,
  .rangeDetectionLoThreshold = 0x0000u,
  .rangeDetectionHiThreshold = 0x0FFFu,
  .mask.grpDone = true,
  .mask.grpCancelled = false,
  .mask.grpOverflow = false,
  .mask.chRange = false,
  .mask.chPulse = false,
  .mask.chOverflow = false,
};

/* Configuration of ADC channel for phase current 1 */
cy_stc_adc_channel_config_t adcChCC1Cfg =
{
  .triggerSelection = CY_ADC_TRIGGER_GENERIC1,
  .channelPriority = 0u,
  .preenptionType = CY_ADC_PREEMPTION_FINISH_RESUME,
  .isGroupEnd = false,
  .doneLevel = CY_ADC_DONE_LEVEL_PULSE,
  .pinAddress = CY_ADC_PIN_ADDRESS_AN0,
  .portAddress = CY_ADC_PORT_ADDRESS_SARMUX0,
  .extMuxSelect = 0u,
  .extMuxEnable = true,
  .preconditionMode = CY_ADC_PRECONDITION_MODE_OFF,
  .overlapDiagMode = CY_ADC_OVERLAP_DIAG_MODE_OFF,
  .sampleTime = 0u, /* Will be set in init function */
  .calibrationValueSelect = CY_ADC_CALIBRATION_VALUE_REGULAR,
  .postProcessingMode = CY_ADC_POST_PROCESSING_MODE_NONE,
  .resultAlignment = CY_ADC_RESULT_ALIGNMENT_RIGHT,
  .signExtention = CY_ADC_SIGN_EXTENTION_UNSIGNED,
  .averageCount = 0u,
  .rightShift = 0u,
  .rangeDetectionMode = CY_ADC_RANGE_DETECTION_MODE_INSIDE_RANGE,
  .rangeDetectionLoThreshold = 0x0000u,
  .rangeDetectionHiThreshold = 0x0FFFu,
  .mask.grpDone = true,
  .mask.grpCancelled = false,
  .mask.grpOverflow = false,
  .mask.chRange = false,
  .mask.chPulse = false,
  .mask.chOverflow = false,
};

/* Configuration of ADC channel for DC voltage
 * (the channel is sampled right after CC1, this is why it does not require its own trigger) */
cy_stc_adc_channel_config_t adcChVdcCfg =
{
  .triggerSelection = CY_ADC_TRIGGER_OFF,
  .channelPriority = 0u,
  .preenptionType = CY_ADC_PREEMPTION_FINISH_RESUME,
  .isGroupEnd = true,
  .doneLevel = CY_ADC_DONE_LEVEL_PULSE,
  .pinAddress = CY_ADC_PIN_ADDRESS_AN1,
  .portAddress = CY_ADC_PORT_ADDRESS_SARMUX0,
  .extMuxSelect = 0u,
  .extMuxEnable = true,
  .preconditionMode = CY_ADC_PRECONDITION_MODE_OFF,
  .overlapDiagMode = CY_ADC_OVERLAP_DIAG_MODE_OFF,
  .sampleTime = 0u, /* Will be set in init function */
  .calibrationValueSelect = CY_ADC_CALIBRATION_VALUE_REGULAR,
  .postProcessingMode = CY_ADC_POST_PROCESSING_MODE_NONE,
  .resultAlignment = CY_ADC_RESULT_ALIGNMENT_RIGHT,
  .signExtention = CY_ADC_SIGN_EXTENTION_UNSIGNED,
  .averageCount = 0u,
  .rightShift = 0u,
  .rangeDetectionMode = CY_ADC_RANGE_DETECTION_MODE_INSIDE_RANGE,
  .rangeDetectionLoThreshold = 0x0000u,
  .rangeDetectionHiThreshold = 0x0FFFu,
  .mask.grpDone = true,
  .mask.grpCancelled = false,
  .mask.grpOverflow = false,
  .mask.chRange = false,
  .mask.chPulse = false,
  .mask.chOverflow = false,
};

/* *INDENT-ON* */

/* [] END OF FILE */
