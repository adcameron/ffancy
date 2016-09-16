// Header for fucntions directly related to FFA execution
// Andrew Cameron, MPIFR, 14/04/2015

// LAST MODIFIED 16/09/2015
// Changelog
// 08/09/2015 - Modified structure of profiledump() to dump out full FFA profile information in custom format
// 16/09/2015 - Modified structure of massFFA() and singleFFA() to be able to dump out normalised profile data as part of profiledump

#include <stdio.h>
#include <stdlib.h>
#include "ffadata.h"
#include "paddedarray.h"

#ifndef FFA_H
#define FFA_H

#define TRUE 1
#define FALSE 0

// ***** FUNCTION PROTOTYPES *****

// adds together the elements of two subarrays of the source array after sliding the contents of the second array by a set amount, then stores the result in a third subarray of result array
void slideAdd(ffadata* sourcearray, ffadata* resultarray, int sourcesubstartpos1, int sourcesubstartpos2, int resultsubstart, int subsize, int slide);

// Oversight function for the FFA
void massFFA(FILE* outputfile, FILE* profilefile, FILE* normprofilefile, paddedArray* sourcedata, int lowperiod, int highperiod, double (*metric)(ffadata*, int, int), int mfsize, int prelim_ds, FILE* redfile, int PRESTO_flag, int timenorm_flag);

// Runs a single FFA for one baseperiod
void singleFFA(FILE* outputfile, FILE* profilefile, FILE* normprofilefile, paddedArray* sourcedata, int baseperiod, double (*metric)(ffadata*, int, int), int mfsize);

// prints out the full profiles produced by an FFA folding sequence to specified filestream
// Format will be "TrialPeriod(%.10f) ScaleFactor(%d) Bin1(%d) Bin2(%d) etc..."
void profiledump(FILE* profilefile, double period, int scalefactor, ffadata* sourcearray, int startpos, int subsize);

// smooths a folded profile according to a predetermined mathched filter - used for analysis / testing purposes
// smoothsize is in sample units
void mfsmoother(ffadata* sourcearray, int startpos, int subsize, int smoothsize);

#endif /* FFA_H */
