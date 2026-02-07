/**
 * @file advanced_fft.c
 * @brief Advanced FFT algorithms — Goertzel, DTMF detection, sliding DFT.
 *
 * ── Goertzel Data Flow ───────────────────────────────────────────
 *
 *   x[0] ──┐
 *   x[1] ──┤    ┌───────────────────────┐
 *   x[2] ──┤───►│ 2nd-order IIR filter  │───► X[k]
 *    ...    │    │ coeff = 2·cos(2πk/N)  │
 *   x[N-1]─┘    └───────────────────────┘
 *
 *   Inner loop (N iterations):
 *     s0 = x[n] + coeff·s1 - s2
 *     s2 = s1;  s1 = s0;
 *
 *   Final output:
 *     X[k].re = s1 - s2·cos(2πk/N)
 *     X[k].im =     -s2·sin(2πk/N)
 *
 * ── Sliding DFT ──────────────────────────────────────────────────
 *
 *   For each new sample x_new replacing oldest sample x_old:
 *
 *     X_new[k] = (X_old[k] - x_old + x_new) · e^{j2πk/N}
 *
 *   Cost: 1 complex multiply + 2 real adds per sample.
 *   Perfect for real-time single-frequency tracking (vibration, pitch).
 */

#include "advanced_fft.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ── Goertzel ────────────────────────────────────────────────────── */

Complex goertzel(const double *x, int n, int k)
{
    /*
     * Goertzel recurrence:
     *   coeff = 2 · cos(2πk/N)
     *   s0 = x[i] + coeff·s1 - s2
     *
     * After N samples:
     *   X[k] = s1 - s2 · e^{-j2πk/N}
     */
    double omega = 2.0 * M_PI * (double)k / (double)n;
    double coeff = 2.0 * cos(omega);
    double s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < n; i++) {
        double s0 = x[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /*
     * Apply numerator zero: s[N] = 2cos(ω)·s1 - s2 (x[N]=0)
     * X[k] = s[N] - e^{-jω}·s[N-1]
     *      = (cos(ω)·s1 - s2) + j·(sin(ω)·s1)
     */
    Complex result;
    result.re = s1 * cos(omega) - s2;
    result.im = s1 * sin(omega);
    return result;
}

double goertzel_magnitude_sq(const double *x, int n, int k)
{
    /*
     * Optimised: compute |X[k]|² without the final complex multiply.
     *   |X[k]|² = s1² + s2² - coeff·s1·s2
     */
    double omega = 2.0 * M_PI * (double)k / (double)n;
    double coeff = 2.0 * cos(omega);
    double s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < n; i++) {
        double s0 = x[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    return s1 * s1 + s2 * s2 - coeff * s1 * s2;
}

Complex goertzel_freq(const double *x, int n, double freq_hz, double fs)
{
    /*
     * Generalised Goertzel: k can be non-integer.
     * k = freq_hz * n / fs
     */
    double k_frac = freq_hz * (double)n / fs;
    double omega = 2.0 * M_PI * k_frac / (double)n;
    double coeff = 2.0 * cos(omega);
    double s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < n; i++) {
        double s0 = x[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    Complex result;
    result.re = s1 * cos(omega) - s2;
    result.im = s1 * sin(omega);
    return result;
}

/* ── DTMF Detection ─────────────────────────────────────────────── */

/*
 * DTMF keypad layout:
 *
 *         1209   1336   1477   1633
 *   697     1      2      3      A
 *   770     4      5      6      B
 *   852     7      8      9      C
 *   941     *      0      #      D
 */

static const double dtmf_row_freqs[4] = {697.0, 770.0, 852.0, 941.0};
static const double dtmf_col_freqs[4] = {1209.0, 1336.0, 1477.0, 1633.0};
static const char dtmf_keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

char dtmf_detect(const double *x, int n, double fs)
{
    /*
     * For each of the 8 DTMF frequencies, compute Goertzel power.
     * Find the strongest row and column.  If both exceed a threshold,
     * return the corresponding key.
     */
    double row_power[4], col_power[4];

    for (int i = 0; i < 4; i++) {
        Complex r = goertzel_freq(x, n, dtmf_row_freqs[i], fs);
        row_power[i] = r.re * r.re + r.im * r.im;

        Complex c = goertzel_freq(x, n, dtmf_col_freqs[i], fs);
        col_power[i] = c.re * c.re + c.im * c.im;
    }

    /* Find strongest row and column */
    int best_row = 0, best_col = 0;
    for (int i = 1; i < 4; i++) {
        if (row_power[i] > row_power[best_row]) best_row = i;
        if (col_power[i] > col_power[best_col]) best_col = i;
    }

    /* Threshold: strongest must be significantly above noise */
    double total_power = 0.0;
    for (int i = 0; i < 4; i++) {
        total_power += row_power[i] + col_power[i];
    }
    double avg_power = total_power / 8.0;

    if (row_power[best_row] < avg_power * 2.0 ||
        col_power[best_col] < avg_power * 2.0)
        return '?';   /* no valid DTMF detected */

    return dtmf_keys[best_row][best_col];
}

/* ── Sliding DFT ─────────────────────────────────────────────────── */

int sliding_dft_init(SlidingDFT *s, int N, int k)
{
    s->N = N;
    s->k = k;
    s->pos = 0;

    double omega = 2.0 * M_PI * (double)k / (double)N;
    s->coeff.re = cos(omega);
    s->coeff.im = sin(omega);

    s->bin.re = 0.0;
    s->bin.im = 0.0;

    s->buffer = (double *)calloc((size_t)N, sizeof(double));
    if (!s->buffer) return -1;

    return 0;
}

Complex sliding_dft_update(SlidingDFT *s, double sample)
{
    /*
     * Replace oldest sample with new one:
     *   delta = new_sample - old_sample
     *   X_new = (X_old + delta) × e^{j2πk/N}
     */
    double old_sample = s->buffer[s->pos];
    s->buffer[s->pos] = sample;
    s->pos = (s->pos + 1) % s->N;

    double delta = sample - old_sample;
    Complex tmp;
    tmp.re = s->bin.re + delta;
    tmp.im = s->bin.im;

    /* Multiply by e^{j2πk/N} */
    s->bin.re = tmp.re * s->coeff.re - tmp.im * s->coeff.im;
    s->bin.im = tmp.re * s->coeff.im + tmp.im * s->coeff.re;

    return s->bin;
}

void sliding_dft_free(SlidingDFT *s)
{
    if (s && s->buffer) {
        free(s->buffer);
        s->buffer = NULL;
    }
}
