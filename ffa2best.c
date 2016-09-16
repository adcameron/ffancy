#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>

// user defined libraries
#include "stringsequal.h"

#define TRUE 1
#define FALSE 0

#define HEADER_STRINGS 13
#define DCMAX_MAD_ALG 50
#define DCMAX_KOND_ALG 20

// Algorithm definitions
#define MAD_ALG 1
#define KOND_ALG 2

// Program to convert ffa periodogram output from FFAncy into BEST format files
// Based upon the tcsh script ffa2best.csh
// Written by Andrew Cameron
// Version 1.2.3 - Last updated 29/04/2016

// CHANGELOG
// 26/04/2016 - v1.1.0 - Added a candidate combiner (algorithm dependent), which groups together nearby peaks that are likely to be related.
//                     - Can factor in knowledge of the pulsar duty cycle being searched for.
//                     - Requires a first pass through a temporary file because I'm too lazy to set up a linked list
// 27/04/2016 - v1.2.0 - Added harmonic matching capability, using prime factors to weed out harmonic peaks, favoring the strongest unrelated peaks to remain
// 29/04/2016 - v1.2.1 - Added 'ranked' flag which outputs peaks in order of highest to lowest SNR
// 12/07/2016 - v1.2.2 - Fixed a bug in the peak finder identified by Ewan Barr
// 11/09/2016 - v1.2.3 - Updated and clarified help menu for publication
//                     - Changed accepted algorithms from 6 & 7 to 1 & 2 as per paper notation

// ***** FUNCTION PROTOTYPES *****

// prints out an explanation of how to use the command line interface
void help();

// writes out a peak to file
void writePeak(FILE *file, double period, double snr, float tsamp);

// reads a peak from a file - basically a mask for fscanf
int readPeak(FILE *file, double *period, double *snr);

// extracts the raw peaks and stores them in a file - returns the number of peaks found
int rawPeakFinder(FILE *inputfile, FILE *outputfile, double thresh, double lthresh, double dthresh, float tsamp);

// conducts the peak combining process, also includes the harmonic process if activated, returns number of peaks remaining
int peakCombiner(FILE *inputfile, FILE *outputfile, int npeaks, float pulsar_dc, float max_dc, float tobs, float tsamp, int harmonic_flag, int highprime, float tolerance);

// calculates the combining window around a given peak
double peakWindow(double period, double tobs, double pulsar_dc, double max_dc);

// writes the peaks out to file in a ranked or unranked fashion
void rankedWriter(FILE *inputfile, FILE *outputfile, int npeaks, float tsamp, int ranked);

// calculates the next prime number after the one given
int nextPrime(int prime);

// returns TRUE or FALSE depending on whether two periods match the provided ratio to within the provided tolerance
int harmonicMatch(double sourceperiod, double checkperiod, double ratio, double tolerance);

// ***** MAIN FUNCTION *****

