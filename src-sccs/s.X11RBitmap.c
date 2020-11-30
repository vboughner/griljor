h49844
s 00000/00000/00099
d D 1.2 92/08/07 01:00:01 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00099/00000/00000
d D 1.1 91/02/16 12:53:33 labc-3id 1 0
c date and time created 91/02/16 12:53:33 by labc-3id
e
u
U
f e 0
t
T
I 1
/* Copyright    Massachusetts Institute of Technology 1985 */

#include <X11/Xlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

extern int errno;

Status XXReadBitmapFile(filename, width, height, data, x_hot, y_hot)
char		*filename;
int		*width, *height;/* RETURN; must be non-NULL */
char		**data;		/* RETURN */
int		*x_hot, *y_hot;	/* RETURN; may be NULL */
{
  char tmp, variable[81];
  int  status, value, i, x, data_length;
  FILE *file;

  if (strcmp("-", filename) == 0) file = stdin;
  else file = fopen(filename, "r");

  if (file==NULL) return 0;

  *width = *height = -1;
  if (x_hot) *x_hot = -1;
  if (y_hot) *y_hot = -1;

  while ((status = fscanf(file, "#define %80s %d\n", variable, &value))==2) {
    if (StringEndsWith(variable, "width")) 
      *width = value;
    else if (StringEndsWith(variable, "height"))
      *height = value;
    else if (StringEndsWith(variable, "x_hot")) {
      if (x_hot) *x_hot = value;
    }
    else if (StringEndsWith(variable, "y_hot")) {
      if (y_hot) *y_hot = value;
    }
  }

  if (*width <= 0) {
    fclose(file);
    errno = EINVAL;
    return -1;
  }

  if (*height <= 0) {
    fclose(file);
    errno = EINVAL;
    return -2;
  }

  data_length = ((*width - 1) / 8 + 1) * (*height);
  *data = (char *) malloc(data_length);
  data_length /= sizeof(char);
  if (*data == NULL) {
    fclose(file);
    return -3;
  }

  status = fscanf(file, "static char %80s = { 0x%2x", variable, &x);
  **data = x;

  if ((status != 2) || !StringEndsWith(variable, "bits[]")) {
    free((char *) *data);
    fclose(file);
    errno = EINVAL;
    return -4;
  }

  for (i=1; i<data_length; i++) {
    /* fill in ith element of array */
    status = fscanf(file, ", 0x%2x", &x);
    *(*data + i) = (char) x;
    if (status != 1) {
      free((char *) *data);
      fclose(file);
      errno = EINVAL;
      return -5;
    }
  }
  fclose(file);
  return 1;
}


/* StringEndsWith returns TRUE if "s" ends with "suffix", else FALSE */

int StringEndsWith(s, suffix)
char *s, *suffix;
{
  int s_len = strlen(s);
  int suffix_len = strlen(suffix);
  return (strcmp(s + s_len - suffix_len, suffix) == 0);
}

          
    
E 1
