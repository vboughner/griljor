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
