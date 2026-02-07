# DSP Tutorial Suite: API Reference

Complete public API for all 23 library modules. Every function is C99,
operates on caller-supplied buffers (no hidden global state), and has
zero external dependencies beyond `<math.h>`.

> **📊 API Overview** — [View full-size API diagram →](diagrams/api_reference.png)
>
> **📊 Module Dependencies** — [View full-size diagram →](diagrams/modules.png)

**Root dependency:** 9 modules include `dsp_utils.h` for the `Complex` type
and window functions: `fft`, `advanced_fft`, `signal_gen`, `iir`, `hilbert`,
`spectrum`, `streaming`, `realtime`, `optimization`.
The remaining 14 modules have zero inter-module dependencies.

---

## 1. dsp_utils.h — Core Utilities

**Header:** [`include/dsp_utils.h`](../include/dsp_utils.h)
| **Source:** [`src/dsp_utils.c`](../src/dsp_utils.c)
| **Tutorials:** [Ch 03 — Complex Numbers](../chapters/03-complex-numbers/tutorial.md), [Ch 09 — Window Functions](../chapters/09-window-functions/tutorial.md)

### Data Types

```c
typedef struct { double re; double im; } Complex;
typedef double (*window_fn)(int n, int i);
```

### Complex Arithmetic (6 functions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `complex_add` | `Complex complex_add(Complex a, Complex b)` | $(a + b)$ |
| `complex_sub` | `Complex complex_sub(Complex a, Complex b)` | $(a - b)$ |
| `complex_mul` | `Complex complex_mul(Complex a, Complex b)` | $(a \times b)$ via $(ac-bd) + (ad+bc)i$ |
| `complex_mag` | `double complex_mag(Complex z)` | $\|z\| = \sqrt{re^2 + im^2}$ |
| `complex_phase` | `double complex_phase(Complex z)` | $\text{atan2}(im, re)$ in radians |
| `complex_from_polar` | `Complex complex_from_polar(double mag, double phase)` | $(r, \theta) \to (a, b)$ |

### Window Functions (3 + 1 applicator)

| Function | Side Lobes | Main Lobe |
|----------|-----------|-----------|
| `hann_window(int n, int i)` | −31 dB | 4 bins |
| `hamming_window(int n, int i)` | −42 dB | 4 bins |
| `blackman_window(int n, int i)` | −58 dB | 6 bins |

`void apply_window(double *signal, int n, window_fn w)` — multiplies in-place.

### Helpers (3 functions)

| Function | Description |
|----------|-------------|
| `int next_power_of_2(int n)` | Smallest power of 2 ≥ `n` |
| `double db_from_magnitude(double mag)` | $20 \log_{10}(mag)$, returns −200 for zero |
| `double rms(const double *signal, int n)` | $\sqrt{\frac{1}{N}\sum x[i]^2}$ |

---

## 2. signal_gen.h — Signal Generation

**Header:** [`include/signal_gen.h`](../include/signal_gen.h)
| **Source:** [`src/signal_gen.c`](../src/signal_gen.c)
| **Tutorial:** [Ch 01 — Signals & Sequences](../chapters/01-signals-and-sequences/tutorial.md)

### Functions (12)

| Function | Description |
|----------|-------------|
| `gen_impulse(double *out, int n, int delay)` | Unit impulse δ[n−delay] |
| `gen_step(double *out, int n, int start)` | Unit step u[n−start] |
| `gen_exponential(double *out, int n, double amp, double base)` | $a \cdot b^n$ |
| `gen_cosine(double *out, int n, double amp, double freq, double phase, double fs)` | Cosine wave |
| `gen_sine(double *out, int n, double amp, double freq, double phase, double fs)` | Sine wave |
| `gen_complex_exp(Complex *out, int n, double amp, double freq, double phase, double fs)` | $A e^{j(2\pi f n/f_s + \phi)}$ |
| `gen_chirp(double *out, int n, double amp, double f0, double f1, double fs)` | Linear FM sweep |
| `gen_multi_tone(double *out, int n, const double *freqs, const double *amps, int nf, double fs)` | Sum of sinusoids |
| `gen_white_noise(double *out, int n, double amp, unsigned int seed)` | Uniform white noise |
| `gen_gaussian_noise(double *out, int n, double mean, double stddev, unsigned int seed)` | Gaussian noise |
| `signal_add(double *a, const double *b, int n)` | In-place: `a[i] += b[i]` |
| `signal_scale(double *x, int n, double scale)` | In-place: `x[i] *= scale` |

