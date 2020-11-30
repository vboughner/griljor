h50869
s 00000/00000/00022
d D 1.2 92/08/07 01:04:31 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00022/00000/00000
d D 1.1 91/07/04 11:58:42 labc-3id 1 0
c date and time created 91/07/04 11:58:42 by labc-3id
e
u
U
f e 0
t
T
I 1
/* vsys.h
 * Volvox Software Group
 * February 1991
 * Specs: Van A. Boughner
 * Author: Van A. Boughner
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "volvox.h"

char *valloc();
char *concat_filename();
char *expand_twiddle();
char *username();
char *user_and_host();
char *home_dir();
char *twostud();
char *string_time();
void zero_memory();
FILE *open_local_file();
E 1
