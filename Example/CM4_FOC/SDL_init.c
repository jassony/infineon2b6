/*
 * Copyright (c) 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

/*******************************************************************************
**                      Includes                                              **
*******************************************************************************/

#include "cy_project.h"
#include "cy_device_headers.h"

#include "main_cm4.h"
#include "SDL_init.h"
#include "SDL_Adc_Cfg.h"
#include "SDL_Gpio_Cfg.h"
#include "SDL_Tcpwm_Cfg.h"
#include "SDL_SysInt_Cfg.h"
#include "Ifx_MS_FocSolutionF16_Cfg.h"
#include "Ifx_MHA_MeasurementADC_CYT2B7.h"
#include "Ifx_MHA_PatternGen_Cfg.h"

#ifndef MMEK_USE_SIX_PWM
#error "MMEK_USE_SIX_PWM not defined"
#endif


static const stc_pin_config_m4 pin_cfg_UVW[] =
{
    {
        .portReg = TCPWMx_LINEx_PORT_U,
        .pinNum  = TCPWMx_LINEx_PIN_U,
        {
        .outVal = 0u,
        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
        .hsiom = P6_0_TCPWM0_LINE256,
        .intEdge = 0u,
        .intMask = 0u,
        .vtrip = 0u,
        .slewRate = 0u,
        .driveSel = 0u,
        }
    },
    {
        .portReg = TCPWMx_LINEx_PORT_U_COMPL,
        .pinNum  = TCPWMx_LINEx_PIN_U_COMPL,
        {
        .outVal = 0u,
        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
        .hsiom = P6_1_TCPWM0_LINE_COMPL256,
        .intEdge = 0u,
        .intMask = 0u,
        .vtrip = 0u,
        .slewRate = 0u,
        .driveSel = 0u,
        }
    },
    {
        .portReg = TCPWMx_LINEx_PORT_V,
        .pinNum  = TCPWMx_LINEx_PIN_V,
        {
        .outVal = 0u,
        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
        .hsiom = P6_2_TCPWM0_LINE257,
        .intEdge = 0u,
        .intMask = 0u,
        .vtrip = 0u,
        .slewRate = 0u,
        .driveSel = 0u,
        }
    },
    {
        .portReg = TCPWMx_LINEx_PORT_V_COMPL,
        .pinNum  = TCPWMx_LINEx_PIN_V_COMPL,
        {
        .outVal = 0u,
        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
        .hsiom = P6_3_TCPWM0_LINE_COMPL257,
        .intEdge = 0u,
        .intMask = 0u,
        .vtrip = 0u,
        .slewRate = 0u,
        .driveSel = 0u,
        }
    },
    {
        .portReg = TCPWMx_LINEx_PORT_W,
        .pinNum  = TCPWMx_LINEx_PIN_W,
        {
        .outVal = 0u,
        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
        .hsiom = P6_4_TCPWM0_LINE258,
        .intEdge = 0u,
        .intMask = 0u,
        .vtrip = 0u,
        .slewRate = 0u,
        .driveSel = 0u,
        }
    },
    {
        .portReg = TCPWMx_LINEx_PORT_W_COMPL,
        .pinNum  = TCPWMx_LINEx_PIN_W_COMPL,
        {
        .outVal = 0u,
        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
        .hsiom = P6_5_TCPWM0_LINE_COMPL258,
        .intEdge = 0u,
        .intMask = 0u,
        .vtrip = 0u,
        .slewRate = 0u,
        .driveSel = 0u,
        }
    },

    

};


