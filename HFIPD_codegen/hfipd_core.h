#ifndef HFIPD_CORE_H
#define HFIPD_CORE_H
#include <stdint.h>
#define HFIPD_TS 0.00005F
#define HFIPD_N 20u
enum { HFIPD_IDLE=0, HFIPD_TRACK=1, HFIPD_SETTLE=2, HFIPD_POS=3,
       HFIPD_POS_TAIL=4, HFIPD_GAP=5, HFIPD_NEG=6, HFIPD_NEG_TAIL=7,
       HFIPD_DONE=8, HFIPD_CHECK_TRACK=9, HFIPD_CHECK_SETTLE=10,
       HFIPD_BAD_PARAM=11, HFIPD_BAD_CURRENT=12, HFIPD_AXIS_REJECT=13,
       HFIPD_POL_REJECT=14, HFIPD_VOLTAGE_REJECT=15 };
typedef struct {
    float hfV, pulseV, kp, ki, initialRad, delaySamples, contrastMin, axisMaxRad, currentMaxA;
    uint16_t track, settle, pulse, tail, gap;
} HFIPD_Params;
typedef struct {
    HFIPD_Params p;
    float history[HFIPD_N], re, im, integral, theta, firstAxis, areaPos, areaNeg;
    float carrier[HFIPD_N], demod[HFIPD_N];
    float voltage, voltageAngle, error, amplitude, axisDifference;
    uint32_t tick;
    uint16_t phaseTick;
    int8_t signCount;
    uint8_t stage, valid;
} HFIPD_State;
/* All state belongs to the caller. start/reset only at an exclusive boundary. */
uint8_t HFIPD_start(HFIPD_State *s, const HFIPD_Params *p);
void HFIPD_step(HFIPD_State *s, float alphaA, float betaA);
void HFIPD_fail(HFIPD_State *s, uint8_t reason);
float HFIPD_wrap(float angle);
#endif
