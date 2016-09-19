// White noise generation package
// Andrew Cameron, MPIFR, 19/12/2014


#ifndef WHITENOISE_H
#define WHITENOISE_H

// ***** FUNCTION PROTOTYPES *****

// seed random number generation process
void startseed(void);

// WRAPPER FUNCTIONS

double generateWhiteNoise(double sigma, double mu);

#endif /* WHITENOISE_H */
