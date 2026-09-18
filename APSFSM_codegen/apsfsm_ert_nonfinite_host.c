/* Host-only implementation of the generated non-finite runtime contract.
 * TinyCC diagnoses its NAN/INFINITY constant macros as division by zero;
 * these comparisons are equivalent for IEEE-754 replay and leave the
 * generated model source itself unchanged. */
#include <float.h>

#include "rt_nonfinite.h"

boolean_T rtIsInf(real_T value)
{
    return (boolean_T)((value > DBL_MAX) || (value < -DBL_MAX));
}

boolean_T rtIsInfF(real32_T value)
{
    return (boolean_T)((value > FLT_MAX) || (value < -FLT_MAX));
}

boolean_T rtIsNaN(real_T value)
{
    return (boolean_T)(value != value);
}

boolean_T rtIsNaNF(real32_T value)
{
    return (boolean_T)(value != value);
}
