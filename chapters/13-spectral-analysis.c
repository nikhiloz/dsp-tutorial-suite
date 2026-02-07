/**
 * @file 13-spectral-analysis.c
 * @brief Chapter 5 demo — Full spectral analysis pipeline.
 *
 * Demonstrates:
 *   - Signal → window → FFT → magnitude → dB → display
 *   - Frequency resolution and bin interpretation
 *   - Effect of FFT size on spectral detail
 *   - Comparing windowed vs unwindowed spectra
 *
 * Build & run:
 *   make chapters && ./build/bin/ch05
 *
 * Read alongside: chapters/13-spectral-analysis.md
 *
 * ════════════════════════════════════════════════════════════════════
 *  THEORY: The Spectral Analysis Pipeline
 * ════════════════════════════════════════════════════════════════════
 *
 *  Spectral analysis converts a time-domain signal into a frequency-
 *  domain representation so we can identify the component tones,
 *  measure their power, and detect noise.
 *
 *  ┌───────────────────────────────────────────────────────────────┐
 *  │  Complete Spectral Analysis Pipeline                         │
 *  │                                                               │
 *  │                     ┌──────────┐                              │
 *  │   x[n]  ────────►   │  Window  │                              │
 *  │   (N samples)       │  w[n]    │                              │
 *  │                     └────┬─────┘                              │
 *  │                          │  x[n]·w[n]                        │
 *  │                          ▼                                    │
 *  │                     ┌──────────┐                              │
 *  │                     │   FFT    │                              │
 *  │                     │ (N-point)│                              │
 *  │                     └────┬─────┘                              │
 *  │                          │  X[k]  (complex)                  │
 *  │                          ▼                                    │
 *  │                     ┌──────────┐                              │
 *  │                     │ |X[k]|²  │   magnitude squared         │
 *  │                     │ = Re²+Im²│   (power spectrum)          │
 *  │                     └────┬─────┘                              │
 *  │                          │  P[k]                             │
 *  │                          ▼                                    │
 *  │                     ┌──────────┐                              │
 *  │                     │ 20·log₁₀ │   convert to decibels       │
 *  │                     │  (dB)    │                              │
 *  │                     └────┬─────┘                              │
 *  │                          │  dB[k]                            │
 *  │                          ▼                                    │
 *  │                     ┌──────────┐                              │
 *  │                     │ Display  │   thresholded table /       │
 *  │                     │ / plot   │   waterfall plot             │
 *  │                     └──────────┘                              │
 *  └───────────────────────────────────────────────────────────────┘
 *
 *  Key concepts:
 *
 *  Frequency resolution (bin width):
 *      Δf = fs / N
 *      Larger N  →  finer resolution  →  but needs more samples.
 *
 *  Resolution bandwidth (RBW):
 *      RBW ≈ ENBW × Δf
 *      where ENBW is the "equivalent noise bandwidth" of the window.
 *      Rectangular: ENBW = 1.0 bin
 *      Hann:        ENBW = 1.5 bins
 *      Hamming:     ENBW = 1.36 bins
 *      Blackman:    ENBW = 1.73 bins
 *
 *  Windowing effect:
 *      Without a window (= rectangular), side lobes are only −13 dB
 *      below the main lobe.  A strong tone can mask a weak nearby
 *      tone via spectral leakage.  Applying a Hann or Hamming
 *      window pushes side lobes to −31 / −42 dB, respectively.
 *      The trade-off: the main lobe widens from 2 bins to ~4 bins.
 *
 *  Interpreting the output:
 *      • Peaks above the noise floor correspond to signal tones.
 *      • Bin k represents frequency f_k = k · fs / N.
 *      • Magnitude is normalised by N/2 so that a unit-amplitude
 *        sine reads 0 dB at its peak bin.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "fft.h"
#include "dsp_utils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FS  8000.0

/**
 * @brief  Run the full spectral analysis pipeline on a signal.
 *
 * @param title  Descriptive label for this analysis run.
 * @param raw    Pointer to the raw time-domain signal (at least n samples).
 * @param n      FFT length (must be a power of 2; max 1024).
 * @param win    Window function to apply, or NULL for rectangular.
 *
 * Pipeline steps:
 *   1. Copy raw signal into local buffer (non-destructive).
 *   2. Apply window function (if provided).
 *   3. Compute N-point real FFT → complex spectrum.
 *   4. Compute magnitude of each bin.
 *   5. Convert to dB (normalised so full-scale sine = 0 dB).
 *   6. Print all bins above −40 dB threshold with peak markers.
 *
 * The frequency resolution printed is Δf = fs/N.  Doubling N halves
 * Δf but requires twice as many samples and ~2× computation.
 */
