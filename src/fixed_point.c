/**
 * @file fixed_point.c
 * @brief Fixed-point arithmetic implementation — Q15 and Q31 operations.
 *
 * ── Saturation Arithmetic ────────────────────────────────────────
 *
 *  In fixed-point DSP, overflow must be detected and clamped
 *  (saturated) to prevent wrap-around artefacts:
 *
 *     Addition:                   Multiplication:
 *
 *     +1.0 ─┬─── saturate ───    Q15: a*b → 32-bit intermediate
 *            │   /                     → shift right by 15
 *     result │  /                      → saturate back to 16-bit
 *            │ /
 *     -1.0 ─┴─── saturate ───    Q31: a*b → 64-bit intermediate
 *            -1.0          +1.0        → shift right by 31
 *                 input                → saturate back to 32-bit
 *
 * ── FIR Filter in Fixed-Point ────────────────────────────────────
 *
 *   Accumulate in Q31 for precision, round to Q15 at output:
 *
 *     acc (Q31) = 0
 *     for k = 0..taps-1:
 *         acc += (int32_t)h[k] * (int32_t)x[n-k]   ← 15+15 = 30 frac bits
 *     y[n] = saturate_q15( acc >> 15 )
 *
 *   The 32-bit accumulator holds up to 2^16 multiply-adds without
 *   overflow (enough for filters up to 65536 taps).
 */

#include "fixed_point.h"
#include <math.h>
#include <string.h>

/* ── Q15 helpers ─────────────────────────────────────────────────── */

/**
 * @brief Saturate a 32-bit value to the Q15 range [-32768, +32767].
 *
 *   value > +32767  → +32767
 *   value < -32768  → -32768
 *   otherwise       → unchanged
 */
static q15_t saturate_q15(int32_t value)
{
    if (value > 32767)  return (q15_t)32767;
    if (value < -32768) return (q15_t)(-32768);
    return (q15_t)value;
}

/* ── Q15 Conversion ──────────────────────────────────────────────── */

q15_t double_to_q15(double x)
{
    /* Scale [-1.0, +1.0) → [-32768, +32767] */
    if (x >= 1.0)  return Q15_ONE;
    if (x < -1.0)  return Q15_MINUS_ONE;
    return (q15_t)(x * 32768.0);
}

double q15_to_double(q15_t x)
{
    return (double)x / 32768.0;
}

/* ── Q15 Arithmetic ──────────────────────────────────────────────── */

q15_t q15_add(q15_t a, q15_t b)
{
    return saturate_q15((int32_t)a + (int32_t)b);
}

q15_t q15_sub(q15_t a, q15_t b)
{
    return saturate_q15((int32_t)a - (int32_t)b);
}

q15_t q15_mul(q15_t a, q15_t b)
{
    /*
     * Fractional multiply:
     *   32-bit product = a * b            (30 fractional bits)
     *   Shift right by 15                 (back to 15 fractional bits)
     *   Saturate to 16-bit range
     */
    int32_t product = (int32_t)a * (int32_t)b;
    return saturate_q15(product >> 15);
}

q15_t q15_neg(q15_t a)
{
    /* -(-32768) = +32768 overflows Q15, saturate to +32767 */
    if (a == Q15_MINUS_ONE) return Q15_ONE;
    return (q15_t)(-(int32_t)a);
}

q15_t q15_abs(q15_t a)
{
    if (a == Q15_MINUS_ONE) return Q15_ONE;
    return (a < 0) ? (q15_t)(-(int32_t)a) : a;
}

/* ── Q31 Conversion ──────────────────────────────────────────────── */

q31_t double_to_q31(double x)
{
    if (x >= 1.0)  return (q31_t)0x7FFFFFFF;
    if (x < -1.0)  return (q31_t)0x80000000;
    return (q31_t)(x * 2147483648.0);
}

double q31_to_double(q31_t x)
{
    return (double)x / 2147483648.0;
}

/* ── Q31 Arithmetic ──────────────────────────────────────────────── */

static q31_t saturate_q31(int64_t value)
{
    if (value > 2147483647LL)  return (q31_t)2147483647;
    if (value < -2147483648LL) return (q31_t)(-2147483647 - 1);
    return (q31_t)value;
}

q31_t q31_add(q31_t a, q31_t b)
{
    return saturate_q31((int64_t)a + (int64_t)b);
}

q31_t q31_sub(q31_t a, q31_t b)
{
    return saturate_q31((int64_t)a - (int64_t)b);
}

q31_t q31_mul(q31_t a, q31_t b)
{
    /*
     * 64-bit product: a * b  → 62 fractional bits
     * Shift right by 31      → 31 fractional bits
     * Saturate to 32-bit
     */
    int64_t product = (int64_t)a * (int64_t)b;
    return saturate_q31(product >> 31);
}

/* ── Block operations ────────────────────────────────────────────── */

void double_array_to_q15(const double *in, q15_t *out, int n)
{
    for (int i = 0; i < n; i++)
        out[i] = double_to_q15(in[i]);
}

void q15_array_to_double(const q15_t *in, double *out, int n)
{
    for (int i = 0; i < n; i++)
        out[i] = q15_to_double(in[i]);
}

/* ── Fixed-point FIR filter ──────────────────────────────────────── */

void fir_filter_q15(const q15_t *x, q15_t *y, int n,
                    const q15_t *h, int taps)
{
    /*
     * Standard FIR: y[i] = Σ_{k=0}^{taps-1} h[k] * x[i-k]
     *
     * Accumulate multiply-adds in 32-bit (Q31-ish) to keep precision.
     * Each h[k]*x[i-k] gives a 30-fractional-bit result in int32_t.
     * Sum of up to taps terms stays within int32_t if taps < 65536.
     * Final Q15 output = accumulator >> 15.
     */
    for (int i = 0; i < n; i++) {
        int32_t acc = 0;
        for (int k = 0; k < taps; k++) {
            int idx = i - k;
            q15_t sample = (idx >= 0) ? x[idx] : 0;
            acc += (int32_t)h[k] * (int32_t)sample;
        }
        y[i] = saturate_q15(acc >> 15);
    }
}

/* ── Quantisation metric ─────────────────────────────────────────── */

double compute_sqnr(const double *ref, const double *quant, int n)
{
    /*
     * Signal-to-Quantisation-Noise Ratio:
     *
     *   SQNR = 10 · log10( Σ ref[i]² / Σ (ref[i] - quant[i])² )
     *
     * For uniform quantisation with B bits:
     *   SQNR_ideal ≈ 6.02 · B + 1.76  dB
     *   Q15 (B=15) → ~92 dB
     *   Q31 (B=31) → ~188 dB
     */
    double sig_power   = 0.0;
    double noise_power = 0.0;

    for (int i = 0; i < n; i++) {
        sig_power   += ref[i] * ref[i];
        double err   = ref[i] - quant[i];
        noise_power += err * err;
    }

    if (noise_power < 1e-300) return 300.0;  /* effectively perfect */
    return 10.0 * log10(sig_power / noise_power);
}