int main (int argc, char** argv) {

  // declare variables and initialise with defaults
  FILE *inputfile = NULL;
  FILE *outputfile = NULL;
  char tempname1[] = "ffa2best.temp1.prd";
  char tempname2[] = "ffa2best.temp2.prd";
  FILE *tempfile1 = NULL;
  FILE *tempfile2 = NULL;
  float dm = 0;
  float acc = 0;
  float thresh = 10;
  float lthresh = thresh - 1;
  float dthresh = 0.2;
  float tsamp = 64; // units of microseconds
  float pulsar_dc = 1;
  float max_dc = DCMAX_MAD_ALG;
  float tobs = 4300;
  int algorithm = MAD_ALG;
  int highprime = 3;
  float tolerance = 1;

  // flags
  int COMBINE_FLAG = FALSE;
  int HARMONIC_FLAG = FALSE;
  int RANKED_FLAG = FALSE;
  

  // counters
  int i;

  // scan arguments and allocate variables
  if (argc > 1) {
    i=1;
    while (i < argc) {
      if (strings_equal(argv[i],"-i")) {
        i++;
        inputfile = fopen(argv[i], "r");
      } else if (strings_equal(argv[i],"-o")) {
        i++;
        outputfile = fopen(argv[i], "w+");
      } else if (strings_equal(argv[i],"-dm")) {
        i++;
        dm = atof(argv[i]);
      } else if (strings_equal(argv[i],"-a")) {
        i++;
        acc = atof(argv[i]);
      } else if (strings_equal(argv[i],"-tsamp")) {
        i++;
        tsamp = atof(argv[i]);
      } else if (strings_equal(argv[i],"-thresh")) {
	i++;
	thresh = atof(argv[i]);
      } else if (strings_equal(argv[i],"-lthresh")) {
	i++;
	lthresh = atof(argv[i]);
      } else if (strings_equal(argv[i],"-dthresh")) {
        i++;
        dthresh = atof(argv[i])/100;
      } else if (strings_equal(argv[i],"-combine")) {
        COMBINE_FLAG = TRUE;
      } else if (strings_equal(argv[i],"-dc")) {
        i++;
        pulsar_dc = atof(argv[i]);
      } else if (strings_equal(argv[i],"-algorithm")) {
        i++;
        algorithm = atoi(argv[i]);
      } else if (strings_equal(argv[i],"-tobs")) {
        i++;
        tobs = atof(argv[i]);
      } else if (strings_equal(argv[i],"-harmonics")) {
        HARMONIC_FLAG = TRUE;
      } else if (strings_equal(argv[i],"-ranked")) {
        RANKED_FLAG = TRUE;
      } else if (strings_equal(argv[i],"-highprime")) {
        i++;
        highprime = atoi(argv[i]);
      } else if (strings_equal(argv[i],"-tolerance")) {
        i++;
        tolerance = atof(argv[i]);
      } else if (strings_equal(argv[i], "-h") || strings_equal(argv[i], "--help")) {
        help();
        exit(0);
      } else {
        printf("Unknown argument (%s) passed to ffa2best.\nUse -h / --help to display help menu with acceptable arguments.\n",argv[i]);
        exit(0);
      }
      i++;
    }
  } else {
    help();
    exit(0);
  }

  // test for valid input
  assert(inputfile != NULL);
  assert(outputfile != NULL);
  assert(dm >= 0);
  assert(acc >= 0);
  assert(tsamp > 0);
  assert(thresh > 0);
  assert(thresh > lthresh);
  assert(dthresh < 1);
  assert(tobs > 0);
  assert(pulsar_dc >= 0);
  assert(algorithm == MAD_ALG || algorithm == KOND_ALG);

  // setup the algorithm max_dc if selected
  if (algorithm == MAD_ALG) {
    max_dc = DCMAX_MAD_ALG;
  } else if (algorithm == KOND_ALG) {
    max_dc = DCMAX_KOND_ALG;
  }

  // echo input back to user to verify
  printf("Launching ffa2best: DM = %.2f | ACC = %.2f | TSAMP = %.2f | THRESHOLD = %.2f | LOWER THRESHOLD = %.2f | DYNAMINC THRESHOLD = %.2f\n", dm, acc, tsamp, thresh, lthresh, dthresh);
  if (COMBINE_FLAG == TRUE) {
    printf("PEAK COMBINING ACTIVATED: Pulsar DC = %.2f | Algorithm = %d | Tobs = %.3f\n", pulsar_dc, algorithm, tobs);
  }
  if (HARMONIC_FLAG == TRUE) {
    printf("HARMONIC MATCHING ACTIVATED: High prime = %d | Tolerance = %.5f\n", highprime, tolerance);
  }

  // print first line of outputfile to output
  fprintf(outputfile, " DM:   %.5f      AC:   %.6f      AD:   0.00000000\n", dm, acc);

  // prepare the tempfiles for use
  tempfile1 = fopen(tempname1, "w+");
  assert(tempfile1 != NULL);
  
  // conduct the first scan to get peaks
  int peaks = rawPeakFinder(inputfile, tempfile1, thresh, lthresh, dthresh, tsamp);
  fclose(tempfile1);

  // reopen tempfile1 to be read
  tempfile1 = fopen(tempname1, "r");
  assert(tempfile1 != NULL);

  // now check for the combine flag
  if (COMBINE_FLAG == TRUE) {

    tempfile2 = fopen(tempname2, "w+");
    assert(tempfile2 != NULL);
  
    // conduct second pass
    peaks = peakCombiner(tempfile1, tempfile2, peaks, pulsar_dc, max_dc, tobs, tsamp, HARMONIC_FLAG, highprime, tolerance);

    // close tempfile1 and reallocate pointers
    fclose(tempfile1);
    fclose(tempfile2);
    tempfile1 = fopen(tempname2, "r");
    assert(tempfile1 != NULL);
  }

  // resulting peaks should now be stored in tempfile1
  // write out to output file, keeping in mind the status of RANKED_FLAG
  rankedWriter(tempfile1, outputfile, peaks, tsamp, RANKED_FLAG);

  /*
  // chose which procedure to follow based on the combine flag
  if (COMBINE_FLAG == FALSE) {
    // skip the tempfile and go straight to the outputfile

    // conduct the scan
    rawPeakFinder(inputfile, outputfile, thresh, lthresh, dthresh, tsamp);
    
  } else if (COMBINE_FLAG == TRUE) {
    // first pass writes out to a temporary file which stores the raw peaks before they are later combined
    // open temp file for writing
    tempfile = fopen(tempname, "w+");
    assert(tempfile != NULL);

    // conduct the first scan using the tempfile
    int peaks = rawPeakFinder(inputfile, tempfile, thresh, lthresh, dthresh, tsamp);

    // first pass now complete - now close tempfile and reopen for reading
    fclose(tempfile);
    tempfile = fopen(tempname, "r");
    assert(tempfile != NULL);

    // conduct candidate combining
    peakCombiner(tempfile, outputfile, peaks, pulsar_dc, max_dc, tobs, tsamp, HARMONIC_FLAG, highprime, tolerance);
    
    // second scan complete
    fclose(tempfile);
    
  }
  */
 
  // I/O complete
  fclose(inputfile);
  fclose(outputfile);
  
  return 0;

}

