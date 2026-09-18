
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "user_funtion.h"
#include "m0_rte.h"


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 
/**
 * ?????????????
 * ??PI??????????????PTC??
 */
/* ==================== ??????? ==================== */
#define POWER_CONTROL_PERIOD_MS     10     /* ????10ms */
#define MAX_POWER_LIMIT             10000.0f  /* ????10KW */
#define MIN_POWER_LIMIT             50.0f     /* ????50W */
#define DUTY_CYCLE_MIN              0.0f      /* ?????0% */
#define DUTY_CYCLE_MAX              1.0f      /* ?????100% */
#define POWER_ERROR_THRESHOLD       100.0f     /* ??????20W */
#define PWM_MAX_VALUE             100//  1000      /* PWM?????100% */
#define PWM_PERIOD_TICKS        100           // PWM??????????

/* ==================== ??????? ==================== */
typedef struct {
    /* ???? */
    float target_power;      /* ????(W) 0-10000 */
    float current_power;     /* ????(W) */
    float output_duty;       /* ?????(0.0-1.0) */
    
    /* PID?? */
    float kp;                /* ???? */
    float ki;                /* ???? */
    float kd;                /* ???? */
    float integral;          /* ??? */
    float last_error;        /* ????????? */
    
    /* ????? */
    float base_kp;           /* ??Kp */
    float base_ki;           /* ??Ki */
    float base_kd;           /* ??Kd */
    
    /* ???? */
    float max_integral;      /* ???? */
    float max_duty_change;   /* ????????? */
    float output_limit;      /* ???? */
    
    /* ????? */
    bool is_enabled;         /* ???? */
    bool is_stable;          /* ?????? */
    uint32_t stable_counter; /* ????? */
    
    /* ???? */
    float max_error;         /* ???? */
    float avg_error;         /* ???? */
    uint32_t control_cycles; /* ?????? */
    
    /* ???? */
    float adaptive_kp;       /* ???Kp */
    float adaptive_ki;       /* ???Ki */
    float last_duty_output;  /* ??????? */
    
} PTC_PowerController;

/* ==================== ??????? ==================== */
static PTC_PowerController g_ptc_ctrl;
static PTC_PowerController* ctrl = &g_ptc_ctrl;
/* ==================== ????? ==================== */
typedef struct {
    float alpha;
    float filtered_value;
    uint32_t sample_count;
} PowerFilter;

static PowerFilter g_power_filter;

/* ==================== ?????? ==================== */
extern float g_ptc_current_power;  /* ??????? */
extern float g_target_power;       /* ??????? */

/* ==================== ???? ==================== */
extern void ptc_duty_control_init(void);
extern void PTC_Controller_SetTarget(float target_power);
extern uint16_t PTC_Controller_Update(uint8_t runCmd);
extern float PTC_Calculate_Adaptive_PID_Params(float target_power);
extern void PTC_Controller_Reset(void);
extern float LimitDutyCycle(float duty);
extern float LimitPower(float power);
extern void PWM_DUTY(void);
extern void PWM_DUTY_TEST(void);
extern float ThickFilm_Heat_Ctrl(float duty_set, float v_bus);
extern float P_MAX ;
#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

