/**
 * @file 14-psd-welch.c
 * @brief Chapter 14 — Power Spectral Density & Welch's Method
 *
 * Demonstrates:
 *  1. Basic periodogram of a two-tone signal
 *  2. Periodogram of noisy signal (high variance)
 *  3. Welch's method — averaged, lower-variance PSD
 *  4. Effect of segment length on Welch resolution
 *  5. Cross-PSD of correlated signals
 *
 * Build:  make           (builds ch14 among all targets)
 * Run:    build/bin/ch14
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "fft.h"
#include "dsp_utils.h"
#include "signal_gen.h"
#include "spectrum.h"
#include "gnuplot.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ------------------------------------------------------------------ */
/*  Demo 1: Basic periodogram of a clean two-tone signal              */
/* ------------------------------------------------------------------ */
static void demo_periodogram_clean(void)
{
    printf("\n=== Demo 1: Periodogram of a Clean Two-Tone Signal ===\n");

    const int    N  = 1024;
    const double fs = 8000.0;
    const double f1 = 1000.0;
    const double f2 = 2500.0;

    double x[1024];
    gen_sine(x, N, 1.0, f1, fs, 0.0);
    double tmp[1024];
    gen_sine(tmp, N, 0.5, f2, fs, 0.0);
    signal_add(x, tmp, N);

    int nfft   = 1024;
    int n_bins = nfft / 2 + 1;
    double psd[513], psd_db[513], freq[513];

    int ret = periodogram(x, N, psd, nfft);
    printf("  Periodogram: %d bins\n", ret);

    psd_to_db(psd, psd_db, n_bins, -120.0);
    psd_freq_axis(freq, n_bins, fs);

    /* Find peaks */
    double peak1 = -200.0, peak2 = -200.0;
    int bin1 = 0, bin2 = 0;
    for (int k = 0; k < n_bins; k++) {
        if (freq[k] > 800 && freq[k] < 1200 && psd_db[k] > peak1) {
            peak1 = psd_db[k]; bin1 = k;
        }
        if (freq[k] > 2300 && freq[k] < 2700 && psd_db[k] > peak2) {
            peak2 = psd_db[k]; bin2 = k;
        }
    }
    printf("  Peak at %.0f Hz: %.1f dB\n", freq[bin1], peak1);
    printf("  Peak at %.0f Hz: %.1f dB\n", freq[bin2], peak2);

    /* Plot */
    gp_plot_spectrum("ch14", "periodogram_clean",
                     "Periodogram — Clean Two-Tone (1 kHz + 2.5 kHz)",
                     freq, psd_db, n_bins);
    printf("  → plots/ch14/periodogram_clean.png\n");
}

/* ------------------------------------------------------------------ */
/*  Demo 2: Periodogram of a noisy signal — high variance             */
/* ------------------------------------------------------------------ */
static void demo_periodogram_noisy(void)
{
    printf("\n=== Demo 2: Periodogram of a Noisy Signal (High Variance) ===\n");

    const int    N  = 4096;
    const double fs = 8000.0;
    const double f1 = 500.0;

    double *x     = (double *)malloc((size_t)N * sizeof(double));
    double *noise = (double *)malloc((size_t)N * sizeof(double));

    gen_sine(x, N, 1.0, f1, fs, 0.0);
    gen_gaussian_noise(noise, N, 0.0, 2.0, 42);
    signal_add(x, noise, N);

    int nfft   = 4096;
    int n_bins = nfft / 2 + 1;
    double *psd    = (double *)calloc((size_t)n_bins, sizeof(double));
    double *psd_db = (double *)malloc((size_t)n_bins * sizeof(double));
    double *freq   = (double *)malloc((size_t)n_bins * sizeof(double));

    periodogram(x, N, psd, nfft);
    psd_to_db(psd, psd_db, n_bins, -120.0);
    psd_freq_axis(freq, n_bins, fs);

    printf("  Single periodogram — jagged, high variance\n");
    printf("  The 500 Hz tone is hidden in noise fluctuations.\n");

    gp_plot_spectrum("ch14", "periodogram_noisy",
                     "Single Periodogram — Tone + Noise (High Variance)",
                     freq, psd_db, n_bins);
    printf("  → plots/ch14/periodogram_noisy.png\n");

    free(x); free(noise); free(psd); free(psd_db); free(freq);
}

