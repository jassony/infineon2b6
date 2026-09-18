#include "../ConfigWizard/FocTiming_Cfg.h"
#include "fwc_q15_adapter.h"

#include "../Utilities/no_opt.h"

#define FWC_Q15_SAMPLE_TIME_S                 (0.0005F)
#define FWC_Q15_SQRT3                         (1.7320508F)
#define FWC_Q15_CURRENT_LIMIT_Q15             (32767u)
#define FWC_Q15_HARD_ID_LO_Q15                (-13107)
#define FWC_Q15_RECOVERY_FAST_TICKS           (2000u / FOC_CONTROL_PERIOD_US)
#define FWC_Q15_FLOAT_LIMIT                   (1000000.0F)

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Cal_FWC_Enable_u8 = 1u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_FWC_VutilTgt_PU_f32 = 0.95F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_FWC_Kp_PUperPU_f32 = 0.5F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_FWC_Ki_PUperPUs_f32 = 40.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile int16_t Cal_FWC_IdLo_Q15_s16 = FWC_Q15_HARD_ID_LO_Q15;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_FWC_IdDnRate_PUps_f32 = 4.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_FWC_IdUpRate_PUps_f32 = 1.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_FWC_SatEps_PU_f32 = 0.01F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_FWC_CurAwGain_PU_f32 = 1.0F;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_FWC_LpfTau_ms_f32 = 5.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_FWC_Hyst_PU_f32 = 0.02F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Cal_FWC_Enter_ms_u16 = 2u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint16_t Cal_FWC_Exit_ms_u16 = 20u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_FWC_VreqFlt_PU_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_FWC_VactFlt_PU_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_FWC_WeakAct_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile int16_t Meas_FWC_IdFw_Q15_s16 = 0;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_FWC_Act_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_FWC_Valid_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_FWC_Stat_u8 = FWC_Q15_STATUS_OFF;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_FWC_VutilReq_PU_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_FWC_VutilAct_PU_f32 = 0.0F;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile int16_t Meas_FWC_Vdc_Q15_s16 = 0;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile int16_t Meas_FWC_IdBase_Q15_s16 = 0;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile int16_t Meas_FWC_IdRef_Q15_s16 = 0;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile int16_t Meas_FWC_IqRef_Q15_s16 = 0;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_FWC_Sat_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_FWC_RefGov_Act_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_FWC_CurAw_Act_u8 = 0u;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_FWC_RecoveryCnt_u32 = 0u;

typedef struct
{
    volatile uint32_t sequence;
    volatile uint16_t saturationEpsilonQ15;
    volatile uint16_t currentAwGainQ15;
    volatile uint8_t enabled;
    volatile uint8_t idAtFloor;
} Fwc_Q15_FastConfigStorage;

static float fwcIntegratorPu;
static float fwcIdFwPu;
static uint8_t fwcRecoveryActive;
static float fwcRequestFilteredPu;
static float fwcActualFilteredPu;
static uint8_t fwcFilterPrimed;
static uint8_t fwcWeakActive;
static uint16_t fwcEnterCount;
static uint16_t fwcExitCount;
static uint16_t fwcRecoveryExitCount;
static uint8_t fwcCalibrationPrimed;
static float fwcPreviousTarget;
static float fwcPreviousHysteresis;
static float fwcPreviousSatEpsilon;
static uint16_t fwcPreviousEnterTicks;
static uint16_t fwcPreviousExitTicks;
static Fwc_Q15_FastConfigStorage fwcFastConfig;

/* A uint16 millisecond calibration is exact at 2 kHz (two ticks/ms).
 * Saturating counters make 0 ms an immediate decision without overflow. */
static uint16_t Fwc_Q15_confirmationTicks(uint16_t milliseconds,
                                         const uint16_t maximumMs)
{
    if (milliseconds > maximumMs)
    {
        milliseconds = maximumMs;
    }
    return (uint16_t)(milliseconds * 2u);
}

static uint8_t Fwc_Q15_confirm(const uint8_t condition, uint16_t *count,
                              const uint16_t requiredTicks)
{
    if (condition == 0u)
    {
        *count = 0u;
        return 0u;
    }
    if (*count < requiredTicks)
    {
        ++(*count);
    }
    return (*count >= requiredTicks) ? 1u : 0u;
}

