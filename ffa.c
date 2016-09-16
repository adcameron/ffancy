// C file for fucntions directly related to FFA execution
// Andrew Cameron, MPIFR, 02/03/2015
// Last modified 14/04/2016

// Changelog
// 06/02/2015 - Modified singleFFA so that it now produces a copy of the sourcearray for each execution, so that elements in last row can be modified to handle zero padding
// 02/03/2015 - Added mfsmoother for Kondratiev testing
// 20/03/2015 - Modified downsampling routine such that the scalefactor is now built into the struct, so as to better handle pre-FFA downsampling
// 10/04/2015 - Adjusted downsampling routine to make sure that downsampling occurs whenever the length of the profile being tested doubles
// 08/09/2015 - Modified structure of profiledump() to dump out full FFA profile information in custom format
// 16/09/2015 - Made slight modification to massFFA downsampling proceudre such that execution will abort if the initial lowperiod is not an integer multiple of the initial scalefactor
//            - This should only affect executions of the code which use pre-downsampling, and will prevent mathematically incorrect attempts by the code to run fractional
//              search periods into singleFFA
//            - Also added functionality into massFFA and singleFFA to allow them to be told to do dump MAD normalised profiles as opposed to just regular profiles




#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "ffadata.h"
#include "power2resizer.h"
#include "paddedarray.h"
#include "dataarray.h"
#include "ffa.h"
#include "mad.h"

void slideAdd(ffadata* sourcearray, ffadata* resultarray, int sourcesubstartpos1, int sourcesubstartpos2, int resultsubstart, int subsize, int slide) {

  assert(sourcearray != NULL);
  assert(resultarray != NULL);

  // scroll through the elements of the result sub array, and add in the required elements of the source sub arrays, with the appropriate slide
  int i, j;

  for (i = 0; i < subsize; i++) {

    // result element is i
    // the position in the first source sub array will just be i
    // the position in the second source sub array will be i + slide, modulated by the subsize
    j = (i + slide) % subsize;

    int sourcesubpos1 = i + sourcesubstartpos1;
    int sourcesubpos2 = j + sourcesubstartpos2;

    // now add
    resultarray[i + resultsubstart] = add(sourcearray[sourcesubpos1], sourcearray[sourcesubpos2]);

  }

  return;
}