// FUNCTION BODIES

void help() {

  printf("\nffa2best - a program to convert FFAncy periodogram output into BEST format files.\n");
  printf("Version 1.2.3, last updated 11/09/2016.\n");
  printf("Written by Andrew Cameron, MPIFR IMPRS PhD Student.\n");
  printf("\n*****\n\n");
  printf("Input options:\n");

  printf("-i [file]           Name of the periodogram file to be converted.\n");
  printf("-o [file]           Name of the output file.\n");
  printf("-dm [float]         The DM at which the periodogram was produced. (default = 0)\n");
  printf("-a [float]          The acceleration at which the periodogram was produced (ms^-2). (default = 0)\n");
  printf("-tsamp [float]      The sample time of the original time series used to produce periodogram (us). (default = 64)\n");
  printf("-thresh [float]     The signal to noise cutoff used to filter candidates from the periodogram. Units of SNR. (default = 10)\n");
  printf("-lthresh [float]    The lower signal to noise cutoff used to control the separation of separate peaks. Units of SNR. By default set to [thresh] - 1.\n");
  printf("-dthresh [float]    (Optional) The dynaminc threshhold used to control the selection of separate peaks. Units of SNR (percent), eg, 20. (default = 20)\n");

  printf("\n----- Peak combining options -----\n");
  printf("-combine            Stand alone flag which turns on peak combining functionality - following arguments will be otherwise ignored.\n");
  printf("-dc [float]         Duty cycle being searched for (percent), eg, 5. (default = 1)\n");
  printf("-tobs [float]       Length of the observation (s). (default = 4300)\n");
  printf("-algorithm [int]    FFAncy algorithm used in creating the periodogram (either 1 or 2, default 1).\n");

  printf("\n----- Harmonic matching options -----\n");
  printf("-harmonics          Stand alone flag which turns on harmonc matching functionality - following arguments will be otherwise ignored.\n(Only works with -combine active).\n");
  printf("-highprime [int]    The highest prime factor to be used when searching for harmonics.\n");
  printf("                    A higher number will result in more aggressive harmonic matching (default = 3).\n");
  printf("-tolerance [float]  Percent window within which a harmonic can called a match (default = 1).\n");

  printf("\n----- Miscellaneous -----\n");
  printf("-ranked             Outputs the peaks ranked in order of SNR, highest to lowest.\n");
  printf("-h / --help         Displays this useful and informative help menu.\n\n");

  return;

}

void writePeak(FILE *file, double period, double snr, float tsamp) {

  // check valid input
  assert(file != NULL);
  assert(tsamp > 0);
  
  double real_period = period * tsamp / 1000;
  //printf("Writing %f %f\n", snr, real_period); 
  fprintf(file, "    %.1f    %.8f \n", snr, real_period);
  
  return;
  
}

