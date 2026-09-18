/* Host-only TinyCC bridge to the double-precision MSVCRT math exports. */
extern double cos(double value);
extern double floor(double value);
extern double sin(double value);
extern double sqrt(double value);

float cosf(float value)
{
    return (float)cos((double)value);
}

float floorf(float value)
{
    return (float)floor((double)value);
}

float sinf(float value)
{
    return (float)sin((double)value);
}

float sqrtf(float value)
{
    return (float)sqrt((double)value);
}
