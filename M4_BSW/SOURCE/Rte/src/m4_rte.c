/*
 * m4_rte.c
 *
 *  Created on: 2026年2月24日
 *      Author: hzldy
 */


#include "m4_rte.h"
#include "no_opt.h"
#include "Ifx_MS_FocSolutionF16.h"
#include "SDL_Adc_Cfg.h"
#include "lpf1.h"
#include <math.h>

static void Rte_Bsw_To_Swc(void);
static void Rte_Swc_To_Bsw(void);

void Rte_Process(void)
{
    Rte_Bsw_To_Swc();
    Rte_Swc_To_Bsw();
}
extern NO_OPT Ifx_MS_FocSolutionF16        FocDemoClosedLoop;
extern uint8_t                             IPMFAULT_STATE;

/*===========================================================================
 * Power Estimation — Physical Loss Model
 * P_est = P_dq + P_cu + P_fe + P_inv_cond + P_inv_switch
 *
 * P_dq        = 1.5 * (Vd*Id + Vq*Iq)    [motor electromagnetic power]
 * P_cu        = K_CU * (Id² + Iq²)       [copper loss, ∝ I²]
 * P_fe        = K_FE * (ω/ω_base)²       [iron loss, ∝ ω²]
 * P_inv_cond  = K_INV_COND * I_rms       [inverter conduction loss]
 * P_inv_sw    = K_INV_SW * I_rms * Vdc   [inverter switching loss]
 *
 * System base values (from ConfigWizard):
 *   BASE_VOLTAGE = 1000V, BASE_CURRENT = 50A,
 *   BASE_SPEED   = IFX_MS_FOCSOLUTIONF16_BASE_MECH_SPEED_RPM,
 *   BASE_POWER   = 50000W
 *
 * Calibration: least-squares fit of
 *   (P_dc_measured - P_dq) vs [I², ω², I, I*Vdc]
 *   across 20–36 operating points (voltage × speed × load).
 *===========================================================================*/

/* ---- XCP-calibratable loss model coefficients ---- */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_PwrEst_KCu_unitless_f32 = 0.069212151f;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_PwrEst_KFe_unitless_f32 = 0.018992319f;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_PwrEst_KInvCond_unitless_f32 = -0.045289222f;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_PwrEst_KInvSw_unitless_f32 = -0.051495967f;

/* ---- Power-estimation input LPF calibration ----
 * The single cutoff is shared by Vd, Vq, Id, and Iq.  It affects the
 * power model only; the 0x20F debug-channel LPFs retain their raw inputs.
 * A cutoff of zero bypasses this filter.
 */
#define PWR_EST_LPF_TS_S                 0.1f
#define PWR_EST_DQ_LPF_DEFAULT_FC_HZ     1.0f
#define PWR_EST_POWER_LPF_DEFAULT_FC_HZ  0.5f
#define PWR_EST_LPF_MAX_FC_HZ            (0.5f / PWR_EST_LPF_TS_S)

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_PwrEst_DqLpfFc_Hz_f32 = PWR_EST_DQ_LPF_DEFAULT_FC_HZ;

/* ---- Output-power first-order LPF calibration ----
 * This filters the computed power after the dq loss model.  A cutoff of
 * zero bypasses the filter; the same 100 ms update period as the dq LPF is
 * used for the discrete coefficient.
 */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_PwrEst_PowerLpfFc_Hz_f32 = PWR_EST_POWER_LPF_DEFAULT_FC_HZ;

/* ---- XCP observations for each power-model contribution ---- */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_PwrEst_Pdq_W_f32 = 0.0f;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_PwrEst_Pcu_W_f32 = 0.0f;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_PwrEst_Pfe_W_f32 = 0.0f;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_PwrEst_PinvCond_W_f32 = 0.0f;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_PwrEst_PinvSw_W_f32 = 0.0f;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_PwrEst_Ptotal_W_f32 = 0.0f;
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_PwrEst_Pout_W_f32 = 0.0f;

static float   g_vdc_volts = 0.0f;    /* cached bus voltage for bus-current calc */
static LPF1_t  g_pwrEstVdLpf;
static LPF1_t  g_pwrEstVqLpf;
static LPF1_t  g_pwrEstIdLpf;
static LPF1_t  g_pwrEstIqLpf;
static bool    g_pwrEstDqLpfPrimed = false;
static LPF1_t  g_pwrEstPowerLpf;
static bool    g_pwrEstPowerLpfPrimed = false;