int rawPeakFinder(FILE *inputfile, FILE *outputfile, double thresh, double lthresh, double dthresh, float tsamp) {

  // check for valid input
  assert(inputfile != NULL);
  assert(outputfile != NULL);
  assert(thresh > 0);
  assert(thresh > lthresh);
  assert(dthresh < 1);

  // setup variables for reading file
  double snr;
  double period;
  int ds_factor;
  double ds_period;
  int i;

  // setup variables for computing peaks
  double highest_snr;
  double highest_period;
  double trough_snr;
  int peak_counter = 0;

  // setup flags
  int ABOVE_THRESHOLD = FALSE; // determines whether or not we are interested in scanning for peaks
  int PEAK_FLAG = FALSE;
  
  
  // commence reading file
  // need to skip the first line
  char header_string[50];
  for (i = 0; i < HEADER_STRINGS; i++) {
    fscanf(inputfile, "%s", header_string);
    // removes the first line from the scan
  }

  i = 0;
  while (fscanf(inputfile, "%lf %d %lf %lf", &period, &ds_factor, &ds_period, &snr) != EOF) {
    // now begins the algorithm proper

    // check if we are above the cutoff
    if (ABOVE_THRESHOLD == TRUE) {

      // need to determine which case we are in
      
      if (snr < lthresh) {
	// the obvious case - if we've fallen below the lower threshold, we're done
	// write out the peak, unless we've already done so
	if (PEAK_FLAG == FALSE) {
	  writePeak(outputfile, highest_period, highest_snr, tsamp);
	  peak_counter++;
	}
	ABOVE_THRESHOLD = FALSE;
	PEAK_FLAG = FALSE;
      } else if (snr > highest_snr && PEAK_FLAG == FALSE) {
	// we haven't yet reached a peak, and we're still climbing
	highest_snr = snr;
	highest_period = period;
      } else if (snr < (1-dthresh)*highest_snr && PEAK_FLAG == FALSE) {
	// we have fallen sufficiently far from the highest_snr to classify it as its own peak
	PEAK_FLAG = TRUE;
	trough_snr = snr;
	writePeak(outputfile, highest_period, highest_snr, tsamp);
	peak_counter++;
      } else if (PEAK_FLAG == TRUE && snr < trough_snr) {
	// we have fallen deeper into the valley since the previous peak
	trough_snr = snr;
      } else if (PEAK_FLAG == TRUE && snr > (1+dthresh)*trough_snr && snr > thresh) {
	// we have climbed sufficiently far out of the valley to call the new ridge its own peak
	PEAK_FLAG = FALSE;
	highest_snr = snr;
	highest_period = period;
      }
      

    } else if (ABOVE_THRESHOLD == FALSE) {

      // determine if we have just now crossed the threshold
      if (snr > thresh) {
	// threshold crossed
	highest_snr = snr;
	highest_period = period;
	ABOVE_THRESHOLD = TRUE;
      }

      // if we have not crossed the threshold, nothing to be done
    }
    
    i++;
  }
  
  return peak_counter;
  
}

int readPeak(FILE *file, double *period, double *snr) {

  // check for valid input
  assert(file != NULL);
  assert(period != NULL);
  assert(snr != NULL);
  
  return fscanf(file, "%lf %lf", snr, period);
  
}