/* ------------------------------------------------------------------ */
/*  Demo 3: Welch's method — averaged, lower variance                 */
/* ------------------------------------------------------------------ */
static void demo_welch(void)
{
    printf("\n=== Demo 3: Welch's Method — Averaged PSD ===\n");

    const int    N  = 4096;
    const double fs = 8000.0;
    const double f1 = 500.0;

    double *x     = (double *)malloc((size_t)N * sizeof(double));
    double *noise = (double *)malloc((size_t)N * sizeof(double));

    gen_sine(x, N, 1.0, f1, fs, 0.0);
    gen_gaussian_noise(noise, N, 0.0, 2.0, 42);
    signal_add(x, noise, N);

    int seg_len = 512;
    int nfft    = 512;
    int overlap = 256;   /* 50% overlap */
    int n_bins  = nfft / 2 + 1;

    double *psd    = (double *)calloc((size_t)n_bins, sizeof(double));
    double *psd_db = (double *)malloc((size_t)n_bins * sizeof(double));
    double *freq   = (double *)malloc((size_t)n_bins * sizeof(double));

    int n_segs = welch_psd(x, N, psd, nfft, seg_len, overlap, hann_window);
    psd_to_db(psd, psd_db, n_bins, -120.0);
    psd_freq_axis(freq, n_bins, fs);

    printf("  Welch PSD: %d segments averaged\n", n_segs);
    printf("  Much smoother — 500 Hz peak clearly visible.\n");

    gp_plot_spectrum("ch14", "welch_psd",
                     "Welch PSD — Same Signal, Much Lower Variance",
                     freq, psd_db, n_bins);
    printf("  → plots/ch14/welch_psd.png\n");

    free(x); free(noise); free(psd); free(psd_db); free(freq);
}

/* ------------------------------------------------------------------ */
/*  Demo 4: Welch resolution trade-off (segment length)               */
/* ------------------------------------------------------------------ */
static void demo_welch_resolution(void)
{
    printf("\n=== Demo 4: Welch Resolution vs Variance Trade-off ===\n");

    const int    N  = 8192;
    const double fs = 8000.0;

    double *x     = (double *)malloc((size_t)N * sizeof(double));
    double *noise = (double *)malloc((size_t)N * sizeof(double));

    /* Two closely-spaced tones */
    double freqs[2]  = {900.0, 1100.0};
    double amps[2]   = {1.0, 1.0};
    gen_multi_tone(x, N, freqs, amps, 2, fs);
    gen_gaussian_noise(noise, N, 0.0, 1.0, 99);
    signal_add(x, noise, N);

    /* Three different segment lengths */
    int seg_lens[3] = {128, 512, 2048};
    const char *labels[3] = {"128-pt (low res)", "512-pt (medium)", "2048-pt (high res)"};

    FILE *gp = gp_open("ch14", "welch_resolution", 900, 500);
    if (gp) {
        fprintf(gp, "set title 'Welch PSD — Segment Length vs Resolution'\n");
        fprintf(gp, "set xlabel 'Frequency (Hz)'\n");
        fprintf(gp, "set ylabel 'PSD (dB)'\n");
        fprintf(gp, "set grid\n");
        fprintf(gp, "set xrange [0:2000]\n");
        fprintf(gp, "plot ");

        for (int si = 0; si < 3; si++) {
            int sl     = seg_lens[si];
            int nf     = next_power_of_2(sl);
            int nb     = nf / 2 + 1;
            int ov     = sl / 2;

            double *psd    = (double *)calloc((size_t)nb, sizeof(double));
            double *psd_db = (double *)malloc((size_t)nb * sizeof(double));
            double *freq   = (double *)malloc((size_t)nb * sizeof(double));

            int ns = welch_psd(x, N, psd, nf, sl, ov, hann_window);
            psd_to_db(psd, psd_db, nb, -120.0);
            psd_freq_axis(freq, nb, fs);

            printf("  seg=%d: %d segments, %d bins\n", sl, ns, nb);

            if (si > 0) fprintf(gp, ", ");
            fprintf(gp, "'-' using 1:2 with lines lw 2 title '%s'", labels[si]);

            free(psd); free(psd_db); free(freq);
        }
        fprintf(gp, "\n");

        /* Now send the data */
        for (int si = 0; si < 3; si++) {
            int sl = seg_lens[si];
            int nf = next_power_of_2(sl);
            int nb = nf / 2 + 1;
            int ov = sl / 2;

            double *psd    = (double *)calloc((size_t)nb, sizeof(double));
            double *psd_db = (double *)malloc((size_t)nb * sizeof(double));
            double *freq   = (double *)malloc((size_t)nb * sizeof(double));

            welch_psd(x, N, psd, nf, sl, ov, hann_window);
            psd_to_db(psd, psd_db, nb, -120.0);
            psd_freq_axis(freq, nb, fs);

            for (int k = 0; k < nb; k++)
                fprintf(gp, "%.2f %.4f\n", freq[k], psd_db[k]);
            fprintf(gp, "e\n");

            free(psd); free(psd_db); free(freq);
        }
        gp_close(gp);
        printf("  → plots/ch14/welch_resolution.png\n");
    }

    free(x); free(noise);
}

