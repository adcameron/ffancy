// Implementation of Metric 5 - Kondratiev Metric - based on the metric described in Kondratiev 2009, ApJ
// Implements a mimiced version of the metric described in Kondratiev 2009 ApJ
// Metric (SNR) = (I_max - <I>)/RMS, where <I> and RMS are calculated on the full profile excluding the 20% surrounding the position of I_max
// WARNING - THIS METRIC MAY CAUSE UNPREDICTABLE RESULTS WHEN CALLED WITH A SUBSIZE < 10

// Andrew Cameron, MPIFR, 02/02/2015

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "metrics.h"
#include "ffadata.h"

#define KONDRATIEV_EXCLUSION_WIDTH 0.2

double kondratievMetric(ffadata* sourcearray, int startpos, int subsize) {

  assert(subsize > 0);

  // Initialise metric parameters
  ffadata I_max = sourcearray[startpos];
  int I_maxpos = 0;

  double I_average = 0;
  double RMS = 0;

  // Find the value and position of the maximum element, and begin computing I_average simultaneously
  int i;
  for (i = startpos; i < startpos + subsize; i++) {
    if (sourcearray[i] > I_max) {
      I_max = sourcearray[i];
      I_maxpos = i - startpos;
    }
    
    I_average = I_average + sourcearray[i];
  }

  // Now exclude the 20% window around I_max
  // Two cases exist - if the window is entirely contained within the profile, or if it overlaps the end of the profile (say I_max occurs at a phase of 0.95, window is 0.85 - 1.05)
  
  double exclusion_fraction = KONDRATIEV_EXCLUSION_WIDTH;
  int exclusion_halfwidth = (int)ceil(subsize*exclusion_fraction/(double)2); // half of the exclusion window in sample sizes

  // now we count through this window of the subarray, modulo the subarray's size, and remove these elements from I_average

  int exclusion_start = (I_maxpos - exclusion_halfwidth + subsize)%subsize; // have to add an extra subsize because % can't handle negative numbers properly

  for (i = exclusion_start; i < exclusion_start + 2*exclusion_halfwidth; i++) {
    I_average = I_average - sourcearray[i%subsize + startpos];
  }

  // Window excluded for I_average - finalise
  int remaining_length = subsize - 2*exclusion_halfwidth;
  I_average = I_average/((double)remaining_length);
  
  // Only now that we have calculated I_average can the RMS be accurately calculated
  // scan array again for RMS
  for (i = startpos; i < startpos + subsize; i++) {
    if (!((i >= exclusion_start + startpos) && (i < exclusion_start + 2*exclusion_halfwidth + startpos))) {
      RMS = RMS + pow((sourcearray[i] - I_average), 2);
    } 
  }

  // finalise RMS
  RMS = sqrt(RMS/((double)remaining_length));

  // return metric
  return (double)((I_max - I_average)/RMS);

}
