/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: rrc_dob_voltage_compensation_wrapper.c
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

#include "rrc_dob_voltage_compensation_wrapper.h"
#include "rtwtypes.h"
#include "multiword_types.h"
#include "rrc_dob_voltage_compensation_wrapper_private.h"

/* Block states (default storage) */
DW_rrc_dob_voltage_compensati_T rrc_dob_voltage_compensation_DW;

/* External inputs (root inport signals with default storage) */
ExtU_rrc_dob_voltage_compensa_T rrc_dob_voltage_compensation__U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_rrc_dob_voltage_compensa_T rrc_dob_voltage_compensation__Y;

/* Real-time model */
static RT_MODEL_rrc_dob_voltage_comp_T rrc_dob_voltage_compensation_M_;
RT_MODEL_rrc_dob_voltage_comp_T *const rrc_dob_voltage_compensation_M =
  &rrc_dob_voltage_compensation_M_;

/* Forward declaration for local functions */
static void rrc_dob_vo_localScaleStateValue(int32_T b_value, const int64m_T
  ratio_Q30, int32_T *scaledValue, boolean_T *valid);
static void rrc_dob_voltage_localAccumulate(const int64m_T accumulator, const
  int64m_T term, int64m_T *result, boolean_T *valid);
static void rrc_dob_voltage_c_localAxisStep(const int32_T state[3], int16_T
  previousAppliedVoltage_Q15, int16_T current_Q15, const int32_T
  coefficients_Q27[19], int16_T *eHat_Q15, int32_T stateNext[3], boolean_T
  *valid);
boolean_T sMultiWordLt(const uint32_T u1[], const uint32_T u2[], int32_T n)
{
  return sMultiWordCmp(u1, u2, n) < 0;
}

int32_T sMultiWordCmp(const uint32_T u1[], const uint32_T u2[], int32_T n)
{
  int32_T y;
  uint32_T su1;
  uint32_T su2;
  su1 = u1[n - 1] & 2147483648U;
  su2 = u2[n - 1] & 2147483648U;
  if (su1 != su2) {
    y = su1 != 0U ? -1 : 1;
  } else {
    int32_T i;
    y = 0;
    i = n;
    while ((y == 0) && (i > 0)) {
      i--;
      su1 = u1[i];
      su2 = u2[i];
      if (su1 != su2) {
        y = su1 > su2 ? 1 : -1;
      }
    }
  }

  return y;
}

void sMultiWord2MultiWord(const uint32_T u1[], int32_T n1, uint32_T y[], int32_T
  n)
{
  int32_T i;
  int32_T nm;
  nm = n1 < n ? n1 : n;
  for (i = 0; i < nm; i++) {
    y[i] = u1[i];
  }

  if (n > n1) {
    uint32_T u1i;
    u1i = (u1[n1 - 1] & 2147483648U) != 0U ? MAX_uint32_T : 0U;
    for (i = nm; i < n; i++) {
      y[i] = u1i;
    }
  }
}

void sMultiWordShl(const uint32_T u1[], int32_T n1, uint32_T n2, uint32_T y[],
                   int32_T n)
{
  int32_T i;
  int32_T nb;
  int32_T nc;
  uint32_T u1i;
  uint32_T ys;
  nb = (int32_T)(n2 >> 5);
  ys = (u1[n1 - 1] & 2147483648U) != 0U ? MAX_uint32_T : 0U;
  nc = nb > n ? n : nb;
  u1i = 0U;
  for (i = 0; i < nc; i++) {
    y[i] = 0U;
  }

  if (nb < n) {
    uint32_T nl;
    nl = n2 - ((uint32_T)nb << 5);
    nb += n1;
    if (nb > n) {
      nb = n;
    }

    nb -= i;
    if (nl > 0U) {
      for (nc = 0; nc < nb; nc++) {
        uint32_T yi;
        yi = u1i >> (32U - nl);
        u1i = u1[nc];
        y[i] = u1i << nl | yi;
        i++;
      }

      if (i < n) {
        y[i] = u1i >> (32U - nl) | ys << nl;
        i++;
      }
    } else {
      for (nc = 0; nc < nb; nc++) {
        y[i] = u1[nc];
        i++;
      }
    }
  }

  while (i < n) {
    y[i] = ys;
    i++;
  }
}

void sLong2MultiWord(int32_T u, uint32_T y[], int32_T n)
{
  int32_T i;
  uint32_T yi;
  y[0] = (uint32_T)u;
  yi = u < 0 ? MAX_uint32_T : 0U;
  for (i = 1; i < n; i++) {
    y[i] = yi;
  }
}

boolean_T sMultiWordGt(const uint32_T u1[], const uint32_T u2[], int32_T n)
{
  return sMultiWordCmp(u1, u2, n) > 0;
}

void sMultiWord2sMultiWordSat(const uint32_T u1[], int32_T n1, uint32_T y[],
  int32_T n)
{
  int32_T i;
  int32_T nm1;
  uint32_T ys;
  boolean_T doSaturation = false;
  nm1 = n - 1;
  ys = (u1[n1 - 1] & 2147483648U) != 0U ? MAX_uint32_T : 0U;
  if (n1 > n) {
    doSaturation = (((u1[n1 - 1] ^ u1[n - 1]) & 2147483648U) != 0U);
    i = n1 - 1;
    while (!doSaturation && (i >= n)) {
      doSaturation = (u1[i] != ys);
      i--;
    }
  }

  if (doSaturation) {
    ys = ~ys;
    for (i = 0; i < nm1; i++) {
      y[i] = ys;
    }

    y[i] = ys ^ 2147483648U;
  } else {
    nm1 = n1 < n ? n1 : n;
    for (i = 0; i < nm1; i++) {
      y[i] = u1[i];
    }

    while (i < n) {
      y[i] = ys;
      i++;
    }
  }
}

void sMultiWordDivFloor(const uint32_T u1[], int32_T n1, const uint32_T u2[],
  int32_T n2, uint32_T y1[], int32_T m1, uint32_T y2[], int32_T m2, uint32_T t1[],
  int32_T l1, uint32_T t2[], int32_T l2)
{
  boolean_T denNeg;
  boolean_T numNeg;
  numNeg = ((u1[n1 - 1] & 2147483648U) != 0U);
  denNeg = ((u2[n2 - 1] & 2147483648U) != 0U);
  if (numNeg) {
    MultiWordNeg(u1, t1, n1);
  } else {
    sMultiWord2MultiWord(u1, n1, t1, l1);
  }

  if (denNeg) {
    MultiWordNeg(u2, t2, n2);
  } else {
    sMultiWord2MultiWord(u2, n2, t2, l2);
  }

  if (uMultiWordDiv(t1, l1, t2, l2, y1, m1, y2, m2) < 0) {
    if (numNeg) {
      MultiWordSetSignedMin(y1, m1);
    } else {
      MultiWordSetSignedMax(y1, m1);
    }
  } else if ((boolean_T)(numNeg ^ denNeg)) {
    if (MultiWord2Bool(y2, m2)) {
      uMultiWordInc(y1, m1);
    }

    MultiWordNeg(y1, y1, m1);
  }
}

void MultiWordNeg(const uint32_T u1[], uint32_T y[], int32_T n)
{
  int32_T i;
  uint32_T carry = 1U;
  for (i = 0; i < n; i++) {
    uint32_T yi;
    yi = ~u1[i] + carry;
    y[i] = yi;
    carry = (uint32_T)(yi < carry);
  }
}

void MultiWordSetSignedMin(uint32_T y[], int32_T n)
{
  int32_T i;
  int32_T n1;
  n1 = n - 1;
  for (i = 0; i < n1; i++) {
    y[i] = 0U;
  }

  y[n - 1] = 2147483648U;
}

