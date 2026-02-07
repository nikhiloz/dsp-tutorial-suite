/**
 * @file fixed_point.h
 * @brief Fixed-point arithmetic for embedded DSP — Q-format operations.
 *
 * Provides Q15 (1.15) and Q31 (1.31) fixed-point types and operations
 * for DSP on platforms without an FPU. All operations are implemented
 * in portable C99 with no floating-point instructions in the hot path.
 *
 * ── Q-Format Representation ──────────────────────────────────────
 *
 *   Q15 (signed 16-bit, 1 sign + 15 fractional bits):
 *
 *      Bit:  15  14  13  12  11  10  9  8  7  6  5  4  3  2  1  0
 *            S   .   F   F   F   F   F  F  F  F  F  F  F  F  F  F
 *
 *      Range:  -1.0  ≤  x  <  +1.0 - 2^{-15}
 *      Resolution: 2^{-15} ≈ 3.05 × 10^{-5}
 *
 *   Q31 (signed 32-bit, 1 sign + 31 fractional bits):
 *
 *      Range:  -1.0  ≤  x  <  +1.0 - 2^{-31}
 *      Resolution: 2^{-31} ≈ 4.66 × 10^{-10}
 *
 * See chapters/18-fixed-point.md for the full tutorial.
 */

#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#include <stdint.h>

/* ── Type aliases ────────────────────────────────────────────────── */

typedef int16_t q15_t;    /**< Q1.15 fixed-point: 16-bit signed         */
typedef int32_t q31_t;    /**< Q1.31 fixed-point: 32-bit signed         */

/* ── Q15 Constants ───────────────────────────────────────────────── */

#define Q15_ONE       ((q15_t)0x7FFF)   /**<  +0.999969 (max positive) */
#define Q15_MINUS_ONE ((q15_t)0x8000)   /**<  -1.0                     */
#define Q15_HALF      ((q15_t)0x4000)   /**<  +0.5                     */
#define Q15_ZERO      ((q15_t)0x0000)   /**<   0.0                     */

/* ── Q15 Conversion ──────────────────────────────────────────────── */

/**
 * @brief Convert a double in [-1.0, +1.0) to Q15.
 * Saturates at Q15_ONE / Q15_MINUS_ONE.
 */
q15_t double_to_q15(double x);

/**
 * @brief Convert Q15 back to double.
 */
double q15_to_double(q15_t x);

/* ── Q15 Arithmetic (saturating) ─────────────────────────────────── */

/** @brief Saturating add: clamp result to [-1, +1) range */
q15_t q15_add(q15_t a, q15_t b);

/** @brief Saturating subtract */
q15_t q15_sub(q15_t a, q15_t b);

/**
 * @brief Fractional multiply: (a * b) >> 15
 *
 * Uses 32-bit intermediate to avoid overflow.
 *   result = (int32_t)a * b  →  shift right by 15  →  saturate to 16-bit
 */
q15_t q15_mul(q15_t a, q15_t b);

/** @brief Negate: -a (saturates -1.0 → +0.999969) */
q15_t q15_neg(q15_t a);

/** @brief Absolute value |a| (saturates -1.0 → +0.999969) */
q15_t q15_abs(q15_t a);

/* ── Q31 Conversion ──────────────────────────────────────────────── */

/** @brief Convert double [-1.0, +1.0) to Q31 */
q31_t double_to_q31(double x);

/** @brief Convert Q31 back to double */
double q31_to_double(q31_t x);

/* ── Q31 Arithmetic ──────────────────────────────────────────────── */

q31_t q31_add(q31_t a, q31_t b);
q31_t q31_sub(q31_t a, q31_t b);

/**
 * @brief Q31 fractional multiply: (a * b) >> 31
 * Uses 64-bit intermediate for full precision.
 */
q31_t q31_mul(q31_t a, q31_t b);

/* ── Block operations ────────────────────────────────────────────── */

/**
 * @brief Convert an array of doubles to Q15 (with saturation).
 * @param in   Input doubles [-1.0, +1.0)
 * @param out  Output Q15 array (caller-allocated, length n)
 * @param n    Number of samples
 */
void double_array_to_q15(const double *in, q15_t *out, int n);

/**
 * @brief Convert Q15 array back to doubles.
 */
void q15_array_to_double(const q15_t *in, double *out, int n);

/**
 * @brief Fixed-point FIR filter in Q15.
 *
 * Performs y[n] = Σ h[k] · x[n-k]  entirely in Q15 arithmetic.
 * Uses Q31 accumulator to maintain precision, then rounds back to Q15.
 *
 * @param x     Input signal (Q15), length n
 * @param y     Output signal (Q15), length n (caller-allocated)
 * @param n     Signal length
 * @param h     Filter coefficients (Q15), length taps
 * @param taps  Number of filter taps
 */
void fir_filter_q15(const q15_t *x, q15_t *y, int n,
                    const q15_t *h, int taps);

/**
 * @brief Compute quantisation error (SNR) between float and fixed-point.
 *
 * SQNR = 10 · log10( Σ x² / Σ (x - x̂)² )    [dB]
 *
 * @param ref    Reference signal (double precision)
 * @param quant  Quantised/reconstructed signal (double precision)
 * @param n      Signal length
 * @return SQNR in dB (higher = better)
 */
double compute_sqnr(const double *ref, const double *quant, int n);

#endif /* FIXED_POINT_H */
