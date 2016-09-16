#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>

// User defined libraries
#include "metrics.h"
#include "ffadata.h"
#include "paddedarray.h"
#include "dataarray.h"
#include "stringsequal.h"
#include "ffa.h"
#include "mad.h"

#define TRUE 1
#define FALSE 0

// Program to test an implementation of the FFA algorithm (Staelin 1969)
// Written by Andrew Cameron
// Version 1.8.3 - Last updated 11/09/2016
// Based upon earlier program ffatest4 - this program would be equivalent to Version 5.0 - see ffatest4.0 for previous changelog

/*

CHANGELOG:

29/01/2015 - v0.1 - Began conversion from previous program, ffatest4, with aim of streamlining and modularising code
03/02/2015 - v0.2 - Remodelled input to FFA functions so as to allow for modularisation, including metric passing and period selection
                  - modified input parameters to program to allow for easier period range selection
06/02/2015 - v1.0 - First full compilation using modular file structure and an associated Makefile
11/02/2015 - v1.1 - Added changes to parrot and a new ascii output function
25/02/2015 - v1.2 - Modified the power2resizer so that it no longer trims data, it always pads. This is to try and restore hyperbolic sensitivity curves from Kondratiev 2009
                  - This require changing the default padding in the dataArray from 1.35 to 2
02/03/2015 - v1.3 - Introduced an option to apply a matched filter of a given size (in samples) to folded profiles before metric evaluation
                  - THIS DOES NOT EQUATE TO THE MATCHED FILTER METRIC, although the methodology for both is similar
16/03/2015 - v1.4 - Rewrote and began testing a new form of the matched filter metric. Multiple versions to be written, each handling the statistics/normalisation in different ways.
17/03/2015        - Modified the Integral Metric (#4) to MAD normalise its profiles before calculation.
                  - Modified the Average Metric (#5) to MAD normalise its profiles before calculation.
		  - Both of these metrics now rely heavily on the validity/robustness of the MAD technique - vigorous testing of MAD is required.
20/03/2015 - v1.5 - Significant rewrite in downsampling procedures
                  - scalefactor is now an inherent part of the struct - changed so that pre-FFA downsampling is now better accounted for
08/04/2015 - v1.6 - Addition of Metric 7
                  - Repair of the MAD normalisation scheme to get around an intger math problem
10/04/2015 - v1.6.1 - Modification to downsampling loop structure within the FFA execution to correct for unforeseen effect of previous downsampling modifications
06/08/2015 - v1.7 - Implementation of a dereddening scheme using a running median filter borrowed from SigPyProc (Ewan Barr)
11/08/2015 - v1.7.1 - Implemented testing function to allow for de-reddened time series to be output after initial read in, similar to the 
12/08/2015 - v1.7.2 - Implemented float writer to match PRESTO requirements for FFT analysis of rednoise
08/09/2015 - v1.7.3 - Modified profile dump function, changed format of output for possible neural net testing
16/09/2015 - v1.7.4 - Modified massFFA handling of pre-downsampling - see changelog in ffa.c for details
16/09/2015 - v1.7.5 - Modified massFFA and singleFFA to allow for dumping of MAD normalised profiles
08/03/2016 - v1.7.6 - Added functionality to read in PRETSO time series
17/03/2016 - v1.8   - Changing the operation of the rednoise code. The de-reddening window will now be automatically determined by default.
                    - The dered window will also automatically double in size after each downsampling, relative to the original sample scale
		    - This will occur no matter which version of the dered input is chosen
14/04/2016 - v1.8.1 - Added code to normalise a time series using MAD before FFA execution - can be turned on or off
                    - Also modified the rednoise scheme
		    - Program now completes one broad de-reddening of the original time series, just once for the entire FFA.
		    - It then de-reddens again after each individual downsampling.
06/06/2016 - v1.8.2 - Clarified useage with improved help menu
11/09/2016 - v1.8.3 - Further updated the help menu
                    - Converted Algorithm notation such that published metrics are now numbered 1 & 2

FUTURE IMPROVEMENTS
* The format of the data (ASCII vs PRESTO) could be re-written to be included as a part of the struct rather than a flag passed between functions.
  The pass of this flag to massFFA was a quickfix to bring the auto de-reddening regime online, but should be tidied up later

*/

// ***** FUNCTION PROTOTYPES *****

// prints out an explanation of how to use the command line interface
void ffa_help();

// ***** MAIN FUNCTION *****

