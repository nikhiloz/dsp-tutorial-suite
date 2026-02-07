/**
 * @file 15-correlation.c
 * @brief Chapter 15 — Correlation & Autocorrelation
 *
 * Demonstrates:
 *  1. Cross-correlation to detect a known pulse in noise
 *  2. Normalised cross-correlation and peak-lag detection
 *  3. Autocorrelation for pitch estimation
 *  4. Autocorrelation of white noise (delta function)
 *  5. Correlation-based time-delay estimation
 *
 * Build:  make           (builds ch15 among all targets)
 * Run:    build/bin/ch15
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "fft.h"
#include "dsp_utils.h"
#include "signal_gen.h"
#include "correlation.h"
#include "gnuplot.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ------------------------------------------------------------------ */
/*  Demo 1: Detect known pulse in noisy signal                        */
/* ------------------------------------------------------------------ */
static void demo_pulse_detection(void)
{
    printf("\n=== Demo 1: Pulse Detection via Cross-Correlation ===\n");

    const int N = 1024;
    const int pulse_len = 32;
    const int pulse_pos = 400;

    /* Create a short pulse (raised cosine) */
    double pulse[32];
    for (int i = 0; i < pulse_len; i++)
        pulse[i] = 3.0 * (1.0 - cos(2.0 * M_PI * i / (pulse_len - 1)));

    /* Embed pulse in noise */
    double *signal = (double *)calloc((size_t)N, sizeof(double));
    double *noise  = (double *)malloc((size_t)N * sizeof(double));

    gen_gaussian_noise(noise, N, 0.0, 0.2, 77);
    memcpy(signal, noise, (size_t)N * sizeof(double));
    for (int i = 0; i < pulse_len && pulse_pos + i < N; i++)
        signal[pulse_pos + i] += pulse[i];

    /* Cross-correlate */
    int r_len = N + pulse_len - 1;
    double *r = (double *)calloc((size_t)r_len, sizeof(double));
    int ret = xcorr(signal, N, pulse, pulse_len, r);
    printf("  xcorr output: %d samples\n", ret);

    int peak_lag = xcorr_peak_lag(r, r_len, N - 1);
    int detected_pos = -peak_lag;   /* pulse position = -lag */
    printf("  Pulse embedded at sample %d\n", pulse_pos);
    printf("  Peak cross-correlation at lag %d → detected position %d (error = %d)\n",
           peak_lag, detected_pos, abs(detected_pos - pulse_pos));

    /* Plot: signal and xcorr */
    double *lag_axis = (double *)malloc((size_t)r_len * sizeof(double));
    for (int i = 0; i < r_len; i++)
        lag_axis[i] = (double)(i - (pulse_len - 1));

    GpSeries series[2];
    /* Show only the portion of r that corresponds to positive lags (in-range) */
    int plot_start = pulse_len - 1;   /* lag=0 */
    int plot_len   = N;                /* lags 0 to N-1 */

    double *idx = (double *)malloc((size_t)N * sizeof(double));
    for (int i = 0; i < N; i++) idx[i] = (double)i;

    series[0].label = "Signal + Noise";
    series[0].x     = idx;
    series[0].y     = signal;
    series[0].n     = N;
    series[0].style = "lines";

    /* Normalise r for display */
    double rmax = 0;
    for (int i = 0; i < plot_len; i++) {
        double v = fabs(r[plot_start + i]);
        if (v > rmax) rmax = v;
    }
    double *r_norm = (double *)malloc((size_t)plot_len * sizeof(double));
    for (int i = 0; i < plot_len; i++)
        r_norm[i] = r[plot_start + i] / (rmax + 1e-30) * 2.0;

    series[1].label = "Cross-correlation (scaled)";
    series[1].x     = idx;
    series[1].y     = r_norm;
    series[1].n     = plot_len;
    series[1].style = "lines";

    gp_plot_multi("ch15", "pulse_detection",
                  "Pulse Detection via Cross-Correlation",
                  "Sample", "Amplitude",
                  series, 2);
    printf("  → plots/ch15/pulse_detection.png\n");

    free(signal); free(noise); free(r); free(lag_axis);
    free(idx); free(r_norm);
}

