/* Compiled with extracted production ISR, PWM stateOn and ADC calc bodies.
 * Hardware registers, clock and other FOC math are test doubles. */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "FocTiming_Cfg.h"
#include "Ifx_MDA_FocControllerF16_Cfg.h"
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int16_t sint16;
typedef int16_t Ifx_Math_Fract16;
static struct { uint32 CYCCNT; } clockRegisters;
#define DWT (&clockRegisters)
static uint32 calls, acks, submissions, disables, profilerCalls, profileStarts;
static uint32 requestedCycles=100, appliedCompare;
static uint8 focPwmPhase;
static uint32 Meas_Foc_PwmIrqCount_u32, Meas_Foc_FastLoopCount_u32;
static uint32 Meas_Foc_FastLoopLastCycles_u32, Meas_Foc_FastLoopMaxCycles_u32;
static uint32 Meas_Foc_PwmIrqLastCycles_u32, Meas_Foc_PwmIrqMaxCycles_u32;
static uint32 Meas_Foc_FastLoopBudgetCycles_u32=8000, Meas_Foc_FastLoopOverrunCount_u32;
typedef uint32 McuLoadProfilerToken;
#define MCU_LOAD_CONTEXT_FOC_FAST 0
static McuLoadProfilerToken McuLoadProfiler_begin(void) { return DWT->CYCCNT; }
static uint32 McuLoadProfiler_end(int c,McuLoadProfilerToken t)
{ assert(c==0); ++profilerCalls; return DWT->CYCCNT-t; }
static void McuFastProfile_start(McuLoadProfilerToken t) { (void)t; ++profileStarts; }
#define TCPWMx_GRPx_CNTx_U 0
static void Cy_Tcpwm_Counter_ClearTC_Intr(int c) { (void)c; ++acks; DWT->CYCCNT+=5; }
typedef enum { Ifx_MHA_PatternGen_CYT2B7_State_on,
    Ifx_MHA_PatternGen_CYT2B7_State_off,
    Ifx_MHA_PatternGen_CYT2B7_State_fault } Ifx_MHA_PatternGen_CYT2B7_State;
typedef struct {
    struct {bool p_enable;} _Super_Ifx_MHA_PatternGen;
    uint16 p_compareValues_tick[6], p_triggerTime_tick[2];
} Ifx_MHA_PatternGen_CYT2B7;
static Ifx_MHA_PatternGen_CYT2B7 pattern;
static void Ifx_MHA_PatternGen_CYT2B7_actionDisable(void) { ++disables; }
static void Ifx_MHA_PatternGen_CYT2B7_updateCompareAndTriggers(uint16 *c,uint16 *t)
{ (void)t; ++submissions; appliedCompare=c[0]; }
/* INSERT_PATTERN */
static int FocDemoClosedLoop;
static void Ifx_MS_FocSolutionF16_executeControlMode(int *s)
{
    (void)s; ++calls;
    pattern.p_compareValues_tick[0]=(uint16)calls;
    (void)Ifx_MHA_PatternGen_CYT2B7_stateOn(&pattern,false);
    DWT->CYCCNT+=requestedCycles;
}
/* INSERT_ISR */