void massFFA(FILE* outputfile, FILE* profilefile, FILE* normprofilefile, paddedArray* sourcedata, int lowperiod, int highperiod, double (*metric)(ffadata*, int, int), int mfsize,  int prelim_ds, FILE* redfile, int PRESTO_flag, int timenorm_flag) {

  // UPDATE - THIS SCRIPT MUST REFER ANY DE-REDDENING AND RESULTANT DOWNSAMPLING BACK TO THE ORIGINAL SOURCEDATA ARRAY FOR COMPUTATIONAL CORRECTNESS
  // DOUBLE UPDATE 15/04/2016 - THE DOWNSAMPLING FUNCTION NO LONGER INCLUDES AUTOMATIC DE-REDDENING
  // IT IS NOW APPLIED ONCE AFTER THE FULL DOWNSAMPLING LOOP IS COMPLETE

  // validity checks
  assert(outputfile != NULL);
  assert(sourcedata != NULL);
  
  if (fmod(lowperiod, getPaddedArrayScaleFactor(sourcedata)) != 0) {
    printf("ERROR: Low search period (%d) must be integer multiple of initial downsampling factor (%d).\nPlease correct and try again.\n", lowperiod, getPaddedArrayScaleFactor(sourcedata));
    exit(EXIT_FAILURE);
  }

  // This function needs to generate all the base periods to test, re-scale the array for each base period, and then execute the individual FFA algorithm for each base period
  // Function also needs to oversee downsampling by a factor of 2 each time the lowperiod is doubled.

  int i = lowperiod;
  paddedArray* workingdata = sourcedata;
  paddedArray* tempdata;
  int loopscalefactor = 0; //used for controlling the downsampling during FFA operation

  fprintf(outputfile, "# Period (original samples) | Downsample factor | Period (downsampled samples) | Metric\n");

  while (i < highperiod) {

    if (i == lowperiod*((int)pow(2, loopscalefactor))) {
      // this means that we have either just started the scan, or have reached a downsampling point
      printf("\nDownsampling initiated: i = %d\n", i);
      
      // the process will first involve de-reddening the datarray (but only if de-reddening is selected
      // clean up the current workingdata - it is no longer needed
      if (workingdata != sourcedata) {
	deletePaddedArray(workingdata);
      }

      // reset to sourcedata
      workingdata = sourcedata;

      /*if (getRedFlag(sourcedata) == TRUE) {
	// the de-reddening window needs to be increased with each downsample loop
	int old_window = getWindow(sourcedata);
	int new_window = old_window*((int)pow(2, loopscalefactor));
	printf("Dereddening with window of %d original samples...\n", new_window);
	setWindow(sourcedata, new_window); // this will need to be reset once a new paddedarray is created

	// de-redden
	workingdata = dereddenDataArray(sourcedata);
	// reset sourcedata's window
	setWindow(sourcedata, old_window);

	// if this is the first pass through, and the file is set, now is the time to write the file
	if ((redfile != NULL) && (i == lowperiod)) {
	  if (PRESTO_flag == FALSE) {
	    writeASCIIDataArray(redfile, workingdata);
	  } else if (PRESTO_flag == TRUE) {
	    writeFloatDataArray(redfile, workingdata);
	  }
	}
	printf("De-reddened copy of data written to file.\n");
      } else {
        workingdata = sourcedata;
	}*/

      // array has been de-reddened, now downsample according to the correct number of loops
      int jj = 0;
      
      while (jj < prelim_ds + loopscalefactor) {
	tempdata = downsampleDataArray(workingdata);
	// we need to clean up the old workingdata struct
	if (workingdata != sourcedata) {
	  deletePaddedArray(workingdata);
	}
	workingdata = tempdata;

	jj++;
	printf("Downsample loop %d completed.\n", jj);
      }
      
      printf("Downsample factor: %d\n", prelim_ds + loopscalefactor);

      // NEW SECTION - NOW DEREDDEN, IF NECCESSARY
      if (getRedFlag(workingdata) == TRUE) {
	// the de-reddening window needs to be increased with each downsample loop
	int old_window = getWindow(workingdata);
	int new_window = old_window*((int)pow(2, loopscalefactor));
	printf("Dereddening with window of %d original samples...\n", new_window);
	setWindow(workingdata, new_window); // this will need to be reset once a new paddedarray is created

	// de-redden
	tempdata = dereddenDataArray(workingdata);
	// cleanup
	if (workingdata != sourcedata) {
	  deletePaddedArray(workingdata);
	}
	workingdata = tempdata;
	// reset sourcedata's window
	setWindow(workingdata, old_window);

	// if this is the first pass through, and the file is set, now is the time to write the file
	if ((redfile != NULL) && (i == lowperiod)) {
	  if (PRESTO_flag == FALSE) {
	    writeASCIIDataArray(redfile, workingdata);
	  } else if (PRESTO_flag == TRUE) {
	    writeFloatDataArray(redfile, workingdata);
	  }
	}
	printf("De-reddened copy of data written to file.\n");
      }

      // modify the internal scale factor
      loopscalefactor++;

    }

    // downsampling, if neccessary, is now complete
    
    // perform new normalisation pass using MAD if required
    if (timenorm_flag == TRUE) {
      mad(getPaddedArrayDataArray(workingdata), getPaddedArrayDataSize(workingdata));
      printf("Downsampled time-series normalised via MAD.\n");
    }
    
    // we now need to pass the relevant parameters to the singleFFA function
    // baseperiod must be calculated to match with the current downsampling
    printf("\nCalling single FFA search for a period of %d original samples...\n", i);
    singleFFA(outputfile, profilefile, normprofilefile, workingdata, i/getPaddedArrayScaleFactor(workingdata), metric, mfsize);

    // increment i according to the scale factor
    i = i + getPaddedArrayScaleFactor(workingdata);

  }

  // final clean up
  if ((workingdata != sourcedata) && (workingdata != NULL)) {
      deletePaddedArray(workingdata);
  }

  return;
}

