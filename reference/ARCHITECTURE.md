# DSP Tutorial Suite: Architecture & Design

Complete architectural overview of the DSP Tutorial Suite based on PlantUML diagrams.

## System Architecture

> **System Architecture** — [View full-size diagram →](diagrams/architecture.png)
>
> *Source: [architecture.puml](diagrams/architecture.puml)*

The project is organized in four layers:

### Chapter Demos (Application Layer)
30 self-contained demo binaries (`ch01`–`ch30`), each exercising specific library
modules with rich ASCII output and gnuplot visualisations. Each chapter lives in its
own subdirectory under `chapters/` with `tutorial.md`, `demo.c`, `README.md`, and `plots/`.

### DSP Core Library (`libdsp_core.a`)
23 source modules compiled into a static library. Organized into functional groups:

1. **Foundation** (3 modules)
   - `dsp_utils` — Complex arithmetic, window functions (Hann, Hamming, Blackman), helpers
   - `signal_gen` — Discrete-time signal generation (impulse, cosine, chirp, noise)
   - `convolution` — Linear/causal convolution, cross/auto-correlation, energy/power

2. **Transforms** (2 modules)
   - `fft` — Cooley-Tukey Radix-2 FFT/IFFT, real-valued FFT, magnitude/phase extraction
   - `advanced_fft` — Goertzel algorithm, DTMF detection, sliding DFT

3. **Spectral Analysis** (3 modules)
   - `spectrum` — Periodogram, Welch PSD, cross-PSD, frequency axis utilities
   - `spectral_est` — MUSIC and Capon (minimum variance) super-resolution methods
   - `cepstrum` — Real/complex cepstrum, Mel filterbank, MFCC pipeline, DCT-II

4. **Filters** (4 modules)
   - `filter` — FIR filter (direct convolution, moving average, windowed-sinc lowpass)
   - `iir` — Biquad design/processing, SOS cascades, Butterworth & Chebyshev I design
   - `remez` — Parks-McClellan (IRLS) optimal equiripple FIR design
   - `adaptive` — LMS, NLMS, RLS adaptive filtering algorithms

5. **Multirate & Streaming** (2 modules)
   - `multirate` — Decimation, interpolation, rational resampling, polyphase filters
   - `streaming` — Overlap-Add and Overlap-Save block FFT convolution

6. **Analysis** (4 modules)
   - `correlation` — FFT-based cross-correlation, autocorrelation, normalized variants
   - `hilbert` — Hilbert transform FIR design, analytic signal, envelope, instantaneous frequency
   - `lpc` — Levinson-Durbin recursion, AR modelling, LPC spectral envelope
   - `averaging` — Coherent averaging, EMA, moving average, median filter

7. **Numeric & 2-D** (2 modules)
   - `fixed_point` — Q15/Q31 fixed-point arithmetic, saturating ops, FIR-Q15, SQNR
   - `dsp2d` — 2-D convolution, Sobel/Gaussian/LoG kernels, 2D FFT

8. **Real-Time & Optimisation** (2 modules)
   - `realtime` — Lock-free ring buffer (SPSC), frame processor, latency measurement
   - `optimization` — Radix-4 FFT, pre-computed twiddle tables, benchmarking, aligned memory

### Tools & Visualisation
- `gnuplot` module — Pipe-based PNG plot generation via gnuplot
- `generate_plots` — Batch tool that generates 70+ plots across all chapters
- PlantUML diagrams — 4 common + 31 chapter-specific concept diagrams

### Build System
- GNU Make with 39 targets (30 demos + 8 test suites + generate_plots)
- Static library `libdsp_core.a` (23 `.o` files)
- C99 strict: `-Wall -Wextra -Werror -std=c99 -fPIC`
- Debug and release configurations
- Zero external dependencies (only `libc` + `libm`)

## Signal Processing Pipeline

> **Signal Processing Pipeline** — [View full-size diagram →](../chapters/13-spectral-analysis/signal_flow.png)
>
> *Source: [signal_flow.puml](../chapters/13-spectral-analysis/signal_flow.puml)*

Typical DSP workflow flows through two domains:

### Time Domain Operations
1. **Input Signal**: Generated or loaded sample buffer
2. **Windowing**: Apply Hann/Hamming/Blackman window to reduce spectral leakage
3. **Optional Filtering**: Time-domain FIR or IIR filtering
4. **Post-Processing**: Normalization, scaling, buffer management
5. **Output Signal**: Processed samples

### Frequency Domain Operations
1. **FFT Transform**: Convert to frequency domain (Radix-2 or Radix-4)
2. **Spectral Analysis**: Extract magnitude, phase, PSD
3. **IFFT Transform**: Convert back to time domain

