/* Burt's methods of getting information from the user
   Van Boughner    May  1989     */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "burt.h"
#include "extern.h"



remove_first_space(line)
char *line;
{
  char result[LINE_LENGTH];

  if (line[0]==' ')  strcpy(result, line+1);
  else  strcpy(result, line);

  strcpy(line, result);
}


make_first_char_capital(line)
char *line;
{
  int diff;

  diff = 'A' - 'a';
  if (islower(line[0]))  line[0] += diff;
}


make_into_lines(line)
char *line;
{
  int line_start, i;
  int line_length = 79;

  line_start = 0;
  while (strlen(line) - line_start > line_length) {
    i = line_start + line_length;
    while (line[i]!=' ')  i--;
    line[i] = '\n';
    line_start = i + 1;
  }
}



remove_twiddles(s)
char *s;
{
  int i, j = 0;
  char new[LINE_LENGTH];

  for (i=0; s[i]; i++)
    if (s[i] != '~')  new[j++] = s[i];

  new[j] = '\0';
  strcpy(s, new);
}
