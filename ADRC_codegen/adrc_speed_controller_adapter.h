#ifndef ADRC_SPEED_CONTROLLER_ADAPTER_H
#define ADRC_SPEED_CONTROLLER_ADAPTER_H

#include <stdint.h>

#include "Ifx_Math.h"

#define ADRC_SPEED_CONTROLLER_STATUS_IDLE              (0u)
#define ADRC_SPEED_CONTROLLER_STATUS_VALID             (1u)
#define ADRC_SPEED_CONTROLLER_STATUS_PARAMETER_INVALID (2u)
#define ADRC_SPEED_CONTROLLER_STATUS_NUMERICAL_INVALID (3u)

typedef struct
{
    float criticalGain_PU_per_PU_s2;
    float controlBandwidth_radps;
    float observerBandwidth_radps;
    Ifx_Math_Fract16 iqUpperLimitQ15;
    Ifx_Math_Fract16 iqLowerLimitQ15;
} ADRC_SpeedControllerCalibration;

typedef struct
{
    float referenceFilteredPU;
    float outputPU;
    float estimatedSpeedPU;
    float estimatedAccelerationPU_per_s;
    float estimatedDisturbancePU_per_s2;
    uint8_t status;
} ADRC_SpeedControllerDiagnostics;

void ADRC_SpeedController_initialize(void);
void ADRC_SpeedController_reset(void);

/* iqReferenceQ15 is an in/out handoff value. After initialize/reset, preload
 * it with the outgoing q-current command; the first valid ADRC output starts
 * from that value, limited by the configured Iq bounds. Later calls ignore
 * the incoming value and overwrite it with the calculated ADRC command. */
uint8_t ADRC_SpeedController_execute(
    Ifx_Math_Fract16 referenceSpeedQ15,
    Ifx_Math_Fract16 measuredSpeedQ15,
    const ADRC_SpeedControllerCalibration *calibration,
    Ifx_Math_Fract16 *iqReferenceQ15,
    ADRC_SpeedControllerDiagnostics *diagnostics);

#endif /* ADRC_SPEED_CONTROLLER_ADAPTER_H */