static void run_analysis(const char *title, const double *raw, int n, window_fn win) {
    double signal[1024];
    Complex spectrum[1024];
    double mag[1024];

    memcpy(signal, raw, (size_t)n * sizeof(double));
    if (win) apply_window(signal, n, win);
    fft_real(signal, spectrum, n);
    fft_magnitude(spectrum, mag, n);

    printf("  %s (N=%d, resolution=%.1f Hz/bin):\n", title, n, FS / n);
    printf("  Frequency   | dB\n");
    printf("  ────────────┼──────\n");
    int printed = 0;
    for (int k = 0; k < n / 2; k++) {
        double freq = (double)k * FS / n;
        double db = db_from_magnitude(mag[k] / (n / 2));
        if (db > -40.0) {
            printf("  %7.1f Hz  | %+6.1f dB", freq, db);
            if (db > -15.0) printf("  ◄── peak");
            printf("\n");
            printed++;
        }
    }
    if (printed == 0) printf("  (no bins above -40 dB threshold)\n");
    printf("\n");
}

int main(void) {
    printf("=== Chapter 5: Spectral Analysis ===\n\n");

    /*
     * ── Generate test signal: 440 Hz + 1000 Hz + 2500 Hz ────────
     *
     * We build a composite signal with three sinusoidal components
     * at different amplitudes (1.0, 0.5, 0.3).  Two array lengths
     * are prepared so we can compare 256-point vs 512-point FFTs.
     *
     * All three frequencies are below Nyquist (fs/2 = 4000 Hz),
     * so there is no aliasing.
     */
    double signal_256[256], signal_512[512];

    for (int i = 0; i < 512; i++) {
        double t = (double)i / FS;
        double s = 1.0 * sin(2.0 * M_PI * 440.0 * t)
                 + 0.5 * sin(2.0 * M_PI * 1000.0 * t)
                 + 0.3 * sin(2.0 * M_PI * 2500.0 * t);
        if (i < 256) signal_256[i] = s;
        signal_512[i] = s;
    }

    /*
     * ── Part 1: No window vs Hann window ────────────────────────
     *
     * Theory: With no window (rectangular), the 440 Hz component
     * at bin 14.08 leaks across many bins, raising the spectral
     * floor.  The Hann window suppresses side lobes by ~31 dB,
     * yielding a much cleaner spectrum.
     *
     *   Rectangular:                Hann window:
     *     dB                          dB
     *   0 ┤  █   █     █            0 ┤  █   █     █
     * -10 ┤ ███ ███                -10 ┤  █   █
     * -20 ┤█████████    █          -20 ┤  █   █     █
     * -30 ┤████████████████        -30 ┤ ███ ███   ███
     * -40 ┤████████████████████    -40 ┤  █   █     █
     *     └──────────────── f          └──────────────── f
     *     (lots of leakage)           (clean peaks)
     */
    printf("── Part 1: Windowing effect (N=256) ──\n\n");
    run_analysis("No window (rectangular)", signal_256, 256, NULL);
    run_analysis("Hann window", signal_256, 256, hann_window);

    printf("  With the Hann window, peaks are narrower and side lobes\n");
    printf("  are suppressed — cleaner spectrum at the cost of slightly\n");
    printf("  wider main lobes.\n\n");

    /*
     * ── Part 2: FFT size comparison ─────────────────────────────
     *
     * Theory: Frequency resolution Δf = fs / N.
     *   N=256  → Δf = 8000/256  = 31.25 Hz/bin
     *   N=512  → Δf = 8000/512  = 15.625 Hz/bin
     *
     * With finer resolution, peaks appear narrower (fewer bins
     * wide) and nearby tones are easier to distinguish.  However,
     * we need more samples (longer observation time = N/fs) and
     * the FFT takes O(N log N) operations → roughly 2× slower.
     */
    printf("── Part 2: N=256 vs N=512 (Hann window) ──\n\n");
    run_analysis("N=256 (31.25 Hz/bin)", signal_256, 256, hann_window);
    run_analysis("N=512 (15.63 Hz/bin)", signal_512, 512, hann_window);

    printf("  Doubling N halves the bin width → peaks are better resolved.\n");
    printf("  Trade-off: need 2x more samples and 2x more computation.\n\n");

    /*
     * ── Part 3: RMS comparison ──────────────────────────────────
     *
     * Theory: The RMS (root-mean-square) of a signal is:
     *   RMS = sqrt( (1/N) Σ x[n]² )
     *
     * By Parseval's theorem, the total energy in time equals the
     * total energy in frequency.  RMS should be the same regardless
     * of whether we look at 256 or 512 samples of the same signal
     * (assuming no transient effects).
     */
    printf("── Part 3: Signal statistics ──\n\n");
    printf("  RMS (256 samples): %.4f\n", rms(signal_256, 256));
    printf("  RMS (512 samples): %.4f\n", rms(signal_512, 512));
    printf("  Nyquist frequency: %.0f Hz (fs/2)\n", FS / 2.0);
    printf("  All 3 frequencies (440, 1000, 2500 Hz) are below Nyquist → no aliasing.\n");

    return 0;
}
