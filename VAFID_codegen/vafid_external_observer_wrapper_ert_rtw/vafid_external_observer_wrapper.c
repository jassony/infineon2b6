/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: vafid_external_observer_wrapper.c
 *
 * Code generated for Simulink model 'vafid_external_observer_wrapper'.
 *
 * Model version                  : 1.2
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Fri Aug 28 00:13:18 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "vafid_external_observer_wrapper.h"
#include "rtwtypes.h"
#include "vafid_external_observer_wrapper_types.h"
#include "vafid_external_observer_wrapper_private.h"
#include "rt_nonfinite.h"
#include <math.h>
#include <string.h>
#include "zero_crossing_types.h"

/* Block states (default storage) */
DW_vafid_external_observer_wr_T vafid_external_observer_wrap_DW;

/* Previous zero-crossings (trigger) states */
PrevZCX_vafid_external_observ_T vafid_external_observer_PrevZCX;

/* External inputs (root inport signals with default storage) */
ExtU_vafid_external_observer__T vafid_external_observer_wrapp_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_vafid_external_observer__T vafid_external_observer_wrapp_Y;

/* Real-time model */
static RT_MODEL_vafid_external_obser_T vafid_external_observer_wrap_M_;
RT_MODEL_vafid_external_obser_T *const vafid_external_observer_wrap_M =
  &vafid_external_observer_wrap_M_;

/* Forward declaration for local functions */
static boolean_T vafid_localConfigurationIsValid(const
  sXUMNpzAt0y7k62cD7EcU0C_vafid_T *config);
static real32_T vafid_external_observer_wra_mod(real32_T x);
static void vafid_external_observer_wra_sum(const real32_T x[9], real32_T y[3]);
static real32_T vafid_external_observer_maximum(const real32_T x[3]);
static void vafid_exte_localSolveParameters(const creal32_T voltageD[2], const
  creal32_T voltageQ[2], const creal32_T currentD[2], const creal32_T currentQ[2],
  real32_T meanElectricalOmega_radps, real32_T config_probeD_Frequency_Hz,
  real32_T config_probeQ_Frequency_Hz, real32_T candidate[3], boolean_T *solved,
  real32_T *conditionNumber, real32_T *relativeResidual);
static void v_vafid_discrete_reference_step(uint8_T mode, boolean_T resetRequest,
  boolean_T sampleValid, real32_T voltageAlpha_V, real32_T voltageBeta_V,
  real32_T currentAlpha_A, real32_T currentBeta_A, real32_T
  kreElectricalAngle_rad, real32_T kreElectricalOmega_radps, real32_T
  kreActiveFlux_Wb, const sXUMNpzAt0y7k62cD7EcU0C_vafid_T *config, real32_T
  *probeD_PU, real32_T *probeQ_PU, real32_T *rs_Ohm, real32_T *ld_H, real32_T
  *lq_H, real32_T *fluxPM_Wb, boolean_T *estimateValid, boolean_T *freshEstimate,
  boolean_T *staleEstimate, uint8_T *status, real32_T *conditionNumber, real32_T
  *relativeResidual, uint16_T *consecutiveAcceptedWindows, uint32_T
  *windowSampleCount);
real32_T rt_roundf_snf(real32_T u)
{
  real32_T y;
  if (fabsf(u) < 8.388608E+6F) {
    if (u >= 0.5F) {
      y = floorf(u + 0.5F);
    } else if (u > -0.5F) {
      y = u * 0.0F;
    } else {
      y = ceilf(u - 0.5F);
    }
  } else {
    y = u;
  }

  return y;
}

/* Function for MATLAB Function: '<S1>/VafidDiscreteStep' */
static boolean_T vafid_localConfigurationIsValid(const
  sXUMNpzAt0y7k62cD7EcU0C_vafid_T *config)
{
  return !rtIsInfF(config->sampleTime_s) && !rtIsNaNF(config->sampleTime_s) &&
    ((config->sampleTime_s > 0.0F) && (!rtIsInfF(config->probeD_Frequency_Hz) &&
      !rtIsNaNF(config->probeD_Frequency_Hz) && ((config->probeD_Frequency_Hz >
        0.0F) && (!rtIsInfF(config->probeQ_Frequency_Hz) && !rtIsNaNF
                  (config->probeQ_Frequency_Hz) && ((config->probeQ_Frequency_Hz
          > 0.0F) && (config->probeD_Frequency_Hz != config->probeQ_Frequency_Hz)
         && (!rtIsInfF(config->probeD_Amplitude_PU) && !rtIsNaNF
             (config->probeD_Amplitude_PU) && (!rtIsInfF
           (config->probeQ_Amplitude_PU) && !rtIsNaNF
           (config->probeQ_Amplitude_PU) && (!rtIsInfF
            (config->probeD_MaxAmplitude_PU) && !rtIsNaNF
            (config->probeD_MaxAmplitude_PU) && ((config->probeD_MaxAmplitude_PU
              > 0.0F) && (!rtIsInfF(config->probeQ_MaxAmplitude_PU) && !rtIsNaNF
              (config->probeQ_MaxAmplitude_PU) &&
              ((config->probeQ_MaxAmplitude_PU > 0.0F) &&
               (config->windowLength_samples > 0U) &&
               (config->requiredAcceptedWindows > 0) &&
               (config->staleRejectedWindows > 0) && (!rtIsInfF
    (config->parameterFusion) && !rtIsNaNF(config->parameterFusion) &&
    ((config->parameterFusion > 0.0F) && (config->parameterFusion <= 1.0F) &&
     (!rtIsInfF(config->fluxFusion) && !rtIsNaNF(config->fluxFusion) &&
      ((config->fluxFusion > 0.0F) && (config->fluxFusion <= 1.0F) && (!rtIsInfF
    (config->conditionLimit) && !rtIsNaNF(config->conditionLimit) &&
    ((config->conditionLimit > 1.0F) && (!rtIsInfF(config->relativeResidualLimit)
    && !rtIsNaNF(config->relativeResidualLimit) &&
    ((config->relativeResidualLimit > 0.0F) && (config->minimumRs_Ohm > 0.0F) &&
     (config->maximumRs_Ohm > config->minimumRs_Ohm) && (config->minimumLd_H >
    0.0F) && (config->maximumLd_H > config->minimumLd_H) && (config->minimumLq_H
    > 0.0F) && (config->maximumLq_H > config->minimumLq_H) &&
     (config->minimumFluxPM_Wb > 0.0F) && (config->maximumFluxPM_Wb >
    config->minimumFluxPM_Wb))))))))))))))))))));
}

/* Function for MATLAB Function: '<S1>/VafidDiscreteStep' */
static real32_T vafid_external_observer_wra_mod(real32_T x)
{
  real32_T r;
  if (rtIsNaNF(x)) {
    r = (rtNaNF);
  } else if (rtIsInfF(x)) {
    r = (rtNaNF);
  } else {
    real32_T q;
    q = fabsf(x / 6.2831855F);
    if (fabsf(q - floorf(q + 0.5F)) > 1.1920929E-7F * q) {
      r = fmodf(x, 6.2831855F);
    } else {
      r = 0.0F;
    }

    if (r == 0.0F) {
      r = 0.0F;
    } else if (r < 0.0F) {
      r += 6.2831855F;
    }
  }

  return r;
}

