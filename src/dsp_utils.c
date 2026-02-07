#define _GNU_SOURCE
#include "dsp_utils.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double hann_window(int n, int i) {
    return 0.5 * (1.0 - cos(2.0 * M_PI * i / (n - 1)));
}
