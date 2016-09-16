// Implementation of Metric 2 - Kondratiev Matched Filter Metric
// Applies varying matched filters to a folded profile, then applies the Kondratiev Metric 
// CAUTION - Refers to Metric 5 (the non matched filter version) as part of its operation - changing Metric 5 will change this metric. 
// Andrew Cameron, MPIFR, 08/04/2014

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "metrics.h"
#include "ffadata.h"

#define MAX_FILTER_WIDTH 0.2

ffadata kondratievMFMetric(ffadata* sourcearray, int startpos, int subsize) {

  assert(subsize > 0);
  assert(sourcearray != NULL);

  // metric needs to scan array using successively larger matched filters up to some set limit
  // Run with 20% to match Kondratiev, choosing next highest power of 2 above the 20% width as the max filter size

  int n_layers = (int)ceil(log2(subsize * MAX_FILTER_WIDTH));

  double max_SNR; // stores the maximum SNR detection from this profile across all matched filters
  double temp_SNR; // stores temporary SNR to save on repeated execution

  // loop counters
  int ii;
  int jj;

  // create a copy of the array
  ffadata* copyarray = (ffadata*)malloc(sizeof(ffadata) * subsize);
  for (ii = 0; ii < subsize; ii++) {
    copyarray[ii] = sourcearray[ii + startpos];
  }

  // as a baseline, first perform a scan for a matched filter size of 1
  max_SNR = kondratievMetric(copyarray, 0, subsize);

  // now begin the metric proper - begin scanning through successive matched filters of power 2
  // build copy and temp arrays to use for optimised pointer swapping
  ffadata* resultarray = (ffadata*)malloc(sizeof(ffadata) * subsize);
  ffadata* temparray;

  for (ii = 0; ii < n_layers; ii++) {
    int shift = 1 << ii;

    // convolve the matched filter and scan for max SNR simultaneously
    for (jj = 0; jj < subsize; jj++) {
      resultarray[jj] = copyarray[jj] + copyarray[(jj + shift + subsize)%subsize];
    }
    
    // convolved array is complete
    // apply Kondratiev Metric and evaluate if the new SNR is higher
    temp_SNR = kondratievMetric(resultarray, 0, subsize);
    if (temp_SNR > max_SNR) {
      max_SNR = temp_SNR;
    }
    
    // now swap arrays for next loop
    
    temparray = copyarray;
    copyarray = resultarray;
    resultarray = temparray;
  }

  // cleanup
  free(resultarray);
  free(copyarray);

  // max SNR should by now have been isolated
  return max_SNR;

}
