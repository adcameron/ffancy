// White noise generation package
// Andrew Cameron, MPIFR, 19/12/2014
// Last updated - 19/09/2016 - Removed unauthorised code and replaced with code released under Wikipedia Creative Commons
// http://creativecommons.org/licenses/by-sa/3.0/ (applies to generateWhiteNoise)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include "whitenoise.h"
#include <float.h>
#include <stdbool.h>

#define MY_PI 3.14159265358979323846264

void startseed(void) {

  // simply seed using current time and srand
  srand(time(NULL));

  return; 
}

double generateWhiteNoise(double sigma, double mu) {
  
  static double z0, z1;
  static bool generate;
  generate = !generate;

  if (!generate)
    return z1 * sigma + mu;
  
  double u1, u2;
  do
    {
      u1 = rand() * (1.0 / RAND_MAX);
      u2 = rand() * (1.0 / RAND_MAX);
    }
  while ( u1 <= DBL_MIN );
  
  z0 = sqrt(-2.0 * log(u1)) * cos(MY_PI*2 * u2);
  z1 = sqrt(-2.0 * log(u1)) * sin(MY_PI*2 * u2);
  return z0 * sigma + mu; 
}
