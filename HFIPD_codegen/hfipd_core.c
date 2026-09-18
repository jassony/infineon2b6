#include "../ConfigWizard/FocTiming_Cfg.h"
#if FOC_AUX_ALGORITHMS_ENABLE
#include "hfipd_core.h"
#include <math.h>
#include <string.h>
#define PI_F 3.14159265358979323846F
#define TWO_PI_F (2.0F * PI_F)

float HFIPD_wrap(float a)
{
    /* Bounded inputs only: start validates the initial angle; per-step motion
     * is limited by rejecting a nonphysical >pi update, not silently clipping. */
    if (a >= PI_F) a -= TWO_PI_F;
    if (a < -PI_F) a += TWO_PI_F;
    return a;
}
void HFIPD_fail(HFIPD_State *s, uint8_t reason)
{
    s->stage=reason; s->valid=0u; s->voltage=0.0F;
}
uint8_t HFIPD_start(HFIPD_State *s, const HFIPD_Params *p)
{
    unsigned j;
    memset(s,0,sizeof(*s));
    /* Explicit comparisons reject NaN as well as infinity. Limits are the
     * supported numerical/electrical envelope, not motor-specific tuning. */
    if (!(p->hfV>0.0F && p->hfV<=100.0F && p->pulseV>0.0F && p->pulseV<=100.0F
        && p->kp>0.0F && p->kp<=100000.0F && p->ki>=0.0F && p->ki<=1000000.0F
        && p->initialRad>=-PI_F && p->initialRad<=PI_F
        && p->delaySamples>=0.0F && p->delaySamples<=19.0F
        && p->contrastMin>0.0F && p->contrastMin<1.0F
        && p->axisMaxRad>0.0F && p->axisMaxRad<PI_F/4.0F
        && p->currentMaxA>0.0F && p->currentMaxA<=1000.0F
        && p->track>=2u*HFIPD_N && p->track<=20000u
        && p->settle>=HFIPD_N && p->settle<=20000u
        && p->pulse>0u && p->pulse<=20000u && p->tail>0u && p->tail<=20000u
        && p->gap>=HFIPD_N && p->gap<=20000u)) {
        HFIPD_fail(s,HFIPD_BAD_PARAM); return 0u;
    }
    s->p=*p; s->theta=HFIPD_wrap(p->initialRad); s->stage=HFIPD_TRACK;
    for(j=0;j<HFIPD_N;j++) {
        s->carrier[j]=sinf(TWO_PI_F*(float)j/(float)HFIPD_N);
        s->demod[j]=cosf(TWO_PI_F*((float)j-p->delaySamples)/(float)HFIPD_N);
    }
    return 1u;
}
static void nextPhase(HFIPD_State *s, uint8_t phase)
{ s->stage=phase; s->phaseTick=0u; }
void HFIPD_step(HFIPD_State *s, float alphaA, float betaA)
{
    float c,sn,id,iq;
    unsigned j;
    if(s->stage==HFIPD_IDLE || s->stage>=HFIPD_BAD_PARAM || s->stage==HFIPD_DONE) return;
    s->voltage=0.0F;
    if (!(fabsf(alphaA)<=s->p.currentMaxA && fabsf(betaA)<=s->p.currentMaxA)
        || alphaA*alphaA+betaA*betaA>s->p.currentMaxA*s->p.currentMaxA) {
        HFIPD_fail(s,HFIPD_BAD_CURRENT); return;
    }
    c=cosf(s->theta); sn=sinf(s->theta);
    id=c*alphaA+sn*betaA; iq=-sn*alphaA+c*betaA;
    s->voltageAngle=s->theta;
    if(s->stage==HFIPD_TRACK || s->stage==HFIPD_CHECK_TRACK) {
        /* Eq.12: positive-frequency sliding transform; Eq.17 reconstruction. */
        const float cw=0.9510565163F, sw=0.3090169944F;
        float d=iq-s->history[HFIPD_N-1u];
        float re=cw*(s->re+d)-sw*s->im;
        float im=sw*(s->re+d)+cw*s->im;
        float recon, sign, a, delta;
        for(j=HFIPD_N-1u;j>0u;j--) s->history[j]=s->history[j-1u];
        s->history[0]=iq; s->re=re; s->im=im;
        s->amplitude=(2.0F/(float)HFIPD_N)*sqrtf(re*re+im*im);
        recon=(2.0F/(float)HFIPD_N)*(re*cw+im*sw);
        a=recon*s->demod[s->phaseTick%HFIPD_N];
        if(a>=0.0F) {
            s->signCount++;
            if(s->signCount>=1) {s->signCount=1; sign=1.0F;}
            else {s->signCount=0; sign=-1.0F;}
        } else {
            s->signCount--;
            if(s->signCount<=-1) {s->signCount=-1; sign=-1.0F;}
            else {s->signCount=0; sign=1.0F;}
        }
        s->error=0.0F;
        if(s->phaseTick>=HFIPD_N) {
            s->error=sign*s->amplitude;
            delta=HFIPD_TS*(s->p.kp*s->error+s->integral);
            if(!(fabsf(delta)<=PI_F)) { HFIPD_fail(s,HFIPD_AXIS_REJECT); return; }
            s->theta=HFIPD_wrap(s->theta-delta);
            s->integral+=HFIPD_TS*s->p.ki*s->error;
        }
        s->voltage=s->p.hfV*s->carrier[s->phaseTick%HFIPD_N];
    } else if(s->stage==HFIPD_POS) s->voltage=s->p.pulseV;
    else if(s->stage==HFIPD_POS_TAIL) s->areaPos+=HFIPD_TS*fmaxf(id,0.0F);
    else if(s->stage==HFIPD_NEG) s->voltage=-s->p.pulseV;
    else if(s->stage==HFIPD_NEG_TAIL) s->areaNeg+=HFIPD_TS*fmaxf(-id,0.0F);
    s->tick++; s->phaseTick++;
    switch(s->stage) {
    case HFIPD_TRACK:
        if(s->phaseTick>=s->p.track) {s->firstAxis=s->theta; nextPhase(s,HFIPD_CHECK_SETTLE);} break;
    case HFIPD_CHECK_SETTLE:
        if(s->phaseTick>=s->p.settle) {
            memset(s->history,0,sizeof(s->history)); s->re=0; s->im=0;
            s->integral=0; s->signCount=0;
            s->theta=HFIPD_wrap(s->p.initialRad+PI_F/4.0F);
            nextPhase(s,HFIPD_CHECK_TRACK);
        } break;
    case HFIPD_CHECK_TRACK:
        if(s->phaseTick>=s->p.track) {
            float diff=fabsf(HFIPD_wrap(s->theta-s->firstAxis));
            if(diff>PI_F/2.0F) diff=PI_F-diff;
            s->axisDifference=diff;
            if(diff>s->p.axisMaxRad) HFIPD_fail(s,HFIPD_AXIS_REJECT);
            else nextPhase(s,HFIPD_SETTLE);
        } break;
    case HFIPD_SETTLE: if(s->phaseTick>=s->p.settle) nextPhase(s,HFIPD_POS); break;
    case HFIPD_POS: if(s->phaseTick>=s->p.pulse) nextPhase(s,HFIPD_POS_TAIL); break;
    case HFIPD_POS_TAIL: if(s->phaseTick>=s->p.tail) nextPhase(s,HFIPD_GAP); break;
    case HFIPD_GAP: if(s->phaseTick>=s->p.gap) nextPhase(s,HFIPD_NEG); break;
    case HFIPD_NEG: if(s->phaseTick>=s->p.pulse) nextPhase(s,HFIPD_NEG_TAIL); break;
    case HFIPD_NEG_TAIL:
        if(s->phaseTick>=s->p.tail) {
            float sum=s->areaPos+s->areaNeg;
            if(sum>0.0F && fabsf(s->areaPos-s->areaNeg)>s->p.contrastMin*sum) {
                if(s->areaPos>s->areaNeg) s->theta=HFIPD_wrap(s->theta-PI_F);
                s->valid=1u; nextPhase(s,HFIPD_DONE);
            } else HFIPD_fail(s,HFIPD_POL_REJECT);
        } break;
    default: break;
    }
}

#endif /* FOC_AUX_ALGORITHMS_ENABLE */
