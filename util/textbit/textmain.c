
/* reads a bitmap in x10 file format and outputs it using textbit */

main()
{
  int width,height,x_hot,y_hot;
  short *data;

  XXReadBitmapFile("-",&width,&height,&data,&x_hot,&y_hot);
  textbit(width,height,data);
}
