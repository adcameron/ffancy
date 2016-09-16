// Header for the dataArray functionality
// Built on the paddedArray data type
// Andrew Cameron, MPIFR, 25/02/2015
// 08/03/2016 - Added function to read Float data for sake of PRESTO
// 06/06/2016 - Added function for float data in SIGPYPROC format - a hybrid of SIGPROC and PRESTO - LARGELY UNTESTED - USE WITH CAUTION

#include <stdio.h>
#include <stdlib.h>
#include "ffadata.h"
#include "paddedarray.h"
#include "whitenoise.h"

#ifndef DATAARRAY_H
#define DATAARRAY_H

// ***** CONSTANT DEFINITIONS *****

#define ARRAY_PADDING 2
#define PULSE 1
#define NO_PULSE 0
#define ZERO_PADDING 0

// ***** FUNCTION PROTOTYPES *****

// creates an initialised padded array, with the datasize of the array being equal to the nearest power of 2 to rawsize and the padded size being determined by the scaling factor ARRAY_PADDING
paddedArray* basicPulsarDataArray(int rawsize, int pulseperiod, int pulsewidth);

// reads in data from an ASCII file in order to seed the data array
paddedArray* readASCIIDataArray(FILE *inputfile);

// reads in data from a PRESTO binary file in float format in order to seed the data array
paddedArray* readFloatDataArray(FILE *inputfile);

// reads in data from a SIGPYPROC hybrid timeseries in float format in order to seed the data array
paddedArray* readSIGPYPROCDataArray(FILE *inputfile);

// writes a padded array back out in ASCII format to a specified file
void writeASCIIDataArray(FILE *outputfile, paddedArray* sourcedata);

// writes a padded array back out in binary float format to a specified file
void writeFloatDataArray(FILE *outputfile, paddedArray* sourcedata);

// writes a padded array back out in SIGPYPROC float format to a specified file
void writeSIGPYPROCDataArray(FILE *outputfile, paddedArray* sourcedata);

// takes an existing filled PaddedArray struct and returns a new one with its array downsampled by a factor of 2
paddedArray* downsampleDataArray(paddedArray* sourcedata);

// takes an existing filled PaddedArray struct and returns a new one that has been dereddened according to its internal specifications
paddedArray* dereddenDataArray(paddedArray* sourcedata);

// generates noise for padding purposes
long seedNoisyPadding();

ffadata generateNoisyPadding(long *idum, double rms, double mean);

ffadata generateZeroPadding();

#endif /* DATAARRAY_H */