/* ------------------------------------------------------------------ */
/*  Demo 5: Cross-PSD of correlated signals                           */
/* ------------------------------------------------------------------ */
static void demo_cross_psd(void)
{
    printf("\n=== Demo 5: Cross-PSD of Correlated Signals ===\n");

    const int    N  = 4096;
    const double fs = 8000.0;

    double *x = (double *)malloc((size_t)N * sizeof(double));
    double *y = (double *)malloc((size_t)N * sizeof(double));
    double *nx_noise = (double *)malloc((size_t)N * sizeof(double));
    double *ny_noise = (double *)malloc((size_t)N * sizeof(double));

    /* Common signal at 1 kHz */
    gen_sine(x, N, 1.0, 1000.0, fs, 0.0);
    gen_sine(y, N, 1.0, 1000.0, fs, 0.0);

    /* Add independent noise */
    gen_gaussian_noise(nx_noise, N, 0.0, 3.0, 10);
    gen_gaussian_noise(ny_noise, N, 0.0, 3.0, 20);
    signal_add(x, nx_noise, N);
    signal_add(y, ny_noise, N);

    int seg_len = 512;
    int nfft    = 512;
    int overlap = 256;
    int n_bins  = nfft / 2 + 1;

    Complex *cpsd = (Complex *)calloc((size_t)n_bins, sizeof(Complex));
    double  *mag  = (double *)malloc((size_t)n_bins * sizeof(double));
    double  *mag_db = (double *)malloc((size_t)n_bins * sizeof(double));
    double  *freq = (double *)malloc((size_t)n_bins * sizeof(double));

    int ns = cross_psd(x, y, N, cpsd, nfft, seg_len, overlap, hann_window);
    printf("  Cross-PSD: %d segments\n", ns);

    /* Magnitude of cross-PSD */
    for (int k = 0; k < n_bins; k++)
        mag[k] = sqrt(cpsd[k].re * cpsd[k].re + cpsd[k].im * cpsd[k].im);
    psd_to_db(mag, mag_db, n_bins, -120.0);
    psd_freq_axis(freq, n_bins, fs);

    printf("  Cross-PSD reveals common 1 kHz component,\n");
    printf("  independent noise averages out.\n");

    gp_plot_spectrum("ch14", "cross_psd",
                     "Cross-PSD — Reveals Common Signal (1 kHz)",
                     freq, mag_db, n_bins);
    printf("  → plots/ch14/cross_psd.png\n");

    free(x); free(y); free(nx_noise); free(ny_noise);
    free(cpsd); free(mag); free(mag_db); free(freq);
}

/* ================================================================== */

int main(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  Chapter 14: Power Spectral Density & Welch's Method   ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    gp_init("ch14");

    demo_periodogram_clean();
    demo_periodogram_noisy();
    demo_welch();
    demo_welch_resolution();
    demo_cross_psd();

    printf("\n=== Chapter 14 Complete ===\n");
    return 0;
}
