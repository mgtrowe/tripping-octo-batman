#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "CW2_Biquads.h"
#include <string.h>
#include "portsf.h"
#include "CW2_Biquads.h"
#define FIR_FILTER_ORDER 126
#define N_FRAMES_IN_BUFFER 1024

void biquad( float *buffer, float *circBuffer, int *circBufferIndex, long num_frames,double *coefficients) {

   
        for (int s = 0; s < num_frames; s ++){
            circBuffer[*circBufferIndex] = buffer[s];
            buffer[s] = firFilter(circBuffer, FIR_FILTER_ORDER, *circBufferIndex, &*coefficients);
            *circBufferIndex = (*circBufferIndex + 1) % FIR_FILTER_ORDER;
            
        }


}

void zero_io_buffer(float *buffer) {
    memset(buffer,0,N_FRAMES_IN_BUFFER*sizeof(float));
}

void calculateLowpassCoefficients(double *coefficients, long fs, int N, float fc){

for (int n = 0; n < N+1; n++){
    coefficients[n] = (0.54 - (0.46 * cos((2.0 * M_PI * n) / N))) * (((2.0 * fc) / fs) * sinc(((2.0 * n - N) * fc) / fs));
    }


}

double sinc(double x){
    if (x != 0)
    return (sin(M_PI * x)) / (M_PI * x);
    else
    return 1;
}
double firFilter(float *circbuffer, int order, int circBufferIndex, double *coefficients){
    int n; //counter for orders of delay
    double sample = 0;
    for (int z = 0; z <= order; z++){
        n = (z+circBufferIndex)%order; //line up 0th value in circular buffer
        sample += circbuffer[n] * coefficients[z];
    }
    return sample;
}