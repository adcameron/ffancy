// C file for the strings_equal function
// Function from SIGPROC
// Andrew Cameron, MPIFR, 02/02/2015

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stringsequal.h"

int strings_equal (char *string1, char *string2) {
  if (!strcmp(string1,string2)) {
    return 1;
  } else {
    return 0;
  }
}
