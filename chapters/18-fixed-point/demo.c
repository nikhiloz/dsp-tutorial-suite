/**
 * @file 18-fixed-point.c
 * @brief Chapter 18 — Fixed-Point Arithmetic & Quantisation
 *
 * Demonstrates:
 *  1. Q15 conversion round-trip (double → Q15 → double)
 *  2. Q15 arithmetic: add, multiply, saturation behaviour
 *  3. Quantisation noise: SQNR for Q15 vs Q31
 *  4. Fixed-point FIR filter vs floating-point reference
 *  5. Overflow / saturation demo
 *
 * ── Why Fixed-Point? ─────────────────────────────────────────────
 *
 *   Many embedded DSP chips (Cortex-M0/M4, C2000, SHARC) lack an FPU
 *   or have limited floating-point throughput.  Fixed-point arithmetic
 *   uses integer ALU instructions with an implied binary point:
 *
 *     Floating-point:          Fixed-point (Q15):
 *     ┌─────────────────┐      ┌───┬────────────────┐
 *     │ sign│exp│mantissa│      │ S │ 15 frac bits   │
 *     └─────────────────┘      └───┴────────────────┘
 *      32 bits, ~7 digits       16 bits, ~4.5 digits
 *      hardware FPU needed      integer ALU only
 *      ~10 cycles (no FPU)      1-2 cycles
 *
 * ── Quantisation Noise ───────────────────────────────────────────
 *
 *   Converting float → Q15 introduces quantisation error:
 *
 *     x(t)  ──► Quantise ──► x̂[n]
 *                   │
 *                   ▼
 *              e[n] = x[n] - x̂[n]    (quantisation noise)
 *
 *   SQNR_ideal = 6.02 · B + 1.76  dB
 *   Q15 (B=15):  ~92 dB
 *   Q31 (B=31): ~188 dB
 *
 * Build:  make           (builds ch18 among all targets)
 * Run:    build/bin/ch18
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "fixed_point.h"
#include "dsp_utils.h"
#include "signal_gen.h"
#include "filter.h"
#include "gnuplot.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ------------------------------------------------------------------ */
/*  Demo 1: Q15 Conversion Round-Trip                                 */
/*                                                                    */
/*  double → Q15 → double:                                            */
/*    0.5    →  16384  →  0.500000                                    */
/*   -0.25   →  -8192  → -0.250000                                   */
/*    0.001  →     33  →  0.001007  (quantisation error!)             */
/* ------------------------------------------------------------------ */
static void demo_conversion(void)
{
    printf("\n=== Demo 1: Q15 Conversion Round-Trip ===\n\n");

    double test_vals[] = {0.0, 0.5, -0.5, 0.25, -0.25, 0.001, -0.999, 0.999};
    int n = (int)(sizeof(test_vals) / sizeof(test_vals[0]));

    printf("  %-12s %-8s %-12s %-12s\n",
           "Original", "Q15", "Recovered", "Error");
    printf("  %-12s %-8s %-12s %-12s\n",
           "--------", "---", "---------", "-----");

    for (int i = 0; i < n; i++) {
        q15_t q = double_to_q15(test_vals[i]);
        double recovered = q15_to_double(q);
        double error = test_vals[i] - recovered;
        printf("  %+10.6f  %6d  %+10.6f  %+.2e\n",
               test_vals[i], (int)q, recovered, error);
    }

    printf("\n  Q15 resolution = 2^{-15} ≈ %.2e\n", 1.0 / 32768.0);
}

/* ------------------------------------------------------------------ */
/*  Demo 2: Q15 Arithmetic — saturation behaviour                     */
/*                                                                    */
/*   Normal:     0.5 + 0.25 = 0.75     ✓                             */
/*   Saturated:  0.75 + 0.5 → +0.999   (not +1.25!)                  */
/*   Multiply:   0.5 × 0.5 = 0.25      ✓                             */
/* ------------------------------------------------------------------ */
static void demo_arithmetic(void)
{
    printf("\n=== Demo 2: Q15 Saturating Arithmetic ===\n\n");

    q15_t a = double_to_q15(0.5);
    q15_t b = double_to_q15(0.25);
    q15_t c = double_to_q15(0.75);
    q15_t d = double_to_q15(0.5);

    /* Normal add */
    q15_t sum1 = q15_add(a, b);
    printf("  0.5 + 0.25 = %+.6f  (Q15: %d)\n",
           q15_to_double(sum1), (int)sum1);

    /* Saturating add */
    q15_t sum2 = q15_add(c, d);
    printf("  0.75 + 0.5 = %+.6f  (saturated! float would give 1.25)\n",
           q15_to_double(sum2));

    /* Multiply */
    q15_t prod1 = q15_mul(a, a);
    printf("  0.5 × 0.5  = %+.6f  (exact: 0.25)\n",
           q15_to_double(prod1));

    q15_t prod2 = q15_mul(a, b);
    printf("  0.5 × 0.25 = %+.6f  (exact: 0.125)\n",
           q15_to_double(prod2));

    /* Negate edge case */
    q15_t neg = q15_neg(Q15_MINUS_ONE);
    printf("  -(-1.0)    = %+.6f  (saturated: can't represent +1.0)\n",
           q15_to_double(neg));
}

