#include <stdio.h>
#include <math.h>
#include "../fwc_speed_recovery.h"

static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { printf("FAIL line %d\n", __LINE__); return 1; } } while (0)

int main(void)
{
    Fwc_SpeedRecovery s;
    int sign;
    unsigned k;
    int32_t integral;
    Fwc_SpeedRecovery_reset(&s);
    CHECK(s.active == 0 && s.speedCapQ15 == 0 && s.iqFilteredQ15 == 0.0F);
    CHECK(Fwc_SpeedRecovery_limitReference(&s, -32768) == -32768);
    CHECK(Fwc_SpeedRecovery_trackIntegral(&s, 100, 20000, 0, 123, 456,
        256, 15, -26215, 26214) == 456);
    for (sign = -1; sign <= 1; sign += 2)
    {
        Fwc_SpeedRecovery_setActive(&s, 1, sign*28000, sign*27000, sign*12000);
        CHECK(s.speedCapQ15 == 27000 && s.iqFilteredQ15 == sign*12000.0F);
        CHECK(Fwc_SpeedRecovery_limitReference(&s, sign*29000) == sign*27000);
        CHECK(Fwc_SpeedRecovery_limitReference(&s, sign*16000) == sign*16000);
        CHECK(Fwc_SpeedRecovery_limitReference(&s, 0) == 0);
        CHECK(Fwc_SpeedRecovery_limitReference(&s, -sign*29000) == -sign*27000);
        for (k = 0; k < 1000; ++k)
        {
            Fwc_SpeedRecovery_setActive(&s, 1, sign*28000,
                sign*(26000+(int)(k%2000)), sign*(int)k);
            CHECK(s.speedCapQ15 == 27000);
            integral = Fwc_SpeedRecovery_trackIntegral(&s, sign*100,
                sign*16000, sign*(12000+(int)(k%2)*2000-1000),
                sign*8000000, sign*8001000, 256, 15, -26215, 26214);
            CHECK(sign*integral < 8000000); /* Outward accumulation removed. */
            CHECK(sign*integral > 7900000); /* Bounded, not a feedback reset. */
        }
        integral = Fwc_SpeedRecovery_trackIntegral(&s, -sign*100,
            sign*16000, sign*12000, sign*8000000, sign*7999000,
            256, 15, -26215, 26214);
        CHECK(integral == sign*7999000); /* Reversed error can unwind. */
        Fwc_SpeedRecovery_setActive(&s, 0, 0, 0, 0);
        CHECK(s.active == 0 && s.iqFilteredQ15 == 0.0F);
        CHECK(Fwc_SpeedRecovery_limitReference(&s, sign*29000) == sign*29000);
    }
    Fwc_SpeedRecovery_setActive(&s, 1, -32768, -32768, 0);
    CHECK(s.speedCapQ15 == 32767);
    integral = Fwc_SpeedRecovery_trackIntegral(&s, 1, 32767, -32768,
        -13421056, -13421055, 32767, 0, -26215, 26214);
    CHECK(integral == -26215*512); /* Badly large gain bounded to one. */
    Fwc_SpeedRecovery_reset(&s);
    Fwc_SpeedRecovery_setActive(&s, 1, 20000, 21000, 10000);
    CHECK(s.speedCapQ15 == 20000); /* Never raise an already-lower reference. */
    CHECK(Fwc_SpeedRecovery_trackIntegral(&s, 100, 16000, 0,
        123456, 124000, -1, 15, -26215, 26214) == 123456);
    CHECK(Fwc_SpeedRecovery_trackIntegral(&s, 100, 16000, 0,
        123456, 124000, 256, 255, -26215, 26214) == 123456);

    /* Deterministic per-cycle output for independent MATLAB comparison.
     * Production target/integrator operations; deliberately no plant model. */
    Fwc_SpeedRecovery_reset(&s);
    for (k = 0; k < 160; ++k)
    {
        int16_t target;
        int active = (k >= 4 && k < 75) || (k >= 80 && k < 156);
        sign = k < 80 ? 1 : -1;
        Fwc_SpeedRecovery_setActive(&s, active, sign*28000,
            sign*(27000+(int)(k%9)*80), sign*12000);
        target = Fwc_SpeedRecovery_limitReference(&s,
            sign*(k%40 < 20 ? 29000 : 16000));
        integral = Fwc_SpeedRecovery_trackIntegral(&s,
            sign*(k%16 < 8 ? 100 : -100), sign*16000,
            sign*(12000+(int)(k%5)*600-1200), sign*8000000,
            sign*(k%16 < 8 ? 8001000 : 7999000),
            k%20 == 0 ? 0 : 256, 15, -26215, 26214);
        printf("VECTOR,%u,%d,%d,%.6f,%ld\n", k, (int)s.active,
            (int)target, (double)s.iqFilteredQ15, (long)integral);
    }
    printf("FWC speed recovery tests passed: %u checks\n", checks);
    return 0;
}
