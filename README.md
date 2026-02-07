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
| [00](chapters/00-overview.md) | Project overview & build system | — | — |
| [01](chapters/01-signals-and-sequences.md) | Discrete-time signals & sequences | `ch01` | [`signal_gen.h`](include/signal_gen.h) |
| [02](chapters/02-sampling-and-aliasing.md) | Sampling, aliasing & Nyquist theorem | `ch02` | — |
| [03](chapters/03-complex-numbers.md) | Complex numbers & Euler's formula | `ch03` | [`dsp_utils.h`](include/dsp_utils.h) |
| [04](chapters/04-lti-systems.md) | LTI systems & discrete convolution | `ch04` | [`convolution.h`](include/convolution.h) |

### Part II — Transform Domain

| Ch | Topic | Demo | Key Files |
|----|-------|------|-----------|
| [05](chapters/05-z-transform.md) | The Z-Transform | `ch05` | — |
| 06 | Frequency response, poles & zeros | — | *planned* |
| [07](chapters/07-dft-theory.md) | The DFT — theory & properties | `ch07` | — |
| [08](chapters/08-fft-fundamentals.md) | FFT algorithms (Cooley-Tukey Radix-2) | `ch08` | [`fft.h`](include/fft.h), [`fft.c`](src/fft.c) |
| [09](chapters/09-window-functions.md) | Window functions & spectral leakage | `ch09` | [`dsp_utils.c`](src/dsp_utils.c) |

### Part III — Filter Design

| Ch | Topic | Demo | Key Files |
|----|-------|------|-----------|
| [10](chapters/10-digital-filters.md) | FIR filter design | `ch10` | [`filter.h`](include/filter.h), [`filter.c`](src/filter.c) |
| 11 | IIR filter design | — | *planned* |
| 12 | Filter structures (biquads, SOS) | — | *planned* |

### Part IV — Analysis

| Ch | Topic | Demo | Key Files |
|----|-------|------|-----------|
| [13](chapters/13-spectral-analysis.md) | Spectral analysis | `ch13` | — |
| 14 | Power spectral density (Welch) | — | *planned* |
| 15 | Correlation & autocorrelation | — | *planned* |

### Part V — Advanced (UG Final Year)

| Ch | Topic | Status |
|----|-------|--------|
| 16 | Overlap-add/save & streaming | 🔜 |
| 17 | Multirate DSP | 🔜 |
| 18 | Fixed-point arithmetic & quantization | 🔜 |
| 19 | Advanced FFT (Goertzel, Radix-4) | 🔜 |
| 20 | Quadrature signals & Hilbert transform | 🔜 |
| 21 | Signal averaging & noise reduction | 🔜 |
| 22 | Advanced FIR (Parks-McClellan) | 🔜 |

### Part VI — Postgraduate

| Ch | Topic | Status |
|----|-------|--------|
| 23 | Adaptive filters (LMS / RLS) | 🔜 |
| 24 | Linear prediction & parametric modelling | 🔜 |
| 25 | Parametric spectral estimation (MUSIC, ESPRIT) | 🔜 |
| 26 | Cepstrum analysis & MFCCs | 🔜 |

### Part VII — Applied / Capstone

| Ch | Topic | Demo | Status |
|----|-------|------|--------|
| 27 | 2D DSP & image processing | — | 🔜 |
| [28](chapters/28-real-time-streaming.md) | Real-time system design | — | 📋 design |
| [29](chapters/29-optimisation.md) | SIMD & hardware optimisation | — | 📋 design |
| [30](chapters/30-putting-it-together.md) | End-to-end projects | `ch30` | ✅ |

## Quick Start

```bash
# Clone
git clone git@github.com:nikhiloz/dsp-tutorial-suite.git
cd dsp-tutorial-suite

# Build everything (C99, no external deps)
make release

# Run a specific chapter demo
./build/bin/ch01    # Signals & sequences
./build/bin/ch07    # DFT theory
./build/bin/ch08    # FFT fundamentals

# Run the test suite (12 tests)
make test

# Run all chapter demos
make run
```

### Requirements

