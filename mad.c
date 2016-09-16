// Defines all MAD normalisation functions
// Andrew Cameron, MPIFR, 29/04/2015

// AS OF 07/04/2015, CURRENTLY IN TESTING PHASE - POTENTIAL ERRORS IN ALGORITHM HAVE BEEN IDENTIFIED AND ARE BEING INVESTIGATED.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "ffadata.h"
#include "mad.h"

void mad(ffadata* array, int size) {

  //printf("Entered MAD normalisation function...\n");

  assert(array != NULL);
  int i;

  // STEP 1 - Get the median of the array
  
  // copy array for sorting
  ffadata* sortedarray = (ffadata*)malloc(sizeof(ffadata)*size);
  for (i = 0; i < size; i++) {
    sortedarray[i] = array[i];
    
    // TEST SECTION
    //printf("%d %f\n", i, sortedarray[i]); 

  }

  //printf("Array copied...\n");

  // sort array
  qsort(sortedarray, size, sizeof(ffadata), compare);

  // TEST SECTION
  /*printf("Array sorted...displaying contents\n");
  for (i = 0; i < size; i++) {
     printf("%d %f\n", i, sortedarray[i]); 
     }*/

  // get median
  int median_pos = floor((double)size/((double)2));
  ffadata median = sortedarray[median_pos];

  //printf("Median obtained = %f...\n", median);
  
  // STEP 2 - Remove median and calculate the absolute value of the deviances

  ffadata* deviances = (ffadata*)malloc(sizeof(ffadata)*size);
  for (i = 0; i < size; i++) {
    array[i] = array[i] - median;
    deviances[i] = fabs(array[i]);
    //printf("Deviance %d: %f\n", i, array[i]); 
    //printf("Abs. Deviance %d: %f\n", i, deviances[i]); 
  }

  //printf("Median removed...\n");

  // STEP 3 - Get the median of the deviances (MAD)
  qsort(deviances, size, sizeof(ffadata), compare);
  ffadata median_deviance = deviances[median_pos];

  //printf("Median deviance obtained = %f...\n", median_deviance);

  // STEP 4 - Divide all elements by MAD * K
  for (i = 0; i < size; i++) {
    array[i] = array[i]/(median_deviance * K);
  }

  free(sortedarray);
  free(deviances);

  //printf("MAD normalisation complete.\n");
  return;
}

ffadata* getDeviances(ffadata* array, int size) {

  assert(array != NULL);
  int i;

  // STEP 1 - Get the median of the array
  
  // copy array for sorting
  ffadata* sortedarray = (ffadata*)malloc(sizeof(ffadata)*size);
  for (i = 0; i < size; i++) {
    sortedarray[i] = array[i];
  }

  // sort array
  qsort(sortedarray, size, sizeof(ffadata), compare);

  // get median
  int median_pos = floor((double)size/((double)2));
  ffadata median = sortedarray[median_pos];
  
  // STEP 2 - Remove median and calculate the absolute value of the deviances

  ffadata* deviances = (ffadata*)malloc(sizeof(ffadata)*size);
  for (i = 0; i < size; i++) {
    sortedarray[i] = sortedarray[i] - median;
    deviances[i] = fabs(sortedarray[i]); 
  }

  // STEP 3 - Get the median of the deviances (MAD)
  qsort(deviances, size, sizeof(ffadata), compare);

  // cleanup
  free(sortedarray);

  return deviances;
}

/*void postMadProfileNormaliser(ffadata* array, int startpos, int subsize, int turns) {

  assert(array != NULL);
  int i;

  // STEP 1 - Get the median of the profile

  // copy array for sorting
  ffadata* sortedarray = (ffadata*)malloc(sizeof(ffadata)*subsize);
  for (i = 0; i < subsize; i++) {
    sortedarray[i] = array[i + startpos];
  }

  // sort array
  qsort(sortedarray, subsize, sizeof(ffadata), compare);

  // get median
  int median_pos = floor((double)subsize/((double)2));
  ffadata median = sortedarray[median_pos];

  // STEP 2 - Remove median and rescale SNR by sqrt of turns
  
  double snr_scale = sqrt(turns);

  for (i = startpos; i < startpos + subsize; i++) {
    array[i] = array[i] - median;
    array[i] = array[i]/snr_scale;
  }

  free(sortedarray);

  // normalisation complete

  return;
  }*/
