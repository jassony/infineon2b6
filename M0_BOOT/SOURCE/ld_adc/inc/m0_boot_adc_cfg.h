/*
 * m0_adc_cfg.h
 *
 *  Created on: 2026年1月22日
 *      Author: hzldy
 */

#ifndef M0_BSW_SOURCE_LD_ADC_INC_M0_ADC_CFG_H_
#define M0_BSW_SOURCE_LD_ADC_INC_M0_ADC_CFG_H_

#include "cy_project.h"
#include "cy_device_headers.h"
#define ADC_OPERATION_FREQUENCY_MAX_IN_HZ 26670000u

typedef struct
{
  cy_en_adc_trigger_selection_t triggerSelection; //触发方式
  uint8_t                        channelPriority; //优先级0最高7最低
  cy_en_adc_preemption_type_t    preenptionType; //adc被打断后的流程
  cy_en_adc_pin_address_t        pinAddress;    //PIN的地址
  uint16_t                       sampleTime;  //采样时序，foc采样需要特别注意时间，其余默认60
  bool                           isGroupEnd;  
  bool                           grpDone;      
  uint32_t                       ch; // SAR通道需要被pwm触发特殊
}_st_adc_init;

typedef struct
{
  cy_stc_adc_ch_status_t adcChStatus;
  uint16_t value;
}_st_adc_result;
typedef enum
{
//  ADC0_IU_SENSE_CHM = 0,
//  ADC0_IU_SENSE_CHM1,
  ADC0_CN_NUM ,
//  ADC1_IV_SENSE_CHM = ADC0_CN_NUM,
//  ADC1_SOFT_START = ADC1_IV_SENSE_CHM,
//  ADC1_HEAT_M_CH12,
//  ADC1_HEAT_OUT_CH13,
//  ADC1_HEAT_IN_CH14,
//  ADC1_T_SENSE_SIC_COMPRESS_CH15,
//  ADC1_T_SENSE_SIC_HEAT_CH22,
  ADC1_CN_NUM = ADC0_CN_NUM,
//  ADC2_VBAT_SENSE_CHM = ADC1_CN_NUM,
//  ADC2_SOFT_START = ADC2_VBAT_SENSE_CHM,
//  ADC2_PCB_SENSE_CH0 ,
  ADC2_CN_NUM = ADC1_CN_NUM,
  ADC_CN_NUM 
}_em_adc_type;

typedef enum
{
//  ADC_HEAT_M,
//  ADC_HEAT_OUT,
//  ADC_HEAT_IN,
//  ADC_T_SENSE_SIC_COMPRESS,
//  ADC_T_SENSE_SIC_HEAT,
//  ADC_PCB_SENSE,
  ADC_SOFT_NUM
}_em_adc_soft_trg;

//#define ADC1_HEAT_M                     ADC1_HEAT_M_CH12 - ADC1_SOFT_START
//#define ADC1_HEAT_OUT                   ADC1_HEAT_OUT_CH13 - ADC1_SOFT_START
//#define ADC1_HEAT_IN                    ADC1_HEAT_IN_CH14 - ADC1_SOFT_START
//#define ADC1_T_SENSE_SIC_COMPRESS       ADC1_T_SENSE_SIC_COMPRESS_CH15 - ADC1_SOFT_START
//#define ADC1_T_SENSE_SIC_HEAT           ADC1_T_SENSE_SIC_HEAT_CH22 - ADC1_SOFT_START
//#define ADC2_PCB_SENSE                  ADC2_PCB_SENSE_CH0 - ADC2_SOFT_START
//#define ADC1_SOFT_NUM                   (ADC1_CN_NUM - ADC1_SOFT_START) 
//#define ADC2_SOFT_NUM                   (ADC2_CN_NUM - ADC2_SOFT_START)

#define ADC_CHN_NUM_CC0         5u

/* ADC channel number for measuring phase current 1 (CC1) */
#define ADC_CHN_NUM_CC1         0u

/* ADC channel number for measuring DC voltage */
#define ADC_CHN_NUM_VDC         0u

void Adc_Init(void);
//void Adc_Process(void);
//uint16_t Adc_Original_Val_Get(_em_adc_soft_trg typ);


#endif /* M0_BSW_SOURCE_LD_ADC_INC_M0_ADC_CFG_H_ */
