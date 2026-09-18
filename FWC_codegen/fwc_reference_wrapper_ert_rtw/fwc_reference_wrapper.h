/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: fwc_reference_wrapper.h
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

#ifndef fwc_reference_wrapper_h_
#define fwc_reference_wrapper_h_
#ifndef fwc_reference_wrapper_COMMON_INCLUDES_
#define fwc_reference_wrapper_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "rt_nonfinite.h"
#include "math.h"
#endif                              /* fwc_reference_wrapper_COMMON_INCLUDES_ */

#include "fwc_reference_wrapper_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block states (default storage) for system '<Root>' */
typedef struct {
  real32_T State_DSTATE[16];           /* '<Root>/State' */
} DW_fwc_reference_wrapper_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  uint8_T Enable;                      /* '<Root>/Enable' */
  uint8_T Reset;                       /* '<Root>/Reset' */
  uint8_T Eligible;                    /* '<Root>/Eligible' */
  uint8_T VdcValid;                    /* '<Root>/VdcValid' */
  real32_T VutilReq;                   /* '<Root>/VutilReq' */
  real32_T VutilAct;                   /* '<Root>/VutilAct' */
  real32_T IdBase;                     /* '<Root>/IdBase' */
  real32_T Params[11];                 /* '<Root>/Params' */
  uint32_T FloorSatTicks;              /* '<Root>/FloorSatTicks' */
  uint32_T UnsatTicks;                 /* '<Root>/UnsatTicks' */
} ExtU_fwc_reference_wrapper_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real32_T IdFw;                       /* '<Root>/IdFw' */
  real32_T IdRef;                      /* '<Root>/IdRef' */
  uint8_T Active;                      /* '<Root>/Active' */
  uint8_T Valid;                       /* '<Root>/Valid' */
  uint8_T Saturated;                   /* '<Root>/Saturated' */
  uint8_T IdAtLo;                      /* '<Root>/IdAtLo' */
  uint8_T Status;                      /* '<Root>/Status' */
  real32_T ReqFlt;                     /* '<Root>/ReqFlt' */
  real32_T ActFlt;                     /* '<Root>/ActFlt' */
  uint8_T WeakAct;                     /* '<Root>/WeakAct' */
  uint8_T RecoveryAct;                 /* '<Root>/RecoveryAct' */
} ExtY_fwc_reference_wrapper_T;

/* Real-time Model Data Structure */
struct tag_RTM_fwc_reference_wrapper_T {
  const char_T * volatile errorStatus;
};

/* Block states (default storage) */
extern DW_fwc_reference_wrapper_T fwc_reference_wrapper_DW;

/* External inputs (root inport signals with default storage) */
extern ExtU_fwc_reference_wrapper_T fwc_reference_wrapper_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_fwc_reference_wrapper_T fwc_reference_wrapper_Y;

/* Model entry point functions */
extern void fwc_reference_wrapper_initialize(void);
extern void fwc_reference_wrapper_step(void);
extern void fwc_reference_wrapper_terminate(void);

/* Real-time Model object */
extern RT_MODEL_fwc_reference_wrappe_T *const fwc_reference_wrapper_M;

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
 * '<Root>' : 'fwc_reference_wrapper'
 * '<S1>'   : 'fwc_reference_wrapper/FwcStep'
 */
#endif                                 /* fwc_reference_wrapper_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
