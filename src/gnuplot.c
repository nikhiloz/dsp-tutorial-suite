/*
 * gnuplot.c - Implementation of gnuplot pipe helper for DSP tutorial
 *
 * Opens gnuplot via popen(), sends commands + inline data to produce
 * publication-quality PNG plots.  All output goes to plots/<chapter>/.
 *
 * Default style:
 *   - pngcairo terminal (anti-aliased, TrueType fonts)
 *   - White background, grid lines
 *   - Line width 2, consistent colour palette
 *
 * Requires: gnuplot >= 5.0 with pngcairo support
 */

/* Need _POSIX_C_SOURCE for popen/pclose in strict C99 mode */
#define _POSIX_C_SOURCE 200809L

#include "gnuplot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* ── Output directory base (relative to project root) ── */
#define GP_BASE_DIR  "plots"

/* ── Default plot styling ── */
#define GP_FONT      "Arial,11"
#define GP_LINE_W    "2"

/*
 * Colour palette — chosen for readability + colour-blind safety.
 *
 *   1: #2166AC  (strong blue)
 *   2: #B2182B  (strong red)
 *   3: #1B7837  (green)
 *   4: #E08214  (orange)
 *   5: #7570B3  (purple)
 *   6: #66C2A5  (teal)
 */
static const char *GP_PALETTE =
    "set linetype 1 lc rgb '#2166AC' lw 2\n"
    "set linetype 2 lc rgb '#B2182B' lw 2\n"
    "set linetype 3 lc rgb '#1B7837' lw 2\n"
    "set linetype 4 lc rgb '#E08214' lw 2\n"
    "set linetype 5 lc rgb '#7570B3' lw 2\n"
    "set linetype 6 lc rgb '#66C2A5' lw 2\n";

/* ── Helper: build path string ── */
static void build_path(char *buf, int sz,
                       const char *chapter, const char *name,
                       const char *ext)
{
    snprintf(buf, (size_t)sz, "%s/%s/%s%s", GP_BASE_DIR, chapter, name, ext);
}

/* ── Helper: recursive mkdir (like mkdir -p) ── */
static int mkdirs(const char *path)
{
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) return -1;
            *p = '/';
        }
    }
    return mkdir(tmp, 0755) == 0 || errno == EEXIST ? 0 : -1;
}

/* ── Send default styling commands to an open pipe ── */
static void setup_defaults(FILE *gp)
{
    fprintf(gp, "%s", GP_PALETTE);
    fprintf(gp, "set style line 1 lw 2\n");
    fprintf(gp, "set grid\n");
    fprintf(gp, "set border 3\n");          /* bottom + left only   */
    fprintf(gp, "set tics nomirror\n");      /* ticks on 2 sides     */
    fprintf(gp, "set key top right\n");
    fprintf(gp, "set samples 1000\n");       /* smooth function plots */
}

/* ================================================================== */
/* Public API                                                         */
/* ================================================================== */

int gp_init(const char *chapter)
{
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/%s", GP_BASE_DIR, chapter);
    return mkdirs(dir);
}

FILE *gp_open(const char *chapter, const char *name, int w, int h)
{
    /* Ensure directory exists */
    gp_init(chapter);

    FILE *gp = popen("gnuplot", "w");
    if (!gp) {
        fprintf(stderr, "[gnuplot] ERROR: cannot open gnuplot pipe\n");
        return NULL;
    }

    /* Set PNG output */
    char path[512];
    build_path(path, (int)sizeof(path), chapter, name, ".png");
    fprintf(gp, "set terminal pngcairo size %d,%d enhanced font '%s'\n",
            w, h, GP_FONT);
    fprintf(gp, "set output '%s'\n", path);

    setup_defaults(gp);

    return gp;
}

void gp_close(FILE *gp)
{
    if (!gp) return;
    fprintf(gp, "unset output\n");
    fflush(gp);
    pclose(gp);
}

/* ── Inline data blocks ── */

void gp_send_y(FILE *gp, const double *y, int n)
{
    if (!gp || !y) return;
    for (int i = 0; i < n; i++) {
        fprintf(gp, "%d\t%.10g\n", i, y[i]);
    }
    fprintf(gp, "e\n");
}

void gp_send_xy(FILE *gp, const double *x, const double *y, int n)
{
    if (!gp || !y) return;
    for (int i = 0; i < n; i++) {
        double xv = x ? x[i] : (double)i;
        fprintf(gp, "%.10g\t%.10g\n", xv, y[i]);
    }
    fprintf(gp, "e\n");
}

/* ── High-level plotters ── */

void gp_plot_1(const char *chapter, const char *name,
               const char *title, const char *xlabel, const char *ylabel,
               const double *x, const double *y, int n,
               const char *style)
{
    FILE *gp = gp_open(chapter, name, 800, 500);
    if (!gp) return;

    fprintf(gp, "set title '%s'\n", title);
    fprintf(gp, "set xlabel '%s'\n", xlabel);
    fprintf(gp, "set ylabel '%s'\n", ylabel);
    fprintf(gp, "plot '-' with %s lw 2 notitle\n", style);
    gp_send_xy(gp, x, y, n);
    gp_close(gp);
}

void gp_plot_multi(const char *chapter, const char *name,
                   const char *title, const char *xlabel, const char *ylabel,
                   const GpSeries *series, int n_series)
{
    FILE *gp = gp_open(chapter, name, 800, 500);
    if (!gp) return;

    fprintf(gp, "set title '%s'\n", title);
    fprintf(gp, "set xlabel '%s'\n", xlabel);
    fprintf(gp, "set ylabel '%s'\n", ylabel);

    /* Build the "plot" command with multiple '-' sources */
    fprintf(gp, "plot ");
    for (int s = 0; s < n_series; s++) {
        if (s > 0) fprintf(gp, ", ");
        fprintf(gp, "'-' with %s lw 2 title '%s'",
                series[s].style ? series[s].style : "lines",
                series[s].label ? series[s].label : "");
    }
    fprintf(gp, "\n");

    /* Send each data block */
    for (int s = 0; s < n_series; s++) {
        gp_send_xy(gp, series[s].x, series[s].y, series[s].n);
    }

    gp_close(gp);
}

void gp_plot_spectrum(const char *chapter, const char *name,
                      const char *title,
                      const double *freq, const double *mag_db, int n)
{
    FILE *gp = gp_open(chapter, name, 800, 500);
    if (!gp) return;

    fprintf(gp, "set title '%s'\n", title);
    fprintf(gp, "set xlabel 'Normalised Frequency (f/f_s)'\n");
    fprintf(gp, "set ylabel 'Magnitude (dB)'\n");
    fprintf(gp, "set xrange [0:0.5]\n");
    fprintf(gp, "plot '-' with lines lw 2 notitle\n");
    gp_send_xy(gp, freq, mag_db, n);
    gp_close(gp);
}
