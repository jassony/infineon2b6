#include "mex.h"
#include "../hfipd_core.h"
void mexFunction(int nlhs,mxArray **out,int nrhs,const mxArray **in)
{
    HFIPD_State s;
    HFIPD_Params p;
    const float *v,*i;
    float *y;
    size_t n,k;
    if(nrhs!=2 || nlhs!=1 || !mxIsSingle(in[0]) || mxGetNumberOfElements(in[0])!=14
        || !mxIsSingle(in[1]) || mxGetN(in[1])!=2)
        mexErrMsgIdAndTxt("HFIPD:args","single p(14), currents(N,2) required");
    v=(const float*)mxGetData(in[0]); i=(const float*)mxGetData(in[1]);
    p.hfV=v[0];p.pulseV=v[1];p.kp=v[2];p.ki=v[3];p.initialRad=v[4];
    p.delaySamples=v[5];p.contrastMin=v[6];p.axisMaxRad=v[7];p.currentMaxA=v[8];
    p.track=(uint16_t)v[9];p.settle=(uint16_t)v[10];p.pulse=(uint16_t)v[11];
    p.tail=(uint16_t)v[12];p.gap=(uint16_t)v[13];
    HFIPD_start(&s,&p); n=mxGetM(in[1]);
    out[0]=mxCreateNumericMatrix(n,8,mxSINGLE_CLASS,mxREAL); y=mxGetData(out[0]);
    for(k=0;k<n;k++) {
        HFIPD_step(&s,i[k],i[k+n]);
        y[k]=s.theta;y[k+n]=s.voltage;y[k+2*n]=s.voltageAngle;
        y[k+3*n]=(float)s.stage;y[k+4*n]=(float)s.valid;
        y[k+5*n]=s.areaPos;y[k+6*n]=s.areaNeg;y[k+7*n]=s.axisDifference;
    }
}
