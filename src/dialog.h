/***************************************************************************
 * @(#) dialog.h 1.12 - last change made 08/07/92
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

/* dialog ids (don't change these, it would make many files unreadable) */
#define BLANKBOX 0
#define BOLBOX 1
#define INTBOX 2
#define TITLEBOX 3
#define STRBOX 4
#define LSTRBOX 5
#define QUITBOX 6
#define MARKSHO 7
#define MARKHID 8
#define ICONPIC 9
#define LINKPIC 10
#define HIDSTR 11
#define NEWCOL 12
#define ENDLIST 13	/* marker for end of one diag description */
#define LONGINT 14



/* stuff for keeping track of old properties no longer in use */
#define OLDDIAG			128
#define OLDBLANKBOX		(BLANKBOX | OLDDIAG)
#define OLDBOLBOX		(BOLBOX | OLDDIAG)
#define OLDINTBOX		(INTBOX | OLDDIAG)
#define OLDTITLEBOX		(TITLEBOX | OLDDIAG)
#define OLDSTRBOX		(STRBOX | OLDDIAG)
#define OLDLSTRBOX		(LSTRBOX | OLDDIAG)
#define OLDQUITBOX		(QUITBOX | OLDDIAG)
#define OLDMARKSHO		(MARKSHO | OLDDIAG)
#define OLDMARKHID		(MARKHID | OLDDIAG)
#define OLDICONPIC		(ICONPIC | OLDDIAG)
#define OLDLINKPIC		(LINKPIC | OLDDIAG)
#define OLDHIDSTR		(HIDSTR | OLDDIAG)
#define OLDNEWCOL		(NEWCOL | OLDDIAG)
#define OLDLONGINT		(LONGINT | OLDDIAG)



/* OFFSET and NOOFFSET take care of TWO fields! */
#define OFFSET(a,b) (((char *) &(a.b)) - ((char *) &(a))),sizeof(a.b)
#define NOOFFSET       0,0

#define COLPAD   4
#define BITPAD  2
#define COLWIDTH 21   /* number of CHARACTERS per column */
#define STRMAXSIZE 200

#define BITMAPFIELD  1
#define MASKFIELD    2
#define MASKEDFIELD  3

#define NULLID  		(0)	/* use for all non-variable props */
#define END_OF_SECTION_ID	(32767)	/* special id, don't use in the
					   definitions in objprops.c */


typedef struct _item 
{
  int x1,y1,x2,y2;
  long value;
} DialogEntry;


typedef struct _dialoglist 
{
  char *name;
  int type;
  int offset;  /* REMEMBER! the OFFSET macro makes TWO fields! */
  int size;    /* so size must immediately follow offset */
  int (*call)();
  long data;
  int id;      /* Id number for purposes of saving and loading files.
		  Id numbers should be between 2 and 32765, and each
		  object flag field needs a unique one. */
} DialogList;


#ifndef NOT_OBTOR

typedef struct _dialog 
{
  Window win;
  int readonly:1;
  int changed:1;
  DialogList *list;
  DialogEntry *ent;
  int count,width,height;
  XFontStruct *fi;
  Pixmap *pixlist;
  int pixlistsize;
  Cursor handcursor, watchcursor;
  Window *windowlist;
  int (*hook)();
  int (*key)();
} Dialog;

Dialog *make_dialog();

#endif