/*===========================================================================*/
/* 0x20F Debug Frame — physical-value Q-format packing + state-gated LPF     */
/*===========================================================================*/
/* Frame layout (8 bytes):
 *   b0..1: Vd [V] int16 Q5   (Vd_phys = int16 / 32)         range ±1024V
 *   b2..3: Vq [V] int16 Q5   (Vq_phys = int16 / 32)         range ±1024V
 *   b4..5: Id [A] int16 Q4   (Id_phys = int16 / 16)         range ±2048A
 *   b6..7: Iq [A] int16 Q4   (Iq_phys = int16 / 16)         range ±2048A
 *  Voltage uses Q5 (±1024V, 31.3mV res) to cover the full Vq swing
 *  in flux-weakening/high-speed region (Vq can exceed 128V, the Q8
 *  limit). Current stays at Q4 (62.5mA res, ample for ±50A range).
 */
#define DBG_VD_Q_SHIFT   5
#define DBG_I_Q_SHIFT    4
#define DBG_V_SCALE      (float)(1 << DBG_VD_Q_SHIFT)   /* 32 */
#define DBG_I_SCALE      (float)(1 << DBG_I_Q_SHIFT)   /* 16 */
#define DBG_V_CLIP       ((1 << 15) - 1)               /* 32767 */
#define DBG_I_CLIP       ((1 << 15) - 1)

/* State gate: require this many consecutive 'run' samples before sending
 * debug values, so transients during startup don't contaminate the log. */
#define DBG_RUN_SETTLE_CNT 10u

/* 0x20F is sent at the Rte_Process() call rate. Ts is that period in
 * seconds. If the call rate changes, alpha recomputes automatically.
 *
 *  tau (RC time constant)  = 1 / (2*pi*fc)
 *  alpha (discrete coeff)  = Ts / (tau + Ts)   [equivalent form]
 *
 *  Defaults below: Ts=100ms, tau=0.9s  →  alpha = 0.1  →  fc ≈ 0.177 Hz.
 *  Tune tau via DBG_LPF_TAU_S; raising it → smoother (more lag).
 */
#define DBG_LPF_TS_S        0.1f          /* 100 ms (Rte_Process period) */
#define DBG_LPF_TAU_S       0.9f          /* desired RC time constant [s] */
#define DBG_LPF_INIT         0.0f

/* Derived discrete alpha. Using the time-constant form because tau is
 * more intuitive to tune than fc for low-pass smoothing of slow signals.
 *   alpha = Ts / (tau + Ts)
 * If you prefer fc, replace the macro body with:
 *   LPF1_alpha_from_fc(<fc_Hz>, DBG_LPF_TS_S)  with fc = 1/(2*pi*tau). */
#define DBG_LPF_ALPHA       (LPF1_alpha_from_tau(DBG_LPF_TAU_S, DBG_LPF_TS_S))

/* 0x210 voltage-utilization LPF calibration.
 *
 * Only the published utilization field is filtered; the raw modulation ratio
 * and SVPWM overmodulation flag stay untouched for diagnostics.  A cutoff of
 * zero, an invalid cutoff, or a disabled filter bypasses the LPF.  The cutoff
 * is bounded at Nyquist because this path runs once per Rte_Process() call.
 */
#define DBG_VOLT_UTIL_LPF_DEFAULT_FC_HZ  0.5f
#define DBG_VOLT_UTIL_LPF_MAX_FC_HZ      (0.5f / DBG_LPF_TS_S)

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Cal_Rte_VoltUtilLpfEnable_u8 = 1u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Cal_Rte_VoltUtilLpfFc_Hz_f32 = DBG_VOLT_UTIL_LPF_DEFAULT_FC_HZ;

/* XCP observations for calibrating the voltage-utilization LPF. */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_Rte_VoltUtilRaw_pct_f32 = 0.0f;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_Rte_VoltUtilFlt_pct_f32 = 0.0f;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile float Meas_Rte_VoltUtilLpfAlpha_f32 = 1.0f;

/* Source-side diagnostics for voltage-utilization validity.  These are
 * deliberately captured in the RTE task, outside the FOC control law. */
