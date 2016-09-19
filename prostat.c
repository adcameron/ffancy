#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>

// User defined libraries
#include "stats.h"
#include "equalstrings.h"

#define TRUE 1
#define FALSE 0

#define STATS_OUT "stats.out"

// PROSTAT - Program to run statistical anaylsis on profiles
// Written by Andrew Cameron
// Version 1.1 - Last updated 25/03/2015

/*

CHANGELOG:

20/03/2015 - v0.1 - Began writing program, modifying home file from progeny.c
24/03/2015 - v1.0 - Operatonal version complete
25/03/2015 - v1.1 - Implemented exclusion window for studying baseline statistics
                  - Impelemtned stats dump to file "stats.out"
11/09/2016 - v1.2 - Updated help menu for publication
*/

// ***** FUNCTION PROTOTYPES *****

// prints out an explanation of how to use the command line interface
void prostat_help();

// ***** MAIN FUNCTION *****

int main(int argc, char** argv) {

  // declare variables and initialise with defaults
  FILE *inputfile = NULL;
  FILE *outputfile = fopen(STATS_OUT, "w+");
  int exclusion_flag = FALSE;
  int startbin = 0;
  int endbin = 1;

  int i; // counter

  // check that a valid number of arguments have been passed
  if (argc < 2) {
    prostat_help();
    exit(0);
  }

  // scan arguments and allocate variables
  if (argc > 1) {
    i=1;
    while (i < argc) {
      if (equal_strings(argv[i],"-i")) {
	i++;
	inputfile = fopen(argv[i], "r");
      } else if (equal_strings(argv[i], "-ew")) {
	i++;
	startbin = atoi(argv[i]);
	i++;
	endbin = atoi(argv[i]);
	exclusion_flag = TRUE;
      } else if (equal_strings(argv[i], "-h") || equal_strings(argv[i], "--help")) {
	prostat_help();
	exit(0);
      } else {
	printf("Unknown argument (%s) passed to PROSTAT.\nUse -h / --help to display help menu with acceptable arguments.\n",argv[i]);
	exit(0);
      }
      i++;
    }
  }

  // test for valid input
  assert(inputfile != NULL);
  assert(outputfile != NULL);
  if (exclusion_flag == TRUE) {
    assert(endbin >= startbin);
  }

  int bin;
  double value;
  int size = 0;  

  // work out how big the file is
  while(fscanf(inputfile, "%d %lf", &bin, &value) != EOF) {
    size++;
  }

  printf("\nFile is %d lines long.\n", size);

  // prepare for actual file read
  rewind(inputfile);

  // allocate sufficient memory
  double* dataarray = (double*)malloc(sizeof(double) * size);

  // read data
  i = 0;
  while(fscanf(inputfile, "%d %lf", &bin, &value) != EOF) {
    dataarray[i] = value;
    i++;
  }

  // close input file
  fclose(inputfile);
  printf("\n*** FILE READ COMPLETE ***\n\n");

  // now need to handle an exclusion window, assuming that one has been specified
  if (exclusion_flag == TRUE) {
    printf("Applying exclusion window...");
    // make a copy of the array with only the data outside the window
    int windowsize = endbin - startbin + 1;
    int newsize = size - windowsize;

    double* smallarray = (double*)malloc(sizeof(double) * newsize);

    int j = 0;
    int k = 0;
    while (j < size) {
      if (j < startbin || j > endbin) {
	smallarray[k] = dataarray[j];
	k++;
      }
      j++;
    }

    // cleanup
    free(dataarray);
    dataarray = smallarray;
    size = newsize;
    printf("complete.\n\n");
  }

  double dmax = max(dataarray, size);
  double dmin = min(dataarray, size);
  double dmean = mean(dataarray, size);
  double dmedian = median(dataarray, size);
  double dsigma = stddev(dataarray, size);

  // now perform stats and write out
  printf("Statistics:\n\n");
  printf("Max is: %f\n", dmax);
  printf("Min is: %f\n", dmin);
  printf("Mean is: %f\n", dmean);
  printf("Median is: %f\n", dmedian);
  printf("Std. dev is: %f\n", dsigma);

  fprintf(outputfile, "Max %f\n", dmax);
  fprintf(outputfile, "Min %f\n", dmin);
  fprintf(outputfile, "Mean %f\n", dmean);
  fprintf(outputfile, "Median %f\n", dmedian);
  fprintf(outputfile, "Sigma %f\n", dsigma);

  // cleanup
  free(dataarray);
  fclose(outputfile);
  printf("\nPROSTAT complete.\n");  

  return 0;
  
}

void prostat_help() {

  printf("\nPROSTAT - Program to run statistical anaylsis on profiles.\n");
  printf("Writes statistics summary to file 'stats.out'\n");
  printf("Version 1.2, last updated 11/09/2016.\n");
  printf("Written by Andrew Cameron, MPIFR IMPRS PhD Student.\n");
  printf("\n*****\n\n");
  printf("Input options:\n");

  printf("\n----- BASIC PARAMETERS -----\n");
  printf("-i [file]        Name of the PROGENY format input file.\n");

  printf("\n----- DATA SELECTION -----\n");
  printf("-ew [int] [int]  Specifies an exclusion window using two integers - the first and last bins of data\n");
  printf("                 to be excluded from statistical calculations (inclusive).\n");
  printf("                 E.g. For an array of 10 elements, bins are numbered 0 to 9.\n");

  printf("\n----- MISCELLANEOUS -----\n");
  printf("-h / --help      Displays this useful and informative help menu.\n\n");

  return;

}