/* Function for MATLAB Function: '<S1>/VafidDiscreteStep' */
static void vafid_external_observer_wra_sum(const real32_T x[9], real32_T y[3])
{
  int32_T xi;
  for (xi = 0; xi < 3; xi++) {
    int32_T xpageoffset;
    xpageoffset = xi * 3;
    y[xi] = (x[xpageoffset + 1] + x[xpageoffset]) + x[xpageoffset + 2];
  }
}

/* Function for MATLAB Function: '<S1>/VafidDiscreteStep' */
static real32_T vafid_external_observer_maximum(const real32_T x[3])
{
  int32_T idx;
  int32_T k;
  real32_T ex;
  if (!rtIsNaNF(x[0])) {
    idx = 1;
  } else {
    boolean_T exitg1;
    idx = 0;
    k = 2;
    exitg1 = false;
    while (!exitg1 && (k < 4)) {
      if (!rtIsNaNF(x[k - 1])) {
        idx = k;
        exitg1 = true;
      } else {
        k++;
      }
    }
  }

  if (idx == 0) {
    ex = x[0];
  } else {
    ex = x[idx - 1];
    for (k = idx + 1; k < 4; k++) {
      real32_T x_0;
      x_0 = x[k - 1];
      if (ex < x_0) {
        ex = x_0;
      }
    }
  }

  return ex;
}

