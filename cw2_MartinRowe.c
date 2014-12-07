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

#define N_SAMPLES_IN_BUFFER 1024
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
    float **circBuffer = NULL; // Init circular Buffer size will be determined by number of chans
    int *circBufferIndex = NULL;

    float *buffer = NULL; // Buffer for Portsf - size will be allocated when input file length is known
    float **deinterlacedBuffer = NULL; // Array for storing deinterleaved samples while processing
    int bufferLength;
    long num_frames_written;
    long num_frames_read;

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
        goto CLEANUP;
    }

    // Open the input file
    if ((in_fID = psf_sndOpen(inputFilename, &audio_properties,DO_NOT_AUTO_RESCALE))<0) {
        printf("Unable to open file %s\n",inputFilename);
        return_value = EXIT_FAILURE;
        goto CLEANUP;
    }

    // Open the output file
    if ((out_fID = psf_sndCreate(outputFilename, &audio_properties, CLIP_FLOATS, DO_NOT_MINIMISE_HDR, PSF_CREATE_RDWR))<0) 
    {   
        printf("Unable to create file %s\n",outputFilename);
        return_value = EXIT_FAILURE;
        goto CLEANUP;
    }

    // Check input file channel count
    if (audio_properties.chans > 2){
        printf("Number of channels in input file (%s) exceeds 1. Please select a mono input file.\n",inputFilename);
        return_value = EXIT_FAILURE;
        goto CLEANUP;
    }
    // Init frame counter
    DWORD nFrames = N_SAMPLES_IN_BUFFER / audio_properties.chans; 
    bufferLength = N_SAMPLES_IN_BUFFER;

    // Allocate memory for buffer
    if ((buffer = malloc(bufferLength * sizeof(float)))==NULL) {
        printf("Unable to allocate memory for buffer.\n");
        return_value = EXIT_FAILURE;
        goto CLEANUP;
    }

    // Allocate memory for deinterlacedBuffer 2D audio buffer
   if ((deinterlacedBuffer = malloc(audio_properties.chans))==NULL) {
        printf("Unable to allocate memory for deinterlacedBuffer channels.\n");
        return_value = EXIT_FAILURE;
        goto CLEANUP;
    }
    // The total buffer size is split between audio channels
   for (int i = 0; i < audio_properties.chans; i++){
       if ((deinterlacedBuffer[i] = malloc(nFrames * sizeof(float)))==NULL) {
            printf("Unable to allocate memory for deinterlacedBuffer buffer channel %i samples.\n",i);
            return_value = EXIT_FAILURE;
            goto CLEANUP;
        }
    }

    //allocate memory for circular buffer per audio channel
   if ((circBuffer = malloc(audio_properties.chans))==NULL) {
        printf("Unable to allocate memory for circularBuffer channels.\n");
        return_value = EXIT_FAILURE;
        goto CLEANUP;
    }
    for (int i = 0; i < audio_properties.chans; i++){
        if ((circBuffer[i] = malloc(circBufferLength * sizeof(float)))==NULL) {
            printf("Unable to allocate memory for circularBuffer channel %i samples.\n",i);
            return_value = EXIT_FAILURE;
            goto CLEANUP;
        }
    }

    if ((circBufferIndex = malloc(audio_properties.chans * sizeof(int)))==NULL) {
        printf("Unable to allocate memory for circular buffer index.\n");
        return_value = EXIT_FAILURE;
        goto CLEANUP;
    }

    //---------------------------------------------------------------------
    // COEFFICIENT CALCULATION AND AUDIO PROCESSING

    // Calculate coefficients after input file sample rate is read
    calculateLowpassCoefficients(coefficients, audio_properties.srate, FIR_FILTER_ORDER, fc);  

    // Read frames from input file into buffer
    while ((num_frames_read=psf_sndReadFloatFrames(in_fID, buffer, nFrames)) > 0) { 

        // Deinterlace buffer for easy per-channel processing
        deinterlace(buffer, deinterlacedBuffer, num_frames_read, audio_properties.chans);

        // Cycle through each channel for processing
        for (int chan = 0; chan < audio_properties.chans; chan++){
            // Filter signal for each deinterlaced channel
            biquad(deinterlacedBuffer[chan],circBuffer[chan],&circBufferIndex[chan],num_frames_read,FIR_FILTER_ORDER,coefficients);
        }
        // Interlace samples buffer
        interlace(deinterlacedBuffer, buffer, num_frames_read, audio_properties.chans);

        // Write the buffer to the output file
        num_frames_written = psf_sndWriteFloatFrames(out_fID,buffer,num_frames_read);

        if (num_frames_written!=num_frames_read) 
        {
            printf("Unable to write to %s\n",outputFilename);
            return_value = EXIT_FAILURE;
            goto CLEANUP;
        }
    }

    //---------------------------------------------------------------------
    CLEANUP:

	// Free the memory for the frame
    if (buffer)
        free(buffer);

    if (circBuffer)
        free(circBuffer);

    if (circBufferIndex)
        free(circBufferIndex);
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