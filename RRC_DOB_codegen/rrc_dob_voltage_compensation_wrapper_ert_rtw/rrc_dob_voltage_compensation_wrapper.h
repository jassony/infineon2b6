/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: rrc_dob_voltage_compensation_wrapper.h
 *
 * Code generated for Simulink model 'rrc_dob_voltage_compensation_wrapper'.
 *
 * Model version                  : 1.2
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Thu Aug 27 22:31:27 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef rrc_dob_voltage_compensation_wrapper_h_
#define rrc_dob_voltage_compensation_wrapper_h_
#ifndef rrc_dob_voltage_compensation_wrapper_COMMON_INCLUDES_
#define rrc_dob_voltage_compensation_wrapper_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif               /* rrc_dob_voltage_compensation_wrapper_COMMON_INCLUDES_ */

#include "rrc_dob_voltage_compensation_wrapper_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block states (default storage) for system '<Root>' */
typedef struct {
  int32_T RRC_DOB_State_DSTATE[8];     /* '<Root>/RRC_DOB_State' */
} DW_rrc_dob_voltage_compensati_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  int16_T Previous_Applied_Voltage_DQ_Q15[2];
                                  /* '<Root>/Previous_Applied_Voltage_DQ_Q15' */
  int16_T Current_DQ_Q15[2];           /* '<Root>/Current_DQ_Q15' */
  int16_T Raw_PI_Voltage_DQ_Q15[2];    /* '<Root>/Raw_PI_Voltage_DQ_Q15' */
  int32_T Coefficients_DQ_Q27[38];     /* '<Root>/Coefficients_DQ_Q27' */
  int32_T Q_Q29;                       /* '<Root>/Q_Q29' */
  uint8_T Reset_u8;                    /* '<Root>/Reset_u8' */
  uint32_T Ramp_Q31;                   /* '<Root>/Ramp_Q31' */
  int16_T Output_Limit_Q15;            /* '<Root>/Output_Limit_Q15' */
} ExtU_rrc_dob_voltage_compensa_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  int16_T Command_DQ_Q15[2];           /* '<Root>/Command_DQ_Q15' */
  int16_T Error_Hat_DQ_Q15[2];         /* '<Root>/Error_Hat_DQ_Q15' */
  int32_T State_Q26[8];                /* '<Root>/State_Q26' */
  uint8_T Status_u8;                   /* '<Root>/Status_u8' */
} ExtY_rrc_dob_voltage_compensa_T;

/* Real-time Model Data Structure */
struct tag_RTM_rrc_dob_voltage_compe_T {
  const char_T * volatile errorStatus;
};

/* Block states (default storage) */
extern DW_rrc_dob_voltage_compensati_T rrc_dob_voltage_compensation_DW;

/* External inputs (root inport signals with default storage) */
extern ExtU_rrc_dob_voltage_compensa_T rrc_dob_voltage_compensation__U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_rrc_dob_voltage_compensa_T rrc_dob_voltage_compensation__Y;

/* Model entry point functions */
extern void rrc_dob_voltage_compensation_wrapper_initialize(void);
extern void rrc_dob_voltage_compensation_wrapper_step(void);
extern void rrc_dob_voltage_compensation_wrapper_terminate(void);

/* Real-time Model object */
extern RT_MODEL_rrc_dob_voltage_comp_T *const rrc_dob_voltage_compensation_M;

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
 * '<Root>' : 'rrc_dob_voltage_compensation_wrapper'
 * '<S1>'   : 'rrc_dob_voltage_compensation_wrapper/RRC_DOB_FixedStep'
 */
#endif                             /* rrc_dob_voltage_compensation_wrapper_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
