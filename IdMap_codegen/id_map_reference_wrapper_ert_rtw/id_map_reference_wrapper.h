/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: id_map_reference_wrapper.h
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

#ifndef id_map_reference_wrapper_h_
#define id_map_reference_wrapper_h_
#ifndef id_map_reference_wrapper_COMMON_INCLUDES_
#define id_map_reference_wrapper_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "math.h"
#endif                           /* id_map_reference_wrapper_COMMON_INCLUDES_ */

#include "id_map_reference_wrapper_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* External inputs (root inport signals with default storage) */
typedef struct {
  real32_T Speed_rpm;                  /* '<Root>/Speed_rpm' */
  real32_T Iq_A;                       /* '<Root>/Iq_A' */
} ExtU_id_map_reference_wrapper_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real32_T Id_ref_A;                   /* '<Root>/Id_ref_A' */
} ExtY_id_map_reference_wrapper_T;

/* Parameters (default storage) */
struct P_id_map_reference_wrapper_T_ {
  uint32_T IdMap_2D_maxIndex[2];       /* Computed Parameter: IdMap_2D_maxIndex
                                        * Referenced by: '<Root>/IdMap_2D'
                                        */
};

/* Real-time Model Data Structure */
struct tag_RTM_id_map_reference_wrap_T {
  const char_T * volatile errorStatus;
};

/* Block parameters (default storage) */
extern P_id_map_reference_wrapper_T id_map_reference_wrapper_P;

/* External inputs (root inport signals with default storage) */
extern ExtU_id_map_reference_wrapper_T id_map_reference_wrapper_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_id_map_reference_wrapper_T id_map_reference_wrapper_Y;

/*
 * Exported Global Parameters
 *
 * Note: Exported global parameters are tunable parameters with an exported
 * global storage class designation.  Code generation will declare the memory for
 * these parameters and exports their symbols.
 *
 */
extern real32_T Cal_IdMap_Iq_A_f32[11];/* Variable: Cal_IdMap_Iq_A_f32
                                        * Referenced by: '<Root>/IdMap_2D'
                                        * Id map q-axis current breakpoints in A.
                                        */
extern real32_T Cal_IdMap_Spd_rpm_f32[11];/* Variable: Cal_IdMap_Spd_rpm_f32
                                           * Referenced by: '<Root>/IdMap_2D'
                                           * Id map speed breakpoints in rpm.
                                           */
extern real32_T Cal_IdMap_Table_A_f32[121];/* Variable: Cal_IdMap_Table_A_f32
                                            * Referenced by: '<Root>/IdMap_2D'
                                            * Id map d-axis current output table in A.
                                            */

/* Model entry point functions */
extern void id_map_reference_wrapper_initialize(void);
extern void id_map_reference_wrapper_step(void);
extern void id_map_reference_wrapper_terminate(void);

/* Real-time Model object */
extern RT_MODEL_id_map_reference_wra_T *const id_map_reference_wrapper_M;

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
 * '<Root>' : 'id_map_reference_wrapper'
 */
#endif                                 /* id_map_reference_wrapper_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
