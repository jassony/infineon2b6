/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: vafid_external_observer_wrapper.h
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

#ifndef vafid_external_observer_wrapper_h_
#define vafid_external_observer_wrapper_h_
#ifndef vafid_external_observer_wrapper_COMMON_INCLUDES_
#define vafid_external_observer_wrapper_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "rt_nonfinite.h"
#include "math.h"
#endif                    /* vafid_external_observer_wrapper_COMMON_INCLUDES_ */

#include "vafid_external_observer_wrapper_types.h"
#include "rtGetInf.h"
#include "rtGetNaN.h"
#include "zero_crossing_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

#define vafid_external_observer_wrapper_M (vafid_external_observer_wrap_M)

/* Block states (default storage) for system '<Root>' */
typedef struct {
  real32_T phaseD_rad;                 /* '<S1>/VafidDiscreteStep' */
  real32_T phaseQ_rad;                 /* '<S1>/VafidDiscreteStep' */
  real32_T trackedAngle_rad;           /* '<S1>/VafidDiscreteStep' */
  real32_T filteredOmega_radps;        /* '<S1>/VafidDiscreteStep' */
  real32_T filteredActiveFlux_Wb;      /* '<S1>/VafidDiscreteStep' */
  real32_T phasorReal[8];              /* '<S1>/VafidDiscreteStep' */
  real32_T phasorImag[8];              /* '<S1>/VafidDiscreteStep' */
  real32_T sumOmega_radps;             /* '<S1>/VafidDiscreteStep' */
  real32_T sumId_A;                    /* '<S1>/VafidDiscreteStep' */
  real32_T rsEstimate_Ohm;             /* '<S1>/VafidDiscreteStep' */
  real32_T ldEstimate_H;               /* '<S1>/VafidDiscreteStep' */
  real32_T lqEstimate_H;               /* '<S1>/VafidDiscreteStep' */
  real32_T fluxEstimate_Wb;            /* '<S1>/VafidDiscreteStep' */
  real32_T lastConditionNumber;        /* '<S1>/VafidDiscreteStep' */
  real32_T lastRelativeResidual;       /* '<S1>/VafidDiscreteStep' */
  uint32_T settleSampleCount;          /* '<S1>/VafidDiscreteStep' */
  uint32_T accumulatedSampleCount;     /* '<S1>/VafidDiscreteStep' */
  uint16_T acceptedWindowCount;        /* '<S1>/VafidDiscreteStep' */
  uint16_T rejectedWindowCount;        /* '<S1>/VafidDiscreteStep' */
  boolean_T activeFluxInitialized;     /* '<S1>/VafidDiscreteStep' */
  boolean_T angleInitialized;          /* '<S1>/VafidDiscreteStep' */
  boolean_T rsEstimate_Ohm_not_empty;  /* '<S1>/VafidDiscreteStep' */
  boolean_T ldEstimate_H_not_empty;    /* '<S1>/VafidDiscreteStep' */
  boolean_T lqEstimate_H_not_empty;    /* '<S1>/VafidDiscreteStep' */
  boolean_T fluxEstimate_Wb_not_empty; /* '<S1>/VafidDiscreteStep' */
  boolean_T estimateIsCurrent;         /* '<S1>/VafidDiscreteStep' */
  boolean_T wasEnabled;                /* '<S1>/VafidDiscreteStep' */
} DW_vafid_external_observer_wr_T;

/* Zero-crossing (trigger) state */
typedef struct {
  ZCSigState VafidResettable_Reset_ZCE;/* '<Root>/VafidResettable' */
} PrevZCX_vafid_external_observ_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  uint8_T mode;                        /* '<Root>/mode' */
  boolean_T resetRequest;              /* '<Root>/resetRequest' */
  boolean_T sampleValid;               /* '<Root>/sampleValid' */
  real32_T voltageAlpha_V;             /* '<Root>/voltageAlpha_V' */
  real32_T voltageBeta_V;              /* '<Root>/voltageBeta_V' */
  real32_T currentAlpha_A;             /* '<Root>/currentAlpha_A' */
  real32_T currentBeta_A;              /* '<Root>/currentBeta_A' */
  real32_T kreElectricalAngle_rad;     /* '<Root>/kreElectricalAngle_rad' */
  real32_T kreElectricalOmega_radps;   /* '<Root>/kreElectricalOmega_radps' */
  real32_T kreActiveFlux_Wb;           /* '<Root>/kreActiveFlux_Wb' */
  real32_T configValues[30];           /* '<Root>/configValues' */
} ExtU_vafid_external_observer__T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real32_T probeD_PU;                  /* '<Root>/probeD_PU' */
  real32_T probeQ_PU;                  /* '<Root>/probeQ_PU' */
  real32_T rs_Ohm;                     /* '<Root>/rs_Ohm' */
  real32_T ld_H;                       /* '<Root>/ld_H' */
  real32_T lq_H;                       /* '<Root>/lq_H' */
  real32_T fluxPM_Wb;                  /* '<Root>/fluxPM_Wb' */
  boolean_T estimateValid;             /* '<Root>/estimateValid' */
  boolean_T freshEstimate;             /* '<Root>/freshEstimate' */
  boolean_T staleEstimate;             /* '<Root>/staleEstimate' */
  uint8_T status;                      /* '<Root>/status' */
  real32_T conditionNumber;            /* '<Root>/conditionNumber' */
  real32_T relativeResidual;           /* '<Root>/relativeResidual' */
  uint16_T consecutiveAcceptedWindows; /* '<Root>/consecutiveAcceptedWindows' */
  uint32_T windowSampleCount;          /* '<Root>/windowSampleCount' */
} ExtY_vafid_external_observer__T;

/* Real-time Model Data Structure */
struct tag_RTM_vafid_external_observ_T {
  const char_T * volatile errorStatus;
};

/* Block states (default storage) */
extern DW_vafid_external_observer_wr_T vafid_external_observer_wrap_DW;

/* Zero-crossing (trigger) state */
extern PrevZCX_vafid_external_observ_T vafid_external_observer_PrevZCX;

/* External inputs (root inport signals with default storage) */
extern ExtU_vafid_external_observer__T vafid_external_observer_wrapp_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_vafid_external_observer__T vafid_external_observer_wrapp_Y;

/* Model entry point functions */
extern void vafid_external_observer_wrapper_initialize(void);
extern void vafid_external_observer_wrapper_step(void);
extern void vafid_external_observer_wrapper_terminate(void);

/* Real-time Model object */
extern RT_MODEL_vafid_external_obser_T *const vafid_external_observer_wrap_M;

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
 * '<Root>' : 'vafid_external_observer_wrapper'
 * '<S1>'   : 'vafid_external_observer_wrapper/VafidResettable'
 * '<S2>'   : 'vafid_external_observer_wrapper/VafidResettable/VafidDiscreteStep'
 */
#endif                                 /* vafid_external_observer_wrapper_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
