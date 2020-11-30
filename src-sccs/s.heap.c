h22088
s 00000/00000/00316
d D 1.5 92/08/07 01:04:48 vbo 5 4
c source copied to a separate tree for work on new map and object format
e
s 00027/00000/00289
d D 1.4 91/09/10 21:41:31 labc-4lc 4 3
c 
e
s 00043/00029/00246
d D 1.3 91/09/10 13:31:59 labc-4lc 3 2
c 
e
s 00131/00082/00144
d D 1.2 91/09/08 22:03:26 labc-4lc 2 1
c implemented bubbling of the heap
e
s 00226/00000/00000
d D 1.1 91/09/06 22:41:26 labb-3li 1 0
c date and time created 91/09/06 22:41:26 by labb-3li
e
u
U
f e 0
t
T
I 1
/***************************************************************************
 * %Z% %M% %I% - last change made %G%
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


I 2
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



E 2
/* ====================== I N T E R F A C E ============================ */


D 2
HeapInfo *InitializeHeap(size)
E 2
I 2
D 3
HeapInfo *InitializeHeap(size, topmax)
E 3
I 3
HeapInfo *InitializeHeap(size, topmax, freeproc)
E 3
E 2
/* initializes the heap and allocates space for 'size' elements.  Caller
   should call FreeHeap() later on to free the heap created by this routine.
   This routine returns a pointer to the heapinfo structure of the new
D 2
   heap.  */
E 2
I 2
   heap.  'topmax' should be passed as TRUE if the caller wants a heap where
   the highest numbered key is on the top.  Caller should pass FALSE for
D 3
   a heap where the lowest numbered key is on top. */
E 3
I 3
   a heap where the lowest numbered key is on top.  'freeproc' is the
   procedure that the caller will want used to free the data stored under
   the keys when those keys are deleted later on.  if 'freeproc' is passed
   as NULL, then we can assume user does not want data freed. */
E 3
E 2
int size;
I 2
int topmax;
I 3
void *freeproc;
E 3
E 2
{
  HeapInfo *new = (HeapInfo *) malloc(sizeof(HeapInfo));
  demand(new, "not enough memory for a heap");

  new->array = (HeapElement *) malloc(sizeof(HeapElement) * size);
  demand(new->array, "not enough memory for heap elements");
  new->elements_allocated = size;
  new->elements_filled = 0;
I 2
  new->top_is_maximum = topmax;
I 3
  new->freeproc = freeproc;
E 3
E 2
}


I 2
void AddToHeap(heap, key, data)
/* allows caller to add something to the heap.  Caller must provide a key
   and a pointer to some data.  The data should be a newly allocated copy if
D 3
   the caller is going to request that FreeHeap() free it later.
E 3
I 3
   the caller is going to request that FreeHeap() free it later with the
   'freeproc' that the caller specified in InitializeHeap().
E 3
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



E 2
D 3
void FreeHeap(heap, freedata)
/* frees a heap and all the elements within it.  If freedata is TRUE, then
   the data associated with each heap element will also be freed. */
E 3
I 3
void FreeHeap(heap)
/* frees a heap and all the elements within it.  If a 'freeproc' was
   specified when the heap was created, it will be used to free the data
   associated with each key. */