Processing modes:
- **Batch processing**: Process entire signals at once
- **Streaming mode**: Overlap-Add/Save with configurable frame size
- **Real-time framing**: Ring buffer → frame processor pipeline

## Module Dependencies

> **Module Dependencies** — [View full-size diagram →](diagrams/modules.png)
>
> *Source: [modules.puml](diagrams/modules.puml)*

### Root Dependency: `dsp_utils.h`

9 of 23 modules depend on `dsp_utils.h` for the `Complex` type and window functions:
`fft`, `advanced_fft`, `signal_gen`, `iir`, `hilbert`, `spectrum`, `streaming`, `realtime`, `optimization`

The remaining 14 modules have no inter-module `#include` dependencies and can be used standalone.

### Module Responsibilities

| Module | Purpose | Dependencies |
|--------|---------|---|
| **dsp_utils** | Complex arithmetic, windows, helpers (13 functions) | None |
| **signal_gen** | Signal generation: impulse, cosine, chirp, noise (12 functions) | dsp_utils |
| **convolution** | Convolution, correlation, energy (7 functions) | None |
| **fft** | Radix-2 FFT/IFFT, real FFT, magnitude/phase (5 functions) | dsp_utils |
| **advanced_fft** | Goertzel, DTMF detection, sliding DFT (7 functions) | dsp_utils |
| **filter** | FIR filter, moving average, lowpass (3 functions) | None |
| **iir** | Biquad, SOS, Butterworth, Chebyshev (17 functions) | dsp_utils |
| **spectrum** | Periodogram, Welch PSD, cross-PSD (6 functions) | dsp_utils |
| **spectral_est** | MUSIC, Capon, eigendecomposition (5 functions) | None |
| **cepstrum** | Cepstrum, Mel filterbank, MFCCs (8 functions) | None |
| **correlation** | FFT-based xcorr, autocorr (5 functions) | None |
| **hilbert** | Analytic signal, envelope, inst frequency (5 functions) | dsp_utils |
| **lpc** | Levinson-Durbin, AR spectrum (6 functions) | None |
| **averaging** | Coherent avg, EMA, median filter (5 functions) | None |
| **remez** | Parks-McClellan equiripple FIR (3 functions) | None |
| **adaptive** | LMS, NLMS, RLS adaptive filtering (12 functions) | None |
| **multirate** | Decimation, interpolation, polyphase (4 functions) | None |
| **streaming** | Overlap-Add/Save block convolution (6 functions) | dsp_utils |
| **fixed_point** | Q15/Q31 arithmetic, FIR-Q15, SQNR (16 functions) | None |
| **dsp2d** | 2-D conv, Sobel, FFT2D (10 functions) | None |
| **realtime** | Ring buffer, frame processor, latency (17 functions) | dsp_utils |
| **optimization** | Radix-4 FFT, twiddle tables, benchmarks (10 functions) | dsp_utils |
| **gnuplot** | Pipe-based PNG plot output (8 functions) | None (ext: gnuplot) |

**Total: 23 modules, ~150 public functions, 19 struct/typedef types**

## FFT Processing Sequence

![FFT Processing Sequence](../chapters/08-fft-fundamentals/fft_sequence.png)

*Source: [fft_sequence.puml](../chapters/08-fft-fundamentals/fft_sequence.puml)*

### Key Features
- **Windowing**: Reduces spectral leakage from signal discontinuities
- **Bit-reversal permutation**: In-place reordering for DIT
- **Butterfly operations**: O(N log N) complex multiply-add
- **Radix-2**: Standard Cooley-Tukey (all power-of-2 sizes)
- **Radix-4**: 25% fewer multiplications (power-of-4 sizes, falls back to radix-2)

## Real-Time Streaming Architecture

![Real-Time Streaming Architecture](../chapters/28-real-time-streaming/realtime_architecture.png)

*Source: [realtime_architecture.puml](../chapters/28-real-time-streaming/realtime_architecture.puml)*

### Design
- **Lock-free ring buffer**: Single-producer/single-consumer with atomic indices
- **Frame processor**: Accumulates samples → applies Hann window → runs FFT → extracts peaks
- **Latency timer**: Microsecond-resolution timing via `clock_gettime(CLOCK_MONOTONIC)`
- **Stats tracking**: Min/max/average latency across processing cycles

### Ring Buffer API
- `ring_buffer_create/destroy` — lifecycle
- `ring_buffer_write/read/peek/skip` — data operations
- `ring_buffer_available/space` — status queries
- `ring_buffer_reset` — flush

## Performance Optimisation Strategy

