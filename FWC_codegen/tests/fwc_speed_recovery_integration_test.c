/* The runner inserts the three production MS function bodies at the marker.
 * The real Infineon PI and acceleration limiter are linked, not mocked.
 * Hardware/state-machine scheduling is not simulated by this fixture. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Ifx_Math_PiF16.h"
#include "Ifx_Math_AccelLimitF16.h"
#include "Ifx_Math_LimitF16.h"
#include "Ifx_Math_Sub.h"
#include "Ifx_MS_FocSolutionF16_Cfg.h"
#include "fwc_speed_recovery.h"

enum { Ifx_MS_FocSolutionF16_State_run = 4,
    Ifx_MS_FocSolutionF16_ControlMode_foc = 1,
    Ifx_MS_FocSolutionF16_SubState_closedLoop = 2 };
typedef struct {
    struct { int state, actualControlMode, subState; } p_status;
    struct { Ifx_Math_Fract16 estimatedSpeedQ15; } p_output;
    struct { Ifx_Math_CmpFract16 currentDQ; } focController;
    bool p_enablePowerStage, p_enableControl, p_enableDirectInterface;
    bool p_externalSpeedControllerEnabled, p_externalSpeedControllerHandoffPending;
    uint8_t (*p_externalSpeedControllerCallback)(void *, int16_t, int16_t, int16_t *);
    void *p_externalSpeedControllerContext;
    Ifx_Math_Fract16 p_previousQCommand, rateLimitInSpeedQ15;
    Ifx_Math_CmpFract16 dqCommand;
    Ifx_Math_PiF16 speedPi;
    Ifx_Math_AccelLimitF16 accelerationLimit;
    Ifx_Math_LimitF16 speedLimit;
    Fwc_SpeedRecovery p_speedRecovery;
} Ifx_MS_FocSolutionF16;

/* PRODUCTION_FUNCTIONS */

static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { printf("FAIL integration line %d\n", __LINE__); return 1; } } while (0)
static void setup(Ifx_MS_FocSolutionF16 *s, int sign)
{
    Ifx_Math_PiF16_Qformats q;
    memset(s, 0, sizeof(*s));
    s->p_status.state = 4; s->p_status.actualControlMode = 1; s->p_status.subState = 2;
    s->p_enablePowerStage = true; s->p_enableControl = true;
    s->rateLimitInSpeedQ15 = sign*28000; s->p_output.estimatedSpeedQ15 = sign*27000;
    s->dqCommand.imag = sign*12000; s->focController.currentDQ.imag = sign*12000;
    s->speedLimit.p_lowerLimit = -30000; s->speedLimit.p_upperLimit = 30000;
    Ifx_Math_AccelLimitF16_init(&s->accelerationLimit);
    Ifx_Math_AccelLimitF16_setSpeedStepPreviousValue(&s->accelerationLimit, sign*28000);
    Ifx_Math_AccelLimitF16_setSpeedStepUpLimit(&s->accelerationLimit, 4*32768);
    Ifx_Math_AccelLimitF16_setSpeedStepDownLimit(&s->accelerationLimit, 6*32768);
    q.qFormatAntiWindupGainSamplingTime = Ifx_Math_FractQFormat_q15;
    q.qFormatPropGain = Ifx_Math_FractQFormat_q15;
    q.qFormatIntegGainSamplingTime = Ifx_Math_FractQFormat_q15;
    q.qFormatOutput = Ifx_Math_FractQFormat_q15;
    q.qFormatError = Ifx_Math_FractQFormat_q14;
    Ifx_Math_PiF16_init(&s->speedPi, q);
    Ifx_Math_PiF16_setPropGain(&s->speedPi, 8192);
    Ifx_Math_PiF16_setIntegGainSamplingTime(&s->speedPi, IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_KI_TS_Q);
    Ifx_Math_PiF16_setAntiWindupGainSamplingTime(&s->speedPi, IFX_MS_FOCSOLUTIONF16_CFG_SPEED_PI_KAW_TS_Q);
    Ifx_Math_PiF16_setUpperLimit(&s->speedPi, 26214);
    Ifx_Math_PiF16_setLowerLimit(&s->speedPi, -26215);
    Ifx_Math_PiF16_setIntegPreviousValue(&s->speedPi, sign*12000);
}