#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_Rte_VoltUtilState_u8 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile int16_t Meas_Rte_VoltUtilAct_Q15_s16 = 0;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile int16_t Meas_Rte_VoltUtilVdc_Q15_s16 = 0;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_Rte_VoltUtilValid_u8 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint8_t Meas_Rte_VoltUtilForceDuty_u8 = 0u;

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m4"
#endif
NO_OPT volatile uint32_t Meas_Rte_VoltUtilUpdCnt_u32 = 0u;

static LPF1_t g_dbg_VdLpf;
static LPF1_t g_dbg_VqLpf;
static LPF1_t g_dbg_IdLpf;
static LPF1_t g_dbg_IqLpf;
static LPF1_t g_dbgVoltUtilLpf;
static bool g_dbgVoltUtilLpfPrimed = false;
static uint16_t g_dbgRunSettleCnt = 0;   /* consecutive run samples (state gate) */

typedef struct
{
    float modRatio;
    float filteredPct;
} RteVoltUtilSample;

static RteVoltUtilSample g_rteVoltUtilSample;

static float Rte_VoltUtilLpfAlpha(void)
{
    float cutoffHz = Cal_Rte_VoltUtilLpfFc_Hz_f32;

    /* The ordered comparison also rejects NaN. */
    if ((Cal_Rte_VoltUtilLpfEnable_u8 == 0u) || !(cutoffHz >= 0.0f))
    {
        return 1.0f;  /* bypass */
    }

    if (cutoffHz > DBG_VOLT_UTIL_LPF_MAX_FC_HZ)
    {
        cutoffHz = DBG_VOLT_UTIL_LPF_MAX_FC_HZ;
    }

    return LPF1_alpha_from_fc(cutoffHz, DBG_LPF_TS_S);
}

static bool Rte_VoltUtilIsActive(const Ifx_MS_FocSolutionF16_State state)
{
    return ((state == Ifx_MS_FocSolutionF16_State_run)
        || (state == Ifx_MS_FocSolutionF16_State_rampDown));
}

/* Keep the observation contract independent from the power-estimation state
 * gate below.  The numerator and denominator are the exact Q15 values used by
 * the modulator, so the reported ratio is not affected by a second, delayed
 * foreground ADC read. */
static void Rte_UpdateVoltUtilObservation(void)
{
    const Ifx_MS_FocSolutionF16_State state = FocDemoClosedLoop.p_status.state;
    const int16_t actualVoltageQ15 =
        (int16_t)FocDemoClosedLoop.modulator.p_output.actualVoltage.amplitude;
    const int16_t dcLinkVoltageQ15 =
        (int16_t)FocDemoClosedLoop.measurementADCCYT2B7.p_output.dcLinkVoltageQ15;
    const bool forceDutyEnabled = (FocDemoClosedLoop.modulator.p_forceDutyEnable != false);
    const bool active = Rte_VoltUtilIsActive(state);
    const bool valid = ((active != false)
        && (dcLinkVoltageQ15 > 0)
        && (actualVoltageQ15 >= 0)
        && (forceDutyEnabled == false));
    const float alpha = Rte_VoltUtilLpfAlpha();
    float rawPct = 0.0f;
    float filteredPct = 0.0f;
    float modRatio = 0.0f;

    Meas_Rte_VoltUtilState_u8 = (uint8_t)state;
    Meas_Rte_VoltUtilAct_Q15_s16 = actualVoltageQ15;
    Meas_Rte_VoltUtilVdc_Q15_s16 = dcLinkVoltageQ15;
    Meas_Rte_VoltUtilForceDuty_u8 = (forceDutyEnabled != false) ? 1u : 0u;
    Meas_Rte_VoltUtilValid_u8 = (valid != false) ? 1u : 0u;
    Meas_Rte_VoltUtilUpdCnt_u32++;
    Meas_Rte_VoltUtilLpfAlpha_f32 = alpha;

    if (valid != false)
    {
        modRatio = (float)actualVoltageQ15 / (float)dcLinkVoltageQ15;
        if (!(modRatio >= 0.0f))
        {
            modRatio = 0.0f;
        }

        rawPct = modRatio * 1.7320508f * 100.0f;
        if (!(rawPct >= 0.0f))
        {
            rawPct = 0.0f;
        }

        if (g_dbgVoltUtilLpfPrimed == false)
        {
            /* The first valid sample must be directly observable at startup. */
            LPF1_init(&g_dbgVoltUtilLpf, alpha, rawPct);
            g_dbgVoltUtilLpfPrimed = true;
            filteredPct = rawPct;
        }
        else
        {
            LPF1_set_alpha(&g_dbgVoltUtilLpf, alpha);
            filteredPct = LPF1_update(&g_dbgVoltUtilLpf, rawPct);
        }

        if (!(filteredPct >= 0.0f))
        {
            filteredPct = 0.0f;
        }
    }
    else
    {
        LPF1_reset(&g_dbgVoltUtilLpf);
        g_dbgVoltUtilLpfPrimed = false;
    }

    g_rteVoltUtilSample.modRatio = modRatio;
    g_rteVoltUtilSample.filteredPct = filteredPct;
    Meas_Rte_VoltUtilRaw_pct_f32 = rawPct;
    Meas_Rte_VoltUtilFlt_pct_f32 = filteredPct;
}