void singleFFA(FILE* outputfile, FILE* profilefile, FILE* normprofilefile, paddedArray* sourcedata, int baseperiod, double (*metric)(ffadata*, int, int), int mfsize) {

  // basic validity checks
  assert(outputfile != NULL);
  assert(sourcedata != NULL);

  printf("Entered singleFFA with baseperiod of %d samples and a scalefactor of %d...\n", baseperiod, getPaddedArrayScaleFactor(sourcedata));
  // need the size of the array to use based on N/n = 2^x
  int size = power2Resizer(getPaddedArrayDataSize(sourcedata), baseperiod);
  // double check that the array size is still within memory limits 
  assert(size <= getPaddedArrayFullSize(sourcedata));
  printf("Array size rescaled from %d to %d (%.1f%% change).\n", getPaddedArrayDataSize(sourcedata), size, abs(getPaddedArrayDataSize(sourcedata) - size)*100/(float)(getPaddedArrayDataSize(sourcedata)));

  // initialise counters
  int i, j, k;

  // setup variables controlling the scale of the FFA
  int branches = (int)size/baseperiod;
  int addition_iterations = (int)log2(branches);
  double period_increment = (double)1/((double)(branches - 1));

  // build this many new arrays matching the original size to store the cumulative additions - the first should be the source array
  ffadata* sumarrays[addition_iterations + 1];

  for (i = 0; i <= addition_iterations; i++) {
    if (i == 0) {
      sumarrays[i] = copyPaddedArrayDataArray(sourcedata);
    } else {
      sumarrays[i] = (ffadata*)malloc(sizeof(ffadata)*size);
    }
  }

  // NEW SECTION - HANDLES ZERO PADDING ISSUE
  // If array has been padded out, then a branch of the sourcearray data will contain part data and part zeroes, causing baseline jumps and false detections
  // This row must be entirely set to zeroes
  for (i = 0; i < branches; i++) {
    if ((((i + 1) * baseperiod) > getPaddedArrayDataSize(sourcedata)) && ((i * baseperiod) < getPaddedArrayDataSize(sourcedata))) {
      // if we are a partial row of data, clean it out
      for (j = i*baseperiod ; j < (i+1)*baseperiod ; j++) {
	(sumarrays[0])[j] = generateZeroPadding();
      }
    }
  }

  // for file output
  double period;

  // start counting through the addition steps
  for (i = 1; i <= addition_iterations; i++) {

    ffadata* startarray = sumarrays[i-1];
    ffadata* endarray = sumarrays[i];

    // a segment represents the self-contained module of array elements that are adding together at each addition step
    int segmentsize = (int)pow(2, i);
    int segments = (int)branches/segmentsize;

    for (j = 0; j < segments; j++) {

      for (k = 0; k < segmentsize; k++) {
	int slide = (int)ceil((float)k/2);
	int sourcecellpos1 = ((int)floor((float)k/2) + j*segmentsize)*baseperiod;

	// we have now honed in on the result cell, and have enough information to select the source cells to use in the addition and the slide amount
	// add sub array cells
	slideAdd(startarray, endarray, sourcecellpos1, sourcecellpos1 + baseperiod*segmentsize/2, (k + j*segmentsize)*baseperiod, baseperiod, slide);

	// if this is the last iteration of the FFA additions, we can output the metric now without having to re-scan the loop
	if (i == addition_iterations) {
	  period = k * period_increment + baseperiod; // this is the tested period in units of (downsampled) samples

	  // normalise the profile for post-MAD
	  //postMadProfileNormaliser(sumarrays[i], (k + j*segmentsize)*baseperiod, baseperiod, (int)ceil(sourcedata->datasize/((double)baseperiod)));
	  //postMadProfileNormaliser(sumarrays[i], (k + j*segmentsize)*baseperiod, baseperiod, branches);
	  if (mfsize > 0) {
	    mfsmoother(sumarrays[i], (k + j*segmentsize)*baseperiod, baseperiod, mfsize);
	  }
	  
	  fprintf(outputfile, "%.10f %d %.10f %.10f\n", period*getPaddedArrayScaleFactor(sourcedata), getPaddedArrayScaleFactor(sourcedata), period, metric(sumarrays[i], (k + j*segmentsize)*baseperiod, baseperiod));
	 
	  // PROFILE DUMP
	  if ((profilefile != NULL)) {
	    profiledump(profilefile, period*getPaddedArrayScaleFactor(sourcedata), getPaddedArrayScaleFactor(sourcedata), sumarrays[i], (k + j*segmentsize)*baseperiod, baseperiod);
	  }
	  // Alternatively, dump normalised profiles (don't need to worry about copying the array as we're about to delete it anyway)
	  if ((normprofilefile != NULL)) {
	    // normalise the profiles using MAD
	    mad(&sumarrays[i][(k + j*segmentsize)*baseperiod], baseperiod);
	    profiledump(normprofilefile, period*getPaddedArrayScaleFactor(sourcedata), getPaddedArrayScaleFactor(sourcedata), sumarrays[i], (k + j*segmentsize)*baseperiod, baseperiod);
	  }
	}
      }

    }

  }

  // individual FFA execution should now be complete

  // free memory - but don't delete the original array that is part of the paddedArray struct

  for (i = 0; i <= addition_iterations; i++) {
    free(sumarrays[i]);
  }
  return;
}

