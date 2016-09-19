#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>

// User defined libraries
#include "equalstrings.h"
#include "whitenoise.h"

#define TRUE 1
#define FALSE 0

#define TOP_HAT 1
#define GAUSSIAN 2

#define SIZE_FACTOR 5
#define SIZE_OFFSET 2

// PROGENY - Program to build custom profiles for metric / normalisation standardised testing
// Written by Andrew Cameron
// Version 1.4.2 - Last updated 19/09/2015

/*

CHANGELOG:

20/03/2015 - v0.1 - Began writing program, modifying home file from ffancy.c
14/03/2015 - v1.1 - Impelemented 'format' flag option
12/11/2015 - v1.2 - Began implementation of Gaussian pulse profiles
13/11/2015 - v1.2.1 - Modified Gaussian curve to fit FWHM as width rather than RMS width
                    - added ability to accept duty cycles as well as widths
17/11/2015 - v1.2.2 - Modified acceptance of width to allow floats
24/11/2015 - v1.3.0 - Modified to allow scattering tails to be included with Gaussians
                    - Added alternate input method to specify multiple pulse components
08/12/2015 - v1.3.1 - Updated scattered Gaussian to correctly implement exponential convolution
           - V1.4.0 - Rewrote pulse seeded algorithm to allow for scattering of any pulse shape combination
11/09/2016 - v1.4.1 - Rewrote help menu to update for publication
19/09/2016 - v1.4.2 - Modified noise generation code to reflect changes in whitenoise.c/h
*/

// ***** FUNCTION PROTOTYPES *****

// prints out an explanation of how to use the command line interface
void progeny_help();

// adds a top hat function to a profile
void pulseshape_tophat(double *profile, int size, double width, double height, int center);

// adds a gaussian function to a profile
void pulseshape_gaussian(double *profile, int size, double width, double height, int center);

// convolves a scattering exponential with a provided profile
void convolve_exponential(double *profile, int size, double scatter_time);

// folds a longer profile back to the expected size
void fold_profile(double *input_profile, double *output_profile, int input_size, int output_size);

// ***** MAIN FUNCTION *****

