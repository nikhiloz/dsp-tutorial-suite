# Chapter 29: DSP Optimisation

Radix-4 FFT, twiddle tables, benchmarking, aligned memory.

## Contents

| File | Description |
|------|------------|
| [tutorial.md](tutorial.md) | Full theory tutorial with equations and exercises |
| [demo.c](demo.c) | Self-contained runnable demo |
| [`optimization.h`](../../include/optimization.h) | Library API |
| [optimization_roadmap.puml](optimization_roadmap.puml) | PlantUML diagram source |

## Diagram

![Optimization Roadmap](optimization_roadmap.png)

## Generated Plots

![Radix Comparison](plots/radix_comparison.png)
![Throughput](plots/throughput.png)
![Twiddle Speedup](plots/twiddle_speedup.png)

---

[← Ch 28](../28-real-time-streaming/README.md) | [Index](../../reference/CHAPTER_INDEX.md) | [Ch 30 →](../30-putting-it-together/README.md)
