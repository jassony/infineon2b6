#include "../ConfigWizard/FocTiming_Cfg.h"
#if FOC_AUX_ALGORITHMS_ENABLE
#include "hfipd_injection.h"
#include "../Utilities/no_opt.h"
#include <string.h>
#include <math.h>
#if defined(__ICCARM__)
#define XCP _Pragma("location=\".xcp_cal_m4\"")
#else
#define XCP
#endif
XCP NO_OPT volatile uint8_t Cal_HFIPD_Enable_u8=0u;
/* Odd = editing, even = complete group; change only while stopped. */
XCP NO_OPT volatile uint16_t Cal_HFIPD_Seq_u16=0u;
XCP NO_OPT volatile float Cal_HFIPD_Hf_V_f32=20.0F;
XCP NO_OPT volatile float Cal_HFIPD_Pulse_V_f32=10.0F;
XCP NO_OPT volatile float Cal_HFIPD_Kp_radpsperA_f32=2000.0F;
XCP NO_OPT volatile float Cal_HFIPD_Ki_radps2perA_f32=0.0F;
XCP NO_OPT volatile float Cal_HFIPD_Init_rad_f32=0.0F;
XCP NO_OPT volatile float Cal_HFIPD_Delay_tick_f32=2.0F;
XCP NO_OPT volatile float Cal_HFIPD_Contrast_PU_f32=0.01F;
XCP NO_OPT volatile float Cal_HFIPD_AxisHi_rad_f32=0.13962634F;
XCP NO_OPT volatile float Cal_HFIPD_IsHi_A_f32=45.0F;
XCP NO_OPT volatile uint16_t Cal_HFIPD_Track_tick_u16=300u;
XCP NO_OPT volatile uint16_t Cal_HFIPD_Settle_tick_u16=400u;
XCP NO_OPT volatile uint16_t Cal_HFIPD_Pulse_tick_u16=800u;
XCP NO_OPT volatile uint16_t Cal_HFIPD_Tail_tick_u16=800u;
XCP NO_OPT volatile uint16_t Cal_HFIPD_Gap_tick_u16=300u;
XCP NO_OPT volatile uint8_t Meas_HFIPD_Act_u8=0u;
XCP NO_OPT volatile uint8_t Meas_HFIPD_Valid_u8=0u;
XCP NO_OPT volatile uint8_t Meas_HFIPD_Stat_u8=0u;
XCP NO_OPT volatile float Meas_HFIPD_Init_rad_f32=0.0F;
XCP NO_OPT volatile float Meas_HFIPD_Track_rad_f32=0.0F;
XCP NO_OPT volatile float Meas_HFIPD_Pos_As_f32=0.0F;
XCP NO_OPT volatile float Meas_HFIPD_Neg_As_f32=0.0F;
XCP NO_OPT volatile float Meas_HFIPD_Axis_rad_f32=0.0F;
XCP NO_OPT volatile float Meas_HFIPD_Err_A_f32=0.0F;
XCP NO_OPT volatile float Meas_HFIPD_Hf_A_f32=0.0F;
XCP NO_OPT volatile float Meas_HFIPD_Out_V_f32=0.0F;
XCP NO_OPT volatile uint32_t Meas_HFIPD_Time_tick_u32=0u;
static HFIPD_State state;
static uint8_t active,session;
static uint32_t angleIndex(float rad)
{
    /* Signed half-cycle conversion stays inside int32, including rounding
     * at +/-pi. Unsigned conversion then supplies modulo-2^32 angle. */
    float index=rad*683565275.5764316F;
    if(index>=2147483520.0F) return 0x80000000u;
    return (uint32_t)(int32_t)index;
}
static void publish(void)
{
    Meas_HFIPD_Act_u8=active; Meas_HFIPD_Stat_u8=state.stage;
    Meas_HFIPD_Valid_u8=state.valid;
    Meas_HFIPD_Init_rad_f32=state.valid ? state.theta : 0.0F;
    Meas_HFIPD_Track_rad_f32=state.theta;
    Meas_HFIPD_Pos_As_f32=state.areaPos;Meas_HFIPD_Neg_As_f32=state.areaNeg;
    Meas_HFIPD_Axis_rad_f32=state.axisDifference;Meas_HFIPD_Time_tick_u32=state.tick;
    Meas_HFIPD_Err_A_f32=state.error;Meas_HFIPD_Hf_A_f32=state.amplitude;
    Meas_HFIPD_Out_V_f32=(active && !HFIPDInjection_isComplete()) ? state.voltage : 0.0F;
}
static void capture(HFIPD_Params *p)
{
    memset(p,0,sizeof(*p));
    p->hfV=Cal_HFIPD_Hf_V_f32;p->pulseV=Cal_HFIPD_Pulse_V_f32;
    p->kp=Cal_HFIPD_Kp_radpsperA_f32;p->ki=Cal_HFIPD_Ki_radps2perA_f32;
    p->initialRad=Cal_HFIPD_Init_rad_f32;p->delaySamples=Cal_HFIPD_Delay_tick_f32;
    p->contrastMin=Cal_HFIPD_Contrast_PU_f32;p->axisMaxRad=Cal_HFIPD_AxisHi_rad_f32;
    p->currentMaxA=Cal_HFIPD_IsHi_A_f32;p->track=Cal_HFIPD_Track_tick_u16;
    p->settle=Cal_HFIPD_Settle_tick_u16;p->pulse=Cal_HFIPD_Pulse_tick_u16;
    p->tail=Cal_HFIPD_Tail_tick_u16;p->gap=Cal_HFIPD_Gap_tick_u16;
}
void HFIPDInjection_begin(void)
{
    HFIPD_Params p,q;
    uint16_t seq;
    /* Stable OFF path does not scan parameters or reset the core. */
    if(Cal_HFIPD_Enable_u8==0u) return;
    session=1u;active=1u;seq=Cal_HFIPD_Seq_u16;
    capture(&p);capture(&q);
    if((seq&1u)!=0u || seq!=Cal_HFIPD_Seq_u16 || memcmp(&p,&q,sizeof(p))!=0) {
        memset(&state,0,sizeof(state));HFIPD_fail(&state,HFIPD_BAD_PARAM);
    } else (void)HFIPD_start(&state,&p);
    publish();
}
void HFIPDInjection_cancel(void)
{
    if(session==0u) return;
    active=0u;session=0u;memset(&state,0,sizeof(state));publish();
}
uint8_t HFIPDInjection_isActive(void) {return active;}
uint8_t HFIPDInjection_isComplete(void)
{return active && (state.stage==HFIPD_DONE || state.stage>=HFIPD_BAD_PARAM);}
uint8_t HFIPDInjection_consume(uint32_t *angleQ32)
{
    uint8_t valid;
    if(!HFIPDInjection_isComplete()) return 0u;
    valid=state.valid;
    if(valid && angleQ32!=0) *angleQ32=angleIndex(state.theta);
    active=0u;publish();return valid;
}
void HFIPDInjection_execute(float alphaA,float betaA,float availableV,
                            float *voltageV,uint32_t *angleQ32)
{
    *voltageV=0.0F;*angleQ32=0u;
    if(!active) return;
    if(!HFIPDInjection_isComplete()) {
        if(!(availableV>=state.p.hfV && availableV>=state.p.pulseV && isfinite(availableV)))
            HFIPD_fail(&state,HFIPD_VOLTAGE_REJECT);
        else HFIPD_step(&state,alphaA,betaA);
    }
    if(!HFIPDInjection_isComplete()) {
        *voltageV=state.voltage;*angleQ32=angleIndex(state.voltageAngle);
    }
    publish();
}

#endif /* FOC_AUX_ALGORITHMS_ENABLE */
