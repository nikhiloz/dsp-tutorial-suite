/**
 * @file 16-overlap-add-save.c
 * @brief Chapter 16 — Overlap-Add/Save Streaming Convolution
 *
 * Demonstrates:
 *  1. Overlap-Add: block FIR filtering with boundary handling
 *  2. Overlap-Save: alternative with discard strategy
 *  3. OLA vs direct convolution — verify identical output
 *  4. Streaming a long signal through OLA in chunks
 *  5. Latency & efficiency analysis
 *
 * ── The Streaming Problem ────────────────────────────────────────
 *
 *   Direct convolution of L samples with M-tap filter: O(L·M).
 *   For long signals (audio, radar), L may be millions of samples.
 *
 *   Solution: process in blocks using FFT-based convolution.
 *   But naïve blocking creates boundary artefacts!
 *
 * ── Overlap-Add ──────────────────────────────────────────────────
 *
 *   Block 0:  [===input===|000000]  → FFT → ×H → IFFT → [==valid==|tail]
 *   Block 1:  [===input===|000000]  → FFT → ×H → IFFT → [==valid==|tail]
 *
 *   Output:   [==valid0== ]
 *              [==tail0==+valid1====]
 *                          [==tail1==+valid2====]
 *
 *   ✓ Tails from adjacent blocks overlap by M-1 samples and are summed.
 *   ✓ Output = exact linear convolution.
 *
 * ── Overlap-Save ─────────────────────────────────────────────────
 *
 *   Input segment:  [prev M-1 | new L samples | pad]
 *                    └─overlap─┘
 *
 *   → FFT → ×H → IFFT → [discard M-1 | valid L samples]
 *                         └──artefact──┘
 *
 *   ✓ No accumulation needed — just discard corrupted prefix.
 *   ✓ Slightly more memory-efficient than OLA.
 *
 * Build:  make           (builds ch16 among all targets)
 * Run:    build/bin/ch16
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "streaming.h"
#include "fft.h"
#include "filter.h"
#include "dsp_utils.h"
#include "signal_gen.h"
#include "convolution.h"
#include "gnuplot.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ------------------------------------------------------------------ */
/*  Demo 1: Overlap-Add basic demo                                    */
/*                                                                    */
/*  Filter a short signal with OLA and compare to direct convolution. */
/* ------------------------------------------------------------------ */
static void demo_ola_basic(void)
{
    printf("\n=== Demo 1: Overlap-Add — Basic ===\n\n");

    const int sig_len  = 512;
    const int taps     = 31;
    const int blk_size = 128;

    /* Design a lowpass filter */
    double h[31];
    fir_lowpass(h, taps, 0.25);   /* normalised cutoff = 0.25 */

    /* Generate test signal: two tones + noise */
    double *x     = (double *)malloc((size_t)sig_len * sizeof(double));
    double *noise = (double *)malloc((size_t)sig_len * sizeof(double));
    gen_sine(x, sig_len, 1.0, 300.0, 8000.0, 0.0);
    double tmp[512];
    gen_sine(tmp, sig_len, 0.8, 3500.0, 8000.0, 0.0);
    signal_add(x, tmp, sig_len);
    gen_gaussian_noise(noise, sig_len, 0.0, 0.2, 42);
    signal_add(x, noise, sig_len);

    /* Direct FIR for reference */
    double *y_ref = (double *)calloc((size_t)sig_len, sizeof(double));
    fir_filter(x, y_ref, sig_len, h, taps);

    /* Overlap-Add */
    OlaState ola;
    ola_init(&ola, h, taps, blk_size);

    double *y_ola = (double *)calloc((size_t)sig_len, sizeof(double));
    int n_blocks = sig_len / blk_size;

    for (int b = 0; b < n_blocks; b++)
        ola_process(&ola, x + b * blk_size, y_ola + b * blk_size);

    /* Compare */
    double max_err = 0.0;
    for (int i = 0; i < sig_len; i++) {
        double err = fabs(y_ref[i] - y_ola[i]);
        if (err > max_err) max_err = err;
    }

    printf("  Signal: %d samples, Filter: %d taps, Block: %d\n",
           sig_len, taps, blk_size);
    printf("  FFT size: %d (auto)\n", ola.fft_size);
    printf("  Blocks processed: %d\n", n_blocks);
    printf("  Max error (OLA vs direct): %.2e\n", max_err);

    /* Plot */
    double *idx = (double *)malloc((size_t)sig_len * sizeof(double));
    for (int i = 0; i < sig_len; i++) idx[i] = (double)i;

    int plot_n = 256;
    GpSeries s[2];
    s[0].label = "Direct FIR";
    s[0].x = idx; s[0].y = y_ref; s[0].n = plot_n; s[0].style = "lines";
    s[1].label = "Overlap-Add";
    s[1].x = idx; s[1].y = y_ola; s[1].n = plot_n; s[1].style = "lines";

    gp_plot_multi("16-overlap-add-save", "ola_vs_direct",
                  "Overlap-Add vs Direct FIR Convolution",
                  "Sample", "Amplitude", s, 2);
    printf("  → plots/ch16/ola_vs_direct.png\n");

    ola_free(&ola);
    free(x); free(noise); free(y_ref); free(y_ola); free(idx);
}

