// C file for the equal_strings function
// Andrew Cameron, MPIFR, 02/02/2015

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "equalstrings.h"

int equal_strings (char *string1, char *string2) {
  if (!strcmp(string1,string2)) {
    return 1;
  } else {
    return 0;
  }
}
