/**
 * @file correlation.h
 * @brief Cross-correlation, autocorrelation, and normalised variants.
 *
 * All functions use FFT-based computation for O(N log N) performance.
 * Linear (non-circular) correlation is obtained via zero-padding.
 *
 * Output conventions:
 *   xcorr:    r has length nx + ny - 1, centred at index (nx - 1).
 *   autocorr: r has length 2*n - 1, centred at index (n - 1).
 *             r[n-1] = autocorrelation at lag 0.
 *
 * If only positive lags are desired use r[centre .. centre + max_lag].
 */
#ifndef CORRELATION_H
#define CORRELATION_H

/**
 * Cross-correlation of x and y (linear, unbiased scaling).
 *
 * R_xy[m] = sum_n  x[n] · y[n + m]
 *
 * @param x      First signal (length nx).
 * @param nx     Length of x.
 * @param y      Second signal (length ny).
 * @param ny     Length of y.
 * @param r      Output array of length nx + ny - 1.
 * @return       Number of output samples (nx + ny - 1), or -1 on error.
 */
int xcorr(const double *x, int nx, const double *y, int ny, double *r);

/**
 * Normalised cross-correlation in [-1, 1].
 * Divides xcorr by sqrt(E_x · E_y) where E = sum(x²).
 *
 * @param x, y, nx, ny  Signals.
 * @param r      Output array of length nx + ny - 1.
 * @return       Output length or -1.
 */
int xcorr_normalized(const double *x, int nx,
                     const double *y, int ny, double *r);

/**
 * Autocorrelation of x — equivalent to xcorr(x, n, x, n, r).
 *
 * @param x   Input signal (length n).
 * @param n   Signal length.
 * @param r   Output array of length 2*n - 1.
 * @return    Output length (2*n - 1) or -1.
 */
int autocorr(const double *x, int n, double *r);

/**
 * Normalised autocorrelation in [-1, 1].
 * r[n-1] (lag 0) is always 1.0.
 *
 * @param x   Input signal (length n).
 * @param n   Signal length.
 * @param r   Output array of length 2*n - 1.
 * @return    Output length or -1.
 */
int autocorr_normalized(const double *x, int n, double *r);

/**
 * Find the lag (sample offset) of the peak in a cross-correlation.
 * Searches r[0 .. r_len-1].
 *
 * @param r      Correlation output.
 * @param r_len  Length of r.
 * @param centre Index of lag-0 in r (for xcorr: ny-1; for autocorr: n-1).
 * @return       Lag in samples (can be negative).
 */
int xcorr_peak_lag(const double *r, int r_len, int centre);

#endif /* CORRELATION_H */
