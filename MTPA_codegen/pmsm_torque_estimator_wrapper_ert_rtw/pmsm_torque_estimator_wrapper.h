/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: pmsm_torque_estimator_wrapper.h
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

#ifndef pmsm_torque_estimator_wrapper_h_
#define pmsm_torque_estimator_wrapper_h_
#ifndef pmsm_torque_estimator_wrapper_COMMON_INCLUDES_
#define pmsm_torque_estimator_wrapper_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "math.h"
#endif                      /* pmsm_torque_estimator_wrapper_COMMON_INCLUDES_ */

#include "pmsm_torque_estimator_wrapper_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

#define pmsm_torque_estimator_wrapper_M (pmsm_torque_estimator_wrappe_M)

/* Invariant block signals (default storage) */
typedef struct {
  const real32_T Ld_Port;              /* '<S4>/Gain' */
  const real32_T Switch;               /* '<S4>/Switch' */
  const real32_T Lq_Port;              /* '<S4>/Gain1' */
  const real32_T Switch1;              /* '<S4>/Switch1' */
  const real32_T FluxPM_Port;          /* '<S4>/Gain2' */
  const real32_T Switch2;              /* '<S4>/Switch2' */
  const real32_T Ld_Lq;                /* '<S3>/Subtract' */
} ConstB_pmsm_torque_estimator__T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  int16_T Id_q15;                      /* '<Root>/Id_q15' */
  int16_T Iq_q15;                      /* '<Root>/Iq_q15' */
  int16_T Speed_q15;                   /* '<Root>/Speed_q15' */
} ExtU_pmsm_torque_estimator_wr_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  int16_T Te_q15;                      /* '<Root>/Te_q15' */
  int16_T Pe_q15;                      /* '<Root>/Pe_q15' */
} ExtY_pmsm_torque_estimator_wr_T;

/* Real-time Model Data Structure */
struct tag_RTM_pmsm_torque_estimator_T {
  const char_T * volatile errorStatus;
};

/* External inputs (root inport signals with default storage) */
extern ExtU_pmsm_torque_estimator_wr_T pmsm_torque_estimator_wrapper_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_pmsm_torque_estimator_wr_T pmsm_torque_estimator_wrapper_Y;
extern const ConstB_pmsm_torque_estimator__T pmsm_torque_estimator_wr_ConstB;/* constant block i/o */

/* Model entry point functions */
extern void pmsm_torque_estimator_wrapper_initialize(void);
extern void pmsm_torque_estimator_wrapper_step(void);
extern void pmsm_torque_estimator_wrapper_terminate(void);

/* Real-time Model object */
extern RT_MODEL_pmsm_torque_estimato_T *const pmsm_torque_estimator_wrappe_M;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S3>/Data Type Duplicate' : Unused code path elimination
 * Block '<S3>/T_si2pu' : Eliminated nontunable gain of 1
 * Block '<S3>/id_pu2si' : Eliminated nontunable gain of 1
 * Block '<S3>/iq_pu2si' : Eliminated nontunable gain of 1
 * Block '<S3>/wm_pu2si' : Eliminated nontunable gain of 1
 */

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
 * '<Root>' : 'pmsm_torque_estimator_wrapper'
 * '<S1>'   : 'pmsm_torque_estimator_wrapper/PMSM_Torque_Estimator'
 * '<S2>'   : 'pmsm_torque_estimator_wrapper/PMSM_Torque_Estimator/Variant Subsystem'
 * '<S3>'   : 'pmsm_torque_estimator_wrapper/PMSM_Torque_Estimator/Variant Subsystem/Torque Estimator_LumpedParameters_InputPort'
 * '<S4>'   : 'pmsm_torque_estimator_wrapper/PMSM_Torque_Estimator/Variant Subsystem/Torque Estimator_LumpedParameters_InputPort/LumpedParams_InputPorts'
 */
#endif                                 /* pmsm_torque_estimator_wrapper_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
