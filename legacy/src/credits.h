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

