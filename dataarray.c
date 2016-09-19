// C file for the dataArray functionality
// Built on the paddedArray data type
// Andrew Cameron, MPIFR, 11/02/2015

// Future improvements to be made
// * Inclusion of MAD normalisation
// * Ability to select which noise type is used for padding

// CHANGELOG
// 06/08/2015 - Began adding a section into the downsampling code to allow for automatic de-reddening based on a flag setting
// 12/08/2015 - Added the binary float writer
// 08/03/2016 - Added the binary float read function
// 18/03/2016 - Modified ASCII read function to strip headers from time series
// 19/03/2016 - Reset the dered code to keep the de-reddening filter size fixed with respect to the original sample size. Window is scaled with respect to the scalefactor such
//              that the window remains constant in proportion to the sample size.
// 15/04/2016 - Downsampling routine no longer includes automatic de-reddening. This must be applied separately.
// 06/06/2016 - Added SIGPYPROC read/write functionality
// 19/09/2016 - Updated noise generation code

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "ffadata.h"
#include "paddedarray.h"
#include "dataarray.h"
#include "whitenoise.h"
#include "runningmedian.h"

paddedArray* basicPulsarDataArray(int rawsize, int pulseperiod, int pulsewidth) {

  // Build fake pulsar profile with PULSE for pulse value and NO_PULSE for off pulse value
  // Build array by additional factor of ARRAY_PADDING - pad with zeroes

  // Validity Checks
  assert(pulseperiod > 0 && pulsewidth > 0);
  assert(pulseperiod > pulsewidth);

  // Build paddedArray with fullsize = rawsize*ARRAY_PADDING
  int paddedsize = (int)ceil(rawsize*ARRAY_PADDING);

  // To assist with downsampling later, make paddedsize a multiple of two
  if ((paddedsize % 2) != 0) {
    paddedsize++;
  }

  // Create paddedArray
  paddedArray* sourcedata = createPaddedArray(rawsize, paddedsize);

  // Seed data section of array with PULSE and NO_PULSE values, and padding with zeroes

  int i = 0;
  int j = 0;

  ffadata* array = getPaddedArrayDataArray(sourcedata);

  for(i = 0; i < paddedsize; i++) {
    if (i >= rawsize) {
      // padding region
      array[i] = generateZeroPadding();
    } else if ((i % pulseperiod) == 0) {
      array[i] = PULSE;
      j = pulsewidth - 1;
    } else if (j > 0) {
      array[i] = PULSE;
      j--;
    } else {
      array[i] = NO_PULSE;
    }
  }

  return sourcedata;

}


paddedArray* readASCIIDataArray(FILE *inputfile) {

  // NOTE: Issue of whether to use noisy padding or zero padding is still undecided.
  // Code relating to noisy generation has been left inside this function but has been commented out.
  // Future functionality may include ability to nominate noise generation method via input variable.

  assert(inputfile != NULL);

  // seed random generation for noise padding
  //long seed = seedNoisyPadding();
  
  // Determine the size of the header
  int headersize = 0;
  char headerstring[1000];
  headerstring[headersize] = '\0';
  while (strcmp(headerstring,"HEADER_END")==0) {
    headerstring[headersize] = fgetc(inputfile);
    headersize++;
    headerstring[headersize] = '\0'; 
  }
  
  // NOW determine the size of the remaining file
  int datasize = 0;
  while (fgetc(inputfile) != EOF) {
    datasize++;
  }
  
  // Reset inputfile to prepare for reading
  rewind(inputfile);

  int paddedsize = (int)ceil(datasize*ARRAY_PADDING);

  // to assist with downsampling later, make padded size a multiple of two
  if ((paddedsize % 2) != 0) {
    paddedsize++;
  }
  
  paddedArray* sourcedata = createPaddedArray(datasize, paddedsize);
  ffadata* array = getPaddedArrayDataArray(sourcedata);

  // Read data into array

  // to allow for noise generation, need to calculate rms and average on the fly as best as possible, then use to seed noise
  // double rms = 0;
  // double mean = 0;

  // read through the header
  int header_i = 0;
  while (header_i < headersize) {
    fgetc(inputfile);
  }
  
  int i = 0;
  unsigned int x;
  while ((i < paddedsize) && ((x = fgetc(inputfile)) != EOF)) {
    array[i] = x;
    // mean = mean + x;
    i++;
  }

  /*
  // Data read complete - now finalise parameters for noise calculation
  mean = mean/((double)i);

  int j;
  for (j = 0; j < i; j++) {
    rms = rms + pow((array[j] - mean), 2);
  }
  rms = sqrt(rms/((double)i));
  */

  while (i < paddedsize) {
    array[i] = generateZeroPadding();
    //array[i] = generateNoisyPadding(&seed, rms, mean);
    i++;
  }

  // Array initialised
  printf("Data array initialised: Data size = %d | Full size = %d\n", getPaddedArrayDataSize(sourcedata), getPaddedArrayFullSize(sourcedata));
  /* MAD NORMALISATION YET TO BE TESTED / IMPLEMENTED
  printf("Now normalising dataset via MAD...\n");
  mad(sourcedata->dataarray, sourcedata->fullsize);
  */

  return sourcedata;

}