- GCC or Clang with C99 support
- GNU Make
- Linux / macOS (POSIX)
- *Optional*: Java 11+ and [PlantUML](https://plantuml.com) to regenerate diagrams

## Project Layout

```
dsp-tutorial-suite/
├── include/          ← Public headers (start reading here)
│   ├── dsp_utils.h       Complex type, windows, helpers
│   ├── fft.h             FFT / IFFT API
│   ├── filter.h          FIR filter API
│   ├── signal_gen.h      Signal generation (sine, noise, chirp)
│   └── convolution.h     Convolution & correlation
├── src/              ← Implementations (heavily commented)
│   ├── dsp_utils.c       Complex arithmetic, 3 window functions
│   ├── fft.c             Cooley-Tukey Radix-2 DIT
│   ├── filter.c          Direct convolution + sinc design
│   ├── signal_gen.c      Signal generators (Box-Muller noise, chirp)
│   └── convolution.c     Linear/causal conv, cross/auto-correlation
├── tests/            ← Unit tests (zero-dependency framework)
│   ├── test_framework.h
│   ├── test_fft.c        6 FFT tests
│   └── test_filter.c     6 FIR filter tests
├── chapters/         ← START HERE — progressive tutorial (30 chapters)
├── reference/        ← Architecture, API reference, diagrams
│   ├── ARCHITECTURE.md
│   ├── API.md
│   └── diagrams/     PlantUML sources + rendered PNGs
├── Makefile          ← Primary build (15+ targets)
└── CMakeLists.txt    ← Cross-platform alternative
```

## Diagrams

All architecture diagrams live in [`reference/diagrams/`](reference/diagrams/). They are
rendered from PlantUML source as high-resolution PNGs. Click any link below to
view the full-size image:

| Diagram | Description |
|---------|-------------|
| [System Architecture](reference/diagrams/architecture.png) | Four-layer system design |
| [Signal Flow](reference/diagrams/signal_flow.png) | Time → Frequency domain pipeline |
| [Module Dependencies](reference/diagrams/modules.png) | Source file dependency graph |
| [FFT Sequence](reference/diagrams/fft_sequence.png) | Runtime call sequence |
| [Real-Time Architecture](reference/diagrams/realtime_architecture.png) | Streaming pipeline |
| [Optimisation Roadmap](reference/diagrams/optimization_roadmap.png) | 5-stage speedup plan |
| [API Reference](reference/diagrams/api_reference.png) | Public function map |
| [Project Roadmap](reference/diagrams/roadmap.png) | 6-phase development plan |
| [Benchmarks](reference/diagrams/benchmarks.png) | Latency comparison |
| [Use Cases](reference/diagrams/use_cases.png) | Target applications |

To regenerate PNGs after editing `.puml` files:

```bash
java -jar ~/tools/plantuml.jar -tpng reference/diagrams/*.puml
```

## Test Output

```
=== Test Suite: FFT Functions ===
  [TEST] DC component (constant signal) .............. PASS
  [TEST] Impulse gives flat spectrum .................. PASS
  [TEST] Alternating signal → Nyquist bin ............. PASS
  [TEST] FFT then IFFT recovers original ............. PASS
  [TEST] Pure sine wave: peaks at correct bin ......... PASS
  [TEST] fft_real matches manual complex FFT .......... PASS
Total: 6, Passed: 6, Failed: 0

=== Test Suite: Filter Functions ===
  [TEST] Identity filter (1-tap passthrough) .......... PASS
  [TEST] Zero input gives zero output ................. PASS
  [TEST] Impulse response matches coefficients ........ PASS
  [TEST] Moving average smooths step input ............ PASS
  [TEST] Lowpass filter coefficients sum to 1.0 ....... PASS
  [TEST] Lowpass attenuates high frequency ............ PASS
Total: 6, Passed: 6, Failed: 0
```

## License

MIT — see [LICENSE](LICENSE).

## References

- Oppenheim & Willsky, *Signals and Systems* (3rd ed.)
- Oppenheim & Schafer, *Discrete-Time Signal Processing* (3rd ed.)
- Proakis & Manolakis, *Digital Signal Processing* (4th ed.)
- Haykin, *Adaptive Filter Theory* (5th ed.)
- Lyons, *Understanding DSP* (3rd ed.)
- Smith, *The Scientist and Engineer's Guide to DSP* (free online)
