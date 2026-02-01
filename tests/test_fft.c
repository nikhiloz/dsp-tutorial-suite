#include <assert.h>
#include "fft.h"
int main(){double in[2]={1,0},out[2];fft(in,out,2);assert(out[0]==1.0);return 0;}
