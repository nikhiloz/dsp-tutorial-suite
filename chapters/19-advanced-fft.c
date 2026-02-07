/**
 * @file 19-advanced-fft.c
 * @brief Chapter 19 — Advanced FFT: Goertzel, DTMF, Sliding DFT
 *
 * Demonstrates:
 *  1. Goertzel vs full FFT — single-bin extraction
 *  2. DTMF tone detection using Goertzel
 *  3. Goertzel for non-integer frequency (generalised)
 *  4. Sliding DFT for real-time frequency tracking
 *  5. Algorithm comparison: when to use which
 *
 * ── Algorithm Selection Guide ────────────────────────────────────
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │  Need ALL N frequency bins?                                │
 *   │    YES → use fft()              O(N log N)                 │
 *   │    NO  ──► Need < log₂N bins?                              │
 *   │              YES → use goertzel()     O(N) per bin         │
 *   │              NO  → use fft()          O(N log N)           │
 *   │                                                             │
 *   │  Streaming, single bin, per-sample update?                  │
 *   │    YES → use sliding_dft()      O(1) per sample            │
 *   └─────────────────────────────────────────────────────────────┘
 *
 * ── DTMF Keypad ──────────────────────────────────────────────────
 *
 *          1209 Hz  1336 Hz  1477 Hz  1633 Hz
 *   697 Hz    1        2        3        A
 *   770 Hz    4        5        6        B
 *   852 Hz    7        8        9        C
 *   941 Hz    *        0        #        D
 *
 *   Each key = one row tone + one column tone.
 *   Goertzel checks 8 frequencies in O(8N) — much cheaper than FFT.
 *
 * Build:  make           (builds ch19 among all targets)
 * Run:    build/bin/ch19
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "advanced_fft.h"
#include "fft.h"
#include "dsp_utils.h"
#include "signal_gen.h"
#include "gnuplot.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ------------------------------------------------------------------ */
/*  Demo 1: Goertzel vs Full FFT — single bin                         */
/*                                                                    */
/*  Compute X[k] for a single frequency using both methods.           */
/*  The Goertzel result should match the FFT bin exactly.             */
/* ------------------------------------------------------------------ */
static void demo_goertzel_vs_fft(void)
{
    printf("\n=== Demo 1: Goertzel vs Full FFT ===\n\n");

    const int    N  = 1024;
    const double fs = 8000.0;
    const double f0 = 1000.0;

    /* Generate a pure 1 kHz tone */
    double *x = (double *)malloc((size_t)N * sizeof(double));
    gen_sine(x, N, 1.0, f0, fs, 0.0);

    /* Target bin: k = f0 * N / fs */
    int k = (int)(f0 * N / fs);
    printf("  Target: %.0f Hz → bin k = %d\n", f0, k);

    /* Full FFT */
    Complex *X = (Complex *)malloc((size_t)N * sizeof(Complex));
    for (int i = 0; i < N; i++) {
        X[i].re = x[i];
        X[i].im = 0.0;
    }
    fft(X, N);

    printf("  FFT[%d]      = %.4f + j%.4f   (mag = %.4f)\n",
           k, X[k].re, X[k].im, complex_mag(X[k]));

    /* Goertzel (same bin) */
    Complex G = goertzel(x, N, k);
    printf("  Goertzel[%d] = %.4f + j%.4f   (mag = %.4f)\n",
           k, G.re, G.im, complex_mag(G));

    /* Error */
    double err = sqrt((X[k].re - G.re) * (X[k].re - G.re) +
                      (X[k].im - G.im) * (X[k].im - G.im));
    printf("  Difference: %.2e (should be ~0)\n", err);

    printf("\n  Complexity comparison for %d samples:\n", N);
    printf("    FFT:      O(N log₂ N) = %d operations\n",
           N * (int)(log2((double)N)));
    printf("    Goertzel: O(N)         = %d operations (for 1 bin)\n", N);

    free(x); free(X);
}

