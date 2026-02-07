/**
 * @file correlation.c
 * @brief FFT-based cross-correlation and autocorrelation.
 *
 * Linear (non-circular) correlation is achieved by zero-padding both
 * signals to length nfft >= nx + ny - 1, computing FFTs, multiplying
 * conj(X)·Y in frequency domain, and IFFT-ing.
 */
#define _POSIX_C_SOURCE 200809L
#include "correlation.h"
#include "fft.h"
#include "dsp_utils.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Core FFT-based cross-correlation                                  */
/* ------------------------------------------------------------------ */

/**
 * Internal: compute raw cross-correlation of x (nx) and y (ny).
 * Result written to r which must hold nx + ny - 1 doubles.
 */
static int xcorr_raw(const double *x, int nx,
                     const double *y, int ny, double *r)
{
    if (!x || !y || !r || nx <= 0 || ny <= 0)
        return -1;

    int r_len = nx + ny - 1;
    int nfft  = next_power_of_2(r_len);

    Complex *bx = (Complex *)calloc((size_t)nfft, sizeof(Complex));
    Complex *by = (Complex *)calloc((size_t)nfft, sizeof(Complex));
    if (!bx || !by) {
        free(bx);
        free(by);
        return -1;
    }

    /* Load signals (zero-padded) */
    for (int i = 0; i < nx; i++) { bx[i].re = x[i]; bx[i].im = 0.0; }
    for (int i = 0; i < ny; i++) { by[i].re = y[i]; by[i].im = 0.0; }

    fft(bx, nfft);
    fft(by, nfft);

    /* conj(X) · Y */
    for (int k = 0; k < nfft; k++) {
        double xr = bx[k].re, xi = bx[k].im;
        double yr = by[k].re, yi = by[k].im;
        bx[k].re = xr * yr + xi * yi;   /* Re(conj(X)·Y) */
        bx[k].im = xr * yi - xi * yr;   /* Im(conj(X)·Y) */
    }

    ifft(bx, nfft);

    /*
     * IFFT(conj(X)·Y)[m] = sum_n x[n]·y[n+m].
     *
     * Valid lag range: -(nx-1) .. +(ny-1).
     * Centre (lag 0) stored at output index (nx-1).
     *
     * In the circular buffer bx:
     *   lag  0      → bx[0]
     *   lag +1      → bx[1]
     *   ...
     *   lag +(ny-1) → bx[ny-1]
     *   lag -1      → bx[nfft - 1]
     *   ...
     *   lag -(nx-1) → bx[nfft - (nx-1)]
     */
    /* Non-negative lags: 0 .. ny-1 → r[nx-1 .. nx-1+ny-1] */
    for (int m = 0; m < ny; m++)
        r[nx - 1 + m] = bx[m].re;

    /* Negative lags: -(nx-1) .. -1 → r[0 .. nx-2] */
    for (int m = 1; m < nx; m++)
        r[nx - 1 - m] = bx[nfft - m].re;

    free(bx);
    free(by);
    return r_len;
}

/* ------------------------------------------------------------------ */
/*  Public API                                                        */
/* ------------------------------------------------------------------ */

int xcorr(const double *x, int nx, const double *y, int ny, double *r)
{
    return xcorr_raw(x, nx, y, ny, r);
}

int xcorr_normalized(const double *x, int nx,
                     const double *y, int ny, double *r)
{
    int r_len = xcorr_raw(x, nx, y, ny, r);
    if (r_len < 0) return -1;

    /* Energy of both signals */
    double ex = 0.0, ey = 0.0;
    for (int i = 0; i < nx; i++) ex += x[i] * x[i];
    for (int i = 0; i < ny; i++) ey += y[i] * y[i];

    double norm = sqrt(ex * ey);
    if (norm < 1e-30) return r_len;

    for (int i = 0; i < r_len; i++)
        r[i] /= norm;

    return r_len;
}

int autocorr(const double *x, int n, double *r)
{
    return xcorr_raw(x, n, x, n, r);
}

int autocorr_normalized(const double *x, int n, double *r)
{
    int r_len = xcorr_raw(x, n, x, n, r);
    if (r_len < 0) return -1;

    /* Normalise by lag-0 value (energy) */
    double r0 = r[n - 1];   /* centre = n - 1 for autocorr */
    if (fabs(r0) < 1e-30) return r_len;

    for (int i = 0; i < r_len; i++)
        r[i] /= r0;

    return r_len;
}

int xcorr_peak_lag(const double *r, int r_len, int centre)
{
    if (!r || r_len <= 0) return 0;
    int best = 0;
    double best_val = fabs(r[0]);
    for (int i = 1; i < r_len; i++) {
        if (fabs(r[i]) > best_val) {
            best_val = fabs(r[i]);
            best = i;
        }
    }
    return best - centre;
}