![Performance Optimization Roadmap](../chapters/29-optimisation/optimization_roadmap.png)

*Source: [optimization_roadmap.puml](../chapters/29-optimisation/optimization_roadmap.puml)*

### Implemented Optimisations

| Technique | Implementation | Benefit |
|-----------|---------------|---------|
| **Radix-4 FFT** | `fft_radix4()` / `ifft_radix4()` | ~25% fewer multiplications vs radix-2 |
| **Pre-computed twiddles** | `twiddle_create()` / `fft_with_twiddles()` | Avoid repeated `cos`/`sin` calls |
| **Aligned memory** | `aligned_alloc_dsp()` (64-byte alignment) | Cache-line friendly allocation |
| **Benchmarking** | `bench_fft_radix2()` / `bench_fft_radix4()` / `bench_print()` | Measure min/avg/max/MFLOP/s |
| **Compiler flags** | `-O3 -fPIC` in release mode | Compiler auto-vectorisation |

## API Reference Structure

> **Public API Reference** — [View full-size diagram →](diagrams/api_reference.png)
>
> *Source: [api_reference.puml](diagrams/api_reference.puml)*

The API is organized by implementation phase:

| Phase | Headers | Functions |
|-------|---------|-----------|
| Phase 1: Foundation | dsp_utils, signal_gen, convolution | ~32 |
| Phase 2: Transforms | fft, advanced_fft | ~12 |
| Phase 2–4: Spectral | spectrum, spectral_est, cepstrum | ~19 |
| Phase 3: Filters | filter, iir, remez, adaptive, multirate, streaming | ~45 |
| Phase 4: Analysis | correlation, hilbert, lpc, averaging | ~21 |
| Phase 5–6: Numeric | fixed_point, dsp2d | ~26 |
| Phase 7: Real-Time | realtime, optimization | ~27 |

## Development Phases

All 7 phases are **complete**:

1. **Phase 1: Foundation** — Ch01–05, Ch07 (signals, sampling, complex, LTI, Z-transform, DFT)
2. **Phase 2: Transforms** — Ch06, Ch08–09 (frequency response, FFT, windows)
3. **Phase 3: Filters** — Ch10–12 (FIR, IIR, filter structures)
4. **Phase 4: C-Specific DSP** — Ch16, Ch18–19 (streaming, fixed-point, Goertzel)
5. **Phase 5: Advanced UG** — Ch17, Ch20–22 (multirate, Hilbert, averaging, Remez)
6. **Phase 6: Postgraduate** — Ch23–27 (adaptive, LPC, MUSIC, MFCC, 2D-DSP)
7. **Phase 7: Systems & Capstone** — Ch28–30 (real-time, optimisation, capstone)

## Use Cases

> **Use Cases** — [View full-size diagram →](diagrams/use_cases.png)
>
> *Source: [use_cases.puml](diagrams/use_cases.puml)*

### Primary Applications

**Education & Learning**
- Progressive DSP curriculum from undergraduate to postgraduate
- Self-contained demos with gnuplot visualisations
- Textbook cross-references (Oppenheim, Proakis, Haykin)

**Algorithm Prototyping**
- Pure C99 implementations for easy porting
- Zero external dependencies for embedded targets
- Fixed-point support for MCU deployment

**Audio & Signal Analysis**
- Spectral analysis (FFT, Welch PSD, MUSIC, Capon)
- Filter design (FIR, IIR, adaptive)
- Feature extraction (MFCCs, cepstrum, LPC)

## Test Coverage

98 tests across 8 suites — all passing:

| Suite | Tests | Modules Covered |
|-------|-------|-----------------|
| test_fft | 6 | fft |
| test_filter | 6 | filter |
| test_iir | 10 | iir, freq_response |
| test_spectrum_corr | 12 | spectrum, correlation |
| test_phase4 | 12 | fixed_point, advanced_fft, streaming |
| test_phase5 | 15 | multirate, hilbert, averaging, remez |
| test_phase6 | 19 | adaptive, lpc, spectral_est, cepstrum, dsp2d |
| test_phase7 | 18 | realtime, optimization |

## Related Documentation

- [API.md](API.md) — Complete function reference
- [CHAPTER_INDEX.md](CHAPTER_INDEX.md) — Chapter-by-chapter quick reference
- [diagrams/](diagrams/) — PlantUML source files and rendered PNGs

---

**Note**: Architecture diagrams are split between `reference/diagrams/` (common) and
individual `chapters/XX-*/` directories (chapter-specific). To regenerate PNGs:

```bash
java -jar ~/tools/plantuml.jar -tpng reference/diagrams/*.puml chapters/*/*.puml
```
