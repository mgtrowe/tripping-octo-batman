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

#define USAGE "cw2_MartinRowe <input wav> <output wav> <cutoff frequency in Hz>"

// To ensure that we do not use portsf to close a file that was never opened.
#define INVALID_PORTSF_FID -1

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
	PSF_PROPS audio_properties;
	int return_value = EXIT_SUCCESS;

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



	printf("%i\n",Biquadtest());

	return 0;
}