/* ------------------------------------------------------------------ */
/*  Demo 2: DTMF Detection                                           */
/*                                                                    */
/*  Generate DTMF tones for several digits and detect them using      */
/*  the Goertzel-based dtmf_detect() function.                       */
/* ------------------------------------------------------------------ */
static void demo_dtmf(void)
{
    printf("\n=== Demo 2: DTMF Tone Detection ===\n\n");

    const double fs = 8000.0;
    const int    N  = 205;    /* ~25 ms at 8 kHz — standard DTMF frame */

    /* DTMF frequency pairs for digits '1'-'9','0','*','#' */
    struct { char key; double row; double col; } digits[] = {
        {'1', 697, 1209}, {'5', 770, 1336}, {'9', 852, 1477},
        {'0', 941, 1336}, {'*', 941, 1209}, {'#', 941, 1477},
        {'A', 697, 1633}, {'D', 941, 1633}
    };
    int n_digits = (int)(sizeof(digits) / sizeof(digits[0]));

    printf("  %-6s %-10s %-10s %-10s\n",
           "Key", "Row (Hz)", "Col (Hz)", "Detected");
    printf("  %-6s %-10s %-10s %-10s\n",
           "---", "--------", "--------", "--------");

    double *tone = (double *)malloc((size_t)N * sizeof(double));
    double *tmp  = (double *)malloc((size_t)N * sizeof(double));

    int correct = 0;
    for (int d = 0; d < n_digits; d++) {
        gen_sine(tone, N, 0.5, digits[d].row, fs, 0.0);
        gen_sine(tmp,  N, 0.5, digits[d].col, fs, 0.0);
        signal_add(tone, tmp, N);

        char detected = dtmf_detect(tone, N, fs);
        int match = (detected == digits[d].key);
        correct += match;

        printf("  '%c'    %-10.0f %-10.0f '%c' %s\n",
               digits[d].key, digits[d].row, digits[d].col,
               detected, match ? "✓" : "✗");
    }

    printf("\n  Accuracy: %d/%d correct\n", correct, n_digits);
    free(tone); free(tmp);
}

/* ------------------------------------------------------------------ */
/*  Demo 3: Generalised Goertzel — non-integer frequency              */
/*                                                                    */
/*  Detect a frequency that doesn't align with a DFT bin.             */
/* ------------------------------------------------------------------ */
static void demo_generalised_goertzel(void)
{
    printf("\n=== Demo 3: Generalised Goertzel (Non-Integer Frequency) ===\n\n");

    const int    N  = 1000;   /* not a power of 2! */
    const double fs = 8000.0;
    const double f_target = 1234.5;   /* not a DFT bin */

    double *x = (double *)malloc((size_t)N * sizeof(double));
    gen_sine(x, N, 1.0, f_target, fs, 0.0);

    /* Scan a range of frequencies with Goertzel */
    printf("  Scanning 1000–1500 Hz in 10 Hz steps:\n\n");
    printf("  %8s  %10s\n", "Freq", "|X(f)|");
    printf("  %8s  %10s\n", "----", "------");

    double peak_freq = 0.0, peak_mag = 0.0;
    for (double f = 1000.0; f <= 1500.0; f += 10.0) {
        Complex G = goertzel_freq(x, N, f, fs);
        double mag = complex_mag(G);
        if (mag > peak_mag) {
            peak_mag = mag;
            peak_freq = f;
        }
        if (f >= 1200.0 && f <= 1280.0)
            printf("  %8.1f  %10.2f%s\n", f, mag,
                   (fabs(f - f_target) < 15.0) ? "  ← near target" : "");
    }

    printf("\n  Target frequency: %.1f Hz\n", f_target);
    printf("  Peak detected at: %.1f Hz (|X| = %.2f)\n", peak_freq, peak_mag);

    /* Fine scan ±20 Hz */
    printf("\n  Fine scan (1 Hz steps around peak):\n");
    peak_mag = 0.0;
    for (double f = peak_freq - 20.0; f <= peak_freq + 20.0; f += 1.0) {
        Complex G = goertzel_freq(x, N, f, fs);
        double mag = complex_mag(G);
        if (mag > peak_mag) {
            peak_mag = mag;
            peak_freq = f;
        }
    }
    printf("  Refined peak: %.1f Hz (error: %.1f Hz)\n",
           peak_freq, fabs(peak_freq - f_target));

    free(x);
}

