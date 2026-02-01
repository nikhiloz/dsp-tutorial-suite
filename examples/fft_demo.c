#include <stdio.h>
#include "fft.h"
int main(){double in[4]={1,2,3,4},out[4];fft(in,out,4);for(int i=0;i<4;i++)printf("%f\n",out[i]);return 0;}
