/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: fado_external_observer_wrapper.c
 *
 * Code generated for Simulink model 'fado_external_observer_wrapper'.
 *
 * Model version                  : 1.4
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Mon Aug  3 16:44:39 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "fado_external_observer_wrapper.h"
#include "rtwtypes.h"
#include <string.h>
#include "rt_nonfinite.h"
#include <math.h>
#include "fado_external_observer_wrapper_private.h"
#include "rt_defines.h"

/* Block states (default storage) */
DW_fado_external_observer_wra_T fado_external_observer_wrapp_DW;

/* External inputs (root inport signals with default storage) */
ExtU_fado_external_observer_w_T fado_external_observer_wrappe_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_fado_external_observer_w_T fado_external_observer_wrappe_Y;

/* Real-time model */
static RT_MODEL_fado_external_observ_T fado_external_observer_wrapp_M_;
RT_MODEL_fado_external_observ_T *const fado_external_observer_wrapp_M =
  &fado_external_observer_wrapp_M_;

/* Forward declaration for local functions */
static real32_T fado_external_observer_wrap_mod(real32_T x);
real32_T rt_atan2f_snf(real32_T u0, real32_T u1)
{
  real32_T y;
  if (rtIsNaNF(u0) || rtIsNaNF(u1)) {
    y = (rtNaNF);
  } else if (rtIsInfF(u0) && rtIsInfF(u1)) {
    int32_T tmp;
    int32_T tmp_0;
    if (u0 > 0.0F) {
      tmp = 1;
    } else {
      tmp = -1;
    }

    if (u1 > 0.0F) {
      tmp_0 = 1;
    } else {
      tmp_0 = -1;
    }

    y = atan2f((real32_T)tmp, (real32_T)tmp_0);
  } else if (u1 == 0.0F) {
    if (u0 > 0.0F) {
      y = RT_PIF / 2.0F;
    } else if (u0 < 0.0F) {
      y = -(RT_PIF / 2.0F);
    } else {
      y = 0.0F;
    }
  } else {
    y = atan2f(u0, u1);
  }

  return y;
}

/* Function for MATLAB Function: '<Root>/FadoDiscreteUpdate' */
static real32_T fado_external_observer_wrap_mod(real32_T x)
{
  real32_T r;
  if (rtIsNaNF(x)) {
    r = (rtNaNF);
  } else if (rtIsInfF(x)) {
    r = (rtNaNF);
  } else {
    /* T2S angles are normalized every sample, so the normal path is at most
     * one turn outside [0, 2*pi). Keep fmodf only as an abnormal fallback. */
    r = x;
    if (r >= 6.2831855F) {
      r -= 6.2831855F;
    } else if (r < 0.0F) {
      r += 6.2831855F;
    }

    if ((r >= 6.2831855F) || (r < 0.0F)) {
      r = fmodf(r, 6.2831855F);
      if (r < 0.0F) {
        r += 6.2831855F;
      }
    }
  }

  return r;
}

