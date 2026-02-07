# DSP Tutorial Suite: Visual Documentation

PlantUML diagrams for the DSP Tutorial Suite project. Diagrams are split between
this common directory and individual chapter folders (co-located where chapter-specific).

## Common Diagrams (this directory)

| Diagram | Source | Description |
|---------|--------|-------------|
| ![](architecture.png) | [architecture.puml](architecture.puml) | System architecture — 4-layer overview (demos, library, tools, build) |
| ![](modules.png) | [modules.puml](modules.puml) | Module dependency graph — all 23 headers with dependency arrows |
| ![](api_reference.png) | [api_reference.puml](api_reference.puml) | Public API reference — ~150 functions across 23 headers |
| ![](use_cases.png) | [use_cases.puml](use_cases.puml) | Primary use cases — audio, embedded, research applications |

## Chapter-Specific Diagrams (co-located)

Every chapter (00–30) has its own conceptual PlantUML diagram co-located
in its directory. Each chapter's `README.md` embeds the rendered PNG.

| Ch | Diagram | Description |
|----|---------|-------------|
| 00 | [course_overview](../../chapters/00-overview/) | Learning path roadmap |
| 01 | [signal_classification](../../chapters/01-signals-and-sequences/) | Signal taxonomy |
| 02 | [sampling_pipeline](../../chapters/02-sampling-and-aliasing/) | ADC → reconstruction |
| 03 | [complex_plane](../../chapters/03-complex-numbers/) | Rectangular/polar forms |
| 04 | [lti_system](../../chapters/04-lti-systems/) | LTI convolution model |
| 05 | [z_transform](../../chapters/05-z-transform/) | Time ↔ Z domain |
| 06 | [freq_response_flow](../../chapters/06-frequency-response/) | Poles/zeros → H(e^jω) |
| 07 | [dft_analysis_synthesis](../../chapters/07-dft-theory/) | DFT analysis/synthesis |
| 08 | [fft_sequence](../../chapters/08-fft-fundamentals/) | Cooley-Tukey call flow |
| 09 | [window_tradeoff](../../chapters/09-window-functions/) | Window zoo & trade-offs |
| 10 | [fir_structure](../../chapters/10-digital-filters/) | Transversal FIR structure |
| 11 | [iir_design_flow](../../chapters/11-iir-filter-design/) | Analog → bilinear → digital |
| 12 | [filter_structures](../../chapters/12-filter-structures/) | DF1 / DF2T / SOS comparison |
| 13 | [signal_flow](../../chapters/13-spectral-analysis/) | Processing pipeline |
| 14 | [welch_method](../../chapters/14-psd-welch/) | Segment → window → average |
| 15 | [correlation_uses](../../chapters/15-correlation/) | Xcorr/autocorr applications |
| 16 | [ola_ols_pipeline](../../chapters/16-overlap-add-save/) | OLA vs OLS block conv |
| 17 | [multirate_chain](../../chapters/17-multirate-dsp/) | Decimate → process → interpolate |
| 18 | [fixed_point_format](../../chapters/18-fixed-point/) | Q15/Q31 format & operations |
| 19 | [goertzel_dtmf](../../chapters/19-advanced-fft/) | Goertzel & DTMF detection |
| 20 | [analytic_signal](../../chapters/20-hilbert-transform/) | Hilbert → envelope → inst freq |
| 21 | [averaging_methods](../../chapters/21-signal-averaging/) | Noise reduction methods |
| 22 | [remez_design](../../chapters/22-advanced-fir/) | Remez exchange iteration |
| 23 | [adaptive_loop](../../chapters/23-adaptive-filters/) | LMS/NLMS/RLS feedback |
| 24 | [lpc_model](../../chapters/24-linear-prediction/) | Analysis → residual → synthesis |
| 25 | [music_algorithm](../../chapters/25-parametric-spectral/) | MUSIC subspace method |
| 26 | [mfcc_pipeline](../../chapters/26-cepstrum-mfcc/) | Signal → Mel → DCT → MFCC |
| 27 | [image_processing](../../chapters/27-2d-dsp/) | 2-D spatial & frequency domain |
| 28 | [realtime_architecture](../../chapters/28-real-time-streaming/) | Ring buffer streaming |
| 29 | [optimization_roadmap](../../chapters/29-optimisation/) | Multi-stage optimisation |
| 30 | [capstone_pipeline](../../chapters/30-putting-it-together/) | End-to-end DSP pipeline |

## Rendering

### VS Code Extension
```bash
code --install-extension jebbs.plantuml
```
Open any `.puml` file and press `Alt+D` to preview.

### Command Line
```bash
# Render common diagrams
java -jar ~/tools/plantuml.jar -tpng reference/diagrams/*.puml

# Render chapter-specific diagrams
for f in chapters/*//*.puml; do
  java -jar ~/tools/plantuml.jar -tpng "$f"
done
```

### Automated Script
```bash
./reference/diagrams/render_diagrams.sh
```

## Updating Diagrams

1. Edit the `.puml` source
2. Regenerate PNG with PlantUML
3. Commit both `.puml` and `.png` files

---

**Note**: All `.png` files are generated from `.puml` sources.
Edit the `.puml` files directly; regenerate PNG on changes.
