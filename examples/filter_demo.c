/**
 * @file filter_demo.c
 * @brief FIR filter demonstration — lowpass filters a noisy signal.
 *
 * This example corresponds to tutorial/04-digital-filters.md.
 *
 * What it does:
 *   1. Creates a 200 Hz sine wave contaminated with high-frequency noise
 *   2. Designs a lowpass FIR filter (cutoff = 500 Hz at 8000 Hz sample rate)
 *   3. Filters the signal
 *   4. Shows RMS before/after to demonstrate noise reduction
 *
 * Build & run:
 *   make && ./build/bin/filter_demo
 */

#include <stdio.h>
#include <math.h>
#include "filter.h"
#include "dsp_utils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define N      256        /* Signal length            */
#define FS     8000.0     /* Sample rate (Hz)         */
#define FREQ   200.0      /* Desired signal frequency */
#define TAPS   31         /* Filter order (odd)       */

int main(void) {
    double noisy[N], clean[N], filtered[N];
    double h[TAPS];

    printf("=== FIR Filter Demo ===\n");
    printf("Signal: %.0f Hz sine + high-freq noise\n", FREQ);
    printf("Filter: %d-tap lowpass, cutoff = 500 Hz\n\n", TAPS);

    /* ── Step 1: Generate noisy signal ───────────────────────────── */
    /*
     * We add a "pseudo-random" high-frequency component to simulate noise.
     * In practice you'd use real sensor data or audio samples.
     */
    for (int i = 0; i < N; i++) {
        double t = (double)i / FS;
        clean[i] = sin(2.0 * M_PI * FREQ * t);

        /* Simulate noise: high-frequency components */
        double noise = 0.3 * sin(2.0 * M_PI * 2800.0 * t)
                     + 0.2 * sin(2.0 * M_PI * 3500.0 * t);
        noisy[i] = clean[i] + noise;
    }

    /* ── Step 2: Design the lowpass filter ───────────────────────── */
    /*
     * Cutoff = 500 Hz → normalized = 500/8000 = 0.0625
     * This will pass our 200 Hz signal and reject everything above ~500 Hz.
     * See tutorial/04-digital-filters.md § "Windowed-Sinc Design"
     */
    double cutoff_normalized = 500.0 / FS;   /* 0.0625 */
    fir_lowpass(h, TAPS, cutoff_normalized);

    printf("Filter coefficients (first 5 of %d):\n", TAPS);
    for (int i = 0; i < 5 && i < TAPS; i++) {
        printf("  h[%2d] = %+.6f\n", i, h[i]);
    }
    printf("  ...\n\n");

    /* ── Step 3: Apply the filter ────────────────────────────────── */
    fir_filter(noisy, filtered, N, h, TAPS);

    /* ── Step 4: Compare results ─────────────────────────────────── */
    double rms_clean    = rms(clean, N);
    double rms_noisy    = rms(noisy, N);

    /* Skip initial transient (filter settling time) */
    double rms_filtered_settled = rms(filtered + TAPS, N - TAPS);

    printf("Signal Analysis:\n");
    printf("  Clean signal RMS:    %.4f\n", rms_clean);
    printf("  Noisy signal RMS:    %.4f  (noise added %.1f%%)\n",
           rms_noisy, (rms_noisy - rms_clean) / rms_clean * 100);
    printf("  Filtered signal RMS: %.4f  (after settling)\n",
           rms_filtered_settled);

    /* Calculate error between clean and filtered */
    double error_sum = 0.0;
    for (int i = TAPS; i < N; i++) {
        double e = filtered[i] - clean[i];
        error_sum += e * e;
    }
    double error_rms = sqrt(error_sum / (N - TAPS));
    printf("  RMS error (vs clean): %.4f\n", error_rms);
    printf("\n  Noise reduction: the filter successfully removed the\n");
    printf("  high-freq components while preserving the %.0f Hz signal.\n", FREQ);

    return 0;
}
