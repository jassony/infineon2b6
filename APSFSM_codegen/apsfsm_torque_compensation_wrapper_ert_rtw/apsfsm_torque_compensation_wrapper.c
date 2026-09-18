/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: apsfsm_torque_compensation_wrapper.c
 *
 * Code generated for Simulink model 'apsfsm_torque_compensation_wrapper'.
 *
 * Model version                  : 1.2
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Thu Aug 27 18:26:27 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "apsfsm_torque_compensation_wrapper.h"
#include "rt_nonfinite.h"
#include <math.h>
#include "rtwtypes.h"

/* Block states (default storage) */
DW_apsfsm_torque_compensation_wrapper_T apsfsm_torque_compensation_wrapper_DW;

/* External inputs (root inport signals with default storage) */
ExtU_apsfsm_torque_compensation_wrapper_T apsfsm_torque_compensation_wrapper_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_apsfsm_torque_compensation_wrapper_T apsfsm_torque_compensation_wrapper_Y;

/* Real-time model */
static RT_MODEL_apsfsm_torque_compensation_wrapper_T
  apsfsm_torque_compensation_wrapper_M_;
RT_MODEL_apsfsm_torque_compensation_wrapper_T *const
  apsfsm_torque_compensation_wrapper_M = &apsfsm_torque_compensation_wrapper_M_;

