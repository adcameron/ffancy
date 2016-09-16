// Defines the ffadata data type
// Andrew Cameron, MPIFR, 30/01/2015

#include <stdio.h>
#include <stdlib.h>

#ifndef FFADATA_H
#define FFADATA_H

// ***** DATA TYPES  *****

typedef double ffadata;

// ***** FUNCTION PROTOTYPES *****

// Defines the addition function for the data values
ffadata add(ffadata x, ffadata y);

// Resamples two values into one value
ffadata resample(ffadata x, ffadata y);

// comparison function - returns -1 if a < b, 0 if a = b, and 1 if a > b
int compare(const void * a, const void * b);

#endif /* FFADATA_H */
