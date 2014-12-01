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
	// int in_fID = INVALID_PORTSF_FID;
	// char input_filename[100];

	// // Initialise portsf library
 //    if(psf_init()) {
 //        printf("Unable to start portsf library.\n");
 //        return EXIT_FAILURE;
 //    }

    // parse user input

	if(argv[1] != NULL){

		/* this error checking will actually be done in the file-opening process a la http://www.cprogramming.com/tutorial/c/lesson14.html */
		if (isWav(argv[1]) == 1){
			printf( "Input file: %s\n", argv[1] );
		} else {
			printf("Please check name of input file - usage: %s\n", USAGE);
		}
		if (isWav(argv[2]) == 1){
			printf( "Output file: %s\n", argv[2] );
		} else {
			printf("Please check name of output file - usage: %s\n", USAGE);
		}

		printf( "Cut-off frequency: %s\n", argv[3] );
	} else {
		printf("Instructions for use: use the number ");		
	}

    // // Open the input file
    // if ((in_fID = psf_sndOpen(INPUT_FILENAME, &audio_properties,
    //     DO_NOT_AUTO_RESCALE))<0) {

    //     printf("Unable to open file %s\n",INPUT_FILENAME);
    //     return_value = EXIT_FAILURE;
    //     goto CLEAN_UP;
    // }



	printf("%i\n",Biquadtest());

	return 0;
}

