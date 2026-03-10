/* gets a bitmap file, changes its appearance according to the following:
 * given a code 0-7, returns a bitmap with the proper rotation:
 * 0 -- identity
 * 1 -- rotate 90 degrees clockwise
 * 2 -- rotate 180 degrees
 * 3 -- rotate 270 degrees clockwise (90 degrees counterclockwise)
 * 4 -- mirror image of identity
 * 5 -- 4 rotated 90 degrees clockwise
 * 6 -- 4 rotated 180 degrees
 * 7 -- 4 rotated 270 degrees clockwise (90 degrees counterclockwise)
 */

#include <stdio.h>
unsigned char *rotate_bitmap();

main (int argc, char **argv) 
{
  char *mask_name;
  unsigned char *data,*result;
  int width,height,x_hot,y_hot,stat;
  int i;

  /* read name of bitmap file (argument #1) */
  if (argc != 3) {
    printf("Error: Bad command line\n");
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
  result = rotate_bitmap(32,32,data,atoi(argv[2]));

/* write it back out */
  XXWriteBitmapFile("-","mask",width,height,result,x_hot,y_hot);
}