static uint8_t Fwc_Q15_isFinite(const float value)
{
    return ((value == value)
        && (value < FWC_Q15_FLOAT_LIMIT)
        && (value > -FWC_Q15_FLOAT_LIMIT)) ? 1u : 0u;
}

static float Fwc_Q15_clampFloat(float value, const float lower, const float upper)
{
    if (value < lower)
    {
        value = lower;
    }
    else if (value > upper)
    {
        value = upper;
    }

    return value;
}

static int16_t Fwc_Q15_clampIdLowerLimit(const int16_t requestedIdLoQ15)
{
    int32_t value = (int32_t)requestedIdLoQ15;

    if (value < FWC_Q15_HARD_ID_LO_Q15)
    {
        value = FWC_Q15_HARD_ID_LO_Q15;
    }
    else if (value > 0)
    {
        value = 0;
    }

    return (int16_t)value;
}

static int16_t Fwc_Q15_clampId(const int16_t idQ15, const int16_t idLoQ15)
{
    int32_t value = (int32_t)idQ15;

    if (value < (int32_t)idLoQ15)
    {
        value = (int32_t)idLoQ15;
    }
    else if (value > 0)
    {
        value = 0;
    }

    return (int16_t)value;
}

static uint32_t Fwc_Q15_integerSquareRoot(uint32_t value)
{
    uint32_t result = 0u;
    uint32_t bit = (uint32_t)1u << 30u;

    while (bit > value)
    {
        bit >>= 2u;
    }

    while (bit != 0u)
    {
        if (value >= (result + bit))
        {
            value -= result + bit;
            result = (result >> 1u) + bit;
        }
        else
        {
            result >>= 1u;
        }

        bit >>= 2u;
    }

    return result;
}

static int16_t Fwc_Q15_limitIqWithIdPriority(const int16_t idQ15,
                                              const int16_t iqQ15)
{
    const int32_t id = (int32_t)idQ15;
    const int32_t iq = (int32_t)iqQ15;
    const uint32_t maximumSquare = FWC_Q15_CURRENT_LIMIT_Q15
        * FWC_Q15_CURRENT_LIMIT_Q15;
    const uint32_t idSquare = (uint32_t)(id * id);
    uint32_t iqLimit;
    int32_t limitedIq;

    if (idSquare >= maximumSquare)
    {
        return 0;
    }

    iqLimit = Fwc_Q15_integerSquareRoot(maximumSquare - idSquare);
    limitedIq = iq;
    if (limitedIq > (int32_t)iqLimit)
    {
        limitedIq = (int32_t)iqLimit;
    }
    else if (limitedIq < -(int32_t)iqLimit)
    {
        limitedIq = -(int32_t)iqLimit;
    }

    return (int16_t)limitedIq;
}

static uint16_t Fwc_Q15_floatToQ15(const float value)
{
    float limited = value;

    if (Fwc_Q15_isFinite(limited) == 0u)
    {
        limited = 0.0F;
    }
    limited = Fwc_Q15_clampFloat(limited, 0.0F, 1.0F);
    return (uint16_t)((limited * 32768.0F) + 0.5F);
}

static void Fwc_Q15_publishFastConfig(const uint8_t enabled,
                                      const float saturationEpsilonPu,
                                      const float currentAwGainPu,
                                      const uint8_t idAtFloor)
{
    fwcFastConfig.sequence++;
    fwcFastConfig.saturationEpsilonQ15 = Fwc_Q15_floatToQ15(saturationEpsilonPu);
    fwcFastConfig.currentAwGainQ15 = Fwc_Q15_floatToQ15(currentAwGainPu);
    fwcFastConfig.enabled = enabled;
    fwcFastConfig.idAtFloor = idAtFloor;
    fwcFastConfig.sequence++;
}