---

## 3. convolution.h — Convolution & Correlation

**Header:** [`include/convolution.h`](../include/convolution.h)
| **Source:** [`src/convolution.c`](../src/convolution.c)
| **Tutorial:** [Ch 04 — LTI Systems](../chapters/04-lti-systems/tutorial.md)

### Functions (7)

| Function | Description |
|----------|-------------|
| `convolve(x, x_len, h, h_len, y)` | Linear convolution, output length x_len+h_len−1 |
| `convolve_causal(x, x_len, h, h_len, y)` | Causal conv (output length = x_len) |
| `cross_correlate(x, x_len, y, y_len, r)` | Cross-correlation rxy[l] |
| `auto_correlate(x, x_len, r)` | Autocorrelation rxx[l] |
| `is_bibo_stable(h, h_len)` | Returns 1 if Σ\|h[n]\| < ∞ |
| `signal_energy(x, x_len)` | $\sum\|x[n]\|^2$ |
| `signal_power(x, x_len)` | $\frac{1}{N}\sum\|x[n]\|^2$ |

---

## 4. fft.h — Fast Fourier Transform

**Header:** [`include/fft.h`](../include/fft.h)
| **Source:** [`src/fft.c`](../src/fft.c)
| **Tutorial:** [Ch 08 — FFT Algorithms](../chapters/08-fft-fundamentals/tutorial.md)

**Algorithm:** Cooley-Tukey Radix-2 DIT. **Constraint:** `n` must be power of 2.

### Functions (5)

| Function | Description |
|----------|-------------|
| `void fft(Complex *x, int n)` | In-place forward FFT |
| `void fft_real(const double *in, Complex *out, int n)` | Real → complex FFT wrapper |
| `void ifft(Complex *x, int n)` | In-place inverse FFT (conjugate trick + 1/N) |
| `void fft_magnitude(const Complex *x, double *mag, int n)` | Extract \|X[k]\| |
| `void fft_phase(const Complex *x, double *phase, int n)` | Extract ∠X[k] |

---

## 5. advanced_fft.h — Goertzel, DTMF, Sliding DFT

**Header:** [`include/advanced_fft.h`](../include/advanced_fft.h)
| **Source:** [`src/advanced_fft.c`](../src/advanced_fft.c)
| **Tutorial:** [Ch 19 — Advanced FFT](../chapters/19-advanced-fft/tutorial.md)

### Functions (7)

| Function | Description |
|----------|-------------|
| `Complex goertzel(x, n, k)` | Single-bin DFT via 2nd-order IIR |
| `double goertzel_magnitude_sq(x, n, k)` | \|X[k]\|² without sqrt |
| `Complex goertzel_freq(x, n, freq_hz, fs)` | Goertzel at arbitrary frequency |
| `char dtmf_detect(x, n, fs)` | Detect DTMF digit (8 Goertzel filters) |
| `int sliding_dft_init(SlidingDFT *s, int N, int k)` | Init sliding window |
| `Complex sliding_dft_update(SlidingDFT *s, double sample)` | O(1) per-sample update |
| `void sliding_dft_free(SlidingDFT *s)` | Release memory |

---

## 6. filter.h — FIR Digital Filters

**Header:** [`include/filter.h`](../include/filter.h)
| **Source:** [`src/filter.c`](../src/filter.c)
| **Tutorial:** [Ch 10 — FIR Filters](../chapters/10-digital-filters/tutorial.md)

### Functions (3)

| Function | Description |
|----------|-------------|
| `void fir_filter(in, out, n, h, order)` | Direct-form FIR convolution |
| `void fir_moving_average(h, taps)` | Generate uniform coefficients |
| `void fir_lowpass(h, taps, cutoff)` | Windowed-sinc + Hamming lowpass |

---

## 7. iir.h — IIR Filter Design & Processing

**Header:** [`include/iir.h`](../include/iir.h)
| **Source:** [`src/iir.c`](../src/iir.c)
| **Tutorials:** [Ch 06 — Frequency Response](../chapters/06-frequency-response/tutorial.md), [Ch 11 — IIR Design](../chapters/11-iir-filter-design/tutorial.md), [Ch 12 — Filter Structures](../chapters/12-filter-structures/tutorial.md)

### Data Types

```c
typedef struct { double b[3]; double a[3]; } Biquad;
typedef struct { Biquad sections[MAX_SOS]; int count; double gain; ... } SOSCascade;
```

