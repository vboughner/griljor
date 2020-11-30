h47641
s 00000/00000/00012
d D 1.2 92/08/07 01:00:38 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00012/00000/00000
d D 1.1 91/05/17 04:16:20 labc-3id 1 0
c date and time created 91/05/17 04:16:20 by labc-3id
e
u
U
f e 0
t
T
I 1
/* typedef for a linked list of programmers and their contributions */

typedef struct _credits {
	char	*name;		/* name of programmer */
	char	*contrib1;  /* programmer's main contribution */
	char	*contrib2;  /* programmer's main contribution (second line) */
	Pixmap  face;
	Pixmap  mask;
	int width, height;
	struct _credits *next;
} Credits;

E 1