/* ------------------------------------------------------------------ */
/*  Demo 2: Normalised cross-correlation                              */
/* ------------------------------------------------------------------ */
static void demo_normalized_xcorr(void)
{
    printf("\n=== Demo 2: Normalised Cross-Correlation ===\n");

    const int N = 256;
    double x[256], y[256];

    /* x = sine, y = delayed sine */
    const double fs = 1000.0;
    gen_sine(x, N, 1.0, 50.0, fs, 0.0);
    gen_sine(y, N, 1.0, 50.0, fs, M_PI / 4.0);   /* 45° phase shift */

    int r_len = 2 * N - 1;
    double *r = (double *)malloc((size_t)r_len * sizeof(double));
    int ret = xcorr_normalized(x, N, y, N, r);
    printf("  Normalised xcorr: %d samples\n", ret);

    int peak = xcorr_peak_lag(r, r_len, N - 1);
    double delay_samples = (double)peak;
    double delay_ms = delay_samples / fs * 1000.0;
    printf("  Peak lag: %d samples (%.2f ms)\n", peak, delay_ms);
    printf("  Expected ~%.1f samples for 45° at 50 Hz/1 kHz\n",
           (M_PI / 4.0) / (2.0 * M_PI * 50.0 / fs));

    /* Plot normalised xcorr */
    double *lags = (double *)malloc((size_t)r_len * sizeof(double));
    for (int i = 0; i < r_len; i++)
        lags[i] = (double)(i - (N - 1));

    gp_plot_1("ch15", "normalized_xcorr",
              "Normalised Cross-Correlation (50 Hz, 45° shift)",
              "Lag (samples)", "Correlation",
              lags, r, r_len, "lines");
    printf("  → plots/ch15/normalized_xcorr.png\n");

    free(r); free(lags);
}

/* ------------------------------------------------------------------ */
/*  Demo 3: Autocorrelation for pitch estimation                      */
/* ------------------------------------------------------------------ */
static void demo_pitch_estimation(void)
{
    printf("\n=== Demo 3: Autocorrelation — Pitch Estimation ===\n");

    const int    N  = 2048;
    const double fs = 16000.0;
    const double f0 = 440.0;   /* A4 */

    double *x = (double *)malloc((size_t)N * sizeof(double));
    double *noise = (double *)malloc((size_t)N * sizeof(double));

    /* Generate a harmonic-rich signal (fundamental + harmonics) */
    double freqs[4] = {f0, 2*f0, 3*f0, 4*f0};
    double amps[4]  = {1.0, 0.5, 0.25, 0.125};
    gen_multi_tone(x, N, freqs, amps, 4, fs);

    gen_gaussian_noise(noise, N, 0.0, 0.3, 55);
    signal_add(x, noise, N);

    int r_len = 2 * N - 1;
    double *r = (double *)malloc((size_t)r_len * sizeof(double));
    autocorr_normalized(x, N, r);

    /* Find first peak after lag 0 (skip lag=0 which is always 1.0) */
    int centre = N - 1;
    int min_lag = (int)(fs / 2000.0);   /* max 2 kHz pitch = 8 samples */
    int max_lag = (int)(fs / 50.0);     /* min 50 Hz pitch = 320 samples */

    int best_lag = min_lag;
    double best_val = r[centre + min_lag];
    for (int lag = min_lag + 1; lag <= max_lag && centre + lag < r_len; lag++) {
        if (r[centre + lag] > best_val) {
            best_val = r[centre + lag];
            best_lag = lag;
        }
    }

    double estimated_f0 = fs / (double)best_lag;
    printf("  True pitch: %.1f Hz\n", f0);
    printf("  Autocorr peak at lag %d → %.1f Hz (error: %.1f%%)\n",
           best_lag, estimated_f0,
           fabs(estimated_f0 - f0) / f0 * 100.0);

    /* Plot positive-lag portion of autocorrelation */
    int plot_len = max_lag + 50;
    if (plot_len > N) plot_len = N;
    double *lags = (double *)malloc((size_t)plot_len * sizeof(double));
    double *rpos = (double *)malloc((size_t)plot_len * sizeof(double));
    for (int i = 0; i < plot_len; i++) {
        lags[i] = (double)i;
        rpos[i] = r[centre + i];
    }

    gp_plot_1("ch15", "autocorr_pitch",
              "Autocorrelation — Pitch Detection (A4 = 440 Hz)",
              "Lag (samples)", "Normalised Autocorrelation",
              lags, rpos, plot_len, "lines");
    printf("  → plots/ch15/autocorr_pitch.png\n");

    free(x); free(noise); free(r); free(lags); free(rpos);
}

