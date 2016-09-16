// Header for array statistics package
// Andrew Cameron, MPIFR, 20/03/2015
// Last modified 20/03/2015

// Changelog

#include <stdio.h>
#include <stdlib.h>

#ifndef STATS_H
#define STATS_H

// ***** FUNCTION PROTOTYPES *****

// return the mean of an array
double mean(double* array, int size);

// return the median of an array
double median(double* array, int size);

// return the standard deviation of an array
double stddev(double* array, int size);

// return the max of an array
double max(double* array, int size);

// return the min of an array
double min(double* array, int size);

// for use in qsort
int compare(const void * a, const void * b);

#endif /* STATS_H */
