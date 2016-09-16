// Andrew Cameron, 29/07/2015
// Code to implement a running median de-reddening scheme as part of FFANCY
// Borrowed almost in whole from SigPyProc, as written by Ewan Barr

// CHANGELOG
// 06/08/2015 - Modified working type to alias to ffadata (double)

// Modified into a .h & .c file structure

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <complex.h>
#include <omp.h>
#include "ffadata.h"

#ifndef RUNNINGMEDIAN_H
#define RUNNINGMEDIAN_H

typedef ffadata Item;
typedef struct Mediator_t
{
  Item* data;  //circular queue of values
  int*  pos;   //index into `heap` for each value
  int*  heap;  //max/median/min heap holding indexes into `data`.
  int   N;     //allocated size.
  int   idx;   //position in circular queue
  int   minCt; //count of items in min heap
  int   maxCt; //count of items in max heap
} Mediator;
 
/*--- Helper Functions ---*/
 
//returns 1 if heap[i] < heap[j]
inline int mmless(Mediator* m, int i, int j);
 
//swaps items i&j in heap, maintains indexes
int mmexchange(Mediator* m, int i, int j);
 
//swaps items i&j if i<j;  returns true if swapped
inline int mmCmpExch(Mediator* m, int i, int j);
 
//maintains minheap property for all items below i.
void minSortDown(Mediator* m, int i);
 
//maintains maxheap property for all items below i. (negative indexes)
void maxSortDown(Mediator* m, int i);
 
//maintains minheap property for all items above i, including median
//returns true if median changed
inline int minSortUp(Mediator* m, int i);
 
//maintains maxheap property for all items above i, including median
//returns true if median changed
inline int maxSortUp(Mediator* m, int i);


/*--- Public Interface ---*/
 
 
//creates new Mediator: to calculate `nItems` running median. 
//mallocs single block of memory, caller must free.
Mediator* MediatorNew(int nItems); 
 
//Inserts item, maintains median in O(lg nItems)
void MediatorInsert(Mediator* m, Item v);
 
//returns median item (or average of 2 when item count is even)
Item MediatorMedian(Mediator* m);

// this would appear to be the function that actually gets called for external interfacing
void runningMedian(ffadata* inbuffer, ffadata* outbuffer, int window, int nsamps);

#endif /* RUNNINGMEDIAN_H */
