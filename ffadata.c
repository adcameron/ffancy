// Defines the ffadata data type
// Andrew Cameron, MPIFR, 30/01/2015

#include <stdio.h>
#include <stdlib.h>
#include "ffadata.h"

ffadata add(ffadata x, ffadata y) {

  ffadata result = x + y;
  
  return result;
}

ffadata resample(ffadata x, ffadata y) {

  // basic implementation - just return the sum
  return (x + y);

}

int compare(const void * a, const void * b) {

  if ( *(ffadata*)a <  *(ffadata*)b ) return -1;
  if ( *(ffadata*)a == *(ffadata*)b ) return 0;
  if ( *(ffadata*)a >  *(ffadata*)b ) return 1;
  
  // backup for function completeness
  return 0;
}
