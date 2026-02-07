# DSP Tutorial Suite

**A comprehensive C tutorial for Digital Signal Processing — from first principles to postgraduate topics.**

This repository is a progressive learning resource covering 30 chapters of DSP, from
discrete-time signals through adaptive filters. Every source file is written to be
*read*, with detailed comments that cross-reference tutorial chapters, textbook
sections, and diagrams. Zero external dependencies — just C99 and `math.h`.

---

## What You'll Learn

### Part I — Foundations

| Ch | Topic | Demo | Key Files |
|----|-------|------|-----------|
| [01](chapters/01-signals-and-sequences/) | Discrete-time signals & sequences | `ch01` | [`signal_gen.h`](include/signal_gen.h) |
| [02](chapters/02-sampling-and-aliasing/) | Sampling, aliasing & Nyquist theorem | `ch02` | — |
| [03](chapters/03-complex-numbers/) | Complex numbers & Euler's formula | `ch03` | [`dsp_utils.h`](include/dsp_utils.h) |
| [04](chapters/04-lti-systems/) | LTI systems & discrete convolution | `ch04` | [`convolution.h`](include/convolution.h) |

### Part II — Transform Domain

| Ch | Topic | Demo | Key Files |
|----|-------|------|-----------|
| [05](chapters/05-z-transform/) | The Z-Transform | `ch05` | — |
| [06](chapters/06-frequency-response/) | Frequency response, poles & zeros | `ch06` | [`iir.h`](include/iir.h) |
| [07](chapters/07-dft-theory/) | The DFT — theory & properties | `ch07` | — |
| [08](chapters/08-fft-fundamentals/) | FFT algorithms (Cooley-Tukey Radix-2) | `ch08` | [`fft.h`](include/fft.h) |
| [09](chapters/09-window-functions/) | Window functions & spectral leakage | `ch09` | [`dsp_utils.h`](include/dsp_utils.h) |

### Part III — Filter Design

| Ch | Topic | Demo | Key Files |
|----|-------|------|-----------|
| [10](chapters/10-digital-filters/) | FIR filter design | `ch10` | [`filter.h`](include/filter.h) |
| [11](chapters/11-iir-filter-design/) | IIR filter design (Butterworth, Chebyshev) | `ch11` | [`iir.h`](include/iir.h) |
| [12](chapters/12-filter-structures/) | Filter structures (biquads, SOS cascades) | `ch12` | — |

### Part IV — Analysis

