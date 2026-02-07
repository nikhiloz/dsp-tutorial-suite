/**
 * @file gnuplot.c
 * @brief Gnuplot pipe helper — opens gnuplot via popen() to produce PNG plots.
 *
 * Sends commands and inline data to gnuplot to create publication-quality
 * PNG visualisations.  All output goes to plots/<chapter>/.
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

/* ── Output directory structure ── */
#define GP_BASE_DIR    "chapters"
#define GP_PLOT_SUBDIR "plots"

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
    snprintf(buf, (size_t)sz, "%s/%s/%s/%s%s",
             GP_BASE_DIR, chapter, GP_PLOT_SUBDIR, name, ext);
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

/**
 * @brief Initialise the output directory for a chapter's plots.
 * @param chapter  Chapter subdirectory name (e.g. "ch01").
 * @return         0 on success, −1 on directory creation failure.
 */
int gp_init(const char *chapter)
{
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/%s/%s", GP_BASE_DIR, chapter, GP_PLOT_SUBDIR);
    return mkdirs(dir);
}

/**
 * @brief Open a gnuplot pipe configured for PNG output.
 * @param chapter  Chapter subdirectory name.
 * @param name     Plot filename (without extension).
 * @param w        Image width in pixels.
 * @param h        Image height in pixels.
 * @return         Open FILE* pipe to gnuplot, or NULL on failure.
 */
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

/**
 * @brief Flush and close a gnuplot pipe.
 * @param gp  Gnuplot pipe (NULL-safe).
 */
void gp_close(FILE *gp)
{
    if (!gp) return;
    fprintf(gp, "unset output\n");
    fflush(gp);
    pclose(gp);
}

/* ── Inline data blocks ── */

/**
 * @brief Send y-only inline data to gnuplot (x = sample index).
 * @param gp  Open gnuplot pipe.
 * @param y   Y-axis data array.
 * @param n   Number of data points.
 */
void gp_send_y(FILE *gp, const double *y, int n)
{
    if (!gp || !y) return;
    for (int i = 0; i < n; i++) {
        fprintf(gp, "%d\t%.10g\n", i, y[i]);
    }
    fprintf(gp, "e\n");
}

/**
 * @brief Send x-y inline data to gnuplot.
 * @param gp  Open gnuplot pipe.
 * @param x   X-axis data (NULL → use sample index).
 * @param y   Y-axis data array.
 * @param n   Number of data points.
 */
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

/**
 * @brief Plot a single data series and save as PNG.
 * @param chapter  Chapter subdirectory name.
 * @param name     Output filename (no extension).
 * @param title    Plot title string.
 * @param xlabel   X-axis label.
 * @param ylabel   Y-axis label.
 * @param x        X-axis data (NULL → sample index).
 * @param y        Y-axis data.
 * @param n        Number of data points.
 * @param style    Gnuplot style string (e.g. "lines", "impulses").
 */
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

/**
 * @brief Plot multiple data series on one chart and save as PNG.
 * @param chapter   Chapter subdirectory name.
 * @param name      Output filename (no extension).
 * @param title     Plot title string.
 * @param xlabel    X-axis label.
 * @param ylabel    Y-axis label.
 * @param series    Array of GpSeries structs (label, x, y, n, style).
 * @param n_series  Number of series.
 */
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

/**
 * @brief Plot a frequency-domain magnitude spectrum in dB and save as PNG.
 * @param chapter  Chapter subdirectory name.
 * @param name     Output filename (no extension).
 * @param title    Plot title string.
 * @param freq     Normalised frequency array (0 … 0.5).
 * @param mag_db   Magnitude in dB.
 * @param n        Number of data points.
 */
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