### Functions (17)

| Category | Function | Description |
|----------|----------|-------------|
| Direct | `iir_filter(b, b_len, a, a_len, x, y, n)` | General IIR (any order) |
| Direct | `iir_impulse_response(b, b_len, a, a_len, h, n)` | Compute h[n] |
| Biquad | `biquad_df1_init / biquad_df2t_init` | Zero the state |
| Biquad | `biquad_process_df1 / biquad_process_df2t` | Process one sample |
| Biquad | `biquad_process_block(bq, state, x, y, n)` | Block processing |
| SOS | `sos_init / sos_process_sample / sos_process_block` | Cascade of biquads |
| Design | `butterworth_lowpass(order, cutoff, sos)` | Butterworth LP |
| Design | `butterworth_highpass(order, cutoff, sos)` | Butterworth HP |
| Design | `chebyshev1_lowpass(order, ripple_db, cutoff, sos)` | Chebyshev Type-I LP |
| Analysis | `freq_response(b, b_len, a, a_len, mag, phase, n)` | H(e^jω) |
| Analysis | `biquad_freq_response / sos_freq_response` | Biquad/SOS response |
| Analysis | `group_delay_at(b, b_len, a, a_len, omega)` | τ(ω) at one frequency |

---

## 8. spectrum.h — Spectral Estimation

**Header:** [`include/spectrum.h`](../include/spectrum.h)
| **Source:** [`src/spectrum.c`](../src/spectrum.c)
| **Tutorials:** [Ch 13 — Spectral Analysis](../chapters/13-spectral-analysis/tutorial.md), [Ch 14 — PSD & Welch](../chapters/14-psd-welch/tutorial.md)

### Functions (6)

| Function | Description |
|----------|-------------|
| `periodogram(x, n, psd, nfft)` | \|X[k]\|²/N (rectangular window) |
| `periodogram_windowed(x, n, psd, nfft, w)` | Windowed periodogram |
| `welch_psd(x, n, psd, nfft, seg_len, overlap, w)` | Welch's method (averaged segments) |
| `cross_psd(x, y, n, cpsd, nfft, seg_len, overlap, w)` | Cross-PSD Sxy(f) |
| `psd_to_db(psd, psd_db, n_bins, floor_db)` | Convert to dB scale |
| `psd_freq_axis(freq, n_bins, fs)` | Generate frequency axis |

---

## 9. spectral_est.h — Parametric Methods (MUSIC, Capon)

**Header:** [`include/spectral_est.h`](../include/spectral_est.h)
| **Source:** [`src/spectral_est.c`](../src/spectral_est.c)
| **Tutorial:** [Ch 25 — Parametric Spectral](../chapters/25-parametric-spectral/tutorial.md)

### Functions (5)

| Function | Description |
|----------|-------------|
| `correlation_matrix(x, n, p, R)` | Estimate p×p correlation matrix |
| `eigen_symmetric(A, p, eigenvalues, eigenvectors)` | Eigendecomposition |
| `music_spectrum(x, n, p, n_sigs, psd, nfreqs, fs)` | MUSIC pseudo-spectrum |
| `music_frequencies(x, n, p, n_sigs, freqs, fs)` | Peak frequency extraction |
| `capon_spectrum(x, n, p, psd, nfreqs, fs)` | Capon (MVDR) spectrum |

---

## 10. cepstrum.h — Cepstrum & MFCCs

**Header:** [`include/cepstrum.h`](../include/cepstrum.h)
| **Source:** [`src/cepstrum.c`](../src/cepstrum.c)
| **Tutorial:** [Ch 26 — Cepstrum & MFCC](../chapters/26-cepstrum-mfcc/tutorial.md)

### Functions (8)

| Function | Description |
|----------|-------------|
| `cepstrum_real(x, n, c, nfft)` | Real cepstrum via IFFT{log\|X\|} |
| `cepstrum_complex(x, n, c, nfft)` | Complex cepstrum |
| `cepstrum_lifter(c, nfft, L, envelope)` | Low-quefrency liftering |
| `hz_to_mel(f_hz)` / `mel_to_hz(mel)` | Mel scale conversion |
| `mel_filterbank(power_spec, nfft, fs, n_filters, output)` | Triangular Mel filters |
| `compute_mfcc(frame, frame_len, nfft, n_filters, n_mfcc, mfcc, fs)` | Full MFCC pipeline |
| `dct_ii(x, y, n)` | DCT Type-II |

