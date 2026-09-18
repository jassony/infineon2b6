/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: kre_external_observer_wrapper.h
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

#ifndef kre_external_observer_wrapper_h_
#define kre_external_observer_wrapper_h_
#ifndef kre_external_observer_wrapper_COMMON_INCLUDES_
#define kre_external_observer_wrapper_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "rt_nonfinite.h"
#include "math.h"
#endif                      /* kre_external_observer_wrapper_COMMON_INCLUDES_ */

#include "kre_external_observer_wrapper_types.h"
#include "rtGetNaN.h"
#include "zero_crossing_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block states (default storage) for system '<Root>' */
typedef struct {
  real32_T h2Vri[2];                   /* '<S1>/KreExternalObserverStep' */
  real32_T h2I[2];                     /* '<S1>/KreExternalObserverStep' */
  real32_T h2Reg;                      /* '<S1>/KreExternalObserverStep' */
  real32_T h2D;                        /* '<S1>/KreExternalObserverStep' */
  real32_T qState[4];                  /* '<S1>/KreExternalObserverStep' */
  real32_T yState[2];                  /* '<S1>/KreExternalObserverStep' */
  real32_T lambdaHat[2];               /* '<S1>/KreExternalObserverStep' */
  real32_T pllTheta;                   /* '<S1>/KreExternalObserverStep' */
  real32_T pllOmegaInt;                /* '<S1>/KreExternalObserverStep' */
  real32_T speedFilt;                  /* '<S1>/KreExternalObserverStep' */
  boolean_T pllInitialized;            /* '<S1>/KreExternalObserverStep' */
} DW_kre_external_observer_wrap_T;

/* Zero-crossing (trigger) state */
typedef struct {
  ZCSigState KreObserverResettable_Reset_ZCE;/* '<Root>/KreObserverResettable' */
} PrevZCX_kre_external_observer_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  uint8_T kreEnable;                   /* '<Root>/kreEnable' */
  real32_T kreViFb[4];                 /* '<Root>/kreViFb' */
  real32_T kreParams[14];              /* '<Root>/kreParams' */
  uint8_T kreReset;                    /* '<Root>/kreReset' */
  real32_T krePllParams[2];            /* '<Root>/krePllParams' */
} ExtU_kre_external_observer_wr_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real32_T krePositionPU;              /* '<Root>/krePositionPU' */
  real32_T kreSpeedPU;                 /* '<Root>/kreSpeedPU' */
  real32_T kreFluxMagnitudePU;         /* '<Root>/kreFluxMagnitudePU' */
  uint8_T kreStatus;                   /* '<Root>/kreStatus' */
  real32_T kreActiveFlux_Wb;           /* '<Root>/kreActiveFlux_Wb' */
  real32_T kreRawOmega_radps;          /* '<Root>/kreRawOmega_radps' */
} ExtY_kre_external_observer_wr_T;

/* Real-time Model Data Structure */
struct tag_RTM_kre_external_observer_T {
  const char_T * volatile errorStatus;
};

/* Block states (default storage) */
extern DW_kre_external_observer_wrap_T kre_external_observer_wrappe_DW;

/* Zero-crossing (trigger) state */
extern PrevZCX_kre_external_observer_T kre_external_observer_w_PrevZCX;

/* External inputs (root inport signals with default storage) */
extern ExtU_kre_external_observer_wr_T kre_external_observer_wrapper_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_kre_external_observer_wr_T kre_external_observer_wrapper_Y;

/* Local C customization: derived only during initialization/parameter staging.
 * Publish with kreParams/krePllParams while observer execution is excluded.
 * Model regeneration must preserve this contract and the cached step uses. */
typedef struct {
  real32_T h2Gain;
  real32_T speedGain;
  real32_T pllKp;
  real32_T pllKiTs;
  real32_T inductanceDelta;
} KreDiscreteCoefficients;
extern KreDiscreteCoefficients kre_external_observer_coefficients;

/* Model entry point functions */
extern void kre_external_observer_wrapper_initialize(void);
extern void kre_external_observer_wrapper_step(void);
extern void kre_external_observer_wrapper_terminate(void);

/* Real-time Model object */
extern RT_MODEL_kre_external_observe_T *const kre_external_observer_wrappe_M;

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
 * '<Root>' : 'kre_external_observer_wrapper'
 * '<S1>'   : 'kre_external_observer_wrapper/KreObserverResettable'
 * '<S2>'   : 'kre_external_observer_wrapper/KreObserverResettable/KreExternalObserverStep'
 */
#endif                                 /* kre_external_observer_wrapper_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
