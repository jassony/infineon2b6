#ifndef HFO_EXTERNAL_OBSERVER_ADAPTER_H
#define HFO_EXTERNAL_OBSERVER_ADAPTER_H

#include <stdint.h>

#define HFO_EXTERNAL_OBSERVER_STATUS_IDLE                  (0u)
#define HFO_EXTERNAL_OBSERVER_STATUS_VALID                 (1u)
#define HFO_EXTERNAL_OBSERVER_STATUS_NUMERICAL_INVALID     (2u)
#define HFO_EXTERNAL_OBSERVER_STATUS_PARAMETER_INVALID     (3u)
#define HFO_EXTERNAL_OBSERVER_STATUS_ACTIVE_FLUX_TOO_SMALL (4u)

/* The HFO inputs and outputs use SI units. The manager owns estimator
 * selection and supplies the latest available alpha-beta voltage/current. */
void HfoExternalObserver_initialize(void);
void HfoExternalObserver_reset(void);
void HfoExternalObserver_execute(float voltageAlpha_V,
                                 float voltageBeta_V,
                                 float currentAlpha_A,
                                 float currentBeta_A);
uint8_t HfoExternalObserver_getFocEstimate(float *electricalAngle_rad,
                                           float *mechanicalSpeed_rpm);

/* HFO-specific virtual-flux and correction gains. Motor parameters are read
 * from the existing Applied motor/MTPA calibrations used by KRE. */
extern volatile float Cal_Hfo_Kob_radps_f32;
extern volatile float Cal_Hfo_Kinj_f32;
extern volatile float Cal_Hfo_SpeedFilter_Hz_f32;

/* XCP-visible HFO state outputs. */
extern volatile float Meas_Hfo_ElecAngle_rad_f32;
extern volatile float Meas_Hfo_MechSpeed_rpm_f32;

/* Non-optimized diagnostic outputs for debugger or an extended A2L. */
extern volatile float Meas_Hfo_Id_A_f32;
extern volatile float Meas_Hfo_Iq_A_f32;
extern volatile float Meas_Hfo_PsiDInjection_Wb_f32;
extern volatile float Meas_Hfo_PsiQInjection_Wb_f32;
extern volatile float Meas_Hfo_PsiCurrentAlpha_Wb_f32;
extern volatile float Meas_Hfo_PsiCurrentBeta_Wb_f32;
extern volatile float Meas_Hfo_PsiVoltageAlpha_Wb_f32;
extern volatile float Meas_Hfo_PsiVoltageBeta_Wb_f32;
extern volatile float Meas_Hfo_PsiActiveAlpha_Wb_f32;
extern volatile float Meas_Hfo_PsiActiveBeta_Wb_f32;
extern volatile float Meas_Hfo_ActiveFluxMagnitude_Wb_f32;
extern volatile uint8_t Meas_Hfo_Valid_u8;
extern volatile uint8_t Meas_Hfo_Status_u8;

#endif /* HFO_EXTERNAL_OBSERVER_ADAPTER_H */
