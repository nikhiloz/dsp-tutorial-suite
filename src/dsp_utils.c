/**
 * @file dsp_utils.c
 * @brief Core DSP utilities — complex math, windows, helpers.
 *
 * TUTORIAL CROSS-REFERENCES:
 *   Complex arithmetic  → chapters/03-complex-numbers.md
 *   Window functions    → chapters/09-window-functions.md
 */

#define _GNU_SOURCE
#include "dsp_utils.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ════════════════════════════════════════════════════════════════════
 *  Complex arithmetic
 *  Tutorial ref: chapters/03-complex-numbers.md § "C Implementation"
 * ════════════════════════════════════════════════════════════════════ */

/**
 * @brief Add two complex numbers: (a + b).
 * @param a  First complex operand.
 * @param b  Second complex operand.
 * @return   Complex sum a + b.
 */
Complex complex_add(Complex a, Complex b) {
    return (Complex){ a.re + b.re, a.im + b.im };
}

/**
 * @brief Subtract two complex numbers: (a − b).
 * @param a  First complex operand.
 * @param b  Second complex operand.
 * @return   Complex difference a − b.
 */
Complex complex_sub(Complex a, Complex b) {
    return (Complex){ a.re - b.re, a.im - b.im };
}

/**
 * @brief Multiply two complex numbers: (a · b).
 *
 * Uses the identity (a + bi)(c + di) = (ac − bd) + (ad + bc)i.
 * This is the core operation inside every FFT butterfly.
 * See fft.c butterfly_radix2() for where this is used.
 *
 * @param a  First complex operand.
 * @param b  Second complex operand.
 * @return   Complex product a · b.
 */
Complex complex_mul(Complex a, Complex b) {
    return (Complex){
        a.re * b.re - a.im * b.im,
        a.re * b.im + a.im * b.re
    };
}

/**
 * @brief Compute the magnitude (absolute value) of a complex number.
 * @param z  Complex input.
 * @return   |z| = sqrt(re² + im²).
 */
double complex_mag(Complex z) {
    return sqrt(z.re * z.re + z.im * z.im);
}

/**
 * @brief Compute the phase angle of a complex number.
 * @param z  Complex input.
 * @return   Phase angle in radians (−π, π].
 */
double complex_phase(Complex z) {
    return atan2(z.im, z.re);
}

/**
 * @brief Construct a complex number from polar coordinates.
 * @param mag    Magnitude (radius).
 * @param phase  Phase angle in radians.
 * @return       Complex number mag · e^{j·phase}.
 */
Complex complex_from_polar(double mag, double phase) {
    return (Complex){ mag * cos(phase), mag * sin(phase) };
}

/* ════════════════════════════════════════════════════════════════════
 *  Window functions
 *  Tutorial ref: chapters/09-window-functions.md
 *
 *  Why windows?  When we take an FFT of a finite chunk of signal,
 *  the abrupt edges cause "spectral leakage" — energy smears across
 *  frequency bins.  Multiplying by a window tapers the edges to zero,
 *  trading frequency resolution for reduced leakage.
 * ════════════════════════════════════════════════════════════════════ */

/**
 * @brief Hann window coefficient: w[i] = 0.5 · (1 − cos(2π·i / (N−1))).
 *
 *  Window shape (N=16):
 *    1.0 │        ╭──────╮
 *        │      ╭─╯      ╰─╮
 *    0.5 │    ╭─╯            ╰─╮
 *        │  ╭─╯                ╰─╮
 *    0.0 │──╯                    ╰──
 *        └──────────────────────────
 *         0                      N-1
 *
 *   - Good general-purpose window
 *   - Side-lobe level: −31 dB
 *   - Main-lobe width: 4 bins
 *
 * @param n  Window length (total number of samples).
 * @param i  Sample index (0 ≤ i < n).
 * @return   Window coefficient at index @p i.
 */
double hann_window(int n, int i) {
    return 0.5 * (1.0 - cos(2.0 * M_PI * i / (n - 1)));
}

/**
 * @brief Hamming window coefficient: w[i] = 0.54 − 0.46 · cos(2π·i / (N−1)).
 *
 *  Window shape (N=16):
 *    1.0 │        ╭──────╮
 *        │      ╭─╯      ╰─╮
 *    0.5 │    ╭─╯            ╰─╮
 *        │  ╭─╯                ╰─╮
 *   0.08 │──╯                    ╰──  ← does NOT touch zero
 *        └──────────────────────────
 *         0                      N-1
 *
 *   - Similar to Hann but doesn't touch zero at edges
 *   - Side-lobe level: −42 dB (better than Hann)
 *   - Main-lobe width: 4 bins
 *
 * @param n  Window length (total number of samples).
 * @param i  Sample index (0 ≤ i < n).
 * @return   Window coefficient at index @p i.
 */
double hamming_window(int n, int i) {
    return 0.54 - 0.46 * cos(2.0 * M_PI * i / (n - 1));
}

/**
 * @brief Blackman window coefficient.
 *
 * w[i] = 0.42 − 0.5·cos(2π·i/(N−1)) + 0.08·cos(4π·i/(N−1))
 *
 *   - Excellent side-lobe suppression: −58 dB
 *   - Wider main lobe: 6 bins (poorer frequency resolution)
 *   - Best for detecting weak signals near strong ones
 *
 * @param n  Window length (total number of samples).
 * @param i  Sample index (0 ≤ i < n).
 * @return   Window coefficient at index @p i.
 */
double blackman_window(int n, int i) {
    double t = 2.0 * M_PI * i / (n - 1);
    return 0.42 - 0.5 * cos(t) + 0.08 * cos(2.0 * t);
}

/**
 * @brief Apply a window function to a signal in-place.
 *
 * Multiplies each sample signal[i] by w(n, i).
 *
 * @param signal  Signal buffer to window (modified in-place).
 * @param n       Number of samples.
 * @param w       Window function pointer (e.g. hann_window, hamming_window).
 */
void apply_window(double *signal, int n, window_fn w) {
    for (int i = 0; i < n; i++) {
        signal[i] *= w(n, i);
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  Utility helpers
 * ════════════════════════════════════════════════════════════════════ */

/**
 * @brief Round up to the next power of two.
 *
 * FFT requires power-of-2 lengths; use this to determine zero-padding.
 *
 * @param n  Input value (must be > 0).
 * @return   Smallest power of 2 ≥ @p n.
 */
int next_power_of_2(int n) {
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/**
 * @brief Convert a linear magnitude to decibels.
 *
 * dB = 20 · log₁₀(mag). Returns −200 dB for zero / negative input.
 *
 * @param mag  Linear magnitude value.
 * @return     Value in decibels.
 */
double db_from_magnitude(double mag) {
    if (mag <= 0.0) return -200.0;  /* floor for silence */
    return 20.0 * log10(mag);
}

/**
 * @brief Compute the root-mean-square (RMS) of a signal.
 * @param signal  Input sample array.
 * @param n       Number of samples.
 * @return        RMS value = sqrt( (1/n) Σ x[i]² ).
 */
double rms(const double *signal, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += signal[i] * signal[i];
    }
    return sqrt(sum / n);
}
