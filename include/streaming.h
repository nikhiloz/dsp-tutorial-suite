/**
 * @file streaming.h
 * @brief Overlap-Add and Overlap-Save streaming convolution.
 *
 * Enables real-time, block-based FIR filtering of continuous audio streams
 * without boundary artefacts.  Feed blocks of samples and get filtered
 * output blocks with zero latency beyond the filter group delay.
 *
 * ── Overlap-Add (OLA) ───────────────────────────────────────────
 *
 *   Input blocks:    |===B0===|===B1===|===B2===|  (L samples each)
 *
 *   Each block zero-padded to N = L + M - 1 (M = filter length):
 *     [B0, 0, 0, ..., 0] → FFT → ×H(k) → IFFT → y0 (N samples)
 *     [B1, 0, 0, ..., 0] → FFT → ×H(k) → IFFT → y1 (N samples)
 *
 *   Overlap-add the tails:
 *     output: |==y0[:L]==|     |
 *             |     |=tail0=+y1[:L]|     |
 *             |           |    =tail1=+y2[:L]|
 *
 *   Valid output: L samples per block.
 *
 * ── Overlap-Save (OLS) ──────────────────────────────────────────
 *
 *   Input blocks (N samples, with M-1 overlap):
 *     [x_prev(M-1) | x_new(L)] → FFT → ×H(k) → IFFT → discard first M-1
 *
 *   More memory-efficient: no accumulation buffer needed.
 *   Valid output: L = N - M + 1 samples per block.
 *
 * See chapters/16-overlap-add-save.md for the full tutorial.
 */

#ifndef STREAMING_H
#define STREAMING_H

#include "dsp_utils.h"

/* ── Overlap-Add state ───────────────────────────────────────────── */

typedef struct {
    int    block_size;   /**< L: input block size (user-supplied)     */
    int    fft_size;     /**< N: L + M - 1, rounded to power-of-2    */
    int    filter_len;   /**< M: FIR filter length                    */

    Complex *H;          /**< Pre-computed FFT of filter (N bins)     */
    Complex *Xbuf;       /**< Scratch: FFT of input block             */
    double  *tail;       /**< Overlap tail from previous block (M-1)  */
    double  *padded;     /**< Zero-padded input (N samples)           */
} OlaState;

/**
 * @brief Initialise overlap-add streaming filter.
 *
 * @param s           State struct (caller-allocated)
 * @param h           FIR filter coefficients, length filter_len
 * @param filter_len  Number of filter taps (M)
 * @param block_size  Number of new input samples per call (L)
 * @return            0 on success, -1 on error
 */
int ola_init(OlaState *s, const double *h, int filter_len, int block_size);

/**
 * @brief Process one block of input samples.
 *
 * @param s     Initialised OLA state
 * @param in    Input block, exactly s->block_size samples
 * @param out   Output block, exactly s->block_size samples (caller-allocated)
 */
void ola_process(OlaState *s, const double *in, double *out);

/**
 * @brief Free all resources allocated by ola_init.
 */
void ola_free(OlaState *s);

/* ── Overlap-Save state ──────────────────────────────────────────── */

typedef struct {
    int    block_size;   /**< L: valid output samples per block       */
    int    fft_size;     /**< N: block_size + filter_len - 1 (pow2)   */
    int    filter_len;   /**< M: filter length                        */

    Complex *H;          /**< Pre-computed FFT of filter (N bins)     */
    Complex *Xbuf;       /**< Scratch: FFT of input segment           */
    double  *input_buf;  /**< Current input segment (N samples)       */
} OlsState;

/**
 * @brief Initialise overlap-save streaming filter.
 *
 * @param s           State struct (caller-allocated)
 * @param h           FIR filter coefficients, length filter_len
 * @param filter_len  Number of filter taps (M)
 * @param block_size  Number of valid output samples per call (L)
 * @return            0 on success, -1 on error
 */
int ols_init(OlsState *s, const double *h, int filter_len, int block_size);

/**
 * @brief Process one block of input samples.
 *
 * @param s     Initialised OLS state
 * @param in    Input block, exactly s->block_size samples
 * @param out   Output block, exactly s->block_size samples
 */
void ols_process(OlsState *s, const double *in, double *out);

/**
 * @brief Free all resources allocated by ols_init.
 */
void ols_free(OlsState *s);

#endif /* STREAMING_H */
