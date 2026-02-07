/**
 * @file 09-window-functions.c
 * @brief Chapter 3 demo — Compare window functions and their effect on spectra.
 *
 * Demonstrates:
 *   - Hann, Hamming, and Blackman window shapes
 *   - FFT of a windowed vs unwindowed signal
 *   - Side-lobe levels for each window
 *
 * Build & run:
 *   make chapters && ./build/bin/ch03
 *
 * Read alongside: chapters/09-window-functions.md
 *
 * ════════════════════════════════════════════════════════════════════
 *  THEORY: Window Functions and Spectral Leakage
 * ════════════════════════════════════════════════════════════════════
 *
 *  When we compute the FFT of a finite-length signal, we implicitly
 *  multiply the infinite signal by a rectangular window:
 *
 *      x_windowed[n] = x[n] · w[n]
 *
 *  Multiplication in time  ↔  convolution in frequency.
 *  The rectangular window's spectrum has a narrow main lobe but
 *  LARGE side lobes (−13 dB), causing "spectral leakage" — energy
 *  from one frequency smearing into adjacent bins.
 *
 *  Tapered windows (Hann, Hamming, Blackman, …) trade a wider
 *  main lobe for much lower side lobes → cleaner spectra.
 *
 *  ┌───────────────────────────────────────────────────────────────┐
 *  │  ASCII Window Shape Comparison  (N = 16)                     │
 *  │                                                               │
 *  │  1.0 ┤ ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄   ← Rectangular (all 1.0)         │
 *  │      │                                                        │
 *  │  1.0 ┤         ██                                             │
 *  │  0.8 ┤       ██  ██          ← Hann  (cosine²)               │
 *  │  0.5 ┤     ██      ██                                        │
 *  │  0.2 ┤   ██          ██        w[n] = 0.5(1 - cos(2πn/N))   │
 *  │  0.0 ┤ ██              ██                                     │
 *  │      └────────────────────                                    │
 *  │                                                               │
 *  │  1.0 ┤         ██                                             │
 *  │  0.8 ┤       ██  ██          ← Hamming                       │
 *  │  0.5 ┤     ██      ██         w[n] = 0.54 - 0.46cos(2πn/N)  │
 *  │  0.2 ┤   ██          ██                                       │
 *  │  0.08┤ ██              ██    (never reaches zero!)            │
 *  │      └────────────────────                                    │
 *  │                                                               │
 *  │  1.0 ┤         ██                                             │
 *  │  0.8 ┤        ████           ← Blackman                      │
 *  │  0.5 ┤      ██    ██          w[n] = 0.42 - 0.5cos(2πn/N)   │
 *  │  0.2 ┤    ██        ██              + 0.08cos(4πn/N)         │
 *  │  0.0 ┤ ██            ██                                       │
 *  │      └────────────────────                                    │
 *  └───────────────────────────────────────────────────────────────┘
 *
 *  Summary table:
 *
 *      Window       Main lobe   First side   Typical use
 *                   width       lobe (dB)
 *      ──────────   ─────────   ──────────   ──────────────────────
 *      Rectangular  2 bins      −13 dB       Exact bin-centred tones
 *      Hann         4 bins      −31 dB       General-purpose analysis
 *      Hamming      4 bins      −42 dB       Speech / audio analysis
 *      Blackman     6 bins      −58 dB       High dynamic range work
 *
 *  "Main lobe width" determines the minimum frequency separation
 *  required to resolve two nearby tones.  Wider lobe = worse
 *  resolution but better side-lobe suppression.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "fft.h"
#include "dsp_utils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define N   256
#define FS  8000.0

/**
 * @brief  Analyse the spectral effect of a given window function.
 *
 * @param name         Human-readable name for the window (e.g. "Hann").
 * @param win          Window function pointer, or NULL for rectangular.
 * @param base_signal  Pointer to the original time-domain signal (N samples).
 *
 * Steps performed:
 *   1. Copy the base signal into a local buffer.
 *   2. Apply the window function (if non-NULL).
 *   3. Compute the FFT and convert to magnitude.
 *   4. Locate the peak bin and its magnitude.
 *   5. Scan for the highest side-lobe (at least 3 bins from peak).
 *   6. Print a summary line: peak location, peak dB, side-lobe dB,
 *      and the suppression (peak − side-lobe) in dB.
 *
 * The "3-bin guard" ensures we don't confuse main-lobe energy with
 * a side lobe.  For wider windows (e.g. Blackman), the main lobe
 * itself spans ~6 bins, so a wider guard might be more rigorous.
 */
