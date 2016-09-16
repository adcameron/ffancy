// White noise generation package
// Functions borrowed from random.c in the SIXPROC package
// Andrew Cameron, MPIFR, 19/12/2014

#ifndef WHITENOISE_H
#define WHITENOISE_H

// ***** RANDOM NUMBER GENERATION CONSTANTS *****

#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

// ***** FUNCTION PROTOTYPES *****

// SIXPROC FUNCTIONS

long startseed(void);

int ssm(void);

float gasdev(long *idum);

float nrran2(long *idum);

// WRAPPER FUNCTIONS

double generateWhiteNoise(long *idum, double sigma, double mean);

#endif /* WHITENOISE_H */
