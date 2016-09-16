// Implementation of Metric 3 - Basic Metric (aka Max Metric)
// Andrew Cameron, MPIFR, 02/02/2015

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "metrics.h"
#include "ffadata.h"

double basicMetric(ffadata* sourcearray, int startpos, int subsize) {
  
  // Return the highest value found in the subarray
  assert(sourcearray != NULL);
  assert(subsize > 0);

  ffadata result = 0;

  int i;
  for (i = startpos; i < startpos + subsize; i++) {
    if (sourcearray[i] > result) {
      result = sourcearray[i];
    }
  }
  return result;

}