static void Fwc_Q15_clearDynamicState(void)
{
    fwcIntegratorPu = 0.0F;
    fwcIdFwPu = 0.0F;
    fwcRecoveryActive = 0u;
    fwcRequestFilteredPu = 0.0F;
    fwcActualFilteredPu = 0.0F;
    fwcFilterPrimed = 0u;
    fwcWeakActive = 0u;
    fwcEnterCount = 0u;
    fwcExitCount = 0u;
    fwcRecoveryExitCount = 0u;
    fwcCalibrationPrimed = 0u;
    Meas_FWC_VreqFlt_PU_f32 = 0.0F;
    Meas_FWC_VactFlt_PU_f32 = 0.0F;
    Meas_FWC_WeakAct_u8 = 0u;
    Meas_FWC_IdFw_Q15_s16 = 0;
    Meas_FWC_CurAw_Act_u8 = 0u;
    Meas_FWC_RefGov_Act_u8 = 0u;
}

static void Fwc_Q15_publishOutput(const int16_t baseIdQ15,
                                  const int16_t idQ15,
                                  const int16_t iqQ15,
                                  const uint8_t active,
                                  const uint8_t valid,
                                  const uint8_t saturated,
                                  const uint8_t recoveryActive,
                                  const uint8_t status,
                                  Fwc_Q15_Output *output)
{
    Meas_FWC_Act_u8 = active;
    Meas_FWC_Valid_u8 = valid;
    Meas_FWC_Stat_u8 = status;
    Meas_FWC_IdBase_Q15_s16 = baseIdQ15;
    Meas_FWC_IdRef_Q15_s16 = idQ15;
    Meas_FWC_IqRef_Q15_s16 = iqQ15;
    Meas_FWC_Sat_u8 = saturated;
    Meas_FWC_RefGov_Act_u8 = recoveryActive;

    if (output != 0)
    {
        output->idReferenceQ15 = idQ15;
        output->iqReferenceQ15 = iqQ15;
        output->active = active;
        output->valid = valid;
        output->saturated = saturated;
        output->recoveryActive = recoveryActive;
    }
}

void Fwc_Q15_reset(void)
{
    Fwc_Q15_clearDynamicState();
    Fwc_Q15_publishFastConfig(0u, 0.0F, 0.0F, 0u);
    Meas_FWC_Act_u8 = 0u;
    Meas_FWC_Valid_u8 = 0u;
    Meas_FWC_Stat_u8 = FWC_Q15_STATUS_OFF;
    Meas_FWC_VutilReq_PU_f32 = 0.0F;
    Meas_FWC_VutilAct_PU_f32 = 0.0F;
    Meas_FWC_Vdc_Q15_s16 = 0;
    Meas_FWC_IdBase_Q15_s16 = 0;
    Meas_FWC_IdRef_Q15_s16 = 0;
    Meas_FWC_IqRef_Q15_s16 = 0;
    Meas_FWC_Sat_u8 = 0u;
}

void Fwc_Q15_initialize(void)
{
    Fwc_Q15_reset();
}

uint8_t Fwc_Q15_getFastConfig(Fwc_Q15_FastConfig *config)
{
    uint8_t attempt;

    if (config == 0)
    {
        return 0u;
    }

    for (attempt = 0u; attempt < 2u; ++attempt)
    {
        const uint32_t sequenceStart = fwcFastConfig.sequence;
        uint32_t sequenceEnd;

        if ((sequenceStart & 1u) != 0u)
        {
            continue;
        }

        config->saturationEpsilonQ15 = fwcFastConfig.saturationEpsilonQ15;
        config->currentAwGainQ15 = fwcFastConfig.currentAwGainQ15;
        config->enabled = fwcFastConfig.enabled;
        config->idAtFloor = fwcFastConfig.idAtFloor;
        sequenceEnd = fwcFastConfig.sequence;

        if ((sequenceStart == sequenceEnd)
            && ((sequenceEnd & 1u) == 0u))
        {
            return 1u;
        }
    }

    config->enabled = 0u;
    config->idAtFloor = 0u;
    config->saturationEpsilonQ15 = 0u;
    config->currentAwGainQ15 = 0u;
    return 0u;
}


