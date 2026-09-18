static unsigned checks;
#define CHECK(x) do{checks++;if(!(x)){printf("FAIL startup line %d\n",__LINE__);return 1;}}while(0)
static void setup(unsigned enabled,unsigned mode)
{
    HFIPDInjection_cancel();memset(&FocDemoClosedLoop,0,sizeof(FocDemoClosedLoop));
    FocDemoClosedLoop.p_status.state=Ifx_MS_FocSolutionF16_State_standBy;
    Cal_HFIPD_Enable_u8=enabled;Cal_FocStartupMode_u8=mode;
    Cal_HFIPD_Hf_V_f32=20;enableControl=0;enablePowerStage=1;
    referenceSpeedQ0=1000;fault=false;observerValid=false;inputMode=0;
    seedCalls=handoffCalls=observerCalls=regulationCalls=0;
    startupTick();enableControl=1;startupTick();
}
int main(void)
{
    unsigned k,mode;
    uint32 seeded;
    for(mode=0;mode<2;mode++) {
        setup(1,mode);CHECK(HFIPDInjection_isActive());CHECK(rotorAlignCounter==3);
        for(k=0;k<4900;k++) {
            FocDemoClosedLoop.currentsAlphaBeta.real=(int16_t)(vectors[k][0]*32768.0F/50.0F);
            FocDemoClosedLoop.currentsAlphaBeta.imag=(int16_t)(vectors[k][1]*32768.0F/50.0F);
            Ifx_MS_FocSolutionF16_executeControlMode(&FocDemoClosedLoop);
            CHECK(observerCalls==0 && regulationCalls==0);
            if(k%10==9 && k<4899) {startupTick();CHECK(rotorAlignCounter==3 && seedCalls==0);}
        }
        CHECK(HFIPDInjection_isComplete());CHECK(Meas_HFIPD_Valid_u8);
        startupTick();CHECK(!HFIPDInjection_isActive());CHECK(seedCalls==1);
        CHECK(rotorAlignCounter==3);seeded=FocDemoClosedLoop.iToF.angle;
        CHECK(seeded>300000000u && seeded<410000000u);
        for(k=0;k<3;k++){startupTick();CHECK(FocDemoClosedLoop.iToF.angle==seeded);}
        CHECK(rotorAlignCounter==0);startupTick();
        if(mode==0) {CHECK(handoffCalls==1 && FocDemoClosedLoop.p_status.subState==0);
            observerValid=true;startupTick();CHECK(FocDemoClosedLoop.p_status.subState==1);}
        else CHECK(handoffCalls==0 && FocDemoClosedLoop.speed>0);
        CHECK(seedCalls==1);
        enableControl=0;startupTick();CHECK(!Meas_HFIPD_Valid_u8);
    }
    setup(1,0);enableControl=0;startupTick();
    CHECK(FocDemoClosedLoop.p_status.state==Ifx_MS_FocSolutionF16_State_standBy);
    CHECK(!HFIPDInjection_isActive());enableControl=1;startupTick();
    CHECK(HFIPDInjection_isActive() && Meas_HFIPD_Time_tick_u32==0);
    fault=true;startupTick();CHECK(!HFIPDInjection_isActive() && !Meas_HFIPD_Valid_u8);
    setup(1,1);FocDemoClosedLoop.modulator.p_forceDutyEnable=true;
    Ifx_MS_FocSolutionF16_executeControlMode(&FocDemoClosedLoop);
    CHECK(Meas_HFIPD_Stat_u8==HFIPD_VOLTAGE_REJECT && !Meas_HFIPD_Valid_u8);
    setup(1,1);HFIPDInjection_execute(NAN,0,100,&dummyV,&dummyAngle);startupTick();
    CHECK(seedCalls==0 && !HFIPDInjection_isActive());CHECK(rotorAlignCounter==3);
    startupTick();CHECK(rotorAlignCounter==2); /* invalid continues original alignment */
    for(mode=0;mode<2;mode++) {
        setup(0,mode);CHECK(!HFIPDInjection_isActive());
        for(k=0;k<8;k++){observerValid=true;startupTick();}
        CHECK(seedCalls==0 && rotorAlignCounter==0);
        Ifx_MS_FocSolutionF16_executeControlMode(&FocDemoClosedLoop);
        CHECK(observerCalls==1 && regulationCalls==1 && applied.amplitude==123);
    }
    setup(0,1);inputMode=1;enableControl=0;startupTick();
    FocDemoClosedLoop.p_status.state=Ifx_MS_FocSolutionF16_State_standBy;
    Cal_HFIPD_Enable_u8=1;enableControl=1;startupTick();
    CHECK(!HFIPDInjection_isActive());
    printf("HFIPD startup integration passed: %u checks\n",checks);return 0;
}
