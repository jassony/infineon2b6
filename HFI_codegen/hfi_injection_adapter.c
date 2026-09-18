#include "../ConfigWizard/FocTiming_Cfg.h"
#if FOC_AUX_ALGORITHMS_ENABLE
#include "hfi_injection_adapter.h"

#include "../ConfigWizard/Ifx_MS_FocSolutionF16_Cfg.h"
#include "Ifx_Math_AddSat.h"
#include "Ifx_Math_CartToPolar.h"
#include "Ifx_Math_ConvSat.h"
#include "Ifx_Math_PolarToCart.h"
#include "no_opt.h"

#if defined(__ICCARM__)
#define HFI_XCP_SECTION _Pragma("location=\".xcp_cal_m4\"")
#else
#define HFI_XCP_SECTION
#endif

#define HFI_RAMP_SAMPLES    (100u)
#define HFI_US_TO_SECONDS   (0.000001F)
#define HFI_Q32_PER_CYCLE   (4294967296.0F)

HFI_XCP_SECTION
NO_OPT volatile uint8_t Cal_Hfi_Enable_u8 = 0u;
HFI_XCP_SECTION
NO_OPT volatile float Cal_Hfi_Amplitude_V_f32 = 0.75F;
HFI_XCP_SECTION
NO_OPT volatile float Cal_Hfi_Frequency_Hz_f32 = 400.0F;

static uint32_t hfiPhaseQ32;
static uint32_t hfiPhaseIncrementQ32;
static uint16_t hfiRampSample;
static bool hfiWasActive;
static float hfiLastAmplitude_V;
static float hfiLastFrequency_Hz;
static Ifx_Math_Fract16 hfiAmplitudeQ15;

static void HfiInjection_updateCalibration(void)
{
    if (hfiLastAmplitude_V != Cal_Hfi_Amplitude_V_f32)
    {
        const float amplitudePerUnit = Cal_Hfi_Amplitude_V_f32
            / (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_VOLTAGE_V;

        hfiAmplitudeQ15 = Ifx_Math_ConvSat_Flt32ToF16(amplitudePerUnit, Ifx_Math_FractQFormat_q15);
        hfiLastAmplitude_V = Cal_Hfi_Amplitude_V_f32;
    }

    if (hfiLastFrequency_Hz != Cal_Hfi_Frequency_Hz_f32)
    {
        const float samplingTime_s = (float)IFX_MS_FOCSOLUTIONF16_CFG_SAMPLING_TIME_US * HFI_US_TO_SECONDS;
        const int64_t signedPhaseIncrement = (int64_t)(Cal_Hfi_Frequency_Hz_f32
            * samplingTime_s * HFI_Q32_PER_CYCLE);

        hfiPhaseIncrementQ32 = (uint32_t)signedPhaseIncrement;
        hfiLastFrequency_Hz = Cal_Hfi_Frequency_Hz_f32;
    }
}

void HfiInjection_init(void)
{
    HfiInjection_reset();
}

void HfiInjection_reset(void)
{
    hfiPhaseQ32 = 0u;
    hfiPhaseIncrementQ32 = 0u;
    hfiRampSample = 0u;
    hfiWasActive = false;
    hfiLastAmplitude_V = 0.0F;
    hfiLastFrequency_Hz = 0.0F;
    hfiAmplitudeQ15 = 0;
}

static bool HfiInjection_isActive(const bool kreFocAllowed)
{
    const bool active = (Cal_Hfi_Enable_u8 != 0u) && kreFocAllowed;

    if (active == false)
    {
        if (hfiWasActive == true)
        {
            HfiInjection_reset();
        }
    }
    else if (hfiWasActive == false)
    {
        hfiPhaseQ32 = 0u;
        hfiRampSample = 0u;
        hfiWasActive = true;
    }

    return active;
}

Ifx_Math_PolarFract16 HfiInjection_applyPolar(Ifx_Math_PolarFract16 voltageCommandPolar,
                                              const bool kreFocAllowed)
{
    Ifx_Math_CmpFract16 baseVoltageAlphaBeta;
    Ifx_Math_CmpFract16 hfiVoltageAlphaBeta;
    Ifx_Math_CmpFract16 injectedVoltageAlphaBeta;
    Ifx_Math_PolarFract16 hfiVoltagePolar;
    Ifx_Math_Fract16 rampedAmplitudeQ15;

    if (HfiInjection_isActive(kreFocAllowed) == false)
    {
        return voltageCommandPolar;
    }

    HfiInjection_updateCalibration();

    if (hfiRampSample < HFI_RAMP_SAMPLES)
    {
        hfiRampSample++;
    }

    rampedAmplitudeQ15 = (Ifx_Math_Fract16)(((int32_t)hfiAmplitudeQ15 * (int32_t)hfiRampSample)
        / (int32_t)HFI_RAMP_SAMPLES);

    baseVoltageAlphaBeta = Ifx_Math_PolarToCart_F16(voltageCommandPolar);
    hfiVoltagePolar.amplitude = rampedAmplitudeQ15;
    hfiVoltagePolar.angle = hfiPhaseQ32;
    hfiVoltageAlphaBeta = Ifx_Math_PolarToCart_F16(hfiVoltagePolar);

    injectedVoltageAlphaBeta.real = Ifx_Math_AddSat_F16(baseVoltageAlphaBeta.real,
        hfiVoltageAlphaBeta.real);
    injectedVoltageAlphaBeta.imag = Ifx_Math_AddSat_F16(baseVoltageAlphaBeta.imag,
        hfiVoltageAlphaBeta.imag);

    hfiPhaseQ32 += hfiPhaseIncrementQ32;

    return Ifx_Math_CartToPolar_F16(injectedVoltageAlphaBeta);
}

bool HfiInjection_isOutputActive(void)
{
    return hfiWasActive;
}

#endif /* FOC_AUX_ALGORITHMS_ENABLE */