int main(int argc, char** argv) {

  // declare variables and initialise with defaults
  int samples = (int)pow(2, 15);
  int loops = 1;
  int lowperiod = 128;
  int highperiod;
  int seedperiod = 500;
  int seedwidth = 15;
  int metric_choice = 1;
  int prelim_downsamples = 0;
  FILE *inputfile = NULL;
  FILE *outputfile = NULL;
  FILE *profilefile = NULL;
  FILE *normprofilefile = NULL;
  FILE *parrotfile = NULL;
  FILE *originalfile = NULL;
  FILE *originalderedfile = NULL;
  paddedArray* sourcedata = NULL;
  int mfsize = 0;
  int dered_flag = FALSE;
  int PRESTO_flag = FALSE;
  //int SIGPYPROC_flag = FALSE;
  int user_dw_flag = FALSE;
  int dered_window = 1;
  int timenorm_flag = FALSE;

  double (*metric)(ffadata*, int, int);

  int loop_flag = FALSE;
  int hp_flag = FALSE;

  int i; // counter

  // check that a valid number of arguments have been passed
  if (argc < 2) {
    ffa_help();
    exit(0);
  }

  // scan arguments and allocate variables
  if (argc > 1) {
    i=1;
    while (i < argc) {
      if (strings_equal(argv[i],"-s")) {
	i++;
	samples = atoi(argv[i]);
      } else if (strings_equal(argv[i],"-lp")) {
	i++;
	lowperiod = atoi(argv[i]);
      } else if (strings_equal(argv[i],"-l")) {
	i++;
	loops = atoi(argv[i]);
	loop_flag = TRUE;
      } else if (strings_equal(argv[i],"-pp")) {
	i++;
	seedperiod = atoi(argv[i]);
      } else if (strings_equal(argv[i],"-pw")) {
	i++;
	seedwidth = atoi(argv[i]);
      } else if (strings_equal(argv[i],"-o")) {
	i++;
	outputfile = fopen(argv[i], "w+");
      } else if (strings_equal(argv[i],"-a")) {
	i++;
	metric_choice = atoi(argv[i]);
      } else if (strings_equal(argv[i], "-h") || strings_equal(argv[i], "--help")) {
	ffa_help();
	exit(0);
      } else if (strings_equal(argv[i], "-i")) {
	i++;
	inputfile = fopen(argv[i], "r");
      } else if (strings_equal(argv[i], "-pdump")) {
	i++;
	profilefile = fopen(argv[i], "w+");
	assert(profilefile != NULL);
      } else if (strings_equal(argv[i], "-npdump")) {
	i++;
	normprofilefile = fopen(argv[i], "w+");
	assert(normprofilefile != NULL);
      } else if (strings_equal(argv[i], "-ds")) {
	i++;
	prelim_downsamples = atoi(argv[i]);
      } else if (strings_equal(argv[i], "-parrot")) {
	i++;
	parrotfile = fopen(argv[i], "w+");
	assert(parrotfile != NULL);	
      } else if (strings_equal(argv[i], "-hp")) {
	hp_flag = TRUE;
	i++;
	highperiod = atoi(argv[i]);
      } else if (strings_equal(argv[i], "-original")) {
	i++;
	originalfile = fopen(argv[i], "w+");
	assert(originalfile != NULL);
      } else if (strings_equal(argv[i], "-original-dr")) {
	i++;
	originalderedfile = fopen(argv[i], "w+");
	assert(originalderedfile != NULL);
      } else if (strings_equal(argv[i], "-presto")) {
	PRESTO_flag = TRUE;
	//} else if (strings_equal(argv[i], "-sigpyproc")) {
	//SIGPYPROC_flag = TRUE;
      } else if (strings_equal(argv[i], "-mf")) {
	i++;
	mfsize = atoi(argv[i]);
      } else if (strings_equal(argv[i], "-dered")) {
	dered_flag = TRUE;
      } else if (strings_equal(argv[i], "-dw")) {
	i++;
	user_dw_flag = TRUE;
	dered_window = atoi(argv[i]);
      } else if (strings_equal(argv[i], "-timenorm")) {
	timenorm_flag = TRUE;
      } else {
	printf("Unknown argument (%s) passed to ffancy.\nUse -h / --help to display help menu with acceptable arguments.\n",argv[i]);
	exit(0);
      }
      i++;
    }
  }

  // populate dependent variables
  if ((loop_flag == TRUE) && (hp_flag == FALSE)) {
    highperiod = lowperiod * (int)pow(2,loops);
  } else if ((hp_flag = TRUE) && (loop_flag == FALSE)) {
    assert(highperiod > lowperiod);
  } else if ((hp_flag == TRUE) && (loop_flag == TRUE)) {
    printf("ERROR: Conflicting period options selected. Please choose either '-l' or '-hp'.\n");
    exit(0);
  }

  // calculate dered window if required
  if (user_dw_flag == FALSE && dered_flag == TRUE) {
    // if the user has not specified a dered window manually, and de-reddening is required, we must calculate one
    // the rules are:
    // 1. If the highest period to be searched is more than 2 times the lowest period to be searched, then the window
    //    will be set to 4*N + 1, where N is the size of the lowest period in samples
    // 2. Else, set to 2N+1, where N is the highest period to be searched.

    if (highperiod > 2*lowperiod) {
      dered_window = 4*lowperiod + 1;
    } else {
      dered_window = 2*highperiod + 1;
    }

  }

  // test for valid input
  assert(lowperiod >= 2); // this is the smallest possible period size for the FFA - a period of 1 results in no array shifting
  assert(highperiod > lowperiod);
  assert(outputfile != NULL);
  assert(seedperiod > seedwidth);
  if (mfsize < 0) {
    printf("Matched filter size cannot be less than zero!\n");
    exit(0);
  }
  if (dered_window <= 0) {
    printf("De-reddening window must be greater than 0!\n");
    exit(0);
  }

  // assign metric
  if (metric_choice == 3) {
    metric = basicMetric;
  } else if (metric_choice == 4) {
    metric = maxminMetric;
  } else if (metric_choice == 5) {
    metric = kondratievMetric;
  } else if (metric_choice == 6) {
    printf("Algrithm 6 has been permanently disabled. Please select a different metric.\n");
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
  if (inputfile != NULL) {

    // input file selected - create data array from file   
    printf("Input file selected. Reading file...\n");
    if (PRESTO_flag == FALSE) {
      printf("ASCII format selected.\n");
      sourcedata = readASCIIDataArray(inputfile);
    } else if (PRESTO_flag == TRUE) {
      printf("PRESTO format selected.\n");
      sourcedata = readFloatDataArray(inputfile);
    }
    setRedFlag(sourcedata, dered_flag);
    setWindow(sourcedata, dered_window);

    printf("Input file read successfully - %d samples in length.\n", getPaddedArrayDataSize(sourcedata));
    fclose(inputfile);
    assert(getPaddedArrayDataSize(sourcedata) > highperiod);

    // re-write input array to external file if requested
    if (parrotfile != NULL) {
      printf("Writing parrot file...\n");
      writePaddedArray(parrotfile, sourcedata);
      fclose(parrotfile);
    }

    if (originalfile != NULL) {
      printf("Writing copy of original file...\n");
      if (PRESTO_flag == FALSE) {
	writeASCIIDataArray(originalfile, sourcedata);
      } else if (PRESTO_flag == TRUE) {
	writeFloatDataArray(originalfile, sourcedata);
      }
      fclose(originalfile);
    }
    
  } else {
    
    // generate samples manually
    printf("Generating %d samples...\n", samples);

    // create data array
    sourcedata = basicPulsarDataArray(samples, seedperiod, seedwidth);
    printf("%d samples generated\n", getPaddedArrayDataSize(sourcedata));

  }

  // normalise if required
  if (timenorm_flag == TRUE) {
    // run MAD on sourcedata using only the datasize
    mad(getPaddedArrayDataArray(sourcedata), getPaddedArrayDataSize(sourcedata));
    printf("Time series normalised via MAD.\n");
  }

  // perform first-pass de-reddening if required
  if (dered_flag == TRUE) {
    // need to deredden with a window equal to twice the longest period
    int new_dr_window = highperiod *2 + 1;
    int old_dr_window = getWindow(sourcedata);
    setWindow(sourcedata, new_dr_window);

    paddedArray* tempdata = dereddenDataArray(sourcedata);

    /*
    // we now need to subtract tempdata from sourcedata, and then carry on
    paddedArray* difference = subtractPaddedArray(sourcedata, tempdata);

    // this is the data we will use going forward - allocate and cleanup
    deletePaddedArray(sourcedata);
    deletePaddedArray(tempdata);
    sourcedata = difference;
    */

    deletePaddedArray(sourcedata);
    sourcedata = tempdata;
    setWindow(sourcedata, old_dr_window);
  }

  printf("Now scanning from a period of %d samples to %d original samples...\n", lowperiod, highperiod);

  // now ready to begin FFA

  massFFA(outputfile, profilefile, normprofilefile, sourcedata, lowperiod, highperiod, metric, mfsize, prelim_downsamples, originalderedfile, PRESTO_flag, timenorm_flag);

  // file I/O should now be complete - close files
  fclose(outputfile);
  if (profilefile != NULL) {
    fclose(profilefile);
  }
  if (normprofilefile != NULL) {
    fclose(normprofilefile);
  }
  if (originalderedfile != NULL) {
    fclose(originalderedfile);
  }

  // cleanup
  deletePaddedArray(sourcedata);

  printf("\nFFA complete.\n");
 
  return 0;
  
}

void ffa_help() {

  printf("\nFFAncy - a testbed program for the Fast Folding Algorithm (FFA) (Staelin 1969).\n");
  printf("Version 1.8.3, last updated 11/09/2016.\n");
  printf("Based on earlier testing program 'ffatest4', now retired.\n");
  printf("Written by Andrew Cameron, MPIFR IMPRS PhD Student.\n");
  printf("\n*****\n\n");
  printf("Input options:\n");

  printf("\n----- Simple Dataset Generation -----\n");
  printf("-s [int]             Number of samples to generate for test dataset.\n");
  printf("-pp [int]            Period (in samples) of the fake pulsar to be seeded into the test dataset.\n");
  printf("-pw [int]            Pulse width (in samples) of the fake pulsar.\n");

  printf("\n----- External Dataset Input ----- \n");
  printf("-i [file]            Name of the input file (this deactivates internal data generation and makes most other data seeding parameters redundant).\n");
  printf("                     Default input format is 8-bit unsigned integers, as produced by SIGPROC package dedisperse_all.\n");
  printf("-presto              Changes the input format to a 32-bit (single precision float) PRESTO time series.\n");

  printf("\n----- De-reddening & Normalisation ----- \n");
  printf("-dered               Turn on de-reddening. This is applied both during data initialisation and during each subsequent downsampling.\n");
  printf("                     By default, the de-reddening window is set to 2N+1, where N is th largest period trial that will be run before downsampling.\n");
  printf("                     After each downsampling, the de-reddening window is doubled in size with respect to the original data.\n");
  printf("-dw [int]            (Optional) Manually set initial window for de-reddening, in units of the original sample size.\n");
  printf("-timenorm            EXPERIMENTAL - normalises a time series both pre and post downsampling using MAD.\n");

  printf("\n----- Output -----\n");
  printf("-o [file]            Name of the primary output file which stores period vs. metric data in a GNUPLOT friendly format.\n");
  printf("-pdump [file]        Name of the profile dump file, which stores each individual folded profile.\n");
  printf("-npdump [file]       Same as -pdump, except that output profiles have been normalised via MAD.\n");
  printf("-parrot [file]       Name of file to re-write padded data to after initial data initialisation (writes in GNUPLOT format, used for testing purposes).\n\n");

  printf("-original [file]     Name of file to re-write original input file to, in either SIGPROC or PRESTO format (used for testing purposes).\n");
  printf("-original-dr [file]  Name of file to write the de-reddened version of original input file to (used for testing purposes).\n");
  printf("                     Only activated if '-dered' flag is set.\n");

  printf("\n----- FFA Execution -----\n");
  printf("-ds [int]            Deteremines the number of downsampling loops of the input file to execute before running the FFA.\n");
  printf("-mf [int]            Applies a matched filter of a specified size (in samples) to folded profiles before algorithm evaluation.\n");
  printf("-lp [int]            The lowest period to test for, in units of samples.\n");
  printf("                     NOTE: If -ds is used, period specified by -lp should be an integer multiple of -ds to ensure correct FFA execution.\n\n");

  printf("                     NOTE: Only one of the following options may be chosen:\n");
  printf("-l [int]             Number of downsampling loops to execute during FFA execution (tests periods from [lp] to [lp * 2^l].\n");
  printf("-hp [int]            The highest period to test for, in units of samples (downsampling may still occur if [hp] > [2*lp]).\n\n");

  printf("-a [int]             Algorithm choice for profile evaluation:\n\n");
  printf("                     -* PRIMARY ALGORITHMS *-\n");
  printf("                     1 = Boxcar matched-filter with Median Absolute Deviation (MAD) normalisation.\n");
  printf("                     2 = Boxcar matched-filter with off-pulse window normalisation. Based on work by Kondratiev et al. 2009 ApJ.\n\n");
  printf("                     -* SECONDARY ALGORITHMS *-\n");
  printf("                     3 = Basic algorithm. Returns highest value in the folded profile.\n");
  printf("                     4 = Max-Min algorithm. Returns highest value - lowest value in the folded profile (weighted by sigma).\n");
  printf("                     5 = Off-pulse window normalisation algorithm. Behaves as Algorithm 2, but without the matched-filter.\n");
  printf("                     6 = Faster off-pulse window algorithm. Takes advantage of pre-calculated statistics in an attempt to increase speed. (DISABLED).\n");
  printf("                     7 = Integration algorithm. Takes the integral of the profile minus the integral of the average (now uses Post-MAD profile normalisation).\n");
  printf("                     8 = Average algorithm. Returns the difference between the total and off-peak averages (now uses Post-MAD profile normalisation).\n\n");
  printf("                     NOTE: Algorithms may also be referred to as 'metrics' in source code.\n");
  printf("\n----- Miscellaneous -----\n");
  printf("-h / --help          Displays this useful and informative help menu.\n\n");

  return;

}













