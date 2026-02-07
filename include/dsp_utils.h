/**
 * @file dsp_utils.h
 * @brief Core DSP utility functions — complex arithmetic, window functions, helpers.
 *
 * This is the foundational module. Every other module depends on it.
 * See chapters/03-complex-numbers.md for theory and walkthrough.
 */

#ifndef DSP_UTILS_H
#define DSP_UTILS_H

#include <stddef.h>

/* ── Complex number type ─────────────────────────────────────────── */

typedef struct {
    double re;  /* Real part      */
    double im;  /* Imaginary part */
} Complex;

/* ── Complex arithmetic ──────────────────────────────────────────── */

/** @brief Add two complex numbers: (a.re+b.re) + j(a.im+b.im) */
Complex complex_add(Complex a, Complex b);
/** @brief Subtract: a - b */
Complex complex_sub(Complex a, Complex b);
/** @brief Multiply using (ac-bd) + j(ad+bc) */
Complex complex_mul(Complex a, Complex b);
/** @brief Magnitude |z| = sqrt(re² + im²) */
double  complex_mag(Complex z);
/** @brief Phase angle atan2(im, re) in radians, range (-π, π] */
double  complex_phase(Complex z);
/**
 * @brief Construct complex from polar form: mag * e^{j·phase}
 * @param mag    Magnitude (radius)
 * @param phase  Angle in radians
 */
Complex complex_from_polar(double mag, double phase);

/* ── Window functions ────────────────────────────────────────────── */
/* Each returns w[i] for a window of length n.
 * See chapters/09-window-functions.md for spectral leakage theory.
 *
 *   Window shape (n=16):
 *
 *   1.0 |     ****           Hann (raised cosine)
 *       |   **    **         - Good sidelobe rolloff
 *       |  *        *        - -31.5 dB first sidelobe
 *   0.5 | *          *
 *       |*            *      w[i] = 0.5 * (1 - cos(2πi/(n-1)))
 *       *              *
 *   0.0 *───────────────*──
 *       0    4    8   12  16
 */

/** @brief Hann window: w[i] = 0.5(1 - cos(2πi/(n-1))). First sidelobe: -31.5 dB. */
double hann_window(int n, int i);
/** @brief Hamming window: w[i] = 0.54 - 0.46·cos(2πi/(n-1)). First sidelobe: -42.7 dB. */
double hamming_window(int n, int i);
/** @brief Blackman window: 3-term cosine sum. First sidelobe: -58.1 dB. */
double blackman_window(int n, int i);

/**
 * @brief Apply a window function in-place: signal[i] *= window(n, i)
 * @param signal  Sample buffer (modified in-place)
 * @param n       Number of samples
 * @param w       Window function pointer
 */
typedef double (*window_fn)(int n, int i);
void apply_window(double *signal, int n, window_fn w);

/* ── Utility helpers ─────────────────────────────────────────────── */

/** @brief Smallest power of 2 >= n. Used for zero-padding before FFT. */
int    next_power_of_2(int n);
/** @brief Convert magnitude to decibels: 20·log₁₀(mag). Returns -INFINITY for mag=0. */
double db_from_magnitude(double mag);
/** @brief Root Mean Square of signal: sqrt(Σx[i]²/n). */
double rms(const double *signal, int n);

#endif /* DSP_UTILS_H */
