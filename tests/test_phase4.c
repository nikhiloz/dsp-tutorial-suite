/**
 * @file test_phase4.c
 * @brief Unit tests for Phase 4 modules: fixed_point, advanced_fft, streaming.
 *
 * Tests:
 *   1.  Q15 conversion round-trip
 *   2.  Q15 saturating add
 *   3.  Q15 multiply
 *   4.  Q31 conversion round-trip
 *   5.  SQNR for Q15 sine
 *   6.  FIR Q15 vs float
 *   7.  Goertzel matches FFT
 *   8.  Goertzel magnitude² consistency
 *   9.  DTMF detection
 *   10. Sliding DFT tracks frequency
 *   11. OLA matches direct convolution
 *   12. OLS matches direct convolution
 *
 * Run: make test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "test_framework.h"
#include "fixed_point.h"
#include "advanced_fft.h"
#include "streaming.h"
#include "fft.h"
#include "filter.h"
#include "dsp_utils.h"
#include "signal_gen.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(void)
{
    TEST_SUITE("Phase 4: Fixed-Point, Advanced FFT, Streaming");

    /* ── Test 1: Q15 round-trip ──────────────────────────── */
    TEST_CASE_BEGIN("Q15 conversion round-trip");
    {
        double vals[] = {0.0, 0.5, -0.5, 0.25, -0.999};
        int ok = 1;
        for (int i = 0; i < 5; i++) {
            q15_t q = double_to_q15(vals[i]);
            double back = q15_to_double(q);
            if (fabs(back - vals[i]) > 4e-5) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Round-trip error > 4e-5"); }
    }

    /* ── Test 2: Q15 saturating add ──────────────────────── */
    TEST_CASE_BEGIN("Q15 saturating add");
    {
        q15_t a = double_to_q15(0.75);
        q15_t b = double_to_q15(0.5);
        q15_t sum = q15_add(a, b);
        double result = q15_to_double(sum);
        /* Should saturate to ~+0.999969, not wrap to negative */
        if (result > 0.99 && result <= 1.0) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("0.75+0.5 should saturate to ~1.0"); }
    }

    /* ── Test 3: Q15 multiply ────────────────────────────── */
    TEST_CASE_BEGIN("Q15 fractional multiply");
    {
        q15_t a = double_to_q15(0.5);
        q15_t b = double_to_q15(0.5);
        q15_t prod = q15_mul(a, b);
        double result = q15_to_double(prod);
        if (fabs(result - 0.25) < 1e-4) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("0.5 × 0.5 should ≈ 0.25"); }
    }

    /* ── Test 4: Q31 round-trip ──────────────────────────── */
    TEST_CASE_BEGIN("Q31 conversion round-trip");
    {
        double vals[] = {0.0, 0.5, -0.5, 0.123456};
        int ok = 1;
        for (int i = 0; i < 4; i++) {
            q31_t q = double_to_q31(vals[i]);
            double back = q31_to_double(q);
            if (fabs(back - vals[i]) > 1e-9) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Q31 round-trip error > 1e-9"); }
    }

    /* ── Test 5: SQNR for Q15 sine ──────────────────────── */
    TEST_CASE_BEGIN("Q15 SQNR > 80 dB for sine");
    {
        const int N = 4096;
        double *x = (double *)malloc((size_t)N * sizeof(double));
        gen_sine(x, N, 0.9, 440.0, 8000.0, 0.0);

        q15_t *xq = (q15_t *)malloc((size_t)N * sizeof(q15_t));
        double *xr = (double *)malloc((size_t)N * sizeof(double));
        double_array_to_q15(x, xq, N);
        q15_array_to_double(xq, xr, N);

        double sqnr = compute_sqnr(x, xr, N);
        if (sqnr > 80.0) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("SQNR should be > 80 dB for Q15"); }

        free(x); free(xq); free(xr);
    }

    /* ── Test 6: FIR Q15 vs float ────────────────────────── */
    TEST_CASE_BEGIN("FIR Q15 SQNR > 50 dB vs float");
    {
        const int N = 512, taps = 15;
        double h[15];
        fir_lowpass(h, taps, 0.3);

        double *x = (double *)malloc((size_t)N * sizeof(double));
        gen_sine(x, N, 0.5, 300.0, 8000.0, 0.0);

        double *yf = (double *)calloc((size_t)N, sizeof(double));
        fir_filter(x, yf, N, h, taps);

        q15_t *xq = (q15_t *)malloc((size_t)N * sizeof(q15_t));
        q15_t *hq = (q15_t *)malloc((size_t)taps * sizeof(q15_t));
        q15_t *yq = (q15_t *)calloc((size_t)N, sizeof(q15_t));
        double *yr = (double *)malloc((size_t)N * sizeof(double));

        double_array_to_q15(x, xq, N);
        double_array_to_q15(h, hq, taps);
        fir_filter_q15(xq, yq, N, hq, taps);
        q15_array_to_double(yq, yr, N);

        double sqnr = compute_sqnr(yf, yr, N);
        if (sqnr > 50.0) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("FIR Q15 SQNR should be > 50 dB"); }

        free(x); free(yf); free(xq); free(hq); free(yq); free(yr);
    }

    /* ── Test 7: Goertzel matches FFT ────────────────────── */
    TEST_CASE_BEGIN("Goertzel matches FFT bin");
    {
        const int N = 256;
        double x[256];
        gen_sine(x, N, 1.0, 1000.0, 8000.0, 0.0);
        int k = 32;   /* 1000 Hz bin at fs=8000, N=256 */

        Complex *X = (Complex *)malloc((size_t)N * sizeof(Complex));
        for (int i = 0; i < N; i++) { X[i].re = x[i]; X[i].im = 0.0; }
        fft(X, N);

        Complex G = goertzel(x, N, k);
        double mag_fft = sqrt(X[k].re * X[k].re + X[k].im * X[k].im);
        double err = sqrt((X[k].re - G.re) * (X[k].re - G.re) +
                         (X[k].im - G.im) * (X[k].im - G.im));
        double rel_err = (mag_fft > 0) ? err / mag_fft : err;

        if (rel_err < 1e-10) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("Goertzel relative error should be < 1e-10"); }
        free(X);
    }

    /* ── Test 8: Goertzel magnitude² ─────────────────────── */
    TEST_CASE_BEGIN("Goertzel mag² consistency");
    {
        const int N = 512;
        double x[512];
        gen_sine(x, N, 1.0, 500.0, 8000.0, 0.0);
        int k = (int)(500.0 * N / 8000.0);

        Complex G = goertzel(x, N, k);
        double mag2_complex = G.re * G.re + G.im * G.im;
        double mag2_direct  = goertzel_magnitude_sq(x, N, k);

        if (fabs(mag2_complex - mag2_direct) < 1e-6) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("|X|² from goertzel vs goertzel_magnitude_sq"); }
    }

    /* ── Test 9: DTMF detection ──────────────────────────── */
    TEST_CASE_BEGIN("DTMF detects digit '5'");
    {
        const int N = 205;
        const double fs = 8000.0;
        double tone[205], tmp[205];

        gen_sine(tone, N, 0.5, 770.0, fs, 0.0);   /* row for '5' */
        gen_sine(tmp,  N, 0.5, 1336.0, fs, 0.0);  /* col for '5' */
        signal_add(tone, tmp, N);

        char detected = dtmf_detect(tone, N, fs);
        if (detected == '5') { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("Expected '5'"); }
    }

    /* ── Test 10: Sliding DFT tracks frequency ───────────── */
    TEST_CASE_BEGIN("Sliding DFT magnitude > 0 at target");
    {
        const int win = 128;
        const int k = 16;   /* track bin 16 → freq = 16*fs/128 */

        SlidingDFT sdft;
        sliding_dft_init(&sdft, win, k);

        /* Feed a sine at the tracked frequency */
        double fs = 8000.0;
        double f0 = (double)k * fs / win;
        double x[256];
        gen_sine(x, 256, 1.0, f0, fs, 0.0);

        Complex last_bin = {0, 0};
        for (int i = 0; i < 256; i++)
            last_bin = sliding_dft_update(&sdft, x[i]);

        double mag = complex_mag(last_bin);
        if (mag > 10.0) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("SDFT magnitude should be large at target freq"); }

        sliding_dft_free(&sdft);
    }

    /* ── Test 11: OLA matches direct convolution ─────────── */
    TEST_CASE_BEGIN("OLA matches direct FIR");
    {
        const int N = 512, taps = 31, blk = 128;
        double h[31];
        fir_lowpass(h, taps, 0.25);

        double *x = (double *)malloc((size_t)N * sizeof(double));
        gen_sine(x, N, 1.0, 300.0, 8000.0, 0.0);

        double *y_ref = (double *)calloc((size_t)N, sizeof(double));
        fir_filter(x, y_ref, N, h, taps);

        OlaState ola;
        ola_init(&ola, h, taps, blk);
        double *y_ola = (double *)calloc((size_t)N, sizeof(double));
        for (int b = 0; b < N / blk; b++)
            ola_process(&ola, x + b * blk, y_ola + b * blk);

        double max_err = 0.0;
        for (int i = 0; i < N; i++) {
            double err = fabs(y_ref[i] - y_ola[i]);
            if (err > max_err) max_err = err;
        }

        if (max_err < 1e-10) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("OLA max error should be < 1e-10"); }

        ola_free(&ola);
        free(x); free(y_ref); free(y_ola);
    }

    /* ── Test 12: OLS matches direct convolution ─────────── */
    TEST_CASE_BEGIN("OLS matches direct FIR (after warm-up)");
    {
        const int N = 1024, taps = 31, blk = 256;
        double h[31];
        fir_lowpass(h, taps, 0.25);

        double *x = (double *)malloc((size_t)N * sizeof(double));
        gen_sine(x, N, 1.0, 300.0, 8000.0, 0.0);

        double *y_ref = (double *)calloc((size_t)N, sizeof(double));
        fir_filter(x, y_ref, N, h, taps);

        OlsState ols;
        ols_init(&ols, h, taps, blk);
        double *y_ols = (double *)calloc((size_t)N, sizeof(double));
        for (int b = 0; b < N / blk; b++)
            ols_process(&ols, x + b * blk, y_ols + b * blk);

        /* Skip first block (warm-up) */
        double max_err = 0.0;
        for (int i = blk; i < N; i++) {
            double err = fabs(y_ref[i] - y_ols[i]);
            if (err > max_err) max_err = err;
        }

        if (max_err < 1e-10) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("OLS max error should be < 1e-10 after warm-up"); }

        ols_free(&ols);
        free(x); free(y_ref); free(y_ols);
    }

    printf("\n=== Test Summary ===\n");
    printf("Total: %d, Passed: %d, Failed: %d\n",
           test_count, test_passed, test_failed);
    printf("Pass Rate: %.1f%%\n",
           test_count > 0 ? (100.0 * test_passed / test_count) : 0.0);
    return (test_failed == 0) ? 0 : 1;
}
