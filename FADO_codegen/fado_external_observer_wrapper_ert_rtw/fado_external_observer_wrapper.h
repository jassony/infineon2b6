/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: fado_external_observer_wrapper.h
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

#ifndef fado_external_observer_wrapper_h_
#define fado_external_observer_wrapper_h_
#ifndef fado_external_observer_wrapper_COMMON_INCLUDES_
#define fado_external_observer_wrapper_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "rt_nonfinite.h"
#include "math.h"
#endif                     /* fado_external_observer_wrapper_COMMON_INCLUDES_ */

#include "fado_external_observer_wrapper_types.h"
#include "rtGetNaN.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

#define fado_external_observer_wrapper_M (fado_external_observer_wrapp_M)

/* Block states (default storage) for system '<Root>' */
typedef struct {
  real32_T fadoState_z[10];            /* '<Root>/fadoState_z' */
} DW_fado_external_observer_wra_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  boolean_T fadoEnable;                /* '<Root>/fadoEnable' */
  real32_T fadoVoltageAlpha_V;         /* '<Root>/fadoVoltageAlpha_V' */
  real32_T fadoVoltageBeta_V;          /* '<Root>/fadoVoltageBeta_V' */
  real32_T fadoCurrentAlpha_A;         /* '<Root>/fadoCurrentAlpha_A' */
  real32_T fadoCurrentBeta_A;          /* '<Root>/fadoCurrentBeta_A' */
  real32_T fadoStatorResistance_Ohm;   /* '<Root>/fadoStatorResistance_Ohm' */
  real32_T fadoQuadratureInductance_H; /* '<Root>/fadoQuadratureInductance_H' */
  uint8_T fadoPolePairs;               /* '<Root>/fadoPolePairs' */
  real32_T fadoFluxLimit_Wb;           /* '<Root>/fadoFluxLimit_Wb' */
  real32_T fadoKdf_per_s;              /* '<Root>/fadoKdf_per_s' */
  real32_T fadoLowSpeedThreshold_Hz;   /* '<Root>/fadoLowSpeedThreshold_Hz' */
  real32_T fadoKaf_radps;              /* '<Root>/fadoKaf_radps' */
  real32_T fadoFastT2SBandwidth_Hz;    /* '<Root>/fadoFastT2SBandwidth_Hz' */
  real32_T fadoSlowT2SBandwidth_Hz;    /* '<Root>/fadoSlowT2SBandwidth_Hz' */
  real32_T fadoT2SDamping;             /* '<Root>/fadoT2SDamping' */
  real32_T fadoVoltageAlphaOffset_V;   /* '<Root>/fadoVoltageAlphaOffset_V' */
  real32_T fadoVoltageBetaOffset_V;    /* '<Root>/fadoVoltageBetaOffset_V' */
  boolean_T fadoVoltagePreprocessEnable;
                                      /* '<Root>/fadoVoltagePreprocessEnable' */
} ExtU_fado_external_observer_w_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real32_T fadoOutput[18];             /* '<Root>/fadoOutput' */
} ExtY_fado_external_observer_w_T;

/* Real-time Model Data Structure */
struct tag_RTM_fado_external_observe_T {
  const char_T * volatile errorStatus;
};

/* Block states (default storage) */
extern DW_fado_external_observer_wra_T fado_external_observer_wrapp_DW;

/* External inputs (root inport signals with default storage) */
extern ExtU_fado_external_observer_w_T fado_external_observer_wrappe_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_fado_external_observer_w_T fado_external_observer_wrappe_Y;

/* Model entry point functions */
extern void fado_external_observer_wrapper_initialize(void);
extern void fado_external_observer_wrapper_step(void);
extern void fado_external_observer_wrapper_terminate(void);

/* Real-time Model object */
extern RT_MODEL_fado_external_observ_T *const fado_external_observer_wrapp_M;

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'fado_external_observer_wrapper'
 * '<S1>'   : 'fado_external_observer_wrapper/FadoDiscreteUpdate'
 */
#endif                                 /* fado_external_observer_wrapper_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
