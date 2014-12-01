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

#define NUM_SAMPLES_IN_FRAME 1024
#define USAGE "cw2_MartinRowe <input wav> <output wav> <cutoff frequency in Hz>"

// To ensure that we do not use portsf to close a file that was never opened.
#define INVALID_PORTSF_FID -1

enum float_clipping { DO_NOT_CLIP_FLOATS, CLIP_FLOATS };
enum minheader { DO_NOT_MINIMISE_HDR, MINIMISE_HDR };
enum auto_rescale { DO_NOT_AUTO_RESCALE, AUTO_RESCALE };  

int isWav(char s[]){
	if (strstr(s, ".wav") != NULL){
		return 1;
	} else {
		return 0;
	}
}

int main( int argc, char *argv[] ){
	/* <programme name> Old.wav New.wav 1000 -filtertype highpass */

	/*  PRINT:
		1. The name of the input file (including the .wav suffix),
		2. The name of the output file (including the .wav suffix), and 
		3. The cut-off frequency of the filter (in Hz). */
	int in_fID = INVALID_PORTSF_FID;
	int out_fID = INVALID_PORTSF_FID;
	PSF_PROPS audio_properties;
	int return_value = EXIT_SUCCESS;
 	
 	DWORD nFrames = NUM_SAMPLES_IN_FRAME; 
	float *frame = NULL;
    long num_frames_read;

	// Initialise portsf library
    if(psf_init()) {
        printf("Unable to start portsf library.\n");
        return EXIT_FAILURE;
    }

    // Open the input file
    if ((in_fID = psf_sndOpen(argv[1], &audio_properties,
        DO_NOT_AUTO_RESCALE))<0) {

        printf("Unable to open file %s\n",argv[1]);
        return_value = EXIT_FAILURE;
    }

 	// Create the output file
    if ((out_fID = psf_sndCreate(argv[2], &audio_properties,
        CLIP_FLOATS, DO_NOT_MINIMISE_HDR, PSF_CREATE_RDWR))<0) {
        
        printf("Unable to create file %s\n",argv[2]);
        return_value = EXIT_FAILURE;
    }
    //allocate memory for frame
   if ((frame = (float*)malloc(nFrames
        *audio_properties.chans*sizeof(float)))==NULL) {

        printf("Unable to allocate memory for frame.\n");
        return_value = EXIT_FAILURE;
    }

    // Read frames from input file
    while ((num_frames_read=psf_sndReadFloatFrames(in_fID, frame, nFrames)) > 0) { 
        
        // Write the frame to the output file
        if (psf_sndWriteFloatFrames(out_fID,frame,num_frames_read)!=num_frames_read) {
            printf("Unable to write to %s\n",argv[2]);
            return_value = EXIT_FAILURE;
            break;
        }
    }


    //cleanup
    
	   // Free the memory for the frame
    if (frame)
        free(frame);

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

