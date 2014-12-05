/*
 * CW2.c
 *
 *  Created on: 30 Nov 2014
 *      Author: mr00132
 */

/*
description of program
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "portsf.h"
#include "CW2_Biquads.h"

#define N_FRAMES_IN_BUFFER 1024
#define USAGE "cw2_MartinRowe <input wav> <output wav> <cutoff frequency in Hz>"

// To ensure that portsf does not close a file that was never opened.
#define INVALID_PORTSF_FID -1

//filter order
#define FIR_FILTER_ORDER 126

// To make calls to portsf more readable
enum float_clipping { DO_NOT_CLIP_FLOATS, CLIP_FLOATS };
enum minheader { DO_NOT_MINIMISE_HDR, MINIMISE_HDR };
enum auto_rescale { DO_NOT_AUTO_RESCALE, AUTO_RESCALE }; 

//declare functions
void biquad(float *buffer, float *circBuffer, int *circBufferIndex, long num_frames, int num_chans, double *coefficients);
void zero_io_buffer(float *buffer);
void calculateLowpassCoefficients(double *coefficients, long fs, int N, float f);
double sinc(double x);
double firFilter(float *circbuffer, int order, int circBufferIndex, double *coefficients);
//---------------------------------------------------------------------
int main( int argc, char *argv[] ){

    //fIDs are initialised to help with clean up code (if implimented)
	int in_fID = INVALID_PORTSF_FID;
	int out_fID = INVALID_PORTSF_FID;


	PSF_PROPS audio_properties; //PSF_props is a type defined in portsf
    float circBuffer[FIR_FILTER_ORDER] = {0.0}; //init circular Buffer and set all to zero values to relax the filter
    int circBufferLength = FIR_FILTER_ORDER;
    int circBufferIndex = 0;

	int return_value = EXIT_SUCCESS; //from stdlib

    double coefficients[FIR_FILTER_ORDER+1];

    float *buffer = NULL; //buffer
    long num_frames_written;
    long num_frames_read;
    long num_frames_to_write;




	// Initialise portsf library
    if(psf_init()) {
        printf("Unable to start portsf library.\n");
        return EXIT_FAILURE;
    }

    // Open the input file
    if ((in_fID = psf_sndOpen(argv[1], &audio_properties,DO_NOT_AUTO_RESCALE))<0) {

        printf("Unable to open file %s\n",argv[1]);
        return_value = EXIT_FAILURE;
    }

    // Open the output file
    if ((out_fID = psf_sndCreate(argv[2], &audio_properties, CLIP_FLOATS, DO_NOT_MINIMISE_HDR, PSF_CREATE_RDWR))<0) 
    {   
        printf("Unable to create file %s\n",argv[2]);
        return_value = EXIT_FAILURE;
    }

    //frame counters 
    printf("Number of channels in input file: %i\n",audio_properties.chans);
    DWORD nFrames = N_FRAMES_IN_BUFFER / audio_properties.chans; 

    //allocate memory for buffer
   if ((buffer = malloc(nFrames * audio_properties.chans * sizeof(float)))==NULL) {

        printf("Unable to allocate memory for buffer.\n");
        return_value = EXIT_FAILURE;
    }


    calculateLowpassCoefficients(&*coefficients, audio_properties.srate, FIR_FILTER_ORDER, 220.0);


    // Read frames from input file
    while ((num_frames_read=psf_sndReadFloatFrames(in_fID, buffer, nFrames)) > 0) { 

        //filter signal
        biquad(buffer,circBuffer,&circBufferIndex,num_frames_read,audio_properties.chans,coefficients);

        // Write the buffer to the output file
        num_frames_written = psf_sndWriteFloatFrames(out_fID,buffer,num_frames_read);

        if (num_frames_written!=num_frames_read) 
        {
            printf("Unable to write to %s\n",argv[2]);
            return_value = EXIT_FAILURE;
        }
    }


    // Deal with leftover frames
    for (int buffer_num=0; buffer_num<(circBufferLength-1); buffer_num += nFrames) 
    {    
        // Determine how many frames to write from the buffer
        if ((buffer_num+nFrames)>(circBufferLength-1)) {
            num_frames_to_write = (circBufferLength-1)-buffer_num;
        } else {
            num_frames_to_write = nFrames;
        }

        zero_io_buffer(buffer);
        biquad(buffer,circBuffer,&circBufferIndex,num_frames_to_write,audio_properties.chans,coefficients);

        // Write the buffer to the output file
        num_frames_written = psf_sndWriteFloatFrames(out_fID,buffer,num_frames_to_write);
        if (num_frames_written!=num_frames_to_write) {
            printf("Unable to write to %s\n",argv[2]);
            return_value = EXIT_FAILURE;
        }
    }


    //cleanup
    
	   // Free the memory for the frame
    if (buffer)
        free(buffer);

    // Close the output file
    if (out_fID>=0)
        psf_sndClose(out_fID);
    
    // Close the input file
    if (in_fID>=0)
        psf_sndClose(in_fID);
    
    // Close portsf library
    psf_finish();

	return 0;
}

void biquad( float *buffer, float *circBuffer, int *circBufferIndex, long num_frames, int num_chans, double *coefficients) {
    // int next_circBufferIndex;
    // float current_sample;
    // float delayed_sample;

    for (int c = 0; c < num_chans; c++)
        for (int s = 0; s < num_frames/num_chans; s += num_chans){
            circBuffer[*circBufferIndex] = buffer[s];
            buffer[s] = firFilter(circBuffer, FIR_FILTER_ORDER, *circBufferIndex, &*coefficients);
            *circBufferIndex = (*circBufferIndex + 1) % FIR_FILTER_ORDER;
            
        }

    // for (int frame_idx=0; frame_idx<num_frames*num_chans; frame_idx++) {

        // next_circBufferIndex = (*circBufferIndex+1)%FIR_FILTER_ORDER;
        
        // current_sample = buffer[frame_idx];

        // circBuffer[*circBufferIndex] = current_sample;
        // delayed_sample = circBuffer[next_circBufferIndex];
        // // We are using the same buffer for input and output
        // buffer[frame_idx] = current_sample + (0.5*delayed_sample);
        // *circBufferIndex = next_circBufferIndex;
    // }

}

void zero_io_buffer(float *buffer) {
    memset(buffer,0,N_FRAMES_IN_BUFFER*sizeof(float));
}

void calculateLowpassCoefficients(double *coefficients, long fs, int N, float fc){

for (int n = 0; n < N+1; n++){
    coefficients[n] = (0.54 - (0.46 * cos((2.0 * M_PI * n) / N))) * (((2.0 * fc) / fs) * sinc(((2.0 * n - N) * fc) / fs));
    //coefficients[n] = 0.46 * cos(2 * M_PI * n)
    printf("%i: %lf\n", n+1, coefficients[n]);
    }

printf("success, we found your %i coefficients for\n"
        "cutoff:%f\n"
        "sampling frequency:%lu\n"
    , N+1,fc,fs);

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