static const stc_pin_config_m4 pin_cfg_GPIO[] =
{
    {
        .portReg = TCPWMx_LINEx_PORT_U,
        .pinNum  = TCPWMx_LINEx_PIN_U,
        {
        .outVal = 0u,
        .driveMode = CY_GPIO_DM_STRONG,
        .hsiom = P6_0_GPIO,
        .intEdge = 0u,
        .intMask = 0u,
        .vtrip = 0u,
        .slewRate = 0u,
        .driveSel = 0u,
        }
    },
    {
        .portReg = TCPWMx_LINEx_PORT_U_COMPL,
        .pinNum  = TCPWMx_LINEx_PIN_U_COMPL,
        {
        .outVal = 1u,
        .driveMode = CY_GPIO_DM_STRONG,
        .hsiom = P6_1_GPIO,
        .intEdge = 0u,
        .intMask = 0u,
        .vtrip = 0u,
        .slewRate = 0u,
        .driveSel = 0u,
        }
    },
    {
        .portReg = TCPWMx_LINEx_PORT_V,
        .pinNum  = TCPWMx_LINEx_PIN_V,
        {
        .outVal = 0u,
        .driveMode = CY_GPIO_DM_STRONG,
        .hsiom = P6_2_GPIO,
        .intEdge = 0u,
        .intMask = 0u,
        .vtrip = 0u,
        .slewRate = 0u,
        .driveSel = 0u,
        }
    },
    {
        .portReg = TCPWMx_LINEx_PORT_V_COMPL,
        .pinNum  = TCPWMx_LINEx_PIN_V_COMPL,
        {
        .outVal = 1u,
        .driveMode = CY_GPIO_DM_STRONG,
        .hsiom = P6_3_GPIO,
        .intEdge = 0u,
        .intMask = 0u,
        .vtrip = 0u,
        .slewRate = 0u,
        .driveSel = 0u,
        }
    },
    {
        .portReg = TCPWMx_LINEx_PORT_W,
        .pinNum  = TCPWMx_LINEx_PIN_W,
        {
        .outVal = 0u,
        .driveMode = CY_GPIO_DM_STRONG,
        .hsiom = P6_4_GPIO,
        .intEdge = 0u,
        .intMask = 0u,
        .vtrip = 0u,
        .slewRate = 0u,
        .driveSel = 0u,
        }
    },
    {
        .portReg = TCPWMx_LINEx_PORT_W_COMPL,
        .pinNum  = TCPWMx_LINEx_PIN_W_COMPL,
        {
        .outVal = 1u,
        .driveMode = CY_GPIO_DM_STRONG,
        .hsiom = P6_5_GPIO,
        .intEdge = 0u,
        .intMask = 0u,
        .vtrip = 0u,
        .slewRate = 0u,
        .driveSel = 0u,
        }
    },

    

};

void PortInit_UVW(void)
{
    for (uint8_t i = 0; i < (sizeof(pin_cfg_UVW) / sizeof(pin_cfg_UVW[0])); i++)
    {
        Cy_GPIO_Pin_Init(pin_cfg_UVW[i].portReg, pin_cfg_UVW[i].pinNum, &pin_cfg_UVW[i].cfg);
    }
}

void PortInit_GPIO(void)//配合Set_UVW_GPIO()一起使用
{
    for (uint8_t i = 0; i < (sizeof(pin_cfg_GPIO) / sizeof(pin_cfg_GPIO[0])); i++)
    {
        Cy_GPIO_Pin_Init(pin_cfg_GPIO[i].portReg, pin_cfg_GPIO[i].pinNum, &pin_cfg_GPIO[i].cfg);
    }
    
    //三个上桥都是低
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_U,TCPWMx_LINEx_PIN_U,0);
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_V,TCPWMx_LINEx_PIN_V,0);
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_W,TCPWMx_LINEx_PIN_W,0);
   
   //三个下桥都是高
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_U_COMPL,TCPWMx_LINEx_PIN_U_COMPL,1);
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_V_COMPL,TCPWMx_LINEx_PIN_V_COMPL,1);
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_W_COMPL,TCPWMx_LINEx_PIN_W_COMPL,1);
}

void PortInit_GPIO_0(void)//配合Set_UVW_GPIO()一起使用
{
    for (uint8_t i = 0; i < (sizeof(pin_cfg_GPIO) / sizeof(pin_cfg_GPIO[0])); i++)
    {
        Cy_GPIO_Pin_Init(pin_cfg_GPIO[i].portReg, pin_cfg_GPIO[i].pinNum, &pin_cfg_GPIO[i].cfg);
    }
    
    //三个上桥都是低
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_U,TCPWMx_LINEx_PIN_U,0);
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_V,TCPWMx_LINEx_PIN_V,0);
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_W,TCPWMx_LINEx_PIN_W,0);
   
   //三个下桥都是高
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_U_COMPL,TCPWMx_LINEx_PIN_U_COMPL,0);
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_V_COMPL,TCPWMx_LINEx_PIN_V_COMPL,0);
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_W_COMPL,TCPWMx_LINEx_PIN_W_COMPL,0);
}

