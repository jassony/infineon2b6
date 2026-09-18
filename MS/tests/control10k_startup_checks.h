static void setup(unsigned mode)
{
    memset(&FocDemoClosedLoop,0,sizeof(FocDemoClosedLoop));
    FocDemoClosedLoop.p_status.state=Ifx_MS_FocSolutionF16_State_standBy;
    FocDemoClosedLoop.measurementADCCYT2B7.p_status.state=Ifx_MHA_MeasurementADC_CYT2B7_State_on;
    Cal_FocStartupMode_u8=mode;enableControl=0;enablePowerStage=1;
    referenceSpeedQ0=1000;fault=false;observerValid=false;inputMode=0;
    adcValid=true;handoffCalls=observerCalls=regulationCalls=seedCalls=0;
    startupTick();enableControl=1;startupTick();
}
int main(void)
{
    unsigned mode,n,repeat;
    for(repeat=0;repeat<3;++repeat) for(mode=0;mode<2;++mode) {
        setup(mode);
        assert(rotorAlignCounter==3);
        for(n=0;n<3;++n) {
            startupTick();
            Ifx_MS_FocSolutionF16_executeControlMode(&FocDemoClosedLoop);
            assert(observerCalls==n+1 && regulationCalls==n+1);
            assert(capturedDq.real==FocDemoClosedLoop.dqCommand.real);
            assert(capturedDq.imag==FocDemoClosedLoop.dqCommand.imag);
        }
        assert(rotorAlignCounter==0 && seedCalls==0);
        startupTick();
        if(mode==0) {
            assert(handoffCalls==1 && FocDemoClosedLoop.p_status.subState==0);
            assert(FocDemoClosedLoop.openEnabled && FocDemoClosedLoop.openDq.imag==100);
            observerValid=true;startupTick();
            assert(handoffCalls==2 && FocDemoClosedLoop.p_status.subState==1);
            startupTick();assert(handoffCalls==2); /* Consume success once. */
        } else assert(handoffCalls==0 && FocDemoClosedLoop.speed>0);
        referenceSpeedQ0=-1000;startupTick();assert(FocDemoClosedLoop.speed<0);
        /* Incomplete ADC tuple: no PI/PWM update, invalidate observer; then
         * a stop and a fault must still run the existing shutdown path. */
        adcValid=false;
        n=regulationCalls;
        Ifx_MS_FocSolutionF16_executeControlMode(&FocDemoClosedLoop);
        assert(regulationCalls==n && invalidObserverCalls>0);
        enableControl=0;startupTick();
        assert(FocDemoClosedLoop.p_status.state!=Ifx_MS_FocSolutionF16_State_run);
        Ifx_MS_FocSolutionF16_executeControlMode(&FocDemoClosedLoop);
        assert(regulationCalls==n); /* Ramp-down cannot consume stale ADC either. */
        FocDemoClosedLoop.p_status.state=Ifx_MS_FocSolutionF16_State_fault;
        Ifx_MS_FocSolutionF16_executeControlMode(&FocDemoClosedLoop);
        assert(applied.amplitude==0);
    }
    puts("Control10k startup dispatch tests passed");return 0;
}