void MultiWordSetSignedMax(uint32_T y[], int32_T n)
{
  int32_T i;
  int32_T n1;
  n1 = n - 1;
  for (i = 0; i < n1; i++) {
    y[i] = MAX_uint32_T;
  }

  y[n - 1] = 2147483647U;
}

void uMultiWordInc(uint32_T y[], int32_T n)
{
  int32_T i;
  uint32_T carry = 1U;
  for (i = 0; i < n; i++) {
    uint32_T yi;
    yi = y[i] + carry;
    y[i] = yi;
    carry = (uint32_T)(yi < carry);
  }
}

boolean_T MultiWord2Bool(const uint32_T u[], int32_T n)
{
  int32_T i;
  boolean_T y;
  y = false;
  i = 0;
  while ((i < n) && !y) {
    if (u[i] != 0U) {
      y = true;
    }

    i++;
  }

  return y;
}

int32_T uMultiWordDiv(uint32_T a[], int32_T na, uint32_T b[], int32_T nb,
                      uint32_T q[], int32_T nq, uint32_T r[], int32_T nr)
{
  int32_T ka;
  int32_T kr;
  int32_T nzb;
  int32_T tpi;
  int32_T y;
  nzb = nb;
  tpi = nb - 1;
  while ((nzb > 0) && (b[tpi] == 0U)) {
    nzb--;
    tpi--;
  }

  if (nzb > 0) {
    int32_T nza;
    nza = na;
    for (tpi = 0; tpi < nq; tpi++) {
      q[tpi] = 0U;
    }

    tpi = na - 1;
    while ((nza > 0) && (a[tpi] == 0U)) {
      nza--;
      tpi--;
    }

    if ((nza > 0) && (nza >= nzb)) {
      int32_T na1;
      int32_T nb1;
      nb1 = nzb - 1;
      na1 = nza - 1;
      for (kr = 0; kr < nr; kr++) {
        r[kr] = 0U;
      }

      /* Quick return if dividend and divisor fit into single word. */
      if (nza == 1) {
        uint32_T ak;
        uint32_T bk;
        uint32_T nbq;
        ak = a[0];
        bk = b[0];
        nbq = ak / bk;
        q[0] = nbq;
        r[0] = ak - nbq * bk;
        y = 7;
      } else {
        uint32_T kba;
        uint32_T kbb;
        uint32_T t;

        /* Remove leading zeros from both, dividend and divisor. */
        kbb = 1U;
        t = b[nzb - 1] >> 1U;
        while (t != 0U) {
          kbb++;
          t >>= 1U;
        }

        kba = 1U;
        t = a[nza - 1] >> 1U;
        while (t != 0U) {
          kba++;
          t >>= 1U;
        }

        /* Quick return if quotient is zero. */
        if ((nza > nzb) || (kba >= kbb)) {
          uint32_T ak;
          uint32_T bk;
          uint32_T mask;
          uint32_T nba;
          uint32_T nbb;
          uint32_T nbq;
          uint32_T tnb;
          nba = ((uint32_T)(nza - 1) << 5) + kba;
          nbb = ((uint32_T)(nzb - 1) << 5) + kbb;

          /* Normalize b. */
          if (kbb != 32U) {
            bk = b[nzb - 1];
            for (kr = nzb - 1; kr > 0; kr--) {
              t = bk << (32U - kbb);
              bk = b[kr - 1];
              t |= bk >> kbb;
              b[kr] = t;
            }

            b[kr] = bk << (32U - kbb);
            mask = ~((1U << (32U - kbb)) - 1U);
          } else {
            mask = MAX_uint32_T;
          }

          /* Initialize quotient to zero. */
          tnb = 0U;
          y = 0;

          /* Until exit conditions have been met, do */
          do {
            /* Normalize a */
            if (kba != 32U) {
              tnb = (tnb - kba) + 32U;
              ak = a[na1];
              for (ka = na1; ka > 0; ka--) {
                t = ak << (32U - kba);
                ak = a[ka - 1];
                t |= ak >> kba;
                a[ka] = t;
              }

              a[ka] = ak << (32U - kba);
            }

            /* Compare b against the a. */
            ak = a[na1];
            bk = b[nzb - 1];
            if (((nzb - 1 == 0 ? mask : MAX_uint32_T) & ak) == bk) {
              tpi = 0;
              ka = na1;
              kr = nzb - 1;
              while ((tpi == 0) && (kr > 0)) {
                ka--;
                ak = a[ka];
                kr--;
                bk = b[kr];
                if (((kr == 0 ? mask : MAX_uint32_T) & ak) != bk) {
                  tpi = ak > bk ? 1 : -1;
                }
              }
            } else {
              tpi = ak > bk ? 1 : -1;
            }

            /* If the remainder in a is still greater or equal to b, subtract normalized divisor from a. */
            if ((tpi >= 0) || (nba > nbb)) {
              nbq = nba - nbb;

              /* If the remainder and the divisor are equal, set remainder to zero. */
              if (tpi == 0) {
                ka = na1;
                for (kr = nzb - 1; kr > 0; kr--) {
                  a[ka] = 0U;
                  ka--;
                }

                a[ka] -= b[kr];
              } else {
                /* Otherwise, subtract the divisor from the remainder */
                if (tpi < 0) {
                  ak = a[na1];
                  kba = 31U;
                  for (ka = na1; ka > 0; ka--) {
                    t = ak << 1U;
                    ak = a[ka - 1];
                    t |= ak >> 31U;
                    a[ka] = t;
                  }

                  a[ka] = ak << 1U;
                  tnb++;
                  nbq--;
                }

                bk = 0U;
                ka = (na1 - nzb) + 1;
                for (kr = 0; kr < nzb; kr++) {
                  t = a[ka];
                  ak = (t - b[kr]) - bk;
                  bk = bk != 0U ? (uint32_T)(ak >= t) : (uint32_T)(ak > t);
                  a[ka] = ak;
                  ka++;
                }
              }

              /* Update the quotient. */
              tpi = (int32_T)(nbq >> 5);
              q[tpi] |= 1U << (nbq - ((uint32_T)tpi << 5));

              /* Remove leading zeros from the remainder and check whether the exit conditions have been met. */
              tpi = na1;
              while ((nza > 0) && (a[tpi] == 0U)) {
                nza--;
                tpi--;
              }

              if (nza >= nzb) {
                na1 = nza - 1;
                kba = 1U;
                t = a[nza - 1] >> 1U;
                while (t != 0U) {
                  kba++;
                  t >>= 1U;
                }

                nba = (((uint32_T)(nza - 1) << 5) + kba) - tnb;
                if (nba < nbb) {
                  y = 2;
                }
              } else if (nza == 0) {
                y = 1;
              } else {
                na1 = nza - 1;
                y = 4;
              }
            } else {
              y = 3;
            }
          } while (y == 0);

          /* Return the remainder. */
          if (y == 1) {
            r[0] = a[0];
          } else {
            tpi = (int32_T)(tnb >> 5);
            nbq = tnb - ((uint32_T)tpi << 5);
            if (nbq == 0U) {
              ka = tpi;
              for (kr = 0; kr <= nb1; kr++) {
                r[kr] = a[ka];
                ka++;
              }
            } else {
              ak = a[tpi];
              kr = 0;
              for (ka = tpi + 1; ka <= na1; ka++) {
                t = ak >> nbq;
                ak = a[ka];
                t |= ak << (32U - nbq);
                r[kr] = t;
                kr++;
              }

              r[kr] = ak >> nbq;
            }
          }

          /* Restore b. */
          if (kbb != 32U) {
            bk = b[0];
            for (kr = 0; kr < nb1; kr++) {
              t = bk >> (32U - kbb);
              bk = b[kr + 1];
              t |= bk << kbb;
              b[kr] = t;
            }

            b[kr] = bk >> (32U - kbb);
          }
        } else {
          for (kr = 0; kr < nr; kr++) {
            r[kr] = a[kr];
          }

          y = 6;
        }
      }
    } else {
      for (kr = 0; kr < nr; kr++) {
        r[kr] = a[kr];
      }

      y = 5;
    }
  } else {
    y = -1;
  }

  return y;
}