/* Model step function */
void fado_external_observer_wrapper_step(void)
{
  int32_T i;
  real32_T rtb_fadoState_z[10];
  real32_T fadoDhatAlpha_Wb;
  real32_T fadoFastNaturalFrequency_radps;
  real32_T fadoFastPhaseError_rad;
  real32_T fadoLambdaAlpha1_Wb;
  real32_T fadoLambdaAlphaDot_V;
  real32_T fadoOmegaFast_radps;
  real32_T fadoOmegaSlow_radps;
  real32_T fadoRotorFluxAlphaLimited_Wb;
  real32_T fadoRotorFluxAlpha_Wb;
  real32_T fadoRotorFluxBetaLimited_Wb;
  real32_T fadoRotorFluxBeta_Wb;
  real32_T fadoRotorFluxBeta_Wb_tmp;
  real32_T fadoRotorFluxMag_Wb;
  real32_T fadoThetaDirect_rad;
  boolean_T b[10];
  boolean_T c[10];
  boolean_T exitg1;
  boolean_T fadoFluxLimited;
  boolean_T fadoKafMode;
  boolean_T y;

  /* MATLAB Function: '<Root>/FadoDiscreteUpdate' incorporates:
   *  Inport: '<Root>/fadoVoltageAlpha_V'
   *  Inport: '<Root>/fadoVoltageBeta_V'
   */
  fadoLambdaAlphaDot_V = fado_external_observer_wrappe_U.fadoVoltageAlpha_V;
  fadoLambdaAlpha1_Wb = fado_external_observer_wrappe_U.fadoVoltageBeta_V;
  for (i = 0; i < 10; i++) {
    /* UnitDelay: '<Root>/fadoState_z' */
    rtb_fadoState_z[i] = fado_external_observer_wrapp_DW.fadoState_z[i];

    /* MATLAB Function: '<Root>/FadoDiscreteUpdate' incorporates:
     *  UnitDelay: '<Root>/fadoState_z'
     */
    fado_external_observer_wrapp_DW.fadoState_z[i] = 0.0F;
  }

  /* Outport: '<Root>/fadoOutput' incorporates:
   *  MATLAB Function: '<Root>/FadoDiscreteUpdate'
   */
  memset(&fado_external_observer_wrappe_Y.fadoOutput[0], 0, 18U * sizeof
         (real32_T));

  /* MATLAB Function: '<Root>/FadoDiscreteUpdate' incorporates:
   *  Inport: '<Root>/fadoCurrentAlpha_A'
   *  Inport: '<Root>/fadoCurrentBeta_A'
   *  Inport: '<Root>/fadoEnable'
   *  Inport: '<Root>/fadoFastT2SBandwidth_Hz'
   *  Inport: '<Root>/fadoFluxLimit_Wb'
   *  Inport: '<Root>/fadoKaf_radps'
   *  Inport: '<Root>/fadoKdf_per_s'
   *  Inport: '<Root>/fadoLowSpeedThreshold_Hz'
   *  Inport: '<Root>/fadoPolePairs'
   *  Inport: '<Root>/fadoQuadratureInductance_H'
   *  Inport: '<Root>/fadoSlowT2SBandwidth_Hz'
   *  Inport: '<Root>/fadoStatorResistance_Ohm'
   *  Inport: '<Root>/fadoT2SDamping'
   *  Inport: '<Root>/fadoVoltageAlphaOffset_V'
   *  Inport: '<Root>/fadoVoltageAlpha_V'
   *  Inport: '<Root>/fadoVoltageBetaOffset_V'
   *  Inport: '<Root>/fadoVoltageBeta_V'
   *  Inport: '<Root>/fadoVoltagePreprocessEnable'
   *  Outport: '<Root>/fadoOutput'
   *  UnitDelay: '<Root>/fadoState_z'
   */
  if (fado_external_observer_wrappe_U.fadoEnable) {
    fado_external_observer_wrappe_Y.fadoOutput[17] = 1.0F;
    if (rtIsInfF(fado_external_observer_wrappe_U.fadoStatorResistance_Ohm) ||
        rtIsNaNF(fado_external_observer_wrappe_U.fadoStatorResistance_Ohm) ||
        (rtIsInfF(fado_external_observer_wrappe_U.fadoQuadratureInductance_H) ||
         rtIsNaNF(fado_external_observer_wrappe_U.fadoQuadratureInductance_H) ||
         (rtIsInfF(fado_external_observer_wrappe_U.fadoFluxLimit_Wb) || rtIsNaNF
          (fado_external_observer_wrappe_U.fadoFluxLimit_Wb) || (rtIsInfF
           (fado_external_observer_wrappe_U.fadoKdf_per_s) || rtIsNaNF
           (fado_external_observer_wrappe_U.fadoKdf_per_s) || (rtIsInfF
            (fado_external_observer_wrappe_U.fadoLowSpeedThreshold_Hz) ||
            rtIsNaNF(fado_external_observer_wrappe_U.fadoLowSpeedThreshold_Hz) ||
            (rtIsInfF(fado_external_observer_wrappe_U.fadoKaf_radps) || rtIsNaNF
             (fado_external_observer_wrappe_U.fadoKaf_radps) || (rtIsInfF
              (fado_external_observer_wrappe_U.fadoFastT2SBandwidth_Hz) ||
              rtIsNaNF(fado_external_observer_wrappe_U.fadoFastT2SBandwidth_Hz) ||
              (rtIsInfF(fado_external_observer_wrappe_U.fadoSlowT2SBandwidth_Hz)
               || rtIsNaNF
               (fado_external_observer_wrappe_U.fadoSlowT2SBandwidth_Hz) ||
               (rtIsInfF(fado_external_observer_wrappe_U.fadoT2SDamping) ||
                rtIsNaNF(fado_external_observer_wrappe_U.fadoT2SDamping) ||
                (rtIsInfF
                 (fado_external_observer_wrappe_U.fadoVoltageAlphaOffset_V) ||
                 rtIsNaNF
                 (fado_external_observer_wrappe_U.fadoVoltageAlphaOffset_V) ||
                 (rtIsInfF
                  (fado_external_observer_wrappe_U.fadoVoltageBetaOffset_V) ||
                  rtIsNaNF
                  (fado_external_observer_wrappe_U.fadoVoltageBetaOffset_V) || (
        !(fado_external_observer_wrappe_U.fadoStatorResistance_Ohm > 0.0F) ||
        !(fado_external_observer_wrappe_U.fadoQuadratureInductance_H > 0.0F) ||
        (fado_external_observer_wrappe_U.fadoPolePairs <= 0) ||
        !(fado_external_observer_wrappe_U.fadoFluxLimit_Wb > 0.0F) ||
        !(fado_external_observer_wrappe_U.fadoKdf_per_s >= 0.0F) ||
        !(fado_external_observer_wrappe_U.fadoLowSpeedThreshold_Hz >= 0.0F) ||
        !(fado_external_observer_wrappe_U.fadoKaf_radps > 0.0F) ||
        !(fado_external_observer_wrappe_U.fadoFastT2SBandwidth_Hz > 0.0F) ||
        !(fado_external_observer_wrappe_U.fadoSlowT2SBandwidth_Hz > 0.0F) ||
        !(fado_external_observer_wrappe_U.fadoT2SDamping > 0.0F))))))))))))) {
      fado_external_observer_wrappe_Y.fadoOutput[16] = 3.0F;
    } else {
      if (fado_external_observer_wrappe_U.fadoVoltagePreprocessEnable) {
        fadoLambdaAlphaDot_V =
          fado_external_observer_wrappe_U.fadoVoltageAlpha_V +
          fado_external_observer_wrappe_U.fadoVoltageAlphaOffset_V;
        fadoLambdaAlpha1_Wb = fado_external_observer_wrappe_U.fadoVoltageBeta_V
          + fado_external_observer_wrappe_U.fadoVoltageBetaOffset_V;
      }

      fadoDhatAlpha_Wb = fabsf(rtb_fadoState_z[8]);
      fadoKafMode = (fadoDhatAlpha_Wb <= 6.2831855F *
                     fado_external_observer_wrappe_U.fadoLowSpeedThreshold_Hz);
      fadoFluxLimited = false;
      fadoOmegaFast_radps =
        fado_external_observer_wrappe_U.fadoQuadratureInductance_H *
        fado_external_observer_wrappe_U.fadoCurrentAlpha_A;
      fadoRotorFluxAlpha_Wb = (rtb_fadoState_z[0] - rtb_fadoState_z[4]) -
        fadoOmegaFast_radps;
      fadoRotorFluxBeta_Wb_tmp =
        fado_external_observer_wrappe_U.fadoQuadratureInductance_H *
        fado_external_observer_wrappe_U.fadoCurrentBeta_A;
      fadoRotorFluxBeta_Wb = (rtb_fadoState_z[1] - rtb_fadoState_z[5]) -
        fadoRotorFluxBeta_Wb_tmp;
      fadoRotorFluxMag_Wb = sqrtf(fadoRotorFluxAlpha_Wb * fadoRotorFluxAlpha_Wb
        + fadoRotorFluxBeta_Wb * fadoRotorFluxBeta_Wb);
      if (fadoKafMode) {
        fadoRotorFluxAlphaLimited_Wb = fadoRotorFluxAlpha_Wb;
        fadoRotorFluxBetaLimited_Wb = fadoRotorFluxBeta_Wb;
        if (fadoRotorFluxMag_Wb >
            fado_external_observer_wrappe_U.fadoFluxLimit_Wb) {
          fadoRotorFluxBetaLimited_Wb =
            fado_external_observer_wrappe_U.fadoFluxLimit_Wb /
            fadoRotorFluxMag_Wb;
          fadoRotorFluxAlphaLimited_Wb = fadoRotorFluxAlpha_Wb *
            fadoRotorFluxBetaLimited_Wb;
          fadoRotorFluxBetaLimited_Wb *= fadoRotorFluxBeta_Wb;
          fadoFluxLimited = true;
        }

        fadoLambdaAlphaDot_V = (fadoLambdaAlphaDot_V -
          fado_external_observer_wrappe_U.fadoStatorResistance_Ohm *
          fado_external_observer_wrappe_U.fadoCurrentAlpha_A) -
          (fadoRotorFluxAlpha_Wb - fadoRotorFluxAlphaLimited_Wb) *
          fado_external_observer_wrappe_U.fadoKaf_radps;
        fadoRotorFluxAlpha_Wb = (fadoLambdaAlpha1_Wb -
          fado_external_observer_wrappe_U.fadoStatorResistance_Ohm *
          fado_external_observer_wrappe_U.fadoCurrentBeta_A) -
          (fadoRotorFluxBeta_Wb - fadoRotorFluxBetaLimited_Wb) *
          fado_external_observer_wrappe_U.fadoKaf_radps;
        fadoRotorFluxBeta_Wb = 0.0F;
      } else {
        fadoRotorFluxBeta_Wb = fado_external_observer_wrappe_U.fadoKdf_per_s;
        fadoLambdaAlphaDot_V = (fadoLambdaAlphaDot_V -
          fado_external_observer_wrappe_U.fadoStatorResistance_Ohm *
          fado_external_observer_wrappe_U.fadoCurrentAlpha_A) -
          fado_external_observer_wrappe_U.fadoKdf_per_s * rtb_fadoState_z[4];
        fadoRotorFluxAlpha_Wb = (fadoLambdaAlpha1_Wb -
          fado_external_observer_wrappe_U.fadoStatorResistance_Ohm *
          fado_external_observer_wrappe_U.fadoCurrentBeta_A) -
          fado_external_observer_wrappe_U.fadoKdf_per_s * rtb_fadoState_z[5];
      }

      fadoLambdaAlpha1_Wb = 5.0E-5F * fadoLambdaAlphaDot_V + rtb_fadoState_z[0];
      fadoLambdaAlphaDot_V = 5.0E-5F * fadoRotorFluxAlpha_Wb + rtb_fadoState_z[1];
      fadoRotorFluxAlphaLimited_Wb = fadoLambdaAlpha1_Wb - rtb_fadoState_z[2];
      fadoRotorFluxAlpha_Wb = fadoLambdaAlphaDot_V - rtb_fadoState_z[3];
      fadoDhatAlpha_Wb *= 2.0F;
      fado_external_observer_wrapp_DW.fadoState_z[2] = (((-rtb_fadoState_z[8] *
        rtb_fadoState_z[3] + rtb_fadoState_z[5] * rtb_fadoState_z[8]) +
        fadoDhatAlpha_Wb * fadoRotorFluxAlphaLimited_Wb) - rtb_fadoState_z[8] *
        fadoRotorFluxAlpha_Wb) * 5.0E-5F + rtb_fadoState_z[2];
      fadoRotorFluxAlphaLimited_Wb *= rtb_fadoState_z[8];
      fado_external_observer_wrapp_DW.fadoState_z[3] = (((rtb_fadoState_z[2] *
        rtb_fadoState_z[8] - rtb_fadoState_z[4] * rtb_fadoState_z[8]) +
        fadoRotorFluxAlphaLimited_Wb) + fadoDhatAlpha_Wb * fadoRotorFluxAlpha_Wb)
        * 5.0E-5F + rtb_fadoState_z[3];
      fadoDhatAlpha_Wb = -rtb_fadoState_z[8] * fadoRotorFluxAlpha_Wb * 5.0E-5F +
        rtb_fadoState_z[4];
      fadoRotorFluxAlpha_Wb = fadoRotorFluxAlphaLimited_Wb * 5.0E-5F +
        rtb_fadoState_z[5];
      fadoRotorFluxAlphaLimited_Wb = fadoLambdaAlpha1_Wb - fadoDhatAlpha_Wb;
      fadoRotorFluxBetaLimited_Wb = fadoLambdaAlphaDot_V - fadoRotorFluxAlpha_Wb;
      fadoThetaDirect_rad = rt_atan2f_snf(fadoRotorFluxBetaLimited_Wb -
        fadoRotorFluxBeta_Wb_tmp, fadoRotorFluxAlphaLimited_Wb -
        fadoOmegaFast_radps);
      fadoFastNaturalFrequency_radps = 6.2831855F *
        fado_external_observer_wrappe_U.fadoFastT2SBandwidth_Hz;
      fadoFastPhaseError_rad = fado_external_observer_wrap_mod
        ((fadoThetaDirect_rad - rtb_fadoState_z[6]) + 3.1415927F) - 3.1415927F;
      fadoOmegaFast_radps = 5.0E-5F * fadoFastNaturalFrequency_radps *
        fadoFastNaturalFrequency_radps * fadoFastPhaseError_rad +
        rtb_fadoState_z[8];
      fadoRotorFluxBeta_Wb_tmp = 2.0F *
        fado_external_observer_wrappe_U.fadoT2SDamping;
      fadoFastNaturalFrequency_radps = fado_external_observer_wrap_mod
        (((fadoRotorFluxBeta_Wb_tmp * fadoFastNaturalFrequency_radps *
           fadoFastPhaseError_rad + fadoOmegaFast_radps) * 5.0E-5F +
          rtb_fadoState_z[6]) + 3.1415927F) - 3.1415927F;
      fadoFastPhaseError_rad = 6.2831855F *
        fado_external_observer_wrappe_U.fadoSlowT2SBandwidth_Hz;
      fadoThetaDirect_rad = fado_external_observer_wrap_mod((fadoThetaDirect_rad
        - rtb_fadoState_z[7]) + 3.1415927F) - 3.1415927F;
      fadoOmegaSlow_radps = 5.0E-5F * fadoFastPhaseError_rad *
        fadoFastPhaseError_rad * fadoThetaDirect_rad + rtb_fadoState_z[9];
      fado_external_observer_wrapp_DW.fadoState_z[7] =
        fado_external_observer_wrap_mod(((fadoRotorFluxBeta_Wb_tmp *
        fadoFastPhaseError_rad * fadoThetaDirect_rad + fadoOmegaSlow_radps) *
        5.0E-5F + rtb_fadoState_z[7]) + 3.1415927F) - 3.1415927F;
      fado_external_observer_wrapp_DW.fadoState_z[0] = fadoLambdaAlpha1_Wb;
      fado_external_observer_wrapp_DW.fadoState_z[1] = fadoLambdaAlphaDot_V;
      fado_external_observer_wrapp_DW.fadoState_z[4] = fadoDhatAlpha_Wb;
      fado_external_observer_wrapp_DW.fadoState_z[5] = fadoRotorFluxAlpha_Wb;
      fado_external_observer_wrapp_DW.fadoState_z[6] =
        fadoFastNaturalFrequency_radps;
      fado_external_observer_wrapp_DW.fadoState_z[8] = fadoOmegaFast_radps;
      fado_external_observer_wrapp_DW.fadoState_z[9] = fadoOmegaSlow_radps;
      for (i = 0; i < 10; i++) {
        b[i] = !rtIsInfF(fado_external_observer_wrapp_DW.fadoState_z[i]);
        c[i] = !rtIsNaNF(fado_external_observer_wrapp_DW.fadoState_z[i]);
      }

      y = true;
      i = 0;
      exitg1 = false;
      while (!exitg1 && (i < 10)) {
        if (!b[i] || !c[i]) {
          y = false;
          exitg1 = true;
        } else {
          i++;
        }
      }

      if (!y) {
        for (i = 0; i < 10; i++) {
          fado_external_observer_wrapp_DW.fadoState_z[i] = 0.0F;
        }

        fado_external_observer_wrappe_Y.fadoOutput[16] = 2.0F;
      } else {
        fado_external_observer_wrappe_Y.fadoOutput[0] =
          fado_external_observer_wrap_mod(fadoFastNaturalFrequency_radps);
        fado_external_observer_wrappe_Y.fadoOutput[1] = fadoOmegaFast_radps;
        fado_external_observer_wrappe_Y.fadoOutput[2] = fadoOmegaSlow_radps;
        fado_external_observer_wrappe_Y.fadoOutput[3] = fadoOmegaSlow_radps /
          (6.2831855F * (real32_T)fado_external_observer_wrappe_U.fadoPolePairs)
          * 60.0F;
        fado_external_observer_wrappe_Y.fadoOutput[4] = fadoLambdaAlpha1_Wb;
        fado_external_observer_wrappe_Y.fadoOutput[5] = fadoLambdaAlphaDot_V;
        fado_external_observer_wrappe_Y.fadoOutput[6] =
          fadoRotorFluxAlphaLimited_Wb;
        fado_external_observer_wrappe_Y.fadoOutput[7] =
          fadoRotorFluxBetaLimited_Wb;
        fado_external_observer_wrappe_Y.fadoOutput[8] = fadoDhatAlpha_Wb;
        fado_external_observer_wrappe_Y.fadoOutput[9] = fadoRotorFluxAlpha_Wb;
        fado_external_observer_wrappe_Y.fadoOutput[10] = fadoRotorFluxMag_Wb;
        fado_external_observer_wrappe_Y.fadoOutput[11] = fadoRotorFluxBeta_Wb;
        fado_external_observer_wrappe_Y.fadoOutput[12] = (real32_T)!fadoKafMode;
        fado_external_observer_wrappe_Y.fadoOutput[13] = fadoKafMode;
        fado_external_observer_wrappe_Y.fadoOutput[14] = fadoFluxLimited;
        fado_external_observer_wrappe_Y.fadoOutput[15] = 1.0F;
        fado_external_observer_wrappe_Y.fadoOutput[16] = 1.0F;
      }
    }
  }
}

/* Model initialize function */
void fado_external_observer_wrapper_initialize(void)
{
  /* (no initialization code required) */
}

/* Model terminate function */
void fado_external_observer_wrapper_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
