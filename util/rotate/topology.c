unsigned char *vflip(),*hflip(),*rotate();

unsigned char *rotate_bitmap(int width, int height, char *bitmap, char code)
/* given a code 0-7, returns a bitmap with the proper rotation:
 * 0 -- identity
 * 1 -- rotate 90 degrees clockwise
 * 2 -- rotate 180 degrees
 * 3 -- rotate 270 degrees clockwise (90 degrees counterclockwise)
 * 4 -- mirror image of identity
 * 5 -- 4 rotated 90 degrees clockwise
 * 6 -- 4 rotated 180 degrees
 * 7 -- 4 rotated 270 degrees clockwise (90 degrees counterclockwise)
 */


{ unsigned char *temp,*result;

  if (width != height) {
    printf("Error in rotate_bitmap: bitmap must be square\n");
    exit(1);
  }

  switch (code) {
    case 0:
      temp = vflip(width,height,bitmap);
      result = vflip(width,height,temp);
      free(temp);
      break;
    case 1:
      result = rotate(width,height,bitmap,1);
      break;
    case 2:
      temp = vflip (width,height,bitmap);
      result = hflip(width,height,temp);
      free(temp);
      break;
    case 3:
      result = rotate(width,height,bitmap,0);
      break;
    case 4: 
      result = hflip(width,height,bitmap);
      break;
    case 5:
      temp = rotate(width,height,bitmap,1);
      result = vflip(width,height,temp);
      free(temp);
      break;
    case 6:
      result = vflip(width,height,bitmap);
      break;
    case 7:
      temp = rotate(width,height,bitmap,0);
      result = vflip(width,height,temp);
      free(temp);
      break;
    default: 
      printf("Error: Bad code in rotate_bitmap: %d\n",code);
      exit(1);
  }
  return result;
}