void MultiWordAdd(const uint32_T u1[], const uint32_T u2[], uint32_T y[],
                  int32_T n)
{
  int32_T i;
  uint32_T carry = 0U;
  for (i = 0; i < n; i++) {
    uint32_T u1i;
    uint32_T yi;
    u1i = u1[i];
    yi = (u1i + u2[i]) + carry;
    y[i] = yi;
    carry = carry != 0U ? (uint32_T)(yi <= u1i) : (uint32_T)(yi < u1i);
  }
}

void sMultiWordMul(const uint32_T u1[], int32_T n1, const uint32_T u2[], int32_T
                   n2, uint32_T y[], int32_T n)
{
  int32_T i;
  int32_T j;
  int32_T k;
  uint32_T cb;
  uint32_T cb1;
  uint32_T yk;
  boolean_T isNegative1;
  boolean_T isNegative2;
  isNegative1 = ((u1[n1 - 1] & 2147483648U) != 0U);
  isNegative2 = ((u2[n2 - 1] & 2147483648U) != 0U);
  cb1 = 1U;

  /* Initialize output to zero */
  for (k = 0; k < n; k++) {
    y[k] = 0U;
  }

  for (i = 0; i < n1; i++) {
    int32_T ni;
    uint32_T a0;
    uint32_T a1;
    uint32_T cb2;
    uint32_T u1i;
    cb = 0U;
    u1i = u1[i];
    if (isNegative1) {
      u1i = ~u1i + cb1;
      cb1 = (uint32_T)(u1i < cb1);
    }

    a1 = u1i >> 16U;
    a0 = u1i & 65535U;
    cb2 = 1U;
    ni = n - i;
    ni = n2 <= ni ? n2 : ni;
    k = i;
    for (j = 0; j < ni; j++) {
      uint32_T b1;
      uint32_T w01;
      uint32_T w10;
      u1i = u2[j];
      if (isNegative2) {
        u1i = ~u1i + cb2;
        cb2 = (uint32_T)(u1i < cb2);
      }

      b1 = u1i >> 16U;
      u1i &= 65535U;
      w10 = a1 * u1i;
      w01 = a0 * b1;
      yk = y[k] + cb;
      cb = (uint32_T)(yk < cb);
      u1i *= a0;
      yk += u1i;
      cb += (uint32_T)(yk < u1i);
      u1i = w10 << 16U;
      yk += u1i;
      cb += (uint32_T)(yk < u1i);
      u1i = w01 << 16U;
      yk += u1i;
      cb += (uint32_T)(yk < u1i);
      y[k] = yk;
      cb += w10 >> 16U;
      cb += w01 >> 16U;
      cb += a1 * b1;
      k++;
    }

    if (k < n) {
      y[k] = cb;
    }
  }

  /* Apply sign */
  if (isNegative1 != isNegative2) {
    cb = 1U;
    for (k = 0; k < n; k++) {
      yk = ~y[k] + cb;
      y[k] = yk;
      cb = (uint32_T)(yk < cb);
    }
  }
}

void uLong2MultiWord(uint32_T u, uint32_T y[], int32_T n)
{
  int32_T i;
  y[0] = u;
  for (i = 1; i < n; i++) {
    y[i] = 0U;
  }
}

boolean_T sMultiWordGe(const uint32_T u1[], const uint32_T u2[], int32_T n)
{
  return sMultiWordCmp(u1, u2, n) >= 0;
}

boolean_T sMultiWordEq(const uint32_T u1[], const uint32_T u2[], int32_T n)
{
  return sMultiWordCmp(u1, u2, n) == 0;
}

void sMultiWordShr(const uint32_T u1[], int32_T n1, uint32_T n2, uint32_T y[],
                   int32_T n)
{
  int32_T i;
  int32_T i1;
  int32_T nb;
  uint32_T ys;
  nb = (int32_T)(n2 >> 5);
  i = 0;
  ys = (u1[n1 - 1] & 2147483648U) != 0U ? MAX_uint32_T : 0U;
  if (nb < n1) {
    int32_T nc;
    uint32_T nr;
    nc = n + nb;
    if (nc > n1) {
      nc = n1;
    }

    nr = n2 - ((uint32_T)nb << 5);
    if (nr > 0U) {
      uint32_T u1i;
      uint32_T yi;
      u1i = u1[nb];
      for (i1 = nb + 1; i1 < nc; i1++) {
        yi = u1i >> nr;
        u1i = u1[i1];
        y[i] = u1i << (32U - nr) | yi;
        i++;
      }

      yi = u1i >> nr;
      u1i = nc < n1 ? u1[nc] : ys;
      y[i] = u1i << (32U - nr) | yi;
      i++;
    } else {
      for (i1 = nb; i1 < nc; i1++) {
        y[i] = u1[i1];
        i++;
      }
    }
  }

  while (i < n) {
    y[i] = ys;
    i++;
  }
}

void MultiWordSub(const uint32_T u1[], const uint32_T u2[], uint32_T y[],
                  int32_T n)
{
  int32_T i;
  uint32_T borrow = 0U;
  for (i = 0; i < n; i++) {
    uint32_T u1i;
    uint32_T yi;
    u1i = u1[i];
    yi = (u1i - u2[i]) - borrow;
    y[i] = yi;
    borrow = borrow != 0U ? (uint32_T)(yi >= u1i) : (uint32_T)(yi > u1i);
  }
}

int32_T MultiWord2sLong(const uint32_T u[])
{
  return (int32_T)u[0];
}

boolean_T sMultiWordLe(const uint32_T u1[], const uint32_T u2[], int32_T n)
{
  return sMultiWordCmp(u1, u2, n) <= 0;
}

int32_T sMultiWord2sLongSat(const uint32_T u1[], int32_T n1)
{
  uint32_T y;
  sMultiWord2sMultiWordSat(u1, n1, &y, 1);
  return (int32_T)y;
}

