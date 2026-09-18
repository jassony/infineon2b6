/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: mtpa_reference_wrapper.c
 *
 * Code generated for Simulink model 'mtpa_reference_wrapper'.
 *
 * Model version                  : 1.4
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Fri Jul 24 16:22:32 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "mtpa_reference_wrapper.h"
#include "../mtpa_project_parameters.h"
#include <math.h>
#include "rtwtypes.h"

/* Block signals (default storage) */
B_mtpa_reference_wrapper_T mtpa_reference_wrapper_B;

/* External inputs (root inport signals with default storage) */
ExtU_mtpa_reference_wrapper_T mtpa_reference_wrapper_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_mtpa_reference_wrapper_T mtpa_reference_wrapper_Y;

/* Real-time model */
static RT_MODEL_mtpa_reference_wrapp_T mtpa_reference_wrapper_M_;
RT_MODEL_mtpa_reference_wrapp_T *const mtpa_reference_wrapper_M =
  &mtpa_reference_wrapper_M_;

/* Fixed-parameter generated reference retained for model traceability. */
void mtpa_reference_wrapper_model_step(void)
{
  real32_T rtb_Abs;
  real32_T rtb_Abs_f;
  real32_T rtb_Abs_ka;
  real32_T rtb_Id_ref_q15_limit;
  real32_T rtb_Id_ref_q15_limit_p;
  real32_T rtb_Iq_ref_q15_limit;
  real32_T rtb_Saturation2;
  real32_T rtb_Sqrt;
  real32_T rtb_we;

  /* Gain: '<Root>/Tref_q15_to_pu' incorporates:
   *  DataTypeConversion: '<Root>/Tref_q15_to_single'
   *  Inport: '<Root>/Tref_q15'
   */
  rtb_Iq_ref_q15_limit = 3.0517578E-5F * (real32_T)
    mtpa_reference_wrapper_U.Tref_q15;

  /* Abs: '<S3>/Abs' */
  rtb_Abs = fabsf(rtb_Iq_ref_q15_limit);

  /* Product: '<S9>/Square' incorporates:
   *  Saturate: '<S8>/Saturation'
   */
  rtb_Abs_ka = rtb_Abs * rtb_Abs;

  /* Sum: '<S9>/Sum2' incorporates:
   *  Abs: '<S9>/Abs'
   *  Constant: '<S9>/term1'
   *  Constant: '<S9>/term2'
   *  Gain: '<S9>/term3'
   *  Product: '<S9>/Square1'
   *  Sqrt: '<S9>/Sqrt'
   *  Sum: '<S9>/Sum3'
   */
  rtb_Saturation2 = 0.14722988F - sqrtf(fabsf(-0.5F * rtb_Abs_ka - 0.021676637F));

  /* Saturate: '<S13>/Saturation2' */
  if (rtb_Saturation2 > 0.0F) {
    rtb_Saturation2 = 0.0F;
  }

  /* End of Saturate: '<S13>/Saturation2' */

  /* Abs: '<S1>/Abs' incorporates:
   *  DataTypeConversion: '<Root>/Speed_q15_to_single'
   *  Gain: '<Root>/Speed_q15_to_pu'
   *  Inport: '<Root>/Speed_q15'
   */
  rtb_Abs_f = fabsf(1.05582885E-5F * (real32_T)
                    mtpa_reference_wrapper_U.Speed_q15);

  /* Gain: '<S8>/Gain1' */
  rtb_we = 4.0F * rtb_Abs_f;

  /* Sum: '<S41>/Sum1' incorporates:
   *  Gain: '<S41>/FluxPM'
   *  Gain: '<S41>/Ld'
   *  Product: '<S41>/Product1'
   */
  rtb_Id_ref_q15_limit = rtb_Saturation2 * rtb_we * 0.267372F + 0.14433888F *
    rtb_we;

  /* Sqrt: '<S13>/Sqrt' incorporates:
   *  Abs: '<S13>/Abs'
   *  Product: '<S13>/Product1'
   *  Sum: '<S13>/Sum2'
   */
  rtb_Sqrt = sqrtf(fabsf(rtb_Abs_ka - rtb_Saturation2 * rtb_Saturation2));

  /* Gain: '<S41>/Lq' incorporates:
   *  Product: '<S41>/Product'
   */
  rtb_Id_ref_q15_limit_p = rtb_we * rtb_Sqrt * 0.51246303F;

  /* If: '<S8>/If' incorporates:
   *  Gain: '<S41>/2'
   *  Product: '<S41>/Product2'
   *  Product: '<S41>/Product3'
   *  RelationalOperator: '<S12>/GreaterThan'
   *  Sqrt: '<S41>/Sqrt'
   *  Sum: '<S41>/Sum3'
   *  Switch: '<S21>/Switch'
   */
  if (!(sqrtf(rtb_Id_ref_q15_limit * rtb_Id_ref_q15_limit +
              rtb_Id_ref_q15_limit_p * rtb_Id_ref_q15_limit_p) * 0.0017320508F >=
        mtpa_reference_wrapper_ConstB.Subtract)) {
    /* Outputs for IfAction SubSystem: '<S8>/MTPA condition' incorporates:
     *  ActionPort: '<S11>/Action Port'
     */
    /* SignalConversion generated from: '<S11>/id' incorporates:
     *  Merge: '<S8>/Merge'
     */
    mtpa_reference_wrapper_B.Merge[0] = rtb_Saturation2;

    /* SignalConversion generated from: '<S11>/iq' incorporates:
     *  Merge: '<S8>/Merge'
     */
    mtpa_reference_wrapper_B.Merge[1] = rtb_Sqrt;

    /* End of Outputs for SubSystem: '<S8>/MTPA condition' */
  } else {
    /* Outputs for IfAction SubSystem: '<S8>/FW condition' incorporates:
     *  ActionPort: '<S10>/Action Port'
     */
    if (rtb_Abs_f != 0.0F) {
      /* Switch: '<S21>/Switch' incorporates:
       *  Math: '<S21>/Math Function'
       *
       * About '<S21>/Math Function':
       *  Operator: reciprocal
       */
      rtb_Saturation2 = 1.0F / rtb_Abs_f;
    } else {
      /* Switch: '<S21>/Switch' incorporates:
       *  Constant: '<S21>/Constant'
       */
      rtb_Saturation2 = 1.0F;
    }

    /* Gain: '<S16>/  ' incorporates:
     *  Product: '<S16>/Square1'
     */
    rtb_Saturation2 = rtb_Saturation2 * mtpa_reference_wrapper_ConstB.Subtract1 *
      0.5718405F;

    /* Sum: '<S16>/Sum3' incorporates:
     *  Constant: '<S16>/term2'
     *  Constant: '<S16>/term3'
     *  Gain: '<S16>/ '
     *  Product: '<S16>/Square'
     *  Product: '<S16>/Square2'
     *  Sum: '<S16>/Sum1'
     */
    rtb_Saturation2 = 0.040769774F - (-1.4830285F - (-(rtb_Saturation2 *
      rtb_Saturation2)));

    /* If: '<S22>/If' incorporates:
     *  Constant: '<S38>/Constant'
     *  Gain: '<S39>/zero'
     *  RelationalOperator: '<S38>/Compare'
     *  Sqrt: '<S40>/Sqrt'
     */
    if (rtb_Saturation2 < 0.0F) {
      /* Outputs for IfAction SubSystem: '<S22>/If negative value' incorporates:
       *  ActionPort: '<S39>/Action Port'
       */
      rtb_Saturation2 *= 0.0F;

      /* End of Outputs for SubSystem: '<S22>/If negative value' */
    } else {
      /* Outputs for IfAction SubSystem: '<S22>/If positive value' incorporates:
       *  ActionPort: '<S40>/Action Port'
       */
      rtb_Saturation2 = sqrtf(rtb_Saturation2);

      /* End of Outputs for SubSystem: '<S22>/If positive value' */
    }

    /* Outputs for IfAction SubSystem: '<S20>/D or Q Axis Priority' incorporates:
     *  ActionPort: '<S24>/Action Port'
     */
    /* If: '<S20>/If' incorporates:
     *  Constant: '<S16>/term1'
     *  If: '<S22>/If'
     *  Product: '<S32>/Product'
     *  RelationalOperator: '<S33>/UpperRelop'
     *  Sum: '<S16>/Sum2'
     *  Sum: '<S32>/Sum'
     *  Switch: '<S33>/Switch'
     *  Switch: '<S33>/Switch2'
     */
    if (0.20191526F - rtb_Saturation2 < -1.0F) {
      rtb_Saturation2 = -1.0F;
    } else {
      rtb_Saturation2 = 0.20191526F - rtb_Saturation2;
    }

    rtb_Abs_f = 1.0F - rtb_Saturation2 * rtb_Saturation2;

    /* If: '<S32>/If' incorporates:
     *  If: '<S20>/If'
     *  Product: '<S32>/Product2'
     *  RelationalOperator: '<S32>/Relational Operator'
     *  Switch: '<S34>/Switch1'
     */
    if (rtb_Abs_f >= rtb_Abs_ka) {
      /* Outputs for IfAction SubSystem: '<S32>/passThrough' incorporates:
       *  ActionPort: '<S35>/Action Port'
       */
      /* Switch: '<S24>/Switch1' incorporates:
       *  Saturate: '<S8>/Saturation'
       *  SignalConversion generated from: '<S35>/ref2'
       */
      mtpa_reference_wrapper_B.Merge[1] = rtb_Abs;

      /* End of Outputs for SubSystem: '<S32>/passThrough' */
    } else {
      /* Outputs for IfAction SubSystem: '<S32>/limitRef2' incorporates:
       *  ActionPort: '<S34>/Action Port'
       */
      if (!(rtb_Abs_f > 0.0F)) {
        /* Switch: '<S34>/Switch1' incorporates:
         *  Constant: '<S34>/Constant'
         */
        rtb_Abs_f = 0.0F;
      }

      /* Switch: '<S24>/Switch1' incorporates:
       *  Sqrt: '<S34>/Sqrt'
       *  Switch: '<S34>/Switch'
       *  Switch: '<S34>/Switch1'
       */
      mtpa_reference_wrapper_B.Merge[1] = sqrtf(rtb_Abs_f);

      /* End of Outputs for SubSystem: '<S32>/limitRef2' */
    }

    /* End of If: '<S32>/If' */

    /* If: '<S20>/If' incorporates:
     *  Switch: '<S24>/Switch1'
     */
    mtpa_reference_wrapper_B.Merge[0] = rtb_Saturation2;

    /* End of Outputs for SubSystem: '<S20>/D or Q Axis Priority' */

    /* Outputs for IfAction SubSystem: '<S10>/If Action Subsystem1' incorporates:
     *  ActionPort: '<S15>/Action Port'
     */
    /* If: '<S10>/If' incorporates:
     *  Constant: '<S23>/Constant'
     *  Gain: '<S23>/Gain'
     *  Product: '<S19>/Divide'
     *  Sum: '<S19>/Add1'
     *  Sum: '<S23>/Add'
     */
    rtb_Abs_ka = -1.698025F * mtpa_reference_wrapper_B.Merge[0] + 1.0F;

    /* End of Outputs for SubSystem: '<S10>/If Action Subsystem1' */

    /* Product: '<S23>/Divide' incorporates:
     *  Sum: '<S23>/Add'
     */
    rtb_Abs = 1.0F / rtb_Abs_ka * rtb_Abs * mtpa_reference_wrapper_ConstB.Add1;

    /* If: '<S10>/If' incorporates:
     *  RelationalOperator: '<S10>/GreaterThan'
     */
    if (!(rtb_Abs > mtpa_reference_wrapper_B.Merge[1])) {
      /* Outputs for IfAction SubSystem: '<S10>/If Action Subsystem1' incorporates:
       *  ActionPort: '<S15>/Action Port'
       */
      /* Product: '<S17>/Divide' incorporates:
       *  Gain: '<S10>/Gain'
       *  Gain: '<S17>/2'
       */
      rtb_we = mtpa_reference_wrapper_ConstB.Subtract_k / (rtb_we *
        0.0017320508F);

      /* UnaryMinus: '<S18>/Unary Minus' incorporates:
       *  Product: '<S18>/Divide'
       *  Sum: '<S18>/Sum1'
       */
      rtb_Id_ref_q15_limit = -(rtb_Abs / (mtpa_reference_wrapper_B.Merge[0] -
        0.5889195F));

      /* Sum: '<S18>/constant output' incorporates:
       *  Product: '<S18>/Product'
       */
      rtb_Saturation2 = rtb_Abs - rtb_Id_ref_q15_limit *
        mtpa_reference_wrapper_B.Merge[0];

      /* Sum: '<S17>/A' incorporates:
       *  Constant: '<S17>/ld2_lq2'
       *  Product: '<S17>/m2'
       */
      rtb_Abs_f = rtb_Id_ref_q15_limit * rtb_Id_ref_q15_limit + 0.27221173F;

      /* Sum: '<S17>/B' incorporates:
       *  Constant: '<S17>/fluxPMLd_Lq2'
       *  Product: '<S17>/mc'
       */
      rtb_Id_ref_q15_limit = rtb_Id_ref_q15_limit * rtb_Saturation2 +
        0.14695156F;

      /* Product: '<S17>/Divide1' incorporates:
       *  Abs: '<S17>/Abs'
       *  Constant: '<S17>/fluxPM2_Lq2'
       *  Product: '<S17>/AC'
       *  Product: '<S17>/B2'
       *  Product: '<S17>/Product'
       *  Product: '<S17>/c2'
       *  Sqrt: '<S17>/Sqrt'
       *  Sum: '<S17>/C'
       *  Sum: '<S17>/Subtract4'
       *  Sum: '<S17>/Subtract5'
       */
      rtb_Abs_f = (sqrtf(fabsf(rtb_Id_ref_q15_limit * rtb_Id_ref_q15_limit -
        ((rtb_Saturation2 * rtb_Saturation2 + 0.07933075F) - rtb_we * rtb_we) *
        rtb_Abs_f)) - rtb_Id_ref_q15_limit) / rtb_Abs_f;

      /* Merge: '<S8>/Merge' incorporates:
       *  Constant: '<S19>/Constant'
       *  Gain: '<S19>/Gain'
       *  Product: '<S19>/Divide'
       *  SignalConversion generated from: '<S15>/Out1'
       *  Sum: '<S19>/Add'
       */
      mtpa_reference_wrapper_B.Merge[0] = rtb_Abs_f;
      mtpa_reference_wrapper_B.Merge[1] = 1.0F / (-1.698025F * rtb_Abs_f + 1.0F)
        * rtb_Abs * rtb_Abs_ka;

      /* End of Outputs for SubSystem: '<S10>/If Action Subsystem1' */
    }

    /* End of Outputs for SubSystem: '<S8>/FW condition' */
  }

  /* End of If: '<S8>/If' */

  /* Gain: '<Root>/Id_pu_to_q15' */
  rtb_we = 32768.0F * mtpa_reference_wrapper_B.Merge[0];

  /* Saturate: '<Root>/Id_ref_q15_limit' */
  if (rtb_we > 32767.0F) {
    /* DataTypeConversion: '<Root>/Id_ref_q15_to_int16' */
    rtb_we = 32767.0F;
  } else if (rtb_we < -32768.0F) {
    /* DataTypeConversion: '<Root>/Id_ref_q15_to_int16' */
    rtb_we = -32768.0F;
  }

  /* End of Saturate: '<Root>/Id_ref_q15_limit' */

  /* DataTypeConversion: '<Root>/Id_ref_q15_to_int16' */
  if (rtb_we < 32768.0F) {
    /* Outport: '<Root>/Id_ref_q15' */
    mtpa_reference_wrapper_Y.Id_ref_q15 = (int16_T)rtb_we;
  } else {
    /* Outport: '<Root>/Id_ref_q15' */
    mtpa_reference_wrapper_Y.Id_ref_q15 = MAX_int16_T;
  }

  /* Switch: '<S5>/sign' incorporates:
   *  UnaryMinus: '<S5>/Unary Minus'
   */
  if (rtb_Iq_ref_q15_limit > 0.0F) {
    rtb_Saturation2 = mtpa_reference_wrapper_B.Merge[1];
  } else {
    rtb_Saturation2 = -mtpa_reference_wrapper_B.Merge[1];
  }

  /* Gain: '<Root>/Iq_pu_to_q15' incorporates:
   *  Switch: '<S5>/sign'
   */
  rtb_we = 32768.0F * rtb_Saturation2;

  /* Saturate: '<Root>/Iq_ref_q15_limit' */
  if (rtb_we > 32767.0F) {
    /* DataTypeConversion: '<Root>/Iq_ref_q15_to_int16' */
    rtb_we = 32767.0F;
  } else if (rtb_we < -32768.0F) {
    /* DataTypeConversion: '<Root>/Iq_ref_q15_to_int16' */
    rtb_we = -32768.0F;
  }

  /* End of Saturate: '<Root>/Iq_ref_q15_limit' */

  /* DataTypeConversion: '<Root>/Iq_ref_q15_to_int16' */
  if (rtb_we < 32768.0F) {
    /* Outport: '<Root>/Iq_ref_q15' */
    mtpa_reference_wrapper_Y.Iq_ref_q15 = (int16_T)rtb_we;
  } else {
    /* Outport: '<Root>/Iq_ref_q15' */
    mtpa_reference_wrapper_Y.Iq_ref_q15 = MAX_int16_T;
  }
}

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