paddedArray* readFloatDataArray(FILE *inputfile) {

  assert(inputfile != NULL);
  
  // Determine size of file by scanning file once before storing data
  // fread returns the number of elements successfully read - keep reading until read is no longer successful, which should be triggered by the EOF
  int datasize = 0;
  float tempvalue;
  while (fread(&tempvalue, sizeof(float), 1, inputfile) == 1) {
    datasize++;
  }
  
  // Reset inputfile to prepare for reading
  rewind(inputfile);

  int paddedsize = (int)ceil(datasize*ARRAY_PADDING);

  // to assist with downsampling later, make padded size a multiple of two
  if ((paddedsize % 2) != 0) {
    paddedsize++;
  }
  
  paddedArray* sourcedata = createPaddedArray(datasize, paddedsize);
  ffadata* array = getPaddedArrayDataArray(sourcedata);

  // Read data into array

  int i = 0;
  float x;
  while ((i < paddedsize) && (fread(&x, sizeof(float), 1, inputfile) == 1)) {
    array[i] = (ffadata)x;
    i++;
  }

  while (i < paddedsize) {
    array[i] = generateZeroPadding();
    //array[i] = generateNoisyPadding(&seed, rms, mean);
    i++;
  }

  printf("Data array initialised: Data size = %d | Full size = %d\n", getPaddedArrayDataSize(sourcedata), getPaddedArrayFullSize(sourcedata));

  return sourcedata;

}


paddedArray* readSIGPYPROCDataArray(FILE *inputfile) {

  assert(inputfile != NULL);
  
  // Determine the size of the header
  int headersize = 0;
  char headerstring[1000];
  headerstring[headersize] = '\0';
  while (strcmp(headerstring,"HEADER_END")==0) {
    headerstring[headersize] = fgetc(inputfile);
    headersize++;
    headerstring[headersize] = '\0'; 
  }
  
  // NOW determine the size of the remaining file
  int datasize = 0;
  float tempvalue;
  while (fread(&tempvalue, sizeof(float), 1, inputfile) == 1) {
    datasize++;
  }
  
  // Reset inputfile to prepare for reading
  rewind(inputfile);

  int paddedsize = (int)ceil(datasize*ARRAY_PADDING);

  // to assist with downsampling later, make padded size a multiple of two
  if ((paddedsize % 2) != 0) {
    paddedsize++;
  }
  
  paddedArray* sourcedata = createPaddedArray(datasize, paddedsize);
  ffadata* array = getPaddedArrayDataArray(sourcedata);

  // Read data into array

  // read through the header
  int header_i = 0;
  while (header_i < headersize) {
    fgetc(inputfile);
  }
  
  int i = 0;
  float x;
  while ((i < paddedsize) && (fread(&x, sizeof(float), 1, inputfile) == 1)) {
    array[i] = (ffadata)x;
    i++;
  }

  while (i < paddedsize) {
    array[i] = generateZeroPadding();
    //array[i] = generateNoisyPadding(&seed, rms, mean);
    i++;
  }

  // Array initialised
  printf("Data array initialised: Data size = %d | Full size = %d\n", getPaddedArrayDataSize(sourcedata), getPaddedArrayFullSize(sourcedata));

  return sourcedata;
}


void writeASCIIDataArray(FILE *outputfile, paddedArray* sourcedata) {

  // validity checks
  assert(outputfile != NULL);
  assert(sourcedata != NULL);

  ffadata* dataarray = getPaddedArrayDataArray(sourcedata);
  assert(dataarray != NULL);

  // read through array and write out in ASCII format to file
  // only need to read data elements, not padded elements

  int i;
  for (i = 0; i < getPaddedArrayDataSize(sourcedata); i++) {
    fprintf(outputfile, "%c", (unsigned)(int)dataarray[i]);
  }

  return;

}

void writeFloatDataArray(FILE *outputfile, paddedArray* sourcedata) {

  // validity checks
  assert(outputfile != NULL);
  assert(sourcedata != NULL);

  ffadata* dataarray = getPaddedArrayDataArray(sourcedata);
  assert(dataarray != NULL);

  // read through array and write out to file
  // only need to read data elements, not padded elements

  int i;
  float temp;
  for (i = 0; i < getPaddedArrayDataSize(sourcedata); i++) {
    temp = (float)dataarray[i];
    fwrite(&temp, 1, sizeof(float), outputfile);
  }

  return;

}

