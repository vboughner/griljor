char get_nth_bit(int num,char *source)
/* returns either zero or one depending on the value of the
 * nth bit from the source pointer */
{
  while ((num / (sizeof(char) * 8)) >= 1) {
    num -= (sizeof(char) * 8);
    source ++;
  }
  return ((*source >> num) & 1);

}



char getBit(char *source,
		      int wpos,int hpos,
		      int wsize,int hsize)
/* returns the specified bit from source */
{
  return get_nth_bit(wpos,source + hpos * wsize);
}



put_nth_bit(char bit, int num,char *target)
/* puts bit into the num'th bit of target */

{
  while ((num / (sizeof(char) * 8)) >= 1) {
    num -= (sizeof(char) * 8);
    target ++;
  }
 *target |= (bit << num);
}



putBit(char *source,
		      int wpos,int hpos,
		      int wsize,int hsize,
		      char bit)
/* returns the specified bit from source */
{
  put_nth_bit(bit,wpos,source + hpos * wsize);
}
