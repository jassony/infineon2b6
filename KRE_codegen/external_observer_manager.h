#ifndef EXTERNAL_OBSERVER_MANAGER_H
#define EXTERNAL_OBSERVER_MANAGER_H

#include <stdint.h>

#ifndef FOC_DIAG_FLUX_REFERENCE
#define FOC_DIAG_FLUX_REFERENCE (0u)
#endif

#if ((FOC_DIAG_FLUX_REFERENCE != 0u) && (FOC_DIAG_FLUX_REFERENCE != 1u))
#error "FOC_DIAG_FLUX_REFERENCE must be 0 or 1"
#endif

/* Estimator ownership is a build-time decision: production (0) runs KRE,
 * while a diagnostic reference build (1) runs the internal Flux estimator.
 * There is no runtime estimator selector. */

typedef enum
{
    ExternalObserverResetReason_none = 0u,
    ExternalObserverResetReason_initialize = 1u,
    ExternalObserverResetReason_runStart = 2u,
    ExternalObserverResetReason_runStop = 3u,
    ExternalObserverResetReason_parameterApply = 6u,
    ExternalObserverResetReason_inputInvalid = 7u,
    ExternalObserverResetReason_outputInvalid = 8u,
    ExternalObserverResetReason_unspecified = 0xFFu
} ExternalObserverResetReason;

void ExternalObserverManager_initialize(void);
void ExternalObserverManager_reset(void);
void ExternalObserverManager_resetWithReason(ExternalObserverResetReason reason);
/* Foreground preparation; returns nonzero only for a publishable pending set. */
uint8_t ExternalObserverManager_stageParameters(void);
uint8_t ExternalObserverManager_stageMotorParameters(uint16_t resistance_mOhm,
    uint16_t directInductance_uH, uint16_t quadratureInductance_uH,
    uint16_t flux_mWb, uint8_t polePairs);
uint8_t ExternalObserverManager_applyPendingParameters(void);
uint8_t ExternalObserverManager_getFocEstimate(float *electricalAngle_rad,
                                                float *mechanicalSpeed_rpm);
void ExternalObserverManager_execute(float voltageAlpha_V,
                                     float voltageBeta_V,
                                     float currentAlpha_A,
                                     float currentBeta_A);

#endif /* EXTERNAL_OBSERVER_MANAGER_H */
