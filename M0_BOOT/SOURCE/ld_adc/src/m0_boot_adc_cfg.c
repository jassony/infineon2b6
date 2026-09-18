/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#include "m0_boot_adc_cfg.h"

/* *INDENT-OFF* */

/* ADC configuration */
const cy_stc_adc_config_t c_adcCfg =
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
cy_stc_adc_channel_config_t adcChCfg =
{
  .triggerSelection = CY_ADC_TRIGGER_TCPWM,
  .channelPriority = 0u,
  .preenptionType = CY_ADC_PREEMPTION_ABORT_RESTART ,
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
  .mask.grpDone = false,
  .mask.grpCancelled = false,
  .mask.grpOverflow = false,
  .mask.chRange = false,
  .mask.chPulse = false,
  .mask.chOverflow = false,
};


_st_adc_init adc_cfg[ADC_CN_NUM];
  
  
//  {
//    .triggerSelection = CY_ADC_TRIGGER_GENERIC0,
//    .channelPriority = 0 ,
//    .preenptionType = CY_ADC_PREEMPTION_FINISH_RESUME, 
//    .pinAddress = CY_ADC_PIN_ADDRESS_VMOTOR,   
//    .sampleTime = 7  ,
//    .isGroupEnd = true,
//    .grpDone = true,
//    .ch = 0             //单通道
//  },
//  
//  {
//    .triggerSelection = CY_ADC_TRIGGER_TCPWM,
//    .channelPriority = 0 ,
//    .preenptionType = CY_ADC_PREEMPTION_FINISH_RESUME, 
//    .pinAddress = CY_ADC_PIN_ADDRESS_VMOTOR,   
//    .sampleTime = 7  ,
//    .isGroupEnd = true,
//    .grpDone = true,
//    .ch = ADC_CHN_NUM_CC0       //双通道
//  },
//
//  
//  
//  {
//    .triggerSelection = CY_ADC_TRIGGER_GENERIC0,
//    .channelPriority = 0 ,
//    .preenptionType = CY_ADC_PREEMPTION_FINISH_RESUME, 
//    .pinAddress = CY_ADC_PIN_ADDRESS_VMOTOR,   
//    .sampleTime = 7 , //
//    .isGroupEnd = true,
//    .grpDone = true,
//    .ch = ADC_CHN_NUM_CC1
//
//  },
//  
//  {
//    .triggerSelection = CY_ADC_TRIGGER_OFF,
//    .channelPriority = 1 ,
//    .preenptionType = CY_ADC_PREEMPTION_ABORT_RESTART, 
//    .pinAddress = CY_ADC_PIN_ADDRESS_AN12,   
//    .sampleTime = 7, //
//    .isGroupEnd = false,
//    .grpDone = false,
//    .ch = ADC1_HEAT_M_CH12 - ADC0_CN_NUM
//  },
//     
//  {
//    .triggerSelection = CY_ADC_TRIGGER_OFF,
//    .channelPriority = 1 ,
//    .preenptionType = CY_ADC_PREEMPTION_ABORT_RESTART, 
//    .pinAddress = CY_ADC_PIN_ADDRESS_AN13,   
//    .sampleTime = 7, //
//    .isGroupEnd = false,
//    .grpDone = false,
//    .ch = ADC1_HEAT_OUT_CH13 - ADC0_CN_NUM
//  },
//  
//  {
//    .triggerSelection = CY_ADC_TRIGGER_OFF,
//    .channelPriority = 1 ,
//    .preenptionType = CY_ADC_PREEMPTION_ABORT_RESTART, 
//    .pinAddress = CY_ADC_PIN_ADDRESS_AN14,   
//    .sampleTime = 7, //
//    .isGroupEnd = false,
//    .grpDone = false,
//    .ch = ADC1_HEAT_IN_CH14 - ADC0_CN_NUM //
//  },
//  
//  {
//    .triggerSelection = CY_ADC_TRIGGER_OFF,
//    .channelPriority = 1 ,//CY_ADC_TRIGGER_OFF
//    .preenptionType = CY_ADC_PREEMPTION_ABORT_RESTART, 
//    .pinAddress = CY_ADC_PIN_ADDRESS_AN15,   
//    .sampleTime = 7 , //
//    .isGroupEnd = false,
//    .grpDone = false,
//    .ch = ADC1_T_SENSE_SIC_COMPRESS_CH15 - ADC0_CN_NUM //
//  },
//  
//  {
//    .triggerSelection = CY_ADC_TRIGGER_OFF,
//    .channelPriority = 1 ,
//    .preenptionType = CY_ADC_PREEMPTION_ABORT_RESTART, 
//    .pinAddress = CY_ADC_PIN_ADDRESS_AN22,   
//    .sampleTime = 7 , //
//    .isGroupEnd = true,
//    .grpDone = true,
//    .ch = ADC1_T_SENSE_SIC_HEAT_CH22 - ADC0_CN_NUM //
//  },
//  
//  {
//    .triggerSelection = CY_ADC_TRIGGER_GENERIC0,
//    .channelPriority = 0 ,
//    .preenptionType = CY_ADC_PREEMPTION_FINISH_RESUME, 
//    .pinAddress = CY_ADC_PIN_ADDRESS_VMOTOR,   
//    .sampleTime = 7 , //
//    .isGroupEnd = true,
//    .grpDone = true,
//    .ch = ADC_CHN_NUM_VDC //
//  },
//  
//  {
//    .triggerSelection = CY_ADC_TRIGGER_OFF,
//    .channelPriority = 1 ,
//    .preenptionType = CY_ADC_PREEMPTION_ABORT_RESTART, 
//    .pinAddress = CY_ADC_PIN_ADDRESS_AN0,   
//    .sampleTime = 7 , //
//    .isGroupEnd = true,
//    .grpDone = true,
//    .ch = ADC2_PCB_SENSE_CH0 - ADC1_CN_NUM //
//  },
  



