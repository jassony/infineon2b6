#ifndef FADO_EXTERNAL_OBSERVER_ADAPTER_H
#define FADO_EXTERNAL_OBSERVER_ADAPTER_H

#include <stdint.h>

typedef enum
{
    FocEstimatorSelector_flux = 0u,
    FocEstimatorSelector_fado = 1u,
    FocEstimatorSelector_kre = 2u,
    FocEstimatorSelector_invalid = 0xFFu
} FocEstimatorSelector;

/* Common runtime manager for the existing flux observer and external observers. */
void ExternalObserverManager_initialize(void);
void ExternalObserverManager_reset(void);
void ExternalObserverManager_setFocEstimator(FocEstimatorSelector estimator);
FocEstimatorSelector ExternalObserverManager_getFocEstimator(void);
uint8_t ExternalObserverManager_getFocEstimate(float *electricalAngle_rad,
                                                float *mechanicalSpeed_rpm);
void ExternalObserverManager_execute(float voltageAlpha_V,
                                     float voltageBeta_V,
                                     float currentAlpha_A,
                                     float currentBeta_A);

/* Compatibility entry points retained for existing FADO-only users. */
void FadoExternalObserver_initialize(void);
void FadoExternalObserver_reset(void);
void FadoExternalObserver_setFocEstimator(FocEstimatorSelector estimator);
FocEstimatorSelector FadoExternalObserver_getFocEstimator(void);
uint8_t FadoExternalObserver_getFocEstimate(float *electricalAngle_rad,
                                            float *mechanicalSpeed_rpm);
void FadoExternalObserver_execute(float voltageAlpha_V,
                                  float voltageBeta_V,
                                  float currentAlpha_A,
                                  float currentBeta_A);

/* FOC integration selection. 0 selects the existing flux observer; 1 selects
 * FADO; 2 selects KRE. */
extern volatile uint8_t Cal_Foc_EstimatorSelector_u8;
extern volatile uint8_t Meas_Foc_ActiveEstimator_u8;

/* Parameters used directly by the paper's FADO equations. */
extern volatile float Cal_Fado_StatorResistance_Ohm_f32;
extern volatile float Cal_Fado_QuadratureInductance_H_f32;
extern volatile uint8_t Cal_Fado_PolePairs_u8;
extern volatile float Cal_Fado_FluxLimit_Wb_f32;
extern volatile float Cal_Fado_Kdf_per_s_f32;
extern volatile float Cal_Fado_LowSpeedThreshold_Hz_f32;
extern volatile float Cal_Fado_Kaf_radps_f32;
extern volatile float Cal_Fado_FastT2SBandwidth_Hz_f32;
extern volatile float Cal_Fado_SlowT2SBandwidth_Hz_f32;
extern volatile float Cal_Fado_T2SDamping_f32;

/* Paper-observer state and outputs. */
extern volatile float Meas_Fado_ElecAngle_rad_f32;
extern volatile float Meas_Fado_OmegaFast_radps_f32;
extern volatile float Meas_Fado_OmegaSlow_radps_f32;
extern volatile float Meas_Fado_MechSpeed_rpm_f32;
extern volatile float Meas_Fado_LambdaAlpha1_Wb_f32;
extern volatile float Meas_Fado_LambdaBeta1_Wb_f32;
extern volatile float Meas_Fado_LambdaAlpha2_Wb_f32;
extern volatile float Meas_Fado_LambdaBeta2_Wb_f32;
extern volatile float Meas_Fado_DhatAlpha_Wb_f32;
extern volatile float Meas_Fado_DhatBeta_Wb_f32;
extern volatile float Meas_Fado_RotorFluxMag_Wb_f32;
extern volatile uint8_t Meas_Fado_KdfMode_u8;
extern volatile uint8_t Meas_Fado_KafMode_u8;
extern volatile uint8_t Meas_Fado_FluxLimited_u8;
extern volatile uint8_t Meas_Fado_Valid_u8;
extern volatile uint8_t Meas_Fado_Status_u8;
extern volatile uint8_t Meas_Fado_Active_u8;

#endif /* FADO_EXTERNAL_OBSERVER_ADAPTER_H */
