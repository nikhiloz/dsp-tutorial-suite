/*
 * gnuplot.h - Pipe-based gnuplot helper for DSP tutorial plots
 *
 * Provides a minimal C99 interface to generate PNG plots via gnuplot.
 * Each chapter demo calls these functions to produce publication-quality
 * visualizations stored in chapters/<topic>/plots/.
 *
 * Architecture:
 *   +----------+    popen("gnuplot","w")   +---------+
 *   |  C demo  | ───── pipe commands ────> | gnuplot | ──> .png
 *   +----------+    inline '-' data        +---------+
 *
 * Usage:
 *   gp_init("01-signals-and-sequences");   // mkdir -p chapters/.../plots/
 *   FILE *gp = gp_open("01-signals-and-sequences", "impulse", 800, 500);
 *   fprintf(gp, "set title 'Unit Impulse'\n");
 *   fprintf(gp, "plot '-' w impulses lw 2 notitle\n");
 *   gp_send_y(gp, y, 16);
 *   gp_close(gp);
 *
 * Or use high-level helpers:
 *   gp_plot_1("ch01", "impulse", "Unit Impulse",
 *             "n", "x[n]", NULL, y, 16, "impulses");
 *
 * Build:  Link with dsp_core (already includes gnuplot.o)
 * Run:    Requires gnuplot installed (apt install gnuplot)
 */

#ifndef GNUPLOT_H
#define GNUPLOT_H

#include <stdio.h>

/* ── Directory / lifecycle ── */

/* Create output directory: plots/<chapter>/
 * Returns 0 on success, -1 on failure. */
int gp_init(const char *chapter);

/* Open gnuplot pipe.  Sets pngcairo terminal to plots/<chapter>/<name>.png
 * with size w x h.  Returns FILE* pipe, or NULL on error. */
FILE *gp_open(const char *chapter, const char *name, int w, int h);

/* Close gnuplot pipe (flushes and waits for process). */
void gp_close(FILE *gp);

/* ── Inline data sending ── */

/* Send index-vs-value data block:  "0\ty[0]\n 1\ty[1]\n ... e\n"
 * Call after a "plot '-' ..." command. */
void gp_send_y(FILE *gp, const double *y, int n);

/* Send x-vs-y paired data block.  If x==NULL, uses 0..n-1. */
void gp_send_xy(FILE *gp, const double *x, const double *y, int n);

/* ── High-level one-shot plotters ── */

/* Descriptor for a single data series in an overlay plot. */
typedef struct {
    const char *label;   /* legend label      */
    const double *x;     /* x-values (NULL → 0..n-1) */
    const double *y;     /* y-values          */
    int n;               /* number of points  */
    const char *style;   /* "lines", "impulses", "linespoints", "points" */
} GpSeries;

/* Plot a single signal.  x may be NULL for integer index. */
void gp_plot_1(const char *chapter, const char *name,
               const char *title, const char *xlabel, const char *ylabel,
               const double *x, const double *y, int n,
               const char *style);

/* Plot multiple overlaid series on one axis. */
void gp_plot_multi(const char *chapter, const char *name,
                   const char *title, const char *xlabel, const char *ylabel,
                   const GpSeries *series, int n_series);

/* Plot a dB-scale magnitude spectrum (x-axis = normalised freq 0..0.5). */
void gp_plot_spectrum(const char *chapter, const char *name,
                      const char *title,
                      const double *freq, const double *mag_db, int n);

#endif /* GNUPLOT_H */
