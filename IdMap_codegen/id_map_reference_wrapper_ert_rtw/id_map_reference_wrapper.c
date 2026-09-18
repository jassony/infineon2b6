/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: id_map_reference_wrapper.c
 *
 * Code generated for Simulink model 'id_map_reference_wrapper'.
 *
 * Model version                  : 1.6
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Thu Aug 20 15:55:20 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "id_map_reference_wrapper.h"
#include <math.h>
#include "rtwtypes.h"
#include "id_map_reference_wrapper_private.h"

/* Exported block parameters */
real32_T Cal_IdMap_Iq_A_f32[11] = { 0.0F, 5.0F, 10.0F, 15.0F, 20.0F, 25.0F,
  30.0F, 35.0F, 40.0F, 45.0F, 50.0F } ;/* Variable: Cal_IdMap_Iq_A_f32
                                        * Referenced by: '<Root>/IdMap_2D'
                                        * Id map q-axis current breakpoints in A.
                                        */

real32_T Cal_IdMap_Spd_rpm_f32[11] = { 0.0F, 1000.0F, 2000.0F, 3000.0F, 4000.0F,
  5000.0F, 6000.0F, 7000.0F, 8000.0F, 9000.0F, 10000.0F } ;/* Variable: Cal_IdMap_Spd_rpm_f32
                                                            * Referenced by: '<Root>/IdMap_2D'
                                                            * Id map speed breakpoints in rpm.
                                                            */

real32_T Cal_IdMap_Table_A_f32[121] = { 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
  0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
  0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
  0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
  0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
  0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
  0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
  0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
  0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
  0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F } ;/* Variable: Cal_IdMap_Table_A_f32
                                                                 * Referenced by: '<Root>/IdMap_2D'
                                                                 * Id map d-axis current output table in A.
                                                                 */

/* External inputs (root inport signals with default storage) */
ExtU_id_map_reference_wrapper_T id_map_reference_wrapper_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_id_map_reference_wrapper_T id_map_reference_wrapper_Y;

/* Real-time model */
static RT_MODEL_id_map_reference_wra_T id_map_reference_wrapper_M_;
RT_MODEL_id_map_reference_wra_T *const id_map_reference_wrapper_M =
  &id_map_reference_wrapper_M_;
real32_T look2_iflf_binlcpw(real32_T u0, real32_T u1, const real32_T bp0[],
  const real32_T bp1[], const real32_T table[], const uint32_T maxIndex[2],
  uint32_T stride)
{
  real32_T fractions[2];
  real32_T frac;
  real32_T yL_0d0;
  real32_T yL_0d1;
  uint32_T bpIndices[2];
  uint32_T bpIdx;
  uint32_T iLeft;
  uint32_T iRght;

  /* Column-major Lookup 2-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = 0.0F;
  } else if (u0 < bp0[maxIndex[0U]]) {
    /* Binary Search */
    bpIdx = maxIndex[0U] >> 1U;
    iLeft = 0U;
    iRght = maxIndex[0U];
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex[0U] - 1U;
    frac = 1.0F;
  }

  fractions[0U] = frac;
  bpIndices[0U] = iLeft;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u1 <= bp1[0U]) {
    iLeft = 0U;
    frac = 0.0F;
  } else if (u1 < bp1[maxIndex[1U]]) {
    /* Binary Search */
    bpIdx = maxIndex[1U] >> 1U;
    iLeft = 0U;
    iRght = maxIndex[1U];
    while (iRght - iLeft > 1U) {
      if (u1 < bp1[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u1 - bp1[iLeft]) / (bp1[iLeft + 1U] - bp1[iLeft]);
  } else {
    iLeft = maxIndex[1U] - 1U;
    frac = 1.0F;
  }

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'portable wrapping'
   */
  bpIdx = iLeft * stride + bpIndices[0U];
  yL_0d0 = table[bpIdx];
  yL_0d0 += (table[bpIdx + 1U] - yL_0d0) * fractions[0U];
  bpIdx += stride;
  yL_0d1 = table[bpIdx];
  return (((table[bpIdx + 1U] - yL_0d1) * fractions[0U] + yL_0d1) - yL_0d0) *
    frac + yL_0d0;
}

/* Model step function */
void id_map_reference_wrapper_step(void)
{
  /* Outport: '<Root>/Id_ref_A' incorporates:
   *  Abs: '<Root>/AbsIq_A'
   *  Abs: '<Root>/AbsSpeed_rpm'
   *  Inport: '<Root>/Iq_A'
   *  Inport: '<Root>/Speed_rpm'
   *  Lookup_n-D: '<Root>/IdMap_2D'
   */
  id_map_reference_wrapper_Y.Id_ref_A = look2_iflf_binlcpw(fabsf
    (id_map_reference_wrapper_U.Speed_rpm), fabsf
    (id_map_reference_wrapper_U.Iq_A), Cal_IdMap_Spd_rpm_f32, Cal_IdMap_Iq_A_f32,
    Cal_IdMap_Table_A_f32, id_map_reference_wrapper_P.IdMap_2D_maxIndex, 11U);
}

/* Model initialize function */
void id_map_reference_wrapper_initialize(void)
{
  /* (no initialization code required) */
}

/* Model terminate function */
void id_map_reference_wrapper_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
