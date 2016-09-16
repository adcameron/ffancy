// Implementation of Metric 1 - Matched Filter Metric - based on work by Ewan Barr
// This version applies MAD normalisation only after the profiles have been folded - as part of the metric during runtime
// Andrew Cameron, MPIFR, 17/03/2015

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "metrics.h"
#include "ffadata.h"
#include "mad.h"

#define MAX_FILTER_WIDTH 0.2

ffadata postMadMatchedFilterMetric(ffadata* sourcearray, int startpos, int subsize) {

  assert(subsize > 0);
  assert(sourcearray != NULL);

  // metric needs to scan array using successively larger matched filters up to some set limit
  // can use the same principle as Kondratiev - 20%? 25%? Use nearest power of two?
  // Run with 20% to match Kondratiev, choosing next highest power of 2 above the 20% width as the max filter size

  int n_layers = (int)ceil(log2(subsize * MAX_FILTER_WIDTH));

  double max_SNR = 0; // stores the maximum SNR detection from this profile across all matched filters

  // loop counters
  int ii;
  int jj;

  // create a copy of the array
  ffadata* normarray = (ffadata*)malloc(sizeof(ffadata) * subsize);
  for (ii = 0; ii < subsize; ii++) {
    normarray[ii] = sourcearray[ii + startpos];
  }
 
  // normalise it using MAD 
  mad(normarray, subsize);

  // as a baseline, first perform a scan for a matched filter size of 1
  for (ii = 0; ii < subsize; ii++) {    
    if (normarray[ii] > max_SNR) {
      max_SNR = normarray[ii];
    }
  }

  // now begin the metric proper - begin scanning through successive matched filters of power 2
  // build copy and temp arrays to use for optimised pointer swapping
  ffadata* resultarray = (ffadata*)malloc(sizeof(ffadata) * subsize);
  ffadata* temparray;

  for (ii = 0; ii < n_layers; ii++) {
    int shift = 1 << ii;

    // convolve the matched filter and scan for max SNR simultaneously
    for (jj = 0; jj < subsize; jj++) {
      resultarray[jj] = normarray[jj] + normarray[(jj + shift + subsize)%subsize];
      
      if (resultarray[jj]/sqrt(shift * 2) > max_SNR) {
	max_SNR = resultarray[jj]/sqrt(shift * 2);
      }
    }
    
    // convolved array is complete
    // now swap arrays for next loop
    
    temparray = normarray;
    normarray = resultarray;
    resultarray = temparray;
  }

  // cleanup
  free(resultarray);
  free(normarray);

  // max SNR should by now have been isolated
  return max_SNR;

}
