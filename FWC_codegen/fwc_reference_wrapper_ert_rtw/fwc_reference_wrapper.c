/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: fwc_reference_wrapper.c
 *
 * Code generated for Simulink model 'fwc_reference_wrapper'.
 *
 * Model version                  : 1.4
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Wed Sep  9 15:52:31 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "fwc_reference_wrapper.h"
#include <string.h>
#include "rt_nonfinite.h"
#include <math.h>
#include "fwc_reference_wrapper_private.h"
#include "rtwtypes.h"

/* Block states (default storage) */
DW_fwc_reference_wrapper_T fwc_reference_wrapper_DW;

/* External inputs (root inport signals with default storage) */
ExtU_fwc_reference_wrapper_T fwc_reference_wrapper_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_fwc_reference_wrapper_T fwc_reference_wrapper_Y;

/* Real-time model */
static RT_MODEL_fwc_reference_wrappe_T fwc_reference_wrapper_M_;
RT_MODEL_fwc_reference_wrappe_T *const fwc_reference_wrapper_M =
  &fwc_reference_wrapper_M_;
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

/* Model step function */
void fwc_reference_wrapper_step(void)
{
  int32_T enterTicks;
  int32_T exitTicks;
  real32_T s[16];
  real32_T b_x;
  real32_T c_x;
  real32_T e_x;
  real32_T eps;
  real32_T f_x;
  real32_T hi;
  real32_T hyst;
  real32_T idLo;
  real32_T lo;
  real32_T target;
  real32_T tau;

  /* MATLAB Function: '<Root>/FwcStep' */
  memset(&s[0], 0, sizeof(real32_T) << 4U);

  /* Outport: '<Root>/IdFw' incorporates:
   *  MATLAB Function: '<Root>/FwcStep'
   */
  fwc_reference_wrapper_Y.IdFw = 0.0F;

  /* MATLAB Function: '<Root>/FwcStep' incorporates:
   *  Inport: '<Root>/IdBase'
   */
  fwc_reference_wrapper_Y.IdRef = fwc_reference_wrapper_U.IdBase;

  /* Outport: '<Root>/ReqFlt' incorporates:
   *  MATLAB Function: '<Root>/FwcStep'
   */
  fwc_reference_wrapper_Y.ReqFlt = 0.0F;

  /* Outport: '<Root>/ActFlt' incorporates:
   *  MATLAB Function: '<Root>/FwcStep'
   */
  fwc_reference_wrapper_Y.ActFlt = 0.0F;

  /* Outport: '<Root>/Active' incorporates:
   *  MATLAB Function: '<Root>/FwcStep'
   */
  fwc_reference_wrapper_Y.Active = 0U;

  /* Outport: '<Root>/Valid' incorporates:
   *  MATLAB Function: '<Root>/FwcStep'
   */
  fwc_reference_wrapper_Y.Valid = 0U;

  /* MATLAB Function: '<Root>/FwcStep' */
  fwc_reference_wrapper_Y.Saturated = 0U;
  fwc_reference_wrapper_Y.IdAtLo = 0U;

  /* Outport: '<Root>/Status' incorporates:
   *  MATLAB Function: '<Root>/FwcStep'
   */
  fwc_reference_wrapper_Y.Status = 0U;

  /* Outport: '<Root>/WeakAct' incorporates:
   *  MATLAB Function: '<Root>/FwcStep'
   */
  fwc_reference_wrapper_Y.WeakAct = 0U;

  /* MATLAB Function: '<Root>/FwcStep' incorporates:
   *  Inport: '<Root>/Eligible'
   *  Inport: '<Root>/Enable'
   *  Inport: '<Root>/FloorSatTicks'
   *  Inport: '<Root>/IdBase'
   *  Inport: '<Root>/Params'
   *  Inport: '<Root>/Reset'
   *  Inport: '<Root>/UnsatTicks'
   *  Inport: '<Root>/VdcValid'
   *  Inport: '<Root>/VutilAct'
   *  Inport: '<Root>/VutilReq'
   *  Outport: '<Root>/RecoveryAct'
   *  UnitDelay: '<Root>/State'
   */
  fwc_reference_wrapper_Y.RecoveryAct = 0U;
  if ((fwc_reference_wrapper_U.Enable != 0) && (fwc_reference_wrapper_U.Reset ==
       0)) {
    if (fwc_reference_wrapper_U.Eligible == 0) {
      /* Outport: '<Root>/Status' */
      fwc_reference_wrapper_Y.Status = 4U;
    } else if (fwc_reference_wrapper_U.VdcValid == 0) {
      /* Outport: '<Root>/Status' */
      fwc_reference_wrapper_Y.Status = 5U;
    } else if (rtIsInfF(fwc_reference_wrapper_U.VutilReq) || rtIsNaNF
               (fwc_reference_wrapper_U.VutilReq) || !(fabsf
                (fwc_reference_wrapper_U.VutilReq) < 1.0E+6F)) {
      /* Outport: '<Root>/Status' */
      fwc_reference_wrapper_Y.Status = 5U;
    } else if (rtIsInfF(fwc_reference_wrapper_U.VutilAct) || rtIsNaNF
               (fwc_reference_wrapper_U.VutilAct) || !(fabsf
                (fwc_reference_wrapper_U.VutilAct) < 1.0E+6F) ||
               (fwc_reference_wrapper_U.VutilReq < 0.0F) ||
               (fwc_reference_wrapper_U.VutilAct < 0.0F)) {
      /* Outport: '<Root>/Status' */
      fwc_reference_wrapper_Y.Status = 5U;
    } else {
      idLo = fwc_reference_wrapper_U.Params[0];
      if (rtIsInfF(fwc_reference_wrapper_U.Params[0]) || rtIsNaNF
          (fwc_reference_wrapper_U.Params[0]) || !(fabsf
           (fwc_reference_wrapper_U.Params[0]) < 1.0E+6F)) {
        idLo = 0.95F;
      }

      target = fminf(fmaxf(idLo, 0.0F), 1.0F);
      b_x = fwc_reference_wrapper_U.Params[1];
      if (rtIsInfF(fwc_reference_wrapper_U.Params[1]) || rtIsNaNF
          (fwc_reference_wrapper_U.Params[1]) || !(fabsf
           (fwc_reference_wrapper_U.Params[1]) < 1.0E+6F)) {
        b_x = 0.0F;
      }

      c_x = fwc_reference_wrapper_U.Params[2];
      if (rtIsInfF(fwc_reference_wrapper_U.Params[2]) || rtIsNaNF
          (fwc_reference_wrapper_U.Params[2]) || !(fabsf
           (fwc_reference_wrapper_U.Params[2]) < 1.0E+6F)) {
        c_x = 0.0F;
      }

      idLo = fwc_reference_wrapper_U.Params[3];
      if (rtIsInfF(fwc_reference_wrapper_U.Params[3]) || rtIsNaNF
          (fwc_reference_wrapper_U.Params[3]) || !(fabsf
           (fwc_reference_wrapper_U.Params[3]) < 1.0E+6F)) {
        idLo = 0.0F;
      }

      idLo = fminf(fmaxf(idLo, -0.3999939F), 0.0F);
      e_x = fwc_reference_wrapper_U.Params[4];
      if (rtIsInfF(fwc_reference_wrapper_U.Params[4]) || rtIsNaNF
          (fwc_reference_wrapper_U.Params[4]) || !(fabsf
           (fwc_reference_wrapper_U.Params[4]) < 1.0E+6F)) {
        e_x = 0.0F;
      }

      f_x = fwc_reference_wrapper_U.Params[5];
      if (rtIsInfF(fwc_reference_wrapper_U.Params[5]) || rtIsNaNF
          (fwc_reference_wrapper_U.Params[5]) || !(fabsf
           (fwc_reference_wrapper_U.Params[5]) < 1.0E+6F)) {
        f_x = 0.0F;
      }

      eps = fwc_reference_wrapper_U.Params[6];
      if (rtIsInfF(fwc_reference_wrapper_U.Params[6]) || rtIsNaNF
          (fwc_reference_wrapper_U.Params[6]) || !(fabsf
           (fwc_reference_wrapper_U.Params[6]) < 1.0E+6F)) {
        eps = 0.0F;
      }

      eps = fminf(fmaxf(eps, 0.0F), 1.0F);
      lo = fwc_reference_wrapper_U.Params[7];
      if (rtIsInfF(fwc_reference_wrapper_U.Params[7]) || rtIsNaNF
          (fwc_reference_wrapper_U.Params[7]) || !(fabsf
           (fwc_reference_wrapper_U.Params[7]) < 1.0E+6F)) {
        lo = 5.0F;
      }

      tau = fminf(fmaxf(lo, 0.0F), 100.0F);
      lo = fwc_reference_wrapper_U.Params[8];
      if (rtIsInfF(fwc_reference_wrapper_U.Params[8]) || rtIsNaNF
          (fwc_reference_wrapper_U.Params[8]) || !(fabsf
           (fwc_reference_wrapper_U.Params[8]) < 1.0E+6F)) {
        lo = 0.02F;
      }

      hyst = fminf(fmaxf(lo, 0.0F), fminf(0.05F, fminf(target, 1.0F - target)));
      lo = fwc_reference_wrapper_U.Params[9];
      if (rtIsInfF(fwc_reference_wrapper_U.Params[9]) || rtIsNaNF
          (fwc_reference_wrapper_U.Params[9]) || !(fabsf
           (fwc_reference_wrapper_U.Params[9]) < 1.0E+6F)) {
        lo = 2.0F;
      }

      enterTicks = (int32_T)ceilf(fminf(fmaxf(lo, 0.0F), 100.0F) * 2.0F);
      lo = fwc_reference_wrapper_U.Params[10];
      if (rtIsInfF(fwc_reference_wrapper_U.Params[10]) || rtIsNaNF
          (fwc_reference_wrapper_U.Params[10]) || !(fabsf
           (fwc_reference_wrapper_U.Params[10]) < 1.0E+6F)) {
        lo = 20.0F;
      }

      exitTicks = (int32_T)ceilf(fminf(fmaxf(lo, 0.0F), 1000.0F) * 2.0F);
      hi = target + hyst;
      lo = target - hyst;
      memcpy(&s[0], &fwc_reference_wrapper_DW.State_DSTATE[0], sizeof(real32_T) <<
             4U);
      if (fwc_reference_wrapper_DW.State_DSTATE[15] == 0.0F) {
        s[6] = 0.0F;
      } else if (fwc_reference_wrapper_DW.State_DSTATE[10] != target) {
        s[6] = 0.0F;
      } else if (fwc_reference_wrapper_DW.State_DSTATE[11] != hyst) {
        s[6] = 0.0F;
      } else if (fwc_reference_wrapper_DW.State_DSTATE[12] != enterTicks) {
        s[6] = 0.0F;
      }

      if (fwc_reference_wrapper_DW.State_DSTATE[15] == 0.0F) {
        s[7] = 0.0F;
      } else if (fwc_reference_wrapper_DW.State_DSTATE[10] != target) {
        s[7] = 0.0F;
      } else if (fwc_reference_wrapper_DW.State_DSTATE[11] != hyst) {
        s[7] = 0.0F;
      } else if (fwc_reference_wrapper_DW.State_DSTATE[13] != exitTicks) {
        s[7] = 0.0F;
      }

      if (fwc_reference_wrapper_DW.State_DSTATE[15] == 0.0F) {
        s[9] = 0.0F;
      } else if (fwc_reference_wrapper_DW.State_DSTATE[13] != exitTicks) {
        s[9] = 0.0F;
      } else if (fwc_reference_wrapper_DW.State_DSTATE[14] != eps) {
        s[9] = 0.0F;
      }

      s[10] = target;
      s[11] = hyst;
      s[12] = (real32_T)enterTicks;
      s[13] = (real32_T)exitTicks;
      s[14] = eps;
      s[15] = 1.0F;
      if ((s[4] == 0.0F) || (tau == 0.0F)) {
        s[2] = fwc_reference_wrapper_U.VutilReq;
        s[3] = fwc_reference_wrapper_U.VutilAct;
        s[4] = 1.0F;
      } else {
        target = 0.5F / (tau + 0.5F);
        s[2] += (fwc_reference_wrapper_U.VutilReq - s[2]) * target;
        s[3] += (fwc_reference_wrapper_U.VutilAct - s[3]) * target;
      }

      /* Outport: '<Root>/ReqFlt' incorporates:
       *  Inport: '<Root>/Params'
       *  Inport: '<Root>/VutilAct'
       *  Inport: '<Root>/VutilReq'
       */
      fwc_reference_wrapper_Y.ReqFlt = s[2];

      /* Outport: '<Root>/ActFlt' */
      fwc_reference_wrapper_Y.ActFlt = s[3];
      if (s[5] == 0.0F) {
        target = s[6];
        s[6] = 0.0F;
        if (s[2] > hi) {
          s[6] = fminf(target + 1.0F, (real32_T)enterTicks);
        }

        if ((s[2] > hi) && (s[6] >= enterTicks)) {
          s[5] = 1.0F;
          s[6] = 0.0F;
        }
      }

      target = 0.0F;
      if (s[5] != 0.0F) {
        if (s[2] > hi) {
          target = fminf(s[2] - hi, 1.0F);
        } else if (s[2] < lo) {
          target = fmaxf(s[2] - lo, -1.0F);
        }
      }

      hi = 0.0F;
      if (s[5] != 0.0F) {
        b_x = fminf(fmaxf(b_x, 0.0F), 100.0F) * target;
        tau = b_x + s[0];
        hi = fminf(fmaxf(tau, 0.0F), -idLo);
        if ((!(tau >= -idLo) || !(target > 0.0F)) && (!(tau <= 0.0F) || !(target
              < 0.0F))) {
          s[0] = fminf(fmaxf(fminf(fmaxf(c_x, 0.0F), 10000.0F) * 0.0005F *
                             target + s[0], idLo), -idLo);
          hi = fminf(fmaxf(b_x + s[0], 0.0F), -idLo);
        }
      }

      if (hi > s[1]) {
        s[1] += fminf(hi - s[1], fminf(fmaxf(e_x, 0.0F), 100.0F) * 0.0005F);
      } else {
        s[1] -= fminf(s[1] - hi, fminf(fmaxf(f_x, 0.0F), 100.0F) * 0.0005F);
      }

      s[1] = fminf(fmaxf(s[1], 0.0F), -idLo);

      /* Outport: '<Root>/IdFw' */
      fwc_reference_wrapper_Y.IdFw = -s[1];
      fwc_reference_wrapper_Y.IdRef = fmaxf(fminf(fminf
        (fwc_reference_wrapper_U.IdBase, 0.0F), -s[1]), idLo);
      fwc_reference_wrapper_Y.IdAtLo = (uint8_T)(truncf
        (fwc_reference_wrapper_Y.IdRef * 32768.0F - 0.5F) <= truncf(idLo *
        32768.0F));
      fwc_reference_wrapper_Y.Saturated = (uint8_T)
        (fwc_reference_wrapper_U.VutilReq - fwc_reference_wrapper_U.VutilAct >
         eps);
      if (s[8] == 0.0F) {
        s[9] = 0.0F;
        if ((fwc_reference_wrapper_Y.Saturated != 0) &&
            (fwc_reference_wrapper_Y.IdAtLo != 0) &&
            (fwc_reference_wrapper_U.FloorSatTicks >= 40U)) {
          s[8] = 1.0F;
        }
      } else {
        c_x = s[9];
        s[9] = 0.0F;
        if ((fwc_reference_wrapper_Y.Saturated == 0) &&
            (fwc_reference_wrapper_U.UnsatTicks >= 40U) && (s[2] - s[3] <= 0.5F *
             eps)) {
          s[9] = fminf(c_x + 1.0F, (real32_T)exitTicks);
          if (s[9] >= exitTicks) {
            s[8] = 0.0F;
            s[9] = 0.0F;
          }
        }
      }

      eps = s[7];
      s[7] = 0.0F;
      if ((s[5] != 0.0F) && ((s[2] < lo) && ((truncf(s[1] * 32768.0F + 0.5F) <=
             1.0F) && (s[8] == 0.0F)))) {
        s[7] = fminf(eps + 1.0F, (real32_T)exitTicks);
        if (s[7] >= exitTicks) {
          s[0] = 0.0F;
          s[1] = 0.0F;
          s[5] = 0.0F;
          s[7] = 0.0F;

          /* Outport: '<Root>/IdFw' */
          fwc_reference_wrapper_Y.IdFw = 0.0F;
          fwc_reference_wrapper_Y.IdRef = fminf(fmaxf
            (fwc_reference_wrapper_U.IdBase, idLo), 0.0F);
        }
      }

      idLo = rt_roundf_snf(s[5]);
      if (idLo < 256.0F) {
        if (idLo >= 0.0F) {
          /* Outport: '<Root>/WeakAct' */
          fwc_reference_wrapper_Y.WeakAct = (uint8_T)idLo;
        }
      } else {
        /* Outport: '<Root>/WeakAct' */
        fwc_reference_wrapper_Y.WeakAct = MAX_uint8_T;
      }

      idLo = rt_roundf_snf(s[8]);
      if (idLo < 256.0F) {
        if (idLo >= 0.0F) {
          fwc_reference_wrapper_Y.RecoveryAct = (uint8_T)idLo;
        } else {
          fwc_reference_wrapper_Y.RecoveryAct = 0U;
        }
      } else {
        fwc_reference_wrapper_Y.RecoveryAct = MAX_uint8_T;
      }

      /* Outport: '<Root>/Active' incorporates:
       *  Inport: '<Root>/FloorSatTicks'
       *  Inport: '<Root>/IdBase'
       *  Inport: '<Root>/UnsatTicks'
       *  Inport: '<Root>/VutilAct'
       *  Inport: '<Root>/VutilReq'
       */
      fwc_reference_wrapper_Y.Active = 1U;

      /* Outport: '<Root>/Valid' */
      fwc_reference_wrapper_Y.Valid = 1U;

      /* Outport: '<Root>/Status' */
      fwc_reference_wrapper_Y.Status = 1U;
      if (fwc_reference_wrapper_Y.RecoveryAct != 0) {
        /* Outport: '<Root>/Status' */
        fwc_reference_wrapper_Y.Status = 3U;
      } else if (fwc_reference_wrapper_Y.Saturated != 0) {
        /* Outport: '<Root>/Status' */
        fwc_reference_wrapper_Y.Status = 2U;
      }
    }
  }

  memcpy(&fwc_reference_wrapper_DW.State_DSTATE[0], &s[0], sizeof(real32_T) <<
         4U);
}

/* Model initialize function */
void fwc_reference_wrapper_initialize(void)
{
  /* (no initialization code required) */
}

/* Model terminate function */
void fwc_reference_wrapper_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