int main(int argc, char** argv) {

  // declare variables and initialise with defaults
  int nbins = 100;
  double base = 0;
  double sigma = 1;
  double pulseheight = 1;
  double pulsewidth = 1;
  int pulsecenter = 0;
  int pulseshape = TOP_HAT;
  int whitenoise_flag = FALSE;
  int dc_flag = FALSE;
  double dc = 0;
  FILE *inputfile = NULL;
  FILE *outputfile = NULL;
  double scatter_time = 0;

  int format = 1;

  int i; // counter
  // start the random number generator
  startseed();

  // check that a valid number of arguments have been passed
  if (argc < 2) {
    progeny_help();
    exit(0);
  }

  // scan arguments and allocate variables
  if (argc > 1) {
    i=1;
    while (i < argc) {
      if (equal_strings(argv[i],"-nbins")) {
	i++;
	nbins = atoi(argv[i]);
      } else if (equal_strings(argv[i],"-baseline")) {
	i++;
	base = atof(argv[i]);
      } else if (equal_strings(argv[i],"-height")) {
	i++;
	pulseheight = atof(argv[i]);
      } else if (equal_strings(argv[i],"-width")) {
	i++;
	pulsewidth = atof(argv[i]);
      } else if (equal_strings(argv[i],"-dc")) { 
	i++;
	dc_flag = TRUE;
	dc = atof(argv[i]);
      } else if (equal_strings(argv[i],"-center")) {
	i++;
	pulsecenter = atoi(argv[i]);
      } else if (equal_strings(argv[i],"-whitenoise")) {
	whitenoise_flag = TRUE;
      } else if (equal_strings(argv[i],"-o")) {
	i++;
	outputfile = fopen(argv[i], "w+");
      } else if (equal_strings(argv[i], "-i")) {
	i++;
	inputfile = fopen(argv[i], "r");
      } else if (equal_strings(argv[i],"-format")) {
	i++;
	format = atoi(argv[i]);
      } else if (equal_strings(argv[i],"-shape")) {
	i++;
	pulseshape = atoi(argv[i]);
      } else if (equal_strings(argv[i],"-sigma")) {
	i++;
	sigma = atof(argv[i]);
      } else if (equal_strings(argv[i], "-h") || equal_strings(argv[i], "--help")) {
	progeny_help();
	exit(0);
      } else if (equal_strings(argv[i], "-scatter")) {
	i++;
	scatter_time = atof(argv[i]);
      } else {
	printf("Unknown argument (%s) passed to ffancy.\nUse -h / --help to display help menu with acceptable arguments.\n",argv[i]);
	exit(0);
      }
      i++;
    }
  }

  // test for valid input
  assert(nbins > 0);
  assert(outputfile != NULL);
  assert(format == 1 || format == 2);

  // now begin profile generation
  // construct empty profile

  printf("Creating empty profile with %d bins, baseline %.10f...\n", nbins, base);
  double* profile = (double*)malloc(sizeof(double) * SIZE_FACTOR * nbins);
  for (i = 0; i < SIZE_FACTOR * nbins; i++) {
    profile[i] = 0;
  }
  printf("Empty profile created.\n");

  if (inputfile == NULL) {
    // create pulse based on command line
    printf("Now creating pulse based on command line input...\n");

    // check for alternative DC input and convert
    if (dc_flag == TRUE) {
      pulsewidth = nbins * (dc / 100); // POTENTIAL PROBLEM WITH THIS LINE
    }

    // test for valid input
    assert(pulsewidth < nbins);
    assert(pulseheight >= 0);
    assert(pulsewidth > 0);
    assert(sigma > 0);
    
    pulsecenter = pulsecenter % nbins;
    assert(pulsecenter >= 0 && pulsecenter < nbins);

    printf("Profile parameters:\nNBINS = %d\nBASELINE = %.10f\nSIGMA = %.10f\nPULSE HEIGHT = %.10f\nPULSE WIDTH = %.10f bins\n\n", nbins, base, sigma, pulseheight, pulsewidth);

    if (pulseshape == TOP_HAT) {
      printf("TOP HAT profile selected.\n");
      pulseshape_tophat(profile, nbins * SIZE_FACTOR, pulsewidth, pulseheight, pulsecenter);
    } else if (pulseshape == GAUSSIAN) {
      printf("GAUSSIAN profile selected.\n");
      pulseshape_gaussian(profile, nbins * SIZE_FACTOR, pulsewidth, pulseheight, pulsecenter);
    } else {
      printf("\nInvalid pulse shape chosen! Program aborting...\n");
      exit(0);
    }
  } else {
    printf("Now creating pulse based on file input...\n");
    // scan the input file and process each profile in turn
    // Format: ${height} w/d ${width}/${dc} ${center} ${shape}\n\n");

    int jj = 1;
    
    while(fscanf(inputfile, "%lf %d %lf %d %d", &pulseheight, &dc_flag, &pulsewidth, &pulsecenter, &pulseshape) != EOF) {
      
      // convert dc to width if neccessary
      if (dc_flag == TRUE) {
	pulsewidth = nbins * (pulsewidth / 100);
      } else if (dc_flag != FALSE) {
	printf("Invalid width/duty cycle flag specified - aborting...\n");
	exit(0);
      }

      // test for valid input
      assert(pulsewidth < nbins);
      assert(pulseheight >= 0);
      assert(pulsewidth > 0);
      assert(sigma > 0);
      pulsecenter = pulsecenter % nbins;
      assert(pulsecenter >= 0 && pulsecenter < nbins);
      printf("Profile %d parameters:\nNBINS = %d\nBASELINE = %.10f\nSIGMA = %.10f\nPULSE HEIGHT = %.10f\nPULSE WIDTH = %.10f bins\n\n", jj, nbins, base, sigma, pulseheight, pulsewidth);
      
      if (pulseshape == TOP_HAT) {
	printf("TOP HAT profile selected.\n");
	pulseshape_tophat(profile, nbins * SIZE_FACTOR, pulsewidth, pulseheight, pulsecenter);
      } else if (pulseshape == GAUSSIAN) {
	printf("GAUSSIAN profile selected.\n");
	pulseshape_gaussian(profile, nbins * SIZE_FACTOR, pulsewidth, pulseheight, pulsecenter);
      } else {
	printf("\nInvalid pulse shape chosen! Program aborting...\n");
	exit(0);
      }

      jj++;
    }
    
    // close input file
    fclose(inputfile);

  }

  // apply scattering tail
  convolve_exponential(profile, nbins * SIZE_FACTOR, scatter_time);

  // fold profile
  double* folded_profile = (double*)malloc(sizeof(double) * nbins);
  fold_profile(profile, folded_profile, nbins * SIZE_FACTOR, nbins);
  free(profile);

  // add whitenoise and baseline
  printf("Adding whitenoise and baseline...\n");

  for (i = 0; i < nbins; i++) {

    // generate baseline with noise if required

    if (whitenoise_flag == TRUE) {
      folded_profile[i] = folded_profile[i] + generateWhiteNoise(sigma, base);
    } else {
      folded_profile[i] = folded_profile[i] + base;
    }

  }

  // write to file
  for (i = 0; i < nbins; i++) {
    if (format == 1) {
      fprintf(outputfile, "%d %.10f\n", i, folded_profile[i]);
    } else if (format == 2) {
      fprintf(outputfile, "%c", (char)((int)folded_profile[i]));
    }
  }

  // file output should now be complete - close file
  fclose(outputfile);

  // cleanup
  free(folded_profile);

  printf("\nProfile generation complete.\n");  

  return 0;
  
}

