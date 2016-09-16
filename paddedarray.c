// C file for the paddedArray data type
// Andrew Cameron, MPIFR, 30/01/2015
// Last modified 06/08/2015

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ffadata.h"
#include "paddedarray.h"

// Allocates the memory for a padded array struct and returns a pointer. Internal values are uninitialised.
paddedArray* createPaddedArray(int datasize, int fullsize) {

  paddedArray* x = (paddedArray*)malloc(sizeof(paddedArray));
  assert(x != NULL);

  x->dataarray = (ffadata*)malloc(sizeof(ffadata)*fullsize);
  assert(x->dataarray != NULL);

  x->datasize = datasize;
  x->fullsize = fullsize;
  x->scalefactor = 1;
  x->redflag = FALSE;
  x->window = 1;

  return x;

}

// leans up the DataArray once processing is complete
void deletePaddedArray(paddedArray* x) {

  assert(x != NULL);

  // delete memory-allocated contents
  free(x->dataarray);

  // delete struct itself
  free(x);

  // all memory cleared

  return;
}

// Displays the contents of a padded array to stdout
void displayPaddedArray(paddedArray* x) {

  assert(x != NULL);

  // display full contents of a padded array
  // delineate which sections are the data and padding

  printf("\nDatasize = %d | Fullsize = %d\n", x->datasize, x->fullsize);
  printf("Data Elements: ");

  int i;
  for (i = 0; i < x->datasize; i++) {
    printf("%.2f | ", x->dataarray[i]);
  }

  printf("\nPadding Elements: ");
  
  for (i = x->datasize; i < x->fullsize; i++) {
    printf("%.2f | ", x->dataarray[i]);
  }
  
  printf("\n\n");

  return;

}

// Outputs the contents of a padded array to a specified file in a GNUPLOT friendly format
void writePaddedArray(FILE* inputfile, paddedArray* x) {

  assert(inputfile != NULL);
  assert(x != NULL);

  // scan through the array and write contents to file
  int i;

  fprintf(inputfile, "# Bin number | Scaled bin number | Data value\n");
  
  for (i = 0; i < x->fullsize; i++) {
    fprintf(inputfile, "%d %d %.6f\n", i, i*(x->scalefactor), x->dataarray[i]);
  }

  return;

}

int getPaddedArrayDataSize(paddedArray* x) {

  assert(x != NULL);

  return x->datasize;

}

int getPaddedArrayFullSize(paddedArray* x) {

  assert(x != NULL);

  return x->fullsize;

}

ffadata* getPaddedArrayDataArray(paddedArray* x) {

  assert(x != NULL);

  return x->dataarray;

}

ffadata* copyPaddedArrayDataArray(paddedArray* x) {

  assert(x != NULL);
  int fullsize = getPaddedArrayFullSize(x);

  ffadata* copyarray = (ffadata*)malloc(sizeof(ffadata)*fullsize);
  ffadata* sourcearray = getPaddedArrayDataArray(x);
  assert(sourcearray != NULL);

  int i;
  for (i = 0; i < fullsize; i++) {

    copyarray[i] = sourcearray[i];

  }

  return copyarray;

}

int getPaddedArrayScaleFactor(paddedArray *x) {

  assert(x != NULL);

  return(x->scalefactor);

}

void setPaddedArrayScaleFactor(paddedArray *x, int newfactor) {
  
  assert(x != NULL);

  x->scalefactor = newfactor;

  return;

}

int getRedFlag(paddedArray *x) {

  assert(x != NULL);

  return(x->redflag);

}

void setRedFlag(paddedArray *x, int newflag) {

  assert(x != NULL);

  if (newflag) {
    x->redflag = TRUE;
  } else {
    x->redflag = FALSE;
  }

  return;

}

int getWindow(paddedArray *x) {

  assert(x != NULL);

  return(x->window);

}

void setWindow(paddedArray *x, int newwindow) {

  assert(x != NULL);
  assert(newwindow >= 0);

  x->window = newwindow;

  return;

}

paddedArray* subtractPaddedArray(paddedArray* x, paddedArray* y) {

  // check valid input
  assert(x != NULL);
  assert(y != NULL);
  assert(x->datasize == y->datasize);
  assert(x->fullsize == y->fullsize);

  // create new, empty padded array
  paddedArray* result = createPaddedArray(x->datasize, x->fullsize);
  result->scalefactor = x->scalefactor;
  result->window = x->window;
  result->redflag = x->redflag;

  // scan arrays and compute difference
  int ii;
  for (ii = 0; ii < result->fullsize; ii++) {
    result->dataarray[ii] = x->dataarray[ii] - y->dataarray[ii];
  }
  
  return result;
}
