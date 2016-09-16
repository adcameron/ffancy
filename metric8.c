// Implementation of Metric 8 - Average Metric
// Andrew Cameron, MPIFR, 17/03/2015

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "metrics.h"
#include "ffadata.h"
#include "mad.h"

double averageMetric(ffadata* sourcearray, int startpos, int subsize) {

  assert(subsize > 0);
  assert(sourcearray != NULL);

  int ii;

  // create copy of the array
  ffadata* normarray = (ffadata*)malloc(sizeof(ffadata) * subsize);
  for (ii = 0; ii < subsize; ii++) {
    normarray[ii] = sourcearray[ii + startpos];
  }

  // now normalise using MAD
  mad(normarray, subsize);

  // baseline of array should now be zero, so we should just be able to compute the average and return
  double average = 0;

  for (ii = 0; ii < subsize; ii++) {
    average = average + normarray[ii];
  }

  // finalise average
  average = average/((double)subsize);

  // cleanup
  free(normarray);

  //return metric
  return average;
}

// OLD METRIC VERSION

/*
double averageMetric(ffadata* sourcearray, int startpos, int subsize) {

  assert(subsize > 0);
  assert(sourcearray != NULL);

  // scan subarray once, caalculating the total average and locating the peak position
  ffadata I_max = sourcearray[startpos];
  int I_maxpos = 0;
  double total_average = 0;
  
  int i;

  for (i = startpos; i < (startpos + subsize); i++) {
    
    if (sourcearray[i] > I_max) {
      I_max = sourcearray[i];
      I_maxpos = i - startpos;
    }

    total_average = total_average + sourcearray[i];
    
  }

  // now scan the exclusion window to remove pulse from off-peak average
  double offpeak_average = total_average;

  double exclusion_fraction = 0.2;
  int exclusion_halfwidth = (int)ceil(subsize*exclusion_fraction/(double)2); // half of the exclusion window in sample sizes
  int exclusion_start = (I_maxpos - exclusion_halfwidth + subsize)%subsize; // have to add an extra subsize because % can't handle negative numbers properly

  for (i = exclusion_start; i < exclusion_start + 2*exclusion_halfwidth; i++) {
    offpeak_average = offpeak_average - sourcearray[i%subsize+startpos];
  }

  // finalise averages
  int remaining_length = subsize - 2*exclusion_halfwidth;
  offpeak_average = offpeak_average/((double)remaining_length);
  total_average = total_average/((double)subsize);

  // now calculate sigma

  double sigma = 0;

  for (i = startpos; i < startpos + subsize; i++) {
    if (!((i >= exclusion_start + startpos) && (i < exclusion_start + 2*exclusion_halfwidth + startpos))) {
      sigma = sigma + pow((sourcearray[i] - offpeak_average), 2);
    } 
  }

  sigma = sqrt(sigma/((double)remaining_length));

  //return metric
  return ((total_average - offpeak_average)/(sigma));
}
*/