/* Function for MATLAB Function: '<S1>/VafidDiscreteStep' */
static void vafid_exte_localSolveParameters(const creal32_T voltageD[2], const
  creal32_T voltageQ[2], const creal32_T currentD[2], const creal32_T currentQ[2],
  real32_T meanElectricalOmega_radps, real32_T config_probeD_Frequency_Hz,
  real32_T config_probeQ_Frequency_Hz, real32_T candidate[3], boolean_T *solved,
  real32_T *conditionNumber, real32_T *relativeResidual)
{
  int32_T b_k;
  int32_T columnIndex;
  int32_T k;
  real32_T normalMatrix_tmp[24];
  real32_T regression[24];
  real32_T scaledRegression[24];
  real32_T b_y[9];
  real32_T inverseNormalMatrix[9];
  real32_T normalMatrix[9];
  real32_T observation[8];
  real32_T residual[8];
  real32_T columnScale[3];
  real32_T tmp[3];
  real32_T tmp_0[3];
  real32_T c13;
  real32_T determinant;
  real32_T probeOmega_radps_idx_0;
  real32_T probeOmega_radps_idx_1;
  boolean_T c[3];
  boolean_T d[3];
  boolean_T exitg1;
  boolean_T guard1;
  boolean_T y;
  candidate[0] = 0.0F;
  candidate[1] = 0.0F;
  candidate[2] = 0.0F;
  probeOmega_radps_idx_0 = 6.2831855F * config_probeD_Frequency_Hz;
  probeOmega_radps_idx_1 = 6.2831855F * config_probeQ_Frequency_Hz;
  regression[0] = currentD[0].re;
  regression[8] = -probeOmega_radps_idx_0 * currentD[0].im;
  regression[16] = -meanElectricalOmega_radps * currentQ[0].re;
  regression[1] = currentD[0].im;
  regression[9] = probeOmega_radps_idx_0 * currentD[0].re;
  regression[17] = -meanElectricalOmega_radps * currentQ[0].im;
  regression[2] = currentQ[0].re;
  regression[10] = meanElectricalOmega_radps * currentD[0].re;
  regression[18] = -probeOmega_radps_idx_0 * currentQ[0].im;
  regression[3] = currentQ[0].im;
  regression[11] = meanElectricalOmega_radps * currentD[0].im;
  regression[19] = probeOmega_radps_idx_0 * currentQ[0].re;
  observation[0] = voltageD[0].re;
  observation[1] = voltageD[0].im;
  observation[2] = voltageQ[0].re;
  observation[3] = voltageQ[0].im;
  regression[4] = currentD[1].re;
  regression[12] = -probeOmega_radps_idx_1 * currentD[1].im;
  regression[20] = -meanElectricalOmega_radps * currentQ[1].re;
  regression[5] = currentD[1].im;
  regression[13] = probeOmega_radps_idx_1 * currentD[1].re;
  regression[21] = -meanElectricalOmega_radps * currentQ[1].im;
  regression[6] = currentQ[1].re;
  regression[14] = meanElectricalOmega_radps * currentD[1].re;
  regression[22] = -probeOmega_radps_idx_1 * currentQ[1].im;
  regression[7] = currentQ[1].im;
  regression[15] = meanElectricalOmega_radps * currentD[1].im;
  regression[23] = probeOmega_radps_idx_1 * currentQ[1].re;
  observation[4] = voltageD[1].re;
  observation[5] = voltageD[1].im;
  observation[6] = voltageQ[1].re;
  observation[7] = voltageQ[1].im;
  memset(&scaledRegression[0], 0, 24U * sizeof(real32_T));
  for (columnIndex = 0; columnIndex < 3; columnIndex++) {
    for (k = 0; k < 8; k++) {
      residual[k] = fabsf(regression[(columnIndex << 3) + k]);
    }

    if (!rtIsNaNF(residual[0])) {
      k = 1;
    } else {
      k = 0;
      b_k = 2;
      exitg1 = false;
      while (!exitg1 && (b_k < 9)) {
        if (!rtIsNaNF(residual[b_k - 1])) {
          k = b_k;
          exitg1 = true;
        } else {
          b_k++;
        }
      }
    }

    if (k == 0) {
      probeOmega_radps_idx_1 = residual[0];
      columnScale[columnIndex] = residual[0];
    } else {
      probeOmega_radps_idx_1 = residual[k - 1];
      for (b_k = k + 1; b_k < 9; b_k++) {
        probeOmega_radps_idx_0 = residual[b_k - 1];
        if (probeOmega_radps_idx_1 < probeOmega_radps_idx_0) {
          probeOmega_radps_idx_1 = probeOmega_radps_idx_0;
        }
      }

      columnScale[columnIndex] = probeOmega_radps_idx_1;
    }

    if (probeOmega_radps_idx_1 > 1.0E-12F) {
      for (k = 0; k < 8; k++) {
        b_k = (columnIndex << 3) + k;
        scaledRegression[b_k] = regression[b_k] / probeOmega_radps_idx_1;
      }
    }
  }

  for (k = 0; k < 8; k++) {
    normalMatrix_tmp[3 * k] = scaledRegression[k];
    normalMatrix_tmp[3 * k + 1] = scaledRegression[k + 8];
    normalMatrix_tmp[3 * k + 2] = scaledRegression[k + 16];
  }

  for (k = 0; k < 3; k++) {
    probeOmega_radps_idx_1 = 0.0F;
    c13 = 0.0F;
    determinant = 0.0F;
    for (columnIndex = 0; columnIndex < 8; columnIndex++) {
      probeOmega_radps_idx_0 = scaledRegression[(k << 3) + columnIndex];
      probeOmega_radps_idx_1 += normalMatrix_tmp[3 * columnIndex] *
        probeOmega_radps_idx_0;
      c13 += normalMatrix_tmp[3 * columnIndex + 1] * probeOmega_radps_idx_0;
      determinant += normalMatrix_tmp[3 * columnIndex + 2] *
        probeOmega_radps_idx_0;
    }

    normalMatrix[3 * k + 2] = determinant;
    normalMatrix[3 * k + 1] = c13;
    normalMatrix[3 * k] = probeOmega_radps_idx_1;
  }

  probeOmega_radps_idx_0 = normalMatrix[4] * normalMatrix[8] - normalMatrix[5] *
    normalMatrix[7];
  probeOmega_radps_idx_1 = normalMatrix[2] * normalMatrix[7] - normalMatrix[1] *
    normalMatrix[8];
  c13 = normalMatrix[1] * normalMatrix[5] - normalMatrix[2] * normalMatrix[4];
  determinant = (normalMatrix[0] * probeOmega_radps_idx_0 + normalMatrix[3] *
                 probeOmega_radps_idx_1) + normalMatrix[6] * c13;
  guard1 = false;
  if (!rtIsInfF(determinant) && !rtIsNaNF(determinant) && (fabsf(determinant) >
       1.0E-9F)) {
    inverseNormalMatrix[0] = probeOmega_radps_idx_0 / determinant;
    inverseNormalMatrix[3] = (normalMatrix[5] * normalMatrix[6] - normalMatrix[3]
      * normalMatrix[8]) / determinant;
    inverseNormalMatrix[6] = (normalMatrix[3] * normalMatrix[7] - normalMatrix[4]
      * normalMatrix[6]) / determinant;
    inverseNormalMatrix[1] = probeOmega_radps_idx_1 / determinant;
    inverseNormalMatrix[4] = (normalMatrix[0] * normalMatrix[8] - normalMatrix[2]
      * normalMatrix[6]) / determinant;
    inverseNormalMatrix[7] = (normalMatrix[1] * normalMatrix[6] - normalMatrix[0]
      * normalMatrix[7]) / determinant;
    inverseNormalMatrix[2] = c13 / determinant;
    inverseNormalMatrix[5] = (normalMatrix[2] * normalMatrix[3] - normalMatrix[0]
      * normalMatrix[5]) / determinant;
    inverseNormalMatrix[8] = (normalMatrix[0] * normalMatrix[4] - normalMatrix[1]
      * normalMatrix[3]) / determinant;
    y = false;
    columnIndex = 0;
    exitg1 = false;
    while (!exitg1 && (columnIndex < 3)) {
      if (columnScale[columnIndex] <= 1.0E-12F) {
        y = true;
        exitg1 = true;
      } else {
        columnIndex++;
      }
    }

    if (y) {
      guard1 = true;
    } else {
      for (columnIndex = 0; columnIndex < 9; columnIndex++) {
        b_y[columnIndex] = fabsf(normalMatrix[columnIndex]);
        normalMatrix[columnIndex] = fabsf(inverseNormalMatrix[columnIndex]);
      }

      vafid_external_observer_wra_sum(b_y, tmp);
      vafid_external_observer_wra_sum(normalMatrix, tmp_0);
      *conditionNumber = sqrtf(fmaxf(vafid_external_observer_maximum(tmp) *
        vafid_external_observer_maximum(tmp_0), 0.0F));
      probeOmega_radps_idx_1 = 0.0F;
      c13 = 0.0F;
      determinant = 0.0F;
      for (k = 0; k < 8; k++) {
        probeOmega_radps_idx_0 = observation[k];
        probeOmega_radps_idx_1 += normalMatrix_tmp[3 * k] *
          probeOmega_radps_idx_0;
        c13 += normalMatrix_tmp[3 * k + 1] * probeOmega_radps_idx_0;
        determinant += normalMatrix_tmp[3 * k + 2] * probeOmega_radps_idx_0;
      }

      for (k = 0; k < 3; k++) {
        candidate[k] = ((inverseNormalMatrix[k + 3] * c13 +
                         inverseNormalMatrix[k] * probeOmega_radps_idx_1) +
                        inverseNormalMatrix[k + 6] * determinant) /
          columnScale[k];
      }

      probeOmega_radps_idx_0 = candidate[1];
      probeOmega_radps_idx_1 = candidate[0];
      c13 = candidate[2];
      for (k = 0; k < 8; k++) {
        determinant = observation[k];
        residual[k] = ((regression[k + 8] * probeOmega_radps_idx_0 +
                        regression[k] * probeOmega_radps_idx_1) + regression[k +
                       16] * c13) - determinant;
        observation[k] = determinant * determinant;
      }

      probeOmega_radps_idx_1 = observation[0];
      for (columnIndex = 0; columnIndex < 7; columnIndex++) {
        probeOmega_radps_idx_1 += observation[columnIndex + 1];
      }

      for (k = 0; k < 8; k++) {
        probeOmega_radps_idx_0 = residual[k];
        residual[k] = probeOmega_radps_idx_0 * probeOmega_radps_idx_0;
      }

      probeOmega_radps_idx_0 = residual[0];
      for (columnIndex = 0; columnIndex < 7; columnIndex++) {
        probeOmega_radps_idx_0 += residual[columnIndex + 1];
      }

      *relativeResidual = sqrtf(probeOmega_radps_idx_0) / fmaxf(sqrtf
        (probeOmega_radps_idx_1), 1.0E-12F);
      c[0] = !rtIsInfF(candidate[0]);
      d[0] = !rtIsNaNF(candidate[0]);
      c[1] = !rtIsInfF(candidate[1]);
      d[1] = !rtIsNaNF(candidate[1]);
      c[2] = !rtIsInfF(candidate[2]);
      d[2] = !rtIsNaNF(candidate[2]);
      y = true;
      columnIndex = 0;
      exitg1 = false;
      while (!exitg1 && (columnIndex < 3)) {
        if (!c[columnIndex] || !d[columnIndex]) {
          y = false;
          exitg1 = true;
        } else {
          columnIndex++;
        }
      }

      *solved = (y && (!rtIsInfF(*conditionNumber) && (!rtIsInfF
        (*relativeResidual) && !rtIsNaNF(*relativeResidual))));
    }
  } else {
    guard1 = true;
  }

  if (guard1) {
    *solved = false;
    *conditionNumber = (rtInfF);
    *relativeResidual = (rtInfF);
  }
}

