// Implementation of Metric 4 - MaxMin Metric
// Andrew Cameron, MPIFR, 02/02/2015

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "metrics.h"
#include "ffadata.h"

double maxminMetric(ffadata* sourcearray, int startpos, int subsize) {

  // return the difference between the highest and lowest values in the subarray - now weighted by sigma
  assert(subsize > 0);
  assert(sourcearray != NULL);

  ffadata max = sourcearray[startpos];
  ffadata min = sourcearray[startpos];

  // now need to scale the values by sigma to produce a SNR value
  double mean = 0;
  double sigma = 0;

  int i;
  for (i = startpos; i < startpos + subsize; i++) {
    if (sourcearray[i] > max) {
      max = sourcearray[i];
    }
    if (sourcearray[i] < min) {
      min = sourcearray[i];
    }
    mean = mean + sourcearray[i];
  }

  // finalise mean
  mean = mean/((double)subsize);

  // calculate sigma
  for (i = startpos; i < startpos + subsize; i++) {
    sigma = sigma + pow((sourcearray[i] - mean), 2);
  }

  // finalise sigma
  sigma = sqrt(sigma/((double)subsize));

  // return result scaled by sigma
  return (max-min)/sigma;

}
