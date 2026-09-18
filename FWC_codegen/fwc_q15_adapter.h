#ifndef FWC_Q15_ADAPTER_H
#define FWC_Q15_ADAPTER_H

#include <stdint.h>

#include "Ifx_Math.h"

/* FWC state values are an XCP contract. Do not reorder them. */
#define FWC_Q15_STATUS_OFF          (0u)
#define FWC_Q15_STATUS_ACTIVE       (1u)
#define FWC_Q15_STATUS_VOLT_SAT     (2u)
#define FWC_Q15_STATUS_RECOVERY_GOV (3u)
#define FWC_Q15_STATUS_INELIGIBLE   (4u)
#define FWC_Q15_STATUS_INVALID_VDC  (5u)

/* sqrt(3) represented in Q15. The fast loop uses this to compare a
 * requested/actual voltage gap with Cal_FWC_SatEps_PU_f32 without a float
 * divide: sqrt(3) * (Vreq - Vact) / Vdc > SatEps. */
#define FWC_Q15_SQRT3_Q15           (56756u)

/* The fast loop publishes this coherent snapshot after the modulator has
 * constrained the command against the sampled DC link voltage. */
typedef struct
{
    uint32_t sequence;
    uint32_t saturationStreakFast;
    uint32_t unsaturationStreakFast;
    uint32_t idAtFloorSaturationStreakFast;
    Ifx_Math_Fract16 requestedVoltageQ15;
    Ifx_Math_Fract16 actualVoltageQ15;
    Ifx_Math_Fract16 dcLinkVoltageQ15;
    uint8_t valid;
    uint8_t saturated;
} Fwc_Q15_VoltageSnapshot;

typedef struct
{
    uint8_t enabled;
    uint8_t idAtFloor;
    uint16_t saturationEpsilonQ15;
    uint16_t currentAwGainQ15;
} Fwc_Q15_FastConfig;

typedef struct
{
    Ifx_Math_Fract16 idReferenceQ15;
    Ifx_Math_Fract16 iqReferenceQ15;
    uint8_t active;
    uint8_t valid;
    uint8_t saturated;
    uint8_t recoveryActive;
} Fwc_Q15_Output;

extern volatile uint8_t Cal_FWC_Enable_u8;
extern volatile float Cal_FWC_VutilTgt_PU_f32;
extern volatile float Cal_FWC_Kp_PUperPU_f32;
extern volatile float Cal_FWC_Ki_PUperPUs_f32;
extern volatile int16_t Cal_FWC_IdLo_Q15_s16;
extern volatile float Cal_FWC_IdDnRate_PUps_f32;
extern volatile float Cal_FWC_IdUpRate_PUps_f32;
extern volatile float Cal_FWC_SatEps_PU_f32;
extern volatile float Cal_FWC_CurAwGain_PU_f32;
extern volatile float Cal_FWC_LpfTau_ms_f32;
extern volatile float Cal_FWC_Hyst_PU_f32;
extern volatile uint16_t Cal_FWC_Enter_ms_u16;
extern volatile uint16_t Cal_FWC_Exit_ms_u16;

extern volatile uint8_t Meas_FWC_Act_u8;
extern volatile uint8_t Meas_FWC_Valid_u8;
extern volatile uint8_t Meas_FWC_Stat_u8;
extern volatile float Meas_FWC_VutilReq_PU_f32;
extern volatile float Meas_FWC_VutilAct_PU_f32;
extern volatile int16_t Meas_FWC_Vdc_Q15_s16;
extern volatile int16_t Meas_FWC_IdBase_Q15_s16;
extern volatile int16_t Meas_FWC_IdRef_Q15_s16;
extern volatile int16_t Meas_FWC_IqRef_Q15_s16;
extern volatile uint8_t Meas_FWC_Sat_u8;
extern volatile uint8_t Meas_FWC_RefGov_Act_u8;
extern volatile uint8_t Meas_FWC_CurAw_Act_u8;
extern volatile uint32_t Meas_FWC_RecoveryCnt_u32;
extern volatile float Meas_FWC_VreqFlt_PU_f32;
extern volatile float Meas_FWC_VactFlt_PU_f32;
/* WeakAct is the hysteretic weakening latch; Act remains path ownership. */
extern volatile uint8_t Meas_FWC_WeakAct_u8;
extern volatile int16_t Meas_FWC_IdFw_Q15_s16;

void Fwc_Q15_initialize(void);
void Fwc_Q15_reset(void);

/* Execute once per 2 kHz speed-control boundary. baseIdQ15 is the absolute
 * IdMap output; FWC returns the more-negative absolute Id and applies an
 * Id-priority current-circle limit to Iq. */
void Fwc_Q15_execute(const Fwc_Q15_VoltageSnapshot *voltageSnapshot,
                     uint8_t controlEligible,
                     Ifx_Math_Fract16 baseIdQ15,
                     Ifx_Math_Fract16 iqReferenceQ15,
                     Fwc_Q15_Output *output);

/* Apply the final same-cycle Id-priority current-circle limit after the
 * internal speed PI has produced Iq. idReferenceQ15 is the already-clamped
 * output sampled at this 2 kHz boundary, so this call never re-reads a live
 * XCP calibration mid-cycle. This does not advance FWC states. */
void Fwc_Q15_applyCurrentLimit(Ifx_Math_CmpFract16 *dqCommand,
                               Ifx_Math_Fract16 idReferenceQ15,
                               uint8_t fwcActive);

/* The 20 kHz path reads a coherent fixed-point copy of the live 2 kHz
 * calibration. It never reads XCP float calibrations directly. */
uint8_t Fwc_Q15_getFastConfig(Fwc_Q15_FastConfig *config);

#endif /* FWC_Q15_ADAPTER_H */