/* Model step function */
void apsfsm_torque_compensation_wrapper_step(void)
{
  int32_T rtb_diagnostics_idx_1;
  int32_T rtb_diagnostics_idx_2;
  real32_T candidateMagnitude;
  real32_T coefficientMagnitude;
  real32_T covariance;
  real32_T iqRaw;
  real32_T minimumCovariance;
  real32_T phase;
  real32_T resetState_idx_0;
  real32_T resetState_idx_1;
  real32_T resetState_idx_2;
  real32_T resetState_idx_3;
  real32_T thetaMech;
  boolean_T controlDiscrete;

  /* MATLAB Function: '<Root>/APSFSM_DiscreteStep' */
  resetState_idx_0 = 0.0F;

  /* Outport: '<Root>/Iq_Raw_PU' incorporates:
   *  MATLAB Function: '<Root>/APSFSM_DiscreteStep'
   */
  apsfsm_torque_compensation_wrapper_Y.Iq_Raw_PU = 0.0F;

  /* MATLAB Function: '<Root>/APSFSM_DiscreteStep' */
  resetState_idx_1 = 0.0F;
  rtb_diagnostics_idx_1 = 0;
  resetState_idx_2 = 0.0F;
  rtb_diagnostics_idx_2 = 0;
  resetState_idx_3 = 0.0F;

  /* Outport: '<Root>/Speed_Error_PU' incorporates:
   *  MATLAB Function: '<Root>/APSFSM_DiscreteStep'
   */
  apsfsm_torque_compensation_wrapper_Y.Speed_Error_PU = 0.0F;

  /* MATLAB Function: '<Root>/APSFSM_DiscreteStep' incorporates:
   *  Inport: '<Root>/Control'
   *  Inport: '<Root>/Parameters'
   *  Inport: '<Root>/Speed_Feedback_PU'
   *  Inport: '<Root>/Speed_Reference_PU'
   *  UnitDelay: '<Root>/APSFSM_State'
   */
  if (rtIsInfF(apsfsm_torque_compensation_wrapper_U.Parameters[0]) || rtIsNaNF
      (apsfsm_torque_compensation_wrapper_U.Parameters[0]) || (rtIsInfF
       (apsfsm_torque_compensation_wrapper_U.Parameters[1]) || rtIsNaNF
       (apsfsm_torque_compensation_wrapper_U.Parameters[1]) || (rtIsInfF
        (apsfsm_torque_compensation_wrapper_U.Parameters[2]) || rtIsNaNF
        (apsfsm_torque_compensation_wrapper_U.Parameters[2]) || (rtIsInfF
         (apsfsm_torque_compensation_wrapper_U.Parameters[3]) || rtIsNaNF
         (apsfsm_torque_compensation_wrapper_U.Parameters[3]) || (rtIsInfF
          (apsfsm_torque_compensation_wrapper_U.Parameters[4]) || rtIsNaNF
          (apsfsm_torque_compensation_wrapper_U.Parameters[4]))))) ||
      (!(apsfsm_torque_compensation_wrapper_U.Parameters[0] > 0.0F) ||
       (!(apsfsm_torque_compensation_wrapper_U.Parameters[1] > 0.0F) ||
        (!(apsfsm_torque_compensation_wrapper_U.Parameters[3] > 0.0F) ||
         (!(apsfsm_torque_compensation_wrapper_U.Parameters[3] < 1.0F) ||
          !(apsfsm_torque_compensation_wrapper_U.Parameters[4] > 0.0F)))))) {
    rtb_diagnostics_idx_2 = 4;
  } else {
    minimumCovariance = 0.5F * apsfsm_torque_compensation_wrapper_U.Parameters[1]
      * apsfsm_torque_compensation_wrapper_U.Parameters[1];
    if (rtIsInfF(minimumCovariance) || rtIsNaNF(minimumCovariance)) {
      rtb_diagnostics_idx_2 = 4;
    } else if (minimumCovariance <= 0.0F) {
      rtb_diagnostics_idx_2 = 4;
    } else {
      resetState_idx_3 = minimumCovariance;
      if ((apsfsm_torque_compensation_wrapper_U.Control[0] == 0.0F) ||
          (apsfsm_torque_compensation_wrapper_U.Control[0] == 1.0F)) {
        if ((apsfsm_torque_compensation_wrapper_U.Control[1] == 0.0F) ||
            (apsfsm_torque_compensation_wrapper_U.Control[1] == 1.0F)) {
          controlDiscrete = ((apsfsm_torque_compensation_wrapper_U.Control[2] ==
                              0.0F) ||
                             (apsfsm_torque_compensation_wrapper_U.Control[2] ==
                              1.0F));
        } else {
          controlDiscrete = false;
        }
      } else {
        controlDiscrete = false;
      }

      if (rtIsInfF(apsfsm_torque_compensation_wrapper_U.Control[0]) || rtIsNaNF
          (apsfsm_torque_compensation_wrapper_U.Control[0]) || (rtIsInfF
           (apsfsm_torque_compensation_wrapper_U.Control[1]) || rtIsNaNF
           (apsfsm_torque_compensation_wrapper_U.Control[1]) || (rtIsInfF
            (apsfsm_torque_compensation_wrapper_U.Control[2]) || rtIsNaNF
            (apsfsm_torque_compensation_wrapper_U.Control[2]))) ||
          !controlDiscrete) {
        rtb_diagnostics_idx_2 = 5;
      } else if (rtIsInfF
                 (apsfsm_torque_compensation_wrapper_U.Speed_Reference_PU) ||
                 rtIsNaNF
                 (apsfsm_torque_compensation_wrapper_U.Speed_Reference_PU)) {
        rtb_diagnostics_idx_2 = 5;
      } else if (rtIsInfF(apsfsm_torque_compensation_wrapper_U.Speed_Feedback_PU)
                 || rtIsNaNF
                 (apsfsm_torque_compensation_wrapper_U.Speed_Feedback_PU)) {
        rtb_diagnostics_idx_2 = 5;
      } else if (!(apsfsm_torque_compensation_wrapper_U.Control[2] == 1.0F) &&
                 !(apsfsm_torque_compensation_wrapper_U.Control[0] == 0.0F)) {
        covariance = apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[3];
        controlDiscrete = (!rtIsInfF
                           (apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE
                            [0]) && !rtIsNaNF
                           (apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE
                            [0]) && (!rtIsInfF
          (apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[1]) &&
          !rtIsNaNF(apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[1])
          && (!rtIsInfF
              (apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[2]) &&
              !rtIsNaNF
              (apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[2]) &&
              (!rtIsInfF
               (apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[3]) &&
               !rtIsNaNF
               (apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[3])))));
        if (controlDiscrete &&
            (apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[3] >=
             0.0F) &&
            (apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[3] <
             minimumCovariance)) {
          covariance = minimumCovariance;
        }

        coefficientMagnitude = sqrtf
          (apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[0] *
           apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[0] +
           apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[1] *
           apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[1]);
        if (!controlDiscrete || (!(covariance >= minimumCovariance) || (rtIsInfF
              (coefficientMagnitude) || rtIsNaNF(coefficientMagnitude) ||
              !(coefficientMagnitude <=
                apsfsm_torque_compensation_wrapper_U.Parameters[4] * 1.00001F))))
        {
          rtb_diagnostics_idx_2 = 5;
        } else {
          thetaMech = apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE
            [2] - floorf
            (apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[2] /
             6.2831855F) * 6.2831855F;
          if (thetaMech >= 6.2831855F) {
            thetaMech = 0.0F;
          } else if (thetaMech < 0.0F) {
            thetaMech += 6.2831855F;
          }

          coefficientMagnitude =
            apsfsm_torque_compensation_wrapper_U.Speed_Reference_PU -
            apsfsm_torque_compensation_wrapper_U.Speed_Feedback_PU;
          iqRaw = apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[0] *
            sinf(thetaMech) +
            apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[1] * cosf
            (thetaMech);
          thetaMech += 0.0005F *
            apsfsm_torque_compensation_wrapper_U.Parameters[0] *
            apsfsm_torque_compensation_wrapper_U.Speed_Feedback_PU;
          thetaMech -= floorf(thetaMech / 6.2831855F) * 6.2831855F;
          if (thetaMech >= 6.2831855F) {
            thetaMech = 0.0F;
          } else if (thetaMech < 0.0F) {
            thetaMech += 6.2831855F;
          }

          if (apsfsm_torque_compensation_wrapper_U.Control[1] == 1.0F) {
            minimumCovariance =
              apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[0];
            phase = apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[1];
          } else {
            covariance = apsfsm_torque_compensation_wrapper_U.Parameters[3] *
              covariance + minimumCovariance;
            if (covariance < minimumCovariance) {
              covariance = minimumCovariance;
            }

            minimumCovariance =
              (apsfsm_torque_compensation_wrapper_U.Parameters[2] + 3.1415927F)
              - floorf((apsfsm_torque_compensation_wrapper_U.Parameters[2] +
                        3.1415927F) / 6.2831855F) * 6.2831855F;
            if (minimumCovariance >= 6.2831855F) {
              minimumCovariance = 0.0F;
            } else if (minimumCovariance < 0.0F) {
              minimumCovariance += 6.2831855F;
            }

            phase = (minimumCovariance - 3.1415927F) + thetaMech;
            minimumCovariance = apsfsm_torque_compensation_wrapper_U.Parameters
              [1] * sinf(phase) * coefficientMagnitude / covariance +
              apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[0];
            phase = apsfsm_torque_compensation_wrapper_U.Parameters[1] * cosf
              (phase) * coefficientMagnitude / covariance +
              apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[1];
            candidateMagnitude = sqrtf(minimumCovariance * minimumCovariance +
              phase * phase);
            if (candidateMagnitude >
                apsfsm_torque_compensation_wrapper_U.Parameters[4]) {
              candidateMagnitude =
                apsfsm_torque_compensation_wrapper_U.Parameters[4] /
                candidateMagnitude;
              minimumCovariance *= candidateMagnitude;
              phase *= candidateMagnitude;
            }
          }

          if (rtIsInfF(iqRaw) || rtIsNaNF(iqRaw) || (rtIsInfF
               (coefficientMagnitude) || rtIsNaNF(coefficientMagnitude) ||
               (rtIsInfF(thetaMech) || rtIsNaNF(thetaMech) || (rtIsInfF
                 (minimumCovariance) || rtIsNaNF(minimumCovariance) || (rtIsInfF
                  (phase) || rtIsNaNF(phase) || (rtIsInfF(covariance) ||
                   rtIsNaNF(covariance))))))) {
            rtb_diagnostics_idx_2 = 5;
          } else {
            resetState_idx_0 = minimumCovariance;
            resetState_idx_1 = phase;
            resetState_idx_2 = thetaMech;
            resetState_idx_3 = covariance;

            /* Outport: '<Root>/Iq_Raw_PU' */
            apsfsm_torque_compensation_wrapper_Y.Iq_Raw_PU = iqRaw;
            rtb_diagnostics_idx_1 = 1;
            rtb_diagnostics_idx_2 = 2;

            /* Outport: '<Root>/Speed_Error_PU' */
            apsfsm_torque_compensation_wrapper_Y.Speed_Error_PU =
              coefficientMagnitude;
          }
        }
      }
    }
  }

  apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[0] =
    resetState_idx_0;
  apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[1] =
    resetState_idx_1;
  apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[2] =
    resetState_idx_2;
  apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[3] =
    resetState_idx_3;

  /* Outport: '<Root>/BHat_PU' incorporates:
   *  UnitDelay: '<Root>/APSFSM_State'
   */
  apsfsm_torque_compensation_wrapper_Y.BHat_PU =
    apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[0];

  /* Outport: '<Root>/CHat_PU' incorporates:
   *  UnitDelay: '<Root>/APSFSM_State'
   */
  apsfsm_torque_compensation_wrapper_Y.CHat_PU =
    apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[1];

  /* Outport: '<Root>/Theta_rad' incorporates:
   *  UnitDelay: '<Root>/APSFSM_State'
   */
  apsfsm_torque_compensation_wrapper_Y.Theta_rad =
    apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[2];

  /* Outport: '<Root>/Covariance' incorporates:
   *  UnitDelay: '<Root>/APSFSM_State'
   */
  apsfsm_torque_compensation_wrapper_Y.Covariance =
    apsfsm_torque_compensation_wrapper_DW.APSFSM_State_DSTATE[3];

  /* Outport: '<Root>/Valid_u8' incorporates:
   *  DataTypeConversion: '<Root>/Valid_To_Uint8'
   */
  apsfsm_torque_compensation_wrapper_Y.Valid_u8 = (uint8_T)rtb_diagnostics_idx_1;

  /* Outport: '<Root>/Status_u8' incorporates:
   *  DataTypeConversion: '<Root>/Status_To_Uint8'
   */
  apsfsm_torque_compensation_wrapper_Y.Status_u8 = (uint8_T)
    rtb_diagnostics_idx_2;
}

/* Model initialize function */
void apsfsm_torque_compensation_wrapper_initialize(void)
{
  /* (no initialization code required) */
}

/* Model terminate function */
void apsfsm_torque_compensation_wrapper_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
