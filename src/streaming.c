/**
 * @file streaming.c
 * @brief Overlap-Add and Overlap-Save streaming convolution implementation.
 *
 * ── Overlap-Add Data Flow ────────────────────────────────────────
 *
 *   Block i arrives (L samples):
 *
 *   [x_block, 0, 0, ..., 0]    ← zero-pad to N = L + M - 1
 *         │
 *         ▼
 *       FFT(N)  ──► X[k] × H[k]  ──► IFFT(N)  ──► y_full[0..N-1]
 *                                                       │
 *                                            ┌──────────┴──────────┐
 *                                            │                     │
 *                                       y_full[0..L-1]      y_full[L..N-1]
 *                                       + tail_prev          = tail_new
 *                                            │
 *                                            ▼
 *                                       output[0..L-1]
 *
 * ── Overlap-Save Data Flow ───────────────────────────────────────
 *
 *   Maintain a running buffer of N samples:
 *     [last M-1 samples | new L samples]
 *         overlap            new data
 *
 *   [input_buf]  ──► FFT(N)  ──► X[k]×H[k]  ──► IFFT(N)  ──► y_full
 *                                                                 │
 *                              Discard first M-1 samples  ◄───────┘
 *                              Output y_full[M-1 .. N-1]  (L samples)
 */

#include "streaming.h"
#include "fft.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ── Overlap-Add Implementation ──────────────────────────────────── */

int ola_init(OlaState *s, const double *h, int filter_len, int block_size)
{
    s->filter_len = filter_len;
    s->block_size = block_size;

    /* FFT size = next power-of-2 ≥ block_size + filter_len - 1 */
    int min_n = block_size + filter_len - 1;
    s->fft_size = next_power_of_2(min_n);

    /* Pre-compute H[k] = FFT of zero-padded filter */
    s->H = (Complex *)calloc((size_t)s->fft_size, sizeof(Complex));
    if (!s->H) return -1;
    for (int i = 0; i < filter_len; i++) {
        s->H[i].re = h[i];
        s->H[i].im = 0.0;
    }
    fft(s->H, s->fft_size);

    /* Allocate scratch buffers */
    s->Xbuf   = (Complex *)calloc((size_t)s->fft_size, sizeof(Complex));
    s->tail   = (double *)calloc((size_t)(s->fft_size - block_size), sizeof(double));
    s->padded = (double *)calloc((size_t)s->fft_size, sizeof(double));

    if (!s->Xbuf || !s->tail || !s->padded) {
        ola_free(s);
        return -1;
    }

    return 0;
}

void ola_process(OlaState *s, const double *in, double *out)
{
    int N = s->fft_size;
    int L = s->block_size;
    int tail_len = N - L;

    /* Zero-pad input to N samples */
    memset(s->padded, 0, (size_t)N * sizeof(double));
    memcpy(s->padded, in, (size_t)L * sizeof(double));

    /* FFT of input block */
    for (int i = 0; i < N; i++) {
        s->Xbuf[i].re = s->padded[i];
        s->Xbuf[i].im = 0.0;
    }
    fft(s->Xbuf, N);

    /* Frequency-domain multiply: Y[k] = X[k] · H[k] */
    for (int k = 0; k < N; k++)
        s->Xbuf[k] = complex_mul(s->Xbuf[k], s->H[k]);

    /* IFFT back to time domain */
    ifft(s->Xbuf, N);

    /* Output: first L samples + overlap tail from previous block */
    for (int i = 0; i < L; i++)
        out[i] = s->Xbuf[i].re + s->tail[i];

    /* Save new tail for next block (samples L..N-1) */
    for (int i = 0; i < tail_len; i++)
        s->tail[i] = s->Xbuf[L + i].re;
}

void ola_free(OlaState *s)
{
    if (s) {
        free(s->H);      s->H      = NULL;
        free(s->Xbuf);   s->Xbuf   = NULL;
        free(s->tail);   s->tail   = NULL;
        free(s->padded); s->padded = NULL;
    }
}

/* ── Overlap-Save Implementation ─────────────────────────────────── */

int ols_init(OlsState *s, const double *h, int filter_len, int block_size)
{
    s->filter_len = filter_len;
    s->block_size = block_size;

    /* FFT size = next power-of-2 ≥ block_size + filter_len - 1 */
    int min_n = block_size + filter_len - 1;
    s->fft_size = next_power_of_2(min_n);

    /* Recompute L to match: L = N - M + 1 may differ from block_size */
    /* We keep block_size as requested; user must ensure consistency */

    /* Pre-compute H[k] */
    s->H = (Complex *)calloc((size_t)s->fft_size, sizeof(Complex));
    if (!s->H) return -1;
    for (int i = 0; i < filter_len; i++) {
        s->H[i].re = h[i];
        s->H[i].im = 0.0;
    }
    fft(s->H, s->fft_size);

    s->Xbuf = (Complex *)calloc((size_t)s->fft_size, sizeof(Complex));
    s->input_buf = (double *)calloc((size_t)s->fft_size, sizeof(double));

    if (!s->Xbuf || !s->input_buf) {
        ols_free(s);
        return -1;
    }

    return 0;
}

void ols_process(OlsState *s, const double *in, double *out)
{
    int N = s->fft_size;
    int M = s->filter_len;
    int L = s->block_size;

    /* Shift: keep last M-1 samples, append new L samples */
    memmove(s->input_buf, s->input_buf + L, (size_t)(M - 1) * sizeof(double));
    memcpy(s->input_buf + (M - 1), in, (size_t)L * sizeof(double));

    /* Zero-pad rest (if N > M-1 + L) */
    int filled = M - 1 + L;
    if (filled < N)
        memset(s->input_buf + filled, 0, (size_t)(N - filled) * sizeof(double));

    /* FFT of input segment */
    for (int i = 0; i < N; i++) {
        s->Xbuf[i].re = s->input_buf[i];
        s->Xbuf[i].im = 0.0;
    }
    fft(s->Xbuf, N);

    /* Y[k] = X[k] · H[k] */
    for (int k = 0; k < N; k++)
        s->Xbuf[k] = complex_mul(s->Xbuf[k], s->H[k]);

    /* IFFT */
    ifft(s->Xbuf, N);

    /* Discard first M-1 samples (circular convolution artefacts) */
    for (int i = 0; i < L; i++)
        out[i] = s->Xbuf[M - 1 + i].re;
}

void ols_free(OlsState *s)
{
    if (s) {
        free(s->H);         s->H         = NULL;
        free(s->Xbuf);      s->Xbuf      = NULL;
        free(s->input_buf); s->input_buf = NULL;
    }
}
