/**
 * @file 10-digital-filters.c
 * @brief Chapter 4 demo — FIR filter design and noise reduction.
 *
 * Demonstrates:
 *   - Moving average filter on a step signal
 *   - Windowed-sinc lowpass filter design
 *   - Filtering a noisy signal and measuring SNR improvement
 *   - Filter coefficient inspection
 *
 * Build & run:
 *   make chapters && ./build/bin/ch04
 *
 * Read alongside: chapters/10-digital-filters.md
 *
 * ════════════════════════════════════════════════════════════════════
 *  THEORY: FIR (Finite Impulse Response) Filters
 * ════════════════════════════════════════════════════════════════════
 *
 *  An FIR filter computes each output sample y[n] as a weighted sum
 *  of the current and past M-1 input samples:
 *
 *      y[n] = h[0]·x[n] + h[1]·x[n-1] + … + h[M-1]·x[n-M+1]
 *
 *  where h[0..M-1] are the filter "taps" (coefficients) and M is
 *  the filter order + 1.
 *
 *  ┌───────────────────────────────────────────────────────────────┐
 *  │  FIR Filter Block Diagram  (M taps, delay line structure)    │
 *  │                                                               │
 *  │  x[n] ──┬──────►[×h[0]]──┐                                   │
 *  │         │                 │                                   │
 *  │        [z⁻¹]              ▼                                   │
 *  │         │              [  +  ]──┐                              │
 *  │         ├──►[×h[1]]──►   ▲     │                              │
 *  │         │                │     │                              │
 *  │        [z⁻¹]             │     │                              │
 *  │         │                │     │                              │
 *  │         ├──►[×h[2]]──────┘     │                              │
 *  │         │                      ▼                              │
 *  │        [z⁻¹]                [  +  ]──┐                        │
 *  │         │                      ▲     │                        │
 *  │         ⋮                      │     │                        │
 *  │        [z⁻¹]                   │     │                        │
 *  │         │                      │     ▼                        │
 *  │         └──►[×h[M-1]]──────────┘   y[n]                      │
 *  │                                                               │
 *  │  Each [z⁻¹] is a one-sample delay (memory element).          │
 *  │  Each [×h[k]] multiplies by the k-th tap coefficient.        │
 *  │  All products are summed to produce y[n].                    │
 *  └───────────────────────────────────────────────────────────────┘
 *
 *  Windowed-sinc design method:
 *  ─────────────────────────────
 *  An ideal lowpass filter has the impulse response:
 *
 *      h_ideal[n] = sin(2π f_c n) / (π n)     (a sinc function)
 *
 *  This is infinite in length, so we truncate it to M samples and
 *  apply a window (Hamming, Blackman, etc.) to control the side
 *  lobes in the resulting frequency response.  A wider window /
 *  more taps → sharper transition band but more computation.
 *
 *  Frequency-domain interpretation:
 *  ──────────────────────────────────
 *  The FIR frequency response is:
 *
 *      H(e^{jω}) = Σ h[k] · e^{-jωk}       (the DTFT of h)
 *                  k=0..M-1
 *
 *  For a symmetric FIR (h[k] = h[M-1-k]), the phase is exactly
 *  linear: φ(ω) = −ω(M-1)/2.  Linear phase means no waveform
 *  distortion — all frequencies are delayed equally.
 */

#include <stdio.h>
#include <math.h>
#include "filter.h"
#include "dsp_utils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define N      256
#define FS     8000.0