// FUNCTION AREA

void pulseshape_tophat(double *profile, int size, double width, double height, int center) {

  // check validity
  assert(profile != NULL);
  assert(size > 0);
  assert(width > 0);

  int i;
  int sub_size = size/SIZE_FACTOR;

  // determine pulse start and end positions
  int pulsestart = center - (int)floor(width/((double)2));
  int pulseend = pulsestart + (int)ceil(width);

  for (i = pulsestart + SIZE_OFFSET*sub_size; i < pulseend + SIZE_OFFSET*sub_size; i++) {
    profile[i] = profile[i] + height;
  }
  
  return;
}

void pulseshape_gaussian(double *profile, int size, double width, double height, int center) {

  // check validity
  assert(profile != NULL);
  assert(size > 0);
  assert(width > 0);

  int i;
  int sub_size = size/SIZE_FACTOR;

  // convert the FWHM width to the RMS width to use in the code
  double RMSwidth = width/(2*sqrt(2*log(2)));

  // scroll through profile and add Gaussian component to the middle section of the profile
  for (i = 0; i < size; i++) {
    profile[i] = profile[i] + height*exp(-pow(((i-sub_size*SIZE_OFFSET)-center), 2)/(2*RMSwidth*RMSwidth));
  }

  return;
}

void convolve_exponential(double *profile, int size, double scatter_time) {

  // check for valid input
  assert(profile != NULL);
  assert(size > 0);
  assert(scatter_time >= 0);

  // variable setup
  int i;
  int n;
  int m;
  int sub_size = size/SIZE_FACTOR;

  // only proceed if scattering is actually needed
  if (scatter_time > 0) {
    
    // integrate source profile for initial area - needed later for normalisation
    double initial_profile_area = 0;
    for (i = 0; i < size; i++) {
      initial_profile_area = initial_profile_area + profile[i];
    }

    // build empty profile
    double* exp_profile = (double*)malloc(sizeof(double) * size);

    // insert scattered exponential profile - in reverse
    for (i = 0; i < size; i++) {
      int deltaT = (i-SIZE_OFFSET*sub_size);

      if (deltaT >= 0) {
	exp_profile[i] = exp(-deltaT/scatter_time);
      } else {
	exp_profile[i] = 0;
      }
    }

    // profile constructed - now convolve using modulo techniques
    double* convolved_profile = (double*)malloc(sizeof(double) * size);
    
    for (n = 0; n < size; n++) {
      convolved_profile[n] = 0;

      for (m = 0; m < size; m++) {
	convolved_profile[n] = convolved_profile[n] +  exp_profile[(n-m + size)%(size)] * profile[(m + size)%(size)];
      }
    }

    // then calculate area under convolved curve using rectangular approximation
    double area_convolved = 0;
    for (i = 0; i < size; i++) {
      area_convolved = area_convolved + convolved_profile[i];
    }

    // determine correction factor
    double norm_factor = initial_profile_area / area_convolved; 

    // and apply normalisation while copying the result into the original profile
    for (i = 0; i < size; i++) {
      profile[i] = convolved_profile[i] * norm_factor;
    }

    // cleanup
    free(convolved_profile);
  }

  return;
}