/* Function for MATLAB Function: '<S1>/VafidDiscreteStep' */
static void v_vafid_discrete_reference_step(uint8_T mode, boolean_T resetRequest,
  boolean_T sampleValid, real32_T voltageAlpha_V, real32_T voltageBeta_V,
  real32_T currentAlpha_A, real32_T currentBeta_A, real32_T
  kreElectricalAngle_rad, real32_T kreElectricalOmega_radps, real32_T
  kreActiveFlux_Wb, const sXUMNpzAt0y7k62cD7EcU0C_vafid_T *config, real32_T
  *probeD_PU, real32_T *probeQ_PU, real32_T *rs_Ohm, real32_T *ld_H, real32_T
  *lq_H, real32_T *fluxPM_Wb, boolean_T *estimateValid, boolean_T *freshEstimate,
  boolean_T *staleEstimate, uint8_T *status, real32_T *conditionNumber, real32_T
  *relativeResidual, uint16_T *consecutiveAcceptedWindows, uint32_T
  *windowSampleCount)
{
  creal32_T phasorScale_0[2];
  creal32_T phasorScale_1[2];
  creal32_T phasorScale_2[2];
  creal32_T phasorScale_3[2];
  int32_T i;
  int32_T phasorReal_tmp;
  real32_T candidate[3];
  real32_T phaseCosine[2];
  real32_T phaseSine[2];
  real32_T cosAngle;
  real32_T phasorScale;
  real32_T predictedAngle_rad;
  real32_T probeQ_PU_tmp;
  real32_T signals_idx_0;
  real32_T signals_idx_1;
  real32_T sinAngle;
  uint32_T qY;
  boolean_T solved;
  if (!vafid_external_observer_wrap_DW.rsEstimate_Ohm_not_empty) {
    vafid_external_observer_wrap_DW.rsEstimate_Ohm = config->nominalRs_Ohm;
    vafid_external_observer_wrap_DW.rsEstimate_Ohm_not_empty = true;
  }

  if (!vafid_external_observer_wrap_DW.ldEstimate_H_not_empty) {
    vafid_external_observer_wrap_DW.ldEstimate_H = config->nominalLd_H;
    vafid_external_observer_wrap_DW.ldEstimate_H_not_empty = true;
  }

  if (!vafid_external_observer_wrap_DW.lqEstimate_H_not_empty) {
    vafid_external_observer_wrap_DW.lqEstimate_H = config->nominalLq_H;
    vafid_external_observer_wrap_DW.lqEstimate_H_not_empty = true;
  }

  if (!vafid_external_observer_wrap_DW.fluxEstimate_Wb_not_empty) {
    vafid_external_observer_wrap_DW.fluxEstimate_Wb = config->nominalFluxPM_Wb;
    vafid_external_observer_wrap_DW.fluxEstimate_Wb_not_empty = true;
  }

  *probeD_PU = 0.0F;
  *probeQ_PU = 0.0F;
  *freshEstimate = false;
  if (resetRequest) {
    vafid_external_observer_wrap_DW.phaseD_rad = 0.0F;
    vafid_external_observer_wrap_DW.phaseQ_rad = 0.0F;
    vafid_external_observer_wrap_DW.trackedAngle_rad = 0.0F;
    vafid_external_observer_wrap_DW.filteredOmega_radps = 0.0F;
    vafid_external_observer_wrap_DW.filteredActiveFlux_Wb = 0.0F;
    vafid_external_observer_wrap_DW.activeFluxInitialized = false;
    vafid_external_observer_wrap_DW.angleInitialized = false;
    for (i = 0; i < 8; i++) {
      vafid_external_observer_wrap_DW.phasorReal[i] = 0.0F;
      vafid_external_observer_wrap_DW.phasorImag[i] = 0.0F;
    }

    vafid_external_observer_wrap_DW.sumOmega_radps = 0.0F;
    vafid_external_observer_wrap_DW.sumId_A = 0.0F;
    vafid_external_observer_wrap_DW.settleSampleCount = 0U;
    vafid_external_observer_wrap_DW.accumulatedSampleCount = 0U;
    vafid_external_observer_wrap_DW.rsEstimate_Ohm = config->nominalRs_Ohm;
    vafid_external_observer_wrap_DW.ldEstimate_H = config->nominalLd_H;
    vafid_external_observer_wrap_DW.lqEstimate_H = config->nominalLq_H;
    vafid_external_observer_wrap_DW.fluxEstimate_Wb = config->nominalFluxPM_Wb;
    vafid_external_observer_wrap_DW.acceptedWindowCount = 0U;
    vafid_external_observer_wrap_DW.rejectedWindowCount = 0U;
    vafid_external_observer_wrap_DW.estimateIsCurrent = false;
    vafid_external_observer_wrap_DW.wasEnabled = false;
    vafid_external_observer_wrap_DW.lastConditionNumber = (rtInfF);
    vafid_external_observer_wrap_DW.lastRelativeResidual = (rtInfF);
    *status = 1U;
    *rs_Ohm = vafid_external_observer_wrap_DW.rsEstimate_Ohm;
    *ld_H = vafid_external_observer_wrap_DW.ldEstimate_H;
    *lq_H = vafid_external_observer_wrap_DW.lqEstimate_H;
    *fluxPM_Wb = vafid_external_observer_wrap_DW.fluxEstimate_Wb;
    *conditionNumber = (rtInfF);
    *relativeResidual = (rtInfF);
    *consecutiveAcceptedWindows = 0U;
    *windowSampleCount = 0U;
    *estimateValid = false;
    *staleEstimate = true;
  } else if (mode == 0) {
    vafid_external_observer_wrap_DW.phaseD_rad = 0.0F;
    vafid_external_observer_wrap_DW.phaseQ_rad = 0.0F;
    for (i = 0; i < 8; i++) {
      vafid_external_observer_wrap_DW.phasorReal[i] = 0.0F;
      vafid_external_observer_wrap_DW.phasorImag[i] = 0.0F;
    }

    vafid_external_observer_wrap_DW.sumOmega_radps = 0.0F;
    vafid_external_observer_wrap_DW.sumId_A = 0.0F;
    vafid_external_observer_wrap_DW.settleSampleCount = 0U;
    vafid_external_observer_wrap_DW.accumulatedSampleCount = 0U;
    vafid_external_observer_wrap_DW.acceptedWindowCount = 0U;
    vafid_external_observer_wrap_DW.rejectedWindowCount = 0U;
    vafid_external_observer_wrap_DW.estimateIsCurrent = false;
    vafid_external_observer_wrap_DW.wasEnabled = false;
    vafid_external_observer_wrap_DW.angleInitialized = false;
    vafid_external_observer_wrap_DW.activeFluxInitialized = false;
    *status = 0U;
    *rs_Ohm = vafid_external_observer_wrap_DW.rsEstimate_Ohm;
    *ld_H = vafid_external_observer_wrap_DW.ldEstimate_H;
    *lq_H = vafid_external_observer_wrap_DW.lqEstimate_H;
    *fluxPM_Wb = vafid_external_observer_wrap_DW.fluxEstimate_Wb;
    *conditionNumber = vafid_external_observer_wrap_DW.lastConditionNumber;
    *relativeResidual = vafid_external_observer_wrap_DW.lastRelativeResidual;
    *consecutiveAcceptedWindows = 0U;
    *windowSampleCount = 0U;
    *estimateValid = false;
    *staleEstimate = true;
  } else if (!vafid_localConfigurationIsValid(config) || (!sampleValid ||
              (rtIsInfF(voltageAlpha_V) || rtIsNaNF(voltageAlpha_V) || (rtIsInfF
                (voltageBeta_V) || rtIsNaNF(voltageBeta_V) || (rtIsInfF
      (currentAlpha_A) || rtIsNaNF(currentAlpha_A) || (rtIsInfF(currentBeta_A) ||
    rtIsNaNF(currentBeta_A) || (rtIsInfF(kreElectricalAngle_rad) || rtIsNaNF
      (kreElectricalAngle_rad) || (rtIsInfF(kreElectricalOmega_radps) ||
      rtIsNaNF(kreElectricalOmega_radps) || (rtIsInfF(kreActiveFlux_Wb) ||
      rtIsNaNF(kreActiveFlux_Wb)))))))))) {
    vafid_external_observer_wrap_DW.phaseD_rad = 0.0F;
    vafid_external_observer_wrap_DW.phaseQ_rad = 0.0F;
    for (i = 0; i < 8; i++) {
      vafid_external_observer_wrap_DW.phasorReal[i] = 0.0F;
      vafid_external_observer_wrap_DW.phasorImag[i] = 0.0F;
    }

    vafid_external_observer_wrap_DW.sumOmega_radps = 0.0F;
    vafid_external_observer_wrap_DW.sumId_A = 0.0F;
    vafid_external_observer_wrap_DW.settleSampleCount = 0U;
    vafid_external_observer_wrap_DW.accumulatedSampleCount = 0U;
    vafid_external_observer_wrap_DW.acceptedWindowCount = 0U;
    vafid_external_observer_wrap_DW.rejectedWindowCount = 0U;
    vafid_external_observer_wrap_DW.estimateIsCurrent = false;
    vafid_external_observer_wrap_DW.wasEnabled = false;
    vafid_external_observer_wrap_DW.angleInitialized = false;
    vafid_external_observer_wrap_DW.activeFluxInitialized = false;
    *status = 8U;
    *rs_Ohm = vafid_external_observer_wrap_DW.rsEstimate_Ohm;
    *ld_H = vafid_external_observer_wrap_DW.ldEstimate_H;
    *lq_H = vafid_external_observer_wrap_DW.lqEstimate_H;
    *fluxPM_Wb = vafid_external_observer_wrap_DW.fluxEstimate_Wb;
    *conditionNumber = vafid_external_observer_wrap_DW.lastConditionNumber;
    *relativeResidual = vafid_external_observer_wrap_DW.lastRelativeResidual;
    *consecutiveAcceptedWindows = 0U;
    *windowSampleCount = 0U;
    *estimateValid = false;
    *staleEstimate = true;
  } else {
    if (!vafid_external_observer_wrap_DW.wasEnabled) {
      vafid_external_observer_wrap_DW.phaseD_rad = 0.0F;
      vafid_external_observer_wrap_DW.phaseQ_rad = 0.0F;
      vafid_external_observer_wrap_DW.trackedAngle_rad = kreElectricalAngle_rad;
      vafid_external_observer_wrap_DW.trackedAngle_rad =
        vafid_external_observer_wra_mod
        (vafid_external_observer_wrap_DW.trackedAngle_rad + 3.1415927F) -
        3.1415927F;
      vafid_external_observer_wrap_DW.filteredOmega_radps =
        kreElectricalOmega_radps;
      vafid_external_observer_wrap_DW.filteredActiveFlux_Wb = kreActiveFlux_Wb;
      vafid_external_observer_wrap_DW.activeFluxInitialized = true;
      vafid_external_observer_wrap_DW.angleInitialized = true;
      for (i = 0; i < 8; i++) {
        vafid_external_observer_wrap_DW.phasorReal[i] = 0.0F;
        vafid_external_observer_wrap_DW.phasorImag[i] = 0.0F;
      }

      vafid_external_observer_wrap_DW.sumOmega_radps = 0.0F;
      vafid_external_observer_wrap_DW.sumId_A = 0.0F;
      vafid_external_observer_wrap_DW.settleSampleCount = 0U;
      vafid_external_observer_wrap_DW.accumulatedSampleCount = 0U;
      vafid_external_observer_wrap_DW.acceptedWindowCount = 0U;
      vafid_external_observer_wrap_DW.rejectedWindowCount = 0U;
      vafid_external_observer_wrap_DW.estimateIsCurrent = false;
      vafid_external_observer_wrap_DW.wasEnabled = true;
    }

    phasorScale = sinf(vafid_external_observer_wrap_DW.phaseD_rad);
    *probeD_PU = fminf(fabsf(config->probeD_Amplitude_PU),
                       config->probeD_MaxAmplitude_PU) * phasorScale;
    probeQ_PU_tmp = sinf(vafid_external_observer_wrap_DW.phaseQ_rad);
    *probeQ_PU = fminf(fabsf(config->probeQ_Amplitude_PU),
                       config->probeQ_MaxAmplitude_PU) * probeQ_PU_tmp;
    vafid_external_observer_wrap_DW.filteredOmega_radps += (1.0F - expf
      (-6.2831855F * config->omegaLpf_Hz * config->sampleTime_s)) *
      (kreElectricalOmega_radps -
       vafid_external_observer_wrap_DW.filteredOmega_radps);
    if (!vafid_external_observer_wrap_DW.angleInitialized) {
      vafid_external_observer_wrap_DW.trackedAngle_rad = kreElectricalAngle_rad;
      vafid_external_observer_wrap_DW.trackedAngle_rad =
        vafid_external_observer_wra_mod
        (vafid_external_observer_wrap_DW.trackedAngle_rad + 3.1415927F) -
        3.1415927F;
      vafid_external_observer_wrap_DW.angleInitialized = true;
    } else {
      predictedAngle_rad = vafid_external_observer_wra_mod((config->sampleTime_s
        * vafid_external_observer_wrap_DW.filteredOmega_radps +
        vafid_external_observer_wrap_DW.trackedAngle_rad) + 3.1415927F) -
        3.1415927F;
      vafid_external_observer_wrap_DW.trackedAngle_rad = (1.0F - expf
        (-6.2831855F * config->angleTrack_Hz * config->sampleTime_s)) *
        (vafid_external_observer_wra_mod((kreElectricalAngle_rad -
           predictedAngle_rad) + 3.1415927F) - 3.1415927F) + predictedAngle_rad;
      vafid_external_observer_wrap_DW.trackedAngle_rad =
        vafid_external_observer_wra_mod
        (vafid_external_observer_wrap_DW.trackedAngle_rad + 3.1415927F) -
        3.1415927F;
    }

    if (!vafid_external_observer_wrap_DW.activeFluxInitialized) {
      vafid_external_observer_wrap_DW.filteredActiveFlux_Wb = kreActiveFlux_Wb;
      vafid_external_observer_wrap_DW.activeFluxInitialized = true;
    } else {
      vafid_external_observer_wrap_DW.filteredActiveFlux_Wb += (1.0F - expf
        (-6.2831855F * config->activeFluxLpf_Hz * config->sampleTime_s)) *
        (kreActiveFlux_Wb -
         vafid_external_observer_wrap_DW.filteredActiveFlux_Wb);
    }

    cosAngle = cosf(vafid_external_observer_wrap_DW.trackedAngle_rad);
    sinAngle = sinf(vafid_external_observer_wrap_DW.trackedAngle_rad);
    predictedAngle_rad = cosAngle * currentAlpha_A + sinAngle * currentBeta_A;
    signals_idx_0 = rt_roundf_snf(config->settleTime_s / config->sampleTime_s);
    if (signals_idx_0 < 4.2949673E+9F) {
      if (signals_idx_0 >= 0.0F) {
        qY = (uint32_T)signals_idx_0;
      } else {
        qY = 0U;
      }
    } else {
      qY = MAX_uint32_T;
    }

    if (vafid_external_observer_wrap_DW.settleSampleCount < qY) {
      vafid_external_observer_wrap_DW.settleSampleCount++;
      *status = 2U;
    } else {
      signals_idx_0 = cosAngle * voltageAlpha_V + sinAngle * voltageBeta_V;
      signals_idx_1 = -sinAngle * voltageAlpha_V + cosAngle * voltageBeta_V;
      cosAngle = -sinAngle * currentAlpha_A + cosAngle * currentBeta_A;
      phaseCosine[0] = cosf(vafid_external_observer_wrap_DW.phaseD_rad);
      phaseCosine[1] = cosf(vafid_external_observer_wrap_DW.phaseQ_rad);
      phaseSine[0] = phasorScale;
      phaseSine[1] = probeQ_PU_tmp;
      for (i = 0; i < 2; i++) {
        phasorScale = phaseCosine[i];
        probeQ_PU_tmp = phaseSine[i];
        phasorReal_tmp = i << 2;
        vafid_external_observer_wrap_DW.phasorReal[phasorReal_tmp] +=
          signals_idx_0 * phasorScale;
        vafid_external_observer_wrap_DW.phasorImag[phasorReal_tmp] -=
          signals_idx_0 * probeQ_PU_tmp;
        vafid_external_observer_wrap_DW.phasorReal[phasorReal_tmp + 1] +=
          signals_idx_1 * phasorScale;
        vafid_external_observer_wrap_DW.phasorImag[phasorReal_tmp + 1] -=
          signals_idx_1 * probeQ_PU_tmp;
        vafid_external_observer_wrap_DW.phasorReal[phasorReal_tmp + 2] +=
          predictedAngle_rad * phasorScale;
        vafid_external_observer_wrap_DW.phasorImag[phasorReal_tmp + 2] -=
          predictedAngle_rad * probeQ_PU_tmp;
        vafid_external_observer_wrap_DW.phasorReal[phasorReal_tmp + 3] +=
          cosAngle * phasorScale;
        vafid_external_observer_wrap_DW.phasorImag[phasorReal_tmp + 3] -=
          cosAngle * probeQ_PU_tmp;
      }

      vafid_external_observer_wrap_DW.sumOmega_radps +=
        vafid_external_observer_wrap_DW.filteredOmega_radps;
      vafid_external_observer_wrap_DW.sumId_A += predictedAngle_rad;
      qY = vafid_external_observer_wrap_DW.accumulatedSampleCount +
        /*MW:OvSatOk*/ 1U;
      if (vafid_external_observer_wrap_DW.accumulatedSampleCount + 1U <
          vafid_external_observer_wrap_DW.accumulatedSampleCount) {
        qY = MAX_uint32_T;
      }

      vafid_external_observer_wrap_DW.accumulatedSampleCount = qY;
      *status = 3U;
      if (vafid_external_observer_wrap_DW.accumulatedSampleCount >=
          config->windowLength_samples) {
        phasorScale = 2.0F / (real32_T)config->windowLength_samples;
        predictedAngle_rad = vafid_external_observer_wrap_DW.sumId_A / (real32_T)
          config->windowLength_samples;
        phasorScale_0[0].re = phasorScale *
          vafid_external_observer_wrap_DW.phasorReal[0];
        phasorScale_0[0].im = phasorScale *
          vafid_external_observer_wrap_DW.phasorImag[0];
        phasorScale_1[0].re = phasorScale *
          vafid_external_observer_wrap_DW.phasorReal[1];
        phasorScale_1[0].im = phasorScale *
          vafid_external_observer_wrap_DW.phasorImag[1];
        phasorScale_2[0].re = phasorScale *
          vafid_external_observer_wrap_DW.phasorReal[2];
        phasorScale_2[0].im = phasorScale *
          vafid_external_observer_wrap_DW.phasorImag[2];
        phasorScale_3[0].re = phasorScale *
          vafid_external_observer_wrap_DW.phasorReal[3];
        phasorScale_3[0].im = phasorScale *
          vafid_external_observer_wrap_DW.phasorImag[3];
        phasorScale_0[1].re = phasorScale *
          vafid_external_observer_wrap_DW.phasorReal[4];
        phasorScale_0[1].im = phasorScale *
          vafid_external_observer_wrap_DW.phasorImag[4];
        phasorScale_1[1].re = phasorScale *
          vafid_external_observer_wrap_DW.phasorReal[5];
        phasorScale_1[1].im = phasorScale *
          vafid_external_observer_wrap_DW.phasorImag[5];
        phasorScale_2[1].re = phasorScale *
          vafid_external_observer_wrap_DW.phasorReal[6];
        phasorScale_2[1].im = phasorScale *
          vafid_external_observer_wrap_DW.phasorImag[6];
        phasorScale_3[1].re = phasorScale *
          vafid_external_observer_wrap_DW.phasorReal[7];
        phasorScale_3[1].im = phasorScale *
          vafid_external_observer_wrap_DW.phasorImag[7];
        vafid_exte_localSolveParameters(phasorScale_0, phasorScale_1,
          phasorScale_2, phasorScale_3,
          vafid_external_observer_wrap_DW.sumOmega_radps / (real32_T)
          config->windowLength_samples, config->probeD_Frequency_Hz,
          config->probeQ_Frequency_Hz, candidate, &solved,
          &vafid_external_observer_wrap_DW.lastConditionNumber,
          &vafid_external_observer_wrap_DW.lastRelativeResidual);
        phasorScale = vafid_external_observer_wrap_DW.filteredActiveFlux_Wb -
          (candidate[1] - candidate[2]) * predictedAngle_rad;
        if (!solved) {
          *status = 10U;
          vafid_external_observer_wrap_DW.acceptedWindowCount = 0U;
          if (vafid_external_observer_wrap_DW.rejectedWindowCount < 65535) {
            vafid_external_observer_wrap_DW.rejectedWindowCount++;
          }
        } else if (vafid_external_observer_wrap_DW.lastConditionNumber >=
                   config->conditionLimit) {
          *status = 5U;
          vafid_external_observer_wrap_DW.acceptedWindowCount = 0U;
          if (vafid_external_observer_wrap_DW.rejectedWindowCount < 65535) {
            vafid_external_observer_wrap_DW.rejectedWindowCount++;
          }
        } else if (vafid_external_observer_wrap_DW.lastRelativeResidual >=
                   config->relativeResidualLimit) {
          *status = 6U;
          vafid_external_observer_wrap_DW.acceptedWindowCount = 0U;
          if (vafid_external_observer_wrap_DW.rejectedWindowCount < 65535) {
            vafid_external_observer_wrap_DW.rejectedWindowCount++;
          }
        } else if (!(candidate[0] >= config->minimumRs_Ohm) || (!(candidate[0] <=
          config->maximumRs_Ohm) || (!(candidate[1] >= config->minimumLd_H) || (
                      !(candidate[1] <= config->maximumLd_H) || (!(candidate[2] >=
          config->minimumLq_H) || !(candidate[2] <= config->maximumLq_H))))) ||
                   (rtIsInfF(phasorScale) || rtIsNaNF(phasorScale) ||
                    (!(phasorScale >= config->minimumFluxPM_Wb) || !(phasorScale
          <= config->maximumFluxPM_Wb)))) {
          *status = 7U;
          vafid_external_observer_wrap_DW.acceptedWindowCount = 0U;
          if (vafid_external_observer_wrap_DW.rejectedWindowCount < 65535) {
            vafid_external_observer_wrap_DW.rejectedWindowCount++;
          }
        } else {
          vafid_external_observer_wrap_DW.rsEstimate_Ohm += (candidate[0] -
            vafid_external_observer_wrap_DW.rsEstimate_Ohm) *
            config->parameterFusion;
          vafid_external_observer_wrap_DW.ldEstimate_H += (candidate[1] -
            vafid_external_observer_wrap_DW.ldEstimate_H) *
            config->parameterFusion;
          vafid_external_observer_wrap_DW.lqEstimate_H += (candidate[2] -
            vafid_external_observer_wrap_DW.lqEstimate_H) *
            config->parameterFusion;
          if (vafid_external_observer_wrap_DW.acceptedWindowCount < 65535) {
            vafid_external_observer_wrap_DW.acceptedWindowCount++;
          }

          vafid_external_observer_wrap_DW.rejectedWindowCount = 0U;
          *status = 4U;
          *freshEstimate = true;
          if (vafid_external_observer_wrap_DW.acceptedWindowCount >=
              config->requiredAcceptedWindows) {
            vafid_external_observer_wrap_DW.fluxEstimate_Wb +=
              ((vafid_external_observer_wrap_DW.filteredActiveFlux_Wb -
                (vafid_external_observer_wrap_DW.ldEstimate_H -
                 vafid_external_observer_wrap_DW.lqEstimate_H) *
                predictedAngle_rad) -
               vafid_external_observer_wrap_DW.fluxEstimate_Wb) *
              config->fluxFusion;
            vafid_external_observer_wrap_DW.estimateIsCurrent = true;
          } else {
            vafid_external_observer_wrap_DW.estimateIsCurrent = false;
          }
        }

        vafid_external_observer_wrap_DW.estimateIsCurrent =
          ((vafid_external_observer_wrap_DW.rejectedWindowCount <
            config->staleRejectedWindows) &&
           vafid_external_observer_wrap_DW.estimateIsCurrent);
        for (i = 0; i < 8; i++) {
          vafid_external_observer_wrap_DW.phasorReal[i] = 0.0F;
          vafid_external_observer_wrap_DW.phasorImag[i] = 0.0F;
        }

        vafid_external_observer_wrap_DW.sumOmega_radps = 0.0F;
        vafid_external_observer_wrap_DW.sumId_A = 0.0F;
        vafid_external_observer_wrap_DW.accumulatedSampleCount = 0U;
      }
    }

    vafid_external_observer_wrap_DW.phaseD_rad += 6.2831855F *
      config->probeD_Frequency_Hz * config->sampleTime_s;
    vafid_external_observer_wrap_DW.phaseD_rad = vafid_external_observer_wra_mod
      (vafid_external_observer_wrap_DW.phaseD_rad + 3.1415927F) - 3.1415927F;
    vafid_external_observer_wrap_DW.phaseQ_rad += 6.2831855F *
      config->probeQ_Frequency_Hz * config->sampleTime_s;
    vafid_external_observer_wrap_DW.phaseQ_rad = vafid_external_observer_wra_mod
      (vafid_external_observer_wrap_DW.phaseQ_rad + 3.1415927F) - 3.1415927F;
    *rs_Ohm = vafid_external_observer_wrap_DW.rsEstimate_Ohm;
    *ld_H = vafid_external_observer_wrap_DW.ldEstimate_H;
    *lq_H = vafid_external_observer_wrap_DW.lqEstimate_H;
    *fluxPM_Wb = vafid_external_observer_wrap_DW.fluxEstimate_Wb;
    *conditionNumber = vafid_external_observer_wrap_DW.lastConditionNumber;
    *relativeResidual = vafid_external_observer_wrap_DW.lastRelativeResidual;
    *consecutiveAcceptedWindows =
      vafid_external_observer_wrap_DW.acceptedWindowCount;
    *windowSampleCount = vafid_external_observer_wrap_DW.accumulatedSampleCount;
    *estimateValid = vafid_external_observer_wrap_DW.estimateIsCurrent;
    *staleEstimate = !vafid_external_observer_wrap_DW.estimateIsCurrent;
  }
}