int peakCombiner(FILE *inputfile, FILE *outputfile, int npeaks, float pulsar_dc, float max_dc, float tobs, float tsamp, int harmonic_flag, int highprime, float tolerance) {

  // check for valid input
  assert(inputfile != NULL);
  assert(outputfile != NULL);
  assert(pulsar_dc >= 0);
  assert(max_dc >= 0);
  assert(max_dc > pulsar_dc);

  // variable setup
  int ii;
  double periods[npeaks];
  double snrs[npeaks];
  int active_peaks[npeaks];
  int checked_peaks[npeaks];

  double max_period;
  double max_snr;
  int max_position;

  double min_period;
  double min_snr;
  int min_position;

  // read peaks into arrays and activate all peaks
  for (ii = 0; ii < npeaks; ii++) {
    readPeak(inputfile, &periods[ii], &snrs[ii]);
    active_peaks[ii] = TRUE;
    checked_peaks[ii] = FALSE;
  }

  // now scan through peaks heirarchically, removing peaks until the process stops
  int total_checked = 0;
  while (total_checked < npeaks) {
    // scan the array and find the max entry that hasn't been checked
    max_snr = 0;
    
    for (ii = 0; ii < npeaks; ii++) {
      // has the current entry been checked?
      if (checked_peaks[ii] == FALSE) {
	// is this the first non-checked peak found?
	if (max_snr == 0) {
	  // this peak automatically gets to be top dog
	  max_snr = snrs[ii];
	  max_period = periods[ii];
	  max_position = ii;
	} else {
	  // else another peak has already been allocated - compare and decide
	  if (snrs[ii] > max_snr) {
	    // this peak wins
	    max_snr = snrs[ii];
	    max_period = periods[ii];
	    max_position = ii;
	  }
	}
      }
    }

    // we now have the max peak that has not been checked
    // calculate the window
    double window = peakWindow(max_period, tobs, pulsar_dc, max_dc);
    //printf("Window: period = %f | tobs = %f | pulsar_dc = %f | max_dc = %f | window = %f\n", max_period, tobs, pulsar_dc, max_dc, window);

    // find any unchecked peaks within the window and deactivate + check them
    for (ii = 0; ii < npeaks; ii++) {
      // does the period fall within the window (also check that we're not considering the max peak against itself)
      if ((fabs(periods[ii] - max_period) < window) && (ii != max_position) && (checked_peaks[ii] != TRUE)) {
	// deactivate and check the peak
	checked_peaks[ii] = TRUE;
	active_peaks[ii] = FALSE;
	total_checked++;
      }
    }

    // now mark current peak as being checked (but not deactivated)
    checked_peaks[max_position] = TRUE;
    total_checked++;
  }

  // now we need to run the harmonic filtering, if requested
  if (harmonic_flag == TRUE) {

    // do harmonic stuff in here
    // need to scan through the list of active peaks again, deactivating any weak ones which display a harmonic match
    // uncheck the still active peaks
    total_checked = 0;
    for (ii = 0; ii < npeaks; ii++) {
      if (active_peaks[ii] == TRUE) {
	checked_peaks[ii] = FALSE;
      } else {
	total_checked++;
      }
    }
    
    while(total_checked < npeaks) {
      // need to go by weakest peak first
      // scan the array and find the min entry that hasn't been checked
      min_snr = 0;
    
      for (ii = 0; ii < npeaks; ii++) {
	// has the current entry been checked?
	if (checked_peaks[ii] == FALSE) {
	  // is this the first non-checked peak found?
	  if (min_snr == 0) {
	    // this peak automatically gets to be top dog
	    min_snr = snrs[ii];
	    min_period = periods[ii];
	    min_position = ii;
	  } else {
	    // else another peak has already been allocated - compare and decide
	    if (snrs[ii] < min_snr) {
	      // this peak wins
	      min_snr = snrs[ii];
	      min_period = periods[ii];
	      min_position = ii;
	    }
	  }
	}
      }

      // min snr has now been found - now we need to check for harmonic relationships with the remaining peaks
      // need to do this using rolling prime fractions
      int prime = 2;
      int match = FALSE;
      int new_match = FALSE;
      while (prime <= highprime && match == FALSE) {

	int numdom = 1;
	// numdom forms the numerator or the denominator to the current prime, depending on whether the period being checked is bigger or small than the min_period
	while (numdom < prime) {

	  for (ii=0; ii < npeaks; ii++) {
	    // we are now scanning through the peak list
	    // check to see if the peak hasn't already been checked
	    if (checked_peaks[ii] == FALSE && ii != min_position) {
	      // determine whether the peak is at a higher or lower period
	      if (periods[ii] < min_period) {
		// check to see if the harmonic match is close enough
		new_match = harmonicMatch(min_period, periods[ii], ((double)numdom)/((double)prime), tolerance);
	      } else if (periods[ii] > min_period) {
		new_match = harmonicMatch(min_period, periods[ii], ((double)prime)/((double)numdom), tolerance);
	      }
	    }

	    if (new_match == TRUE) {
	      match = TRUE;
	    }
	  }
	  
	  numdom++;
	  
	}
	
	prime = nextPrime(prime);
	
      }

      // check to see if a match was found
      if (match == TRUE) {
	active_peaks[min_position] = FALSE;
      }
      checked_peaks[min_position] = TRUE;
      total_checked++;
    }
  }

  int return_npeaks = 0;
  // we should now have a list of the remaining active peaks - write to file
  for (ii = 0; ii < npeaks; ii++) {
    //printf("PEAK EVALUATED: Period = %f | SNR = %f | ii = %d | ACTIVE = %d | CHECKED = %d\n", periods[ii], snrs[ii], ii, active_peaks[ii], checked_peaks[ii]);
    if (active_peaks[ii] == TRUE) {
      //printf("Writing out period = %f | snr = %f\n", periods[ii], snrs[ii]);
      writePeak(outputfile, periods[ii]*1000/64, snrs[ii], tsamp);
      return_npeaks++;
    }
  }
  
  return return_npeaks;

}