/* ------------------------------------------------------------------ */
/*  Demo 4: White noise autocorrelation — delta function              */
/* ------------------------------------------------------------------ */
static void demo_noise_autocorr(void)
{
    printf("\n=== Demo 4: Autocorrelation of White Noise ===\n");

    const int N = 4096;
    double *x = (double *)malloc((size_t)N * sizeof(double));
    gen_gaussian_noise(x, N, 0.0, 1.0, 123);

    int r_len = 2 * N - 1;
    double *r = (double *)malloc((size_t)r_len * sizeof(double));
    autocorr_normalized(x, N, r);

    int centre = N - 1;
    printf("  r[0] (lag 0)  = %.4f (should be 1.0)\n", r[centre]);
    printf("  r[1] (lag 1)  = %.4f (should be ~0)\n",  r[centre + 1]);
    printf("  r[10] (lag 10) = %.4f (should be ~0)\n", r[centre + 10]);

    /* Plot lags -50 to +50 */
    int plot_half = 50;
    int plot_len  = 2 * plot_half + 1;
    double *lags = (double *)malloc((size_t)plot_len * sizeof(double));
    double *rp   = (double *)malloc((size_t)plot_len * sizeof(double));
    for (int i = 0; i < plot_len; i++) {
        lags[i] = (double)(i - plot_half);
        rp[i]   = r[centre + i - plot_half];
    }

    gp_plot_1("ch15", "noise_autocorr",
              "Autocorrelation of White Noise — Impulse at Lag 0",
              "Lag (samples)", "Normalised Autocorrelation",
              lags, rp, plot_len, "impulses");
    printf("  → plots/ch15/noise_autocorr.png\n");

    free(x); free(r); free(lags); free(rp);
}

/* ------------------------------------------------------------------ */
/*  Demo 5: Time-delay estimation                                     */
/* ------------------------------------------------------------------ */
static void demo_time_delay(void)
{
    printf("\n=== Demo 5: Time-Delay Estimation ===\n");

    const int    N  = 2048;
    const double fs = 44100.0;
    const int    true_delay = 73;   /* samples */

    /* x is the original, y is a delayed + noisy copy */
    double *x = (double *)malloc((size_t)N * sizeof(double));
    double *y = (double *)calloc((size_t)N, sizeof(double));
    double *n1 = (double *)malloc((size_t)N * sizeof(double));
    double *n2 = (double *)malloc((size_t)N * sizeof(double));

    /* Broadband chirp signal */
    gen_chirp(x, N, 1.0, 100.0, 5000.0, fs);

    /* y[n] = 0.8 * x[n - delay] + noise */
    for (int i = true_delay; i < N; i++)
        y[i] = 0.8 * x[i - true_delay];

    gen_gaussian_noise(n1, N, 0.0, 0.3, 41);
    gen_gaussian_noise(n2, N, 0.0, 0.3, 42);
    signal_add(x, n1, N);
    signal_add(y, n2, N);

    int r_len = 2 * N - 1;
    double *r = (double *)malloc((size_t)r_len * sizeof(double));
    xcorr_normalized(x, N, y, N, r);

    int est_delay = xcorr_peak_lag(r, r_len, N - 1);
    double delay_ms = (double)est_delay / fs * 1000.0;

    printf("  True delay:      %d samples (%.3f ms)\n",
           true_delay, (double)true_delay / fs * 1000.0);
    printf("  Estimated delay: %d samples (%.3f ms)\n",
           est_delay, delay_ms);
    printf("  Error: %d samples\n", abs(est_delay - true_delay));

    /* Plot a zoomed portion around the peak */
    int plot_half = 150;
    int plot_len  = 2 * plot_half + 1;
    double *lags = (double *)malloc((size_t)plot_len * sizeof(double));
    double *rp   = (double *)malloc((size_t)plot_len * sizeof(double));
    int centre = N - 1;
    for (int i = 0; i < plot_len; i++) {
        int lag = i - plot_half;
        lags[i] = (double)lag;
        int idx = centre + lag;
        rp[i] = (idx >= 0 && idx < r_len) ? r[idx] : 0.0;
    }

    gp_plot_1("ch15", "time_delay",
              "Time-Delay Estimation via Cross-Correlation",
              "Lag (samples)", "Normalised Cross-Correlation",
              lags, rp, plot_len, "lines");
    printf("  → plots/ch15/time_delay.png\n");

    free(x); free(y); free(n1); free(n2); free(r);
    free(lags); free(rp);
}

/* ================================================================== */

int main(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  Chapter 15: Correlation & Autocorrelation             ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    gp_init("ch15");

    demo_pulse_detection();
    demo_normalized_xcorr();
    demo_pitch_estimation();
    demo_noise_autocorr();
    demo_time_delay();

    printf("\n=== Chapter 15 Complete ===\n");
    return 0;
}