---

## 11. correlation.h — FFT-Based Correlation

**Header:** [`include/correlation.h`](../include/correlation.h)
| **Source:** [`src/correlation.c`](../src/correlation.c)
| **Tutorial:** [Ch 15 — Correlation](../chapters/15-correlation/tutorial.md)

### Functions (5)

| Function | Description |
|----------|-------------|
| `xcorr(x, nx, y, ny, r)` | FFT-based cross-correlation |
| `xcorr_normalized(x, nx, y, ny, r)` | Normalized ρ ∈ [−1, +1] |
| `autocorr(x, n, r)` | FFT-based autocorrelation |
| `autocorr_normalized(x, n, r)` | Normalized autocorrelation |
| `xcorr_peak_lag(r, r_len, centre)` | Find peak lag index |

---

## 12. hilbert.h — Hilbert Transform & Analytic Signal

**Header:** [`include/hilbert.h`](../include/hilbert.h)
| **Source:** [`src/hilbert.c`](../src/hilbert.c)
| **Tutorial:** [Ch 20 — Hilbert Transform](../chapters/20-hilbert-transform/tutorial.md)

### Functions (5)

| Function | Description |
|----------|-------------|
| `hilbert_design(h, taps)` | Design Hilbert FIR coefficients |
| `analytic_signal(x, n, z, taps)` | x[n] + jx̂[n] via FIR |
| `analytic_signal_fft(x, n, z)` | Analytic signal via FFT method |
| `envelope(x, n, env, taps)` | \|xa[n]\| = amplitude envelope |
| `inst_frequency(x, n, freq, taps)` | Instantaneous frequency |

---

## 13. lpc.h — Linear Prediction

**Header:** [`include/lpc.h`](../include/lpc.h)
| **Source:** [`src/lpc.c`](../src/lpc.c)
| **Tutorial:** [Ch 24 — Linear Prediction](../chapters/24-linear-prediction/tutorial.md)

### Functions (6)

| Function | Description |
|----------|-------------|
| `lpc_autocorrelation(x, n, r, p)` | Compute r[0..p] |
| `levinson_durbin(r, p, a, reflection, pred_error)` | O(p²) LP coefficient solver |
| `lpc_coefficients(x, n, p, a, reflection, pred_error)` | All-in-one wrapper |
| `lpc_residual(x, n, a, p, e)` | Prediction error signal |
| `lpc_synthesise(e, n, a, p, y)` | 1/A(z) synthesis filter |
| `lpc_spectrum(a, p, E, psd, nfreqs)` | AR spectral envelope |

---

## 14. averaging.h — Signal Averaging & Noise Reduction

**Header:** [`include/averaging.h`](../include/averaging.h)
| **Source:** [`src/averaging.c`](../src/averaging.c)
| **Tutorial:** [Ch 21 — Signal Averaging](../chapters/21-signal-averaging/tutorial.md)

### Functions (5)

| Function | Description |
|----------|-------------|
| `coherent_average(trials, K, n, out)` | Average K aligned sweeps (SNR × √K) |
| `ema_filter(x, n, alpha, y)` | Exponential moving average |
| `moving_average(x, n, M, y)` | Simple M-point moving average |
| `median_filter(x, n, M, y)` | Median filter (removes impulse noise) |
| `compute_snr_improvement(x_noisy, x_clean, n)` | Measure SNR gain |

---

## 15. remez.h — Parks-McClellan Equiripple FIR

**Header:** [`include/remez.h`](../include/remez.h)
| **Source:** [`src/remez.c`](../src/remez.c)
| **Tutorial:** [Ch 22 — Advanced FIR](../chapters/22-advanced-fir/tutorial.md)

### Functions (3)

| Function | Description |
|----------|-------------|
| `remez_fir(h, taps, bands, n_bands, max_iter)` | General multiband equiripple FIR |
| `remez_lowpass(h, taps, fpass, fstop, max_iter)` | Convenience lowpass wrapper |
| `remez_bandpass(h, taps, f1, f2, f3, f4, max_iter)` | Convenience bandpass wrapper |

---

## 16. adaptive.h — Adaptive Filters (LMS / NLMS / RLS)

**Header:** [`include/adaptive.h`](../include/adaptive.h)
| **Source:** [`src/adaptive.c`](../src/adaptive.c)
| **Tutorial:** [Ch 23 — Adaptive Filters](../chapters/23-adaptive-filters/tutorial.md)

