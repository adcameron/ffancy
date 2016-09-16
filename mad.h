// Defines all MAD normalisation functions
// Andrew Cameron, MPIFR, 29/04/2015

// STILL IN PROTOTYPING STAGES - TESTED NEEDED BEFORE FULL IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include "ffadata.h"

#ifndef MAD_H
#define MAD_H

#define K 1.4826

// ***** FUNCTION PROTOTYPES *****

// takes in an array of data and normalises it via the MAD technique
void mad(ffadata* array, int size); //DONE

ffadata* getDeviances(ffadata* array, int size);

// a particular MAD variant for application to folded profiles from datasets already MAD normalised
/*void postMadProfileNormaliser(ffadata* array, int startpos, int subsize, int turns);*/

#endif /* MAD_H */