/* ------------------------------------------------------------------ */
/*  Demo 2: Overlap-Save basic demo                                   */
/* ------------------------------------------------------------------ */
static void demo_ols_basic(void)
{
    printf("\n=== Demo 2: Overlap-Save — Basic ===\n\n");

    const int sig_len  = 512;
    const int taps     = 31;
    const int blk_size = 128;

    double h[31];
    fir_lowpass(h, taps, 0.25);

    double *x  = (double *)malloc((size_t)sig_len * sizeof(double));
    gen_sine(x, sig_len, 1.0, 300.0, 8000.0, 0.0);

    /* Direct reference */
    double *y_ref = (double *)calloc((size_t)sig_len, sizeof(double));
    fir_filter(x, y_ref, sig_len, h, taps);

    /* Overlap-Save */
    OlsState ols;
    ols_init(&ols, h, taps, blk_size);

    double *y_ols = (double *)calloc((size_t)sig_len, sizeof(double));
    int n_blocks = sig_len / blk_size;

    for (int b = 0; b < n_blocks; b++)
        ols_process(&ols, x + b * blk_size, y_ols + b * blk_size);

    double max_err = 0.0;
    /* Skip first block (M-1 warm-up samples may differ) */
    for (int i = blk_size; i < sig_len; i++) {
        double err = fabs(y_ref[i] - y_ols[i]);
        if (err > max_err) max_err = err;
    }

    printf("  Overlap-Save: %d blocks × %d samples\n", n_blocks, blk_size);
    printf("  FFT size: %d\n", ols.fft_size);
    printf("  Max error (OLS vs direct, after warm-up): %.2e\n", max_err);

    ols_free(&ols);
    free(x); free(y_ref); free(y_ols);
}

/* ------------------------------------------------------------------ */
/*  Demo 3: OLA vs OLS — both match                                   */
/* ------------------------------------------------------------------ */
static void demo_ola_vs_ols(void)
{
    printf("\n=== Demo 3: OLA vs OLS Comparison ===\n\n");

    const int sig_len  = 2048;
    const int taps     = 63;
    const int blk_size = 256;

    double *h = (double *)malloc((size_t)taps * sizeof(double));
    fir_lowpass(h, taps, 0.3);

    double *x = (double *)malloc((size_t)sig_len * sizeof(double));
    gen_chirp(x, sig_len, 1.0, 100.0, 3500.0, 8000.0);

    /* OLA */
    OlaState ola;
    ola_init(&ola, h, taps, blk_size);
    double *y_ola = (double *)calloc((size_t)sig_len, sizeof(double));
    for (int b = 0; b < sig_len / blk_size; b++)
        ola_process(&ola, x + b * blk_size, y_ola + b * blk_size);

    /* OLS */
    OlsState ols;
    ols_init(&ols, h, taps, blk_size);
    double *y_ols = (double *)calloc((size_t)sig_len, sizeof(double));
    for (int b = 0; b < sig_len / blk_size; b++)
        ols_process(&ols, x + b * blk_size, y_ols + b * blk_size);

    /* Compare (skip first block for OLS warm-up) */
    double max_diff = 0.0;
    for (int i = blk_size; i < sig_len; i++) {
        double diff = fabs(y_ola[i] - y_ols[i]);
        if (diff > max_diff) max_diff = diff;
    }

    printf("  63-tap filter, 256-sample blocks, 2048 total samples\n");
    printf("  OLA FFT size: %d,  OLS FFT size: %d\n", ola.fft_size, ols.fft_size);
    printf("  Max |OLA - OLS|: %.2e  (both implement linear convolution)\n",
           max_diff);

    ola_free(&ola); ols_free(&ols);
    free(h); free(x); free(y_ola); free(y_ols);
}

