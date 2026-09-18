#include "external_observer_manager.h"

#include "kre_external_observer_adapter.h"

void ExternalObserverManager_initialize(void)
{
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
    KreExternalObserver_initialize();
#endif
}

void ExternalObserverManager_reset(void)
{
    ExternalObserverManager_resetWithReason(
        ExternalObserverResetReason_unspecified);
}

void ExternalObserverManager_resetWithReason(
    const ExternalObserverResetReason reason)
{
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
    KreExternalObserver_resetWithReason(
        (KreExternalObserverResetReason)reason);
#else
    (void)reason;
#endif
}

uint8_t ExternalObserverManager_stageParameters(void)
{
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
    return (KreExternalObserver_stageParameters() != 0u)
        ? Meas_KRE_ParamPending_u8 : 0u;
#else
    return 0u;
#endif
}

uint8_t ExternalObserverManager_applyPendingParameters(void)
{
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
    return KreExternalObserver_applyStagedParameters();
#else
    return 1u;
#endif
}

uint8_t ExternalObserverManager_stageMotorParameters(uint16_t resistance_mOhm,
    uint16_t directInductance_uH, uint16_t quadratureInductance_uH,
    uint16_t flux_mWb, uint8_t polePairs)
{
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
    return KreExternalObserver_stageMotorParameters(resistance_mOhm,
        directInductance_uH, quadratureInductance_uH, flux_mWb, polePairs);
#else
    (void)resistance_mOhm;
    (void)directInductance_uH;
    (void)quadratureInductance_uH;
    (void)flux_mWb;
    (void)polePairs;
    return 1u;
#endif
}

uint8_t ExternalObserverManager_getFocEstimate(float *electricalAngle_rad,
                                                float *mechanicalSpeed_rpm)
{
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
    return KreExternalObserver_getFocEstimate(electricalAngle_rad, mechanicalSpeed_rpm);
#else
    (void)electricalAngle_rad;
    (void)mechanicalSpeed_rpm;
    return 0u;
#endif
}

void ExternalObserverManager_execute(const float voltageAlpha_V,
                                     const float voltageBeta_V,
                                     const float currentAlpha_A,
                                     const float currentBeta_A)
{
#if (FOC_DIAG_FLUX_REFERENCE == 0u)
    KreExternalObserver_execute(voltageAlpha_V, voltageBeta_V, currentAlpha_A, currentBeta_A);
#else
    (void)voltageAlpha_V;
    (void)voltageBeta_V;
    (void)currentAlpha_A;
    (void)currentBeta_A;
#endif
}