/* ------------------------------------------------------------------ */
/*  Demo 3: Quantisation Noise — SQNR for Q15 vs Q31                 */
/*                                                                    */
/*   Generate a sine wave, quantise to Q15 and Q31, compute SQNR.    */
/*   Ideal: Q15 → ~92 dB,  Q31 → ~188 dB                            */
/* ------------------------------------------------------------------ */
static void demo_sqnr(void)
{
    printf("\n=== Demo 3: Quantisation Noise — SQNR ===\n\n");

    const int N = 4096;
    const double fs = 8000.0;
    const double f0 = 440.0;
    const double amp = 0.9;   /* stay below 1.0 for Q15 range */

    double *x = (double *)malloc((size_t)N * sizeof(double));
    gen_sine(x, N, amp, f0, fs, 0.0);

    /* Q15 quantisation */
    q15_t *xq15 = (q15_t *)malloc((size_t)N * sizeof(q15_t));
    double *xr15 = (double *)malloc((size_t)N * sizeof(double));
    double_array_to_q15(x, xq15, N);
    q15_array_to_double(xq15, xr15, N);
    double sqnr15 = compute_sqnr(x, xr15, N);

    /* Q31 quantisation */
    double *xr31 = (double *)malloc((size_t)N * sizeof(double));
    for (int i = 0; i < N; i++) {
        q31_t q = double_to_q31(x[i]);
        xr31[i] = q31_to_double(q);
    }
    double sqnr31 = compute_sqnr(x, xr31, N);

    printf("  Signal: 440 Hz sine, amplitude %.1f, %d samples\n", amp, N);
    printf("  Q15 SQNR: %.1f dB  (ideal ~92 dB)\n", sqnr15);
    printf("  Q31 SQNR: %.1f dB  (ideal ~188 dB)\n", sqnr31);

    /* Plot quantisation error for Q15 */
    double *err = (double *)malloc((size_t)N * sizeof(double));
    double *idx = (double *)malloc((size_t)N * sizeof(double));
    for (int i = 0; i < N; i++) {
        err[i] = x[i] - xr15[i];
        idx[i] = (double)i;
    }

    /* Plot first 200 samples of signal and error */
    int plot_n = 200;
    GpSeries s[2];
    s[0].label = "Original signal";
    s[0].x     = idx;
    s[0].y     = x;
    s[0].n     = plot_n;
    s[0].style = "lines";

    double *err_scaled = (double *)malloc((size_t)N * sizeof(double));
    for (int i = 0; i < N; i++)
        err_scaled[i] = err[i] * 1000.0;  /* scale for visibility */

    s[1].label = "Q15 error (×1000)";
    s[1].x     = idx;
    s[1].y     = err_scaled;
    s[1].n     = plot_n;
    s[1].style = "lines";

    gp_plot_multi("18-fixed-point", "quantisation_error",
                  "Q15 Quantisation Error (440 Hz sine)",
                  "Sample", "Amplitude",
                  s, 2);
    printf("  → plots/ch18/quantisation_error.png\n");

    free(x); free(xq15); free(xr15); free(xr31);
    free(err); free(idx); free(err_scaled);
}