/* Function for MATLAB Function: '<Root>/RRC_DOB_FixedStep' */
static void rrc_dob_vo_localScaleStateValue(int32_T b_value, const int64m_T
  ratio_Q30, int32_T *scaledValue, boolean_T *valid)
{
  int128m_T tmp;
  int64m_T magnitude;
  int64m_T saturatedUnaryMinus;
  int64m_T tmp_0;
  int64m_T tmp_1;
  int64m_T tmp_2;
  int96m_T tmp_3;
  int96m_T tmp_4;
  int96m_T tmp_5;
  boolean_T roundingValid;
  static const int64m_T tmp_6 = { { 0U, 0U }/* chunks */
  };

  static const int64m_T tmp_7 = { { 3758096383U, 2147483647U }/* chunks */
  };

  static const int64m_T tmp_8 = { { 2147483647U, 0U }/* chunks */
  };

  static const int64m_T tmp_9 = { { 2147483648U, MAX_uint32_T }/* chunks */
  };

  static const int64m_T tmp_a = { { 536870912U, 0U }/* chunks */
  };

  static const int64m_T tmp_b = { { 0U, 2147483648U }/* chunks */
  };

  static const int64m_T tmp_c = { { MAX_uint32_T, 2147483647U }/* chunks */
  };

  sLong2MultiWord(b_value, &tmp_0.chunks[0U], 2);
  sMultiWordMul(&tmp_0.chunks[0U], 2, &ratio_Q30.chunks[0U], 2, &tmp.chunks[0U],
                4);
  sMultiWord2sMultiWordSat(&tmp.chunks[0U], 4, &magnitude.chunks[0U], 2);
  roundingValid = true;
  if (sMultiWordGe(&magnitude.chunks[0U], &tmp_6.chunks[0U], 2)) {
    if (sMultiWordGt(&magnitude.chunks[0U], &tmp_7.chunks[0U], 2)) {
      roundingValid = false;
      magnitude = tmp_6;
    } else {
      sMultiWord2MultiWord(&magnitude.chunks[0U], 2, &tmp_4.chunks[0U], 3);
      sMultiWord2MultiWord(&tmp_a.chunks[0U], 2, &tmp_5.chunks[0U], 3);
      MultiWordAdd(&tmp_4.chunks[0U], &tmp_5.chunks[0U], &tmp_3.chunks[0U], 3);
      sMultiWord2sMultiWordSat(&tmp_3.chunks[0U], 3, &tmp_2.chunks[0U], 2);
      sMultiWordShr(&tmp_2.chunks[0U], 2, 30U, &magnitude.chunks[0U], 2);
    }
  } else if (sMultiWordEq(&magnitude.chunks[0U], &tmp_b.chunks[0U], 2)) {
    roundingValid = false;
    magnitude = tmp_6;
  } else {
    if (sMultiWordLe(&magnitude.chunks[0U], &tmp_b.chunks[0U], 2)) {
      saturatedUnaryMinus = tmp_c;
    } else {
      MultiWordNeg(&magnitude.chunks[0U], &saturatedUnaryMinus.chunks[0U], 2);
    }

    magnitude = saturatedUnaryMinus;
    if (sMultiWordGt(&saturatedUnaryMinus.chunks[0U], &tmp_7.chunks[0U], 2)) {
      roundingValid = false;
      magnitude = tmp_6;
    } else {
      sMultiWord2MultiWord(&saturatedUnaryMinus.chunks[0U], 2, &tmp_4.chunks[0U],
                           3);
      sMultiWord2MultiWord(&tmp_a.chunks[0U], 2, &tmp_5.chunks[0U], 3);
      MultiWordAdd(&tmp_4.chunks[0U], &tmp_5.chunks[0U], &tmp_3.chunks[0U], 3);
      sMultiWord2sMultiWordSat(&tmp_3.chunks[0U], 3, &tmp_2.chunks[0U], 2);
      sMultiWordShr(&tmp_2.chunks[0U], 2, 30U, &tmp_1.chunks[0U], 2);
      MultiWordNeg(&tmp_1.chunks[0U], &magnitude.chunks[0U], 2);
    }
  }

  *valid = (roundingValid && sMultiWordLe(&magnitude.chunks[0U], &tmp_8.chunks
             [0U], 2) && sMultiWordGe(&magnitude.chunks[0U], &tmp_9.chunks[0U],
             2));
  if (*valid) {
    *scaledValue = sMultiWord2sLongSat(&magnitude.chunks[0U], 2);
  } else {
    *scaledValue = 0;
  }
}

/* Function for MATLAB Function: '<Root>/RRC_DOB_FixedStep' */
static void rrc_dob_voltage_localAccumulate(const int64m_T accumulator, const
  int64m_T term, int64m_T *result, boolean_T *valid)
{
  int64m_T tmp;
  int96m_T tmp_0;
  int96m_T tmp_1;
  int96m_T tmp_2;
  static const int64m_T tmp_3 = { { 0U, 0U }/* chunks */
  };

  static const int64m_T tmp_4 = { { MAX_uint32_T, 2147483647U }/* chunks */
  };

  static const int64m_T tmp_5 = { { 0U, 2147483648U }/* chunks */
  };

  if (sMultiWordGt(&term.chunks[0U], &tmp_3.chunks[0U], 2)) {
    sMultiWord2MultiWord(&tmp_4.chunks[0U], 2, &tmp_1.chunks[0U], 3);
    sMultiWord2MultiWord(&term.chunks[0U], 2, &tmp_2.chunks[0U], 3);
    MultiWordSub(&tmp_1.chunks[0U], &tmp_2.chunks[0U], &tmp_0.chunks[0U], 3);
    sMultiWord2sMultiWordSat(&tmp_0.chunks[0U], 3, &tmp.chunks[0U], 2);
    *valid = sMultiWordLe(&accumulator.chunks[0U], &tmp.chunks[0U], 2);
  } else {
    sMultiWord2MultiWord(&tmp_5.chunks[0U], 2, &tmp_1.chunks[0U], 3);
    sMultiWord2MultiWord(&term.chunks[0U], 2, &tmp_2.chunks[0U], 3);
    MultiWordSub(&tmp_1.chunks[0U], &tmp_2.chunks[0U], &tmp_0.chunks[0U], 3);
    sMultiWord2sMultiWordSat(&tmp_0.chunks[0U], 3, &tmp.chunks[0U], 2);
    *valid = (sMultiWordGe(&term.chunks[0U], &tmp_3.chunks[0U], 2) ||
              sMultiWordGe(&accumulator.chunks[0U], &tmp.chunks[0U], 2));
  }

  if (*valid) {
    sMultiWord2MultiWord(&accumulator.chunks[0U], 2, &tmp_1.chunks[0U], 3);
    *result = term;
    sMultiWord2MultiWord(&term.chunks[0U], 2, &tmp_2.chunks[0U], 3);
    MultiWordAdd(&tmp_1.chunks[0U], &tmp_2.chunks[0U], &tmp_0.chunks[0U], 3);
    sMultiWord2sMultiWordSat(&tmp_0.chunks[0U], 3, &result->chunks[0U], 2);
  } else {
    *result = tmp_3;
  }
}

