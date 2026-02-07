# Chapter Index — DSP Tutorial Suite

Quick-reference linking each chapter to its tutorial, demo, library modules, plots,
and test coverage.

> Rebuild all demos with `make release`. Run tests with `make test`.

---

## Part I — Foundations

| Ch | Tutorial | Demo | Library | Plots | Tests |
|----|----------|------|---------|-------|-------|
| [01](../chapters/01-signals-and-sequences/) | Discrete-time signals | [demo.c](../chapters/01-signals-and-sequences/demo.c) | [`signal_gen.h`](../include/signal_gen.h) | [plots/](../chapters/01-signals-and-sequences/plots/) (5 PNGs) | — |
| [02](../chapters/02-sampling-and-aliasing/) | Sampling & aliasing | [demo.c](../chapters/02-sampling-and-aliasing/demo.c) | — | [plots/](../chapters/02-sampling-and-aliasing/plots/) (3 PNGs) | — |
| [03](../chapters/03-complex-numbers/) | Complex numbers | [demo.c](../chapters/03-complex-numbers/demo.c) | [`dsp_utils.h`](../include/dsp_utils.h) | [plots/](../chapters/03-complex-numbers/plots/) (1 PNG) | — |
| [04](../chapters/04-lti-systems/) | LTI systems | [demo.c](../chapters/04-lti-systems/demo.c) | [`convolution.h`](../include/convolution.h) | [plots/](../chapters/04-lti-systems/plots/) (2 PNGs) | — |

## Part II — Transform Domain

| Ch | Tutorial | Demo | Library | Plots | Tests |
|----|----------|------|---------|-------|-------|
| [05](../chapters/05-z-transform/) | Z-Transform | [demo.c](../chapters/05-z-transform/demo.c) | — | [plots/](../chapters/05-z-transform/plots/) (2 PNGs) | — |
| [06](../chapters/06-frequency-response/) | Frequency response | [demo.c](../chapters/06-frequency-response/demo.c) | [`iir.h`](../include/iir.h) | [plots/](../chapters/06-frequency-response/plots/) (4 PNGs) | — |
| [07](../chapters/07-dft-theory/) | DFT theory | [demo.c](../chapters/07-dft-theory/demo.c) | — | [plots/](../chapters/07-dft-theory/plots/) (3 PNGs) | — |
| [08](../chapters/08-fft-fundamentals/) | FFT algorithms | [demo.c](../chapters/08-fft-fundamentals/demo.c) | [`fft.h`](../include/fft.h) | [plots/](../chapters/08-fft-fundamentals/plots/) (1 PNG) | `test_fft` (6) |
| [09](../chapters/09-window-functions/) | Window functions | [demo.c](../chapters/09-window-functions/demo.c) | [`dsp_utils.h`](../include/dsp_utils.h) | [plots/](../chapters/09-window-functions/plots/) (2 PNGs) | — |

## Part III — Filter Design

| Ch | Tutorial | Demo | Library | Plots | Tests |
|----|----------|------|---------|-------|-------|
| [10](../chapters/10-digital-filters/) | FIR filter design | [demo.c](../chapters/10-digital-filters/demo.c) | [`filter.h`](../include/filter.h) | [plots/](../chapters/10-digital-filters/plots/) (2 PNGs) | `test_filter` (6) |
| [11](../chapters/11-iir-filter-design/) | IIR filter design | [demo.c](../chapters/11-iir-filter-design/demo.c) | [`iir.h`](../include/iir.h) | [plots/](../chapters/11-iir-filter-design/plots/) (3 PNGs) | `test_iir` (10) |
| [12](../chapters/12-filter-structures/) | Filter structures | [demo.c](../chapters/12-filter-structures/demo.c) | — | [plots/](../chapters/12-filter-structures/plots/) (2 PNGs) | — |

## Part IV — Analysis

| Ch | Tutorial | Demo | Library | Plots | Tests |
|----|----------|------|---------|-------|-------|
| [13](../chapters/13-spectral-analysis/) | Spectral analysis | [demo.c](../chapters/13-spectral-analysis/demo.c) | [`spectrum.h`](../include/spectrum.h) | [plots/](../chapters/13-spectral-analysis/plots/) (1 PNG) | — |
| [14](../chapters/14-psd-welch/) | PSD & Welch's method | [demo.c](../chapters/14-psd-welch/demo.c) | [`spectrum.h`](../include/spectrum.h) | [plots/](../chapters/14-psd-welch/plots/) (7 PNGs) | `test_spectrum_corr` (12) |
| [15](../chapters/15-correlation/) | Correlation | [demo.c](../chapters/15-correlation/demo.c) | [`correlation.h`](../include/correlation.h) | [plots/](../chapters/15-correlation/plots/) (5 PNGs) | `test_spectrum_corr` (12) |

## Part V — Advanced UG

