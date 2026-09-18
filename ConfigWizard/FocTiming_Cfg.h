#ifndef FOC_TIMING_CFG_H
#define FOC_TIMING_CFG_H

/* Build-time control contract. CM0+ keeps its 80 MHz / 4000 PWM setup. */
#define FOC_PWM_FREQUENCY_HZ (20000u)
#define FOC_PWM_PERIOD_US (50u)
#ifndef FOC_CONTROL_PERIOD_US
#define FOC_CONTROL_PERIOD_US (100u)
#endif
#define FOC_CONTROL_FREQUENCY_HZ (1000000u / FOC_CONTROL_PERIOD_US)
#define FOC_PWM_PER_CONTROL (FOC_CONTROL_PERIOD_US / FOC_PWM_PERIOD_US)
#define FOC_SPEED_PERIOD_US (500u)
#define FOC_CONTROL_PER_SPEED (FOC_SPEED_PERIOD_US / FOC_CONTROL_PERIOD_US)

/* RRC-DOB runs at the control period. Apply is available by explicit
 * calibration while stopped; the runtime selector still defaults to OFF.
 * Powered Apply/FWC acceptance remains pending (RRC_DOB_codegen/APPLY100US.md). */
#ifndef FOC_RRCDOB_ENABLE
#define FOC_RRCDOB_ENABLE (1u)
#endif
#ifndef FOC_RRCDOB_APPLY_ENABLE
#define FOC_RRCDOB_APPLY_ENABLE (1u)
#endif
#if FOC_RRCDOB_APPLY_ENABLE && !FOC_RRCDOB_ENABLE
#error "RRC-DOB Apply requires RRC-DOB to be compiled"
#endif

/* The initial 10 kHz image is KRE + current/speed PI + FWC. No XCP write
 * can enable a feature that has not passed the new sample-time gate. */
#ifndef FOC_AUX_ALGORITHMS_ENABLE
#define FOC_AUX_ALGORITHMS_ENABLE (0u)
#endif
#if (FOC_CONTROL_PERIOD_US != 50u) && (FOC_CONTROL_PERIOD_US != 100u)
#error "Only the 50 us reference and 100 us control schedules are defined"
#endif
#if (FOC_CONTROL_PERIOD_US % FOC_PWM_PERIOD_US) != 0u || (FOC_SPEED_PERIOD_US % FOC_CONTROL_PERIOD_US) != 0u
#error "Control and speed periods must be integer multiples"
#endif
#if FOC_AUX_ALGORITHMS_ENABLE && (FOC_CONTROL_PERIOD_US != 50u)
#error "Auxiliary algorithms require their validated 50 us schedule"
#endif

#endif