/* Function for MATLAB Function: '<Root>/RRC_DOB_FixedStep' */
static void rrc_dob_voltage_c_localAxisStep(const int32_T state[3], int16_T
  previousAppliedVoltage_Q15, int16_T current_Q15, const int32_T
  coefficients_Q27[19], int16_T *eHat_Q15, int32_T stateNext[3], boolean_T
  *valid)
{
  int128m_T tmp_0;
  int64m_T b_outputAccumulator;
  int64m_T outputAccumulator;
  int64m_T saturatedUnaryMinus;
  int64m_T saturatedUnaryMinus_0;
  int64m_T tmp;
  int64m_T tmp_1;
  int64m_T tmp_2;
  int64m_T tmp_3;
  int64m_T tmp_4;
  int64m_T tmp_8;
  int64m_T tmp_9;
  int64m_T tmp_a;
  int64m_T tmp_b;
  int64m_T tmp_c;
  int64m_T tmp_d;
  int64m_T tmp_e;
  int64m_T tmp_f;
  int64m_T tmp_g;
  int64m_T tmp_h;
  int96m_T tmp_5;
  int96m_T tmp_6;
  int96m_T tmp_7;
  int32_T coefficientOffset;
  int32_T inputOffset;
  int32_T rowIndex;
  boolean_T j_valid;
  static const int64m_T tmp_i = { { 0U, 0U }/* chunks */
  };

  static const int64m_T b_outputAccumulator_0 = { { 4227858431U, 2147483647U }/* chunks */
  };

  static const int64m_T tmp_j = { { 2147483647U, 0U }/* chunks */
  };

  static const int64m_T b_outputAccumulator_1 = { { 2147483648U, MAX_uint32_T }/* chunks */
  };

  static const int64m_T b_outputAccumulator_2 = { { 67108864U, 0U }/* chunks */
  };

  static const int64m_T b_outputAccumulator_3 = { { 0U, 2147483648U }/* chunks */
  };

  static const int64m_T tmp_k = { { MAX_uint32_T, 2147483647U }/* chunks */
  };

  static const int64m_T tmp_l = { { MAX_uint32_T, 2147483615U }/* chunks */
  };

  static const int64m_T tmp_m = { { 32767U, 0U }/* chunks */
  };

  static const int64m_T tmp_n = { { 4294934528U, MAX_uint32_T }/* chunks */
  };

  static const int64m_T tmp_o = { { 0U, 32U }/* chunks */
  };

  int32_T exitg1;
  int32_T exitg2;
  stateNext[0] = 0;
  stateNext[1] = 0;
  stateNext[2] = 0;
  *eHat_Q15 = 0;
  rowIndex = 0;
  do {
    exitg2 = 0;
    if (rowIndex < 3) {
      coefficientOffset = rowIndex * 3;
      inputOffset = (rowIndex << 1) + 9;
      sLong2MultiWord(coefficients_Q27[coefficientOffset], &tmp_9.chunks[0U], 2);
      sLong2MultiWord(state[0], &tmp_a.chunks[0U], 2);
      sMultiWordMul(&tmp_9.chunks[0U], 2, &tmp_a.chunks[0U], 2, &tmp_0.chunks[0U],
                    4);
      sMultiWord2MultiWord(&tmp_0.chunks[0U], 4, &tmp_9.chunks[0U], 2);
      rrc_dob_voltage_localAccumulate(tmp_i, tmp_9, &outputAccumulator, valid);
      if (!*valid) {
        exitg2 = 1;
      } else {
        sLong2MultiWord(coefficients_Q27[coefficientOffset + 1], &tmp_a.chunks
                        [0U], 2);
        sLong2MultiWord(state[1], &tmp_b.chunks[0U], 2);
        sMultiWordMul(&tmp_a.chunks[0U], 2, &tmp_b.chunks[0U], 2, &tmp_0.chunks
                      [0U], 4);
        sMultiWord2MultiWord(&tmp_0.chunks[0U], 4, &tmp_a.chunks[0U], 2);
        rrc_dob_voltage_localAccumulate(outputAccumulator, tmp_a,
          &b_outputAccumulator, valid);
        if (!*valid) {
          exitg2 = 1;
        } else {
          sLong2MultiWord(coefficients_Q27[coefficientOffset + 2],
                          &tmp_b.chunks[0U], 2);
          sLong2MultiWord(state[2], &tmp_c.chunks[0U], 2);
          sMultiWordMul(&tmp_b.chunks[0U], 2, &tmp_c.chunks[0U], 2,
                        &tmp_0.chunks[0U], 4);
          sMultiWord2MultiWord(&tmp_0.chunks[0U], 4, &tmp_b.chunks[0U], 2);
          rrc_dob_voltage_localAccumulate(b_outputAccumulator, tmp_b,
            &outputAccumulator, valid);
          if (!*valid) {
            exitg2 = 1;
          } else {
            sLong2MultiWord(coefficients_Q27[inputOffset], &tmp_d.chunks[0U], 2);
            sLong2MultiWord(previousAppliedVoltage_Q15, &tmp_e.chunks[0U], 2);
            sMultiWordMul(&tmp_d.chunks[0U], 2, &tmp_e.chunks[0U], 2,
                          &tmp_0.chunks[0U], 4);
            sMultiWord2MultiWord(&tmp_0.chunks[0U], 4, &tmp_c.chunks[0U], 2);
            sMultiWordShl(&tmp_c.chunks[0U], 2, 11U, &tmp_d.chunks[0U], 2);
            rrc_dob_voltage_localAccumulate(outputAccumulator, tmp_d,
              &b_outputAccumulator, valid);
            if (!*valid) {
              exitg2 = 1;
            } else {
              sLong2MultiWord(coefficients_Q27[inputOffset + 1], &tmp_e.chunks
                              [0U], 2);
              sLong2MultiWord(current_Q15, &tmp_f.chunks[0U], 2);
              sMultiWordMul(&tmp_e.chunks[0U], 2, &tmp_f.chunks[0U], 2,
                            &tmp_0.chunks[0U], 4);
              sMultiWord2MultiWord(&tmp_0.chunks[0U], 4, &tmp_c.chunks[0U], 2);
              sMultiWordShl(&tmp_c.chunks[0U], 2, 11U, &tmp_e.chunks[0U], 2);
              rrc_dob_voltage_localAccumulate(b_outputAccumulator, tmp_e,
                &outputAccumulator, valid);
              if (!*valid) {
                exitg2 = 1;
              } else {
                j_valid = true;
                tmp_c = tmp_i;
                if (sMultiWordGe(&outputAccumulator.chunks[0U], &tmp_i.chunks[0U],
                                 2)) {
                  tmp_g = b_outputAccumulator_0;
                  if (sMultiWordGt(&outputAccumulator.chunks[0U],
                                   &b_outputAccumulator_0.chunks[0U], 2)) {
                    j_valid = false;
                    outputAccumulator = tmp_i;
                  } else {
                    sMultiWord2MultiWord(&outputAccumulator.chunks[0U], 2,
                                         &tmp_6.chunks[0U], 3);
                    sMultiWord2MultiWord(&b_outputAccumulator_2.chunks[0U], 2,
                                         &tmp_7.chunks[0U], 3);
                    MultiWordAdd(&tmp_6.chunks[0U], &tmp_7.chunks[0U],
                                 &tmp_5.chunks[0U], 3);
                    sMultiWord2sMultiWordSat(&tmp_5.chunks[0U], 3,
                      &tmp_h.chunks[0U], 2);
                    sMultiWordShr(&tmp_h.chunks[0U], 2, 27U,
                                  &outputAccumulator.chunks[0U], 2);
                  }
                } else {
                  tmp_f = b_outputAccumulator_3;
                  if (sMultiWordEq(&outputAccumulator.chunks[0U],
                                   &b_outputAccumulator_3.chunks[0U], 2)) {
                    j_valid = false;
                    outputAccumulator = tmp_i;
                  } else {
                    if (sMultiWordLe(&outputAccumulator.chunks[0U],
                                     &b_outputAccumulator_3.chunks[0U], 2)) {
                      saturatedUnaryMinus_0 = tmp_k;
                    } else {
                      MultiWordNeg(&outputAccumulator.chunks[0U],
                                   &saturatedUnaryMinus_0.chunks[0U], 2);
                    }

                    outputAccumulator = saturatedUnaryMinus_0;
                    if (sMultiWordGt(&saturatedUnaryMinus_0.chunks[0U],
                                     &b_outputAccumulator_0.chunks[0U], 2)) {
                      j_valid = false;
                      outputAccumulator = tmp_i;
                    } else {
                      sMultiWord2MultiWord(&saturatedUnaryMinus_0.chunks[0U], 2,
                                           &tmp_6.chunks[0U], 3);
                      sMultiWord2MultiWord(&b_outputAccumulator_2.chunks[0U], 2,
                                           &tmp_7.chunks[0U], 3);
                      MultiWordAdd(&tmp_6.chunks[0U], &tmp_7.chunks[0U],
                                   &tmp_5.chunks[0U], 3);
                      sMultiWord2sMultiWordSat(&tmp_5.chunks[0U], 3,
                        &tmp_h.chunks[0U], 2);
                      sMultiWordShr(&tmp_h.chunks[0U], 2, 27U, &tmp_g.chunks[0U],
                                    2);
                      MultiWordNeg(&tmp_g.chunks[0U], &outputAccumulator.chunks
                                   [0U], 2);
                    }
                  }
                }

                tmp_h = tmp_j;
                if (!j_valid || sMultiWordGt(&outputAccumulator.chunks[0U],
                     &tmp_j.chunks[0U], 2) || sMultiWordLt
                    (&outputAccumulator.chunks[0U],
                     &b_outputAccumulator_1.chunks[0U], 2)) {
                  *valid = false;
                  exitg2 = 1;
                } else {
                  stateNext[rowIndex] = MultiWord2sLong
                    (&outputAccumulator.chunks[0U]);
                  rowIndex++;
                }
              }
            }
          }
        }
      }
    } else {
      outputAccumulator = tmp_i;
      rowIndex = 0;
      exitg2 = 2;
    }
  } while (exitg2 == 0);

  if (exitg2 == 1) {
  } else {
    do {
      exitg1 = 0;
      if (rowIndex < 3) {
        sLong2MultiWord(coefficients_Q27[rowIndex + 15], &tmp_8.chunks[0U], 2);
        sLong2MultiWord(state[rowIndex], &tmp_9.chunks[0U], 2);
        sMultiWordMul(&tmp_8.chunks[0U], 2, &tmp_9.chunks[0U], 2, &tmp_0.chunks
                      [0U], 4);
        sMultiWord2MultiWord(&tmp_0.chunks[0U], 4, &tmp_8.chunks[0U], 2);
        rrc_dob_voltage_localAccumulate(outputAccumulator, tmp_8,
          &outputAccumulator, valid);
        if (!*valid) {
          exitg1 = 1;
        } else {
          rowIndex++;
        }
      } else {
        sLong2MultiWord(coefficients_Q27[18], &tmp_1.chunks[0U], 2);
        sLong2MultiWord(current_Q15, &tmp_2.chunks[0U], 2);
        sMultiWordMul(&tmp_1.chunks[0U], 2, &tmp_2.chunks[0U], 2, &tmp_0.chunks
                      [0U], 4);
        sMultiWord2MultiWord(&tmp_0.chunks[0U], 4, &tmp.chunks[0U], 2);
        sMultiWordShl(&tmp.chunks[0U], 2, 11U, &tmp_1.chunks[0U], 2);
        rrc_dob_voltage_localAccumulate(outputAccumulator, tmp_1,
          &b_outputAccumulator, valid);
        if (*valid) {
          *valid = true;
          if (sMultiWordGe(&b_outputAccumulator.chunks[0U], &tmp_i.chunks[0U], 2))
          {
            if (sMultiWordGt(&b_outputAccumulator.chunks[0U], &tmp_l.chunks[0U],
                             2)) {
              *valid = false;
              outputAccumulator = tmp_i;
            } else {
              sMultiWord2MultiWord(&b_outputAccumulator.chunks[0U], 2,
                                   &tmp_6.chunks[0U], 3);
              sMultiWord2MultiWord(&tmp_o.chunks[0U], 2, &tmp_7.chunks[0U], 3);
              MultiWordAdd(&tmp_6.chunks[0U], &tmp_7.chunks[0U], &tmp_5.chunks
                           [0U], 3);
              sMultiWord2sMultiWordSat(&tmp_5.chunks[0U], 3, &tmp_4.chunks[0U],
                2);
              sMultiWordShr(&tmp_4.chunks[0U], 2, 38U,
                            &outputAccumulator.chunks[0U], 2);
            }
          } else if (sMultiWordEq(&b_outputAccumulator.chunks[0U],
                                  &b_outputAccumulator_3.chunks[0U], 2)) {
            *valid = false;
            outputAccumulator = tmp_i;
          } else {
            if (sMultiWordLe(&b_outputAccumulator.chunks[0U],
                             &b_outputAccumulator_3.chunks[0U], 2)) {
              saturatedUnaryMinus = tmp_k;
            } else {
              MultiWordNeg(&b_outputAccumulator.chunks[0U],
                           &saturatedUnaryMinus.chunks[0U], 2);
            }

            outputAccumulator = saturatedUnaryMinus;
            if (sMultiWordGt(&saturatedUnaryMinus.chunks[0U], &tmp_l.chunks[0U],
                             2)) {
              *valid = false;
              outputAccumulator = tmp_i;
            } else {
              sMultiWord2MultiWord(&saturatedUnaryMinus.chunks[0U], 2,
                                   &tmp_6.chunks[0U], 3);
              sMultiWord2MultiWord(&tmp_o.chunks[0U], 2, &tmp_7.chunks[0U], 3);
              MultiWordAdd(&tmp_6.chunks[0U], &tmp_7.chunks[0U], &tmp_5.chunks
                           [0U], 3);
              sMultiWord2sMultiWordSat(&tmp_5.chunks[0U], 3, &tmp_4.chunks[0U],
                2);
              sMultiWordShr(&tmp_4.chunks[0U], 2, 38U, &tmp_3.chunks[0U], 2);
              MultiWordNeg(&tmp_3.chunks[0U], &outputAccumulator.chunks[0U], 2);
            }
          }

          if (!*valid || sMultiWordGt(&outputAccumulator.chunks[0U],
               &tmp_m.chunks[0U], 2) || sMultiWordLt(&outputAccumulator.chunks
               [0U], &tmp_n.chunks[0U], 2)) {
            *valid = false;
          } else {
            *eHat_Q15 = (int16_T)MultiWord2sLong(&outputAccumulator.chunks[0U]);
          }
        }

        exitg1 = 1;
      }
    } while (exitg1 == 0);
  }
}

