#include "dsp_utils.h"
#include <math.h>
double hann_window(int n,int i){return 0.5*(1-cos(2*M_PI*i/(n-1)));}
