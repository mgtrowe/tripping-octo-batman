/*
*    Filename: cw2_MartinRowe_Biquads.c
*  Created on: 30 Nov 2014
*      Author: Martin Rowe (mr00132)
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cw2_MartinRowe_Biquads.h"

double sinc(double x){
	if (x != 0)
		return (sin(M_PI * x)) / (M_PI * x);
	else
		return 1;
}
// deinterlace will cycle through samples and place channels in seperate arrays of 2D buffer
void deinterlace(float *buffer, float **deinterlacedBuffer, int nFrames, int chans){
    for (int c = 0; c < chans; c++){
    	for (int s = 0; s < nFrames; s ++){
    		deinterlacedBuffer[c][s] = buffer[(s*chans)+c];
    	}
    }
}
// interlace will take arrays from a 2D buffer and interlace samples to a 1D buffer
void interlace(float **deinterlacedBuffer, float *buffer, int nFrames, int chans){
    for (int c = 0; c < chans; c++){
    	for (int s = 0; s < nFrames; s++){
    		buffer[c+(chans*s)] = deinterlacedBuffer[c][s];
    	}
    }
}

// Function to calculate coefficients used in a biquad filter
void calculateLowpassCoefficients(double *coefficients, long fs, int N, float fc){
	double hammingWindow;
	double fourierLP;
	for (int n = 0; n < N+1; n++){
        // Using a hamming window avoids crude truncation of sinc function due to finite order of filter
		hammingWindow = 0.54 - (0.46 * cos((2.0 * M_PI * n) / N));
		fourierLP = ((2.0 * fc) / fs) * sinc(((2.0 * n - N) * fc) / fs);
		coefficients[n] = hammingWindow * fourierLP;
	}
}

// biquad() places samples in circular buffer for filter and calls firFilter() with the required parameters
void biquad(float *buffer, float *circBuffer, int *circBufferIndex, long numSamples, int order, double *coefficients) {
	for (int s = 0; s < numSamples; s++){
		 	circBuffer[*circBufferIndex] = buffer[s];
			buffer[s] = firFilter(circBuffer, order, *circBufferIndex, coefficients);
			*circBufferIndex = (*circBufferIndex + 1) % order;  
        }
}

// FIR filter uses a circular buffer to delay samples. Circular buffer should be 'relaxed' to 0.0 before use
double firFilter(float *circbuffer, int order, int circBufferIndex, double *coefficients){
    int n; // Counter for orders of delay
    double sample = 0;
    for (int z = 0; z <= order; z++){
        n = (z+circBufferIndex)%order; // Line up 0th value in circular buffer
        sample += circbuffer[n] * coefficients[z];
    }
    return sample;
}