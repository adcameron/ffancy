// Implementation of Metric 7 - Integral Metric
// Andrew Cameron, MPIFR, 17/03/2015

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "metrics.h"
#include "ffadata.h"
#include "mad.h"

#define INTEGRAL_EXCLUSION_WIDTH 0.2

double integralMetric(ffadata* sourcearray, int startpos, int subsize) {

  // Integrates the area under the profile using a trapezoidal approximation
  assert(subsize > 0);
  assert(sourcearray != NULL);

  int ii;

  // create a copy of the array
  ffadata* normarray = (ffadata*)malloc(sizeof(ffadata) * subsize);
  for (ii = 0; ii < subsize; ii++) {
    normarray[ii] = sourcearray[ii + startpos];
  }

  // normalise it
  mad(normarray, subsize);

  // normalisation should reduce the baseline to zero and the sigma to 1 - no need to subract baseline "rectangular average" integral - can just integrate the entire profile
  // applying the integration metric should be akin in some sense to applying an optimal matched filter

  // now calculate integral

  // scan subarray once, calculating the total integral, the average, and locating the peak position
  double integral = 0;
  
  for (ii = 0; ii < subsize; ii++) {
    integral = integral + normarray[ii];
  }

  // QUESTION - DOES THIS NEED TO SOMEHOW BE NORMALISED TO PROFILE LENGTH?

  // cleanup
  free(normarray);

  return integral;
}

// OLD METRIC VERSION

/*
double integralMetric(ffadata* sourcearray, int startpos, int subsize) {

  // Integrates the area under the profile using a trapezoidal approximation
  // WARNING - REQUIRES NORMALISATION

  assert(subsize > 0);
  assert(sourcearray != NULL);

  // scan subarray once, calculating the total integral, the average, and locating the peak position
  double integral = 0;
  ffadata I_max = sourcearray[startpos];
  int I_maxpos = 0;
  double I_average = 0;
  
  int i;
  for (i = startpos; i < (startpos + subsize); i++) {
    if (i < (startpos + subsize - 1)) {
      integral = integral + (sourcearray[i] + sourcearray[i+1])/((double)2);
    }
    if (sourcearray[i] > I_max) {
      I_max = sourcearray[i];
      I_maxpos = i - startpos;
    }

    I_average = I_average + sourcearray[i];
    
  }

  // now scan the exclusion window to remove pulse from average

  double exclusion_fraction = INTEGRAL_EXCLUSION_WIDTH;
  int exclusion_halfwidth = (int)ceil(subsize*exclusion_fraction/(double)2); // half of the exclusion window in sample sizes
  int exclusion_start = (I_maxpos - exclusion_halfwidth + subsize)%subsize; // have to add an extra subsize because % can't handle negative numbers properly

  for (i = exclusion_start; i < exclusion_start + 2*exclusion_halfwidth; i++) {
    I_average = I_average - sourcearray[i%subsize+startpos];
  }

  // finalise statistics
  int remaining_length = subsize - 2*exclusion_halfwidth;
  I_average = I_average/((double)remaining_length);

  double remaining_integral = integral - I_average*subsize;

  return remaining_integral;
}
*/
