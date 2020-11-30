h14475
s 00000/00000/00011
d D 1.4 92/08/07 01:01:23 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00008/00004/00003
d D 1.3 91/08/29 01:39:51 vanb 3 2
c fixed up more compatibility problems
e
s 00001/00001/00006
d D 1.2 91/08/27 23:57:38 vanb 2 1
c 
e
s 00007/00000/00000
d D 1.1 91/05/17 04:16:28 labc-3id 1 0
c date and time created 91/05/17 04:16:28 by labc-3id
e
u
U
f e 0
t
T
I 1
D 2
char *mymalloc(unsigned size);
E 2
I 2
char *mymalloc();
E 2
D 3
Pixmap MyXCreatePixmap(Display *display, Window window, 
		       int width, int height, int depth);
Pixmap MyXCreatePixmapFromBitmapData(Display *display, Window window,
E 3
I 3
/* Pixmap MyXCreatePixmap(Display *display, Window window, 
		       int width, int height, int depth); */
/* Pixmap MyXCreatePixmapFromBitmapData(Display *display, Window window,
E 3
				     char *bits, int width, int height,
				     unsigned long fg, unsigned long bg,
D 3
				     int depth);
E 3
I 3
				     int depth); */
Pixmap MyXCreatePixmap();
Pixmap MyXCreatePixmapFromBitmapData();


E 3
E 1