void Port_Write( void* base, uint32_t pinNum, uint32_t value)
{
  Cy_GPIO_Write((stc_GPIO_PRT_t*) base,pinNum,value);
}



void Set_UVW_GPIO(void)//刹车的时候
{
  //三个上桥都是低
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_U,TCPWMx_LINEx_PIN_U,0);
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_V,TCPWMx_LINEx_PIN_V,0);
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_W,TCPWMx_LINEx_PIN_W,0);
   
   //三个下桥都是高
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_U_COMPL,TCPWMx_LINEx_PIN_U_COMPL,1);
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_V_COMPL,TCPWMx_LINEx_PIN_V_COMPL,1);
   Cy_GPIO_Write((stc_GPIO_PRT_t*) TCPWMx_LINEx_PORT_W_COMPL,TCPWMx_LINEx_PIN_W_COMPL,1);
}






/* API to calculate the divider and period */
SDL_SysClkDivAndPeriod_t SDL_calcDivAndPeriodFromFreq(uint32 const targetFreq_Hz, uint32 const baseFreq_Hz,
                                                      cy_en_divider_types_t const dividerType);

/* API to initialize GPIOs */
uint32_t SDL_initGpio(void);

/* SDL initialization */
uint32 SDL_Init(void)
{
//    SDL_SysClkDivAndPeriod_t divAndPeriodTemp;
//    float32_t                AdcDividerFloat;
    uint32_t                 returnCode;
//    uint32_t                 AdcDivider;
//    uint32_t                 actualAdcOperationFreq;
//    uint64_t                 samplingCycle_u64;
//    uint32_t                 samplingCycle;

    /* Initialize return code to no error */
    returnCode = 0u;

    /*---------------------*/
    /* Clock Configuration */
    /*---------------------*/

    /*
     * Clock Configuration for SpeedLoop
     * */

    /* Calculate divider and period value for given speed loop frequency */
//    divAndPeriodTemp = SDL_calcDivAndPeriodFromFreq(IFX_MS_FOCSOLUTIONF16_CFG_SPEED_LOOP_FREQUENCY_HZ, PCLK_FREQ_HZ,
//        CY_SYSCLK_DIV_16_BIT);
//
//    /* Store period value for later TCPWM configuration */
//    SpeedLoopTimerCfg.period = divAndPeriodTemp.period - 1u;
//
//    /* Assign a 16b divider to the peripheral clock for TCPWM SpeedLoop channel */
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphAssignDivider(PCLK_TCPWM_SPEEDLOOP, CY_SYSCLK_DIV_16_BIT,
//        TCPWM_PERI_CLK_DIVIDER_NO_SPEEDLOOP);
//
//    /* Set previously calculated divider value */
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT,
//        TCPWM_PERI_CLK_DIVIDER_NO_SPEEDLOOP, (divAndPeriodTemp.divider - 1u));
//
//    /* Enable divider */
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT,
//        TCPWM_PERI_CLK_DIVIDER_NO_SPEEDLOOP);

    /*
     * Clock Configuration for U/V/W/CC0/CC1 TCPWMs
     * */

    /* Calculate divider and period value for given PWM frequency */
//    divAndPeriodTemp = SDL_calcDivAndPeriodFromFreq(IFX_MHA_PATTERNGEN_CFG_FREQUENCY_KHZ * 1000u, PCLK_FREQ_HZ,
//        CY_SYSCLK_DIV_16_BIT);
//
//    /* Store period value for later TCPWM configuration */
//    PwmUVWCfg.period = divAndPeriodTemp.period - 1u;
//
//    /* Assign the same 16b divider to all of the peripheral clocks of the TCPWM channels */
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphAssignDivider(PCLK_TCPWM_U,
//        (cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, TCPWM_PERI_CLK_DIVIDER_NO_PWMUVW);
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphAssignDivider(PCLK_TCPWM_V,
//        (cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, TCPWM_PERI_CLK_DIVIDER_NO_PWMUVW);
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphAssignDivider(PCLK_TCPWM_W,
//        (cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, TCPWM_PERI_CLK_DIVIDER_NO_PWMUVW);
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphAssignDivider(PCLK_TCPWM_CC0,
//        (cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, TCPWM_PERI_CLK_DIVIDER_NO_PWMUVW);
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphAssignDivider(PCLK_TCPWM_CC1,
//        (cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, TCPWM_PERI_CLK_DIVIDER_NO_PWMUVW);

    /* Set previously calculated divider value */
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphSetDivider((cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT,
//        TCPWM_PERI_CLK_DIVIDER_NO_PWMUVW, (divAndPeriodTemp.divider - 1u));
//
//    /* Enable divider */
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphEnableDivider((cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT,
//        TCPWM_PERI_CLK_DIVIDER_NO_PWMUVW);
//
    /*
     * Clock Configuration for ADC
     * */

    /* Calculate required divider value for the peripheral clock to not exceed the maximum ADC frequency */
//    AdcDividerFloat = PCLK_FREQ_HZ / ADC_OPERATION_FREQUENCY_MAX_IN_HZ;
//    AdcDivider      = (uint32_t)(AdcDividerFloat + 1.0f);
//
//    /* Assign the same 16b divider to all of the peripheral clocks of all used ADC SAR instances */
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphAssignDivider(PCLK_PASS0_CLOCK_SAR0, CY_SYSCLK_DIV_16_BIT,
//        TCPWM_PERI_CLK_DIVIDER_NO_ADC);
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphAssignDivider(PCLK_PASS0_CLOCK_SAR1, CY_SYSCLK_DIV_16_BIT,
//        TCPWM_PERI_CLK_DIVIDER_NO_ADC);
//
//    /* Set previously calculated divider value */
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT,
//        TCPWM_PERI_CLK_DIVIDER_NO_ADC, (AdcDivider - 1u));
//
//    /* Enable divider */
//    returnCode = returnCode | (uint32_t)Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT,
//        TCPWM_PERI_CLK_DIVIDER_NO_ADC);

    /*-----------------------*/
    /* Configuration for ADC */
    /*-----------------------*/
    /* Calculate actual sampling time based on the ADC frequency and the minimum sampling time */
//    actualAdcOperationFreq = PCLK_FREQ_HZ / AdcDivider;
//    samplingCycle_u64      = (uint64_t)ANALOG_IN_SAMPLING_TIME_MIN_IN_NS * (uint64_t)actualAdcOperationFreq;
//    samplingCycle          = (uint32_t)(((uint64_t)(samplingCycle_u64 + (uint64_t)500000000uL)) /
//                                        (uint64_t)1000000000uL);
//
//    /* Update the sample time in the channel configuration structs */
//    adcChCC0Cfg.sampleTime = samplingCycle;
//    adcChCC1Cfg.sampleTime = samplingCycle;
//    adcChVdcCfg.sampleTime = samplingCycle;
//
//    /* Initialize all used ADC SAR instances */
//    returnCode = returnCode | (uint32_t)Cy_Adc_Init(PASS0_SAR0, &adcCfg);
//    returnCode = returnCode | (uint32_t)Cy_Adc_Init(PASS0_SAR1, &adcCfg);
//
//    /* Initialize ADC channels */
//    returnCode = returnCode | (uint32_t)Cy_Adc_Channel_Init(&ADC_SAR_NUM_CC0->CH[ADC_CHN_NUM_CC0], &adcChCC0Cfg);
//    returnCode = returnCode | (uint32_t)Cy_Adc_Channel_Init(&ADC_SAR_NUM_CC1->CH[ADC_CHN_NUM_CC1], &adcChCC1Cfg);
//    returnCode = returnCode | (uint32_t)Cy_Adc_Channel_Init(&ADC_SAR_NUM_VDC->CH[ADC_CHN_NUM_VDC], &adcChVdcCfg);

    /*
     * Select the correct output trigger from TCPWM as the generic input trigger for the ADC
     * */

    /* TCPWM CC0 -> ADC_NUM_CC0 Generic Input Trigger #0 */
//    Cy_Adc_SetGenericTriggerInput(PASS0_EPASS_MMIO, ADC_NUM_CC0, 0u, TCPWM_CHN_NUM_CC0);
//
//    /* TCPWM CC1 -> ADC_NUM_CC1 Generic Input Trigger #1 */
//    Cy_Adc_SetGenericTriggerInput(PASS0_EPASS_MMIO, ADC_NUM_CC1, 1u, TCPWM_CHN_NUM_CC1);
//
//    /* TCPWM CC1 -> ADC_NUM_VDC Generic Input Trigger #1 */
//    Cy_Adc_SetGenericTriggerInput(PASS0_EPASS_MMIO, ADC_NUM_VDC, 1u, TCPWM_CHN_NUM_CC1);
//    Cy_Adc_Channel_Enable(&ADC_SAR_NUM_CC0->CH[ADC_CHN_NUM_CC0]);
//    Cy_Adc_Channel_Enable(&ADC_SAR_NUM_CC1->CH[ADC_CHN_NUM_CC1]);
//    Cy_Adc_Channel_Enable(&ADC_SAR_NUM_VDC->CH[ADC_CHN_NUM_VDC]);

    /*----------------------*/
    /* Configure TriggerMUX */
    /*----------------------*/
    /* Connect TCPWM CC0 event with ADC Channel #0 */
//    returnCode = returnCode | Cy_TrigMux_Connect(TRIG_IN_MUX_TCPWM_CC0, TRIG_OUT_MUX_ADC_CC0, 0u,
//        TRIGGER_TYPE_PASS_TR_SAR_CH_IN__EDGE, 0u);
//
//    /* Connect TCPWM CC1 event with ADC Channel #1 */
//    returnCode = returnCode | Cy_TrigMux_Connect(TRIG_IN_MUX_TCPWM_CC1, TRIG_OUT_MUX_ADC_CC1, 0u,
//        TRIGGER_TYPE_PASS_TR_SAR_CH_IN__EDGE, 0u);

    /*----------------------*/
    /* Configure Interrupts */
    /*----------------------*/

    /*
     * Configure and enable the speed loop interrupt
     * */
//    returnCode = returnCode | (uint32_t)Cy_SysInt_InitIRQ(&SpeedLoopIrqCfg);
//    Cy_SysInt_SetSystemIrqVector(SpeedLoopIrqCfg.sysIntSrc, Ifx_FOC_speedLoopCallback);
//    NVIC_SetPriority(SpeedLoopIrqCfg.intIdx, 3u);

    /*
     * Configure and enable the current loop interrupt
     * */
//    returnCode = returnCode | (uint32_t)Cy_SysInt_InitIRQ(&CurrentLoopIrqCfg);
//    Cy_SysInt_SetSystemIrqVector(CurrentLoopIrqCfg.sysIntSrc, Ifx_FOC_periodMatchCallback);
//    NVIC_SetPriority(CurrentLoopIrqCfg.intIdx, 1u);

    /*-----------------*/
    /* Configure TCPWM */
    /*-----------------*/

    /*
     * Initialize TCPWM0_GPR0_CNT0 as Timer/Counter and enable the channel plus the corresponding interrupt
     * */
//    returnCode = returnCode | Cy_Tcpwm_Counter_Init(TCPWM_GRPx_CNTx_SPEEDLOOP, &SpeedLoopTimerCfg);
//    Cy_Tcpwm_Counter_Enable(TCPWM_GRPx_CNTx_SPEEDLOOP);
//    Cy_Tcpwm_Counter_SetTC_IntrMask(TCPWM_GRPx_CNTx_SPEEDLOOP);

    /*
     * Initialize PWM counters for U/V/W/CC0/CC1
     * */
//    returnCode = returnCode | Cy_Tcpwm_Pwm_Init(TCPWMx_GRPx_CNTx_U, &PwmUVWCfg);
//    returnCode = returnCode | Cy_Tcpwm_Pwm_Init(TCPWMx_GRPx_CNTx_V, &PwmUVWCfg);
//    returnCode = returnCode | Cy_Tcpwm_Pwm_Init(TCPWMx_GRPx_CNTx_W, &PwmUVWCfg);
//    returnCode = returnCode | Cy_Tcpwm_Pwm_Init(TCPWMx_GRPx_CNTx_CC0, &PwmUVWCfg);
//    returnCode = returnCode | Cy_Tcpwm_Pwm_Init(TCPWMx_GRPx_CNTx_CC1, &PwmUVWCfg);

    /* Configure the channel for CC0 to set its output trigger #1 at CC0 match */
//    TCPWMx_GRPx_CNTx_CC0->unTR_OUT_SEL.stcField.u3OUT1 = CY_TCPWM_COUNTER_CC0_MATCH;
//
//    /* Configure the channel for CC1 to set its output trigger #1 at CC1 match */
//    TCPWMx_GRPx_CNTx_CC1->unTR_OUT_SEL.stcField.u3OUT1 = CY_TCPWM_COUNTER_CC1_MATCH;

    /*
     * Enable PWM counters for U/V/W/CC0/CC1
     * */
//    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_U);
//    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_V);
//    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_W);
//    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_CC0);
//    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_CC1);

    /* Enable interrupt for phase U */
    Cy_Tcpwm_Counter_SetTC_IntrMask(TCPWMx_GRPx_CNTx_U);

    /* Synchronize all counters
     * -> Output the Reload signal to TCPWM_ALL_CNT_TR_IN[2] */
    Cy_TrigMux_SwTrigger(TRIG_OUT_MUX_4_TCPWM_ALL_CNT_TR_IN2, TRIGGER_TYPE_EDGE, 1u);
    
//    while((Cy_Tcpwm_Pwm_GetStatus(TCPWMx_GRPx_CNTx_U)& ((uint32_t)(1<<15))) == 0)
//      ;
//    while((Cy_Tcpwm_Pwm_GetStatus(TCPWMx_GRPx_CNTx_V)& ((uint32_t)(1<<15))) == 0)
//      ;
//    while((Cy_Tcpwm_Pwm_GetStatus(TCPWMx_GRPx_CNTx_W)& ((uint32_t)(1<<15))) == 0)
//      ;
    /* Set stop select to 1 to disable PWM output but keep counter running */
    TCPWMx_GRPx_CNTx_U->unTR_IN_SEL0.stcField.u8STOP_SEL = 1u;
    TCPWMx_GRPx_CNTx_V->unTR_IN_SEL0.stcField.u8STOP_SEL = 1u;
    TCPWMx_GRPx_CNTx_W->unTR_IN_SEL0.stcField.u8STOP_SEL = 1u;
//    TCPWMx_GRPx_CNTx_U->unTR_IN_SEL0.stcField.u8STOP_SEL = 0u;

    /* Set CC1 of the channel which triggers the phase current measurement #0 to the period + 1 */
//    Cy_Tcpwm_Counter_SetCompare1_Buff(TCPWMx_GRPx_CNTx_CC0, 4000);
//
//    /* Set CC0 of the channel which triggers the phase current measurement #1 to 0 */
//    Cy_Tcpwm_Counter_SetCompare0_Buff(TCPWMx_GRPx_CNTx_CC1, 0u);

    /* Port Configuration LEDs and PWM GPIOs */
//    returnCode = returnCode | SDL_initGpio();

    return returnCode;
}


SDL_SysClkDivAndPeriod_t SDL_calcDivAndPeriodFromFreq(uint32 const targetFreq_Hz, uint32 const baseFreq_Hz,
                                                      cy_en_divider_types_t const dividerType)
{
    SDL_SysClkDivAndPeriod_t returnValue;
    float32                  dividerFloat;
    uint32                   maxPeriod;

    switch (dividerType)
    {
        case CY_SYSCLK_DIV_8_BIT:
            {
                maxPeriod = 0xFFu;
            }   break;

        case CY_SYSCLK_DIV_16_BIT:
            {
                maxPeriod = 0xFFFFu;
            }   break;

        default:
            {
                /* assume 8 bit divider per default */
                maxPeriod = 0xFFu;
            }   break;
    }

    /* Calculate required divider value to make sure that the period value fits in 8/16/32 bit
     * targetPeriod = baseFreq_Hz / (divider * targetFreq_Hz)
     *   <=> periDivider = periFreq / (targetPeriod * targetFreq_Hz) */
    dividerFloat        = (float32_t)baseFreq_Hz / ((float32_t)maxPeriod * (float32_t)targetFreq_Hz);
    returnValue.divider = (uint32_t)(dividerFloat + 1.0f);

    /* Calculate the period value to reach the target frequency based on the previously calculated divider value */
    returnValue.period = (baseFreq_Hz / returnValue.divider) / targetFreq_Hz;

    return returnValue;
}


uint32_t SDL_initGpio(void)
{
    uint32_t returnCode;
    returnCode = 0;

    /* Init GPIOs controlled by PWM lines */
//    returnCode = returnCode | (uint32_t)Cy_GPIO_Pin_Init(TCPWMx_LINEx_PORT_U, TCPWMx_LINEx_PIN_U, &GpioPwmUCfg);
//    returnCode = returnCode | (uint32_t)Cy_GPIO_Pin_Init(TCPWMx_LINEx_PORT_V, TCPWMx_LINEx_PIN_V, &GpioPwmVCfg);
//    returnCode = returnCode | (uint32_t)Cy_GPIO_Pin_Init(TCPWMx_LINEx_PORT_W, TCPWMx_LINEx_PIN_W, &GpioPwmWCfg);
//
//#if (MMEK_USE_SIX_PWM == 1)
//    returnCode = returnCode | (uint32_t)Cy_GPIO_Pin_Init(TCPWMx_LINEx_PORT_U_COMPL, TCPWMx_LINEx_PIN_U_COMPL,
//        &GpioPwmUComplCfg);
//    returnCode = returnCode | (uint32_t)Cy_GPIO_Pin_Init(TCPWMx_LINEx_PORT_V_COMPL, TCPWMx_LINEx_PIN_V_COMPL,
//        &GpioPwmVComplCfg);
//    returnCode = returnCode | (uint32_t)Cy_GPIO_Pin_Init(TCPWMx_LINEx_PORT_W_COMPL, TCPWMx_LINEx_PIN_W_COMPL,
//        &GpioPwmWComplCfg);
//#endif /*  (MMEK_USE_SIX_PWM == 1) */
    /* Init GPIOs used as analog inputs */
//    returnCode = returnCode | (uint32_t)Cy_GPIO_Pin_Init(GPIO_ADC_PORT_VDC, GPIO_ADC_PIN_VDC, &GpioAdcVdcCfg);
//    returnCode = returnCode | (uint32_t)Cy_GPIO_Pin_Init(GPIO_ADC_PORT_CC0, GPIO_ADC_PIN_CC0, &GpioAdcPhaseCurrCfg);

    /* Init GPIOs used to verify that the trigger times of the current measurements within a PWM cycle are correct */
//    returnCode = returnCode | (uint32_t)Cy_GPIO_Pin_Init(GPIO_PWM_PORT_ADC_TRIG0, GPIO_PWM_PIN_ADC_TRIG0,
//        &GpioPwmAdcTrig0Cfg);
//    returnCode = returnCode | (uint32_t)Cy_GPIO_Pin_Init(GPIO_PWM_PORT_ADC_TRIG1, GPIO_PWM_PIN_ADC_TRIG1,
//        &GpioPwmAdcTrig1Cfg);

    /* Init GPIOs used as simple toggle pins to measure task runtimes */
//    returnCode             = returnCode | (uint32_t)Cy_GPIO_Pin_Init(GPIO_PRT20, 0u, &GpioTogglePinCfg);
//    GpioTogglePinCfg.hsiom = P20_1_GPIO;
//    returnCode             = returnCode | (uint32_t)Cy_GPIO_Pin_Init(GPIO_PRT20, 1u, &GpioTogglePinCfg);
//    GpioTogglePinCfg.hsiom = P20_2_GPIO;
//    returnCode             = returnCode | (uint32_t)Cy_GPIO_Pin_Init(GPIO_PRT20, 2u, &GpioTogglePinCfg);
//    GpioTogglePinCfg.hsiom = P20_3_GPIO;
//    returnCode             = returnCode | (uint32_t)Cy_GPIO_Pin_Init(GPIO_PRT20, 3u, &GpioTogglePinCfg);

    return returnCode;
}


/* [] END OF FILE */