/* ------------------------------------------------------------------ */
/*  Demo 4: Fixed-point FIR vs floating-point reference               */
/*                                                                    */
/*   Apply the same 31-tap lowpass to a noisy signal using:           */
/*   (a) floating-point fir_filter()                                  */
/*   (b) fixed-point fir_filter_q15()                                 */
/*   Compare outputs via SQNR.                                        */
/* ------------------------------------------------------------------ */
static void demo_fir_comparison(void)
{
    printf("\n=== Demo 4: Fixed-Point FIR vs Floating-Point ===\n\n");

    const int N = 1024;
    const int taps = 31;
    const double fs = 8000.0;
    const double cutoff = 1000.0 / (fs / 2.0);   /* normalised cutoff */

    /* Design lowpass filter (floating-point coefficients) */
    double h_float[31];
    fir_lowpass(h_float, taps, cutoff);

    /* Generate noisy test signal */
    double *x     = (double *)malloc((size_t)N * sizeof(double));
    double *noise = (double *)malloc((size_t)N * sizeof(double));
    gen_sine(x, N, 0.5, 300.0, fs, 0.0);
    gen_gaussian_noise(noise, N, 0.0, 0.3, 42);
    signal_add(x, noise, N);

    /* Floating-point filtering */
    double *y_float = (double *)calloc((size_t)N, sizeof(double));
    fir_filter(x, y_float, N, h_float, taps);

    /* Fixed-point filtering */
    q15_t *xq    = (q15_t *)malloc((size_t)N * sizeof(q15_t));
    q15_t *hq    = (q15_t *)malloc((size_t)taps * sizeof(q15_t));
    q15_t *yq    = (q15_t *)calloc((size_t)N, sizeof(q15_t));
    double *y_fixed = (double *)malloc((size_t)N * sizeof(double));

    double_array_to_q15(x, xq, N);
    double_array_to_q15(h_float, hq, taps);
    fir_filter_q15(xq, yq, N, hq, taps);
    q15_array_to_double(yq, y_fixed, N);

    double sqnr = compute_sqnr(y_float, y_fixed, N);
    printf("  %d-tap FIR lowpass (cutoff=%.0f Hz)\n", taps, cutoff * fs / 2.0);
    printf("  SQNR (float vs Q15): %.1f dB\n", sqnr);

    /* Plot comparison */
    double *idx = (double *)malloc((size_t)N * sizeof(double));
    for (int i = 0; i < N; i++) idx[i] = (double)i;

    int plot_n = 300;
    GpSeries s[2];
    s[0].label = "Float FIR output";
    s[0].x     = idx;
    s[0].y     = y_float;
    s[0].n     = plot_n;
    s[0].style = "lines";

    s[1].label = "Q15 FIR output";
    s[1].x     = idx;
    s[1].y     = y_fixed;
    s[1].n     = plot_n;
    s[1].style = "lines";

    gp_plot_multi("18-fixed-point", "fir_float_vs_q15",
                  "FIR Lowpass: Float vs Q15 Fixed-Point",
                  "Sample", "Amplitude",
                  s, 2);
    printf("  → plots/ch18/fir_float_vs_q15.png\n");

    free(x); free(noise); free(y_float);
    free(xq); free(hq); free(yq); free(y_fixed); free(idx);
}

/* ------------------------------------------------------------------ */
/*  Demo 5: Overflow / saturation visual                              */
/*                                                                    */
/*  Scale a signal past Q15 range to show clipping vs wrap-around.    */
/* ------------------------------------------------------------------ */
static void demo_saturation(void)
{
    printf("\n=== Demo 5: Saturation vs Overflow ===\n\n");

    const int N = 512;
    const double fs = 8000.0;

    double *x = (double *)malloc((size_t)N * sizeof(double));
    gen_sine(x, N, 1.0, 200.0, fs, 0.0);

    /* Amplify beyond Q15 range */
    double gains[] = {0.5, 0.9, 1.5, 2.0};
    const char *labs[] = {"0.5x", "0.9x", "1.5x (clips)", "2.0x (clips)"};
    int n_gains = 4;

    FILE *gp = gp_open("18-fixed-point", "saturation", 900, 500);
    if (gp) {
        fprintf(gp, "set title 'Q15 Saturation at Different Gains'\n");
        fprintf(gp, "set xlabel 'Sample'\n");
        fprintf(gp, "set ylabel 'Amplitude'\n");
        fprintf(gp, "set grid\n");
        fprintf(gp, "set xrange [0:200]\n");
        fprintf(gp, "plot ");
        for (int g = 0; g < n_gains; g++) {
            if (g > 0) fprintf(gp, ", ");
            fprintf(gp, "'-' using 1:2 with lines lw 2 title '%s'", labs[g]);
        }
        fprintf(gp, "\n");

        for (int g = 0; g < n_gains; g++) {
            int plot_n = 200;
            for (int i = 0; i < plot_n; i++) {
                double scaled = x[i] * gains[g];
                q15_t q = double_to_q15(scaled);
                fprintf(gp, "%d %.6f\n", i, q15_to_double(q));
            }
            fprintf(gp, "e\n");

            /* Report clipping */
            int clips = 0;
            for (int i = 0; i < N; i++) {
                double scaled = x[i] * gains[g];
                q15_t q = double_to_q15(scaled);
                if (q == Q15_ONE || q == Q15_MINUS_ONE) clips++;
            }
            if (clips > 0)
                printf("  Gain %.1fx: %d clipped samples (%.1f%%)\n",
                       gains[g], clips, 100.0 * clips / N);
        }
        gp_close(gp);
        printf("  → plots/ch18/saturation.png\n");
    }

    free(x);
}

/* ================================================================== */

int main(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  Chapter 18: Fixed-Point Arithmetic & Quantisation     ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    gp_init("18-fixed-point");

    demo_conversion();
    demo_arithmetic();
    demo_sqnr();
    demo_fir_comparison();
    demo_saturation();

    printf("\n=== Chapter 18 Complete ===\n");
    return 0;
}
