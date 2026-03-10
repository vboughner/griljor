/* gets a bitmap file in, makes a mask for it, and writes the mask to 
 * stdout */

#include <stdio.h>
char *make_mask();

main (int argc, char **argv) 
{
  char *mask_name;
  char *data,*result;
  int width,height,x_hot,y_hot,stat;
  int i;

  /* read name of bitmap file (argument #1) */
  if (argc != 2) {
    printf("\nUsage:  %s filename > outfile\n\n", argv[0]);
    exit(1);
  }
  mask_name = (char *)malloc(strlen(argv[1] + 6));
  if (!mask_name) {
    fprintf(stderr,"Error: Malloc failed in main\n");
    exit(1);
  }
  /* read bitmap file */
  stat = XXReadBitmapFile(argv[1],&width,&height,&data,&x_hot,&y_hot);
  if (!stat || width != 32 || width != 32) {
    fprintf(stderr,"Error in file-read\n");
    exit(1);
  }
    
/* make mask */
  result = make_mask(data);

/* write it back out */
  XXWriteBitmapFile("-","mask",width,height,result,x_hot,y_hot);
}