static float Rte_PwrEstLpfAlpha(float cutoffHz)
{
    /* The ordered comparison also rejects a NaN calibration value. */
    if (!(cutoffHz >= 0.0f))
    {
        return 1.0f;
    }

    if (cutoffHz > PWR_EST_LPF_MAX_FC_HZ)
    {
        cutoffHz = PWR_EST_LPF_MAX_FC_HZ;
    }

    return LPF1_alpha_from_fc(cutoffHz, PWR_EST_LPF_TS_S);
}

static float Rte_PwrEstDqLpfAlpha(void)
{
    return Rte_PwrEstLpfAlpha(Cal_PwrEst_DqLpfFc_Hz_f32);
}

static float Rte_PwrEstPowerLpfAlpha(void)
{
    return Rte_PwrEstLpfAlpha(Cal_PwrEst_PowerLpfFc_Hz_f32);
}

static void Rte_FilterPwrEstDq(float *vd, float *vq, float *id, float *iq)
{
    const float alpha = Rte_PwrEstDqLpfAlpha();

    if (g_pwrEstDqLpfPrimed == false)
    {
        /* Avoid a startup ramp: the first closed-loop sample is the state. */
        LPF1_init(&g_pwrEstVdLpf, alpha, *vd);
        LPF1_init(&g_pwrEstVqLpf, alpha, *vq);
        LPF1_init(&g_pwrEstIdLpf, alpha, *id);
        LPF1_init(&g_pwrEstIqLpf, alpha, *iq);
        g_pwrEstDqLpfPrimed = true;
        return;
    }

    LPF1_set_alpha(&g_pwrEstVdLpf, alpha);
    LPF1_set_alpha(&g_pwrEstVqLpf, alpha);
    LPF1_set_alpha(&g_pwrEstIdLpf, alpha);
    LPF1_set_alpha(&g_pwrEstIqLpf, alpha);

    *vd = LPF1_update(&g_pwrEstVdLpf, *vd);
    *vq = LPF1_update(&g_pwrEstVqLpf, *vq);
    *id = LPF1_update(&g_pwrEstIdLpf, *id);
    *iq = LPF1_update(&g_pwrEstIqLpf, *iq);
}

static float Rte_FilterPwrEstPower(float powerW)
{
    const float alpha = Rte_PwrEstPowerLpfAlpha();

    if (g_pwrEstPowerLpfPrimed == false)
    {
        /* Publish the first run sample directly instead of ramping from zero. */
        LPF1_init(&g_pwrEstPowerLpf, alpha, powerW);
        g_pwrEstPowerLpfPrimed = true;
        return powerW;
    }

    LPF1_set_alpha(&g_pwrEstPowerLpf, alpha);
    return LPF1_update(&g_pwrEstPowerLpf, powerW);
}

