/* Test-only double-precision analytic oracle; production uses the real LUT. */
#include "../rrc_dob_lead.h"
#include "../../Math/src/Ifx_Math_Lut_SinCos_F16_Table.c"
static void testLeadMath(void)
{
    const uint16_t leads[]={0,25,50,75,100,150,200};
    unsigned rpm,j,n,cases=0;
    double maxGainError=0,maxPhaseError=0,maxWaveError=0;
    for(rpm=800;rpm<=4000;rpm+=25)for(j=0;j<7;++j){
        const uint32_t delta=(uint32_t)((double)rpm*4/60*FOC_CONTROL_PERIOD_US*1e-6*4294967296.0+0.5);
        const double omega=(double)delta/4294967296.0*6*6.283185307179586;
        const double phi=omega*leads[j]/FOC_CONTROL_PERIOD_US;
        const RRCDOB_LeadCoefficients c=RrcDobLead_coefficients(delta,leads[j],FOC_CONTROL_PERIOD_US);
        const double a=(double)c.currentQ16/65536,b=(double)c.previousQ16/65536;
        const double real=a-b*cos(omega),imag=b*sin(omega);
        double gainError=fabs(sqrt(real*real+imag*imag)-1);
        double phaseError=fabs(atan2(sin(atan2(imag,real)-phi),cos(atan2(imag,real)-phi)))*180/3.141592653589793;
        if(gainError>maxGainError)maxGainError=gainError;
        if(phaseError>maxPhaseError)maxPhaseError=phaseError;
        assert(gainError<=0.01 && phaseError<=1.0);
        for(n=0;n<16;++n){
            const double phase=n*omega+0.371;
            const int16_t x=(int16_t)floor(4096*sin(phase)+0.5);
            const int16_t previous=(int16_t)floor(4096*sin(phase-omega)+0.5);
            double error=fabs(RrcDobLead_predict(c,x,previous)-4096*sin(phase+phi));
            if(error>maxWaveError)maxWaveError=error;
            assert(error<12);
            assert(RrcDobLead_predict(c,0,0)==0);
            if(leads[j]==0)assert(RrcDobLead_predict(c,x,previous)==x);
        }
        ++cases;
    }
    for(j=0;j<7;++j){
        RRCDOB_LeadCoefficients c=RrcDobLead_coefficients(0,leads[j],FOC_CONTROL_PERIOD_US);
        assert(c.currentQ16-c.previousQ16==65536);
        assert(RrcDobLead_predict(c,123,123)==123);
    }
    /* Tiny angles, all four quadrants, wrap, and full calibration type range. */
    for(j=1;j<100;++j){
        RRCDOB_LeadCoefficients c=RrcDobLead_coefficients(j,65535,FOC_CONTROL_PERIOD_US);
        assert(c.currentQ16>0 && c.previousQ16>0);
        (void)RrcDobLead_predict(c,32767,-32768);
    }
    assert(RrcDobLead_sineQ30(0)==0);
    assert(RrcDobLead_sineQ30(0x40000000u)>1073600000L);
    assert(RrcDobLead_sineQ30(0x80000000u)==0);
    assert(RrcDobLead_sineQ30(0xc0000000u)<-1073600000L);
    printf("Lead math: %u cases gain error %.9g phase error %.9g deg waveform peak %.9g LSB\n",
        cases,maxGainError,maxPhaseError,maxWaveError);
}
