#ifndef APSFSM_TORQUE_COMPENSATION_ADAPTER_H
#define APSFSM_TORQUE_COMPENSATION_ADAPTER_H

#include <stdint.h>

#include "Ifx_Math.h"

#define APSFSM_TORQUE_COMP_MODE_OFF       (0u)
#define APSFSM_TORQUE_COMP_MODE_SHADOW    (1u)
#define APSFSM_TORQUE_COMP_MODE_APPLY     (2u)

#define APSFSM_TORQUE_COMP_STATUS_IDLE              (0u)
#define APSFSM_TORQUE_COMP_STATUS_WAIT_SPEED        (1u)
#define APSFSM_TORQUE_COMP_STATUS_SHADOW_VALID      (2u)
#define APSFSM_TORQUE_COMP_STATUS_APPLY_VALID       (3u)
#define APSFSM_TORQUE_COMP_STATUS_PARAMETER_INVALID (4u)
#define APSFSM_TORQUE_COMP_STATUS_NUMERICAL_INVALID (5u)

typedef struct
{
    uint8_t selector;
    uint8_t reset;
    float kHat;
    float rho_rad;
    float lambda;
    Ifx_Math_Fract16 iqUpperLimitQ15;
    Ifx_Math_Fract16 iqLowerLimitQ15;
    uint16_t speedLowerLimit_rpm;
    uint16_t speedUpperLimit_rpm;
    uint16_t settleTime_ms;
    uint16_t rampTime_ms;
} APSFSM_TorqueCompCalibration;

typedef struct
{
    float iqRawPU;
    float iqAppliedPU;
    float bHatPU;
    float cHatPU;
    float thetaMech_rad;
    float speedErrorPU;
    float covariance;
    Ifx_Math_Fract16 iqOutputQ15;
    uint8_t active;
    uint8_t outputActive;
    uint8_t valid;
    uint8_t status;
    uint8_t clipped;
} APSFSM_TorqueCompDiagnostics;

/* XCP calibration values. APSFSM is disabled by default. */
extern volatile uint8_t Cal_APSFSM_Sel_u8;
extern volatile uint8_t Cal_APSFSM_Rst_u8;
extern volatile float Cal_APSFSM_KHat_f32;
extern volatile float Cal_APSFSM_Rho_rad_f32;
extern volatile float Cal_APSFSM_Lambda_f32;
extern volatile Ifx_Math_Fract16 Cal_APSFSM_IqHi_Q15_s16;
extern volatile Ifx_Math_Fract16 Cal_APSFSM_IqLo_Q15_s16;
extern volatile uint16_t Cal_APSFSM_SpdLo_rpm_u16;
extern volatile uint16_t Cal_APSFSM_SpdHi_rpm_u16;
extern volatile uint16_t Cal_APSFSM_Settle_ms_u16;
extern volatile uint16_t Cal_APSFSM_Ramp_ms_u16;

/* XCP runtime measurements. */
extern volatile uint8_t Meas_APSFSM_Act_u8;
extern volatile uint8_t Meas_APSFSM_OutAct_u8;
extern volatile uint8_t Meas_APSFSM_Valid_u8;
extern volatile uint8_t Meas_APSFSM_Stat_u8;
extern volatile uint8_t Meas_APSFSM_Clipped_u8;
extern volatile float Meas_APSFSM_IqRaw_PU_f32;
extern volatile Ifx_Math_Fract16 Meas_APSFSM_IqOut_Q15_s16;
extern volatile float Meas_APSFSM_BHat_PU_f32;
extern volatile float Meas_APSFSM_CHat_PU_f32;
extern volatile float Meas_APSFSM_Theta_rad_f32;
extern volatile float Meas_APSFSM_SpdErr_PU_f32;
extern volatile float Meas_APSFSM_Cov_f32;

void APSFSM_TorqueComp_initialize(void);
void APSFSM_TorqueComp_reset(void);

/* The input command is passed through in off, wait, shadow, and invalid
 * states. A return value of one means that the adaptive state and published
 * diagnostics are valid for this sample; use diagnostics->outputActive to
 * distinguish shadow learning from applied compensation. */
uint8_t APSFSM_TorqueComp_execute(
    Ifx_Math_Fract16 referenceSpeedQ15,
    Ifx_Math_Fract16 measuredSpeedQ15,
    const Ifx_Math_CmpFract16 *baseDqQ15,
    uint8_t controlEligible,
    const APSFSM_TorqueCompCalibration *calibration,
    Ifx_Math_CmpFract16 *compensatedDqQ15,
    APSFSM_TorqueCompDiagnostics *diagnostics);

#endif /* APSFSM_TORQUE_COMPENSATION_ADAPTER_H */