// prints out the full profiles produced by an FFA folding sequence to specified filestream
// Format will be "TrialPeriod(%.10f) ScaleFactor(%d) Bin1(%d) Bin2(%d) etc..."
void profiledump(FILE* profilefile, double period, int scalefactor, ffadata* sourcearray, int startpos, int subsize) {

  assert(profilefile != NULL);
  assert(sourcearray != NULL);

  // output the first part of the line to the file
  fprintf(profilefile, "%.10f %d", period, scalefactor);

  // scan through the array and output datapoints in the profile to a continuous line of outout, separated by space delimiters
  int i;
  for (i = 0; i < subsize; i++) {
    fprintf(profilefile, " %d", (int)sourcearray[i+startpos]);
  } 

  // end it with a newline
  fprintf(profilefile, "\n");

  return;
}

void mfsmoother(ffadata* sourcearray, int startpos, int subsize, int smoothsize) {

  // validity checks
  assert(sourcearray != NULL);

  // need a copy of the array to store intermediate results
  ffadata* copyarray = (ffadata*)malloc(sizeof(ffadata) * subsize);

  // run a loop through the folded profile to execute the smoothing
  int i;
  
  // to improve efficiency, first build the matched filter by adding together the first set of array elements
  ffadata filterblock = 0;
  for (i = 0; i < smoothsize; i++) {
    filterblock = filterblock + sourcearray[(i + subsize)%subsize + startpos];
  }

  // now scroll the filter through the array, adding/subtracting as you go
  for (i = 0; i < subsize; i++) {

    // now scanning through the subarray
    // need to use modulo to account for profile wrap-around

    copyarray[i] = filterblock;

    // now modify filterblock for next position - subtract first value and add next value
    filterblock = filterblock - sourcearray[(i + subsize)%subsize + startpos];
    filterblock = filterblock + sourcearray[(i + subsize + smoothsize)%subsize + startpos];

  }

  // copy back into original array
  for (i = 0; i < subsize; i++) {
    sourcearray[i+startpos] = copyarray[i];
  }

  // cleanup
  free(copyarray);

  return;

}

/*void mfsmoother(ffadata* sourcearray, int startpos, int subsize, int smoothsize) {

  // validity checks
  assert(sourcearray != NULL);

  // need a copy of the array to store intermediate results
  ffadata* copyarray = (ffadata*)malloc(sizeof(ffadata) * subsize);

  // run a double loop through the folded profile to execute the smoothing
  int i;
  int j;

  for (i = 0; i < subsize; i++) {

    // now scanning through the subarray
    // for each element, we need to add the block of smoothsize elements down the subarray together
    // need to use modulo to account for profile wrap-around

    copyarray[i] = 0;

    for (j = 0; j < smoothsize; j++) {
      copyarray[i] = copyarray[i] + sourcearray[(i + j + subsize)%subsize + startpos];
    }

  }

  // copy back into original array
  for (i = 0; i < subsize; i++) {
    sourcearray[i+startpos] = copyarray[i];
  }

  // cleanup
  free(copyarray);

  return;

}
*/
