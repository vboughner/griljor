/* Burt, an Eliza-like programming project in C
   This particular implementation is by Van Boughner    May   1989
   This is a burt personality file tester program       March 1990  */

#include <stdio.h>
#include <string.h>
#include "burt.h"

#define PROGRAM_NAME "testburt"


main(argc, argv)
int argc;
char *argv[];
{
  char s[250], response[300], *query_personality();

  /* check command line for a file argument */
  if (argc != 2) {
    printf("Usage:  %s file_name\n", argv[0]);
    exit(1);
  }

  /* load the file into personality number 0 */
  if (!set_personality(0, argv[1])) {
    printf("%s: bad personality file\n", argv[0]);
    exit(1);
  }

  /* print welcom message */
  printf("--- Burt Personality Tester    by Van A. Boughner    March 1990\n");
  printf("--- Type 'quit' to end testing '%s'\n\n", argv[1]);

  /* get input and responses until 'quit' is encountered */
  do {
    gets(s);
    strcpy(response, query_personality(0, s));
    make_into_lines(response);
    printf("%s\n", response);
  } while (strcmp(s, "quit"));

  printf("\n--- Thanks for using the Burt Tester\n");
}



/* ============= Additions for compatibility ============================= */


char *read_line(fp)
FILE *fp;
{
  int c;
  static char line[300];
  int  i;

  c = getc(fp);
  for (i=0; ((c!='\n') && (c!=EOF) && (i<299)); i++) {
    line[i] = c;
    c = getc(fp);
  }

  line[i] = '\0';
  return line;
}



Gerror(s)
char *s;
{
  fprintf(stderr, "%s: %s\n", PROGRAM_NAME, s);
  exit(1);
}
