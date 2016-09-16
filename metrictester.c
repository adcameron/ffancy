#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>

// User defined libraries
#include "metrics.h"
#include "stringsequal.h"

#define TRUE 1
#define FALSE 0

// Program to independently test the FFA metrics in isolation from the FFA
// Written by Andrew Cameron
// Version 1.1 - Last updated 11/09/2015

/*

CHANGELOG:
13/11/2015 - v1.0 - Wrote and basic testing completed
11/09/2016 - v1.1 - Updated help menu and reconfigured metric numbering for compatibility with publication
*/

// ***** FUNCTION PROTOTYPES *****

// prints out an explanation of how to use the command line interface
void metrictester_help();

// ***** MAIN FUNCTION *****

int main(int argc, char** argv) {

  // declare variables and initialise with defaults
  int metric_choice = 0;
  FILE *inputfile = NULL;

  double (*metric)(ffadata*, int, int);

  int i; // counter

  // check that a valid number of arguments have been passed
  if (argc < 2) {
    metrictester_help();
    exit(0);
  }

  // scan arguments and allocate variables
  if (argc > 1) {
    i=1;
    while (i < argc) {
      if (strings_equal(argv[i],"-a")) {
	i++;
	metric_choice = atoi(argv[i]);
      } else if (strings_equal(argv[i], "-h") || strings_equal(argv[i], "--help")) {
	metrictester_help();
	exit(0);
      } else if (strings_equal(argv[i], "-i")) {
	i++;
	inputfile = fopen(argv[i], "r");
      } else {
	printf("Unknown argument (%s) passed to Metric Tester.\nUse -h / --help to display help menu with acceptable arguments.\n",argv[i]);
	exit(0);
      }
      i++;
    }
  }

  // test for valid input
  assert(inputfile != NULL);

  // assign metric
  if (metric_choice == 3) {
    metric = basicMetric;
  } else if (metric_choice == 4) {
    metric = maxminMetric;
  } else if (metric_choice == 5) {
    metric = kondratievMetric;
  } else if (metric_choice == 6) {
    printf("Algorithm 6 has been permanently disabled. Please select a different metric.\n");
    exit(0);
  } else if (metric_choice == 7) {
    metric = integralMetric;
  } else if (metric_choice == 8) {
    metric = averageMetric;
  } else if (metric_choice == 1) {
    metric = postMadMatchedFilterMetric;
  } else if (metric_choice == 2) {
    metric = kondratievMFMetric;
  } else {
    printf("Invalid algorithm choice!\n");
    exit(0);
  }

  // intialise array based on input selection

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

  printf("Now applying Algorithm %d to profile...\n", metric_choice);

  double score = metric(dataarray, 0, size);

  // report score to command line
  printf("SCORE: %.10f\n", score);

  // cleanup
  free(dataarray);
  printf("\nMetric Tester complete.\n"); 

  return 0;
  
}

void metrictester_help() {

  printf("\nMetric Tester - a program to evaluate algorithm (metric) performance on individual PROGENY profiles.\n");
  printf("Version 1.1, last updated 11/09/2015.\n");
  printf("Written by Andrew Cameron, MPIFR IMPRS PhD Student.\n");
  printf("\n*****\n\n");
  printf("Input options:\n");

  printf("\n----- External Dataset Input ----- \n");
  printf("-i [file]      Name of the PROGENY input profile.\n");

  printf("\n----- Algorithm Selection -----\n");
  printf("-a [int]       Algorithm choice for profile evaluation:\n\n");
  printf("               -* PRIMARY ALGORITHMS *-\n");
  printf("               1 = Boxcar matched-filter with Median Absolute Deviation (MAD) normalisation.\n");
  printf("               2 = Boxcar matched-filter with off-pulse window normalisation. Based on work by Kondratiev et al. 2009 ApJ.\n\n");
  printf("               -* SECONDARY ALGORITHMS *-\n");
  printf("               3 = Basic algorithm. Returns highest value in the folded profile.\n");
  printf("               4 = Max-Min algorithm. Returns highest value - lowest value in the folded profile (weighted by sigma).\n");
  printf("               5 = Off-pulse window normalisation algorithm. Behaves as Algorithm 2, but without the matched-filter.\n");
  printf("               6 = Faster off-pulse window algorithm. Takes advantage of pre-calculated statistics in an attempt to increase speed. (DISABLED).\n");
  printf("               7 = Integration algorithm. Takes the integral of the profile minus the integral of the average (now uses Post-MAD profile normalisation).\n");
  printf("               8 = Average algorithm. Returns the difference between the total and off-peak averages (now uses Post-MAD profile normalisation).\n\n");
  printf("               NOTE: Algorithms may also be referred to as 'metrics' in source code.\n");
  printf("\n----- Miscellaneous -----\n");
  printf("-h / --help    Displays this useful and informative help menu.\n\n");

  return;

}













