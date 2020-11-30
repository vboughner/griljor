/***************************************************************************
 * @(#) heap.c 1.5 - last change made 08/07/92
 *
 * Copyright 1991 Van A. Boughner, Mel Nicholson, and Albert C. Baker III
 * All Rights Reserved.
 *
 * Griljor by Van A. Boughner
 *            Mel Nicholson
 *            Albert C. Baker III
 *	      Trevor Pering
 *	      Eric van Bezooijen
 *
 * Copyright information is in the README file.  Note that this source code
 * may not be altered by anyone but the authors, except under the conditions
 * outlined in the copyright notice.
 *
 * Released under the supervision of the Volvox Software Group.
 * Many thanks to all the CS undergrads at U.C. Berkeley that helped us out.
 *
 **************************************************************************/

/* code for implementing heaps that key on a number */

#include <stdio.h>
#include "config.h"
#include "def.h"


static void bubble_up(heap, k)
/* bubbles an element in the heap upward through the heap until the
   proper heap formation is restored */
HeapInfo heap;
int k;
{
  int j;
  long t;
  HeapElement tmp;
  
  t = heap->array[k].key;
  tmp.key = heap->array[k].key;
  tmp.data = heap->array[k].data;

  while (k > 0) {
    j = (k - 1) / 2;
      
    /* break out of loop if we are done bubbling */
    if (((heap->top_is_max) && (heap->array[j].key >= t)) ||
	((!heap->top_is_max) && (heap->array[j].key <= t))) break;

    heap->array[k].key = heap->array[j].key;
    heap->array[k].data = heap->array[j].data;
    k = j;
  }

  heap->array[k].key = tmp.key;
  heap->array[k].data = tmp.data;
}



static void bubble_down(heap, k)
/* bubbles an element downward in the heap until proper heap configuration
   requirements are met. */
HeapInfo *heap;
int k;
{
  int j;
  long t;
  HeapElement tmp;

  t = heap->array[k].key;
  tmp.key = heap->array[k].key;
  tmp.data = heap->array[k].data;

  j = k * 2 + 1;

  while (j < heap->elements_filled) {

      if ((j + 1) < heap->elements_filled) {
	if ((heap->top_is_max &&
	     (heap->array[j+1].key > heap->array[j].key)) ||
	    (!heap->top_is_max &&
	     (heap->array[j+1].key < heap->array[j].key)))
 	  j++;
      }
      
      /* break out of loop if we are done bubbling */
      if (((heap->top_is_max) && (heap->array[j].key <= t)) ||
	  ((!heap->top_is_max) && (heap->array[j].key >= t))) break;

      heap->array[k].key = heap->array[j].key;
      heap->array[k].data = heap->array[j].data;
      k = j;
      j = k * 2 + 1;
    }
  
  heap->array[k].key = tmp.key;
  heap->array[k].data = tmp.data;
}



/* ====================== I N T E R F A C E ============================ */


HeapInfo *InitializeHeap(size, topmax, freeproc)
/* initializes the heap and allocates space for 'size' elements.  Caller
   should call FreeHeap() later on to free the heap created by this routine.
   This routine returns a pointer to the heapinfo structure of the new
   heap.  'topmax' should be passed as TRUE if the caller wants a heap where
   the highest numbered key is on the top.  Caller should pass FALSE for
   a heap where the lowest numbered key is on top.  'freeproc' is the
   procedure that the caller will want used to free the data stored under
   the keys when those keys are deleted later on.  if 'freeproc' is passed
   as NULL, then we can assume user does not want data freed. */
int size;
int topmax;
void *freeproc;
{
  HeapInfo *new = (HeapInfo *) malloc(sizeof(HeapInfo));
  demand(new, "not enough memory for a heap");

  new->array = (HeapElement *) malloc(sizeof(HeapElement) * size);
  demand(new->array, "not enough memory for heap elements");
  new->elements_allocated = size;
  new->elements_filled = 0;
  new->top_is_maximum = topmax;
  new->freeproc = freeproc;
}


void AddToHeap(heap, key, data)
/* allows caller to add something to the heap.  Caller must provide a key
   and a pointer to some data.  The data should be a newly allocated copy if
   the caller is going to request that FreeHeap() free it later with the
   'freeproc' that the caller specified in InitializeHeap().
   If there is not enough space on the heap, then the size of the space
   allocated for the heap is doubled. */
