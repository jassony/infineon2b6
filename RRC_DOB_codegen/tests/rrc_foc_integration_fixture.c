/* Actual production function bodies are inserted by the runner. Motor PI,
 * modulator, interrupts and RRC arithmetic have explicit test doubles. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "FocTiming_Cfg.h"
#include "rrc_dob_compensator.h"
enum { Ifx_MS_FocSolutionF16_State_off,Ifx_MS_FocSolutionF16_State_standBy,
    Ifx_MS_FocSolutionF16_State_run,Ifx_MS_FocSolutionF16_State_rampDown,Ifx_MS_FocSolutionF16_State_fault };
enum { Ifx_MS_FocSolutionF16_ControlMode_foc=1,Ifx_MS_FocSolutionF16_SubState_closedLoop=1 };
enum {Ifx_MAS_ModulatorF16_State_on=1,Ifx_MHA_PatternGen_CYT2B7_State_on=1};
typedef struct {int state;} TestStatus;
typedef TestStatus Ifx_MAS_ModulatorF16_Status;
typedef struct {Ifx_Math_CmpFract16 currentDQ,voltageDQ;} TestPi;
typedef struct {Ifx_Math_PolarFract16 voltageCommandPolar;} Ifx_MDA_FocControllerF16_Output;
typedef struct {
    struct {int state,actualControlMode,subState;} p_status;
    bool p_enableControl,p_enablePowerStage,p_enableDirectInterface,p_externalSpeedControllerEnabled;
    bool p_rrcDobFastEligible,p_qCommandZeroCrossing;
    struct {bool p_forceDutyEnable;} modulator;
    int patternGenCYT2B7;
    Ifx_Math_CmpFract16 dqCommand,currentsAlphaBeta,voltageAlphaBeta;
    uint32 angle;int16_t rateLimitInSpeedQ15;
    TestPi focController;
} Ifx_MS_FocSolutionF16;
static Ifx_MS_FocSolutionF16 FocDemoClosedLoop;
volatile uint8_t Cal_RRCDOB_Sel_u8,Cal_RRCDOB_Rst_u8,Meas_RRCDOB_Sel_u8,Meas_RRCDOB_Pend_u8;
volatile uint8_t Cal_RRCDOB_Rev_u8;
volatile uint16_t Cal_RRCDOB_Lead_us_u16,Meas_RRCDOB_Lead_us_u16;
volatile Ifx_Math_Fract16 Cal_RRCDOB_WcRatio_Q15_s16=3277,Cal_RRCDOB_OutHi_Q15_s16=3277;
volatile uint16_t Cal_RRCDOB_SpdLo_rpm_u16=800,Cal_RRCDOB_SpdHi_rpm_u16=4000,Cal_RRCDOB_Ramp_ms_u16=10;
static uint16_t Cal_MotorAppliedPhaseResistance_mOhm_u16=500;
static uint16_t Cal_MotorAppliedDirectInductance_uH_u16=1300,Cal_MotorAppliedQuadratureInductance_uH_u16=1380;
static uint8_t Cal_MotorAppliedPolePairs_u8=4;
static unsigned resets,steps,captures,parameterCalls,cartCalls,disableCalls;
static RRCDOB_Parameters lastParameters;
static uint32 mask;static int raceOnDisable;
static Ifx_Math_CmpFract16 captured,consumed;
static uint32 capturedAngle;
static uint8_t testOutputActive;
static Ifx_Math_CmpFract16 cartInput;
static TestStatus modStatus={1},patternStatus={1};
static uint32 __get_PRIMASK(void){return mask;}
static void __disable_irq(void){mask=1;++disableCalls;if(raceOnDisable==1)FocDemoClosedLoop.p_enableControl=true;
    if(raceOnDisable==2)++Cal_RRCDOB_WcRatio_Q15_s16;
    if(raceOnDisable==3)Cal_RRCDOB_Rev_u8^=1u;
    if(raceOnDisable==4)++Cal_RRCDOB_Lead_us_u16;}
static void __enable_irq(void){mask=0;}
void RrcDobCompensator_reset(void){++resets;}
uint8_t RrcDobCompensator_setParameters(const RRCDOB_Parameters *p)
{assert(mask==1);++parameterCalls;lastParameters=*p;Meas_RRCDOB_Sel_u8=p->selector;
    Meas_RRCDOB_Lead_us_u16=p->leadTime_us;return 1;}
uint8_t RrcDobCompensator_execute(Ifx_Math_CmpFract16 v,Ifx_Math_CmpFract16 i,uint32 a,Ifx_Math_CmpFract16 *out)
{(void)i;(void)a;++steps;consumed=captured;*out=v;
    if(testOutputActive){out->real+=10;out->imag-=20;}return 1;}
void RrcDobCompensator_getOutput(RRCDOB_Output *out)
{memset(out,0,sizeof(*out));out->valid=1;out->outputActive=testOutputActive;}
void RrcDobCompensator_captureAppliedVoltage(Ifx_Math_CmpFract16 v,uint32 a)
{++captures;captured=v;capturedAngle=a;}
static void Ifx_MS_FocSolutionF16_rotateDQRefSystem(Ifx_MS_FocSolutionF16 *s){(void)s;}
static void Ifx_MS_FocSolutionF16_closedLoop(Ifx_MS_FocSolutionF16 *s,uint32 a){s->angle=a;}
static void Ifx_MS_FocSolutionF16_openLoop(Ifx_MS_FocSolutionF16 *s){s->angle=123;}
static void Ifx_MDA_FocControllerF16_execute(TestPi *p,Ifx_Math_CmpFract16 i,Ifx_Math_CmpFract16 ref,uint32 a,int16_t speed)
{(void)a;(void)speed;p->currentDQ=i;p->voltageDQ=ref;}
static void Ifx_MDA_FocControllerF16_getOutput(TestPi *p,Ifx_MDA_FocControllerF16_Output *o)
{(void)p;o->voltageCommandPolar=(Ifx_Math_PolarFract16){321,987};}
Ifx_Math_PolarFract16 Ifx_Math_CartToPolar_F16(Ifx_Math_CmpFract16 c)
{cartInput=c;++cartCalls;return (Ifx_Math_PolarFract16){654,123};}
static TestStatus Ifx_MAS_ModulatorF16_getStatus(void *s){(void)s;return modStatus;}
static TestStatus Ifx_MHA_PatternGen_CYT2B7_getStatus(int *s){(void)s;return patternStatus;}
/* PARAMETER_FUNCTION */
/* ELIGIBILITY_FUNCTION */
/* REGULATION_FUNCTION */
/* FWC_FUNCTION */
static void captureAfterPwm(Ifx_MS_FocSolutionF16 *self,bool rrcDobEligible)
{
    Ifx_MAS_ModulatorF16_Status modulatorStatus;
    /* CAPTURE_BLOCK */
}
static void missingAdc(Ifx_MS_FocSolutionF16 *self)
{
    /* ADC_RESET_BLOCK */
}
int main(void)
{
    unsigned n,before;
    Ifx_MS_FocSolutionF16 *s=&FocDemoClosedLoop;
    Ifx_Math_PolarFract16 voltage;
    s->p_status.state=Ifx_MS_FocSolutionF16_State_standBy;
    FocApplyRrcDobParameters();assert(parameterCalls==1 && !Meas_RRCDOB_Pend_u8);
    for(n=0;n<100;++n)FocApplyRrcDobParameters();assert(parameterCalls==1);
    s->p_status.state=Ifx_MS_FocSolutionF16_State_run;s->p_enableControl=s->p_enablePowerStage=true;
    s->p_status.actualControlMode=1;s->p_status.subState=1;
    for(n=0;n<100;++n){voltage=Ifx_MS_FocSolutionF16_regulationLoop(s,1000+n);
        assert(voltage.amplitude==321 && voltage.angle==987);captureAfterPwm(s,s->p_rrcDobFastEligible);}
    assert(steps==0 && captures==0 && resets==0 && cartCalls==0);
    Cal_RRCDOB_Sel_u8=1;FocApplyRrcDobParameters();
    assert(parameterCalls==1 && Meas_RRCDOB_Pend_u8==1 && Meas_RRCDOB_Sel_u8==0);
    s->p_enableControl=false;s->p_status.state=Ifx_MS_FocSolutionF16_State_standBy;
    raceOnDisable=1;FocApplyRrcDobParameters();assert(parameterCalls==1 && mask==0);
    raceOnDisable=0;s->p_enableControl=false;FocApplyRrcDobParameters();
    assert(parameterCalls==2 && Meas_RRCDOB_Sel_u8==1 && !Meas_RRCDOB_Pend_u8);
    s->p_enableControl=true;s->p_status.state=Ifx_MS_FocSolutionF16_State_run;
    s->p_status.subState=0;voltage=Ifx_MS_FocSolutionF16_regulationLoop(s,999);
    assert(steps==0); /* Alignment/I-f never activates RRC. */
    s->p_status.subState=1;
    assert(FocFwcControlIsEligible()); /* Monitor preserves FWC. */
    for(n=0;n<100;++n){
        int16_t old=captured.real;
        voltage=Ifx_MS_FocSolutionF16_regulationLoop(s,1000+n);
        assert(voltage.amplitude==321 && voltage.angle==987 && consumed.real==old);
        s->voltageAlphaBeta=(Ifx_Math_CmpFract16){(int16_t)(n+10),(int16_t)(n+20)};
        captureAfterPwm(s,s->p_rrcDobFastEligible);
        assert(captured.real==n+10 && capturedAngle==1000+n);
        assert(FocFwcControlIsEligible());
    }
    assert(steps==100 && captures==100 && cartCalls==0);
    patternStatus.state=0;captureAfterPwm(s,true);assert(captures==100);patternStatus.state=1;
    modStatus.state=0;captureAfterPwm(s,true);assert(captures==100);modStatus.state=1;
    missingAdc(s);before=resets;missingAdc(s);assert(resets==before && !s->p_rrcDobFastEligible);
    s->modulator.p_forceDutyEnable=true;voltage=Ifx_MS_FocSolutionF16_regulationLoop(s,1234);assert(steps==100);
    s->modulator.p_forceDutyEnable=false;
    Cal_RRCDOB_WcRatio_Q15_s16=4000;Cal_RRCDOB_Rst_u8=1;FocApplyRrcDobParameters();
    assert(parameterCalls==2 && Meas_RRCDOB_Pend_u8==1 && Cal_RRCDOB_Rst_u8==1);
    s->p_enableControl=false;s->p_status.state=Ifx_MS_FocSolutionF16_State_standBy;
    raceOnDisable=2;FocApplyRrcDobParameters();assert(parameterCalls==2 && mask==0);
    raceOnDisable=0;mask=1;FocApplyRrcDobParameters();assert(mask==1 && parameterCalls==3);
    assert(lastParameters.cutoffRatioQ15==4001 && !Cal_RRCDOB_Rst_u8);mask=0;
    Cal_RRCDOB_Sel_u8=0;FocApplyRrcDobParameters();assert(Meas_RRCDOB_Sel_u8==0);
    before=parameterCalls;for(n=0;n<100;++n)FocApplyRrcDobParameters();assert(parameterCalls==before);
    /* Applied selector gates ownership; a pending request must not take over. */
    Cal_RRCDOB_Sel_u8=2;
    s->p_enableControl=true;s->p_status.state=Ifx_MS_FocSolutionF16_State_run;
    FocApplyRrcDobParameters();assert(Meas_RRCDOB_Sel_u8==0 && Meas_RRCDOB_Pend_u8);
    s->p_enableControl=false;s->p_status.state=Ifx_MS_FocSolutionF16_State_standBy;
    FocApplyRrcDobParameters();assert(Meas_RRCDOB_Sel_u8==2 && !Meas_RRCDOB_Pend_u8);
    assert(lastParameters.reset==1);
    s->p_enableControl=true;s->p_status.state=Ifx_MS_FocSolutionF16_State_run;
    s->dqCommand=(Ifx_Math_CmpFract16){100,200};
    testOutputActive=FOC_RRCDOB_APPLY_ENABLE;before=cartCalls;
    voltage=Ifx_MS_FocSolutionF16_regulationLoop(s,1234);
#if FOC_RRCDOB_APPLY_ENABLE
    assert(cartCalls==before+1 && cartInput.real==110 && cartInput.imag==180);
    assert(voltage.amplitude==654 && voltage.angle==123+1234);
#else
    assert(cartCalls==before && voltage.amplitude==321 && voltage.angle==987);
#endif
    assert(FocFwcControlIsEligible()); /* Existing FWC policy is unchanged. */
    testOutputActive=0;before=cartCalls;
    voltage=Ifx_MS_FocSolutionF16_regulationLoop(s,1234);
    assert(cartCalls==before && voltage.amplitude==321 && voltage.angle==987);
    before=parameterCalls;Cal_RRCDOB_Rev_u8=1;FocApplyRrcDobParameters();
    assert(parameterCalls==before && Meas_RRCDOB_Pend_u8 && lastParameters.reverseOutput==0);
    s->p_enableControl=false;s->p_status.state=Ifx_MS_FocSolutionF16_State_standBy;
    raceOnDisable=3;FocApplyRrcDobParameters();
    assert(parameterCalls==before && mask==0);
    raceOnDisable=0;Cal_RRCDOB_Rev_u8=1;FocApplyRrcDobParameters();
    assert(parameterCalls==before+1 && !Meas_RRCDOB_Pend_u8 && lastParameters.reverseOutput==1);
    FocApplyRrcDobParameters();assert(parameterCalls==before+1);
    s->p_enableControl=true;s->p_status.state=Ifx_MS_FocSolutionF16_State_run;
    before=parameterCalls;Cal_RRCDOB_Lead_us_u16=100;FocApplyRrcDobParameters();
    assert(parameterCalls==before && Meas_RRCDOB_Pend_u8 && Meas_RRCDOB_Lead_us_u16==0);
    s->p_enableControl=false;s->p_status.state=Ifx_MS_FocSolutionF16_State_rampDown;
    FocApplyRrcDobParameters();assert(parameterCalls==before && Meas_RRCDOB_Pend_u8);
    s->p_status.state=Ifx_MS_FocSolutionF16_State_standBy;
    raceOnDisable=4;FocApplyRrcDobParameters();assert(parameterCalls==before && mask==0);
    raceOnDisable=0;Cal_RRCDOB_Lead_us_u16=100;FocApplyRrcDobParameters();
    assert(parameterCalls==before+1 && !Meas_RRCDOB_Pend_u8 && Meas_RRCDOB_Lead_us_u16==100);
    assert(lastParameters.leadTime_us==100);
    FocApplyRrcDobParameters();assert(parameterCalls==before+1);
    Cal_RRCDOB_Sel_u8=0;FocApplyRrcDobParameters();before=parameterCalls;
    Cal_RRCDOB_Lead_us_u16=50;FocApplyRrcDobParameters();
    assert(parameterCalls==before && Meas_RRCDOB_Lead_us_u16==100); /* Stable OFF skips numeric scans. */
    Cal_RRCDOB_Sel_u8=1;FocApplyRrcDobParameters();
    assert(parameterCalls==before+1 && Meas_RRCDOB_Lead_us_u16==50);
    puts("RRC FOC integration tests passed: OFF/Monitor/Apply/FWC/capture/ADC/stopped snapshot/sign/lead recheck");return 0;
}
