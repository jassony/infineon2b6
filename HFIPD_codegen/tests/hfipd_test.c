#include "../hfipd_injection.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "hfipd_vectors.h"
static unsigned checks;
#define CHECK(x) do {checks++;if(!(x)){printf("FAIL line %d\n",__LINE__);return 1;}}while(0)
int main(void)
{
    HFIPD_State s,t;
    HFIPD_Params p={20,10,2000,0,0,2,.01F,.13962634F,45,300,400,800,800,300};
    unsigned k;
    float maxAngle=0, v;
    uint32_t angle=0x12345678u;
    CHECK(HFIPD_start(&s,&p));
    for(k=0;k<5000;k++) {
        float e;
        HFIPD_step(&s,vectors[k][0],vectors[k][1]);
        e=fabsf(s.theta-vectors[k][2]);if(e>maxAngle)maxAngle=e;
        CHECK(e<0.00174533F);
        CHECK(fabsf(s.voltage-vectors[k][3])<0.0001F);
        CHECK(s.stage==(uint8_t)vectors[k][5]);
        CHECK(s.valid==(uint8_t)vectors[k][6]);
        CHECK(fabsf(s.areaPos-vectors[k][7])<0.0001F);
        CHECK(fabsf(s.areaNeg-vectors[k][8])<0.0001F);
    }
    CHECK(s.valid);CHECK(HFIPD_start(&s,&p));CHECK(!s.valid && s.tick==0 && s.areaPos==0);
    t=s;HFIPD_step(&s,1,0);CHECK(t.tick==0 && s.tick==1); /* independent instances */
    HFIPD_step(&s,NAN,0);CHECK(s.stage==HFIPD_BAD_CURRENT && s.voltage==0);
    CHECK(HFIPD_start(&s,&p));HFIPD_step(&s,40,40);CHECK(s.stage==HFIPD_BAD_CURRENT);
    p.kp=NAN;CHECK(!HFIPD_start(&s,&p));p.kp=2000;
    p.initialRad=INFINITY;CHECK(!HFIPD_start(&s,&p));p.initialRad=0;
    p.axisMaxRad=2;CHECK(!HFIPD_start(&s,&p));p.axisMaxRad=.13962634F;
    p.track=0;CHECK(!HFIPD_start(&s,&p));p.track=300;
    Cal_HFIPD_Enable_u8=0;HFIPDInjection_begin();CHECK(!HFIPDInjection_isActive());
    CHECK(!HFIPDInjection_consume(&angle));CHECK(angle==0x12345678u);
    Cal_HFIPD_Enable_u8=1;HFIPDInjection_begin();CHECK(HFIPDInjection_isActive());
    /* Active writes cannot alter the frozen input snapshot. */
    Cal_HFIPD_Hf_V_f32=NAN;Cal_HFIPD_Enable_u8=0;
    for(k=0;k<5000;k++) HFIPDInjection_execute(vectors[k][0],vectors[k][1],100,&v,&angle);
    CHECK(HFIPDInjection_isComplete());CHECK(Meas_HFIPD_Valid_u8);
    CHECK(HFIPDInjection_consume(&angle));CHECK(!HFIPDInjection_isActive());
    angle=0x12345678u;CHECK(!HFIPDInjection_consume(&angle));CHECK(angle==0x12345678u);
    HFIPDInjection_cancel();CHECK(!Meas_HFIPD_Valid_u8 && Meas_HFIPD_Init_rad_f32==0);
    Cal_HFIPD_Enable_u8=1;HFIPDInjection_begin();CHECK(Meas_HFIPD_Stat_u8==HFIPD_BAD_PARAM);
    CHECK(!HFIPDInjection_consume(&angle));CHECK(angle==0x12345678u);
    HFIPDInjection_cancel();Cal_HFIPD_Hf_V_f32=20;Cal_HFIPD_Seq_u16=1;
    HFIPDInjection_begin();CHECK(Meas_HFIPD_Stat_u8==HFIPD_BAD_PARAM);
    HFIPDInjection_cancel();Cal_HFIPD_Seq_u16=2;HFIPDInjection_begin();
    HFIPDInjection_execute(0,0,5,&v,&angle);CHECK(Meas_HFIPD_Stat_u8==HFIPD_VOLTAGE_REJECT);
    CHECK(v==0);HFIPDInjection_cancel();
    HFIPDInjection_begin();HFIPDInjection_execute(0,0,100,&v,&angle);
    HFIPDInjection_cancel();CHECK(!HFIPDInjection_isActive());
    HFIPDInjection_begin();CHECK(Meas_HFIPD_Time_tick_u32==0 && !Meas_HFIPD_Valid_u8);
    printf("HFIPD tests passed: %u checks; replay max angle %.9g rad\n",checks,(double)maxAngle);
    return 0;
}
