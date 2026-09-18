#ifndef VAFID_PARAMETER_IDENTIFIER_H
#define VAFID_PARAMETER_IDENTIFIER_H

#include <stdint.h>

#ifndef VAFID_FEEDBACK_ALLOWED_MASK
#define VAFID_FEEDBACK_ALLOWED_MASK (0u)
#endif

#define VAFID_MODE_OFF              (0u)
#define VAFID_MODE_SHADOW           (1u)

#define VAFID_VALID_RS              (0x01u)
#define VAFID_VALID_LD              (0x02u)
#define VAFID_VALID_LQ              (0x04u)
#define VAFID_VALID_FLUX_PM         (0x08u)

#define VAFID_FEEDBACK_RS           VAFID_VALID_RS
#define VAFID_FEEDBACK_LD           VAFID_VALID_LD
#define VAFID_FEEDBACK_LQ           VAFID_VALID_LQ
#define VAFID_FEEDBACK_FLUX_PM      VAFID_VALID_FLUX_PM

#define VAFID_STATUS_OFF                    (0u)
#define VAFID_STATUS_WAIT_ELIGIBILITY       (1u)
#define VAFID_STATUS_SETTLING               (2u)
#define VAFID_STATUS_COLLECTING             (3u)
#define VAFID_STATUS_WINDOW_READY           (4u)
#define VAFID_STATUS_SHADOW_VALID           (5u)
#define VAFID_STATUS_PARAMETER_INVALID      (6u)
#define VAFID_STATUS_WINDOW_REJECTED        (7u)
#define VAFID_STATUS_RESULT_STALE           (8u)
#define VAFID_STATUS_APPLY_LOCKED           (9u)

#define VAFID_REJECT_ELIGIBILITY       (0x0001u)
#define VAFID_REJECT_DTC_CHANGED       (0x0002u)
#define VAFID_REJECT_ADC_CHANGED       (0x0004u)
#define VAFID_REJECT_PROBE_CLIPPED     (0x0008u)
#define VAFID_REJECT_OVERMODULATION    (0x0010u)
#define VAFID_REJECT_KRE_INVALID       (0x0020u)
#define VAFID_REJECT_VOLTAGE_STALE     (0x0040u)
#define VAFID_REJECT_PARAMETER         (0x0080u)
#define VAFID_REJECT_SOLVER            (0x0100u)
#define VAFID_REJECT_FIT               (0x0200u)
#define VAFID_REJECT_CONDITION         (0x0400u)
#define VAFID_REJECT_RANGE             (0x0800u)
#define VAFID_REJECT_CONSISTENCY       (0x1000u)
#define VAFID_REJECT_STALE             (0x2000u)
#define VAFID_REJECT_SPEED_RANGE       (0x4000u)

#define VAFID_FEEDBACK_RESULT_NONE     (0u)
#define VAFID_FEEDBACK_RESULT_LOCKED   (1u)
#define VAFID_FEEDBACK_RESULT_REVERTED (2u)

typedef struct
{
    float sampleTime_s;
    float currentBase_A;
    float voltageBase_V;
    float resistance_Ohm;
    float inductanceD_H;
    float inductanceQ_H;
    float permanentMagnetFlux_Wb;
} VAFID_NominalParameters;

typedef struct
{
    uint8_t kreClosedLoop;
    uint8_t apsfsmOff;
    uint8_t hfiOff;
    uint8_t rrcOutputInactive;
    uint8_t voltagePathStable;
    uint8_t overmodulationActive;
    uint32_t deadTimeConfigSignature;
    uint16_t adcSampleOffsetTicks;
} VAFID_FastEligibility;

typedef struct
{
    int16_t d;
    int16_t q;
} VAFID_DqQ15;

typedef struct
{
    int16_t dLowerQ15;
    int16_t dUpperQ15;
    int16_t qLowerQ15;
    int16_t qUpperQ15;
    uint16_t currentMagnitudeMaxQ15;
} VAFID_CurrentLimits;

typedef struct
{
    float resistance_Ohm;
    float inductanceD_H;
    float inductanceQ_H;
    float permanentMagnetFlux_Wb;
    float fitError;
    float conditionProxy;
    uint8_t validMask;
    uint32_t completedWindow;
} VAFID_Estimate;

/* XCP calibration values. Identification is disabled by default. */
extern volatile uint8_t Cal_VAFID_Mode_u8;
extern volatile uint8_t Cal_VAFID_Rst_u8;
extern volatile int16_t Cal_VAFID_Ad_Q15_s16;
extern volatile int16_t Cal_VAFID_Aq_Q15_s16;
extern volatile float Cal_VAFID_Fd_Hz_f32;
extern volatile float Cal_VAFID_Fq_Hz_f32;
extern volatile uint16_t Cal_VAFID_Settle_ms_u16;
extern volatile uint16_t Cal_VAFID_Window_ms_u16;
extern volatile uint8_t Cal_VAFID_ConsWin_u8;
extern volatile uint16_t Cal_VAFID_ConsTol_pct_u16;
/* v5 defaults: relative residual < 0.65 and reciprocal 2-norm condition
 * estimate > 1e-4, corresponding to cond(A) < 1e4. */
