/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#ifndef SDL_ADC_CFG_H
#define SDL_ADC_CFG_H

/* Maximum operation frequency of ADC in hertz */
#define ADC_OPERATION_FREQUENCY_MAX_IN_HZ 26670000u

/* Minimum sampling time of one ADC channel in ns */
#define ANALOG_IN_SAMPLING_TIME_MIN_IN_NS 412u

/* ADC SAR instance for measuring phase current 0 (CC0) */
#define ADC_SAR_NUM_CC0 PASS0_SAR0

/* ADC SAR instance for measuring phase current 1 (CC1) */
#define ADC_SAR_NUM_CC1 PASS0_SAR1

/* ADC SAR instance for measuring DC voltage */
#define ADC_SAR_NUM_VDC PASS0_SAR2

/* ADC instance number for measuring phase current 0 (CC0) */
#define ADC_NUM_CC0 0u

/* ADC instance number for measuring phase current 1 (CC1) */
#define ADC_NUM_CC1 1u

/* ADC instance number for measuring DC voltage */
#define ADC_NUM_VDC 2u

/* ADC channel number for measuring phase current 0 (CC0) */
#define ADC_CHN_NUM_CC0 5u

/* ADC channel number for measuring phase current 1 (CC1) */
#define ADC_CHN_NUM_CC1 0u

/* ADC channel number for measuring DC voltage */
#define ADC_CHN_NUM_VDC 0u

/* ADC configuration */
extern cy_stc_adc_config_t const adcCfg;

/* Configuration of ADC channel for DC voltage */
extern cy_stc_adc_channel_config_t adcChVdcCfg;

/* Configuration of ADC channel for phase current 0 */
extern cy_stc_adc_channel_config_t adcChCC0Cfg;

/* Configuration of ADC channel for phase current 1 */
extern cy_stc_adc_channel_config_t adcChCC1Cfg;

typedef struct
{
  uint16_t vdc;
  uint8_t flag;

}_ST_VDC_PARA;;
extern _ST_VDC_PARA g_vdc_para;

#endif /* SDL_ADC_CFG_H */