static void analyze_window(const char *name, window_fn win, double *base_signal) {
    double signal[N];
    Complex spectrum[N];
    double mag[N];

    memcpy(signal, base_signal, N * sizeof(double));

    if (win) {
        apply_window(signal, N, win);
    }

    fft_real(signal, spectrum, N);
    fft_magnitude(spectrum, mag, N);

    /* Find peak bin */
    int peak_bin = 0;
    double peak_mag = 0;
    for (int k = 1; k < N / 2; k++) {
        if (mag[k] > peak_mag) {
            peak_mag = mag[k];
            peak_bin = k;
        }
    }

    double peak_db = db_from_magnitude(peak_mag);

    /* Find highest side-lobe (bins more than 3 away from peak) */
    double max_sidelobe = 0;
    for (int k = 1; k < N / 2; k++) {
        if (abs(k - peak_bin) > 3 && mag[k] > max_sidelobe) {
            max_sidelobe = mag[k];
        }
    }
    double sidelobe_db = db_from_magnitude(max_sidelobe);

    printf("  %-12s  peak at bin %2d (%6.1f Hz)  |  peak: %+6.1f dB  |  "
           "side-lobe: %+6.1f dB  |  suppression: %.0f dB\n",
           name, peak_bin, (double)peak_bin * FS / N,
           peak_db, sidelobe_db, peak_db - sidelobe_db);
}

int main(void) {
    printf("=== Chapter 3: Window Functions ===\n\n");

    /*
     * ── Part 1: Print window shapes ─────────────────────────────
     *
     * Theory: A window w[n] is a finite-length weighting function
     * applied sample-by-sample before the FFT.  It tapers the
     * signal edges toward zero, reducing the discontinuity that
     * causes spectral leakage.
     *
     * We print 16 values so the reader can see the taper:
     *   - Rectangular:  constant 1.0 (no taper)
     *   - Hann:         starts/ends at 0.0, peaks at centre
     *   - Hamming:      starts/ends at 0.08 (non-zero edges)
     *   - Blackman:     starts/ends at ~0.0, narrower shape
     */
    printf("── Window shapes (N=16, showing w[i]) ──\n\n");
    printf("  i   | Rectangular | Hann    | Hamming | Blackman\n");
    printf("  ────┼────────────┼─────────┼─────────┼─────────\n");
    for (int i = 0; i < 16; i++) {
        printf("  %2d  |   %5.3f    | %5.3f   | %5.3f   | %5.3f\n",
               i,
               1.0,
               hann_window(16, i),
               hamming_window(16, i),
               blackman_window(16, i));
    }

    /*
     * ── Part 2: Spectral leakage comparison ─────────────────────
     *
     * Theory: "Spectral leakage" occurs when the signal frequency
     * does NOT fall exactly on a DFT bin centre.  The rectangular
     * window's sinc-like spectrum smears energy across many bins.
     *
     * Here 440 Hz / (fs/N) = 440/31.25 = 14.08, which is NOT an
     * integer → leakage is guaranteed.  We compare how each window
     * controls those side lobes.
     *
     * Good side-lobe suppression means nearby weaker tones won't
     * be masked by leakage from a stronger tone.
     */
    printf("\n── Spectral leakage comparison ──\n");
    printf("  Signal: 440 Hz sine at fs=%g Hz, N=%d\n", FS, N);
    printf("  440/31.25 = 14.08 → falls BETWEEN bins → leakage expected\n\n");

    double signal[N];
    for (int i = 0; i < N; i++) {
        double t = (double)i / FS;
        signal[i] = sin(2.0 * M_PI * 440.0 * t);
    }

    analyze_window("Rectangular", NULL, signal);
    analyze_window("Hann", hann_window, signal);
    analyze_window("Hamming", hamming_window, signal);
    analyze_window("Blackman", blackman_window, signal);

    /*
     * ── Part 3: A bin-centred frequency (no leakage) ────────────
     *
     * Theory: When the frequency is an exact multiple of Δf = fs/N,
     * it lands squarely on one bin.  The window no longer matters
     * because the sinc side lobes are sampled at their zero
     * crossings.  This "control" experiment confirms that leakage
     * is purely an artefact of non-integer bin positions.
     */
    printf("\n── Control: bin-centred frequency (500 Hz = bin 16.0) ──\n");
    printf("  500/31.25 = 16.0 → falls EXACTLY on bin → no leakage\n\n");

    for (int i = 0; i < N; i++) {
        double t = (double)i / FS;
        signal[i] = sin(2.0 * M_PI * 500.0 * t);
    }

    analyze_window("Rectangular", NULL, signal);
    analyze_window("Hann", hann_window, signal);

    printf("\n  When the frequency falls exactly on a bin, all windows\n");
    printf("  give a clean peak. Windows only matter for non-integer bins.\n");

    return 0;
}
