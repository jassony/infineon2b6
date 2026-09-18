/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: mtpa_reference_wrapper.h
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

#ifndef mtpa_reference_wrapper_h_
#define mtpa_reference_wrapper_h_
#ifndef mtpa_reference_wrapper_COMMON_INCLUDES_
#define mtpa_reference_wrapper_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "math.h"
#endif                             /* mtpa_reference_wrapper_COMMON_INCLUDES_ */

#include "mtpa_reference_wrapper_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block signals (default storage) */
typedef struct {
  real32_T Merge[2];                   /* '<S8>/Merge' */
} B_mtpa_reference_wrapper_T;

/* Invariant block signals (default storage) */
typedef struct {
  const real32_T Switch;               /* '<S6>/Switch' */
  const real32_T Gain;                 /* '<S12>/Gain' */
  const real32_T Subtract;             /* '<S12>/Subtract' */
  const real32_T Gain_g;               /* '<S16>/Gain' */
  const real32_T Subtract1;            /* '<S16>/Subtract1' */
  const real32_T Gain1;                /* '<S23>/Gain1' */
  const real32_T Add1;                 /* '<S23>/Add1' */
  const real32_T Gain_l;               /* '<S17>/Gain' */
  const real32_T Subtract_k;           /* '<S17>/Subtract' */
} ConstB_mtpa_reference_wrapper_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  int16_T Tref_q15;                    /* '<Root>/Tref_q15' */
  int16_T Speed_q15;                   /* '<Root>/Speed_q15' */
} ExtU_mtpa_reference_wrapper_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  int16_T Id_ref_q15;                  /* '<Root>/Id_ref_q15' */
  int16_T Iq_ref_q15;                  /* '<Root>/Iq_ref_q15' */
} ExtY_mtpa_reference_wrapper_T;

/* Real-time Model Data Structure */
struct tag_RTM_mtpa_reference_wrappe_T {
  const char_T * volatile errorStatus;
};

/* Block signals (default storage) */
extern B_mtpa_reference_wrapper_T mtpa_reference_wrapper_B;

/* External inputs (root inport signals with default storage) */
extern ExtU_mtpa_reference_wrapper_T mtpa_reference_wrapper_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_mtpa_reference_wrapper_T mtpa_reference_wrapper_Y;
extern const ConstB_mtpa_reference_wrapper_T mtpa_reference_wrapper_ConstB;/* constant block i/o */

/* Model entry point functions */
extern void mtpa_reference_wrapper_initialize(void);
extern void mtpa_reference_wrapper_step(void);
extern void mtpa_reference_wrapper_terminate(void);

/* Real-time Model object */
extern RT_MODEL_mtpa_reference_wrapp_T *const mtpa_reference_wrapper_M;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S13>/Data Type Duplicate' : Unused code path elimination
 * Block '<S33>/Data Type Duplicate' : Unused code path elimination
 * Block '<S33>/Data Type Propagation' : Unused code path elimination
 * Block '<S34>/Data Type Duplicate' : Unused code path elimination
 * Block '<S36>/Data Type Duplicate' : Unused code path elimination
 * Block '<S20>/Data Type Duplicate' : Unused code path elimination
 * Block '<S26>/Data Type Duplicate1' : Unused code path elimination
 * Block '<S26>/Data Type Duplicate2' : Unused code path elimination
 * Block '<S27>/Sqrt' : Unused code path elimination
 * Block '<S21>/Data Type Duplicate' : Unused code path elimination
 * Block '<S8>/Gain' : Eliminated nontunable gain of 1
 * Block '<S26>/enableInportSatLim' : Unused code path elimination
 * Block '<S26>/enableInportSatMethod' : Unused code path elimination
 * Block '<S20>/ReplaceInport_satLim' : Unused code path elimination
 * Block '<S20>/ReplaceInport_satMethod' : Unused code path elimination
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
 * '<Root>' : 'mtpa_reference_wrapper'
 * '<S1>'   : 'mtpa_reference_wrapper/MTPA_Control_Reference'
 * '<S2>'   : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System'
 * '<S3>'   : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM'
 * '<S4>'   : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection'
 * '<S5>'   : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/Subsystem'
 * '<S6>'   : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/Subsystem2'
 * '<S7>'   : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators'
 * '<S8>'   : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate'
 * '<S9>'   : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/Circle_MTPA_intersection'
 * '<S10>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition'
 * '<S11>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/MTPA condition'
 * '<S12>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/Subsystem1'
 * '<S13>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/Circle_MTPA_intersection/Get_Iq_ref'
 * '<S14>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/If Action Subsystem'
 * '<S15>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/If Action Subsystem1'
 * '<S16>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed'
 * '<S17>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/If Action Subsystem1/fieldWeakening'
 * '<S18>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/If Action Subsystem1/fieldWeakening/torqueTangent'
 * '<S19>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/If Action Subsystem1/fieldWeakening/update iq1'
 * '<S20>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter'
 * '<S21>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/recip'
 * '<S22>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/sqrt_controlled'
 * '<S23>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/update iq2'
 * '<S24>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/D or Q Axis Priority'
 * '<S25>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/D-Q Equivalence'
 * '<S26>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/Inport or Dialog Selection'
 * '<S27>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/Magnitude_calc'
 * '<S28>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/D or Q Axis Priority/Compare To Constant'
 * '<S29>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/D or Q Axis Priority/Compare To Constant1'
 * '<S30>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/D or Q Axis Priority/flipInputs'
 * '<S31>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/D or Q Axis Priority/flipInputs1'
 * '<S32>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/D or Q Axis Priority/limiter'
 * '<S33>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/D or Q Axis Priority/limiter/limitRef1'
 * '<S34>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/D or Q Axis Priority/limiter/limitRef2'
 * '<S35>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/D or Q Axis Priority/limiter/passThrough'
 * '<S36>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/D-Q Equivalence/Limiter'
 * '<S37>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/DQ Limiter/D-Q Equivalence/Passthrough'
 * '<S38>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/sqrt_controlled/Compare To Zero'
 * '<S39>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/sqrt_controlled/If negative value'
 * '<S40>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/FW condition/max_iq_at_speed/sqrt_controlled/If positive value'
 * '<S41>'  : 'mtpa_reference_wrapper/MTPA_Control_Reference/Motor_System/Interior PMSM/MTPA_FW_iteratorSelection/no_iterators/fast_and_approximate/Subsystem1/find vs'
 */
#endif                                 /* mtpa_reference_wrapper_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
