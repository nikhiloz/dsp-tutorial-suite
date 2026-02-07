/**
 * @file 08-fft-fundamentals.c
 * @brief Chapter 2 demo — Step-by-step FFT on small signals.
 *
 * Demonstrates:
 *   - 8-point FFT on known signals (impulse, DC, sine)
 *   - Frequency bin interpretation
 *   - FFT → IFFT roundtrip
 *   - Dual-tone spectrum analysis (440 Hz + 1000 Hz)
 *
 * Build & run:
 *   make chapters && ./build/bin/ch02
 *
 * Read alongside: chapters/08-fft-fundamentals.md
 *
 * ════════════════════════════════════════════════════════════════════
 *  THEORY: The Fast Fourier Transform (FFT)
 * ════════════════════════════════════════════════════════════════════
 *
 *  The FFT is an efficient algorithm for computing the Discrete
 *  Fourier Transform (DFT).  The DFT of an N-point sequence x[n] is:
 *
 *       X[k] = Σ  x[n] · e^{-j 2π k n / N}      k = 0 … N-1
 *              n=0..N-1
 *
 *  A naïve evaluation requires O(N²) complex multiplications.
 *  The Cooley–Tukey radix-2 decimation-in-time (DIT) algorithm
 *  recursively splits the DFT into two N/2-point DFTs of the
 *  even- and odd-indexed samples, then combines them with
 *  "twiddle factors" W_N^k = e^{-j 2π k / N}.
 *
 *  Complexity comparison:
 *
 *      Algorithm        Multiplications     Example N=1024
 *      ───────────────  ──────────────────  ──────────────
 *      Direct DFT       O(N²)               1,048,576
 *      Radix-2 FFT      O(N log₂ N)            10,240
 *      Speed-up                                  ×102
 *
 *  ┌──────────────────────────────────────────────────────────────┐
 *  │  Radix-2 Butterfly Diagram  (8-point DIT FFT)               │
 *  │                                                              │
 *  │  Input        Stage 1         Stage 2         Stage 3        │
 *  │  (bit-rev)    (2-pt DFTs)     (4-pt DFTs)     (8-pt DFT)    │
 *  │                                                              │
 *  │  x[0] ──────●────────────●──────────────●──────── X[0]      │
 *  │              │  ╲  W⁰     │  ╲            │  ╲               │
 *  │  x[4] ──────●────────────│───│───────────│───│──── X[1]     │
 *  │                          │   │  W⁰       │   │              │
 *  │  x[2] ──────●────────────●───│───────────│───│──── X[2]     │
 *  │              │  ╲  W⁰       ╲│           │   │              │
 *  │  x[6] ──────●────────────────●───────────│───│──── X[3]     │
 *  │                                          │   │  W⁰          │
 *  │  x[1] ──────●────────────●──────────────●───│──── X[4]     │
 *  │              │  ╲  W⁰     │  ╲               │              │
 *  │  x[5] ──────●────────────│───│──────────────│──── X[5]     │
 *  │                          │   │  W⁰          │              │
 *  │  x[3] ──────●────────────●───│──────────────│──── X[6]     │
 *  │              │  ╲  W⁰       ╲│              │              │
 *  │  x[7] ──────●────────────────●──────────────●──── X[7]     │
 *  │                                                              │
 *  │  Each "●──●" pair is a BUTTERFLY operation:                  │
 *  │                                                              │
 *  │      a ──────●──────── a + W·b                               │
 *  │              │╲                                              │
 *  │              │ ╲  W^k                                        │
 *  │              │╱                                              │
 *  │      b ──────●──────── a - W·b                               │
 *  │                                                              │
 *  │  Total butterflies = N/2 × log₂(N) = 4 × 3 = 12            │
 *  └──────────────────────────────────────────────────────────────┘
 *
 *  Key properties of the DFT / FFT:
 *    • Linearity:    FFT{a·x + b·y} = a·FFT{x} + b·FFT{y}
 *    • Parseval:     Σ|x[n]|² = (1/N) Σ|X[k]|²   (energy preserved)
 *    • Symmetry:     For real x[n], X[k] = conj(X[N-k])
 *    • Bin spacing:  Δf = fs / N
 *    • Nyquist bin:  k = N/2  →  f = fs/2
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "fft.h"
#include "dsp_utils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Print the complex spectrum with magnitude for each bin.
 * @param title  Label printed above the table.
 * @param x      Array of N complex FFT output bins.
 * @param n      Length of the FFT (must equal array size).
 *
 * For a real input signal, bins 0..N/2 are unique; bins N/2+1..N-1
 * are the conjugate-symmetric mirror.  Bin k corresponds to frequency
 * f_k = k · fs / N.
 */
