/**
 * @file spectrum.h
 * @brief Power Spectral Density estimation — periodogram & Welch's method.
 *
 * All functions output one-sided PSD in units of power/Hz (linear scale).
 * Convert to dB with  10·log10(psd[k])  or use db_from_magnitude(sqrt(psd[k])).
 *
 * Frequency axis: psd[k] corresponds to f_k = k · fs / nfft, for k = 0 … nfft/2.
 * Output length is always nfft/2 + 1.
 */
#ifndef SPECTRUM_H
#define SPECTRUM_H

#include "dsp_utils.h"   /* Complex, window_fn */

/**
 * Basic periodogram: PSD = |FFT(x)|² / N.
 *
 * @param x      Input signal (length n).
 * @param n      Number of samples.
 * @param psd    Output array, length nfft/2 + 1 (one-sided).
 * @param nfft   FFT size (must be power-of-2, >= n; zero-padded if nfft > n).
 * @return       Number of output bins (nfft/2 + 1), or -1 on error.
 */
int periodogram(const double *x, int n, double *psd, int nfft);

/**
 * Windowed periodogram: PSD = |FFT(w·x)|² / (sum(w²)·N).
 *
 * @param x      Input signal (length n).
 * @param n      Number of samples.
 * @param psd    Output array, length nfft/2 + 1.
 * @param nfft   FFT size (power-of-2, >= n).
 * @param win    Window function (e.g. hann_window).  NULL → rectangular.
 * @return       Number of output bins, or -1 on error.
 */
int periodogram_windowed(const double *x, int n, double *psd, int nfft,
                         window_fn win);

/**
 * Welch's averaged, modified periodogram.
 *
 * Splits x into overlapping segments, multiplies each by the window,
 * takes the periodogram of each, and averages across segments.
 *
 * @param x        Input signal (length n).
 * @param n        Number of samples.
 * @param psd      Output array, length nfft/2 + 1.
 * @param nfft     FFT size per segment (power-of-2, >= seg_len).
 * @param seg_len  Segment length (samples).  Must be <= nfft.
 * @param overlap  Overlap in samples (typically seg_len/2).
 * @param win      Window function.  NULL → rectangular.
 * @return         Number of segments averaged, or -1 on error.
 */
int welch_psd(const double *x, int n, double *psd, int nfft,
              int seg_len, int overlap, window_fn win);

/**
 * Cross power spectral density via Welch's method: Pxy = conj(X)·Y.
 *
 * @param x, y     Two signals of equal length n.
 * @param n        Number of samples.
 * @param cpsd     Output complex array, length nfft/2 + 1.
 * @param nfft     FFT size per segment (power-of-2, >= seg_len).
 * @param seg_len  Segment length.
 * @param overlap  Overlap in samples.
 * @param win      Window function.  NULL → rectangular.
 * @return         Number of segments averaged, or -1 on error.
 */
int cross_psd(const double *x, const double *y, int n,
              Complex *cpsd, int nfft,
              int seg_len, int overlap, window_fn win);

/**
 * Convert one-sided PSD array to dB scale:  psd_db[k] = 10·log10(psd[k]).
 * Values below floor_db are clamped.
 *
 * @param psd      Input linear PSD.
 * @param psd_db   Output dB-scaled PSD.
 * @param n_bins   Number of bins (nfft/2 + 1).
 * @param floor_db Minimum dB value (e.g. -120).
 */
void psd_to_db(const double *psd, double *psd_db, int n_bins, double floor_db);

/**
 * Build the frequency axis for a one-sided PSD.
 *
 * @param freq     Output array, length n_bins.
 * @param n_bins   Number of bins (nfft/2 + 1).
 * @param fs       Sample rate in Hz.
 */
void psd_freq_axis(double *freq, int n_bins, double fs);

#endif /* SPECTRUM_H */