static void Rte_Bsw_To_Swc(void)
{
  cy_stc_adc_ch_status_t              adcChStatus;

  m4_set_data.bus_current.receive_flag = 1;
  m4_set_data.bus_voltage.receive_flag = 1;
  m4_set_data.phase_current.receive_flag = 1;
  m4_set_data.motor_speed.receive_flag = 1;
  m4_set_data.err_status.receive_flag = 1;
  m4_set_data.running_status.receive_flag = 1;
  m4_set_data.phase_current_u.receive_flag = 1;
  m4_set_data.phase_current_v.receive_flag = 1;
  m4_set_data.phase_current_w.receive_flag = 1;

//  m4_set_data.g_ad_vbat_sense.receive_flag = 1;
   if(g_vdc_para.flag)
   {
     bus_voltage = g_vdc_para.vdc;
     g_vdc_para.flag = 0;
   }
   else
   {
     Cy_Adc_Channel_GetResult(&ADC_SAR_NUM_VDC->CH[ADC_CHN_NUM_VDC], &g_vdc_para.vdc, &adcChStatus);
     if(adcChStatus.valid)
       bus_voltage = g_vdc_para.vdc;

   }

  Rte_UpdateVoltUtilObservation();

  motorspeedfeedbackq15 = FocDemoClosedLoop.p_output.estimatedSpeedQ15;

  m4_set_data.bus_voltage.value = bus_voltage;
  m4_set_data.phase_current.value = FocDemoClosedLoop.focController.currentDQ.imag*50/32768;

  /*=====================================================================*/
  /*  Power Estimation — Physical Loss Model                             */
  /*  P_est = P_dq + P_copper + P_iron + P_inv_cond + P_inv_switch      */
  /*=====================================================================*/
  /* Diagnostic request: publish the power model in every RTE cycle rather
   * than clearing it whenever the FOC main state is not run. */
  {
      /* One-time init of LPF instances (idempotent — uses init defaults) */
      static bool lpfInited = false;
      if (!lpfInited) {
          LPF1_init(&g_dbg_VdLpf, DBG_LPF_ALPHA, DBG_LPF_INIT);
          LPF1_init(&g_dbg_VqLpf, DBG_LPF_ALPHA, DBG_LPF_INIT);
          LPF1_init(&g_dbg_IdLpf, DBG_LPF_ALPHA, DBG_LPF_INIT);
          LPF1_init(&g_dbg_IqLpf, DBG_LPF_ALPHA, DBG_LPF_INIT);
          lpfInited = true;
      }

      /* ---- Step 1: Extract dq values (Q15 format) ---- */
      int32_t Vd_q15 = (int32_t)FocDemoClosedLoop.focController.voltageDQ.real;
      int32_t Vq_q15 = (int32_t)FocDemoClosedLoop.focController.voltageDQ.imag;
      int32_t Id_q15 = (int32_t)FocDemoClosedLoop.focController.currentDQ.real;
      int32_t Iq_q15 = (int32_t)FocDemoClosedLoop.focController.currentDQ.imag;

      /* ---- Step 2: Q15 → physical units ---- */
      float VdRaw = (float)Vd_q15 * (1000.0f / 32768.0f); /* V */
      float VqRaw = (float)Vq_q15 * (1000.0f / 32768.0f); /* V */
      float IdRaw = (float)Id_q15 * (50.0f / 32768.0f);   /* A */
      float IqRaw = (float)Iq_q15 * (50.0f / 32768.0f);   /* A */
      float Vd = VdRaw;
      float Vq = VqRaw;
      float Id = IdRaw;
      float Iq = IqRaw;

      /* ---- State gate: count consecutive run samples ---- */
      if (g_dbgRunSettleCnt < DBG_RUN_SETTLE_CNT) {
          g_dbgRunSettleCnt++;
      }
      /* Until settle count reached, LPF output is bypassed anyway
       * (LPF1 returns input on first samples), so raw vs filtered
       * distinction only matters for very early samples — safe. */

      /* ---- LPF the four dq channels for the 0x20F debug frame ---- */
      /* First few samples after reset: LPF1 returns input directly
       * (bypass), so the filter locks onto the signal instantly.    */
      float Vd_f = LPF1_update(&g_dbg_VdLpf, VdRaw);
      float Vq_f = LPF1_update(&g_dbg_VqLpf, VqRaw);
      float Id_f = LPF1_update(&g_dbg_IdLpf, IdRaw);
      float Iq_f = LPF1_update(&g_dbg_IqLpf, IqRaw);

      /* ---- DEBUG: pack filtered physical values (Q5/Q4 signed) ---- */
      /* Vd/Vq → Q5, Id/Iq → Q4, each as int16 on the wire.        */
      int32_t vd_q5 = (int32_t)(Vd_f * DBG_V_SCALE + (Vd_f >= 0.0f ? 0.5f : -0.5f));
      int32_t vq_q5 = (int32_t)(Vq_f * DBG_V_SCALE + (Vq_f >= 0.0f ? 0.5f : -0.5f));
      int32_t id_q4 = (int32_t)(Id_f * DBG_I_SCALE + (Id_f >= 0.0f ? 0.5f : -0.5f));
      int32_t iq_q4 = (int32_t)(Iq_f * DBG_I_SCALE + (Iq_f >= 0.0f ? 0.5f : -0.5f));

      /* Clip to int16 range to avoid wraparound on unusual spikes */
      if (vd_q5 >  DBG_V_CLIP) vd_q5 =  DBG_V_CLIP;
      if (vd_q5 < -DBG_V_CLIP) vd_q5 = -DBG_V_CLIP;
      if (vq_q5 >  DBG_V_CLIP) vq_q5 =  DBG_V_CLIP;
      if (vq_q5 < -DBG_V_CLIP) vq_q5 = -DBG_V_CLIP;
      if (id_q4 >  DBG_I_CLIP) id_q4 =  DBG_I_CLIP;
      if (id_q4 < -DBG_I_CLIP) id_q4 = -DBG_I_CLIP;
      if (iq_q4 >  DBG_I_CLIP) iq_q4 =  DBG_I_CLIP;
      if (iq_q4 < -DBG_I_CLIP) iq_q4 = -DBG_I_CLIP;

      m4_set_data.debug_VdVq.value = ((uint32_t)(uint16_t)(int16_t)vd_q5)
                                   | ((uint32_t)(uint16_t)(int16_t)vq_q5 << 16);
      m4_set_data.debug_IdIq.value = ((uint32_t)(uint16_t)(int16_t)id_q4)
                                   | ((uint32_t)(uint16_t)(int16_t)iq_q4 << 16);
      m4_set_data.debug_VdVq.receive_flag = 1;
      m4_set_data.debug_IdIq.receive_flag = 1;
      /* (receive_flag write touches bit31 only; pack above used
       *  bit[15:0] and bit[30:16] — bit31 stays clear.)            */

      /* ---- Step 3: Calibratable LPF for power-estimation dq inputs ---- */
      Rte_FilterPwrEstDq(&Vd, &Vq, &Id, &Iq);

      /* ---- Step 4: Full dq electromagnetic power (filtered inputs) ---- */
      /* P = 3/2 * (Vd*Id + Vq*Iq)   [W]                               */
      float P_dq = 1.5f * (Vd * Id + Vq * Iq);

      /* ---- Step 5: Copper loss  P_cu ∝ I² ---- */
      float I_sq = Id * Id + Iq * Iq;                    /* A² */
      float P_cu = Cal_PwrEst_KCu_unitless_f32 * I_sq * 20.0f; /* W */

      /* ---- Step 6: Iron loss  P_fe ∝ ω² ---- */
      float speed_rpm = (float)motorspeedfeedbackq15
                      * ((float)IFX_MS_FOCSOLUTIONF16_BASE_MECH_SPEED_RPM / 32768.0f);
      float speed_pu = speed_rpm / (float)IFX_MS_FOCSOLUTIONF16_BASE_MECH_SPEED_RPM;
      float P_fe = Cal_PwrEst_KFe_unitless_f32 * speed_pu * speed_pu * 50000.0f; /* W */

      /* ---- Step 7: Inverter losses ---- */
      float I_rms = sqrtf(I_sq) * 0.7071f;               /* A_rms, 1/√2 */

      /* Bus voltage in volts (cached for bus-current calc) */
      g_vdc_volts = (float)bus_voltage * (1650.0f / 4096.0f);
      if (g_vdc_volts < 1.0f) g_vdc_volts = 400.0f;     /* clamp: avoid div-by-zero */

      /* The observation has already been sampled from the modulator's exact
       * Q15 input pair above.  Keep this IPC packing path unchanged. */
      float mod_ratio = g_rteVoltUtilSample.modRatio;
      float v_util_pct = g_rteVoltUtilSample.filteredPct;

      /* Keep the ratio and overmodulation indication raw; only bits[29:16]
       * (voltage utilization) are low-pass filtered. */
      uint32_t mq15  = (uint32_t)(mod_ratio  * 32768.0f + 0.5f);
      uint32_t upcts = (uint32_t)(v_util_pct * 100.0f + 0.5f);         /* 0.01%/LSB */
      if (mq15  > 0x7FFFu) mq15  = 0x7FFFu;
      if (upcts > 0x3FFFu) upcts = 0x3FFFu;                            /* 14-bit */
      uint32_t ovmod = FocDemoClosedLoop.modulator.p_status.overmodulationFlag ? 1u : 0u;
      m4_set_data.debug_mod.value = (mq15 & 0xFFFFu)
                                  | ((upcts & 0x3FFFu) << 16)
                                  | (ovmod << 30);
      m4_set_data.debug_mod.receive_flag = 1;

      float Vdc_pu = g_vdc_volts / 1000.0f;              /* per-unit Vdc */
      float I_pu  = I_rms / 50.0f;                       /* per-unit current */

      /* P_inv_cond ∝ I  (IGBT/MOSFET conduction drop)                 */
      float P_inv_cond = Cal_PwrEst_KInvCond_unitless_f32 * I_pu * 50000.0f; /* W */

      /* P_inv_sw ∝ I × Vdc  (E_on + E_off scale with bus voltage)    */
      float P_inv_sw = Cal_PwrEst_KInvSw_unitless_f32 * I_pu * Vdc_pu * 50000.0f; /* W */

      /* ---- Step 8: Total estimated DC input power ---- */
      float P_total = P_dq + P_cu + P_fe + P_inv_cond + P_inv_sw;

      Meas_PwrEst_Pdq_W_f32 = P_dq;
      Meas_PwrEst_Pcu_W_f32 = P_cu;
      Meas_PwrEst_Pfe_W_f32 = P_fe;
      Meas_PwrEst_PinvCond_W_f32 = P_inv_cond;
      Meas_PwrEst_PinvSw_W_f32 = P_inv_sw;
      Meas_PwrEst_Ptotal_W_f32 = P_total;

      /* ---- Step 9: Calibratable first-order output-power LPF ---- */
      float filteredPowerW = Rte_FilterPwrEstPower(P_total);
      Meas_PwrEst_Pout_W_f32 = filteredPowerW;

      /* ---- Step 10: Direct output; no non-negative power clamp ---- */
      int32_t outputPowerW = (filteredPowerW >= 0.0f)
                           ? (int32_t)(filteredPowerW + 0.5f)
                           : (int32_t)(filteredPowerW - 0.5f);
      m4_set_data.motor_power.value = (uint16_t)outputPowerW;
  }

  /* ---- Bus current back-calculation ---- */
  /* I_bus = P_est / Vdc, scaled to 0.1A units                          */
  if (g_vdc_volts > 1.0f)
  {
      float I_bus = (float)m4_set_data.motor_power.value / g_vdc_volts;
      m4_set_data.bus_current.value = (uint16_t)(I_bus * 10.0f + 0.5f);
  }
  else
  {
      m4_set_data.bus_current.value = 0u;
  }

  m4_set_data.motor_speed.value = motorspeedfeedbackq15;
  motor_running_status = (uint16_t)FocDemoClosedLoop.p_status.state;
  motor_err_status = (uint32_t)FocDemoClosedLoop.p_m0FaultStatus;
      if (IPMFAULT_STATE == 1u) { motor_err_status |= 0x00100000u; }
  m4_set_data.err_status.value = motor_err_status;
  m4_set_data.ipm_fault_state.value = IPMFAULT_STATE;
  m4_set_data.running_status.value = motor_running_status;
  m4_set_data.phase_current_u.value = FocDemoClosedLoop.currentsUVW.u*50/32768;
  m4_set_data.phase_current_v.value = FocDemoClosedLoop.currentsUVW.v*50/32768;
  m4_set_data.phase_current_w.value = FocDemoClosedLoop.currentsUVW.w*50/32768;
//  m4_set_data.g_ad_vbat_sense.value = g_ad_vbat_sense;
//  expect_speed = m4_get_data.expect_speed;
//  power_status = m4_get_data.power_status;

  if(m4_get_data.acctime.receive_flag)
  {
    m4_get_data.acctime.receive_flag = 0;
    acctime = m4_get_data.acctime.value;
  }
  if(m4_get_data.redtime.receive_flag)
  {
    m4_get_data.redtime.receive_flag = 0;
    redtime = m4_get_data.redtime.value;
  }
  if(m4_get_data.err_clr.receive_flag)
  {
    m4_get_data.err_clr.receive_flag = 0;
    motor_err_Clear = m4_get_data.err_clr.value;
  }

  if(m4_get_data.motor_control.receive_flag)
  {
    m4_get_data.motor_control.receive_flag = 0;
    motor_enable_command = m4_get_data.motor_control.enable_command;
    motorspeedreferenceq10 = m4_get_data.motor_control.speed;
    motor_power_limiter = m4_get_data.motor_control.power_limiter;

  }
  Ipc_Pipe_Set();
}


static void Rte_Swc_To_Bsw(void)
{


}