E 3
{
  if (heap) {
    if (heap->array) {
D 3
      if (freedata) {
E 3
I 3
      if (heap->freeproc) {
E 3
        int i;
        for (i=0; i<heap->elements_filled; i++)
D 3
          if (heap->array[i].data) free(heap->array[i].data);
E 3
I 3
          if (heap->array[i].data) *(heap->freeproc)(heap->array[i].data);
E 3
      }
      free(heap->array);
    }
    free(heap);
  }
}

D 2
/* ======== changes made up to here Sept 7th ========= */
E 2

D 2
/*****************************************************************/
E 2

D 2
htime(i)
int i;
E 2
I 2
HeapElement *PeekHeapTop(heap)
/* returns a pointer to the top heap element.  Caller should not alter or
   free the element, but rather, should immediately call RemoveHeapTop().
   Returns NULL if the heap is empty. */
HeapInfo *heap;
E 2
{
D 2
  Missile *m;
E 2
I 2
  HeapElement *result = NULL;
E 2

D 2
  m = all_missiles[i];
  if (m == NULL)
    return(0);
  
  return(m->lastmove + m->wait);
}
E 2
I 2
  if (heap)
    if (heap->elements_filled > 0)
      result = heap->array;
E 2

D 2
Missile *TopOfHeap()
{
  if (nummissiles)
    return(all_missiles[0]);
  else
    return(NULL);
E 2
I 2
  return result;
E 2
}

D 2
void AddMissileToHeap(add)
Missile *add;
{
  all_missiles[nummissiles] = add;
  BubbleUp(nummissiles);
  nummissiles++;
  if (nummissiles >= maxmissiles)
    Gerror("Too many missiles!");
}
E 2

D 2
void RemoveTopOfHeap()
{
  if (nummissiles == 0)
    return;
  
  nummissiles--;
E 2

D 2
  all_missiles[0] = all_missiles[nummissiles];
  all_missiles[nummissiles] = NULL;
  
  BubbleDown(0);
}

BubbleUp(k)
int k;
E 2
I 2
D 3
void RemoveHeapTop(heap, freedata)
E 3
I 3
void RemoveHeapTop(heap)
E 3
/* removes the top item in the heap, or does nothing if the heap is empty.
D 3
   If freedata is true, then the caller's data pointed to in the top heap
   element will be freed. */
E 3
I 3
   If a 'freeproc' was defined when the heap was created, it will be used
   to free the data pointed to in the top heap element. */
E 3
HeapInfo *heap;
E 2
{
D 2
  int j,t;
  Missile *m;
  
  m = all_missiles[k];
  t = htime(k);
E 2
I 2
  if (!heap) return;
  if (!heap->elements_filled) return;
D 3
  if (freedata && heap->array[0].data) free(heap->array[0].data);
E 3
I 3
  if (heap->freeproc && heap->array[0].data)
    *(heap->freeproc)(heap->array[0].data);
E 3
E 2

D 2
  while(k > 0)
    {
      j = (k-1)/2;
      
      if (htime(j) <= t)
	break;

      all_missiles[k] = all_missiles[j];
      k = j;
    }

  all_missiles[k] = m;
E 2
I 2
  heap->elements_filled--;
  heap->array[0].key = heap->array[heap->elements_filled].key;
  heap->array[0].data = heap->array[heap->elements_filled].data;
  heap->array[heap->elements_filled].data = NULL;
  bubble_down(heap, 0);
E 2
}

D 2
BubbleDown(k)
int k;
{
  int j,t;
  Missile *m;
E 2

D 2
  m = all_missiles[k];
  j = k*2 + 1;
  t = htime(k);
E 2

D 2
  while (j < nummissiles)
    {
      if (((j+1) < nummissiles)&&(htime(j+1) < htime(j)))
	j++;
      
      if (htime(j) >= t)
	break;
      
      all_missiles[k] = all_missiles[j];
      k = j;
      j = k*2 + 1;
    }
  
  all_missiles[k] = m;
}
E 2
I 2
D 3
/* ======= changes made to here Sept 8th ========= */
E 2

/*****************************************************************/
/* used to check the consistancy of the missile heap - for debugging
  only.  Call with CheckHeap(0) */
  
CheckHeap(w)
E 3
I 3
void HeapConsistancy(heap, w)
/* used to check the consistancy of a heap - for debugging
   only.  Call with HeapConsistancy(heap, 0) */
HeapInfo *heap;
E 3
int w;
{
  int j;
  
  j = w*2+1;
D 3
  if (j >= nummissiles)
    return;
  if (htime(j) < htime(w))
    printf("Bad heap\n");
  CheckHeap(j);
E 3
I 3
  if (j >= heap->elements_filled) return;

  if (((heap->top_is_max) && (heap->array[j].key >= heap->array[w].key)) ||
      ((!heap->top_is_max) && (heap->array[j].key <= heap->array[w].key)))
	printf("Bad Heap\n");

  HeapConsistancy(heap, j);

E 3
  j++;
D 3
  if (j >= nummissiles)
    return;
  if (htime(j) < htime(w))
    printf("Bad heap\n");
  CheckHeap(j);
E 3
I 3
  if (j >= heap->elements_filled) return;

  if (((heap->top_is_max) && (heap->array[j].key >= heap->array[w].key)) ||
      ((!heap->top_is_max) && (heap->array[j].key <= heap->array[w].key)))
	printf("Bad Heap\n");

  HeapConsistancy(heap, j);
E 3
}
I 3



I 4
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
    
E 4
/* === changes effected to here ====== */
I 4

  }
}


E 4
E 3

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
E 1
