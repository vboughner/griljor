unsigned short get_nth_bit();
void put_nth_bit(unsigned short bit, int num, unsigned short *target);

unsigned short getBit(unsigned short *source,
		      int wpos,int hpos,
		      int wsize,int hsize)
/* returns the specified bit from source */
{
  return get_nth_bit(wpos,source + hpos * wsize);
}

unsigned short get_nth_bit(int num,unsigned short *source)
/* returns either zero or one depending on the value of the
 * nth bit from the source pointer */

{
  while ((num / (sizeof(short) * 8)) >= 1) {
    num -= (sizeof(short) * 8);
    source ++;
  }
  return ((*source >> num) & 1);

}
unsigned short putBit(unsigned short *source,
		      int wpos,int hpos,
		      int wsize,int hsize,
		      unsigned short bit)
/* returns the specified bit from source */
{
  put_nth_bit(bit,wpos,source + hpos * wsize);
}

void put_nth_bit(unsigned short bit, int num,unsigned short *target)
/* puts bit into the num'th bit of target */

{
  while ((num / (sizeof(short) * 8)) >= 1) {
    num -= (sizeof(short) * 8);
    target ++;
  }
 *target |= (bit << num);
}
