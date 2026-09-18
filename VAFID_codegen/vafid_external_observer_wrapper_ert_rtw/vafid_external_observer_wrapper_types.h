/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: vafid_external_observer_wrapper_types.h
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

#ifndef vafid_external_observer_wrapper_types_h_
#define vafid_external_observer_wrapper_types_h_
#include "rtwtypes.h"

/* Custom Type definition for MATLAB Function: '<S1>/VafidDiscreteStep' */
#ifndef struct_tag_sXUMNpzAt0y7k62cD7EcU0C
#define struct_tag_sXUMNpzAt0y7k62cD7EcU0C

struct tag_sXUMNpzAt0y7k62cD7EcU0C
{
  real32_T sampleTime_s;
  real32_T probeD_Frequency_Hz;
  real32_T probeQ_Frequency_Hz;
  real32_T probeD_Amplitude_PU;
  real32_T probeQ_Amplitude_PU;
  real32_T probeD_MaxAmplitude_PU;
  real32_T probeQ_MaxAmplitude_PU;
  real32_T settleTime_s;
  uint32_T windowLength_samples;
  real32_T omegaLpf_Hz;
  real32_T angleTrack_Hz;
  real32_T activeFluxLpf_Hz;
  real32_T parameterFusion;
  real32_T fluxFusion;
  real32_T conditionLimit;
  real32_T relativeResidualLimit;
  uint16_T requiredAcceptedWindows;
  uint16_T staleRejectedWindows;
  real32_T nominalRs_Ohm;
  real32_T nominalLd_H;
  real32_T nominalLq_H;
  real32_T nominalFluxPM_Wb;
  real32_T minimumRs_Ohm;
  real32_T maximumRs_Ohm;
  real32_T minimumLd_H;
  real32_T maximumLd_H;
  real32_T minimumLq_H;
  real32_T maximumLq_H;
  real32_T minimumFluxPM_Wb;
  real32_T maximumFluxPM_Wb;
};

#endif                                 /* struct_tag_sXUMNpzAt0y7k62cD7EcU0C */

#ifndef typedef_sXUMNpzAt0y7k62cD7EcU0C_vafid_T
#define typedef_sXUMNpzAt0y7k62cD7EcU0C_vafid_T

typedef struct tag_sXUMNpzAt0y7k62cD7EcU0C sXUMNpzAt0y7k62cD7EcU0C_vafid_T;

#endif                             /* typedef_sXUMNpzAt0y7k62cD7EcU0C_vafid_T */

/* Forward declaration for rtModel */
typedef struct tag_RTM_vafid_external_observ_T RT_MODEL_vafid_external_obser_T;

#endif                            /* vafid_external_observer_wrapper_types_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