void fold_profile(double *input_profile, double *output_profile, int input_size, int output_size) {

  // check for valid input
  assert(input_profile != NULL);
  assert(output_profile != NULL);
  assert(input_size > 0);
  assert(output_size > 0);

  // initialise output profile
  int i;

  for (i = 0; i < output_size; i++) {
    output_profile[i] = 0;
  }

  // fold input profile

  for (i = 0; i < input_size; i++) {
    output_profile[i % output_size] = output_profile[i % output_size] + input_profile[i];
  }

  return;
}

void progeny_help() {

  printf("\nPROGENY - a program to construct artificial pulsar profiles.\n");
  printf("To be used to generate standardised tests for FFANCY normalisation scheme and metrics.\n");
  printf("Version 1.4.2, last updated 19/09/2016.\n");
  printf("Written by Andrew Cameron, MPIFR IMPRS PhD Student.\n");
  printf("\n*****\n\n");
  printf("Input options:\n");

  printf("\n----- BASIC PARAMETERS -----\n");
  printf("-nbins [int]       Number of bins in the profile.\n");
  printf("-baseline -float]  Average value of the profile baseline (off-pulse).\n");

  printf("\n----- NOISE GENERATION ----- \n");
  printf("-whitenoise        Turns on whitenoise generation.\n");
  printf("-sigma [float]     Value of sigma (only used with -whitenoise).\n");

  printf("\n----- PROFILE PARAMETERS -----\n");
  printf("-i [file]          Name of input file specifying profile paramaters. Allows for multiple components to be specified on individual lines.\n");
  printf("                   Nullifies other command line input under this section (scattering excepted).\n"); 
  printf("                   Either width (0) or duty cycle (1) can be specifie at the w/d position.\n");
  printf("                   Format: ${height} w/d ${width}/${dc} ${center} ${shape}\n\n");
  printf("-height [float]    Height of the pulse profile peak above the baseline.\n");
  printf("-width [float]     Width of the pulse profile in bins. For Gaussian profiles this represents the FWHM.\n");
  printf("-dc [float]        Alternatively specify the width in terms of duty cycle (percent).\n");
  printf("-center [int]      Bin position of the center of the pulse.\n");
  printf("-shape [int]       Selects the shape of the profile to be seeded using the following codes:\n");
  printf("                   1 - Top Hat (DEFAULT).\n");
  printf("                   2 - Gaussian.\n");
  printf("-scatter [float]   Specifies the scattering time in units of samples. Applies regardless of pulse combination chosen.\n");

  printf("\n----- OUTPUT -----\n");
  printf("-o [file]          Name of the output file.\n");
  printf("-format [int]      Sets the format of the output according to the following codes:\n");
  printf("                   1 - Writes data in PROFILE format, with two columns, one for bin number and one for the data point (DEFAULT).\n");
  printf("                   2 - Writes data in TIMESERIES format as a single continuous steam of integer values.\n");

  printf("\n----- MISCELLANEOUS -----\n");
  printf("-h / --help        Displays this useful and informative help menu.\n\n");

  return;

}













