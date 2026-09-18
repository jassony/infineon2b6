/* Included after the production adapter and shared parameter fixture. */
static void testLeadPipeline(void)
{
    unsigned mode,sign,limitCase,n,j,cases=0,changed=0;
    const int16_t limits[]={0,1,3277};
    const uint32_t delta=RrcDobCompensator_speedToAngleDeltaQ32(3000,4);
    for(mode=1;mode<=2;++mode)for(sign=0;sign<2;++sign)for(limitCase=0;limitCase<3;++limitCase){
        RRCDOB_Parameters p=parameters((uint8_t)mode);
        uint32_t angle=0;
        p.leadTime_us=100;p.reverseOutput=(uint8_t)sign;p.outputLimitQ15=limits[limitCase];p.reset=1;
        assert(RrcDobCompensator_setParameters(&p));
        assert(Meas_RRCDOB_Lead_us_u16==100);
        step(angle,1); /* acquire initial angle */
        for(n=0;n<200;++n){
            RRCDOB_State before,led;
            Ifx_Math_CmpFract16 ledOut,zeroOut;
            const Ifx_Math_CmpFract16 raw={100,200},voltage={80,-30};
            const Ifx_Math_CmpFract16 current={(int16_t)(700+(int)(n%17)*40),(int16_t)(-400+(int)(n%23)*30)};
            int16_t hat[2],correction[2];
            angle+=delta;
            RrcDobCompensator_captureAppliedVoltage(voltage,angle-delta);
            before=rrcDobState;
            assert(RrcDobCompensator_execute(raw,current,angle,&ledOut));led=rrcDobState;
            rrcDobState=before;rrcDobState.parameters.leadTime_us=0;
            assert(RrcDobCompensator_execute(raw,current,angle,&zeroOut));
            assert(rrcDobState.output.voltageErrorHatDQ15.real==led.output.voltageErrorHatDQ15.real);
            assert(rrcDobState.output.voltageErrorHatDQ15.imag==led.output.voltageErrorHatDQ15.imag);
            for(j=0;j<2;++j)assert(memcmp(rrcDobState.axis[j].stateQ26,led.axis[j].stateQ26,sizeof(led.axis[j].stateQ26))==0);
            if(led.output.correctionDQ15.real!=rrcDobState.output.correctionDQ15.real)++changed;
            rrcDobState=led;
            hat[0]=led.output.voltageErrorHatDQ15.real;hat[1]=led.output.voltageErrorHatDQ15.imag;
            correction[0]=led.output.correctionDQ15.real;correction[1]=led.output.correctionDQ15.imag;
            for(j=0;j<2;++j){
                uint8_t overflow=0;
                int32_t expected=hat[j];
                if(before.axis[j].leadHistoryValid)
                    expected=RrcDobLead_predict(led.lead,hat[j],before.axis[j].previousHatQ15);
                expected=RrcDobCompensator_roundShiftS64((int64_t)expected*before.rampQ31,31,&overflow);
                assert(!overflow);
                if(!sign)expected=-expected;
                if(expected>p.outputLimitQ15)expected=p.outputLimitQ15;
                if(expected<-p.outputLimitQ15)expected=-p.outputLimitQ15;
                assert(correction[j]==expected);
                assert(led.axis[j].previousHatQ15==hat[j] && led.axis[j].leadHistoryValid);
            }
            if(mode==2 && FOC_RRCDOB_APPLY_ENABLE){
                assert(ledOut.real==raw.real+correction[0] && ledOut.imag==raw.imag+correction[1]);
                assert(Meas_RRCDOB_OutAct_u8);
                if(correction[0]!=0 || correction[1]!=0){
                    Ifx_Math_CmpFract16 extreme={(correction[0]>=0)?32767:-32768,(correction[1]>=0)?32767:-32768};
                    Ifx_Math_CmpFract16 clipped;
                    rrcDobState=before;
                    assert(RrcDobCompensator_execute(extreme,current,angle,&clipped));
                    assert(clipped.real==extreme.real && clipped.imag==extreme.imag && Meas_RRCDOB_Sat_u8);
                    rrcDobState=led;RrcDobCompensator_publishOutput();
                }
            }else{assert(ledOut.real==raw.real && ledOut.imag==raw.imag && !Meas_RRCDOB_OutAct_u8);}
            ++cases;
        }
        /* Existing reset, stale capture, qualification and frequency reset clear history. */
        RrcDobCompensator_reset();
        assert(!rrcDobState.axis[0].leadHistoryValid && !rrcDobState.axis[1].leadHistoryValid);
        assert(Meas_RRCDOB_Lead_us_u16==100);
        step(angle,1);angle+=delta;assert(step(angle,1));
        assert(!step(angle+delta,0));assert(!rrcDobState.axis[0].leadHistoryValid);
        step(angle,1);angle+=delta;assert(step(angle,1));
        assert(!step(angle-delta,1));assert(!rrcDobState.axis[0].leadHistoryValid);
        assert(step(angle,1));
        rrcDobState.coefficientAge=RRCDOB_COEFFICIENT_REFRESH_TICKS;
        angle+=delta/3;assert(!step(angle,1));
        assert(Meas_RRCDOB_Stat_u8==RRCDOB_STATUS_INITIALIZING && !rrcDobState.axis[0].leadHistoryValid);
        angle+=delta/3;assert(step(angle,1));
        p.reset=0;p.leadTime_us=50;assert(RrcDobCompensator_setParameters(&p));
        assert(Meas_RRCDOB_Lead_us_u16==50 && !rrcDobState.axis[0].leadHistoryValid);
    }
    assert(changed>0);
    /* Refresh timing: frequency changes do not re-evaluate lead every sample. */
    {
        RRCDOB_Parameters p=parameters(1);
        RRCDOB_LeadCoefficients old,expected;
        uint32_t angle=0;
        p.leadTime_us=75;p.reset=1;assert(RrcDobCompensator_setParameters(&p));
        step(angle,1);angle+=delta;assert(step(angle,1));old=rrcDobState.lead;
        angle+=delta+delta/10;assert(step(angle,1));
        assert(rrcDobState.lead.currentQ16==old.currentQ16);
        rrcDobState.coefficientAge=RRCDOB_COEFFICIENT_REFRESH_TICKS;
        angle+=delta+delta/10;assert(step(angle,1));
        expected=RrcDobLead_coefficients(delta+delta/10,75,FOC_CONTROL_PERIOD_US);
        assert(rrcDobState.lead.currentQ16==expected.currentQ16 && rrcDobState.lead.previousQ16==expected.previousQ16);
    }
    printf("Lead pipeline: %u paired samples, %u changed corrections; states/hat unchanged, ramp/sign/limits/reset/refresh passed\n",cases,changed);
}