void writeSIGPYPROCDataArray(FILE *outputfile, paddedArray* sourcedata) {

  // validity checks
  assert(outputfile != NULL);
  assert(sourcedata != NULL);

  ffadata* dataarray = getPaddedArrayDataArray(sourcedata);
  assert(dataarray != NULL);

  // read through array and write out to file
  // only need to read data elements, not padded elements

  int i;
  float temp;
  for (i = 0; i < getPaddedArrayDataSize(sourcedata); i++) {
    temp = (float)dataarray[i];
    fwrite(&temp, 1, sizeof(float), outputfile);
  }

  return;

}

paddedArray* downsampleDataArray(paddedArray* sourcedata) {

  assert(sourcedata != NULL);

  // NOTE: Issue of whether to use noisy padding or zero padding is still undecided.
  // Code relating to noisy generation has been left inside this function but has been commented out.
  // Future functionality may include ability to nominate noise generation method via input variable.

  // Determine the sizes of the elements in the downsampled paddedArray based on the size of the elements in sourcedata
  // Need both datasizes to be mulitiples of 2
  int sourcedatasize = getPaddedArrayDataSize(sourcedata);
  int sourcefullsize = getPaddedArrayFullSize(sourcedata);

  if (sourcedatasize % 2 != 0) {
    sourcedatasize = sourcedatasize + 1;
  }
  if (sourcefullsize % 2 != 0) {
    sourcefullsize = sourcefullsize + 1;
  }
  
  // this may cause the sizes to be slightly larger than they were originally
  // if sourcedatasize > datasize, it should just read into the empty padding elements
  // if sourcefullsize > fullsize, then there should be a check in the downsampling loop that if an element does not exist in the original array, it is filled in as padding

  // Both sizes are now divisible by two - allocate new sizes
  int downdatasize = sourcedatasize/2;
  int downfullsize = sourcefullsize/2;
  
  // check that the padding ratio still holds, if not, correct this
  while (downfullsize < downdatasize * ARRAY_PADDING) {
    downfullsize = downfullsize + 2;
  }
  
  // sizes configured - initialise the paddedArray
  paddedArray* downdata = createPaddedArray(downdatasize, downfullsize);
  setRedFlag(downdata, getRedFlag(sourcedata));
  setWindow(downdata, getWindow(sourcedata));
  ffadata* downdataarray = getPaddedArrayDataArray(downdata);

  // now perform downsampling using abstracted "average" function to combine values 
  int i, index1, index2, element1, element2;

  for (i = 0; i < downdata->fullsize; i++) {
    index1 = i*2;
    index2 = index1 + 1;
    
    if (index1 > sourcedata->fullsize) {
      element1 = generateZeroPadding();
      element2 = generateZeroPadding();
    } else if (index2 > sourcedata->fullsize) {
      element1 = sourcedata->dataarray[index1];
      element2 = generateZeroPadding();
    } else {
      element1 = sourcedata->dataarray[index1];
      element2 = sourcedata->dataarray[index2];
    }
    
    // now seed new array with the averaged value
    downdataarray[i] = resample(element1, element2);
  }

  // downsampling complete - modify scalefactor
  setPaddedArrayScaleFactor(downdata, getPaddedArrayScaleFactor(sourcedata) * 2);

  // preliminary downsampling is now complete - can now execute the de-reddening
  /*if (getRedFlag(downdata)) {

    printf("De-reddening as part of downsampling...\n");
    
    paddedArray* outdata = dereddenDataArray(downdata);

    // dereddening now complete - replace downdata with outdata and cleanup
    deletePaddedArray(downdata);
    downdata = outdata;

    }
  */

  return downdata;

}

paddedArray* dereddenDataArray(paddedArray* sourcedata) {

  assert(sourcedata != NULL);

  // need to build an outbuffer
  paddedArray* outdata  = createPaddedArray(getPaddedArrayDataSize(sourcedata), getPaddedArrayFullSize(sourcedata));
  setRedFlag(outdata, getRedFlag(sourcedata));
  setWindow(outdata, getWindow(sourcedata));
  setPaddedArrayScaleFactor(outdata, getPaddedArrayScaleFactor(sourcedata));
  
  // convert the window to an integer value appropriate to the downsampled sample size
  int downwindow = (int)ceil(((double)getWindow(sourcedata))/((double)getPaddedArrayScaleFactor(sourcedata)));
  //int downwindow = getWindow(sourcedata);
  //assert(downwindow < getPaddedArrayDataSize(sourcedata));
  
  // now run the median filter
  runningMedian(getPaddedArrayDataArray(sourcedata), getPaddedArrayDataArray(outdata), downwindow, getPaddedArrayDataSize(sourcedata));
  
  return outdata;

}

void seedNoisyPadding() {

  startseed();
  return;
}

ffadata generateNoisyPadding(double rms, double mean) {

  return (ffadata)generateWhiteNoise(rms, mean);

}

ffadata generateZeroPadding() {

  return ZERO_PADDING;

}