/* ------------------------------------------------------------------ */
/*  Demo 4: Streaming a long signal through OLA                       */
/*                                                                    */
/*  Simulate a real-time scenario: process 16384 samples in 128-      */
/*  sample blocks through a lowpass filter.                           */
/* ------------------------------------------------------------------ */
static void demo_streaming(void)
{
    printf("\n=== Demo 4: Streaming 16K Samples Through OLA ===\n\n");

    const int total  = 16384;
    const int taps   = 101;
    const int blk    = 128;
    const double fs  = 44100.0;

    double *h = (double *)malloc((size_t)taps * sizeof(double));
    fir_lowpass(h, taps, 0.1);   /* tight lowpass: ~2.2 kHz at 44.1 kHz */

    /* Long signal: multi-tone + noise */
    double *x     = (double *)malloc((size_t)total * sizeof(double));
    double *noise = (double *)malloc((size_t)total * sizeof(double));
    double freqs[3] = {440.0, 1000.0, 5000.0};
    double amps[3]  = {0.5, 0.3, 0.4};
    gen_multi_tone(x, total, freqs, amps, 3, fs);
    gen_gaussian_noise(noise, total, 0.0, 0.2, 77);
    signal_add(x, noise, total);

    /* OLA streaming */
    OlaState ola;
    ola_init(&ola, h, taps, blk);

    double *y = (double *)calloc((size_t)total, sizeof(double));
    int n_blocks = total / blk;

    for (int b = 0; b < n_blocks; b++)
        ola_process(&ola, x + b * blk, y + b * blk);

    printf("  Total samples: %d  (%.1f ms at %.0f Hz)\n",
           total, 1000.0 * total / fs, fs);
    printf("  Block size: %d  (%.1f ms per block)\n",
           blk, 1000.0 * blk / fs);
    printf("  Blocks: %d\n", n_blocks);
    printf("  FFT size: %d\n", ola.fft_size);
    printf("  Filter: %d taps (cutoff ~%.0f Hz)\n", taps, 0.1 * fs / 2.0);

    /* Plot a portion: input vs output */
    int plot_start = 4096;
    int plot_len   = 512;
    double *idx = (double *)malloc((size_t)plot_len * sizeof(double));
    for (int i = 0; i < plot_len; i++) idx[i] = (double)(plot_start + i);

    GpSeries s[2];
    s[0].label = "Input (multi-tone + noise)";
    s[0].x = idx; s[0].y = x + plot_start; s[0].n = plot_len; s[0].style = "lines";
    s[1].label = "OLA output (lowpass)";
    s[1].x = idx; s[1].y = y + plot_start; s[1].n = plot_len; s[1].style = "lines";

    gp_plot_multi("16-overlap-add-save", "streaming_ola",
                  "Streaming OLA: 101-tap LP on 44.1 kHz Multi-Tone",
                  "Sample", "Amplitude", s, 2);
    printf("  → plots/ch16/streaming_ola.png\n");

    ola_free(&ola);
    free(h); free(x); free(noise); free(y); free(idx);
}

/* ------------------------------------------------------------------ */
/*  Demo 5: Efficiency analysis                                        */
/*                                                                    */
/*  Compare operations: direct FIR vs FFT-OLA for different lengths.  */
/* ------------------------------------------------------------------ */
static void demo_efficiency(void)
{
    printf("\n=== Demo 5: Efficiency — Direct vs OLA ===\n\n");

    printf("  %-10s %-8s %-15s %-15s %-10s\n",
           "Sig Len", "Taps", "Direct (L×M)", "OLA (approx)", "Speedup");
    printf("  %-10s %-8s %-15s %-15s %-10s\n",
           "-------", "----", "-----------", "------------", "-------");

    int lengths[] = {1024, 4096, 16384, 65536};
    int tap_counts[] = {31, 101, 255};

    for (int li = 0; li < 4; li++) {
        for (int ti = 0; ti < 3; ti++) {
            int L = lengths[li];
            int M = tap_counts[ti];
            int blk = 256;

            long direct_ops = (long)L * M;

            /* OLA: ceil(L/blk) blocks, each costs ~5·N·log₂(N) */
            int N = next_power_of_2(blk + M - 1);
            int n_blocks = (L + blk - 1) / blk;
            int log2N = 0;
            for (int nn = N; nn > 1; nn >>= 1) log2N++;
            long ola_ops = (long)n_blocks * 5 * N * log2N;

            double speedup = (double)direct_ops / (double)ola_ops;

            printf("  %-10d %-8d %-15ld %-15ld ×%.1f\n",
                   L, M, direct_ops, ola_ops, speedup);
        }
    }

    printf("\n  OLA wins when M is large (>~32 taps) and L >> M.\n");
}

/* ================================================================== */

int main(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  Chapter 16: Overlap-Add/Save Streaming Convolution    ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    gp_init("16-overlap-add-save");

    demo_ola_basic();
    demo_ols_basic();
    demo_ola_vs_ols();
    demo_streaming();
    demo_efficiency();

    printf("\n=== Chapter 16 Complete ===\n");
    return 0;
}
