/*
 * biquads.h
 *
 *  Created on: 18 Nov 2014
 *      Author: mr00132
 */

#ifndef CW2_BIQUADS_H_
#define CW2_BIQUADS_H_


extern void biquad(float *buffer, float *circBuffer, int *circBufferIndex, long num_frames, double *coefficients);
extern void calculateLowpassCoefficients(double *coefficients, long fs, int N, float f);
extern double sinc(double x);
extern double firFilter(float *circbuffer, int order, int circBufferIndex, double *coefficients);
extern void zero_io_buffer(float *buffer);
#endif /* CW2_BIQUADS_H_ */