/* ------------------------------------------------------------------ */
/*  Demo 4: Sliding DFT — real-time frequency tracking                */
/*                                                                    */
/*  Track a frequency bin as a chirp signal sweeps through it.        */
/*                                                                    */
/*   Signal: chirp 500→2000 Hz over 4096 samples                     */
/*   Track: bin at 1000 Hz                                            */
/*   Output: magnitude over time shows peak when chirp passes 1 kHz  */
/* ------------------------------------------------------------------ */
static void demo_sliding_dft(void)
{
    printf("\n=== Demo 4: Sliding DFT — Frequency Tracking ===\n\n");

    const int    N_total = 4096;
    const int    win     = 256;    /* DFT window size */
    const double fs      = 8000.0;
    const double f_track = 1000.0;  /* frequency to track */

    int k = (int)(f_track * win / fs);
    printf("  Window: %d samples, tracking bin k=%d (%.0f Hz)\n",
           win, k, f_track);

    double *chirp = (double *)malloc((size_t)N_total * sizeof(double));
    gen_chirp(chirp, N_total, 1.0, 500.0, 2000.0, fs);

    SlidingDFT sdft;
    sliding_dft_init(&sdft, win, k);

    double *mag_track = (double *)malloc((size_t)N_total * sizeof(double));
    double *idx       = (double *)malloc((size_t)N_total * sizeof(double));

    for (int i = 0; i < N_total; i++) {
        Complex bin = sliding_dft_update(&sdft, chirp[i]);
        mag_track[i] = complex_mag(bin);
        idx[i] = (double)i;
    }

    /* Find peak (should be near where chirp = 1000 Hz) */
    int peak_idx = 0;
    for (int i = win; i < N_total; i++) {
        if (mag_track[i] > mag_track[peak_idx]) peak_idx = i;
    }

    /* Expected: chirp hits 1000 Hz at sample (1000-500)/(2000-500)*N_total */
    double expected_sample = (f_track - 500.0) / (2000.0 - 500.0) * N_total;
    printf("  Peak magnitude at sample %d\n", peak_idx);
    printf("  Expected (chirp hits %.0f Hz): ~%.0f\n", f_track, expected_sample);
    printf("  Error: %.0f samples\n", fabs((double)peak_idx - expected_sample));

    /* Plot */
    gp_plot_1("ch19", "sliding_dft",
              "Sliding DFT — Tracking 1 kHz Bin During Chirp",
              "Sample", "|X[k]|",
              idx, mag_track, N_total, "lines");
    printf("  → plots/ch19/sliding_dft.png\n");

    sliding_dft_free(&sdft);
    free(chirp); free(mag_track); free(idx);
}

/* ------------------------------------------------------------------ */
/*  Demo 5: Algorithm comparison                                      */
/*                                                                    */
/*  Compare Goertzel and FFT performance for varying numbers of bins. */
/* ------------------------------------------------------------------ */
static void demo_comparison(void)
{
    printf("\n=== Demo 5: Algorithm Complexity Comparison ===\n\n");

    int sizes[] = {256, 1024, 4096, 16384};
    int n_sizes = 4;

    printf("  %-8s  %-15s  %-15s  %-10s\n",
           "N", "FFT (N·log₂N)", "Goertzel (N)", "Break-even");
    printf("  %-8s  %-15s  %-15s  %-10s\n",
           "---", "------------", "-----------", "----------");

    for (int s = 0; s < n_sizes; s++) {
        int N = sizes[s];
        int fft_ops = N * (int)(log2((double)N));
        int goertzel_ops = N;
        int breakeven = (int)(log2((double)N));

        printf("  %-8d  %-15d  %-15d  %d bins\n",
               N, fft_ops, goertzel_ops, breakeven);
    }

    printf("\n  Rule of thumb: use Goertzel when you need fewer than\n");
    printf("  log₂(N) frequency bins.\n");

    /* Generate a comparison plot: Goertzel power spectrum for DTMF '5' */
    const double fs = 8000.0;
    const int frame = 256;
    double *tone = (double *)malloc((size_t)frame * sizeof(double));
    double *tmp  = (double *)malloc((size_t)frame * sizeof(double));

    /* '5' = 770 Hz + 1336 Hz */
    gen_sine(tone, frame, 0.5, 770.0, fs, 0.0);
    gen_sine(tmp,  frame, 0.5, 1336.0, fs, 0.0);
    signal_add(tone, tmp, frame);

    /* Scan 0-4000 Hz */
    int n_pts = 200;
    double *freq_axis = (double *)malloc((size_t)n_pts * sizeof(double));
    double *power     = (double *)malloc((size_t)n_pts * sizeof(double));

    for (int i = 0; i < n_pts; i++) {
        freq_axis[i] = (double)i * (fs / 2.0) / n_pts;
        Complex G = goertzel_freq(tone, frame, freq_axis[i], fs);
        power[i] = 10.0 * log10(G.re * G.re + G.im * G.im + 1e-30);
    }

    gp_plot_1("ch19", "goertzel_spectrum",
              "Goertzel Spectrum Scan - DTMF 5 (770 + 1336 Hz)",
              "Frequency (Hz)", "Power (dB)",
              freq_axis, power, n_pts, "lines");
    printf("  → plots/ch19/goertzel_spectrum.png\n");

    free(tone); free(tmp); free(freq_axis); free(power);
}

/* ================================================================== */

int main(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  Chapter 19: Advanced FFT — Goertzel & Sliding DFT     ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    gp_init("ch19");

    demo_goertzel_vs_fft();
    demo_dtmf();
    demo_generalised_goertzel();
    demo_sliding_dft();
    demo_comparison();

    printf("\n=== Chapter 19 Complete ===\n");
    return 0;
}
