/*
 * biquads.h
 *
 *  Created on: 18 Nov 2014
 *      Author: mr00132
 */

#ifndef CW2_BIQUADS_H_
#define CW2_BIQUADS_H_

extern double sinc(double x);
extern void calculateLowpassCoefficients(double *coefficients, long fs, int N, float f);
extern void biquad(int order, float *buffer, float *circBuffer, int *circBufferIndex, long num_frames, double *coefficients);

extern double firFilter(float *circbuffer, int order, int circBufferIndex, double *coefficients);

#endif /* CW2_BIQUADS_H_ */
