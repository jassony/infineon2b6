/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: pmsm_torque_estimator_wrapper.c
 *
 * Code generated for Simulink model 'pmsm_torque_estimator_wrapper'.
 *
 * Model version                  : 1.3
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Fri Jul 24 16:22:23 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "pmsm_torque_estimator_wrapper.h"
#include "../mtpa_project_parameters.h"
#include "rtwtypes.h"

/* External inputs (root inport signals with default storage) */
ExtU_pmsm_torque_estimator_wr_T pmsm_torque_estimator_wrapper_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_pmsm_torque_estimator_wr_T pmsm_torque_estimator_wrapper_Y;

/* Real-time model */
static RT_MODEL_pmsm_torque_estimato_T pmsm_torque_estimator_wrappe_M_;
RT_MODEL_pmsm_torque_estimato_T *const pmsm_torque_estimator_wrappe_M =
  &pmsm_torque_estimator_wrappe_M_;

static int16_T MTPA_ProjectParameters_toQ15(real32_T value)
{
  real32_T scaledValue = value * 32768.0F;

  if (scaledValue > 32767.0F) {
    return MAX_int16_T;
  }

  if (scaledValue < -32768.0F) {
    return MIN_int16_T;
  }

  return (int16_T)scaledValue;
}

/* Model step function */
void pmsm_torque_estimator_wrapper_step(void)
{
  const MTPA_ProjectParameters *parameters = MTPA_ProjectParameters_get();
  real32_T directCurrentA;
  real32_T quadratureCurrentA;
  real32_T electricalSpeedRadps;
  real32_T torqueNm;
  real32_T powerW;

  if (parameters->valid == 0u) {
    pmsm_torque_estimator_wrapper_Y.Te_q15 = 0;
    pmsm_torque_estimator_wrapper_Y.Pe_q15 = 0;
    return;
  }

  directCurrentA = 3.0517578E-5F * (real32_T)pmsm_torque_estimator_wrapper_U.Id_q15
    * parameters->baseCurrentA;
  quadratureCurrentA = 3.0517578E-5F * (real32_T)pmsm_torque_estimator_wrapper_U.Iq_q15
    * parameters->baseCurrentA;
  electricalSpeedRadps = 3.0517578E-5F * (real32_T)pmsm_torque_estimator_wrapper_U.Speed_q15
    * parameters->baseElectricalSpeedRadps;

  /* Project C calibration replaces the fixed model motor constants. */
  torqueNm = 1.5F * (real32_T)parameters->polePairs
    * (parameters->permanentMagnetFluxWb
      + (parameters->directInductanceH - parameters->quadratureInductanceH) * directCurrentA)
    * quadratureCurrentA;
  powerW = torqueNm * electricalSpeedRadps / (real32_T)parameters->polePairs;

  pmsm_torque_estimator_wrapper_Y.Te_q15 = MTPA_ProjectParameters_toQ15(
    torqueNm / parameters->baseTorqueNm);
  pmsm_torque_estimator_wrapper_Y.Pe_q15 = MTPA_ProjectParameters_toQ15(
    powerW / parameters->basePowerW);
}

/* Model initialize function */
void pmsm_torque_estimator_wrapper_initialize(void)
{
  /* (no initialization code required) */
}

/* Model terminate function */
void pmsm_torque_estimator_wrapper_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