static void print_spectrum(const char *title, Complex *x, int n) {
    printf("  %s:\n", title);
    for (int k = 0; k < n; k++) {
        double mag = complex_mag(x[k]);
        printf("    bin[%d]  %+7.3f %+7.3fi   |X| = %.3f\n",
               k, x[k].re, x[k].im, mag);
    }
    printf("\n");
}

int main(void) {
    printf("=== Chapter 2: FFT Fundamentals ===\n\n");

    /*
     * ── Demo 1: Impulse → flat spectrum ───────────────────────────
     *
     * Theory: The impulse (delta) function δ[n] has value 1 at n=0
     * and 0 elsewhere.  Its DFT is X[k] = 1 for all k, meaning an
     * impulse contains ALL frequencies at equal amplitude.  This is
     * the spectral equivalent of "white" — perfectly flat.
     *
     *   Time domain:             Frequency domain:
     *   │                        │ ─ ─ ─ ─ ─ ─ ─
     *   │█                       │ 1 1 1 1 1 1 1 1
     *   └────────── n            └────────────── k
     */
    printf("── Demo 1: Impulse signal (delta function) ──\n");
    Complex impulse[8] = { {1,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} };
    fft(impulse, 8);
    print_spectrum("FFT of [1,0,0,0,0,0,0,0]", impulse, 8);
    printf("  → All bins have magnitude 1.0 (flat spectrum)\n");
    printf("  → An impulse contains ALL frequencies equally.\n\n");

    /*
     * ── Demo 2: DC signal → energy in bin 0 ─────────────────────
     *
     * Theory: A constant signal x[n] = 1 has zero frequency (DC).
     * Its DFT puts all N units of energy into bin 0:
     *   X[0] = Σ 1·e^0 = N,   X[k≠0] = 0.
     *
     *   Time domain:             Frequency domain:
     *   │████████                │
     *   │████████                │ N
     *   └────────── n            │█
     *                            └────────────── k
     */
    printf("── Demo 2: DC (constant) signal ──\n");
    Complex dc[8];
    for (int i = 0; i < 8; i++) dc[i] = (Complex){1.0, 0.0};
    fft(dc, 8);
    print_spectrum("FFT of [1,1,1,1,1,1,1,1]", dc, 8);
    printf("  → Only bin 0 has energy (magnitude 8 = N).\n");
    printf("  → DC = zero frequency.\n\n");

    /*
     * ── Demo 3: Alternating → Nyquist ─────────────────────────────
     *
     * Theory: The sequence [+1, -1, +1, -1, …] oscillates at the
     * fastest rate possible in discrete time — once every 2 samples.
     * That is exactly the Nyquist frequency: f_N = fs / 2.
     * Its DFT has energy only in bin N/2.
     *
     *   Time domain:             Frequency domain:
     *   │ █ █ █ █                │
     *   │─┼─┼─┼─┼── n           │             N
     *   │█ █ █ █                 │             █
     *                            └──────────── k
     *                                     bin N/2
     */
    printf("── Demo 3: Alternating signal [1,-1,1,-1,...] ──\n");
    Complex alt[8];
    for (int i = 0; i < 8; i++) alt[i] = (Complex){ (i % 2 == 0) ? 1.0 : -1.0, 0.0 };
    fft(alt, 8);
    print_spectrum("FFT of [1,-1,1,-1,1,-1,1,-1]", alt, 8);
    printf("  → Only bin N/2 (bin 4) has energy.\n");
    printf("  → This is the Nyquist frequency (highest representable).\n\n");

    /*
     * ── Demo 4: FFT ↔ IFFT roundtrip ──────────────────────────────
     *
     * Theory: The IFFT is defined as:
     *   x[n] = (1/N) Σ X[k] · e^{+j 2π k n / N}
     *
     * Applying FFT then IFFT (or vice-versa) recovers the original
     * signal exactly (up to floating-point rounding).  This is the
     * fundamental invertibility property of the DFT.
     *
     *   x[n] ──► FFT ──► X[k] ──► IFFT ──► x̂[n]  ≈  x[n]
     *
     * The roundtrip error should be on the order of machine epsilon
     * (~1e-15 for double precision).
     */
    printf("── Demo 4: FFT then IFFT recovers original ──\n");
    Complex original[8], roundtrip[8];
    for (int i = 0; i < 8; i++) {
        original[i] = (Complex){ sin(2.0 * M_PI * i / 8.0), 0.0 };
        roundtrip[i] = original[i];
    }
    fft(roundtrip, 8);
    printf("  After FFT:\n");
    for (int i = 0; i < 8; i++)
        printf("    X[%d] = %+.3f %+.3fi\n", i, roundtrip[i].re, roundtrip[i].im);
    ifft(roundtrip, 8);
    printf("  After IFFT:\n");
    double max_err = 0;
    for (int i = 0; i < 8; i++) {
        double err = fabs(roundtrip[i].re - original[i].re);
        if (err > max_err) max_err = err;
        printf("    x[%d] = %+.6f  (original: %+.6f,  error: %.1e)\n",
               i, roundtrip[i].re, original[i].re, err);
    }
    printf("  Max roundtrip error: %.1e\n\n", max_err);

    /*
     * ── Demo 5: Real-world spectrum analysis ────────────────────
     *
     * Theory: For a real-valued signal sampled at fs, the FFT
     * produces N complex bins.  Only the first N/2+1 are unique
     * (symmetry).  Each bin k maps to frequency:
     *
     *      f_k = k · fs / N
     *
     * The bin spacing (frequency resolution) is:
     *      Δf = fs / N = 8000 / 256 = 31.25 Hz
     *
     * We apply a Hann window before the FFT to reduce spectral
     * leakage (see Chapter 9 for details).
     *
     * Expected peaks:
     *   440 Hz → bin 440/31.25 ≈ 14.08  (between bins → slight spread)
     *  1000 Hz → bin 1000/31.25 = 32.0  (exactly on bin → clean peak)
     */
    printf("── Demo 5: 256-point FFT of 440 Hz + 1000 Hz ──\n");
    #define N   256
    #define FS  8000.0
    double signal[N];
    Complex spectrum[N];
    double mag[N];

    for (int i = 0; i < N; i++) {
        double t = (double)i / FS;
        signal[i] = sin(2.0 * M_PI * 440.0 * t) + 0.5 * sin(2.0 * M_PI * 1000.0 * t);
    }
    apply_window(signal, N, hann_window);
    fft_real(signal, spectrum, N);
    fft_magnitude(spectrum, mag, N);

    printf("  Frequency   | Magnitude (dB)\n");
    printf("  ────────────┼───────────────\n");
    for (int k = 0; k < N / 2; k++) {
        double freq = (double)k * FS / N;
        double db = db_from_magnitude(mag[k] / (N / 2));
        if (db > -40.0) {
            printf("  %7.1f Hz  | %+6.1f dB", freq, db);
            if (fabs(freq - 440.0) < FS / N || fabs(freq - 1000.0) < FS / N)
                printf("  ◄── peak!");
            printf("\n");
        }
    }
    printf("  Resolution: %.1f Hz/bin\n", FS / N);

    return 0;
}
