/**
 * @file advanced_fft.h
 * @brief Advanced FFT algorithms — Goertzel, optimised real-FFT, chirp-Z.
 *
 * These complement the basic radix-2 FFT in fft.h with:
 *   - Goertzel: O(N) for a single frequency bin (DTMF, pitch detect)
 *   - Sliding DFT: O(1) per sample for streaming single-bin tracking
 *
 * ── When to Use Each Algorithm ───────────────────────────────────
 *
 *   Need ALL N bins?       → fft()          O(N log N)
 *   Need < log₂(N) bins?  → goertzel()     O(N) per bin
 *   Need 1 bin, streaming? → sliding_dft()  O(1) per sample
 *   Need non-uniform bins? → chirp-Z (future)
 *
 * ── Goertzel Algorithm ───────────────────────────────────────────
 *
 *   Computes X[k] for a SINGLE k without the full FFT.
 *   Uses a 2nd-order IIR filter tuned to frequency bin k:
 *
 *     s[n] = x[n] + 2·cos(2πk/N)·s[n-1] - s[n-2]
 *
 *   After N iterations:
 *     X[k] = s[N-1] - W_N^k · s[N-2]
 *
 *   where W_N^k = e^{-j2πk/N}
 *
 * See chapters/19-advanced-fft.md for the full tutorial.
 */

#ifndef ADVANCED_FFT_H
#define ADVANCED_FFT_H

#include "dsp_utils.h"   /* Complex type */

/* ── Goertzel Algorithm ──────────────────────────────────────────── */

/**
 * @brief Compute a single DFT bin X[k] using the Goertzel algorithm.
 *
 * Cost: 1 real multiply + 2 adds per sample → O(N) total.
 * More efficient than a full FFT when you need ≤ log₂(N) bins.
 *
 * @param x    Input signal array, length n
 * @param n    Signal length (does NOT need to be a power of 2)
 * @param k    Frequency bin index (0 ≤ k < n)
 * @return     Complex value X[k]
 */
Complex goertzel(const double *x, int n, int k);

/**
 * @brief Compute magnitude² of DFT bin k (avoids final complex multiply).
 *
 * Returns |X[k]|² — useful for power detection (e.g. DTMF).
 * Slightly more efficient than goertzel() + complex_mag()².
 *
 * @param x    Input signal, length n
 * @param n    Signal length
 * @param k    Bin index
 * @return     |X[k]|² (power, not amplitude)
 */
double goertzel_magnitude_sq(const double *x, int n, int k);

/**
 * @brief Compute DFT bin for an arbitrary (non-integer) frequency.
 *
 * Generalised Goertzel: target freq need not align with a DFT bin.
 *
 * @param x        Input signal, length n
 * @param n        Signal length
 * @param freq_hz  Target frequency in Hz
 * @param fs       Sampling rate in Hz
 * @return         Complex spectral value at freq_hz
 */
Complex goertzel_freq(const double *x, int n, double freq_hz, double fs);

/* ── DTMF Detection ─────────────────────────────────────────────── */

/**
 * DTMF frequency pairs (row + column tones):
 *
 *         1209 Hz  1336 Hz  1477 Hz  1633 Hz
 *  697 Hz    1        2        3        A
 *  770 Hz    4        5        6        B
 *  852 Hz    7        8        9        C
 *  941 Hz    *        0        #        D
 */

/**
 * @brief Detect a DTMF digit from an audio frame using Goertzel.
 *
 * Computes power at the 8 DTMF frequencies and finds the strongest
 * row + column pair.
 *
 * @param x      Audio frame, length n
 * @param n      Frame length (typically 205 samples at 8 kHz)
 * @param fs     Sampling rate
 * @return       Detected character ('0'-'9','*','#','A'-'D') or '?' if invalid
 */
char dtmf_detect(const double *x, int n, double fs);

/* ── Sliding DFT ─────────────────────────────────────────────────── */

/**
 * @brief State for a sliding DFT filter (single bin tracker).
 *
 * Maintains a running DFT bin that updates in O(1) per new sample.
 */
typedef struct {
    int    N;           /**< Window size                       */
    int    k;           /**< Bin index being tracked           */
    Complex coeff;      /**< Twiddle: e^{j2πk/N}              */
    Complex bin;        /**< Current DFT bin value             */
    double *buffer;     /**< Circular buffer of last N samples */
    int    pos;         /**< Current write position            */
} SlidingDFT;

/**
 * @brief Initialise a sliding DFT to track bin k of an N-point window.
 * @param s    State struct (caller-allocated)
 * @param N    Window size
 * @param k    Bin to track
 * @return     0 on success, -1 on error (malloc failure)
 */
int sliding_dft_init(SlidingDFT *s, int N, int k);

/**
 * @brief Feed one new sample and get the updated DFT bin.
 * @param s        Initialised sliding DFT state
 * @param sample   New input sample
 * @return         Updated X[k] for the current window
 */
Complex sliding_dft_update(SlidingDFT *s, double sample);

/**
 * @brief Free resources allocated by sliding_dft_init.
 */
void sliding_dft_free(SlidingDFT *s);

#endif /* ADVANCED_FFT_H */