static void MTPA_ProjectParameters_calculateCurrent(const MTPA_ProjectParameters *parameters,
                                                     real32_T currentMagnitudeA,
                                                     real32_T *directCurrentA,
                                                     real32_T *quadratureCurrentA)
{
  real32_T saliencyH = parameters->quadratureInductanceH - parameters->directInductanceH;
  real32_T directCurrent = 0.0F;
  real32_T quadratureCurrentSquared;

  if (saliencyH > 1.0E-9F) {
    directCurrent = (parameters->permanentMagnetFluxWb
      - sqrtf(parameters->permanentMagnetFluxWb * parameters->permanentMagnetFluxWb
        + 8.0F * saliencyH * saliencyH * currentMagnitudeA * currentMagnitudeA))
      / (4.0F * saliencyH);

    if (directCurrent < -currentMagnitudeA) {
      directCurrent = -currentMagnitudeA;
    }
  }

  quadratureCurrentSquared = currentMagnitudeA * currentMagnitudeA
    - directCurrent * directCurrent;
  if (quadratureCurrentSquared < 0.0F) {
    quadratureCurrentSquared = 0.0F;
  }

  *directCurrentA = directCurrent;
  *quadratureCurrentA = sqrtf(quadratureCurrentSquared);
}

static real32_T MTPA_ProjectParameters_calculateTorque(const MTPA_ProjectParameters *parameters,
                                                        real32_T currentMagnitudeA)
{
  real32_T directCurrentA;
  real32_T quadratureCurrentA;

  MTPA_ProjectParameters_calculateCurrent(parameters, currentMagnitudeA,
    &directCurrentA, &quadratureCurrentA);

  return 1.5F * (real32_T)parameters->polePairs
    * (parameters->permanentMagnetFluxWb
      + (parameters->directInductanceH - parameters->quadratureInductanceH) * directCurrentA)
    * quadratureCurrentA;
}

