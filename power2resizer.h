// Header file for function which resizes arrays of data such that
// the size/period ratio equals a power of 2, erring on the padding side

// Andrew Cameron, MPIFR, 25/02/2015

#include <stdio.h>
#include <stdlib.h>
#include "ffadata.h"

#ifndef POWER2_H
#define POWER2_H

// ***** FUNCTION PROTOTYPES *****

int power2Resizer(int size, int period);

#endif /* POWER2_H */
