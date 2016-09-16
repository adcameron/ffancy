// Header for all FFA metric implementations
// Andrew Cameron, MPIFR, 13/03/2015
// Last updated 11/09/2016

// Changlog
// 11/09/2016 - reconfigured numbering to fall in line with publication
//            - Algorithms 6 & 7 now labelled 1 & 2, all other algorithms fall in sequentially as per original ordering

#include <stdio.h>
#include <stdlib.h>
#include "ffadata.h"

#ifndef METRICS_H
#define METRICS_H

// ***** FUNCTION PROTOTYPES *****

// NOTE - All metric functions must conform to the same input and output format to allow for compatibility/comparibility between metric choices

// #3 - Scans a profile subarray and returns the highest value inside the subarray
double basicMetric(ffadata* sourcearray, int startpos, int subsize);

// #4 - Returns the difference between the highest and lowest values in a profile subarray
double maxminMetric(ffadata* sourcearray, int startpos, int subsize);

// #5 - Mimics the metric seen in Kondratiev 2009, ApJ; Returns (I_max - I_average)/sigma using a 20% exclusion window centered on I_max
// Further investigation shows that this metric is lacking the matched fiter coded into the 2009 paper - Metric #2 handles this correctly
double kondratievMetric(ffadata* sourcearray, int startpos, int subsize);

// #6 - does much the same as the first Kondratiev metric, but takes advantage of a pre-calculated mean to speed up processing
// double fasterKondratievMetric(ffadata* sourcearray, int startpos, int subsize); // DEACTIVATED - RELIES ON GLOBAL VARIABLE INPUT - UNSUITABLE FOR ABSTRACTED TESTING

// #7 - Returns the area under the profile curve compared to its baseline
double integralMetric(ffadata* sourcearray, int startpos, int subsize);

// #8 - Returns the total subarray average subtracted by the 80% off-peak average
double averageMetric(ffadata* sourcearray, int startpos, int subsize);

// PRIMARY METRICS

// #1 - Uses multiple sliding top-hat functions, convolved with the profile, to determine and return SNR of pulses - Ewan's metric - returns max SNR of all trials
// This version applies the mad normalisation after the profiles have been folded - at metric runtime
double postMadMatchedFilterMetric(ffadata* sourcearray, int startpos, int subsize);

// #2 - Combines the Kondratiev metric with a blind matched filter, and calculates SNR statistics from each folded profile. 
// Used to correctly confirm and expand on the Kondratiev 2009 FFA results. 
double kondratievMFMetric(ffadata* sourcearray, int startpos, int subsize);

#endif /* METRICS_H */
