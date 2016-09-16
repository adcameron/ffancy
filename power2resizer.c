// C file for function which resizes arrays of data such that
// the size/period ratio equals a power of 2, erring on the side of padding the data

// Andrew Cameron, MPIFR, 25/02/2015

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ffadata.h"
#include "power2resizer.h"

int power2Resizer(int size, int period) {

  // work out the nearest ratios of size/period = 2^x that side above the existing size/period

  double actualpower = log2((double)size/(double)period);
  int highpower2 = (int)pow(2, ceil(actualpower));
  
  int highsize = highpower2 * period;
  
  return highsize;
}




/*
int power2Resizer(int size, int period) {

  // work out the nearest ratios of size/period = 2^x that side above and below the existing size/period, then calculate the data array sizes matching each ratio

  double actualpower = log2((double)size/(double)period);
  int lowpower2 = (int)pow(2, floor(actualpower));
  int highpower2 = (int)pow(2, ceil(actualpower));
  
  int lowsize = lowpower2 * period;
  int highsize = highpower2 * period;

  int finalsize;

  if ((size - lowsize) < (highsize - size)) {
    finalsize = lowsize;
  } else {
    finalsize = highsize;
  }
  
  return finalsize;
}
*/
