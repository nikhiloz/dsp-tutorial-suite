/**
 * @file fft_demo.c
 * @brief Interactive FFT demonstration — generates a signal and shows its spectrum.
 *
 * This example corresponds to tutorial/02-fft-fundamentals.md.
 *
 * What it does:
 *   1. Creates a signal with two sine waves (440 Hz + 1000 Hz)
 *   2. Applies a Hann window to reduce spectral leakage
 *   3. Computes the FFT
 *   4. Prints the magnitude spectrum
 *
 * Build & run:
 *   make && ./build/bin/fft_demo
 */

#include <stdio.h>
#include <math.h>
#include "fft.h"
#include "dsp_utils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define N        256       /* FFT size (power of 2) */
#define FS       8000.0    /* Sample rate in Hz     */
#define FREQ_1   440.0     /* First tone: A4 note   */
#define FREQ_2   1000.0    /* Second tone: 1 kHz    */

int main(void) {
    double signal[N];
    Complex spectrum[N];
    double mag[N];

    printf("=== FFT Demo: %d-point FFT ===\n", N);
    printf("Sample rate: %.0f Hz\n", FS);
    printf("Signal: %.0f Hz + %.0f Hz sine waves\n\n", FREQ_1, FREQ_2);

    /* ── Step 1: Generate a test signal ──────────────────────────── */
    /*
     * Two sine waves added together.
     * In the frequency domain, we expect peaks at 440 Hz and 1000 Hz.
     */
    for (int i = 0; i < N; i++) {
        double t = (double)i / FS;
        signal[i] = 1.0 * sin(2.0 * M_PI * FREQ_1 * t)
                  + 0.5 * sin(2.0 * M_PI * FREQ_2 * t);
    }

    /* ── Step 2: Apply Hann window ───────────────────────────────── */
    /*
     * Without windowing, the FFT of a finite signal chunk has
     * "spectral leakage" — energy smears across all bins.
     * The Hann window tapers edges to zero, reducing this.
     * See tutorial/03-window-functions.md for details.
     */
    apply_window(signal, N, hann_window);

    /* ── Step 3: Compute FFT ─────────────────────────────────────── */
    fft_real(signal, spectrum, N);

    /* ── Step 4: Extract magnitude spectrum ──────────────────────── */
    fft_magnitude(spectrum, mag, N);

    /* ── Step 5: Display results ─────────────────────────────────── */
    printf("Frequency (Hz)  |  Magnitude (dB)\n");
    printf("─────────────────────────────────\n");

    /* Only show first N/2 bins (Nyquist symmetry) */
    for (int k = 0; k < N / 2; k++) {
        double freq = (double)k * FS / N;
        double db = db_from_magnitude(mag[k] / (N / 2));

        /* Only print bins with significant energy */
        if (db > -40.0) {
            printf("%8.1f Hz     |  %+6.1f dB", freq, db);
            if (fabs(freq - FREQ_1) < FS / N || fabs(freq - FREQ_2) < FS / N) {
                printf("  ◄── peak!");
            }
            printf("\n");
        }
    }

    printf("\nExpected peaks at: %.0f Hz and %.0f Hz\n", FREQ_1, FREQ_2);
    printf("Frequency resolution: %.1f Hz per bin\n", FS / N);

    return 0;
}