static void Adc0_Init(void)
{
  Cy_Adc_Init(PASS0_SAR0, &c_adcCfg);
  uint8_t i ;
  for( i = 0; i < ADC0_CN_NUM;i++)
  {
    adcChCfg.triggerSelection = adc_cfg[i].triggerSelection;
    adcChCfg.channelPriority = adc_cfg[i].channelPriority;
    adcChCfg.preenptionType = adc_cfg[i].preenptionType;
    adcChCfg.pinAddress = adc_cfg[i].pinAddress;   
    adcChCfg.sampleTime = adc_cfg[i].sampleTime ; 
    adcChCfg.isGroupEnd = adc_cfg[i].isGroupEnd;;
    adcChCfg.mask.grpDone = adc_cfg[i].grpDone ;

    Cy_Adc_Channel_Init(&PASS0_SAR0->CH[adc_cfg[i].ch], &adcChCfg);

  }
  for( i = 0; i < ADC0_CN_NUM;i++)
  {
    Cy_Adc_Channel_Enable(&PASS0_SAR0->CH[adc_cfg[i].ch]);
  }
}


static void Adc1_Init(void)
{
  Cy_Adc_Init(PASS0_SAR1, &c_adcCfg);
  uint8_t i;
  for( i = ADC0_CN_NUM; i < ADC1_CN_NUM;i++)
  {
    adcChCfg.triggerSelection = adc_cfg[i].triggerSelection;
    adcChCfg.channelPriority = adc_cfg[i].channelPriority;
    adcChCfg.preenptionType = adc_cfg[i].preenptionType;
    adcChCfg.pinAddress = adc_cfg[i].pinAddress;   
    adcChCfg.sampleTime = adc_cfg[i].sampleTime ; 
    adcChCfg.isGroupEnd = adc_cfg[i].isGroupEnd;;
    adcChCfg.mask.grpDone = adc_cfg[i].grpDone ;

    Cy_Adc_Channel_Init(&PASS0_SAR1->CH[adc_cfg[i].ch ], &adcChCfg);

  
  }
  for( i = ADC0_CN_NUM; i < ADC1_CN_NUM;i++)
  {
    Cy_Adc_Channel_Enable(&PASS0_SAR1->CH[adc_cfg[i].ch]);
  }
  
}
static void Adc2_Init(void)
{
  Cy_Adc_Init(PASS0_SAR2, &c_adcCfg);
  uint8_t i ;
  for(i = ADC1_CN_NUM; i < ADC2_CN_NUM;i++)
  {
    adcChCfg.triggerSelection = adc_cfg[i].triggerSelection;
    adcChCfg.channelPriority = adc_cfg[i].channelPriority;
    adcChCfg.preenptionType = adc_cfg[i].preenptionType;
    adcChCfg.pinAddress = adc_cfg[i].pinAddress;   
    adcChCfg.sampleTime = adc_cfg[i].sampleTime ; 
    adcChCfg.isGroupEnd = adc_cfg[i].isGroupEnd;;
    adcChCfg.mask.grpDone = adc_cfg[i].grpDone ;

    Cy_Adc_Channel_Init(&PASS0_SAR2->CH[adc_cfg[i].ch], &adcChCfg);

  
  }
  for(i = ADC1_CN_NUM; i < ADC2_CN_NUM;i++)
  {
    Cy_Adc_Channel_Enable(&PASS0_SAR2->CH[adc_cfg[i].ch]);
  }
  
}