| Ch | Tutorial | Demo | Library | Plots | Tests |
|----|----------|------|---------|-------|-------|
| [16](../chapters/16-overlap-add-save/) | Overlap-Add/Save | [demo.c](../chapters/16-overlap-add-save/demo.c) | [`streaming.h`](../include/streaming.h) | [plots/](../chapters/16-overlap-add-save/plots/) (3 PNGs) | `test_phase4` (12) |
| [17](../chapters/17-multirate-dsp/) | Multirate DSP | [demo.c](../chapters/17-multirate-dsp/demo.c) | [`multirate.h`](../include/multirate.h) | [plots/](../chapters/17-multirate-dsp/plots/) (3 PNGs) | `test_phase5` (15) |
| [18](../chapters/18-fixed-point/) | Fixed-point arithmetic | [demo.c](../chapters/18-fixed-point/demo.c) | [`fixed_point.h`](../include/fixed_point.h) | [plots/](../chapters/18-fixed-point/plots/) (5 PNGs) | `test_phase4` (12) |
| [19](../chapters/19-advanced-fft/) | Advanced FFT | [demo.c](../chapters/19-advanced-fft/demo.c) | [`advanced_fft.h`](../include/advanced_fft.h) | [plots/](../chapters/19-advanced-fft/plots/) (3 PNGs) | `test_phase4` (12) |
| [20](../chapters/20-hilbert-transform/) | Hilbert transform | [demo.c](../chapters/20-hilbert-transform/demo.c) | [`hilbert.h`](../include/hilbert.h) | [plots/](../chapters/20-hilbert-transform/plots/) (2 PNGs) | `test_phase5` (15) |
| [21](../chapters/21-signal-averaging/) | Signal averaging | [demo.c](../chapters/21-signal-averaging/demo.c) | [`averaging.h`](../include/averaging.h) | [plots/](../chapters/21-signal-averaging/plots/) (3 PNGs) | `test_phase5` (15) |
| [22](../chapters/22-advanced-fir/) | Advanced FIR (Remez) | [demo.c](../chapters/22-advanced-fir/demo.c) | [`remez.h`](../include/remez.h) | [plots/](../chapters/22-advanced-fir/plots/) (2 PNGs) | `test_phase5` (15) |

## Part VI — Postgraduate

| Ch | Tutorial | Demo | Library | Plots | Tests |
|----|----------|------|---------|-------|-------|
| [23](../chapters/23-adaptive-filters/) | Adaptive filters | [demo.c](../chapters/23-adaptive-filters/demo.c) | [`adaptive.h`](../include/adaptive.h) | [plots/](../chapters/23-adaptive-filters/plots/) (2 PNGs) | `test_phase6` (19) |
| [24](../chapters/24-linear-prediction/) | Linear prediction | [demo.c](../chapters/24-linear-prediction/demo.c) | [`lpc.h`](../include/lpc.h) | [plots/](../chapters/24-linear-prediction/plots/) (2 PNGs) | `test_phase6` (19) |
| [25](../chapters/25-parametric-spectral/) | Parametric spectral | [demo.c](../chapters/25-parametric-spectral/demo.c) | [`spectral_est.h`](../include/spectral_est.h) | [plots/](../chapters/25-parametric-spectral/plots/) (2 PNGs) | `test_phase6` (19) |
| [26](../chapters/26-cepstrum-mfcc/) | Cepstrum & MFCC | [demo.c](../chapters/26-cepstrum-mfcc/demo.c) | [`cepstrum.h`](../include/cepstrum.h) | [plots/](../chapters/26-cepstrum-mfcc/plots/) (3 PNGs) | `test_phase6` (19) |
| [27](../chapters/27-2d-dsp/) | 2-D DSP | [demo.c](../chapters/27-2d-dsp/demo.c) | [`dsp2d.h`](../include/dsp2d.h) | [plots/](../chapters/27-2d-dsp/plots/) (2 PNGs) | `test_phase6` (19) |

## Part VII — Applied / Capstone

| Ch | Tutorial | Demo | Library | Plots | Tests |
|----|----------|------|---------|-------|-------|
| [28](../chapters/28-real-time-streaming/) | Real-time streaming | [demo.c](../chapters/28-real-time-streaming/demo.c) | [`realtime.h`](../include/realtime.h) | [plots/](../chapters/28-real-time-streaming/plots/) (3 PNGs) | `test_phase7` (18) |
| [29](../chapters/29-optimisation/) | DSP optimisation | [demo.c](../chapters/29-optimisation/demo.c) | [`optimization.h`](../include/optimization.h) | [plots/](../chapters/29-optimisation/plots/) (3 PNGs) | `test_phase7` (18) |
| [30](../chapters/30-putting-it-together/) | Capstone pipeline | [demo.c](../chapters/30-putting-it-together/demo.c) | All (13 modules) | [plots/](../chapters/30-putting-it-together/plots/) (2 PNGs) | — |

---

## System-Level Documentation

| Document | Description |
|----------|-------------|
| [ARCHITECTURE.md](ARCHITECTURE.md) | Layered system design, module dependencies |
| [API.md](API.md) | Complete public function reference |
| [diagrams/](diagrams/) | 4 common PlantUML diagrams + 4 chapter-specific |

## Test Summary (98 tests)

| Suite | File | Count |
|-------|------|-------|
| FFT | [`test_fft.c`](../tests/test_fft.c) | 6 |
| Filter | [`test_filter.c`](../tests/test_filter.c) | 6 |
| IIR | [`test_iir.c`](../tests/test_iir.c) | 10 |
| Spectrum & Correlation | [`test_spectrum_corr.c`](../tests/test_spectrum_corr.c) | 12 |
| Phase 4 | [`test_phase4.c`](../tests/test_phase4.c) | 12 |
| Phase 5 | [`test_phase5.c`](../tests/test_phase5.c) | 15 |
| Phase 6 | [`test_phase6.c`](../tests/test_phase6.c) | 19 |
| Phase 7 | [`test_phase7.c`](../tests/test_phase7.c) | 18 |
| **Total** | | **98** |
