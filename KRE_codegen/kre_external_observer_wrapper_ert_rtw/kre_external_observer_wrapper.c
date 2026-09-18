/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: kre_external_observer_wrapper.c
 *
 * Code generated for Simulink model 'kre_external_observer_wrapper'.
 *
 * Model version                  : 1.9
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Thu Aug 27 23:33:01 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "kre_external_observer_wrapper.h"
#include "rt_nonfinite.h"
#include <math.h>
#include "kre_external_observer_wrapper_private.h"
#include "rtwtypes.h"
#include "zero_crossing_types.h"
#include "rt_defines.h"

/* Block states (default storage) */
DW_kre_external_observer_wrap_T kre_external_observer_wrappe_DW;

/* Previous zero-crossings (trigger) states */
PrevZCX_kre_external_observer_T kre_external_observer_w_PrevZCX;

/* External inputs (root inport signals with default storage) */
ExtU_kre_external_observer_wr_T kre_external_observer_wrapper_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_kre_external_observer_wr_T kre_external_observer_wrapper_Y;
KreDiscreteCoefficients kre_external_observer_coefficients;

/* Real-time model */
static RT_MODEL_kre_external_observe_T kre_external_observer_wrappe_M_;
RT_MODEL_kre_external_observe_T *const kre_external_observer_wrappe_M =
  &kre_external_observer_wrappe_M_;
real32_T kre_rt_atan2f_snf(real32_T u0, real32_T u1)
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