typedef struct { bool valid; } cy_stc_adc_ch_status_t;
static struct {int CH[3];} sar={{0,1,2}};
#define ADC_SAR_NUM_VDC (&sar)
#define ADC_SAR_NUM_CC0 (&sar)
#define ADC_SAR_NUM_CC1 (&sar)
#define ADC_CHN_NUM_VDC 2
#define ADC_CHN_NUM_CC0 0
#define ADC_CHN_NUM_CC1 1
#define IFX_MHA_MEASUREMENTADC_CFG_CONVERT_VDC_TO_Q15 1
#define IFX_MHA_MEASUREMENTADC_CFG_VDC_SHIFT_FACTOR 0
typedef struct { int16_t dcLinkVoltageQ15,shuntCurrentsQ15[2]; bool sampleValid; } AdcOutput;
typedef struct {
    AdcOutput p_output;
    uint16 p_offset[2],p_rawCurrentMeasurements[2];
    struct {int16_t value;uint8 qFormat;} p_currentGain;
} Ifx_MHA_MeasurementADC_CYT2B7;
static uint16 adcRaw[3];
static bool adcValid[3];
static void Cy_Adc_Channel_GetResult(int *c,uint16 *v,cy_stc_adc_ch_status_t *s)
{ *v=adcRaw[*c];s->valid=adcValid[*c]; }
static int16_t Ifx_Math_MulShRSat_F16(int16_t v,int16_t gain,uint8 shift)
{ return (int16_t)(((int32_t)v*gain)>>shift); }
/* INSERT_ADC */
typedef AdcOutput Ifx_MHA_MeasurementADC_CYT2B7_Output;
typedef struct {
    Ifx_MHA_MeasurementADC_CYT2B7 measurementADCCYT2B7;
    int previousCurrentReconstructionInfo, p_currentReconstructionInfo;
    int currentsUVW,currentsAlphaBeta;
} Ifx_MS_FocSolutionF16;
static void Ifx_MHA_MeasurementADC_CYT2B7_execute(Ifx_MHA_MeasurementADC_CYT2B7 *s)
{ Ifx_MHA_MeasurementADC_CYT2B7_calc(s); }
static void Ifx_MHA_MeasurementADC_CYT2B7_getOutput(Ifx_MHA_MeasurementADC_CYT2B7 *s,AdcOutput *out)
{ *out=s->p_output; }
static int Ifx_Math_CurrentReconstruction_F16(int sector,int16_t *currents)
{ (void)currents; return sector; }
static int Ifx_Math_Clarke_F16(int uvw) { return uvw; }
/* INSERT_RECONSTRUCTION */
int main(void)
{
    uint32 n, speedTicks=0;
    Ifx_MHA_MeasurementADC_CYT2B7 adc={0};
    Ifx_MS_FocSolutionF16 foc={0};
    assert(FOC_PWM_FREQUENCY_HZ==20000 && FOC_CONTROL_FREQUENCY_HZ==10000);
    assert(IFX_MDA_FOCCONTROLLERF16_CFG_ID_PI_KI_TS_Q==80);
    assert(IFX_MDA_FOCCONTROLLERF16_CFG_IQ_PI_KAW_TS_Q==162);
    pattern._Super_Ifx_MHA_PatternGen.p_enable=true;
    for(n=1;n<=20000;++n) {
        uint32 prior=appliedCompare;
        Ifx_FOC_periodMatchCallback();
        if(n%10==0) ++speedTicks;
        assert(calls==n/2);
        if(n%2) assert(appliedCompare==prior);
        else assert(appliedCompare==calls);
    }
    assert(Meas_Foc_PwmIrqCount_u32==20000 && Meas_Foc_FastLoopCount_u32==10000);
    assert(acks==20000 && submissions==10000 && speedTicks==2000);
#if MCU_LOAD_PROFILER_ENABLE
    assert(profilerCalls==20000 && profileStarts==10000);
#else
    assert(profilerCalls==0 && profileStarts==0);
#endif
    assert(Meas_Foc_FastLoopLastCycles_u32==100);
    assert(Meas_Foc_PwmIrqMaxCycles_u32==105 && Meas_Foc_FastLoopOverrunCount_u32==0);
    /* Stop on a hold tick, then enable again: phase must not be reset. */
    pattern._Super_Ifx_MHA_PatternGen.p_enable=false;
    Ifx_FOC_periodMatchCallback(); assert(disables==0);
    Ifx_FOC_periodMatchCallback(); assert(disables==1);
    pattern._Super_Ifx_MHA_PatternGen.p_enable=true;
    Ifx_FOC_periodMatchCallback(); assert(calls==10001);
    Ifx_FOC_periodMatchCallback(); assert(calls==10002);
    assert(Ifx_MHA_PatternGen_CYT2B7_stateOn(&pattern,true)==Ifx_MHA_PatternGen_CYT2B7_State_fault);
    /* Deadline remains 50 us, including unsigned DWT wrap. */
    DWT->CYCCNT=0xfffffff0u; requestedCycles=8000;
    Ifx_FOC_periodMatchCallback();Ifx_FOC_periodMatchCallback();
    assert(Meas_Foc_FastLoopOverrunCount_u32==1);
    assert(Meas_Foc_FastLoopMaxCycles_u32==8000 && Meas_Foc_PwmIrqMaxCycles_u32==8005);
    adc.p_currentGain.value=1;
    adc.p_offset[0]=10;adc.p_offset[1]=20;
    adcRaw[0]=110;adcRaw[1]=220;adcRaw[2]=300;
    adcValid[0]=adcValid[1]=adcValid[2]=true;
    Ifx_MHA_MeasurementADC_CYT2B7_calc(&adc);
    assert(adc.p_output.sampleValid && adc.p_output.shuntCurrentsQ15[0]==100);
    assert(adc.p_output.shuntCurrentsQ15[1]==200 && adc.p_output.dcLinkVoltageQ15==300);
    for(n=0;n<3;++n) {
        adcRaw[0]=111;adcRaw[1]=222;adcRaw[2]=333;adcValid[n]=false;
        Ifx_MHA_MeasurementADC_CYT2B7_calc(&adc);
        assert(!adc.p_output.sampleValid);
        assert(adc.p_output.shuntCurrentsQ15[0]==100 && adc.p_output.shuntCurrentsQ15[1]==200);
        assert(adc.p_output.dcLinkVoltageQ15==300);adcValid[n]=true;
    }
    Ifx_MHA_MeasurementADC_CYT2B7_calc(&adc);
    assert(adc.p_output.sampleValid && adc.p_output.shuntCurrentsQ15[0]==101);
    assert(adc.p_output.shuntCurrentsQ15[1]==202 && adc.p_output.dcLinkVoltageQ15==333);
    foc.measurementADCCYT2B7=adc;
    foc.previousCurrentReconstructionInfo=2;foc.p_currentReconstructionInfo=5;
    (void)Ifx_MS_FocSolutionF16_measureAndReconstruct(&foc);
    assert(foc.currentsAlphaBeta==5); /* Last held PWM sector, not older 2. */
    adcValid[1]=false;foc.p_currentReconstructionInfo=6;
    (void)Ifx_MS_FocSolutionF16_measureAndReconstruct(&foc);
    assert(foc.currentsAlphaBeta==5 && foc.previousCurrentReconstructionInfo==5);
    adcValid[1]=true;
    (void)Ifx_MS_FocSolutionF16_measureAndReconstruct(&foc);
    assert(foc.currentsAlphaBeta==6);
    puts("Control rate ISR/PWM/ADC tests passed"); return 0;
}