int main(void)
{
    Ifx_MS_FocSolutionF16 s, copy;
    int sign, k, old, output, other;
    int32_t xi, ramp;
    Ifx_MS_FocSolutionF16_setSpeedRecovery(0, true);
    for (sign=-1; sign<=1; sign+=2)
    {
        setup(&s, sign);
        xi = s.speedPi.p_integPreviousValue;
        ramp = s.accelerationLimit.p_speedStepPreviousValue;
        Ifx_MS_FocSolutionF16_setSpeedRecovery(&s, true);
        CHECK(s.p_speedRecovery.active && s.p_speedRecovery.speedCapQ15 == 27000);
        CHECK(s.speedPi.p_integPreviousValue == xi);
        CHECK(s.accelerationLimit.p_speedStepPreviousValue == ramp);
        for (k=0; k<2000; ++k)
        {
            old = s.rateLimitInSpeedQ15;
            s.p_output.estimatedSpeedQ15 = sign*(26000+(k%2)*2000);
            Ifx_MS_FocSolutionF16_setSpeedRecovery(&s, true);
            Ifx_MS_FocSolutionF16_limitSpeed(&s, sign*29000);
            CHECK(abs(s.rateLimitInSpeedQ15-old) <= 6);
            CHECK(s.p_speedRecovery.speedCapQ15 == 27000);
            CHECK(sign*s.rateLimitInSpeedQ15 >= 27000);
        }
        CHECK(s.rateLimitInSpeedQ15 == sign*27000);
        old = s.rateLimitInSpeedQ15;
        Ifx_MS_FocSolutionF16_limitSpeed(&s, sign*16000);
        CHECK(s.rateLimitInSpeedQ15 == old-sign*6); /* Lower command accepted. */
        old = s.rateLimitInSpeedQ15;
        Ifx_MS_FocSolutionF16_limitSpeed(&s, -sign*16000);
        CHECK(s.rateLimitInSpeedQ15 == old-sign*6); /* Reverse begins deceleration. */
        xi = s.speedPi.p_integPreviousValue;
        Ifx_MS_FocSolutionF16_setSpeedRecovery(&s, false);
        CHECK(!s.p_speedRecovery.active && s.speedPi.p_integPreviousValue == xi);
        old = s.rateLimitInSpeedQ15;
        Ifx_MS_FocSolutionF16_limitSpeed(&s, sign*29000);
        CHECK(s.rateLimitInSpeedQ15 == old+sign*4); /* Release through ramp. */

        setup(&s, sign);
        Ifx_MS_FocSolutionF16_setSpeedRecovery(&s, true);
        copy = s;
        s.focController.currentDQ.imag = sign*3000;
        copy.focController.currentDQ.imag = sign*22000;
        output = Ifx_MS_FocSolutionF16_calcCurrentQRef(&s, sign*27000, 0);
        other = Ifx_MS_FocSolutionF16_calcCurrentQRef(&copy, sign*27000, 0);
        CHECK(output == other); /* No instantaneous Iq feedback feedthrough. */
        CHECK(output != s.focController.currentDQ.imag && output != copy.focController.currentDQ.imag);
        CHECK(s.speedPi.p_integPreviousValue != copy.speedPi.p_integPreviousValue);
        /* The real PI must not accumulate outward under persistent shortfall. */
        for (k=0; k<10000; ++k)
        {
            s.dqCommand.imag = Ifx_MS_FocSolutionF16_calcCurrentQRef(&s, sign*27000, 0);
        }
        CHECK(abs(s.dqCommand.imag) < 4000);
        CHECK(sign*s.dqCommand.imag > 0);
        /* Real speed error still has proportional authority. */
        copy = s;
        output = Ifx_MS_FocSolutionF16_calcCurrentQRef(&s, sign*25000, 0);
        other = Ifx_MS_FocSolutionF16_calcCurrentQRef(&copy, sign*29000, 0);
        CHECK(sign*(output-other) > 0);

        /* Disabled recovery reproduces the original library PI exactly. */
        setup(&s, sign); copy = s;
        output = Ifx_MS_FocSolutionF16_calcCurrentQRef(&s, sign*27000, 0);
        other = Ifx_Math_PiF16_execute(&copy.speedPi, sign*500);
        CHECK(output == other && s.speedPi.p_integPreviousValue == copy.speedPi.p_integPreviousValue);
    }
    /* Stop, direct mode, external owner, and invalid state clear the latch. */
    for (k=0; k<7; ++k)
    {
        setup(&s, 1); Ifx_MS_FocSolutionF16_setSpeedRecovery(&s, true);
        switch(k) {
        case 0: s.p_enableControl=false; break;
        case 1: s.p_enablePowerStage=false; break;
        case 2: s.p_enableDirectInterface=true; break;
        case 3: s.p_externalSpeedControllerEnabled=true; break;
        case 4: s.p_status.state=1; break;
        case 5: s.p_status.subState=0; break;
        default: s.p_status.actualControlMode=0; break;
        }
        Ifx_MS_FocSolutionF16_setSpeedRecovery(&s, true);
        CHECK(!s.p_speedRecovery.active && s.p_speedRecovery.iqFilteredQ15 == 0.0F);
    }
    printf("FWC recovery integration tests passed: %u checks\n", checks);
    return 0;
}
