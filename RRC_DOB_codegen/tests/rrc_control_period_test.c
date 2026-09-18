#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../rrc_dob_compensator.c"
#include "rrc_lead_math_test.h"

/* Replay inputs are already d/q. This identity double isolates the observer;
 * integration tests separately verify capture ordering and angle forwarding. */
Ifx_Math_CmpFract16 Ifx_Math_Park_F16(Ifx_Math_CmpFract16 v,uint32 angle)
{ (void)angle; return v; }

typedef struct {
    int16_t applied[2],current[2],raw[2];
    double estimate[2],command[2];
    uint32_t delta,reset;
} ReplayRow;
#if FOC_CONTROL_PERIOD_US == 100u
#include "rrc_control100us_vectors.h"
#endif

static RRCDOB_Parameters parameters(uint8_t selector)
{
    RRCDOB_Parameters p={0};
    p.selector=selector;p.resistance_mOhm=500;p.inductanceD_uH=1300;p.inductanceQ_uH=1380;
    p.polePairs=4;p.cutoffRatioQ15=3277;p.outputLimitQ15=3277;
    p.speedLowerLimit_rpm=150;p.speedUpperLimit_rpm=5625;p.rampTime_ms=10;
    return p;
}
static uint8_t step(uint32_t angle, int capture)
{
    Ifx_Math_CmpFract16 v={100,200},i={2000,-1000},out;
    if(capture) RrcDobCompensator_captureAppliedVoltage(v,angle);
    return RrcDobCompensator_execute(v,i,angle,&out);
}
static void testPeriodAndValidity(void)
{
    RRCDOB_Parameters p=parameters(RRCDOB_SELECTOR_MONITOR);
    uint32_t angle=0,n;
    const uint32_t delta=RrcDobCompensator_speedToAngleDeltaQ32(1500,4);
    RrcDobCompensator_initialize();
    assert(RrcDobCompensator_setParameters(&p));
    assert(RRCDOB_COEFFICIENT_REFRESH_TICKS*FOC_CONTROL_PERIOD_US==500);
    assert(RRCDOB_FAST_TICKS_PER_MS*FOC_CONTROL_PERIOD_US==1000);
    assert(RrcDobCompensator_angleDeltaToFrequencyHz(delta)==100);
    assert(fabs((double)rrcDobState.axis[0].rQ30/1073741824.0 - 0.5*FOC_CONTROL_PERIOD_US*1e-6/(2*0.0013))<1e-9);
    assert(fabs((double)rrcDobState.axis[0].kvQ27/134217728.0 - FOC_CONTROL_PERIOD_US*1e-6*1000/(2*0.0013*50))<1e-8);
    assert(fabs((double)rrcDobState.axis[0].lambdaQ27/134217728.0 - 0.0013*50/(1000*FOC_CONTROL_PERIOD_US*1e-6))<1e-8);
    assert(!step(angle,0)); assert(Meas_RRCDOB_Stat_u8==RRCDOB_STATUS_APPLIED_VOLTAGE_STALE);
    assert(!step(angle,1));
    for(n=0;n<10*RRCDOB_FAST_TICKS_PER_MS;++n) {
        angle+=delta; assert(step(angle,1));
        assert(Meas_RRCDOB_Stat_u8==RRCDOB_STATUS_MONITOR_VALID && !Meas_RRCDOB_OutAct_u8);
        if(n+1<10*RRCDOB_FAST_TICKS_PER_MS) assert(rrcDobState.rampQ31<RRCDOB_Q31_ONE);
    }
    assert(rrcDobState.rampQ31==RRCDOB_Q31_ONE);
    assert(!step(angle-delta,1)); assert(Meas_RRCDOB_Stat_u8==RRCDOB_STATUS_SPEED_OUT_OF_RANGE);
    p.selector=RRCDOB_SELECTOR_APPLY; assert(RrcDobCompensator_setParameters(&p));
    step(angle,1);angle+=delta;assert(step(angle,1));
    assert(Meas_RRCDOB_OutAct_u8 == FOC_RRCDOB_APPLY_ENABLE);
    assert(Meas_RRCDOB_Stat_u8 == (FOC_RRCDOB_APPLY_ENABLE ? RRCDOB_STATUS_APPLY_VALID : RRCDOB_STATUS_APPLY_LOCKED));
    assert(!step(angle+delta,0));assert(!Meas_RRCDOB_OutAct_u8);
    p.selector=RRCDOB_SELECTOR_OFF;assert(RrcDobCompensator_setParameters(&p));
    for(n=0;n<10;++n){angle+=delta;assert(!step(angle,1));assert(Meas_RRCDOB_Stat_u8==RRCDOB_STATUS_IDLE);}
    p.selector=RRCDOB_SELECTOR_MONITOR;
#if FOC_CONTROL_PERIOD_US == 100u
    p.speedUpperLimit_rpm=5626;assert(RrcDobCompensator_setParameters(&p));
    step(angle,1);
    angle+=RrcDobCompensator_speedToAngleDeltaQ32(5626,4);
    assert(!step(angle,1));assert(Meas_RRCDOB_Stat_u8==RRCDOB_STATUS_SPEED_OUT_OF_RANGE);
#endif
    p=parameters(3);assert(!RrcDobCompensator_setParameters(&p));
    p=parameters(1);p.inductanceD_uH=0;assert(!RrcDobCompensator_setParameters(&p));
    assert(!RrcDobCompensator_setParameters(NULL));
    p=parameters(2);assert(RrcDobCompensator_setParameters(&p));
    assert(!RrcDobCompensator_execute((Ifx_Math_CmpFract16){0,0},(Ifx_Math_CmpFract16){0,0},0,NULL));
    puts("RRC period, ramp, stale/reverse/invalid/OFF gates passed");
}
static void testOutputSign(void)
{
    unsigned mode,limitCase,n,nonzero=0,saturated=0;
    const int16_t limits[3]={3277,1,0};
    const uint32_t delta=RrcDobCompensator_speedToAngleDeltaQ32(1500,4);
    assert(Cal_RRCDOB_Rev_u8==0);
    for(mode=1;mode<=2;++mode)for(limitCase=0;limitCase<3;++limitCase){
        RRCDOB_Parameters p=parameters((uint8_t)mode);
        uint32_t angle=0;
        p.outputLimitQ15=limits[limitCase];p.reset=1;
        assert(RrcDobCompensator_setParameters(&p));
        for(n=0;n<200;++n){
            RRCDOB_State before,normal;
            RRCDOB_Output reversed;
            rrcDobState.parameters.reverseOutput=0;before=rrcDobState;
            angle+=delta;step(angle,1);normal=rrcDobState;
            /* Same captured voltage/current/state: isolate only output sign.
             * Real feedback trajectories need not match on the motor. */
            rrcDobState=before;rrcDobState.parameters.reverseOutput=(n&1)?1:255;
            step(angle,1);reversed=rrcDobState.output;
            assert(memcmp(normal.axis,rrcDobState.axis,sizeof(normal.axis))==0);
            assert(normal.output.voltageErrorHatDQ15.real==reversed.voltageErrorHatDQ15.real);
            assert(normal.output.voltageErrorHatDQ15.imag==reversed.voltageErrorHatDQ15.imag);
            assert(normal.output.correctionDQ15.real==-reversed.correctionDQ15.real);
            assert(normal.output.correctionDQ15.imag==-reversed.correctionDQ15.imag);
            assert(normal.output.saturated==reversed.saturated);
            assert(normal.rampQ31==rrcDobState.rampQ31);
            if(reversed.valid){
                if(reversed.outputActive){
                    assert(reversed.compensatedVoltageDQ15.real==100+reversed.correctionDQ15.real);
                    assert(reversed.compensatedVoltageDQ15.imag==200+reversed.correctionDQ15.imag);
                }else{
                    assert(reversed.compensatedVoltageDQ15.real==100);
                    assert(reversed.compensatedVoltageDQ15.imag==200);
                }
                if(reversed.correctionDQ15.real || reversed.correctionDQ15.imag)++nonzero;
                saturated+=reversed.saturated;
            }
            rrcDobState=normal;
        }
        p.reset=0;p.reverseOutput=1;
        assert(RrcDobCompensator_setParameters(&p));
        assert(rrcDobState.rampQ31==0 && !rrcDobState.previousAngleValid);
        assert(Meas_RRCDOB_Stat_u8==RRCDOB_STATUS_INITIALIZING);
    }
    assert(nonzero && saturated);
    puts("RRC sign tests passed: both axes, unchanged estimates/states, ramp/limits, Monitor/Apply, reset");
}
static void testCalibrationWithoutEngineeringBounds(void)
{
    RRCDOB_Parameters p;
    unsigned n;
    /* Each former engineering bound is crossed independently. Acceptance
     * does not assert closed-loop stability outside the replayed domain. */
    for(n=0;n<17;++n){
        p=parameters(RRCDOB_SELECTOR_MONITOR);
        switch(n){
        case 0:p.resistance_mOhm=0;break;
        case 1:p.resistance_mOhm=3000;break;
        case 2:p.inductanceD_uH=199;break;
        case 3:p.inductanceD_uH=5001;break;
        case 4:p.inductanceQ_uH=199;break;
        case 5:p.inductanceQ_uH=5001;break;
        case 6:p.cutoffRatioQ15=0;break;
        case 7:p.cutoffRatioQ15=1000;break;
        case 8:p.cutoffRatioQ15=7000;break;
        case 9:p.outputLimitQ15=0;break;
        case 10:p.outputLimitQ15=5000;break;
        case 11:p.polePairs=0;break;
        case 12:p.polePairs=17;break;
        case 13:p.speedLowerLimit_rpm=0;break;
        case 14:p.speedLowerLimit_rpm=6000;break;
        case 15:p.speedUpperLimit_rpm=10001;break;
        default:p.rampTime_ms=1001;break;
        }
        assert(RrcDobCompensator_setParameters(&p));
        assert(RrcDobCompensator_parametersEqual(&p,&rrcDobState.parameters));
        assert(Meas_RRCDOB_Sel_u8==1 && Meas_RRCDOB_Stat_u8==RRCDOB_STATUS_INITIALIZING);
    }
    p=parameters(1);p.rampTime_ms=0;assert(!RrcDobCompensator_setParameters(&p));
    p=parameters(1);p.cutoffRatioQ15=-1;assert(!RrcDobCompensator_setParameters(&p));
    p=parameters(1);p.outputLimitQ15=-1;assert(!RrcDobCompensator_setParameters(&p));
    p=parameters(1);p.resistance_mOhm=65535;p.inductanceD_uH=1;
    assert(!RrcDobCompensator_setParameters(&p));
    puts("RRC calibration bounds removed: 17 acceptance cases; numeric/interface checks retained");
}
#if FOC_CONTROL_PERIOD_US == 100u
static void testMatlabReplay(void)
{
    RRCDOB_Parameters p=parameters(RRCDOB_SELECTOR_APPLY);
    unsigned n,j;uint32_t angle=0, fingerprint=2166136261u;
    double sum=0,peak=0,commandPeak=0;
    assert(RrcDobCompensator_setParameters(&p));
    for(n=0;n<sizeof(replay)/sizeof(replay[0]);++n) {
        const ReplayRow *r=&replay[n];
        Ifx_Math_CmpFract16 out;
        int16_t actual[2],command[2];
        if(r->reset) RrcDobCompensator_reset();
        angle+=r->delta;
        RrcDobCompensator_captureAppliedVoltage((Ifx_Math_CmpFract16){r->applied[0],r->applied[1]},angle-r->delta);
        (void)RrcDobCompensator_execute((Ifx_Math_CmpFract16){r->raw[0],r->raw[1]},
            (Ifx_Math_CmpFract16){r->current[0],r->current[1]},angle,&out);
        assert(Meas_RRCDOB_Valid_u8 == (r->reset ? 0 : 1));
        assert(rrcDobState.output.appliedVoltageDQ15.real==r->applied[0]);
        actual[0]=Meas_RRCDOB_VdHat_Q15_s16;actual[1]=Meas_RRCDOB_VqHat_Q15_s16;
        command[0]=out.real;command[1]=out.imag;
        fingerprint=(fingerprint^Meas_RRCDOB_Stat_u8)*16777619u;
        fingerprint=(fingerprint^Meas_RRCDOB_Sat_u8)*16777619u;
        fingerprint=(fingerprint^rrcDobState.rampQ31)*16777619u;
        for(j=0;j<2;++j){
            unsigned stateIndex;
            fingerprint=(fingerprint^(uint16_t)actual[j])*16777619u;
            fingerprint=(fingerprint^(uint16_t)command[j])*16777619u;
            for(stateIndex=0;stateIndex<3;++stateIndex)
                fingerprint=(fingerprint^(uint32_t)rrcDobState.axis[j].stateQ26[stateIndex])*16777619u;
            double error=fabs(actual[j]-r->estimate[j]);
            double commandError=fabs(command[j]-(FOC_RRCDOB_APPLY_ENABLE ? r->command[j] : r->raw[j]));
            sum+=error*error;if(error>peak)peak=error;
            if(commandError>commandPeak)commandPeak=commandError;
            if(error>RRC_REPLAY_PEAK_LSB || commandError>RRC_REPLAY_PEAK_LSB){
                printf("Replay failed row %u axis %u estimate %.9g command %.9g\n",n,j,error,commandError);assert(0);
            }
        }
    }
    printf("100 us RRC MATLAB replay: %u rows RMS %.9g peak %.9g command peak %.9g Q15 LSB\n",n,sqrt(sum/(2*n)),peak,commandPeak);
    assert(sqrt(sum/(2*n))<=RRC_REPLAY_RMS_LSB);
    printf("Zero-lead baseline fingerprint: %08lx\n",(unsigned long)fingerprint);
    /* Recorded from the unmodified 4535356 adapter before adding lead. */
    assert(fingerprint==(FOC_RRCDOB_APPLY_ENABLE ? 0x4762d8cbu : 0x51054257u));
}
#endif
#include "rrc_lead_pipeline_test.h"
int main(void)
{
    testLeadMath();
    testPeriodAndValidity();
    testCalibrationWithoutEngineeringBounds();
    testOutputSign();
    testLeadPipeline();
#if FOC_CONTROL_PERIOD_US == 100u
    testMatlabReplay();
#endif
    puts("RRC control period tests passed");return 0;
}
