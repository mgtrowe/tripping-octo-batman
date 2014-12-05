/*
 * CW2.c
 *
 *  Created on: 30 Nov 2014
 *      Author: mr00132
 */

/*
Program implimenting a FIR low-pass filter 
algorithm and executing the algorithm on a
.wav file using the PortSF library.
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

//filter order (must be int and even)
#define FIR_FILTER_ORDER 126

// To make calls to portsf more readable
enum float_clipping { DO_NOT_CLIP_FLOATS, CLIP_FLOATS };
enum minheader { DO_NOT_MINIMISE_HDR, MINIMISE_HDR };
enum auto_rescale { DO_NOT_AUTO_RESCALE, AUTO_RESCALE }; 

//declare functions
void parseUserInput(int argc, char *argv[], char *inputFilename, char *outputFilename, long *fc);
void zero_io_buffer(float *buffer);

//---------------------------------------------------------------------------

int main( int argc, char *argv[] ){
    // Init variables for user input storage
    long fc;
    char inputFilename[64];
    char outputFilename[64];

    // PortSF initialisation variables
	int in_fID = INVALID_PORTSF_FID; // fIDs are initialised to help clean up code at end of program
	int out_fID = INVALID_PORTSF_FID;
	PSF_PROPS audio_properties; // PSF_props is a type defined in PortSF

    double coefficients[FIR_FILTER_ORDER+1]; // for storing the coefficients calculated with user input freq

    // A circular buffer will be used in the FIR filter implimentation
    int circBufferLength = FIR_FILTER_ORDER; // smallest possible circular buffer size is determined by FIR filter order
    float circBuffer[FIR_FILTER_ORDER] = {0.0}; // Init circular Buffer and set all to zero values to relax the filter
    int circBufferIndex = 0;

    float *buffer = NULL; // Buffer for Portsf - size will be allocated when input file length is known
    long num_frames_written;
    long num_frames_read;
    long num_frames_to_write;

    int return_value = EXIT_SUCCESS; //from stdlib

    //---------------------------------------------------------------------
    // INTERFACE

    // Function to recieve and error-check user input
    parseUserInput(argc, argv, inputFilename, outputFilename, &fc); 

    //---------------------------------------------------------------------
	// PORTSF I/O AND MEMORY ALLOCATION

    // Initialise portsf library
    if(psf_init()) {
        printf("Unable to start portsf library.\n");
        return_value = EXIT_FAILURE;
    }

    // Open the input file
    if ((in_fID = psf_sndOpen(inputFilename, &audio_properties,DO_NOT_AUTO_RESCALE))<0) {
        printf("Unable to open file %s\n",inputFilename);
        return_value = EXIT_FAILURE;
    }

    // Open the output file
    if ((out_fID = psf_sndCreate(outputFilename, &audio_properties, CLIP_FLOATS, DO_NOT_MINIMISE_HDR, PSF_CREATE_RDWR))<0) 
    {   
        printf("Unable to create file %s\n",outputFilename);
        return_value = EXIT_FAILURE;
    }

    // Check input file channel count
    if (audio_properties.chans > 1){
        printf("Number of channels in input file (%i) exceeds 1. Please select a mono input file.\n",audio_properties.chans);
        return_value = EXIT_FAILURE;
    }
    // Init frame counter (for multichannel if implimented)
    DWORD nFrames = N_FRAMES_IN_BUFFER / audio_properties.chans; 

    // Allocate memory for buffer
   if ((buffer = malloc(nFrames * audio_properties.chans * sizeof(float)))==NULL) {
        printf("Unable to allocate memory for buffer.\n");
        return_value = EXIT_FAILURE;
    }
    //---------------------------------------------------------------------
    // COEFFICIENT CALCULATION AND AUDIO PROCESSING

    // Calculate coefficients after input file sample rate is read
    calculateLowpassCoefficients(&*coefficients, audio_properties.srate, FIR_FILTER_ORDER, fc);  

    // Read frames from input file
    while ((num_frames_read=psf_sndReadFloatFrames(in_fID, buffer, nFrames)) > 0) { 

        // Filter signal
        biquad(FIR_FILTER_ORDER,buffer,circBuffer,&circBufferIndex,num_frames_read,coefficients);

        // Write the buffer to the output file
        num_frames_written = psf_sndWriteFloatFrames(out_fID,buffer,num_frames_read);

        if (num_frames_written!=num_frames_read) 
        {
            printf("Unable to write to %s\n",outputFilename);
            return_value = EXIT_FAILURE;
        }
    }


    // Deal with remainder frames
    for (int buffer_num=0; buffer_num<(circBufferLength-1); buffer_num += nFrames) 
    {    
        // Determine how many frames to write from the buffer
        if ((buffer_num+nFrames)>(circBufferLength-1)) {
            num_frames_to_write = (circBufferLength-1)-buffer_num;
        } else {
            num_frames_to_write = nFrames;
        }

        // Function to relax buffer
        zero_io_buffer(buffer);

        // Filter signal
        biquad(FIR_FILTER_ORDER,buffer,circBuffer,&circBufferIndex,num_frames_to_write,coefficients);

        // Write the buffer to the output file
        num_frames_written = psf_sndWriteFloatFrames(out_fID,buffer,num_frames_to_write);

        if (num_frames_written!=num_frames_to_write) {
            printf("Unable to write to %s\n",outputFilename);
            return_value = EXIT_FAILURE;
        }
    }

    //---------------------------------------------------------------------
    // CLEAN UP
    
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

//---------------------------------------------------------------------------

void parseUserInput(int argc, char *argv[], char *inputFilename, char *outputFilename, long *fc){
    strcpy(inputFilename, argv[1]);
    strcpy(outputFilename, argv[2]);
    *fc = atol(argv[3]);
}

void zero_io_buffer(float *buffer) {
    memset(buffer,0,N_FRAMES_IN_BUFFER*sizeof(float));
}
