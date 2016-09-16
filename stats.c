// Impelemtation of array statistics package
// Andrew Cameron, MPIFR, 20/03/2015
// Last modified 20/03/2015

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "stats.h"

// ***** FUNCTION PROTOTYPES *****

// return the mean of an array
double mean(double* array, int size) {
  
  assert(array != NULL);
  assert(size > 0);

  int i;
  double mean = 0;

  for (i = 0; i < size; i++) {
    mean = mean + array[i];
  }

  mean = mean/((double)size);

  return mean;
  
}

// return the median of an array
double median(double* array, int size) {

  assert(array != NULL);
  assert(size > 0);

  double* sortedarray = (double*)malloc(sizeof(double)*size);
  
  int i;
  for (i = 0; i < size; i++) {
    sortedarray[i] = array[i];
  }

  qsort(sortedarray, size, sizeof(double), compare);
  
  int halfpos = (int)floor(size/((double)2));
  double median = sortedarray[halfpos];

  free(sortedarray);

  return median;

}

// return the standard deviation of an array
double stddev(double* array, int size) {

  assert(array != NULL);
  assert(size > 0);

  double average = mean(array, size);
  double stddev = 0;
  int i;

  for (i = 0; i < size; i++) {
    stddev = stddev + pow((array[i]-average),2);
  }

  stddev = sqrt(stddev/((double)size));

  return stddev;

}

// return the max of an array
double max(double* array, int size) {

  assert(array != NULL);
  assert(size > 0);

  double max = array[0];

  int i;
 
  for (i = 0; i < size; i++) {
    if (array[i] > max) {
      max = array[i];
    }
  }

  return max;

}

// return the min of an array
double min(double* array, int size) {

  assert(array != NULL);
  assert(size > 0);

  double min = array[0];

  int i;
 
  for (i = 0; i < size; i++) {
    if (array[i] < min) {
      min = array[i];
    }
  }

  return min;

}

int compare(const void * a, const void * b) {

  if ( *(double*)a <  *(double*)b ) return -1;
  if ( *(double*)a == *(double*)b ) return 0;
  if ( *(double*)a >  *(double*)b ) return 1;
  
  // backup for function completeness
  return 0;
}