void Fwc_Q15_applyCurrentLimit(Ifx_Math_CmpFract16 *dqCommand,
                               const Ifx_Math_Fract16 idReferenceQ15,
                               const uint8_t fwcActive)
{
    if ((dqCommand == 0) || (fwcActive == 0u))
    {
        return;
    }

    /* idReferenceQ15 originates from Fwc_Q15_execute() in this same speed
     * tick. Keep a hard defensive clamp without re-sampling Cal_FWC_IdLo. */
    dqCommand->real = Fwc_Q15_clampId(idReferenceQ15, FWC_Q15_HARD_ID_LO_Q15);
    dqCommand->imag = Fwc_Q15_limitIqWithIdPriority(dqCommand->real,
        dqCommand->imag);
    Meas_FWC_IdRef_Q15_s16 = dqCommand->real;
    Meas_FWC_IqRef_Q15_s16 = dqCommand->imag;
}

void Fwc_Q15_execute(const Fwc_Q15_VoltageSnapshot *voltageSnapshot,
                     const uint8_t controlEligible,
                     const Ifx_Math_Fract16 baseIdQ15,
                     const Ifx_Math_Fract16 iqReferenceQ15,
                     Fwc_Q15_Output *output)
{
    const uint8_t enabled = (Cal_FWC_Enable_u8 != 0u) ? 1u : 0u;
    const int16_t idLoQ15 = Fwc_Q15_clampIdLowerLimit(Cal_FWC_IdLo_Q15_s16);
    const int16_t limitedBaseIdQ15 = Fwc_Q15_clampId(baseIdQ15, idLoQ15);
    uint8_t snapshotValid = 0u;
    uint8_t saturated = 0u;
    int16_t finalIdQ15 = limitedBaseIdQ15;
    int16_t finalIqQ15 = iqReferenceQ15;

    if (voltageSnapshot != 0)
    {
        Meas_FWC_Vdc_Q15_s16 = voltageSnapshot->dcLinkVoltageQ15;
        if ((voltageSnapshot->valid != 0u)
            && (voltageSnapshot->dcLinkVoltageQ15 > 0)
            && (voltageSnapshot->requestedVoltageQ15 >= 0)
            && (voltageSnapshot->actualVoltageQ15 >= 0))
        {
            const float dcLink = (float)voltageSnapshot->dcLinkVoltageQ15;
            const float requested = (float)voltageSnapshot->requestedVoltageQ15;
            const float actual = (float)voltageSnapshot->actualVoltageQ15;

            Meas_FWC_VutilReq_PU_f32 = FWC_Q15_SQRT3 * requested / dcLink;
            Meas_FWC_VutilAct_PU_f32 = FWC_Q15_SQRT3 * actual / dcLink;
            if ((Fwc_Q15_isFinite(Meas_FWC_VutilReq_PU_f32) != 0u)
                && (Fwc_Q15_isFinite(Meas_FWC_VutilAct_PU_f32) != 0u))
            {
                snapshotValid = 1u;
                saturated = voltageSnapshot->saturated;
            }
            else
            {
                Meas_FWC_VutilReq_PU_f32 = 0.0F;
                Meas_FWC_VutilAct_PU_f32 = 0.0F;
            }
        }
        else
        {
            Meas_FWC_VutilReq_PU_f32 = 0.0F;
            Meas_FWC_VutilAct_PU_f32 = 0.0F;
        }
    }
    else
    {
        Meas_FWC_Vdc_Q15_s16 = 0;
        Meas_FWC_VutilReq_PU_f32 = 0.0F;
        Meas_FWC_VutilAct_PU_f32 = 0.0F;
    }

    if (enabled == 0u)
    {
        Fwc_Q15_clearDynamicState();
        Fwc_Q15_publishFastConfig(0u, 0.0F, 0.0F, 0u);
        Fwc_Q15_publishOutput(baseIdQ15, baseIdQ15, iqReferenceQ15, 0u, 0u,
            0u, 0u, FWC_Q15_STATUS_OFF, output);
        return;
    }

    if (controlEligible == 0u)
    {
        Fwc_Q15_clearDynamicState();
        Fwc_Q15_publishFastConfig(0u, 0.0F, 0.0F, 0u);
        Fwc_Q15_publishOutput(baseIdQ15, baseIdQ15, iqReferenceQ15, 0u, 0u,
            0u, 0u, FWC_Q15_STATUS_INELIGIBLE, output);
        return;
    }

    if (snapshotValid == 0u)
    {
        Fwc_Q15_clearDynamicState();
        Fwc_Q15_publishFastConfig(0u, 0.0F, 0.0F, 0u);
        Fwc_Q15_publishOutput(limitedBaseIdQ15, limitedBaseIdQ15, iqReferenceQ15,
            0u, 0u, 0u, 0u, FWC_Q15_STATUS_INVALID_VDC, output);
        return;
    }

    {
        float voltageTarget = Cal_FWC_VutilTgt_PU_f32;
        float proportionalGain = Cal_FWC_Kp_PUperPU_f32;
        float integralGain = Cal_FWC_Ki_PUperPUs_f32;
        float idDownRate = Cal_FWC_IdDnRate_PUps_f32;
        float idUpRate = Cal_FWC_IdUpRate_PUps_f32;
        float saturationEpsilon = Cal_FWC_SatEps_PU_f32;
        float currentAwGain = Cal_FWC_CurAwGain_PU_f32;
        float filterTauMs = Cal_FWC_LpfTau_ms_f32;
        float hysteresisPu = Cal_FWC_Hyst_PU_f32;
        const uint16_t enterTicks = Fwc_Q15_confirmationTicks(Cal_FWC_Enter_ms_u16, 100u);
        const uint16_t exitTicks = Fwc_Q15_confirmationTicks(Cal_FWC_Exit_ms_u16, 1000u);
        float hysteresisMaximum;
        float voltageHigh;
        float voltageLow;
        const float idMaximumMagnitudePu = -(float)idLoQ15 / 32768.0F;
        float errorPu;
        float controllerUnsaturatedPu;
        float controllerTargetPu;
        float candidateIntegratorPu;
        float rateStepPu;
        int16_t idFwQ15;

        if (Fwc_Q15_isFinite(voltageTarget) == 0u)
        {
            voltageTarget = 0.95F;
        }
        if (Fwc_Q15_isFinite(proportionalGain) == 0u)
        {
            proportionalGain = 0.0F;
        }
        if (Fwc_Q15_isFinite(integralGain) == 0u)
        {
            integralGain = 0.0F;
        }
        if (Fwc_Q15_isFinite(idDownRate) == 0u)
        {
            idDownRate = 0.0F;
        }
        if (Fwc_Q15_isFinite(idUpRate) == 0u)
        {
            idUpRate = 0.0F;
        }
        if (Fwc_Q15_isFinite(saturationEpsilon) == 0u)
        {
            saturationEpsilon = 0.0F;
        }
        if (Fwc_Q15_isFinite(currentAwGain) == 0u)
        {
            currentAwGain = 0.0F;
        }

        voltageTarget = Fwc_Q15_clampFloat(voltageTarget, 0.0F, 1.0F);
        proportionalGain = Fwc_Q15_clampFloat(proportionalGain, 0.0F, 100.0F);
        integralGain = Fwc_Q15_clampFloat(integralGain, 0.0F, 10000.0F);
        idDownRate = Fwc_Q15_clampFloat(idDownRate, 0.0F, 100.0F);
        idUpRate = Fwc_Q15_clampFloat(idUpRate, 0.0F, 100.0F);
        saturationEpsilon = Fwc_Q15_clampFloat(saturationEpsilon, 0.0F, 1.0F);
        currentAwGain = Fwc_Q15_clampFloat(currentAwGain, 0.0F, 1.0F);

        if (Fwc_Q15_isFinite(filterTauMs) == 0u)
        {
            filterTauMs = 5.0F;
        }
        if (Fwc_Q15_isFinite(hysteresisPu) == 0u)
        {
            hysteresisPu = 0.02F;
        }
        filterTauMs = Fwc_Q15_clampFloat(filterTauMs, 0.0F, 100.0F);
        hysteresisMaximum = Fwc_Q15_clampFloat(0.05F, 0.0F, voltageTarget);
        hysteresisMaximum = Fwc_Q15_clampFloat(hysteresisMaximum, 0.0F, 1.0F - voltageTarget);
        hysteresisPu = Fwc_Q15_clampFloat(hysteresisPu, 0.0F, hysteresisMaximum);
        voltageHigh = voltageTarget + hysteresisPu;
        voltageLow = voltageTarget - hysteresisPu;

        /* Only confirmation histories affected by an effective calibration
         * change restart. Online LPF changes preserve both LPF and PI state. */
        if ((fwcCalibrationPrimed == 0u) || (fwcPreviousTarget != voltageTarget)
            || (fwcPreviousHysteresis != hysteresisPu) || (fwcPreviousEnterTicks != enterTicks))
        {
            fwcEnterCount = 0u;
        }
        if ((fwcCalibrationPrimed == 0u) || (fwcPreviousTarget != voltageTarget)
            || (fwcPreviousHysteresis != hysteresisPu) || (fwcPreviousExitTicks != exitTicks))
        {
            fwcExitCount = 0u;
        }
        if ((fwcCalibrationPrimed == 0u) || (fwcPreviousExitTicks != exitTicks)
            || (fwcPreviousSatEpsilon != saturationEpsilon))
        {
            fwcRecoveryExitCount = 0u;
        }
        fwcPreviousTarget = voltageTarget;
        fwcPreviousHysteresis = hysteresisPu;
        fwcPreviousEnterTicks = enterTicks;
        fwcPreviousExitTicks = exitTicks;
        fwcPreviousSatEpsilon = saturationEpsilon;
        fwcCalibrationPrimed = 1u;

        if ((fwcFilterPrimed == 0u) || (filterTauMs == 0.0F))
        {
            fwcRequestFilteredPu = Meas_FWC_VutilReq_PU_f32;
            fwcActualFilteredPu = Meas_FWC_VutilAct_PU_f32;
            fwcFilterPrimed = 1u;
        }
        else
        {
            const float alpha = 0.5F / (filterTauMs + 0.5F);
            fwcRequestFilteredPu += alpha * (Meas_FWC_VutilReq_PU_f32 - fwcRequestFilteredPu);
            fwcActualFilteredPu += alpha * (Meas_FWC_VutilAct_PU_f32 - fwcActualFilteredPu);
        }
        Meas_FWC_VreqFlt_PU_f32 = fwcRequestFilteredPu;
        Meas_FWC_VactFlt_PU_f32 = fwcActualFilteredPu;

        if ((fwcWeakActive == 0u)
            && (Fwc_Q15_confirm((fwcRequestFilteredPu > voltageHigh) ? 1u : 0u,
                &fwcEnterCount, enterTicks) != 0u))
        {
            fwcWeakActive = 1u;
            fwcEnterCount = 0u;
        }
        errorPu = 0.0F;
        if (fwcWeakActive != 0u)
        {
            if (fwcRequestFilteredPu > voltageHigh)
            {
                errorPu = Fwc_Q15_clampFloat(fwcRequestFilteredPu - voltageHigh, 0.0F, 1.0F);
            }
            else if (fwcRequestFilteredPu < voltageLow)
            {
                errorPu = Fwc_Q15_clampFloat(fwcRequestFilteredPu - voltageLow, -1.0F, 0.0F);
            }
        }
        controllerUnsaturatedPu = (proportionalGain * errorPu) + fwcIntegratorPu;
        controllerTargetPu = Fwc_Q15_clampFloat(controllerUnsaturatedPu,
            0.0F, idMaximumMagnitudePu);
        candidateIntegratorPu = fwcIntegratorPu
            + (integralGain * FWC_Q15_SAMPLE_TIME_S * errorPu);

        /* Conditional integration: hold inside the deadband and never
         * integrate farther into either Id boundary. */
        if ((fwcWeakActive != 0u)
            && !(((controllerUnsaturatedPu >= idMaximumMagnitudePu) && (errorPu > 0.0F))
            || ((controllerUnsaturatedPu <= 0.0F) && (errorPu < 0.0F))))
        {
            fwcIntegratorPu = Fwc_Q15_clampFloat(candidateIntegratorPu,
                -idMaximumMagnitudePu, idMaximumMagnitudePu);
            controllerTargetPu = Fwc_Q15_clampFloat(
                (proportionalGain * errorPu) + fwcIntegratorPu,
                0.0F, idMaximumMagnitudePu);
        }
        if (fwcWeakActive == 0u)
        {
            controllerTargetPu = 0.0F;
        }

        if (controllerTargetPu > fwcIdFwPu)
        {
            rateStepPu = idDownRate * FWC_Q15_SAMPLE_TIME_S;
            fwcIdFwPu += Fwc_Q15_clampFloat(controllerTargetPu - fwcIdFwPu,
                0.0F, rateStepPu);
        }
        else
        {
            rateStepPu = idUpRate * FWC_Q15_SAMPLE_TIME_S;
            fwcIdFwPu -= Fwc_Q15_clampFloat(fwcIdFwPu - controllerTargetPu,
                0.0F, rateStepPu);
        }
        fwcIdFwPu = Fwc_Q15_clampFloat(fwcIdFwPu, 0.0F, idMaximumMagnitudePu);
        idFwQ15 = (int16_t)(-(fwcIdFwPu * 32768.0F) - 0.5F);
        idFwQ15 = Fwc_Q15_clampId(idFwQ15, idLoQ15);

        finalIdQ15 = (limitedBaseIdQ15 < idFwQ15) ? limitedBaseIdQ15 : idFwQ15;
        finalIdQ15 = Fwc_Q15_clampId(finalIdQ15, idLoQ15);
        finalIqQ15 = Fwc_Q15_limitIqWithIdPriority(finalIdQ15, iqReferenceQ15);

        if (fwcRecoveryActive == 0u)
        {
            fwcRecoveryExitCount = 0u;
            if ((saturated != 0u)
                && (finalIdQ15 <= idLoQ15)
                && (voltageSnapshot->idAtFloorSaturationStreakFast
                    >= FWC_Q15_RECOVERY_FAST_TICKS))
            {
                fwcRecoveryActive = 1u;
                Meas_FWC_RecoveryCnt_u32++;
            }
        }
        else if (Fwc_Q15_confirm(((saturated == 0u)
            && (voltageSnapshot->unsaturationStreakFast >= FWC_Q15_RECOVERY_FAST_TICKS)
            && ((fwcRequestFilteredPu - fwcActualFilteredPu) <= (0.5F * saturationEpsilon))) ? 1u : 0u,
            &fwcRecoveryExitCount, exitTicks) != 0u)
        {
            fwcRecoveryActive = 0u;
            fwcRecoveryExitCount = 0u;
        }

        /* The FWC-only Id must already be released before unlatching. A
         * negative IdMap reference does not keep this latch alive. */
        if (Fwc_Q15_confirm(((fwcWeakActive != 0u)
            && (fwcRequestFilteredPu < voltageLow) && (idFwQ15 >= -1)
            && (fwcRecoveryActive == 0u)) ? 1u : 0u, &fwcExitCount, exitTicks) != 0u)
        {
            fwcIntegratorPu = 0.0F;
            fwcIdFwPu = 0.0F;
            fwcWeakActive = 0u;
            fwcExitCount = 0u;
            idFwQ15 = 0;
            finalIdQ15 = limitedBaseIdQ15;
            finalIqQ15 = Fwc_Q15_limitIqWithIdPriority(finalIdQ15, iqReferenceQ15);
        }
        Meas_FWC_WeakAct_u8 = fwcWeakActive;
        Meas_FWC_IdFw_Q15_s16 = idFwQ15;

        /* Publish the Id-floor state only after this 2 kHz output has been
         * finalized. The fast loop then measures the required continuous
         * 2 ms saturation interval while the command is actually at its
         * negative hard limit. */
        Fwc_Q15_publishFastConfig(1u, saturationEpsilon, currentAwGain,
            (finalIdQ15 <= idLoQ15) ? 1u : 0u);

        Fwc_Q15_publishOutput(limitedBaseIdQ15, finalIdQ15, finalIqQ15, 1u,
            1u, saturated, fwcRecoveryActive,
            (fwcRecoveryActive != 0u) ? FWC_Q15_STATUS_RECOVERY_GOV
            : ((saturated != 0u) ? FWC_Q15_STATUS_VOLT_SAT : FWC_Q15_STATUS_ACTIVE),
            output);
    }
}