| Ch | Topic | Demo | Key Files |
|----|-------|------|-----------|
| [13](chapters/13-spectral-analysis/) | Spectral analysis | `ch13` | [`spectrum.h`](include/spectrum.h) |
| [14](chapters/14-psd-welch/) | Power spectral density (Welch's method) | `ch14` | [`spectrum.h`](include/spectrum.h) |
| [15](chapters/15-correlation/) | Correlation & autocorrelation | `ch15` | [`correlation.h`](include/correlation.h) |

### Part V — C-Specific & Advanced UG

| Ch | Topic | Demo | Key Files |
|----|-------|------|-----------|
| [16](chapters/16-overlap-add-save/) | Overlap-Add/Save streaming convolution | `ch16` | [`streaming.h`](include/streaming.h) |
| [17](chapters/17-multirate-dsp/) | Multirate DSP (decimation, interpolation, polyphase) | `ch17` | [`multirate.h`](include/multirate.h) |
| [18](chapters/18-fixed-point/) | Fixed-point arithmetic (Q15/Q31, SQNR) | `ch18` | [`fixed_point.h`](include/fixed_point.h) |
| [19](chapters/19-advanced-fft/) | Advanced FFT (Goertzel, DTMF, Sliding DFT) | `ch19` | [`advanced_fft.h`](include/advanced_fft.h) |
| [20](chapters/20-hilbert-transform/) | Quadrature signals & Hilbert transform | `ch20` | [`hilbert.h`](include/hilbert.h) |
| [21](chapters/21-signal-averaging/) | Signal averaging & noise reduction | `ch21` | [`averaging.h`](include/averaging.h) |
| [22](chapters/22-advanced-fir/) | Advanced FIR design (Parks-McClellan / IRLS) | `ch22` | [`remez.h`](include/remez.h) |

### Part VI — Postgraduate

| Ch | Topic | Demo | Key Files |
|----|-------|------|----------|
| [23](chapters/23-adaptive-filters/) | Adaptive filters (LMS / NLMS / RLS) | `ch23` | [`adaptive.h`](include/adaptive.h) |
| [24](chapters/24-linear-prediction/) | Linear prediction & AR modelling | `ch24` | [`lpc.h`](include/lpc.h) |
| [25](chapters/25-parametric-spectral/) | Parametric spectral estimation (MUSIC, Capon) | `ch25` | [`spectral_est.h`](include/spectral_est.h) |
| [26](chapters/26-cepstrum-mfcc/) | Cepstrum analysis & MFCCs | `ch26` | [`cepstrum.h`](include/cepstrum.h) |

### Part VII — Applied / Capstone

| Ch | Topic | Demo | Key Files |
|----|-------|------|----------|
| [27](chapters/27-2d-dsp/) | 2-D DSP & image processing | `ch27` | [`dsp2d.h`](include/dsp2d.h) |
| [28](chapters/28-real-time-streaming/) | Real-time system design | `ch28` | [`realtime.h`](include/realtime.h) |
| [29](chapters/29-optimisation/) | DSP optimisation (radix-4, twiddles, aligned mem) | `ch29` | [`optimization.h`](include/optimization.h) |
| [30](chapters/30-putting-it-together/) | End-to-end capstone project | `ch30` | all (13 modules) |

## Quick Start

```bash
# Clone
git clone git@github.com:nikhiloz/dsp-tutorial-suite.git
cd dsp-tutorial-suite

# Build everything (C99, no external deps)
make release

# Run a specific chapter demo
./build/bin/ch01    # Signals & sequences
./build/bin/ch08    # FFT fundamentals
./build/bin/ch18    # Fixed-point arithmetic

# Run the test suite (98 tests across 8 suites)
make test

# Run all chapter demos
make run

# Generate all gnuplot visualisations
make plots
```

### Requirements

- GCC or Clang with C99 support
- GNU Make
- Linux / macOS (POSIX)
- *Optional*: gnuplot >= 5.0 for plot generation (`apt install gnuplot`)
- *Optional*: Java 11+ and [PlantUML](https://plantuml.com) to regenerate diagrams

## Project Layout

```
dsp-tutorial-suite/
├── chapters/         ← START HERE — 30 progressive tutorials
│   ├── 00-overview/
│   │   └── tutorial.md       Course overview
│   ├── 01-signals-and-sequences/
│   │   ├── README.md         Landing page (GitHub auto-renders)
│   │   ├── tutorial.md       Theory with equations & exercises
│   │   ├── demo.c            Self-contained runnable demo
│   │   └── plots/            Generated gnuplot PNGs
│   └── ...                   (31 chapter subdirectories)
│       Each contains: README.md, tutorial.md, demo.c, plots/,
│       <name>.puml + <name>.png (concept diagram)
├── include/          ← Public headers (23 modules)
│   ├── dsp_utils.h       Complex type, windows, helpers
│   ├── fft.h             FFT / IFFT API
│   ├── filter.h          FIR filter API
│   ├── iir.h             IIR filter design (Butterworth, Chebyshev)
│   ├── signal_gen.h      Signal generation (sine, noise, chirp)
│   ├── convolution.h     Convolution & correlation
│   ├── spectrum.h        PSD, Welch's method, cross-PSD
│   ├── correlation.h     Cross/auto-correlation (FFT-based)
│   ├── gnuplot.h         Pipe-based gnuplot plotting helpers
│   ├── fixed_point.h     Q15/Q31 fixed-point arithmetic
│   ├── advanced_fft.h    Goertzel, DTMF detection, sliding DFT
│   ├── streaming.h       Overlap-Add/Save block convolution
│   ├── multirate.h       Decimation, interpolation, polyphase
│   ├── hilbert.h         Hilbert transform, analytic signal
│   ├── averaging.h       Coherent averaging, EMA, median filter
│   ├── remez.h           Parks-McClellan / IRLS equiripple FIR
│   ├── adaptive.h        LMS, NLMS, RLS adaptive filters
│   ├── lpc.h             Linear prediction, Levinson-Durbin
│   ├── spectral_est.h    MUSIC, Capon parametric spectral est.
│   ├── cepstrum.h        Cepstrum, Mel filterbank, MFCCs
│   ├── dsp2d.h           2-D convolution, FFT, image kernels
│   ├── realtime.h        Ring buffer, frame processor, latency
│   └── optimization.h    Radix-4 FFT, twiddle tables, benchmarks
├── src/              ← Reusable library (builds to libdsp_core.a, 23 modules)
├── tests/            ← Unit tests (98 assertions, zero-dependency framework)
│   ├── test_framework.h  Lightweight test macros
│   ├── test_fft.c        6 FFT tests
│   ├── test_filter.c     6 FIR filter tests
│   ├── test_iir.c        10 IIR filter tests
│   ├── test_spectrum_corr.c  12 spectrum & correlation tests
│   ├── test_phase4.c     12 fixed-point, Goertzel, streaming tests
│   ├── test_phase5.c     15 multirate, Hilbert, averaging, Remez tests
│   ├── test_phase6.c     19 adaptive, LPC, spectral est, cepstrum, 2D tests
│   └── test_phase7.c     18 real-time, radix-4, twiddle, aligned memory tests
├── tools/            ← Utilities
│   └── generate_plots.c  Generates 70+ gnuplot PNGs for all chapters
├── reference/        ← Architecture, API reference, diagrams
│   ├── ARCHITECTURE.md
│   ├── CHAPTER_INDEX.md
│   ├── API.md
│   └── diagrams/     4 common PlantUML diagrams (31 chapter-specific in chapters/)
├── Makefile          ← Primary build (39 targets)
└── CMakeLists.txt    ← Cross-platform alternative
```

## Diagrams

Architecture diagrams are split between [`reference/diagrams/`](reference/diagrams/)
(common, cross-cutting) and individual chapter folders (chapter-specific).

### Common Diagrams

| Diagram | Description |
|---------|-------------|
| [System Architecture](reference/diagrams/architecture.png) | Four-layer system design |
| [Module Dependencies](reference/diagrams/modules.png) | 23-module dependency graph |
| [API Reference](reference/diagrams/api_reference.png) | ~150 public functions |
| [Use Cases](reference/diagrams/use_cases.png) | Target applications |

### Chapter-Specific Diagrams

Every chapter (00–30) includes its own conceptual PlantUML diagram.
Each chapter's `README.md` embeds the rendered PNG. Examples:

| Diagram | Location |
|---------|----------|
| [Course Overview](chapters/00-overview/course_overview.png) | Ch 00 — Learning path |
| [Signal Classification](chapters/01-signals-and-sequences/signal_classification.png) | Ch 01 — Signal taxonomy |
| [FFT Sequence](chapters/08-fft-fundamentals/fft_sequence.png) | Ch 08 — Cooley-Tukey call flow |
| [Adaptive Filter Loop](chapters/23-adaptive-filters/adaptive_loop.png) | Ch 23 — LMS/NLMS/RLS |
| [MFCC Pipeline](chapters/26-cepstrum-mfcc/mfcc_pipeline.png) | Ch 26 — Feature extraction |
| [Real-Time Architecture](chapters/28-real-time-streaming/realtime_architecture.png) | Ch 28 — Streaming pipeline |

To regenerate PNGs after editing `.puml` files:

```bash
java -jar ~/tools/plantuml.jar -tpng reference/diagrams/*.puml chapters/*/*.puml
```

## Test Output (98 tests)

```
=== Test Suite: FFT Functions ===
  Total: 6, Passed: 6, Failed: 0   (100%)

=== Test Suite: Filter Functions ===
  Total: 6, Passed: 6, Failed: 0   (100%)

=== Test Suite: IIR Filter Functions ===
  Total: 10, Passed: 10, Failed: 0 (100%)

=== Test Suite: Spectrum & Correlation ===
  Total: 12, Passed: 12, Failed: 0 (100%)

=== Test Suite: Phase 4: Fixed-Point, Advanced FFT, Streaming ===
  Total: 12, Passed: 12, Failed: 0 (100%)

=== Test Suite: Phase 5: Multirate, Hilbert, Averaging, Remez ===
  Total: 15, Passed: 15, Failed: 0 (100%)

=== Test Suite: Phase 6: Adaptive, LPC, Spectral Est, Cepstrum, 2D DSP ===
  Results: 19/19 passed             (100%)

=== Test Suite: Phase 7: Real-Time & Optimisation ===
  Results: 18/18 passed             (100%)
```

## License

MIT

## References

- Oppenheim & Willsky, *Signals and Systems* (3rd ed.)
- Oppenheim & Schafer, *Discrete-Time Signal Processing* (3rd ed.)
- Proakis & Manolakis, *Digital Signal Processing* (4th ed.)
- Haykin, *Adaptive Filter Theory* (5th ed.)
- Lyons, *Understanding DSP* (3rd ed.)
- Smith, *The Scientist and Engineer's Guide to DSP* (free online)
