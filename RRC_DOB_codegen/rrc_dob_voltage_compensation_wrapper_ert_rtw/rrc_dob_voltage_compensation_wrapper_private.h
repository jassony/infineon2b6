/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: rrc_dob_voltage_compensation_wrapper_private.h
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

#ifndef rrc_dob_voltage_compensation_wrapper_private_h_
#define rrc_dob_voltage_compensation_wrapper_private_h_
#include "rtwtypes.h"
#include "multiword_types.h"
#include "rrc_dob_voltage_compensation_wrapper_types.h"
#include "rrc_dob_voltage_compensation_wrapper.h"

extern boolean_T sMultiWordLt(const uint32_T u1[], const uint32_T u2[], int32_T
  n);
extern int32_T sMultiWordCmp(const uint32_T u1[], const uint32_T u2[], int32_T n);
extern void sMultiWord2MultiWord(const uint32_T u1[], int32_T n1, uint32_T y[],
  int32_T n);
extern void sMultiWordShl(const uint32_T u1[], int32_T n1, uint32_T n2, uint32_T
  y[], int32_T n);
extern void sLong2MultiWord(int32_T u, uint32_T y[], int32_T n);
extern boolean_T sMultiWordGt(const uint32_T u1[], const uint32_T u2[], int32_T
  n);
extern void sMultiWord2sMultiWordSat(const uint32_T u1[], int32_T n1, uint32_T
  y[], int32_T n);
extern void sMultiWordDivFloor(const uint32_T u1[], int32_T n1, const uint32_T
  u2[], int32_T n2, uint32_T y1[], int32_T m1, uint32_T y2[], int32_T m2,
  uint32_T t1[], int32_T l1, uint32_T t2[], int32_T l2);
extern void MultiWordNeg(const uint32_T u1[], uint32_T y[], int32_T n);
extern void MultiWordSetSignedMin(uint32_T y[], int32_T n);
extern void MultiWordSetSignedMax(uint32_T y[], int32_T n);
extern void uMultiWordInc(uint32_T y[], int32_T n);
extern boolean_T MultiWord2Bool(const uint32_T u[], int32_T n);
extern int32_T uMultiWordDiv(uint32_T a[], int32_T na, uint32_T b[], int32_T nb,
  uint32_T q[], int32_T nq, uint32_T r[], int32_T nr);
extern void MultiWordAdd(const uint32_T u1[], const uint32_T u2[], uint32_T y[],
  int32_T n);
extern void sMultiWordMul(const uint32_T u1[], int32_T n1, const uint32_T u2[],
  int32_T n2, uint32_T y[], int32_T n);
extern void uLong2MultiWord(uint32_T u, uint32_T y[], int32_T n);
extern boolean_T sMultiWordGe(const uint32_T u1[], const uint32_T u2[], int32_T
  n);
extern boolean_T sMultiWordEq(const uint32_T u1[], const uint32_T u2[], int32_T
  n);
extern void sMultiWordShr(const uint32_T u1[], int32_T n1, uint32_T n2, uint32_T
  y[], int32_T n);
extern void MultiWordSub(const uint32_T u1[], const uint32_T u2[], uint32_T y[],
  int32_T n);
extern int32_T MultiWord2sLong(const uint32_T u[]);
extern boolean_T sMultiWordLe(const uint32_T u1[], const uint32_T u2[], int32_T
  n);
extern int32_T sMultiWord2sLongSat(const uint32_T u1[], int32_T n1);

#endif                     /* rrc_dob_voltage_compensation_wrapper_private_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