/* Project-parameter entry point used by the C FOC integration. */
void mtpa_reference_wrapper_step(void)
{
  const MTPA_ProjectParameters *parameters = MTPA_ProjectParameters_get();
  real32_T requestedTorqueNm;
  real32_T maximumTorqueNm;
  real32_T lowCurrentA;
  real32_T highCurrentA;
  real32_T currentMagnitudeA;
  real32_T directCurrentA;
  real32_T quadratureCurrentA;
  real32_T signedQuadratureCurrentA;
  uint8_T iteration;

  if (parameters->valid == 0u) {
    mtpa_reference_wrapper_B.Merge[0] = 0.0F;
    mtpa_reference_wrapper_B.Merge[1] = 0.0F;
    mtpa_reference_wrapper_Y.Id_ref_q15 = 0;
    mtpa_reference_wrapper_Y.Iq_ref_q15 = 0;
    return;
  }

  requestedTorqueNm = 3.0517578E-5F * (real32_T)mtpa_reference_wrapper_U.Tref_q15
    * parameters->baseTorqueNm;
  if (requestedTorqueNm < 0.0F) {
    requestedTorqueNm = -requestedTorqueNm;
  }

  maximumTorqueNm = MTPA_ProjectParameters_calculateTorque(parameters,
    parameters->baseCurrentA);
  lowCurrentA = 0.0F;
  highCurrentA = parameters->baseCurrentA;

  if (requestedTorqueNm < maximumTorqueNm) {
    for (iteration = 0u; iteration < 12u; ++iteration) {
      currentMagnitudeA = 0.5F * (lowCurrentA + highCurrentA);

      if (MTPA_ProjectParameters_calculateTorque(parameters, currentMagnitudeA)
          < requestedTorqueNm) {
        lowCurrentA = currentMagnitudeA;
      } else {
        highCurrentA = currentMagnitudeA;
      }
    }
  }

  MTPA_ProjectParameters_calculateCurrent(parameters, highCurrentA,
    &directCurrentA, &quadratureCurrentA);
  signedQuadratureCurrentA = (mtpa_reference_wrapper_U.Tref_q15 < 0)
    ? -quadratureCurrentA : quadratureCurrentA;

  mtpa_reference_wrapper_B.Merge[0] = directCurrentA / parameters->baseCurrentA;
  mtpa_reference_wrapper_B.Merge[1] = signedQuadratureCurrentA / parameters->baseCurrentA;
  mtpa_reference_wrapper_Y.Id_ref_q15 = MTPA_ProjectParameters_toQ15(
    mtpa_reference_wrapper_B.Merge[0]);
  mtpa_reference_wrapper_Y.Iq_ref_q15 = MTPA_ProjectParameters_toQ15(
    mtpa_reference_wrapper_B.Merge[1]);
}

/* Model initialize function */
void mtpa_reference_wrapper_initialize(void)
{
  /* (no initialization code required) */
}

/* Model terminate function */
void mtpa_reference_wrapper_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
