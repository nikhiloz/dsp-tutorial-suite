#include <assert.h>
#include "filter.h"
int main(){double in[2]={1,2},out[2];fir_filter(in,out,2);assert(out[0]==1.0);return 0;}
