/***************************************************************************
 * @(#) heap.h 1.5 - last change made 08/07/92
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

/* header for generic heap implementation */

typedef struct _heapelement {
	long key;		/* key of the heap */
	void *data;		/* pointer to data to store in heap */
} HeapElement;



typedef struct _heapinfo {
	int top_is_maximum;	/* TRUE if top of heap is highest key */
	int elements_allocated;	/* amount of space in the heap */
	int elements_filled;	/* number of elements currently in use */
	HeapElement *array;	/* pointer to the heap array */
	void *freeproc;		/* procedure to use to free data */
} HeapInfo;

	
