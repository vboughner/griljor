h17620
s 00000/00000/00120
d D 1.12 92/08/07 21:32:37 vbo 12 11
c fixes for sun port merged in
e
s 00025/00015/00095
d D 1.11 92/03/02 22:06:41 vanb 11 10
c finished getting editmap to work on basics
e
s 00040/00002/00070
d D 1.10 91/12/13 19:33:39 labc-4lc 10 9
c fixed bug that made a completely blank object unreadable and made it possible to move object properties into an old/outdated state
e
s 00002/00001/00070
d D 1.9 91/11/27 13:41:14 labc-4lc 9 8
c fixed string in objects so space is malloced, not static
e
s 00004/00000/00067
d D 1.8 91/09/15 20:53:10 labc-4lc 8 7
c made dialog.c a little more generic, changed obtor to deal with it
e
s 00006/00000/00061
d D 1.7 91/09/05 21:28:34 labb-3li 7 6
c made obtor use the new objprops.c code for dialog lists
e
s 00001/00001/00060
d D 1.6 91/07/04 15:36:47 labc-3id 6 5
c changes made to help effect new object set
e
s 00001/00001/00060
d D 1.5 91/06/30 20:27:36 labc-3id 5 4
c fixed up editing order of properties for obtor
e
s 00008/00001/00053
d D 1.4 91/06/30 18:39:56 labc-3id 4 3
c halfway done fixing up new object structure in griljor
e
s 00001/00000/00053
d D 1.3 91/05/17 02:05:44 labc-3id 3 2
c Changed for showing object numbers
e
s 00031/00009/00022
d D 1.2 91/03/24 18:45:35 labc-3id 2 1
c 
e
s 00031/00000/00000
d D 1.1 91/02/16 13:00:24 labc-3id 1 0
c date and time created 91/02/16 13:00:24 by labc-3id
e
u
U
f e 0
t
T
I 10
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

E 10
I 1
D 11
/* stuff for dialogs */
E 11
I 11
/* dialog ids (don't change these, it would make many files unreadable) */
E 11
D 10
#define NEWCOL -1
E 10
#define BLANKBOX 0
#define BOLBOX 1
#define INTBOX 2
#define TITLEBOX 3
#define STRBOX 4
#define LSTRBOX 5
#define QUITBOX 6
#define MARKSHO 7
#define MARKHID 8
I 2
#define ICONPIC 9
#define LINKPIC 10
I 4
D 9
#define ENDLIST 11
E 9
I 9
#define HIDSTR 11
D 10
#define ENDLIST 12
E 10
I 10
#define NEWCOL 12
D 11
#define ENDLIST 13
E 11
I 11
#define ENDLIST 13	/* marker for end of one diag description */
#define LONGINT 14
E 11

I 11


E 11
/* stuff for keeping track of old properties no longer in use */
#define OLDDIAG			128
D 11
#define OLDBLANKBOX		(0 | OLDDIAG)
#define OLDBOLBOX		(1 | OLDDIAG)
#define OLDINTBOX		(2 | OLDDIAG)
#define OLDTITLEBOX		(3 | OLDDIAG)
#define OLDSTRBOX		(4 | OLDDIAG)
#define OLDLSTRBOX		(5 | OLDDIAG)
#define OLDQUITBOX		(6 | OLDDIAG)
#define OLDMARKSHO		(7 | OLDDIAG)
#define OLDMARKHID		(8 | OLDDIAG)
#define OLDICONPIC		(9 | OLDDIAG)
#define OLDLINKPIC		(10 | OLDDIAG)
#define OLDHIDSTR		(11 | OLDDIAG)
#define OLDNEWCOL		(12 | OLDDIAG)
E 11
I 11
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
E 11

E 10
E 9
E 4
E 2

I 11

E 11
D 2

E 2
I 2
/* OFFSET and NOOFFSET take care of TWO fields! */
E 2
#define OFFSET(a,b) (((char *) &(a.b)) - ((char *) &(a))),sizeof(a.b)
I 2
#define NOOFFSET       0,0
E 2

D 2
typedef struct _item {
	int nomark:1;
        int x1,y1,x2,y2;
	int index;
	long value;
      } DiagEntry;
E 2
I 2
#define COLPAD   4
#define BITPAD  2
D 4
#define COLWIDTH 15   /* number of CHARACTERS per column */
E 4
I 4
D 5
#define COLWIDTH 26   /* number of CHARACTERS per column */
E 5
I 5
#define COLWIDTH 21   /* number of CHARACTERS per column */
E 5
E 4
#define STRMAXSIZE 200
E 2

I 7
#define BITMAPFIELD  1
#define MASKFIELD    2
#define MASKEDFIELD  3
E 7
D 2
typedef struct _dialog {
E 2
I 2

I 11
#define NULLID  		(0)	/* use for all non-variable props */
#define END_OF_SECTION_ID	(32767)	/* special id, don't use in the
					   definitions in objprops.c */
E 11
I 7

I 11

E 11
E 7
typedef struct _item 
{
  int x1,y1,x2,y2;
  long value;
} DialogEntry;

I 7

E 7
typedef struct _dialoglist 
{
E 2
  char *name;
  int type;
  int offset;  /* REMEMBER! the OFFSET macro makes TWO fields! */
  int size;    /* so size must immediately follow offset */
  int (*call)();
D 2
  void *data;
E 2
I 2
  long data;
I 4
  int id;      /* Id number for purposes of saving and loading files.
D 6
		  Id numbers should be between 1 and 32765, and each
E 6
I 6
		  Id numbers should be between 2 and 32765, and each
E 6
		  object flag field needs a unique one. */
E 4
} DialogList;
I 7

E 7

I 4
#ifndef NOT_OBTOR

E 4
typedef struct _dialog 
{
  Window win;
I 11
  int readonly:1;
E 11
  int changed:1;
  DialogList *list;
  DialogEntry *ent;
  int count,width,height;
  XFontStruct *fi;
I 8
  Pixmap *pixlist;
  int pixlistsize;
  Cursor handcursor, watchcursor;
  Window *windowlist;
E 8
  int (*hook)();
I 3
  int (*key)();
E 3
E 2
} Dialog;
I 2

Dialog *make_dialog();
E 2

I 4
#endif
E 4
E 1
