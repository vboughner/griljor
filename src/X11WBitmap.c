/* Copyright    Massachusetts Institute of Technology 1985 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <string.h>

int XXWriteBitmapFile(filename, name, width, height, data, x_hot, y_hot)
char 	       *filename;
char	       *name;
register int   width, height;	/* RETURN; must be non-NULL */
char	       *data;		/* RETURN */
int 	       x_hot, y_hot;	/* RETURN; may be NULL */
{
  char variable[81];
  int  status, value, i, data_length;
  FILE *file;

  if (strcmp("-", filename) == 0) file = stdout;
  else file = fopen(filename, "w");

  if (file==NULL) return 0;

  fprintf(file,"#define %s_width %d\n",name,width);
  fprintf(file,"#define %s_height %d\n",name,height);
  if ((x_hot >= 0) && (y_hot >= 0)) {
    fprintf(file,"#define %s_x_hot %d\n",name,x_hot);
    fprintf(file,"#define %s_y_hot %d\n",name,y_hot);
  }
  fprintf(file,"static char %s_bits[] = {\n   0x%.2x,",name,(int) data[0]);
  data_length = ((width - 1) / 8 + 1) * (height);

  for (i=1; i<data_length - 1; i++) {
    if ((i % 4) == 0) fprintf(file,"\n   ");
    fprintf(file,"0x%.2x, ",(int) *(data+i));
  }
  fprintf(file,"0x%.2x};\n",(int) *(data + data_length));

  fclose(file);
  return 1;
}