/* Model step function */
void rrc_dob_voltage_compensation_wrapper_step(void)
{
  int128m_T tmp_c;
  int64m_T limitedCorrection;
  int64m_T lowerLimit;
  int64m_T magnitude;
  int64m_T tmp;
  int64m_T tmp_2;
  int64m_T tmp_3;
  int64m_T tmp_4;
  int64m_T tmp_5;
  int64m_T tmp_6;
  int64m_T tmp_7;
  int64m_T tmp_8;
  int64m_T tmp_9;
  int64m_T tmp_a;
  int64m_T tmp_b;
  int96m_T tmp_0;
  int96m_T tmp_1;
  int32_T rtb_RRC_DOB_State[8];
  int32_T workingState[6];
  int32_T b[3];
  int32_T i;
  boolean_T dState2SecondValid;
  boolean_T dState3Valid;
  boolean_T qState2FirstValid;
  boolean_T qState2SecondValid;
  boolean_T qState3Valid;
  boolean_T sampleSaturated;
  static const int64m_T tmp_d = { { 2U, 0U }/* chunks */
  };

  static const int64m_T tmp_e = { { 0U, 0U }/* chunks */
  };

  static const int64m_T tmp_f = { { 1073741824U, 0U }/* chunks */
  };

  static const int64m_T tmp_g = { { 32767U, 0U }/* chunks */
  };

  static const int64m_T tmp_h = { { 4294934528U, MAX_uint32_T }/* chunks */
  };

  static const int64m_T tmp_i = { { 0U, 2147483648U }/* chunks */
  };

  int32_T exitg1;
  boolean_T guard1;
  boolean_T guard11;
  boolean_T guard2;

  /* MATLAB Function: '<Root>/RRC_DOB_FixedStep' */
  rrc_dob_voltage_compensation__Y.Error_Hat_DQ_Q15[0] = 0;
  rrc_dob_voltage_compensation__Y.Error_Hat_DQ_Q15[1] = 0;
  for (i = 0; i < 8; i++) {
    /* UnitDelay: '<Root>/RRC_DOB_State' */
    rtb_RRC_DOB_State[i] =
      rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[i];

    /* MATLAB Function: '<Root>/RRC_DOB_FixedStep' incorporates:
     *  UnitDelay: '<Root>/RRC_DOB_State'
     */
    rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[i] = 0;
  }

  /* Outport: '<Root>/Command_DQ_Q15' incorporates:
   *  Inport: '<Root>/Raw_PI_Voltage_DQ_Q15'
   *  MATLAB Function: '<Root>/RRC_DOB_FixedStep'
   */
  rrc_dob_voltage_compensation__Y.Command_DQ_Q15[0] =
    rrc_dob_voltage_compensation__U.Raw_PI_Voltage_DQ_Q15[0];
  rrc_dob_voltage_compensation__Y.Command_DQ_Q15[1] =
    rrc_dob_voltage_compensation__U.Raw_PI_Voltage_DQ_Q15[1];

  /* MATLAB Function: '<Root>/RRC_DOB_FixedStep' incorporates:
   *  Inport: '<Root>/Coefficients_DQ_Q27'
   *  Inport: '<Root>/Current_DQ_Q15'
   *  Inport: '<Root>/Output_Limit_Q15'
   *  Inport: '<Root>/Previous_Applied_Voltage_DQ_Q15'
   *  Inport: '<Root>/Q_Q29'
   *  Inport: '<Root>/Ramp_Q31'
   *  Inport: '<Root>/Raw_PI_Voltage_DQ_Q15'
   *  Inport: '<Root>/Reset_u8'
   *  Outport: '<Root>/Command_DQ_Q15'
   *  UnitDelay: '<Root>/RRC_DOB_State'
   */
  if (rrc_dob_voltage_compensation__U.Reset_u8 != 0) {
    /* Outport: '<Root>/Status_u8' */
    rrc_dob_voltage_compensation__Y.Status_u8 = 1U;
  } else if ((rrc_dob_voltage_compensation__U.Q_Q29 <= 0) ||
             (rrc_dob_voltage_compensation__U.Output_Limit_Q15 < 0) ||
             (rrc_dob_voltage_compensation__U.Ramp_Q31 > 2147483648U)) {
    /* Outport: '<Root>/Status_u8' */
    rrc_dob_voltage_compensation__Y.Status_u8 = 2U;
  } else if ((rtb_RRC_DOB_State[7] != 0) && (rtb_RRC_DOB_State[7] != 1)) {
    /* Outport: '<Root>/Status_u8' */
    rrc_dob_voltage_compensation__Y.Status_u8 = 2U;
  } else if (rtb_RRC_DOB_State[7] == 0) {
    rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[6] =
      rrc_dob_voltage_compensation__U.Q_Q29;
    rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[7] = 1;

    /* Outport: '<Root>/Status_u8' incorporates:
     *  Inport: '<Root>/Q_Q29'
     *  UnitDelay: '<Root>/RRC_DOB_State'
     */
    rrc_dob_voltage_compensation__Y.Status_u8 = 7U;
  } else if (rtb_RRC_DOB_State[6] <= 0) {
    /* Outport: '<Root>/Status_u8' */
    rrc_dob_voltage_compensation__Y.Status_u8 = 2U;
  } else {
    sLong2MultiWord(rrc_dob_voltage_compensation__U.Q_Q29, &tmp_2.chunks[0U], 2);
    sMultiWord2MultiWord(&tmp_2.chunks[0U], 2, &tmp_1.chunks[0U], 3);
    sMultiWordShl(&tmp_1.chunks[0U], 3, 1U, &tmp_0.chunks[0U], 3);
    sMultiWord2MultiWord(&tmp_0.chunks[0U], 3, &tmp.chunks[0U], 2);
    sLong2MultiWord(rtb_RRC_DOB_State[6], &tmp_2.chunks[0U], 2);
    guard1 = false;
    if (sMultiWordLt(&tmp.chunks[0U], &tmp_2.chunks[0U], 2)) {
      guard1 = true;
    } else {
      sLong2MultiWord(rrc_dob_voltage_compensation__U.Q_Q29, &tmp_3.chunks[0U],
                      2);
      sLong2MultiWord(rtb_RRC_DOB_State[6], &tmp_5.chunks[0U], 2);
      sMultiWord2MultiWord(&tmp_5.chunks[0U], 2, &tmp_1.chunks[0U], 3);
      sMultiWordShl(&tmp_1.chunks[0U], 3, 1U, &tmp_0.chunks[0U], 3);
      sMultiWord2MultiWord(&tmp_0.chunks[0U], 3, &tmp_4.chunks[0U], 2);
      if (sMultiWordGt(&tmp_3.chunks[0U], &tmp_4.chunks[0U], 2)) {
        guard1 = true;
      } else {
        for (i = 0; i < 6; i++) {
          workingState[i] = rtb_RRC_DOB_State[i];
        }

        guard11 = false;
        if (rrc_dob_voltage_compensation__U.Q_Q29 != rtb_RRC_DOB_State[6]) {
          sLong2MultiWord(rrc_dob_voltage_compensation__U.Q_Q29, &tmp_7.chunks
                          [0U], 2);
          sMultiWordShl(&tmp_7.chunks[0U], 2, 30U, &tmp_6.chunks[0U], 2);
          sLong2MultiWord(rtb_RRC_DOB_State[6], &tmp_8.chunks[0U], 2);
          tmp_2 = tmp_d;
          sMultiWordDivFloor(&tmp_8.chunks[0U], 2, &tmp_d.chunks[0U], 2,
                             &tmp_1.chunks[0U], 3, &tmp_9.chunks[0U], 2,
                             &tmp_a.chunks[0U], 2, &tmp_b.chunks[0U], 2);
          sMultiWord2MultiWord(&tmp_1.chunks[0U], 3, &tmp_7.chunks[0U], 2);
          MultiWordAdd(&tmp_6.chunks[0U], &tmp_7.chunks[0U], &tmp_5.chunks[0U],
                       2);
          sLong2MultiWord(rtb_RRC_DOB_State[6], &tmp_6.chunks[0U], 2);
          sMultiWordDivFloor(&tmp_5.chunks[0U], 2, &tmp_6.chunks[0U], 2,
                             &tmp_0.chunks[0U], 3, &tmp_7.chunks[0U], 2,
                             &tmp_8.chunks[0U], 2, &tmp_2.chunks[0U], 2);
          sMultiWord2sMultiWordSat(&tmp_0.chunks[0U], 3, &magnitude.chunks[0U],
            2);
          for (i = 0; i < 6; i++) {
            workingState[i] = rtb_RRC_DOB_State[i];
          }

          rrc_dob_vo_localScaleStateValue(rtb_RRC_DOB_State[1], magnitude,
            &workingState[1], &sampleSaturated);
          rrc_dob_vo_localScaleStateValue(workingState[1], magnitude,
            &workingState[1], &dState2SecondValid);
          rrc_dob_vo_localScaleStateValue(rtb_RRC_DOB_State[2], magnitude,
            &workingState[2], &dState3Valid);
          rrc_dob_vo_localScaleStateValue(rtb_RRC_DOB_State[4], magnitude,
            &workingState[4], &qState2FirstValid);
          rrc_dob_vo_localScaleStateValue(workingState[4], magnitude,
            &workingState[4], &qState2SecondValid);
          rrc_dob_vo_localScaleStateValue(rtb_RRC_DOB_State[5], magnitude,
            &workingState[5], &qState3Valid);
          if (!sampleSaturated || !dState2SecondValid || !dState3Valid ||
              !qState2FirstValid || !qState2SecondValid || !qState3Valid) {
            /* Outport: '<Root>/Status_u8' */
            rrc_dob_voltage_compensation__Y.Status_u8 = 5U;
          } else {
            guard11 = true;
          }
        } else {
          guard11 = true;
        }

        if (guard11) {
          rrc_dob_voltage_c_localAxisStep(&workingState[0],
            rrc_dob_voltage_compensation__U.Previous_Applied_Voltage_DQ_Q15[0],
            rrc_dob_voltage_compensation__U.Current_DQ_Q15[0],
            &rrc_dob_voltage_compensation__U.Coefficients_DQ_Q27[0],
            &rrc_dob_voltage_compensation__Y.Error_Hat_DQ_Q15[0], b,
            &sampleSaturated);
          rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[0] = b[0];
          rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[1] = b[1];
          rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[2] = b[2];
          rrc_dob_voltage_c_localAxisStep(&workingState[3],
            rrc_dob_voltage_compensation__U.Previous_Applied_Voltage_DQ_Q15[1],
            rrc_dob_voltage_compensation__U.Current_DQ_Q15[1],
            &rrc_dob_voltage_compensation__U.Coefficients_DQ_Q27[19],
            &rrc_dob_voltage_compensation__Y.Error_Hat_DQ_Q15[1], b,
            &dState2SecondValid);
          rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[3] = b[0];
          rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[4] = b[1];
          rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[5] = b[2];
          if (!sampleSaturated || !dState2SecondValid) {
            rrc_dob_voltage_compensation__Y.Error_Hat_DQ_Q15[0] = 0;
            rrc_dob_voltage_compensation__Y.Error_Hat_DQ_Q15[1] = 0;
            for (i = 0; i < 8; i++) {
              rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[i] = 0;
            }

            /* Outport: '<Root>/Status_u8' incorporates:
             *  UnitDelay: '<Root>/RRC_DOB_State'
             */
            rrc_dob_voltage_compensation__Y.Status_u8 = 5U;
          } else {
            rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[6] =
              rrc_dob_voltage_compensation__U.Q_Q29;
            rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[7] = 1;
            sampleSaturated = false;
            i = 0;
            do {
              exitg1 = 0;
              if (i < 2) {
                sLong2MultiWord
                  (rrc_dob_voltage_compensation__Y.Error_Hat_DQ_Q15[i],
                   &tmp_5.chunks[0U], 2);
                uLong2MultiWord(rrc_dob_voltage_compensation__U.Ramp_Q31,
                                &tmp_6.chunks[0U], 2);
                sMultiWordMul(&tmp_5.chunks[0U], 2, &tmp_6.chunks[0U], 2,
                              &tmp_c.chunks[0U], 4);
                sMultiWord2MultiWord(&tmp_c.chunks[0U], 4, &magnitude.chunks[0U],
                                     2);
                tmp_5 = tmp_e;
                guard2 = false;
                if (sMultiWordGe(&magnitude.chunks[0U], &tmp_e.chunks[0U], 2)) {
                  tmp_8 = tmp_f;
                  MultiWordAdd(&magnitude.chunks[0U], &tmp_f.chunks[0U],
                               &tmp_7.chunks[0U], 2);
                  sMultiWordShr(&tmp_7.chunks[0U], 2, 31U,
                                &limitedCorrection.chunks[0U], 2);
                  guard2 = true;
                } else {
                  tmp_6 = tmp_i;
                  if (sMultiWordEq(&magnitude.chunks[0U], &tmp_i.chunks[0U], 2))
                  {
                    /* Outport: '<Root>/Command_DQ_Q15' incorporates:
                     *  Inport: '<Root>/Raw_PI_Voltage_DQ_Q15'
                     */
                    rrc_dob_voltage_compensation__Y.Command_DQ_Q15[0] =
                      rrc_dob_voltage_compensation__U.Raw_PI_Voltage_DQ_Q15[0];
                    rrc_dob_voltage_compensation__Y.Error_Hat_DQ_Q15[0] = 0;

                    /* Outport: '<Root>/Command_DQ_Q15' incorporates:
                     *  Inport: '<Root>/Raw_PI_Voltage_DQ_Q15'
                     */
                    rrc_dob_voltage_compensation__Y.Command_DQ_Q15[1] =
                      rrc_dob_voltage_compensation__U.Raw_PI_Voltage_DQ_Q15[1];
                    rrc_dob_voltage_compensation__Y.Error_Hat_DQ_Q15[1] = 0;
                    for (i = 0; i < 8; i++) {
                      rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[i] =
                        0;
                    }

                    /* Outport: '<Root>/Status_u8' incorporates:
                     *  UnitDelay: '<Root>/RRC_DOB_State'
                     */
                    rrc_dob_voltage_compensation__Y.Status_u8 = 5U;
                    exitg1 = 1;
                  } else {
                    MultiWordSub(&tmp_f.chunks[0U], &magnitude.chunks[0U],
                                 &tmp_8.chunks[0U], 2);
                    sMultiWordShr(&tmp_8.chunks[0U], 2, 31U, &tmp_7.chunks[0U],
                                  2);
                    MultiWordNeg(&tmp_7.chunks[0U], &limitedCorrection.chunks[0U],
                                 2);
                    guard2 = true;
                  }
                }

                if (guard2) {
                  sLong2MultiWord
                    (rrc_dob_voltage_compensation__U.Output_Limit_Q15,
                     &magnitude.chunks[0U], 2);
                  MultiWordNeg(&magnitude.chunks[0U], &lowerLimit.chunks[0U], 2);
                  if (sMultiWordGt(&limitedCorrection.chunks[0U],
                                   &magnitude.chunks[0U], 2)) {
                    limitedCorrection = magnitude;
                    sampleSaturated = true;
                  } else if (sMultiWordLt(&limitedCorrection.chunks[0U],
                                          &lowerLimit.chunks[0U], 2)) {
                    limitedCorrection = lowerLimit;
                    sampleSaturated = true;
                  }

                  sLong2MultiWord
                    (rrc_dob_voltage_compensation__U.Raw_PI_Voltage_DQ_Q15[i],
                     &tmp_7.chunks[0U], 2);
                  MultiWordSub(&tmp_7.chunks[0U], &limitedCorrection.chunks[0U],
                               &magnitude.chunks[0U], 2);
                  tmp_7 = tmp_g;
                  if (sMultiWordGt(&magnitude.chunks[0U], &tmp_g.chunks[0U], 2))
                  {
                    magnitude = tmp_g;
                    sampleSaturated = true;
                  } else {
                    tmp_8 = tmp_h;
                    if (sMultiWordLt(&magnitude.chunks[0U], &tmp_h.chunks[0U], 2))
                    {
                      magnitude = tmp_h;
                      sampleSaturated = true;
                    }
                  }

                  rrc_dob_voltage_compensation__Y.Command_DQ_Q15[i] = (int16_T)
                    MultiWord2sLong(&magnitude.chunks[0U]);
                  i++;
                }
              } else {
                if (sampleSaturated) {
                  /* Outport: '<Root>/Status_u8' */
                  rrc_dob_voltage_compensation__Y.Status_u8 = 6U;
                } else {
                  /* Outport: '<Root>/Status_u8' */
                  rrc_dob_voltage_compensation__Y.Status_u8 = 0U;
                }

                exitg1 = 1;
              }
            } while (exitg1 == 0);
          }
        }
      }
    }

    if (guard1) {
      rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[6] =
        rrc_dob_voltage_compensation__U.Q_Q29;
      rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[7] = 1;

      /* Outport: '<Root>/Status_u8' incorporates:
       *  Inport: '<Root>/Q_Q29'
       *  UnitDelay: '<Root>/RRC_DOB_State'
       */
      rrc_dob_voltage_compensation__Y.Status_u8 = 7U;
    }
  }

  /* Outport: '<Root>/State_Q26' incorporates:
   *  UnitDelay: '<Root>/RRC_DOB_State'
   */
  for (i = 0; i < 8; i++) {
    rrc_dob_voltage_compensation__Y.State_Q26[i] =
      rrc_dob_voltage_compensation_DW.RRC_DOB_State_DSTATE[i];
  }

  /* End of Outport: '<Root>/State_Q26' */
}

/* Model initialize function */
void rrc_dob_voltage_compensation_wrapper_initialize(void)
{
  /* (no initialization code required) */
}

/* Model terminate function */
void rrc_dob_voltage_compensation_wrapper_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
