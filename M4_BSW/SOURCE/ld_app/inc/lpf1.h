/*
 * lpf1.h
 *
 *  Generic first-order IIR low-pass filter (floating-point, reusable).
 *
 *  Discrete difference equation (Ts is the call period):
 *      y[n] = y[n-1] + alpha * (x[n] - y[n-1])
 *
 *  alpha is the discrete coefficient — it is NOT dimensionless; it is tied
 *  to the Ts you actually call LPF1_update() with. Changing the call period
 *  requires recomputing alpha (see LPF1_alpha_from_fc() below).
 *
 *  Two equivalent ways to derive alpha:
 *    1. Time-constant form:
 *         alpha = Ts / (tau + Ts),     tau = desired RC time constant [s]
 *       Approximate (tau >> Ts):       tau ≈ (1/alpha - 1) * Ts
 *         e.g. Ts=0.1s, alpha=0.1  →  tau ≈ 0.9s
 *              Ts=0.1s, alpha=0.0625  → tau ≈ 1.5s
 *    2. Cutoff-frequency form (exact bilinear/equivalent):
 *         alpha = 1 - exp(-2*pi*fc*Ts), fc = -3dB cutoff [Hz]
 *
 *  Usage:
 *      LPF1_t flt;
 *      // Option A: directly set discrete alpha (fastest, Ts implied)
 *      LPF1_init(&flt, 0.1f, 0.0f);
 *      // Option B: derive alpha from physical fc and the actual Ts
 *      float a = LPF1_alpha_from_fc(1.6f, 0.1f);   // fc=1.6Hz, Ts=100ms
 *      LPF1_init(&flt, a, 0.0f);
 *      float y = LPF1_update(&flt, x);   // call once per Ts
 *      LPF1_reset(&flt);                 // back to init value
 *
 *  Note: alpha in (0, 1]; smaller = smoother (more lag), larger = faster.
 *  alpha = 1.0 → no filtering (y = x).  alpha → 0 → infinite smoothing.
 */

#ifndef LPF1_H
#define LPF1_H

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

/* M_PI may not be defined on some toolchains */
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

typedef struct {
    float  y;            /* current filter output                  */
    float  alpha;        /* discrete smoothing coeff, (0, 1]       */
    float  init_val;     /* value used on init / reset             */
    bool   initialized;  /* set after first update                 */
} LPF1_t;

/* Compute the discrete alpha from a physical -3dB cutoff frequency
 * and the actual sample period.
 *   alpha = 1 - exp(-2*pi*fc*Ts)
 *   fc   - desired cutoff frequency [Hz]
 *   ts   - sample period of LPF1_update() calls [s]
 * Returns alpha in (0, 1]. fc<=0 → alpha=1 (bypass); ts<=0 → alpha=1.
 *
 * Example: fc=1.6Hz, Ts=100ms → alpha≈0.1 → tau≈0.9s (as used in m4_rte.c)
 */
static inline float LPF1_alpha_from_fc(float fc_hz, float ts_s)
{
    if (fc_hz <= 0.0f || ts_s <= 0.0f) {
        return 1.0f;   /* bypass */
    }
    return 1.0f - expf(-2.0f * (float)M_PI * fc_hz * ts_s);
}

/* Compute alpha from a desired time constant instead of cutoff.
 *   alpha = Ts / (tau + Ts)
 *   tau  - desired RC time constant [s]
 *   ts   - sample period [s]
 */
static inline float LPF1_alpha_from_tau(float tau_s, float ts_s)
{
    if (tau_s <= 0.0f || ts_s <= 0.0f) {
        return 1.0f;   /* bypass */
    }
    return ts_s / (tau_s + ts_s);
}

/* Initialize a filter instance.
 * alpha    - discrete smoothing coefficient, must be in (0, 1].
 *            It is tied to the Ts you call LPF1_update() at. Derive it
 *            with LPF1_alpha_from_fc() / LPF1_alpha_from_tau() if you
 *            change Ts, so the cutoff frequency stays consistent.
 * init_val - initial output (used on init and reset)
 */
static inline void LPF1_init(LPF1_t *f, float alpha, float init_val)
{
    /* Clamp alpha to valid range; 0 would freeze the filter, >1 diverges. */
    if (alpha <= 0.0f)        alpha = 1.0f;   /* bypass rather than freeze */
    else if (alpha > 1.0f)    alpha = 1.0f;

    f->alpha       = alpha;
    f->init_val    = init_val;
    f->y           = init_val;
    f->initialized = true;
}

/* Reset filter output to its init value (keep alpha). */
static inline void LPF1_reset(LPF1_t *f)
{
    f->y           = f->init_val;
    f->initialized = true;
}

/* Push one input sample, return filtered output.
 * On the very first sample after init/reset, output = input (bypass)
 * so the filter locks onto the signal instantly instead of ramping
 * from init_val for hundreds of samples.
 */
static inline float LPF1_update(LPF1_t *f, float x)
{
    if (!f->initialized) {
        f->y           = x;
        f->initialized = true;
        return x;
    }
    f->y += f->alpha * (x - f->y);
    return f->y;
}

/* Convenience: set a new alpha at runtime (e.g. adaptive tuning).
 * Recompute with LPF1_alpha_from_fc() if Ts changes. */
static inline void LPF1_set_alpha(LPF1_t *f, float alpha)
{
    if (alpha <= 0.0f)      alpha = 1.0f;
    else if (alpha > 1.0f)  alpha = 1.0f;
    f->alpha = alpha;
}

#endif /* LPF1_H */