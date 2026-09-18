/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: apsfsm_torque_compensation_wrapper.h
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

#ifndef apsfsm_torque_compensation_wrapper_h_
#define apsfsm_torque_compensation_wrapper_h_
#ifndef apsfsm_torque_compensation_wrapper_COMMON_INCLUDES_
#define apsfsm_torque_compensation_wrapper_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "rt_nonfinite.h"
#include "math.h"
#endif                 /* apsfsm_torque_compensation_wrapper_COMMON_INCLUDES_ */

#include "apsfsm_torque_compensation_wrapper_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block states (default storage) for system '<Root>' */
typedef struct {
  real32_T APSFSM_State_DSTATE[4];     /* '<Root>/APSFSM_State' */
} DW_apsfsm_torque_compensation_wrapper_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real32_T Speed_Reference_PU;         /* '<Root>/Speed_Reference_PU' */
  real32_T Speed_Feedback_PU;          /* '<Root>/Speed_Feedback_PU' */
  real32_T Parameters[5];              /* '<Root>/Parameters' */
  real32_T Control[3];                 /* '<Root>/Control' */
} ExtU_apsfsm_torque_compensation_wrapper_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real32_T Iq_Raw_PU;                  /* '<Root>/Iq_Raw_PU' */
  uint8_T Valid_u8;                    /* '<Root>/Valid_u8' */
  uint8_T Status_u8;                   /* '<Root>/Status_u8' */
  real32_T BHat_PU;                    /* '<Root>/BHat_PU' */
  real32_T CHat_PU;                    /* '<Root>/CHat_PU' */
  real32_T Theta_rad;                  /* '<Root>/Theta_rad' */
  real32_T Speed_Error_PU;             /* '<Root>/Speed_Error_PU' */
  real32_T Covariance;                 /* '<Root>/Covariance' */
} ExtY_apsfsm_torque_compensation_wrapper_T;

/* Real-time Model Data Structure */
struct tag_RTM_apsfsm_torque_compensation_wrapper_T {
  const char_T * volatile errorStatus;
};

/* Block states (default storage) */
extern DW_apsfsm_torque_compensation_wrapper_T
  apsfsm_torque_compensation_wrapper_DW;

/* External inputs (root inport signals with default storage) */
extern ExtU_apsfsm_torque_compensation_wrapper_T
  apsfsm_torque_compensation_wrapper_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_apsfsm_torque_compensation_wrapper_T
  apsfsm_torque_compensation_wrapper_Y;

/* Model entry point functions */
extern void apsfsm_torque_compensation_wrapper_initialize(void);
extern void apsfsm_torque_compensation_wrapper_step(void);
extern void apsfsm_torque_compensation_wrapper_terminate(void);

/* Real-time Model object */
extern RT_MODEL_apsfsm_torque_compensation_wrapper_T *const
  apsfsm_torque_compensation_wrapper_M;

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
 * '<Root>' : 'apsfsm_torque_compensation_wrapper'
 * '<S1>'   : 'apsfsm_torque_compensation_wrapper/APSFSM_DiscreteStep'
 */
#endif                               /* apsfsm_torque_compensation_wrapper_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