void Adc_Init(void)
{
  Adc0_Init();
  Adc1_Init();
  Adc2_Init();
}


//static _st_adc_result adcChResult[ADC1_SOFT_NUM+ADC2_SOFT_NUM];

//void Adc_Process(void)
//{
//  Cy_Adc_Channel_SoftwareTrigger(&PASS0_SAR1->CH[ADC1_HEAT_M]);
//  Cy_Adc_Channel_SoftwareTrigger(&PASS0_SAR2->CH[ADC2_PCB_SENSE]);
//  Cy_Adc_Channel_GetResult(&PASS0_SAR1->CH[ADC1_HEAT_M], &adcChResult[ADC_HEAT_M].value, &adcChResult[ADC_HEAT_M].adcChStatus);
//  Cy_Adc_Channel_GetResult(&PASS0_SAR1->CH[ADC1_HEAT_OUT], &adcChResult[ADC_HEAT_OUT].value, &adcChResult[ADC_HEAT_OUT].adcChStatus);
//  Cy_Adc_Channel_GetResult(&PASS0_SAR1->CH[ADC1_HEAT_IN], &adcChResult[ADC_HEAT_IN].value, &adcChResult[ADC_HEAT_IN].adcChStatus);
//  Cy_Adc_Channel_GetResult(&PASS0_SAR1->CH[ADC1_T_SENSE_SIC_COMPRESS], &adcChResult[ADC_T_SENSE_SIC_COMPRESS].value, &adcChResult[ADC_T_SENSE_SIC_COMPRESS].adcChStatus);
//  Cy_Adc_Channel_GetResult(&PASS0_SAR1->CH[ADC1_T_SENSE_SIC_HEAT], &adcChResult[ADC_T_SENSE_SIC_HEAT].value, &adcChResult[ADC_T_SENSE_SIC_HEAT].adcChStatus);
//  Cy_Adc_Channel_GetResult(&PASS0_SAR2->CH[ADC2_PCB_SENSE], &adcChResult[ADC_PCB_SENSE].value, &adcChResult[ADC_PCB_SENSE].adcChStatus);
//
//  
//}


//uint16_t Adc_Original_Val_Get(_em_adc_soft_trg typ)
//{
//  if(typ >= ADC_SOFT_NUM)
//    return 0;
//  
//  if(adcChResult[typ].adcChStatus.valid) 
//    return adcChResult[typ].value;
//  
//  return 0xffff;
//  
//}
//