HeapInfo *heap;
long key;
void *data;
{
  demand(heap, "AddToHeap: bad heap info structure pointer");

  if (heap->elements_filled == heap->elements_allocated) {
    int newsize = max(heap->elements_allocated * 2, 1);
    heap->array = (HeapElement *)
		    realloc(heap->array, sizeof(HeapElement) * newsize);
    heap->elements_allocated = newsize;
  }

  heap->array[heap->elements_filled].key = key;
  heap->array[heap->elements_filled].data = data;
  bubble_up(heap, heap->elements_filled);
  heap->elements_filled++;
}



void FreeHeap(heap)
/* frees a heap and all the elements within it.  If a 'freeproc' was
   specified when the heap was created, it will be used to free the data
   associated with each key. */
{
  if (heap) {
    if (heap->array) {
      if (heap->freeproc) {
        int i;
        for (i=0; i<heap->elements_filled; i++)
          if (heap->array[i].data) *(heap->freeproc)(heap->array[i].data);
      }
      free(heap->array);
    }
    free(heap);
  }
}



HeapElement *PeekHeapTop(heap)
/* returns a pointer to the top heap element.  Caller should not alter or
   free the element, but rather, should immediately call RemoveHeapTop().
   Returns NULL if the heap is empty. */
HeapInfo *heap;
{
  HeapElement *result = NULL;

  if (heap)
    if (heap->elements_filled > 0)
      result = heap->array;

  return result;
}



void RemoveHeapTop(heap)
/* removes the top item in the heap, or does nothing if the heap is empty.
   If a 'freeproc' was defined when the heap was created, it will be used
   to free the data pointed to in the top heap element. */
HeapInfo *heap;
{
  if (!heap) return;
  if (!heap->elements_filled) return;
  if (heap->freeproc && heap->array[0].data)
    *(heap->freeproc)(heap->array[0].data);

  heap->elements_filled--;
  heap->array[0].key = heap->array[heap->elements_filled].key;
  heap->array[0].data = heap->array[heap->elements_filled].data;
  heap->array[heap->elements_filled].data = NULL;
  bubble_down(heap, 0);
}



void HeapConsistancy(heap, w)
/* used to check the consistancy of a heap - for debugging
   only.  Call with HeapConsistancy(heap, 0) */
HeapInfo *heap;
int w;
{
  int j;
  
  j = w*2+1;
  if (j >= heap->elements_filled) return;

  if (((heap->top_is_max) && (heap->array[j].key >= heap->array[w].key)) ||
      ((!heap->top_is_max) && (heap->array[j].key <= heap->array[w].key)))
	printf("Bad Heap\n");

  HeapConsistancy(heap, j);

  j++;
  if (j >= heap->elements_filled) return;

  if (((heap->top_is_max) && (heap->array[j].key >= heap->array[w].key)) ||
      ((!heap->top_is_max) && (heap->array[j].key <= heap->array[w].key)))
	printf("Bad Heap\n");

  HeapConsistancy(heap, j);
}



void FilterHeap(heap, key, proc)
/* goes through the heap looking for values greater than key
   (if (heap->top_is_max)), or less than key (if (!heap->top_is_max)).
   When it finds each such element, it passes the element's key and a
   pointer to the element's data to the 'proc' function.  The 'proc'
   function is expected to return a new value for the key of the element
   for reinsertion into the heap, or -1 if the element is to be removed
   from the heap.
HeapInfo *heap;
long key;
long *proc;
{
  long result;
  HeapElement *ptr = PeekHeapTop(heap);

  while (ptr && ((heap->top_is_max && ptr->key > key) ||
		 (!heap->top_is_max && ptr->key < key))) {

  if (proc) result = *(proc)(ptr->key, ptr->data);
  if (result == -1) RemoveHeapTop(heap);
  else {
    
/* === changes effected to here ====== */

  }
}



/*****************************************************************/

/* Check for the need to move any of the missiles we know about.  If any of
   the missiles we check want to die, then delete them from their room list. */

update_missiles()
{
  Missile *current;
  int missle_end, refire,count;

  current = TopOfHeap();
  
  while((current) && (gametime >= current->lastmove + current->wait))
    {
      RemoveTopOfHeap();

      refire = FALSE;
      
      /* handle the individual missile here */
      missle_end = update_individual_missile(current, &refire);
      
      /* remove the missile if it is at it's end */
      if (!missle_end) 
	AddMissileToHeap(current);
      else
	{
	  /* remove the rest of this missiles movement que */
	  free_move_queue(&(current->moveq));

	  /* refire will only be true when missile is 
	     going on to another room and we musn't explode 
	     or drop it yet */
	  
	  if ((one_of_me(current->owner) && (!refire)))
	    drop_or_explode_it(current);
	  
	  free(current);
	}

      current = TopOfHeap();
    }
}