int main(void) {
    printf("=== Chapter 4: Digital Filters ===\n\n");

    /*
     * ── Part 1: Moving average on a step signal ─────────────────
     *
     * Theory: The moving average is the simplest FIR filter:
     *   h[k] = 1/M  for k = 0 … M-1.
     *
     * It averages the last M samples, which smooths out rapid
     * changes.  Applied to a step function (0→1 at n=4), the
     * output ramps linearly over M samples before settling to 1.
     *
     *   Step input:               Filtered output:
     *   │         ████████        │           ╱────────
     *   │         █               │         ╱
     *   │         █               │       ╱
     *   └──────────────── n       └──────────────── n
     *         n=4                      settling = M samples
     *
     * In the frequency domain, the moving average has a sinc-shaped
     * magnitude response — it passes low frequencies and attenuates
     * high ones, but with poor stop-band rejection (−13 dB/lobe).
     */
    printf("── Part 1: 5-tap moving average on a step signal ──\n\n");

    double step_in[16], step_out[16];
    double ma_h[5];
    fir_moving_average(ma_h, 5);

    for (int i = 0; i < 16; i++) step_in[i] = (i >= 4) ? 1.0 : 0.0;
    fir_filter(step_in, step_out, 16, ma_h, 5);

    printf("  n  | input | output (5-pt avg) | note\n");
    printf("  ───┼───────┼───────────────────┼──────\n");
    for (int i = 0; i < 16; i++) {
        printf("  %2d |  %.1f  |      %.3f        ", i, step_in[i], step_out[i]);
        if (i >= 4 && i < 9)
            printf("| ← ramp (settling)");
        else if (i >= 9)
            printf("| ← settled to 1.0");
        printf("\n");
    }
    printf("\n  The moving average smoothly ramps from 0 to 1.\n");
    printf("  Settling time = %d samples (= filter order).\n\n", 5);

    /*
     * ── Part 2: Lowpass filter coefficients ──────────────────────
     *
     * Theory: We design a lowpass filter using the windowed-sinc
     * method.  The ideal lowpass impulse response is:
     *
     *   h_ideal[n] = 2 f_c · sinc(2 f_c (n − M/2))
     *
     * where f_c = cutoff / fs is the normalised cutoff frequency.
     * Truncating to TAPS samples and applying a window produces a
     * practical filter.  Key properties to verify:
     *
     *   • Symmetry:  h[k] = h[M-1-k]  → linear phase guaranteed
     *   • DC gain:   Σ h[k] = 1.0     → unit gain at 0 Hz
     *   • Centre tap: h[M/2]          → largest coefficient
     */
    printf("── Part 2: 31-tap lowpass filter coefficients ──\n\n");

    #define TAPS 31
    double h[TAPS];
    double cutoff = 500.0 / FS;  /* 0.0625 normalized */
    fir_lowpass(h, TAPS, cutoff);

    printf("  Cutoff: 500 Hz (normalized: %.4f)\n", cutoff);
    printf("  Taps: %d (centre at tap %d)\n\n", TAPS, TAPS / 2);

    /* Print symmetry */
    printf("  Symmetry check (linear-phase FIR):\n");
    int symmetric = 1;
    for (int i = 0; i < TAPS / 2; i++) {
        if (fabs(h[i] - h[TAPS - 1 - i]) > 1e-12) symmetric = 0;
    }
    printf("  h[i] == h[M-1-i]?  %s\n\n", symmetric ? "YES ✓" : "NO ✗");

    double coeff_sum = 0;
    printf("  Coefficients (all %d):\n", TAPS);
    for (int i = 0; i < TAPS; i++) {
        printf("    h[%2d] = %+.6f", i, h[i]);
        if (i == TAPS / 2) printf("  ← centre (largest)");
        printf("\n");
        coeff_sum += h[i];
    }
    printf("  Sum of coefficients: %.6f  (should be 1.0 for unity DC gain)\n\n", coeff_sum);

    /*
     * ── Part 3: Noise reduction ─────────────────────────────────
     *
     * Theory: The classic use case for a lowpass filter is removing
     * high-frequency noise from a signal of interest.
     *
     *   Spectrum before filtering:
     *   │  ██                                   (200 Hz signal)
     *   │                     ██  ██             (2800 & 3500 Hz noise)
     *   │─────────────────────────────── f (Hz)
     *   0        500  1000      2800 3500  4000
     *                  ▲
     *              cutoff = 500 Hz
     *
     *   Spectrum after filtering:
     *   │  ██
     *   │                     ..  ..  (attenuated by >40 dB)
     *   │─────────────────────────────── f (Hz)
     *   0        500  1000      2800 3500  4000
     *
     * We measure the improvement by comparing the RMS of the
     * filtered signal against the clean original.
     * Note: the first TAPS-1 output samples are in the "settling"
     * transient and are excluded from the RMS calculation.
     */
    printf("── Part 3: Lowpass filtering a noisy signal ──\n\n");

    double clean[N], noisy[N], filtered[N];

    for (int i = 0; i < N; i++) {
        double t = (double)i / FS;
        clean[i] = sin(2.0 * M_PI * 200.0 * t);
        noisy[i] = clean[i]
                  + 0.3 * sin(2.0 * M_PI * 2800.0 * t)
                  + 0.2 * sin(2.0 * M_PI * 3500.0 * t);
    }

    fir_filter(noisy, filtered, N, h, TAPS);

    double rms_clean    = rms(clean, N);
    double rms_noisy    = rms(noisy, N);
    double rms_filt     = rms(filtered + TAPS, N - TAPS);

    double error_sum = 0;
    for (int i = TAPS; i < N; i++) {
        double e = filtered[i] - clean[i];
        error_sum += e * e;
    }
    double rms_error = sqrt(error_sum / (N - TAPS));

    printf("  Signal: 200 Hz sine + noise at 2800 Hz and 3500 Hz\n");
    printf("  Filter: %d-tap lowpass at 500 Hz\n\n", TAPS);
    printf("  Clean RMS:     %.4f\n", rms_clean);
    printf("  Noisy RMS:     %.4f  (noise added %.1f%%)\n",
           rms_noisy, (rms_noisy - rms_clean) / rms_clean * 100);
    printf("  Filtered RMS:  %.4f  (after %d-sample settling)\n", rms_filt, TAPS);
    printf("  Error vs clean: %.4f RMS\n", rms_error);
    printf("\n  The filter removed the high-frequency noise while\n");
    printf("  preserving the 200 Hz signal.\n");

    return 0;
}
