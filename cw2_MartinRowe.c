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
#include "portsf.h"
#include "CW2_Biquads.h"

#define N_FRAMES_IN_BUFFER 1024
#define USAGE "cw2_MartinRowe <input wav> <output wav> <cutoff frequency in Hz>"

// To ensure that portsf does not close a file that was never opened.
#define INVALID_PORTSF_FID -1

//circulr buffer
#define CIRCULAR_BUFFER_LENGTH 1024

// To make calls to portsf more readable
enum float_clipping { DO_NOT_CLIP_FLOATS, CLIP_FLOATS };
enum minheader { DO_NOT_MINIMISE_HDR, MINIMISE_HDR };
enum auto_rescale { DO_NOT_AUTO_RESCALE, AUTO_RESCALE }; 

//declare functions
void biquad(float *buffer, float *circBuffer, int *circBufferIndex, long num_frames, int num_chans);
void zero_io_buffer(float *buffer);

//---------------------------------------------------------------------
int main( int argc, char *argv[] ){

    //fIDs are initialised to help with clean up code (if implimented)
	int in_fID = INVALID_PORTSF_FID;
	int out_fID = INVALID_PORTSF_FID;


	PSF_PROPS audio_properties; //PSF_props is a type defined in portsf
    float circBuffer[CIRCULAR_BUFFER_LENGTH] = {0.0}; //init circular Buffer and set all to zero values
    int circBufferIndex;

	int return_value = EXIT_SUCCESS; //from stdlib
 	

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
    float *buffer = NULL;
    long num_frames_written;
    long num_frames_read;
    long num_frames_to_write;

    //allocate memory for buffer
   if ((buffer = malloc(nFrames * audio_properties.chans * sizeof(float)))==NULL) {

        printf("Unable to allocate memory for buffer.\n");
        return_value = EXIT_FAILURE;
    }





    // Read frames from input file
    while ((num_frames_read=psf_sndReadFloatFrames(in_fID, buffer, nFrames)) > 0) { 

        //filter signal
        biquad(buffer,circBuffer,&circBufferIndex,num_frames_read,audio_properties.chans);

        // Write the buffer to the output file
        num_frames_written = psf_sndWriteFloatFrames(out_fID,buffer,num_frames_read);

        if (num_frames_written!=num_frames_read) 
        {
            printf("Unable to write to %s\n",argv[2]);
            return_value = EXIT_FAILURE;
        }
    }


    // Deal with leftover frames
    for (int buffer_num=0; buffer_num<(CIRCULAR_BUFFER_LENGTH-1); buffer_num += nFrames) 
    {    
        // Determine how many frames to write from the buffer
        if ((buffer_num+nFrames)>(CIRCULAR_BUFFER_LENGTH-1)) {
            num_frames_to_write = (CIRCULAR_BUFFER_LENGTH-1)-buffer_num;
        } else {
            num_frames_to_write = nFrames;
        }

        zero_io_buffer(buffer);
        biquad(buffer,circBuffer,&circBufferIndex,num_frames_to_write,audio_properties.chans);

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

void biquad( float *buffer, float *circBuffer, int *circBufferIndex, long num_frames, int num_chans ) {
    int next_circBufferIndex;
    float current_sample;
    float delayed_sample;
    for (int i = 0; i < num_frames*num_chans; i++){
    buffer[i] = 0.0;
    }

    // for (int frame_idx=0; frame_idx<num_frames; frame_idx++) {
    //     next_circBufferIndex = (*circBufferIndex+1)%CIRCULAR_BUFFER_LENGTH;
    //     //buffer[frame_idx] = 0.0;
    //     // current_sample = buffer[frame_idx];
    //     // circBuffer[*circBufferIndex] = current_sample;
    //     // delayed_sample = circBuffer[next_circBufferIndex];
    //     // // We are using the same buffer for input and output
    //     // buffer[frame_idx] = current_sample + (0.5*delayed_sample);
    //     *circBufferIndex = next_circBufferIndex;
    // }

}

void zero_io_buffer(float *buffer) {
    memset(buffer,0,N_FRAMES_IN_BUFFER*sizeof(float));
}
