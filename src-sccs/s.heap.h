h00767
s 00000/00000/00039
d D 1.5 92/08/07 01:04:50 vbo 5 4
c source copied to a separate tree for work on new map and object format
e
s 00000/00000/00039
d D 1.4 91/09/10 21:41:23 labc-4lc 4 3
c 
e
s 00001/00000/00038
d D 1.3 91/09/10 13:32:05 labc-4lc 3 2
c 
e
s 00001/00000/00037
d D 1.2 91/09/08 22:03:31 labc-4lc 2 1
c implemented bubbling of the heap
e
s 00037/00000/00000
d D 1.1 91/09/06 22:41:33 labb-3li 1 0
c date and time created 91/09/06 22:41:33 by labb-3li
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

/* header for generic heap implementation */

typedef struct _heapelement {
	long key;		/* key of the heap */
	void *data;		/* pointer to data to store in heap */
} HeapElement;



typedef struct _heapinfo {
I 2
	int top_is_maximum;	/* TRUE if top of heap is highest key */
E 2
	int elements_allocated;	/* amount of space in the heap */
	int elements_filled;	/* number of elements currently in use */
	HeapElement *array;	/* pointer to the heap array */
I 3
	void *freeproc;		/* procedure to use to free data */
E 3
} HeapInfo;

	
E 1
