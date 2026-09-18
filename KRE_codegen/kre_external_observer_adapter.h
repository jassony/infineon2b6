#ifndef KRE_EXTERNAL_OBSERVER_ADAPTER_H
#define KRE_EXTERNAL_OBSERVER_ADAPTER_H

#include <stdint.h>

#define KRE_EXTERNAL_OBSERVER_STATUS_IDLE              (0u)
#define KRE_EXTERNAL_OBSERVER_STATUS_VALID             (1u)
#define KRE_EXTERNAL_OBSERVER_STATUS_NUMERICAL_INVALID (2u)
#define KRE_EXTERNAL_OBSERVER_STATUS_PARAMETER_INVALID (3u)

typedef enum
{
    KRE_EXTERNAL_OBSERVER_RST_NONE = 0u,
    KRE_EXTERNAL_OBSERVER_RST_INITIALIZE = 1u,
    KRE_EXTERNAL_OBSERVER_RST_RUN_START = 2u,
    KRE_EXTERNAL_OBSERVER_RST_RUN_STOP = 3u,
    KRE_EXTERNAL_OBSERVER_RST_PARAMETER_APPLY = 6u,
    KRE_EXTERNAL_OBSERVER_RST_INPUT_INVALID = 7u,
    KRE_EXTERNAL_OBSERVER_RST_OUTPUT_INVALID = 8u,
    KRE_EXTERNAL_OBSERVER_RST_EXTERNAL_REQUEST = 0xFFu
} KreExternalObserverResetReason;

void KreExternalObserver_initialize(void);
void KreExternalObserver_reset(void);
void KreExternalObserver_resetWithReason(KreExternalObserverResetReason reason);

/* initialize() prepares the initial parameter/coefficient snapshot.
 * Foreground-only stageParameters() captures the complete calibration set and
 * calculates coefficients only when it changed, without changing active inputs.
 * applyStagedParameters() requires caller exclusion of observer/control IRQs;
 * it publishes raw parameters AND coefficients between steps, including while
 * running. It does not reset states or clear validity on a parameter change.
 * All existing calibration inputs participate; Ts/PU bases remain build config.
 * Double reads detect writes during capture, not a multi-write XCP transaction.
 * parametersReady() reports snapshot publication state for diagnostics and
 * parameter service only. It must not gate alignment, I/f, power-stage, or
 * control enable; estimate validity alone guards KRE closed-loop ownership. */
uint8_t KreExternalObserver_stageParameters(void);
/* Stage an accepted shared motor request before its Applied mirrors change.
 * Caller commits the motor fields and this staged KRE set in the same lock. */
uint8_t KreExternalObserver_stageMotorParameters(uint16_t resistance_mOhm,
    uint16_t directInductance_uH, uint16_t quadratureInductance_uH,
    uint16_t flux_mWb, uint8_t polePairs);
uint8_t KreExternalObserver_applyStagedParameters(void);
uint8_t KreExternalObserver_parametersReady(void);

void KreExternalObserver_execute(float voltageAlpha_V,
                                 float voltageBeta_V,
                                 float currentAlpha_A,
                                 float currentBeta_A);
uint8_t KreExternalObserver_getFocEstimate(float *electricalAngle_rad,
                                           float *mechanicalSpeed_rpm);

/* KRE-only real-time calibration parameters. Motor and PU base values are
 * supplied by the project FOC calibration/configuration. */
extern volatile float Cal_Kre_Alpha_radps_f32;
extern volatile float Cal_Kre_A_radps_f32;
extern volatile float Cal_Kre_Gamma_f32;
extern volatile float Cal_Kre_SigmaEpsilon_Wb_f32;
extern volatile float Cal_Kre_SpeedFilter_Hz_f32;
/* PLL tracking parameters are independent of the 14 paper KRE parameters. */
extern volatile float Cal_Kre_PllBandwidth_Hz_f32;
extern volatile float Cal_Kre_PllDamping_f32;

extern volatile float Meas_Kre_ElecAngle_rad_f32;
extern volatile float Meas_Kre_MechSpeed_rpm_f32;
/* Preserve this established signal as the Applied PM-flux reference. */
extern volatile float Meas_Kre_RotorFluxMag_Wb_f32;
extern volatile uint8_t Meas_Kre_Valid_u8;
extern volatile uint8_t Meas_Kre_Status_u8;
/* Same delayed VI sample as the KRE update; active flux remains diagnostic
 * until the independent dead-time-corrected VAFID flux path is validated. */
extern volatile float Meas_KRE_ActFlux_Wb_f32;
/* Signed alpha/beta active flux (lambdaHat - Lq*i), Wb. Same pre-update
 * state and delayed VI sample as ActFlux magnitude; diagnostic only.
 * Cleared on reset/input or output fault. Read with Valid/Status. */
extern volatile float Meas_KRE_ActFluxA_Wb_f32;
extern volatile float Meas_KRE_ActFluxB_Wb_f32;
extern volatile float Meas_KRE_RawOmega_radps_f32;
extern volatile uint32_t Meas_KRE_RstCount_u32;
extern volatile uint8_t Meas_KRE_RstReason_u8;
extern volatile uint8_t Meas_KRE_ParamValid_u8;
extern volatile uint32_t Meas_KRE_ParamApplySeq_u32;
extern volatile uint8_t Meas_KRE_ParamPending_u8;

#endif /* KRE_EXTERNAL_OBSERVER_ADAPTER_H */
