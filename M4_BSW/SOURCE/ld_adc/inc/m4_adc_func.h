/*
 * m4_adc_func.h
 *
 *  Created on: 2026Äê1ÔÂ28ÈÕ
 *      Author: hzldy
 */

#ifndef M4_BSW_SOURCE_LD_ADC_INC_M4_ADC_FUNC_H_
#define M4_BSW_SOURCE_LD_ADC_INC_M4_ADC_FUNC_H_

#include "cy_project.h"

/* ADC channel number for measuring phase current 0 (CC0) */
#define ADC_CHN_NUM_CC0 5u

/* ADC channel number for measuring phase current 1 (CC1) */
#define ADC_CHN_NUM_CC1 0u

/* ADC channel number for measuring DC voltage */
#define ADC_CHN_NUM_VDC 0u
/* ADC instance number for measuring phase current 0 (CC0) */
#define ADC_NUM_CC0 0u

/* ADC instance number for measuring phase current 1 (CC1) */
#define ADC_NUM_CC1 1u

/* ADC instance number for measuring DC voltage */
#define ADC_NUM_VDC 2u
void Adc_Enable_Func(void);

#endif /* M4_BSW_SOURCE_LD_ADC_INC_M4_ADC_FUNC_H_ */