/* Model step function */
void kre_external_observer_wrapper_step(void)
{
  real32_T currentAB_idx_0;
  real32_T currentAB_idx_1;
  real32_T h1I_idx_0;
  real32_T h1I_idx_1;
  real32_T h2Gain;
  real32_T l0;
  real32_T omega1;
  real32_T omega1_idx_0;
  real32_T phi_idx_0;
  real32_T phi_idx_1;
  real32_T pllNaturalFrequency;
  real32_T thetaRaw;
  real32_T xHat_idx_0;
  real32_T xHat_idx_1;
  boolean_T valid;

  /* Outputs for Resettable SubSystem: '<Root>/KreObserverResettable' incorporates:
   *  ResetPort: '<S1>/Reset'
   */
  /* Inport: '<Root>/kreReset' */
  if ((kre_external_observer_wrapper_U.kreReset > 0) &&
      (kre_external_observer_w_PrevZCX.KreObserverResettable_Reset_ZCE !=
       POS_ZCSIG)) {
    /* SystemReset for MATLAB Function: '<S1>/KreExternalObserverStep' */
    kre_external_observer_wrappe_DW.h2Vri[0] = 0.0F;
    kre_external_observer_wrappe_DW.h2I[0] = 0.0F;
    kre_external_observer_wrappe_DW.h2Vri[1] = 0.0F;
    kre_external_observer_wrappe_DW.h2I[1] = 0.0F;
    kre_external_observer_wrappe_DW.h2Reg = 0.0F;
    kre_external_observer_wrappe_DW.h2D = 0.0F;
    kre_external_observer_wrappe_DW.qState[0] = 0.0F;
    kre_external_observer_wrappe_DW.qState[1] = 0.0F;
    kre_external_observer_wrappe_DW.qState[2] = 0.0F;
    kre_external_observer_wrappe_DW.qState[3] = 0.0F;
    kre_external_observer_wrappe_DW.yState[0] = 0.0F;
    kre_external_observer_wrappe_DW.lambdaHat[0] = 0.0F;
    kre_external_observer_wrappe_DW.yState[1] = 0.0F;
    kre_external_observer_wrappe_DW.lambdaHat[1] = 0.0F;
    kre_external_observer_wrappe_DW.pllTheta = 0.0F;
    kre_external_observer_wrappe_DW.pllOmegaInt = 0.0F;
    kre_external_observer_wrappe_DW.speedFilt = 0.0F;
    kre_external_observer_wrappe_DW.pllInitialized = false;
  }

  kre_external_observer_w_PrevZCX.KreObserverResettable_Reset_ZCE = (ZCSigState)
    (kre_external_observer_wrapper_U.kreReset > 0);

  /* End of Inport: '<Root>/kreReset' */

  /* Outport: '<Root>/krePositionPU' incorporates:
   *  MATLAB Function: '<S1>/KreExternalObserverStep'
   */
  kre_external_observer_wrapper_Y.krePositionPU = 0.0F;

  /* Outport: '<Root>/kreSpeedPU' incorporates:
   *  MATLAB Function: '<S1>/KreExternalObserverStep'
   */
  kre_external_observer_wrapper_Y.kreSpeedPU = 0.0F;

  /* Outport: '<Root>/kreActiveFlux_Wb' incorporates:
   *  MATLAB Function: '<S1>/KreExternalObserverStep'
   */
  kre_external_observer_wrapper_Y.kreActiveFlux_Wb = 0.0F;

  /* Outport: '<Root>/kreRawOmega_radps' incorporates:
   *  MATLAB Function: '<S1>/KreExternalObserverStep'
   */
  kre_external_observer_wrapper_Y.kreRawOmega_radps = 0.0F;

  /* Outport: '<Root>/kreStatus' incorporates:
   *  MATLAB Function: '<S1>/KreExternalObserverStep'
   */
  kre_external_observer_wrapper_Y.kreStatus = 0U;

  /* MATLAB Function: '<S1>/KreExternalObserverStep' incorporates:
   *  Inport: '<Root>/kreEnable'
   *  Inport: '<Root>/kreParams'
   *  Inport: '<Root>/krePllParams'
   *  Inport: '<Root>/kreViFb'
   */
  if (kre_external_observer_wrapper_U.kreEnable != 0) {
    valid = (!rtIsInfF(kre_external_observer_wrapper_U.kreViFb[0]) && !rtIsNaNF
             (kre_external_observer_wrapper_U.kreViFb[0]));
    if (valid) {
      valid = (!rtIsInfF(kre_external_observer_wrapper_U.kreViFb[1]) &&
               !rtIsNaNF(kre_external_observer_wrapper_U.kreViFb[1]));
    }

    if (valid) {
      valid = (!rtIsInfF(kre_external_observer_wrapper_U.kreViFb[2]) &&
               !rtIsNaNF(kre_external_observer_wrapper_U.kreViFb[2]));
    }

    if (valid) {
      valid = (!rtIsInfF(kre_external_observer_wrapper_U.kreViFb[3]) &&
               !rtIsNaNF(kre_external_observer_wrapper_U.kreViFb[3]));
    }

    /* Local generated-C customization: parameters are trusted here.
     * Keep input/output finite checks; do not scan parameter values or ranges
     * in the fast step. Reapply this change after model code regeneration. */
    if (!valid) {
      /* Outport: '<Root>/kreStatus' */
      kre_external_observer_wrapper_Y.kreStatus = 2U;
    } else {
      currentAB_idx_0 = kre_external_observer_wrapper_U.kreViFb[2] *
        kre_external_observer_wrapper_U.kreParams[6];
      currentAB_idx_1 = kre_external_observer_wrapper_U.kreViFb[3] *
        kre_external_observer_wrapper_U.kreParams[6];
      l0 = kre_external_observer_coefficients.inductanceDelta;
      h2Gain = kre_external_observer_coefficients.h2Gain;
      thetaRaw = kre_external_observer_wrapper_U.kreViFb[0] *
        kre_external_observer_wrapper_U.kreParams[5] -
        kre_external_observer_wrapper_U.kreParams[0] * currentAB_idx_0;
      pllNaturalFrequency = kre_external_observer_wrapper_U.kreViFb[1] *
        kre_external_observer_wrapper_U.kreParams[5] -
        kre_external_observer_wrapper_U.kreParams[0] * currentAB_idx_1;
      phi_idx_1 = (thetaRaw - kre_external_observer_wrappe_DW.h2Vri[0]) * h2Gain
        + kre_external_observer_wrappe_DW.h2Vri[0];
      kre_external_observer_wrappe_DW.h2Vri[0] = phi_idx_1;
      xHat_idx_1 = (currentAB_idx_0 - kre_external_observer_wrappe_DW.h2I[0]) *
        h2Gain + kre_external_observer_wrappe_DW.h2I[0];
      kre_external_observer_wrappe_DW.h2I[0] = xHat_idx_1;
      h1I_idx_1 = (currentAB_idx_0 - xHat_idx_1) *
        kre_external_observer_wrapper_U.kreParams[9];
      omega1 = phi_idx_1 - kre_external_observer_wrapper_U.kreParams[2] *
        h1I_idx_1;
      omega1_idx_0 = omega1;
      h1I_idx_1 = omega1 - l0 * h1I_idx_1;
      h1I_idx_0 = h1I_idx_1;
      phi_idx_0 = omega1 + h1I_idx_1;
      xHat_idx_0 = kre_external_observer_wrappe_DW.lambdaHat[0] -
        kre_external_observer_wrapper_U.kreParams[2] * currentAB_idx_0;
      phi_idx_1 = (pllNaturalFrequency - kre_external_observer_wrappe_DW.h2Vri[1])
        * h2Gain + kre_external_observer_wrappe_DW.h2Vri[1];
      kre_external_observer_wrappe_DW.h2Vri[1] = phi_idx_1;
      xHat_idx_1 = (currentAB_idx_1 - kre_external_observer_wrappe_DW.h2I[1]) *
        h2Gain + kre_external_observer_wrappe_DW.h2I[1];
      kre_external_observer_wrappe_DW.h2I[1] = xHat_idx_1;
      h1I_idx_1 = (currentAB_idx_1 - xHat_idx_1) *
        kre_external_observer_wrapper_U.kreParams[9];
      omega1 = phi_idx_1 - kre_external_observer_wrapper_U.kreParams[2] *
        h1I_idx_1;
      h1I_idx_1 = omega1 - l0 * h1I_idx_1;
      phi_idx_1 = omega1 + h1I_idx_1;
      xHat_idx_1 = kre_external_observer_wrappe_DW.lambdaHat[1] -
        kre_external_observer_wrapper_U.kreParams[2] * currentAB_idx_1;
      kre_external_observer_wrappe_DW.h2Reg += ((h1I_idx_0 * omega1_idx_0 +
        h1I_idx_1 * omega1) - kre_external_observer_wrappe_DW.h2Reg) * h2Gain;
      kre_external_observer_wrapper_Y.kreActiveFlux_Wb = sqrtf(xHat_idx_0 *
        xHat_idx_0 + xHat_idx_1 * xHat_idx_1);
      if (kre_external_observer_wrapper_Y.kreActiveFlux_Wb >=
          kre_external_observer_wrapper_U.kreParams[12]) {
        h1I_idx_0 = xHat_idx_0 /
          kre_external_observer_wrapper_Y.kreActiveFlux_Wb;
        h1I_idx_1 = xHat_idx_1 /
          kre_external_observer_wrapper_Y.kreActiveFlux_Wb;
      } else {
        h1I_idx_0 = 0.0F;
        h1I_idx_1 = 0.0F;
      }

      h1I_idx_0 = currentAB_idx_0 * h1I_idx_0 + currentAB_idx_1 * h1I_idx_1;
      kre_external_observer_wrappe_DW.h2D += (h1I_idx_0 -
        kre_external_observer_wrappe_DW.h2D) * h2Gain;
      l0 = (-kre_external_observer_wrapper_U.kreParams[3] * l0 *
            kre_external_observer_wrapper_U.kreParams[9] * (h1I_idx_0 -
             kre_external_observer_wrappe_DW.h2D) + (phi_idx_0 * xHat_idx_0 +
             phi_idx_1 * xHat_idx_1)) - (((kre_external_observer_wrappe_DW.h2I[0]
        * omega1_idx_0 + kre_external_observer_wrappe_DW.h2I[1] * omega1) * l0 +
        (omega1_idx_0 * omega1_idx_0 + omega1 * omega1) /
        kre_external_observer_wrapper_U.kreParams[9]) +
        kre_external_observer_wrappe_DW.h2Reg /
        kre_external_observer_wrapper_U.kreParams[9]);
      h1I_idx_0 = -kre_external_observer_wrapper_U.kreParams[11] *
        kre_external_observer_wrappe_DW.yState[0];
      h1I_idx_1 = -kre_external_observer_wrapper_U.kreParams[11] *
        kre_external_observer_wrappe_DW.yState[1];
      h2Gain = kre_external_observer_wrappe_DW.qState[0] * h1I_idx_0 +
        kre_external_observer_wrappe_DW.qState[2] * h1I_idx_1;
      omega1_idx_0 = kre_external_observer_wrappe_DW.qState[1] * h1I_idx_0 +
        kre_external_observer_wrappe_DW.qState[3] * h1I_idx_1;
      kre_external_observer_wrappe_DW.qState[0] +=
        (kre_external_observer_wrappe_DW.qState[0] - phi_idx_0 * phi_idx_0) *
        -kre_external_observer_wrapper_U.kreParams[10] *
        kre_external_observer_wrapper_U.kreParams[4];
      currentAB_idx_0 = phi_idx_1 * phi_idx_0;
      kre_external_observer_wrappe_DW.qState[1] +=
        (kre_external_observer_wrappe_DW.qState[1] - currentAB_idx_0) *
        -kre_external_observer_wrapper_U.kreParams[10] *
        kre_external_observer_wrapper_U.kreParams[4];
      kre_external_observer_wrappe_DW.qState[2] +=
        (kre_external_observer_wrappe_DW.qState[2] - currentAB_idx_0) *
        -kre_external_observer_wrapper_U.kreParams[10] *
        kre_external_observer_wrapper_U.kreParams[4];
      kre_external_observer_wrappe_DW.qState[3] +=
        (kre_external_observer_wrappe_DW.qState[3] - phi_idx_1 * phi_idx_1) *
        -kre_external_observer_wrapper_U.kreParams[10] *
        kre_external_observer_wrapper_U.kreParams[4];
      kre_external_observer_wrappe_DW.yState[0] +=
        ((kre_external_observer_wrappe_DW.yState[0] - phi_idx_0 * l0) *
         -kre_external_observer_wrapper_U.kreParams[10] + h2Gain) *
        kre_external_observer_wrapper_U.kreParams[4];
      kre_external_observer_wrappe_DW.lambdaHat[0] += (thetaRaw + h1I_idx_0) *
        kre_external_observer_wrapper_U.kreParams[4];
      kre_external_observer_wrappe_DW.yState[1] +=
        ((kre_external_observer_wrappe_DW.yState[1] - phi_idx_1 * l0) *
         -kre_external_observer_wrapper_U.kreParams[10] + omega1_idx_0) *
        kre_external_observer_wrapper_U.kreParams[4];
      kre_external_observer_wrappe_DW.lambdaHat[1] += (pllNaturalFrequency +
        h1I_idx_1) * kre_external_observer_wrapper_U.kreParams[4];
      thetaRaw = kre_rt_atan2f_snf(xHat_idx_1, xHat_idx_0);
      if (kre_external_observer_wrapper_Y.kreActiveFlux_Wb >=
          kre_external_observer_wrapper_U.kreParams[12]) {
        if (kre_external_observer_wrappe_DW.pllInitialized) {
          thetaRaw -= kre_external_observer_wrappe_DW.pllTheta;
          thetaRaw -= floorf((thetaRaw + 3.1415927F) / 6.2831855F) * 6.2831855F;
          if (thetaRaw >= 3.1415927F) {
            thetaRaw -= 6.2831855F;
          } else if (thetaRaw < -3.1415927F) {
            thetaRaw += 6.2831855F;
          }

          kre_external_observer_wrappe_DW.pllOmegaInt +=
            kre_external_observer_coefficients.pllKiTs * thetaRaw;
          kre_external_observer_wrapper_Y.kreRawOmega_radps =
            kre_external_observer_coefficients.pllKp * thetaRaw +
            kre_external_observer_wrappe_DW.pllOmegaInt;
          thetaRaw = kre_external_observer_wrapper_U.kreParams[4] *
            kre_external_observer_wrapper_Y.kreRawOmega_radps +
            kre_external_observer_wrappe_DW.pllTheta;
          kre_external_observer_wrappe_DW.pllTheta = thetaRaw - floorf((thetaRaw
            + 3.1415927F) / 6.2831855F) * 6.2831855F;
          if (kre_external_observer_wrappe_DW.pllTheta >= 3.1415927F) {
            kre_external_observer_wrappe_DW.pllTheta -= 6.2831855F;
          } else if (kre_external_observer_wrappe_DW.pllTheta < -3.1415927F) {
            kre_external_observer_wrappe_DW.pllTheta += 6.2831855F;
          }
        } else {
          kre_external_observer_wrappe_DW.pllTheta = thetaRaw;
          kre_external_observer_wrappe_DW.pllOmegaInt = 0.0F;
          kre_external_observer_wrapper_Y.kreRawOmega_radps = 0.0F;
          kre_external_observer_wrappe_DW.pllInitialized = true;
        }
      } else {
        kre_external_observer_wrapper_Y.kreRawOmega_radps =
          kre_external_observer_wrappe_DW.pllOmegaInt;
      }

      if (kre_external_observer_wrappe_DW.pllInitialized) {
        if (kre_external_observer_wrappe_DW.pllTheta < 0.0F) {
          kre_external_observer_wrapper_Y.krePositionPU =
            kre_external_observer_wrappe_DW.pllTheta / 6.2831855F + 1.0F;
        } else {
          kre_external_observer_wrapper_Y.krePositionPU =
            kre_external_observer_wrappe_DW.pllTheta / 6.2831855F;
        }

        thetaRaw = kre_external_observer_wrapper_Y.kreRawOmega_radps * 60.0F /
          (6.2831855F * kre_external_observer_wrapper_U.kreParams[8] *
           kre_external_observer_wrapper_U.kreParams[7]);
      } else {
        kre_external_observer_wrapper_Y.krePositionPU = 0.0F;
        thetaRaw = 0.0F;
      }

      kre_external_observer_wrappe_DW.speedFilt +=
        kre_external_observer_coefficients.speedGain * (thetaRaw -
        kre_external_observer_wrappe_DW.speedFilt);

      /* Outport: '<Root>/kreSpeedPU' */
      kre_external_observer_wrapper_Y.kreSpeedPU =
        kre_external_observer_wrappe_DW.speedFilt;
      if (rtIsInfF(kre_external_observer_wrapper_Y.krePositionPU) || rtIsNaNF
          (kre_external_observer_wrapper_Y.krePositionPU) || (rtIsInfF
           (kre_external_observer_wrappe_DW.speedFilt) || rtIsNaNF
           (kre_external_observer_wrappe_DW.speedFilt)) || (rtIsInfF
           (kre_external_observer_wrapper_Y.kreActiveFlux_Wb) || rtIsNaNF
           (kre_external_observer_wrapper_Y.kreActiveFlux_Wb)) || (rtIsInfF
           (kre_external_observer_wrapper_Y.kreRawOmega_radps) || rtIsNaNF
           (kre_external_observer_wrapper_Y.kreRawOmega_radps))) {
        /* Outport: '<Root>/krePositionPU' */
        kre_external_observer_wrapper_Y.krePositionPU = 0.0F;

        /* Outport: '<Root>/kreSpeedPU' */
        kre_external_observer_wrapper_Y.kreSpeedPU = 0.0F;

        /* Outport: '<Root>/kreActiveFlux_Wb' */
        kre_external_observer_wrapper_Y.kreActiveFlux_Wb = 0.0F;

        /* Outport: '<Root>/kreRawOmega_radps' */
        kre_external_observer_wrapper_Y.kreRawOmega_radps = 0.0F;

        /* Outport: '<Root>/kreStatus' */
        kre_external_observer_wrapper_Y.kreStatus = 2U;
      } else {
        /* Outport: '<Root>/kreStatus' */
        kre_external_observer_wrapper_Y.kreStatus = 1U;
      }
    }
  }

  /* Outport: '<Root>/kreFluxMagnitudePU' incorporates:
   *  MATLAB Function: '<S1>/KreExternalObserverStep'
   */
  kre_external_observer_wrapper_Y.kreFluxMagnitudePU = 0.0F;

  /* End of Outputs for SubSystem: '<Root>/KreObserverResettable' */
}

/* Model initialize function */
void kre_external_observer_wrapper_initialize(void)
{
  kre_external_observer_w_PrevZCX.KreObserverResettable_Reset_ZCE = POS_ZCSIG;
}

/* Model terminate function */
void kre_external_observer_wrapper_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