/* Model step function */
void vafid_external_observer_wrapper_step(void)
{
  sXUMNpzAt0y7k62cD7EcU0C_vafid_T expl_temp;
  int32_T i;
  real32_T tmp;

  /* Outputs for Resettable SubSystem: '<Root>/VafidResettable' incorporates:
   *  ResetPort: '<S1>/Reset'
   */
  /* Inport: '<Root>/resetRequest' */
  if (vafid_external_observer_wrapp_U.resetRequest &&
      (vafid_external_observer_PrevZCX.VafidResettable_Reset_ZCE != POS_ZCSIG))
  {
    /* SystemReset for MATLAB Function: '<S1>/VafidDiscreteStep' */
    vafid_external_observer_wrap_DW.rsEstimate_Ohm_not_empty = false;
    vafid_external_observer_wrap_DW.ldEstimate_H_not_empty = false;
    vafid_external_observer_wrap_DW.lqEstimate_H_not_empty = false;
    vafid_external_observer_wrap_DW.fluxEstimate_Wb_not_empty = false;
    vafid_external_observer_wrap_DW.phaseD_rad = 0.0F;
    vafid_external_observer_wrap_DW.phaseQ_rad = 0.0F;
    vafid_external_observer_wrap_DW.trackedAngle_rad = 0.0F;
    vafid_external_observer_wrap_DW.filteredOmega_radps = 0.0F;
    vafid_external_observer_wrap_DW.filteredActiveFlux_Wb = 0.0F;
    vafid_external_observer_wrap_DW.activeFluxInitialized = false;
    vafid_external_observer_wrap_DW.angleInitialized = false;
    for (i = 0; i < 8; i++) {
      vafid_external_observer_wrap_DW.phasorReal[i] = 0.0F;
      vafid_external_observer_wrap_DW.phasorImag[i] = 0.0F;
    }

    vafid_external_observer_wrap_DW.sumOmega_radps = 0.0F;
    vafid_external_observer_wrap_DW.sumId_A = 0.0F;
    vafid_external_observer_wrap_DW.settleSampleCount = 0U;
    vafid_external_observer_wrap_DW.accumulatedSampleCount = 0U;
    vafid_external_observer_wrap_DW.acceptedWindowCount = 0U;
    vafid_external_observer_wrap_DW.rejectedWindowCount = 0U;
    vafid_external_observer_wrap_DW.estimateIsCurrent = false;
    vafid_external_observer_wrap_DW.wasEnabled = false;
    vafid_external_observer_wrap_DW.lastConditionNumber = (rtInfF);
    vafid_external_observer_wrap_DW.lastRelativeResidual = (rtInfF);

    /* End of SystemReset for MATLAB Function: '<S1>/VafidDiscreteStep' */
  }

  vafid_external_observer_PrevZCX.VafidResettable_Reset_ZCE =
    vafid_external_observer_wrapp_U.resetRequest;

  /* MATLAB Function: '<S1>/VafidDiscreteStep' incorporates:
   *  Inport: '<Root>/configValues'
   *  Inport: '<Root>/currentAlpha_A'
   *  Inport: '<Root>/currentBeta_A'
   *  Inport: '<Root>/kreActiveFlux_Wb'
   *  Inport: '<Root>/kreElectricalAngle_rad'
   *  Inport: '<Root>/kreElectricalOmega_radps'
   *  Inport: '<Root>/mode'
   *  Inport: '<Root>/resetRequest'
   *  Inport: '<Root>/sampleValid'
   *  Inport: '<Root>/voltageAlpha_V'
   *  Inport: '<Root>/voltageBeta_V'
   *  Outport: '<Root>/conditionNumber'
   *  Outport: '<Root>/consecutiveAcceptedWindows'
   *  Outport: '<Root>/estimateValid'
   *  Outport: '<Root>/fluxPM_Wb'
   *  Outport: '<Root>/freshEstimate'
   *  Outport: '<Root>/ld_H'
   *  Outport: '<Root>/lq_H'
   *  Outport: '<Root>/probeD_PU'
   *  Outport: '<Root>/probeQ_PU'
   *  Outport: '<Root>/relativeResidual'
   *  Outport: '<Root>/rs_Ohm'
   *  Outport: '<Root>/staleEstimate'
   *  Outport: '<Root>/status'
   *  Outport: '<Root>/windowSampleCount'
   */
  expl_temp.maximumFluxPM_Wb = vafid_external_observer_wrapp_U.configValues[29];
  expl_temp.minimumFluxPM_Wb = vafid_external_observer_wrapp_U.configValues[28];
  expl_temp.maximumLq_H = vafid_external_observer_wrapp_U.configValues[27];
  expl_temp.minimumLq_H = vafid_external_observer_wrapp_U.configValues[26];
  expl_temp.maximumLd_H = vafid_external_observer_wrapp_U.configValues[25];
  expl_temp.minimumLd_H = vafid_external_observer_wrapp_U.configValues[24];
  expl_temp.maximumRs_Ohm = vafid_external_observer_wrapp_U.configValues[23];
  expl_temp.minimumRs_Ohm = vafid_external_observer_wrapp_U.configValues[22];
  expl_temp.nominalFluxPM_Wb = vafid_external_observer_wrapp_U.configValues[21];
  expl_temp.nominalLq_H = vafid_external_observer_wrapp_U.configValues[20];
  expl_temp.nominalLd_H = vafid_external_observer_wrapp_U.configValues[19];
  expl_temp.nominalRs_Ohm = vafid_external_observer_wrapp_U.configValues[18];
  tmp = rt_roundf_snf(vafid_external_observer_wrapp_U.configValues[17]);
  if (tmp < 65536.0F) {
    if (tmp >= 0.0F) {
      expl_temp.staleRejectedWindows = (uint16_T)tmp;
    } else {
      expl_temp.staleRejectedWindows = 0U;
    }
  } else {
    expl_temp.staleRejectedWindows = MAX_uint16_T;
  }

  tmp = rt_roundf_snf(vafid_external_observer_wrapp_U.configValues[16]);
  if (tmp < 65536.0F) {
    if (tmp >= 0.0F) {
      expl_temp.requiredAcceptedWindows = (uint16_T)tmp;
    } else {
      expl_temp.requiredAcceptedWindows = 0U;
    }
  } else {
    expl_temp.requiredAcceptedWindows = MAX_uint16_T;
  }

  expl_temp.relativeResidualLimit =
    vafid_external_observer_wrapp_U.configValues[15];
  expl_temp.conditionLimit = vafid_external_observer_wrapp_U.configValues[14];
  expl_temp.fluxFusion = vafid_external_observer_wrapp_U.configValues[13];
  expl_temp.parameterFusion = vafid_external_observer_wrapp_U.configValues[12];
  expl_temp.activeFluxLpf_Hz = vafid_external_observer_wrapp_U.configValues[11];
  expl_temp.angleTrack_Hz = vafid_external_observer_wrapp_U.configValues[10];
  expl_temp.omegaLpf_Hz = vafid_external_observer_wrapp_U.configValues[9];
  tmp = rt_roundf_snf(vafid_external_observer_wrapp_U.configValues[8]);
  if (tmp < 4.2949673E+9F) {
    if (tmp >= 0.0F) {
      expl_temp.windowLength_samples = (uint32_T)tmp;
    } else {
      expl_temp.windowLength_samples = 0U;
    }
  } else {
    expl_temp.windowLength_samples = MAX_uint32_T;
  }

  expl_temp.settleTime_s = vafid_external_observer_wrapp_U.configValues[7];
  expl_temp.probeQ_MaxAmplitude_PU =
    vafid_external_observer_wrapp_U.configValues[6];
  expl_temp.probeD_MaxAmplitude_PU =
    vafid_external_observer_wrapp_U.configValues[5];
  expl_temp.probeQ_Amplitude_PU = vafid_external_observer_wrapp_U.configValues[4];
  expl_temp.probeD_Amplitude_PU = vafid_external_observer_wrapp_U.configValues[3];
  expl_temp.probeQ_Frequency_Hz = vafid_external_observer_wrapp_U.configValues[2];
  expl_temp.probeD_Frequency_Hz = vafid_external_observer_wrapp_U.configValues[1];
  expl_temp.sampleTime_s = vafid_external_observer_wrapp_U.configValues[0];
  v_vafid_discrete_reference_step(vafid_external_observer_wrapp_U.mode,
    vafid_external_observer_wrapp_U.resetRequest,
    vafid_external_observer_wrapp_U.sampleValid,
    vafid_external_observer_wrapp_U.voltageAlpha_V,
    vafid_external_observer_wrapp_U.voltageBeta_V,
    vafid_external_observer_wrapp_U.currentAlpha_A,
    vafid_external_observer_wrapp_U.currentBeta_A,
    vafid_external_observer_wrapp_U.kreElectricalAngle_rad,
    vafid_external_observer_wrapp_U.kreElectricalOmega_radps,
    vafid_external_observer_wrapp_U.kreActiveFlux_Wb, &expl_temp,
    &vafid_external_observer_wrapp_Y.probeD_PU,
    &vafid_external_observer_wrapp_Y.probeQ_PU,
    &vafid_external_observer_wrapp_Y.rs_Ohm,
    &vafid_external_observer_wrapp_Y.ld_H, &vafid_external_observer_wrapp_Y.lq_H,
    &vafid_external_observer_wrapp_Y.fluxPM_Wb,
    &vafid_external_observer_wrapp_Y.estimateValid,
    &vafid_external_observer_wrapp_Y.freshEstimate,
    &vafid_external_observer_wrapp_Y.staleEstimate,
    &vafid_external_observer_wrapp_Y.status,
    &vafid_external_observer_wrapp_Y.conditionNumber,
    &vafid_external_observer_wrapp_Y.relativeResidual,
    &vafid_external_observer_wrapp_Y.consecutiveAcceptedWindows,
    &vafid_external_observer_wrapp_Y.windowSampleCount);

  /* End of MATLAB Function: '<S1>/VafidDiscreteStep' */
  /* End of Outputs for SubSystem: '<Root>/VafidResettable' */
}

/* Model initialize function */
void vafid_external_observer_wrapper_initialize(void)
{
  vafid_external_observer_PrevZCX.VafidResettable_Reset_ZCE = POS_ZCSIG;

  /* SystemInitialize for Resettable SubSystem: '<Root>/VafidResettable' */
  /* SystemInitialize for MATLAB Function: '<S1>/VafidDiscreteStep' */
  vafid_external_observer_wrap_DW.lastConditionNumber = (rtInfF);
  vafid_external_observer_wrap_DW.lastRelativeResidual = (rtInfF);

  /* End of SystemInitialize for SubSystem: '<Root>/VafidResettable' */
}

/* Model terminate function */
void vafid_external_observer_wrapper_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