### Functions (12)

| Group | Functions | Description |
|-------|-----------|-------------|
| LMS | `lms_init / lms_update / lms_free` | Least Mean Squares |
| NLMS | `nlms_init / nlms_update / nlms_free` | Normalised LMS |
| RLS | `rls_init / rls_update / rls_free` | Recursive Least Squares |
| Block | `lms_filter / nlms_filter / rls_filter` | Full-signal convenience wrappers |

---

## 17. multirate.h — Decimation & Interpolation

**Header:** [`include/multirate.h`](../include/multirate.h)
| **Source:** [`src/multirate.c`](../src/multirate.c)
| **Tutorial:** [Ch 17 — Multirate DSP](../chapters/17-multirate-dsp/tutorial.md)

### Functions (4)

| Function | Description |
|----------|-------------|
| `decimate(x, n, M, y)` | Downsample by M with anti-alias LPF |
| `interpolate(x, n, L, y)` | Upsample by L with anti-image LPF |
| `resample(x, n, L, M, y)` | Rational L/M rate conversion |
| `polyphase_decimate(x, n, h, h_len, M, y)` | Efficient polyphase decimation |

---

## 18. streaming.h — Overlap-Add/Save Block Convolution

**Header:** [`include/streaming.h`](../include/streaming.h)
| **Source:** [`src/streaming.c`](../src/streaming.c)
| **Tutorial:** [Ch 16 — Overlap-Add/Save](../chapters/16-overlap-add-save/tutorial.md)

### Functions (6)

| Group | Functions | Description |
|-------|-----------|-------------|
| OLA | `ola_init / ola_process / ola_free` | Overlap-Add block convolution |
| OLS | `ols_init / ols_process / ols_free` | Overlap-Save block convolution |

---

## 19. fixed_point.h — Q15/Q31 Arithmetic

**Header:** [`include/fixed_point.h`](../include/fixed_point.h)
| **Source:** [`src/fixed_point.c`](../src/fixed_point.c)
| **Tutorial:** [Ch 18 — Fixed-Point](../chapters/18-fixed-point/tutorial.md)

### Functions (16)

| Category | Functions | Description |
|----------|-----------|-------------|
| Q15 | `double_to_q15 / q15_to_double` | Conversion |
| Q15 | `q15_add / q15_sub / q15_mul / q15_neg / q15_abs` | Saturating arithmetic |
| Q31 | `double_to_q31 / q31_to_double / q31_add / q31_sub / q31_mul` | 32-bit variants |
| Array | `double_array_to_q15 / q15_array_to_double` | Bulk conversion |
| Filter | `fir_filter_q15(x, y, n, h, taps)` | Fixed-point FIR |
| Quality | `compute_sqnr(ref, quant, n)` | Signal-to-quantisation noise ratio |

---

## 20. dsp2d.h — 2-D DSP & Image Processing

**Header:** [`include/dsp2d.h`](../include/dsp2d.h)
| **Source:** [`src/dsp2d.c`](../src/dsp2d.c)
| **Tutorial:** [Ch 27 — 2-D DSP](../chapters/27-2d-dsp/tutorial.md)

### Functions (10)

| Function | Description |
|----------|-------------|
| `conv2d(img, rows, cols, kernel, ksize, out)` | 2-D convolution |
| `kernel_gaussian(kernel, ksize, sigma)` | Gaussian blur kernel |
| `kernel_sobel(gx, gy)` | 3×3 Sobel edge kernels |
| `kernel_log(kernel, ksize, sigma)` | Laplacian-of-Gaussian |
| `kernel_sharpen(kernel, alpha)` | Unsharp masking kernel |
| `sobel_magnitude(img, rows, cols, mag)` | Edge magnitude √(Gx²+Gy²) |
| `fft2d / ifft2d` | 2-D forward/inverse FFT |
| `filter2d_freq(img, rows, cols, H, out)` | Frequency-domain filtering |
| `lpf2d_ideal(H_re, H_im, rows, cols, cutoff)` | Ideal 2-D LPF |

---

## 21. realtime.h — Ring Buffer, Frame Processor, Latency

**Header:** [`include/realtime.h`](../include/realtime.h)
| **Source:** [`src/realtime.c`](../src/realtime.c)
| **Tutorial:** [Ch 28 — Real-Time Streaming](../chapters/28-real-time-streaming/tutorial.md)

### Ring Buffer (9 functions)

