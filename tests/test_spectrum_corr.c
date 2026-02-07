/**
 * @file test_spectrum_corr.c
 * @brief Tests for spectrum.h (PSD/Welch) and correlation.h modules.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "fft.h"
#include "dsp_utils.h"
#include "signal_gen.h"
#include "spectrum.h"
#include "correlation.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ------------------------------------------------------------------ */
/*  Minimal test framework (same as other test files)                 */
/* ------------------------------------------------------------------ */
static int tests_run    = 0;
static int tests_passed = 0;

#define RUN_TEST(name)                                             \
    do {                                                           \
        printf("  [TEST] %-50s ", #name);                          \
        tests_run++;                                               \
        if (test_##name()) { printf("PASS\n"); tests_passed++; }   \
        else               { printf("FAIL\n"); }                   \
    } while (0)

/* ------------------------------------------------------------------ */
/*  Spectrum tests                                                    */
/* ------------------------------------------------------------------ */

/** Periodogram of a pure cosine → peak at correct bin */
static int test_periodogram_peak(void)
{
    const int N = 256;
    const double fs = 1000.0;
    const double f0 = 250.0;   /* exactly on bin: bin = f0/fs * N = 64 */
    double x[256];
    gen_cosine(x, N, 1.0, f0, fs, 0.0);

    int nfft = 256;
    int nb   = nfft / 2 + 1;
    double psd[129], freq[129];
    periodogram(x, N, psd, nfft);
    psd_freq_axis(freq, nb, fs);

    /* Peak should be at bin 64 = 250 Hz */
    int peak_bin = 0;
    double peak_val = psd[0];
    for (int k = 1; k < nb; k++) {
        if (psd[k] > peak_val) { peak_val = psd[k]; peak_bin = k; }
    }
    return (peak_bin == 64);
}

/** Welch returns positive number of segments */
static int test_welch_segments(void)
{
    const int N = 4096;
    double *x = (double *)malloc((size_t)N * sizeof(double));
    gen_white_noise(x, N, 1.0, 42);

    int nfft = 512, seg = 512, ov = 256;
    int nb = nfft / 2 + 1;
    double *psd = (double *)calloc((size_t)nb, sizeof(double));

    int ns = welch_psd(x, N, psd, nfft, seg, ov, hann_window);
    free(x); free(psd);

    /* Expected: (4096 - 512) / 256 + 1 = 15 segments */
    return (ns == 15);
}

/** Welch PSD of white noise is approximately flat */
static int test_welch_white_noise_flat(void)
{
    const int N = 16384;
    double *x = (double *)malloc((size_t)N * sizeof(double));
    gen_gaussian_noise(x, N, 0.0, 1.0, 99);

    int nfft = 512, seg = 512, ov = 256;
    int nb = nfft / 2 + 1;
    double *psd = (double *)calloc((size_t)nb, sizeof(double));

    welch_psd(x, N, psd, nfft, seg, ov, hann_window);

    /* Skip DC and Nyquist; check that max/min ratio < 10 (within 10 dB) */
    double pmin = psd[1], pmax = psd[1];
    for (int k = 2; k < nb - 1; k++) {
        if (psd[k] < pmin) pmin = psd[k];
        if (psd[k] > pmax) pmax = psd[k];
    }
    free(x); free(psd);

    double ratio = pmax / (pmin + 1e-30);
    return (ratio < 10.0);
}

/** psd_to_db converts correctly */
static int test_psd_to_db(void)
{
    double psd[3]    = {1.0, 0.01, 100.0};
    double psd_db[3];
    psd_to_db(psd, psd_db, 3, -120.0);

    /* 10*log10(1) = 0, 10*log10(0.01) = -20, 10*log10(100) = 20 */
    return (fabs(psd_db[0] - 0.0)  < 0.01 &&
            fabs(psd_db[1] + 20.0) < 0.01 &&
            fabs(psd_db[2] - 20.0) < 0.01);
}

/** psd_freq_axis produces correct values */
static int test_freq_axis(void)
{
    double freq[5];
    psd_freq_axis(freq, 5, 1000.0);
    /* nfft = (5-1)*2 = 8; freq[k] = k * 1000 / 8 = {0, 125, 250, 375, 500} */
    return (fabs(freq[0] - 0.0) < 0.01 &&
            fabs(freq[2] - 250.0) < 0.01 &&
            fabs(freq[4] - 500.0) < 0.01);
}

/** Cross-PSD detects shared tone */
static int test_cross_psd_shared_tone(void)
{
    const int N = 4096;
    const double fs = 8000.0;
    double *x = (double *)malloc((size_t)N * sizeof(double));
    double *y = (double *)malloc((size_t)N * sizeof(double));

    gen_sine(x, N, 1.0, 1000.0, fs, 0.0);
    gen_sine(y, N, 1.0, 1000.0, fs, 0.0);

    /* Add independent noise */
    double *nx_noise = (double *)malloc((size_t)N * sizeof(double));
    double *ny_noise = (double *)malloc((size_t)N * sizeof(double));
    gen_gaussian_noise(nx_noise, N, 0.0, 5.0, 10);
    gen_gaussian_noise(ny_noise, N, 0.0, 5.0, 20);
    signal_add(x, nx_noise, N);
    signal_add(y, ny_noise, N);

    int nfft = 512, seg = 512, ov = 256;
    int nb = nfft / 2 + 1;
    Complex *cpsd = (Complex *)calloc((size_t)nb, sizeof(Complex));

    cross_psd(x, y, N, cpsd, nfft, seg, ov, hann_window);

    /* Find peak magnitude bin */
    double *freq = (double *)malloc((size_t)nb * sizeof(double));
    psd_freq_axis(freq, nb, fs);

    int peak_bin = 0;
    double peak_mag = 0;
    for (int k = 0; k < nb; k++) {
        double m = cpsd[k].re * cpsd[k].re + cpsd[k].im * cpsd[k].im;
        if (m > peak_mag) { peak_mag = m; peak_bin = k; }
    }

    /* Peak should be near 1000 Hz: bin = 1000/fs * nfft = 64 */
    int ok = (abs(peak_bin - 64) <= 2);

    free(x); free(y); free(nx_noise); free(ny_noise);
    free(cpsd); free(freq);
    return ok;
}

/* ------------------------------------------------------------------ */
/*  Correlation tests                                                 */
/* ------------------------------------------------------------------ */

/** Autocorrelation at lag 0 = energy of signal */
static int test_autocorr_lag0(void)
{
    const int N = 128;
    double x[128];
    gen_sine(x, N, 1.0, 10.0, 100.0, 0.0);

    int r_len = 2 * N - 1;
    double *r = (double *)malloc((size_t)r_len * sizeof(double));
    autocorr(x, N, r);

    /* r[N-1] should equal sum(x[i]^2) */
    double energy = 0;
    for (int i = 0; i < N; i++) energy += x[i] * x[i];

    int ok = (fabs(r[N - 1] - energy) < energy * 0.01);
    free(r);
    return ok;
}

/** Normalised autocorrelation at lag 0 = 1.0 */
static int test_autocorr_norm_unity(void)
{
    const int N = 256;
    double x[256];
    gen_cosine(x, N, 2.5, 50.0, 1000.0, 0.0);

    int r_len = 2 * N - 1;
    double *r = (double *)malloc((size_t)r_len * sizeof(double));
    autocorr_normalized(x, N, r);

    int ok = (fabs(r[N - 1] - 1.0) < 1e-10);
    free(r);
    return ok;
}

/** Cross-correlation detects known delay */
static int test_xcorr_delay(void)
{
    const int N = 512;
    const int delay = 30;
    double x[512], y[512];

    gen_chirp(x, N, 1.0, 10.0, 200.0, 1000.0);
    memset(y, 0, sizeof(y));
    for (int i = delay; i < N; i++)
        y[i] = x[i - delay];

    int r_len = 2 * N - 1;
    double *r = (double *)malloc((size_t)r_len * sizeof(double));
    xcorr(x, N, y, N, r);

    int peak = xcorr_peak_lag(r, r_len, N - 1);
    free(r);
    return (peak == delay);
}

/** Normalised xcorr of identical signals = 1.0 at lag 0 */
static int test_xcorr_norm_identical(void)
{
    const int N = 128;
    double x[128];
    gen_sine(x, N, 1.0, 25.0, 500.0, 0.0);

    int r_len = 2 * N - 1;
    double *r = (double *)malloc((size_t)r_len * sizeof(double));
    xcorr_normalized(x, N, x, N, r);

    /* Peak at lag 0 (centre = N-1) should be 1.0 */
    int ok = (fabs(r[N - 1] - 1.0) < 1e-10);
    free(r);
    return ok;
}

/** White noise autocorrelation: lag-0 >> lag-k for k > 0 */
static int test_noise_autocorr_delta(void)
{
    const int N = 4096;
    double *x = (double *)malloc((size_t)N * sizeof(double));
    gen_gaussian_noise(x, N, 0.0, 1.0, 77);

    int r_len = 2 * N - 1;
    double *r = (double *)malloc((size_t)r_len * sizeof(double));
    autocorr_normalized(x, N, r);

    /* All lags > 5 should be small (< 0.1) */
    int ok = 1;
    for (int lag = 5; lag < 100; lag++) {
        if (fabs(r[N - 1 + lag]) > 0.1) { ok = 0; break; }
    }
    free(x); free(r);
    return ok;
}

/** xcorr_peak_lag returns correct value */
static int test_peak_lag_api(void)
{
    double r[5] = {0.1, 0.3, 0.9, 0.5, 0.2};
    int lag = xcorr_peak_lag(r, 5, 2);   /* centre at index 2 */
    return (lag == 0);   /* peak is at index 2 = lag 0 */
}

/* __________________________________________________________________ */

int main(void)
{
    printf("\n=== Test Suite: Spectrum & Correlation ===\n");

    /* Spectrum tests */
    RUN_TEST(periodogram_peak);
    RUN_TEST(welch_segments);
    RUN_TEST(welch_white_noise_flat);
    RUN_TEST(psd_to_db);
    RUN_TEST(freq_axis);
    RUN_TEST(cross_psd_shared_tone);

    /* Correlation tests */
    RUN_TEST(autocorr_lag0);
    RUN_TEST(autocorr_norm_unity);
    RUN_TEST(xcorr_delay);
    RUN_TEST(xcorr_norm_identical);
    RUN_TEST(noise_autocorr_delta);
    RUN_TEST(peak_lag_api);

    printf("\n=== Test Summary ===\n");
    printf("Total: %d, Passed: %d, Failed: %d\n",
           tests_run, tests_passed, tests_run - tests_passed);
    printf("Pass Rate: %.1f%%\n",
           100.0 * (double)tests_passed / (double)tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
