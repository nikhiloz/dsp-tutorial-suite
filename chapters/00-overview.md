# Chapter 0 — Project Overview

Welcome to the **DSP Tutorial Suite**. This is a from-scratch
Digital Signal Processing library and tutorial written in C99 with no external
dependencies. Every source file is designed to be *read* as a learning
resource. The curriculum covers UG through PG-level DSP.

---

## How to Navigate This Tutorial

Each chapter follows a consistent pattern:

1. **Theory** — The math and intuition behind the concept
2. **Diagram** — A visual reference (click to view full-size)
3. **Implementation Walk-Through** — Line-by-line analysis of the C code
4. **Try It Yourself** — Commands to build and run the related demo
5. **Exercises** — Practice problems to deepen understanding
6. **Next Chapter** — Link to continue the learning path

## Chapter Map

### Part I — Foundations

| # | Chapter | Demo | Status |
|---|---------|------|--------|
| 01 | [Discrete-Time Signals & Sequences](01-signals-and-sequences.md) | `ch01` | ✅ |
| 02 | [Sampling, Aliasing & Nyquist](02-sampling-and-aliasing.md) | `ch02` | ✅ |
| 03 | [Complex Numbers & Euler's Formula](03-complex-numbers.md) | `ch03` | ✅ |
| 04 | [LTI Systems & Discrete Convolution](04-lti-systems.md) | `ch04` | ✅ |

### Part II — Transform Domain

| # | Chapter | Demo | Status |
|---|---------|------|--------|
| 05 | [The Z-Transform](05-z-transform.md) | `ch05` | ✅ |
| 06 | Frequency Response, Poles & Zeros | — | 🔜 |
| 07 | [The DFT — Theory & Properties](07-dft-theory.md) | `ch07` | ✅ |
| 08 | [FFT Algorithms — Cooley-Tukey Radix-2](08-fft-fundamentals.md) | `ch08` | ✅ |
| 09 | [Window Functions & Spectral Leakage](09-window-functions.md) | `ch09` | ✅ |

### Part III — Filter Design

| # | Chapter | Demo | Status |
|---|---------|------|--------|
| 10 | [FIR Filter Design](10-digital-filters.md) | `ch10` | ✅ |
| 11 | IIR Filter Design | — | 🔜 |
| 12 | Filter Structures (Biquads, SOS) | — | 🔜 |

### Part IV — Analysis

| # | Chapter | Demo | Status |
|---|---------|------|--------|
| 13 | [Spectral Analysis](13-spectral-analysis.md) | `ch13` | ✅ |
| 14 | Power Spectral Density (Welch) | — | 🔜 |
| 15 | Correlation & Autocorrelation | — | 🔜 |

### Part V — Advanced Topics

| # | Chapter | Status |
|---|---------|--------|
| 16 | Overlap-Add/Save & Streaming | 🔜 |
| 17 | Multirate DSP | 🔜 |
| 18 | Fixed-Point Arithmetic | 🔜 |
| 19 | Advanced FFT (Goertzel, Radix-4) | 🔜 |
| 20 | Quadrature Signals & Hilbert Transform | 🔜 |
| 21 | Signal Averaging & Noise Reduction | 🔜 |
| 22 | Advanced FIR (Parks-McClellan) | 🔜 |

### Part VI — Postgraduate

| # | Chapter | Status |
|---|---------|--------|
| 23 | Adaptive Filters (LMS/RLS) | 🔜 |
| 24 | Linear Prediction | 🔜 |
| 25 | Parametric Spectral Estimation | 🔜 |
| 26 | Cepstrum & MFCCs | 🔜 |

### Part VII — Applied / Capstone

| # | Chapter | Demo | Status |
|---|---------|------|--------|
| 27 | 2D DSP & Image Processing | — | 🔜 |
| 28 | [Real-Time System Design](28-real-time-streaming.md) | — | 📋 Design |
| 29 | [SIMD & Hardware Optimisation](29-optimisation.md) | — | 📋 Design |
| 30 | [End-to-End Projects](30-putting-it-together.md) | `ch30` | ✅ |

## Project Structure

```
dsp-tutorial-suite/
├── include/            ← Start reading the PUBLIC API here
│   ├── dsp_utils.h         Complex type + windows + helpers
│   ├── fft.h               FFT/IFFT API
│   ├── filter.h            FIR filter API
│   ├── signal_gen.h        Signal generation (sine, noise, chirp, etc.)
│   └── convolution.h       Convolution & correlation
│
├── src/                ← Then dig into the IMPLEMENTATIONS
│   ├── dsp_utils.c         Complex arithmetic, 3 window functions
│   ├── fft.c               Cooley-Tukey Radix-2 DIT (~185 lines)
│   ├── filter.c            FIR convolution + windowed-sinc design
│   ├── signal_gen.c        Signal generators (Box-Muller noise, chirp)
│   └── convolution.c       Linear/causal conv, cross/auto-correlation
│
├── tests/              ← Verify correctness
│   ├── test_framework.h    Zero-dependency test macros
│   ├── test_fft.c          6 FFT tests
│   └── test_filter.c       6 FIR filter tests
│
├── chapters/           ← YOU ARE HERE — start with 00-overview.md
├── reference/          ← Architecture docs + diagrams
│   ├── ARCHITECTURE.md
│   ├── API.md
│   └── diagrams/       PlantUML sources + PNG renders
│
├── Makefile            ← GNU Make (primary build)
└── CMakeLists.txt      ← CMake (cross-platform)
```

## Building & Running

```bash
# Build everything (release, warnings-as-errors)
make release

# Run a specific chapter demo
./build/bin/ch01    # Signals & sequences
./build/bin/ch07    # DFT theory

# Run all 12 tests
make test

# Run all chapter demos
make run

# Clean build artefacts
make clean
```

## Architecture at a Glance

> **📊 System Architecture** — [View full-size diagram →](../reference/diagrams/architecture.png)

The toolkit is organised in layers:

- **Application Layer** — Your code (demos, tests, and custom programs)
- **Core Library** — `fft.c`, `filter.c`, `dsp_utils.c`, `signal_gen.c`, `convolution.c`
- **System Interface** — File I/O, future ALSA audio
- **Platform Abstraction** — POSIX, math library

> **📊 Module Dependencies** — [View full-size diagram →](../reference/diagrams/modules.png)

Dependency rule: everything depends on `dsp_utils` (the `Complex` type lives
there). `fft.c` and `filter.c` are independent of each other.

---

**Next:** [Chapter 01 — Discrete-Time Signals & Sequences →](01-signals-and-sequences.md)