extern volatile uint16_t Cal_VAFID_FitHi_pct_u16;
extern volatile float Cal_VAFID_CondLo_f32;
extern volatile uint16_t Cal_VAFID_Stale_ms_u16;
extern volatile uint8_t Cal_VAFID_FbMask_u8;
extern volatile uint8_t Cal_VAFID_Apply_u8;
extern volatile uint8_t Cal_VAFID_Revert_u8;

/* XCP measurements. All values are scalar for manual A2L integration. */
extern volatile uint8_t Meas_VAFID_Act_u8;
extern volatile uint8_t Meas_VAFID_Stat_u8;
extern volatile uint8_t Meas_VAFID_ValidMask_u8;
extern volatile uint16_t Meas_VAFID_RejectMask_u16;
extern volatile int16_t Meas_VAFID_ProbeD_Q15_s16;
extern volatile int16_t Meas_VAFID_ProbeQ_Q15_s16;
extern volatile uint8_t Meas_VAFID_ProbeClip_u8;
extern volatile float Meas_VAFID_Rs_Ohm_f32;
extern volatile float Meas_VAFID_Ld_H_f32;
extern volatile float Meas_VAFID_Lq_H_f32;
extern volatile float Meas_VAFID_FluxPM_Wb_f32;
extern volatile float Meas_VAFID_ActFluxFlt_Wb_f32;
extern volatile float Meas_VAFID_IdMean_A_f32;
extern volatile float Meas_VAFID_Fit_f32;
extern volatile float Meas_VAFID_Cond_f32;
extern volatile uint32_t Meas_VAFID_Win_u32;
extern volatile uint8_t Meas_VAFID_ConsWin_u8;
extern volatile uint8_t Meas_VAFID_FbResult_u8;

void VAFID_initialize(void);
void VAFID_reset(void);
/* ISR-safe immediate invalidation. It clears fast eligibility, pending V/I
 * records, published validity and the active collection generation. */
void VAFID_abortFast(uint16_t rejectMask);

/* Call from a non-ISR context. Nominal values are diagnostic seeds only and
 * are never written back to KRE, FOC, MTPA, or another owner. */
uint8_t VAFID_configure(const VAFID_NominalParameters *nominal);
void VAFID_service(void);

/* Call once after the completed PWM path is known to qualify the next probe.
 * The caller must also use VAFID_abortFast() before probe application when a
 * present-cycle owner/gate changes. A DTC-signature or ADC-offset change
 * discards the active window and restarts the configured settling interval. */
void VAFID_setFastEligibility(const VAFID_FastEligibility *eligibility);

/* Adds the dual-frequency probe to the final local d-q current command. The
 * returned command obeys the caller's axis limits and current circle. Any
 * clipping rejects the complete identification window. */
void VAFID_applyProbe(const VAFID_DqQ15 *baseCommand,
                      const VAFID_CurrentLimits *limits,
                      VAFID_DqQ15 *probedCommand);

/* Capture the modulator's ideal PWM voltage and the independent dead-time
 * error model in SI units. The identifier uses Veff = VmodActual - VdtModel;
 * KRE ownership and its voltage input remain outside this module. */
void VAFID_captureMotorVoltage(float modulatorActualAlpha_V,
                               float modulatorActualBeta_V,
                               float deadTimeModelAlpha_V,
                               float deadTimeModelBeta_V,
                               uint8_t overmodulationActive);

/* Called beside the KRE execution. The current i[k] is consumed directly with
 * the previously captured motor-voltage/probe record u[k-1]. Because the KRE
 * outputs correspond to its own delayed V/I tuple, its angle is advanced by
 * Ts*rawOmega before constructing the slow dq frame.
 * activeFluxQualified controls only the FluxPM validity bit; Rs/Ld/Lq remain
 * available in shadow mode when the finite KRE active-flux signal is not yet
 * qualified as an unbiased source. Rs/Ld/Lq use 0.20 accepted-window fusion;
 * active flux is filtered at 25 Hz and FluxPM starts 0.35 fusion after three
 * accepted windows. The default ten-window gate controls publication only. */
void VAFID_observerStep(float currentAlpha_A,
                        float currentBeta_A,
                        float kreElectricalAngle_rad,
                        float kreRawElectricalOmega_radps,
                        float kreActiveFlux_Wb,
                        uint8_t kreValid,
                        uint8_t activeFluxQualified);

uint8_t VAFID_getEstimate(VAFID_Estimate *estimate);

/* Phase-one hard lock: with VAFID_FEEDBACK_ALLOWED_MASK == 0 every non-zero
 * request returns LOCKED and this module has no API capable of writing an
 * estimator or control parameter. */
uint8_t VAFID_requestFeedback(uint8_t requestedMask);
void VAFID_revertFeedback(void);

#endif /* VAFID_PARAMETER_IDENTIFIER_H */