| Function | Description |
|----------|-------------|
| `ring_buffer_create(capacity)` / `ring_buffer_destroy(rb)` | Lifecycle |
| `ring_buffer_write(rb, data, n)` / `ring_buffer_read(rb, data, n)` | Data I/O |
| `ring_buffer_peek(rb, data, n)` / `ring_buffer_skip(rb, n)` | Non-destructive read |
| `ring_buffer_available(rb)` / `ring_buffer_space(rb)` | Status |
| `ring_buffer_reset(rb)` | Flush |

### Frame Processor (5 functions)

| Function | Description |
|----------|-------------|
| `frame_processor_create(frame_size, hop_size)` | Create windowed FFT processor |
| `frame_processor_destroy(fp)` | Release memory |
| `frame_processor_feed(fp, samples, n)` | Feed samples, returns frames processed |
| `frame_processor_peak_bin(fp)` | Bin index of spectral peak |
| `frame_processor_peak_freq(fp, fs)` | Peak frequency in Hz |

### Latency Measurement (4 functions)

`timer_usec()` · `latency_init(ls)` · `latency_record(ls, us)` · `latency_avg(ls)`

---

## 22. optimization.h — Radix-4 FFT, Twiddles, Benchmarks

**Header:** [`include/optimization.h`](../include/optimization.h)
| **Source:** [`src/optimization.c`](../src/optimization.c)
| **Tutorial:** [Ch 29 — Optimisation](../chapters/29-optimisation/tutorial.md)

### Functions (10)

| Category | Function | Description |
|----------|----------|-------------|
| FFT | `fft_radix4(x, n)` / `ifft_radix4(x, n)` | Radix-4 FFT (~25% fewer muls) |
| Twiddle | `twiddle_create(n)` / `twiddle_destroy(tt)` | Pre-computed twiddle table |
| Twiddle | `fft_with_twiddles(x, n, tt)` | FFT using cached twiddles |
| Memory | `aligned_alloc_dsp(alignment, size)` / `aligned_free_dsp(ptr)` | 64-byte cache-aligned alloc |
| Bench | `bench_fft_radix2(n, runs)` / `bench_fft_radix4(n, runs)` | Timing with MFLOP/s |
| Bench | `bench_print(label, result)` | Pretty-print benchmark results |

---

## 23. gnuplot.h — Plot Generation

**Header:** [`include/gnuplot.h`](../include/gnuplot.h)
| **Source:** [`src/gnuplot.c`](../src/gnuplot.c)
| **Used by:** `generate_plots` tool and all chapter demos

### Functions (8)

| Function | Description |
|----------|-------------|
| `gp_init(chapter)` | Create output directory for chapter |
| `gp_open(chapter, name, w, h)` | Open gnuplot pipe → PNG |
| `gp_close(gp)` | Close pipe |
| `gp_send_y(gp, y, n)` | Send y-values (auto x-axis) |
| `gp_send_xy(gp, x, y, n)` | Send (x,y) pairs |
| `gp_plot_1(chapter, name, title, xlabel, ylabel, y, n)` | One-liner single plot |
| `gp_plot_multi(chapter, name, title, xlabel, ylabel, ...)` | Multi-trace plot |
| `gp_plot_spectrum(chapter, name, title, mag, n, fs)` | Magnitude spectrum plot |

---

## Compilation & Linking

### Build with Make

```bash
make              # Debug build (-g -Wall -Wextra -Werror -std=c99)
make release      # Optimised build (-O3 -DNDEBUG)
make test         # Build + run all 98 tests
make clean        # Remove build artefacts
```

### Link Your Application

Against the static library (recommended):

```bash
make release
cc -Iinclude -o my_app my_app.c build/lib/libdsp_core.a -lm
```

Or compile specific modules:

```bash
cc -Iinclude -o my_app my_app.c src/fft.c src/dsp_utils.c -lm
```

### CMake

```cmake
add_subdirectory(dsp-tutorial-suite)
target_link_libraries(my_app PRIVATE dsp_core_static)
```

---

## See Also

- [ARCHITECTURE.md](ARCHITECTURE.md) — System design, module dependencies, 23-module inventory
- [CHAPTER_INDEX.md](CHAPTER_INDEX.md) — Chapter-by-chapter quick reference
- [chapters/](../chapters/00-overview/README.md) — Progressive learning chapters
- [diagrams/](diagrams/) — PlantUML diagrams (4 common + 31 chapter-specific)