double peakWindow(double period, double tobs, double pulsar_dc, double max_dc) {

  // check valid input
  assert(period > 0);
  assert(tobs > 0);
  assert(pulsar_dc >= 0);
  assert(max_dc >= 0);
  assert(max_dc > pulsar_dc);
  
  return period*period*(max_dc - pulsar_dc)/(tobs * 1000 * 100);
  
}

int nextPrime(int prime) {

  // check valid input
  assert(prime > 0);

  int check = FALSE;
  int ii;
  int new_prime = prime;
  
  while (check == FALSE) {
    new_prime++;
    check = TRUE;
    
    for (ii = 2; ii <= sqrt(new_prime); ii++) {
      if (new_prime % ii == 0) {
	check = FALSE;
      }
    }
  }

  return new_prime;
  
}

int harmonicMatch(double sourceperiod, double checkperiod, double ratio, double tolerance) {
  
  // check valid input
  assert(sourceperiod > 0);
  assert(checkperiod > 0);
  assert(ratio > 0);
  assert(tolerance > 0);

  int return_val;
  double result = fabs(((sourceperiod * ratio) - checkperiod)*100/checkperiod);

  //printf("Matching %f to %f with a ratio of %f and a tolerance of %f | Result = %f | ", sourceperiod, checkperiod, ratio, tolerance, result);

  if (result < tolerance) {
    return_val = TRUE;
  } else {
    return_val = FALSE;
  }
  
  //printf("Return val = %d\n", return_val);
  return return_val;
  
}

void rankedWriter(FILE *inputfile, FILE *outputfile, int npeaks, float tsamp, int ranked) {

  // check valid input
  assert(inputfile != FALSE);
  assert(outputfile != FALSE);
  assert(npeaks > 0);
  assert(ranked == TRUE || ranked == FALSE);
  
  // read the peaks into arrays
  int ii;
  double periods[npeaks];
  double snrs[npeaks];
  int checked_peaks[npeaks];

  double max_snr;
  int max_position;

  for (ii = 0; ii < npeaks; ii++) {
    readPeak(inputfile, &periods[ii], &snrs[ii]);
    checked_peaks[ii] = FALSE;
  }

  // now write them back out, depending on the status of ranked
  if (ranked == TRUE) {

    int total_checked = 0;
    
    while (total_checked < npeaks) {
      
      max_snr = 0;
      
      for (ii = 0; ii < npeaks; ii++) {
	// has the current entry been checked?
	if (checked_peaks[ii] == FALSE) {
	  // is this the first non-checked peak found?
	  if (max_snr == 0) {
	    // this peak automatically gets to be top dog
	    max_snr = snrs[ii];
	    max_position = ii;
	  } else {
	    // else another peak has already been allocated - compare and decide
	    if (snrs[ii] > max_snr) {
	      // this peak wins
	      max_snr = snrs[ii];
	      max_position = ii;
	    }
	  }
	}
      }

      // we now have the maximum peak that has not been checked
      //printf("RANKED: Found max at period %f and snr %f.\n", periods[max_position], snrs[max_position]);
      // write it out
      writePeak(outputfile, periods[max_position]*1000/64, snrs[max_position], tsamp);
      checked_peaks[max_position] = TRUE;
      total_checked++;
    }
  } else if (ranked == FALSE) {
    for (ii = 0; ii < npeaks; ii++) {
      writePeak(outputfile, periods[ii]*1000/64, snrs[ii], tsamp);
    }
  }
  return;
    
}
