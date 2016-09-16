// Header for the paddedArray data type
// Andrew Cameron, MPIFR, 30/01/2015
// Last modified 06/08/2015

// Changelog
// 20/03/2015 - Added scalefactor as a part of the struct to make handling downsampling easier
// 06/08/2015 - Also added de-reddening parameters inside the struct for ease of implementation

#include <stdio.h>
#include <stdlib.h>
#include "ffadata.h"

#ifndef PADDEDARRAY_H
#define PADDEDARRAY_H

#define TRUE 1
#define FALSE 0

// A padded array is an array of data that has two sizes
// The datasize is the amount of "real" data that is stored in the array
// The fullsize is the actual size of the array in terms of its allocated memory
// The scalefactor refers to downsampling of the array - real period (in samples) = stored period (array index) * scalefactor
// redflag indicates whether or not to de-redden the dataset with each initialisation/downsampling
// window gives the size of the dereddening to be used in units of the original samples (so scaling by scalefactor may be required)

typedef struct paddedArray {
  ffadata* dataarray;
  int datasize;
  int fullsize;
  int scalefactor;
  int redflag;
  int window;
} paddedArray;

// ***** FUNCTION PROTOTYPES *****

// Allocates the memory for a padded array struct and returns a pointer. Internal values are uninitialised.
paddedArray* createPaddedArray(int datasize, int fullsize); // HEAVILY MODIFIED

// leans up the DataArray once processing is complete
void deletePaddedArray(paddedArray* x);

// creates a new paddedarray equal to the contents of x - y
// arrays MUST be the same datasize AND fullsize
// other parameters will be inherited from x
paddedArray* subtractPaddedArray(paddedArray* x, paddedArray* y);

// Displays the contents of a padded array to stdout
void displayPaddedArray(paddedArray* x);

// Outputs the contents of a padded array to a specified file in a GNUPLOT friendly format
void writePaddedArray(FILE* inputfile, paddedArray* x);

int getPaddedArrayDataSize(paddedArray* x);

int getPaddedArrayFullSize(paddedArray* x);

ffadata* getPaddedArrayDataArray(paddedArray* x);

// returns a copy of the data array stored in a padded array - WARNING: RETURNS HEAP ALLOCATED MEMORY WHICH SHOULD BE FREED
ffadata* copyPaddedArrayDataArray(paddedArray* x);

// 20/03/2015 - read scalefactor
int getPaddedArrayScaleFactor(paddedArray *x);

// 20/03/2015 - change scalefactor
void setPaddedArrayScaleFactor(paddedArray *x, int newfactor);

// 06/08/2015 - functions to handle reading and setting de-reddening values

int getRedFlag(paddedArray *x);

void setRedFlag(paddedArray *x, int newflag);

int getWindow(paddedArray *x);

void setWindow(paddedArray *x, int newwindow);

#endif /* PADDEDARRAY_H */
