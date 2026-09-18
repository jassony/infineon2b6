#ifndef RRC_DOB_COMPENSATOR_H
#define RRC_DOB_COMPENSATOR_H

#include <stdint.h>

#include "Ifx_Math.h"

#define RRCDOB_SELECTOR_OFF     (0u)
#define RRCDOB_SELECTOR_MONITOR (1u)
#define RRCDOB_SELECTOR_APPLY   (2u)

#define RRCDOB_STATUS_IDLE                    (0u)
#define RRCDOB_STATUS_INITIALIZING            (1u)
#define RRCDOB_STATUS_MONITOR_VALID           (2u)
#define RRCDOB_STATUS_APPLY_VALID             (3u)
#define RRCDOB_STATUS_APPLY_LOCKED            (4u) /* Monitor only in a bench-locked build. */
#define RRCDOB_STATUS_PARAMETER_INVALID       (5u)
#define RRCDOB_STATUS_SPEED_OUT_OF_RANGE       (6u)
#define RRCDOB_STATUS_APPLIED_VOLTAGE_STALE   (7u)
#define RRCDOB_STATUS_ARITHMETIC_OVERFLOW      (8u)

typedef struct
{
    uint8_t selector;
    uint8_t reset;
    uint16_t resistance_mOhm;
    uint16_t inductanceD_uH;
    uint16_t inductanceQ_uH;
    uint8_t polePairs;
    Ifx_Math_Fract16 cutoffRatioQ15;
    Ifx_Math_Fract16 outputLimitQ15;
    uint16_t speedLowerLimit_rpm;
    uint16_t speedUpperLimit_rpm;
    uint16_t rampTime_ms;
    uint8_t reverseOutput; /* 0: subtract estimate; nonzero: add estimate. */
    uint16_t leadTime_us; /* Output-only sixth-harmonic advance; 0 bypasses. */
} RRCDOB_Parameters;

typedef struct
{
    Ifx_Math_CmpFract16 appliedVoltageDQ15;
    Ifx_Math_CmpFract16 voltageErrorHatDQ15;
    Ifx_Math_CmpFract16 correctionDQ15;
    Ifx_Math_CmpFract16 compensatedVoltageDQ15;
    uint16_t electricalFrequency_Hz;
    uint8_t active;
    uint8_t outputActive;
    uint8_t valid;
    uint8_t status;
    uint8_t saturated;
} RRCDOB_Output;

/* XCP calibration values. RRC-DOB is disabled by default. */
extern volatile uint8_t Cal_RRCDOB_Sel_u8;
extern volatile uint8_t Cal_RRCDOB_Rst_u8;
extern volatile uint8_t Cal_RRCDOB_Rev_u8;
extern volatile uint16_t Cal_RRCDOB_Lead_us_u16;
extern volatile Ifx_Math_Fract16 Cal_RRCDOB_WcRatio_Q15_s16;
extern volatile Ifx_Math_Fract16 Cal_RRCDOB_OutHi_Q15_s16;
extern volatile uint16_t Cal_RRCDOB_SpdLo_rpm_u16;
extern volatile uint16_t Cal_RRCDOB_SpdHi_rpm_u16;
extern volatile uint16_t Cal_RRCDOB_Ramp_ms_u16;

/* XCP runtime measurements. */
extern volatile uint8_t Meas_RRCDOB_Sel_u8; /* Applied selector; changes only while stopped. */
extern volatile uint8_t Meas_RRCDOB_Pend_u8;
extern volatile uint16_t Meas_RRCDOB_Lead_us_u16; /* Last successfully applied snapshot. */
extern volatile uint8_t Meas_RRCDOB_Act_u8;
extern volatile uint8_t Meas_RRCDOB_OutAct_u8;
extern volatile uint8_t Meas_RRCDOB_Valid_u8;
extern volatile uint8_t Meas_RRCDOB_Stat_u8;
extern volatile uint8_t Meas_RRCDOB_Sat_u8;
extern volatile uint16_t Meas_RRCDOB_Freq_Hz_u16;
extern volatile Ifx_Math_Fract16 Meas_RRCDOB_VdHat_Q15_s16;
extern volatile Ifx_Math_Fract16 Meas_RRCDOB_VqHat_Q15_s16;
extern volatile Ifx_Math_Fract16 Meas_RRCDOB_VdOut_Q15_s16;
extern volatile Ifx_Math_Fract16 Meas_RRCDOB_VqOut_Q15_s16;

void RrcDobCompensator_initialize(void);
void RrcDobCompensator_reset(void);
uint8_t RrcDobCompensator_setParameters(const RRCDOB_Parameters *parameters);
void RrcDobCompensator_captureAppliedVoltage(
    Ifx_Math_CmpFract16 voltageAlphaBetaQ15,
    uint32_t electricalAngleQ32);
uint8_t RrcDobCompensator_execute(
    Ifx_Math_CmpFract16 rawVoltageDQ_Q15,
    Ifx_Math_CmpFract16 currentDQ_Q15,
    uint32_t electricalAngleQ32,
    Ifx_Math_CmpFract16 *compensatedVoltageDQ_Q15);
void RrcDobCompensator_getOutput(RRCDOB_Output *output);

#endif /* RRC_DOB_COMPENSATOR_H */
