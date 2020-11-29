#include "obtor.h"  

char buf[1000];

/*****************************************************************/

main()
{
  printf("#include \"obtor.h\"\n");
  printf("\nObjInfo tobj;\n");
  printf("\nmain()\n{\n");

  while(ReadUntilOffset())
    SubstituteOffset();

  printf("}\n");
  exit(0);
}

/*****************************************************************/

char offstr[] = "OFFSET";

ReadUntilOffset()
{
  int c,pos,i;

  printf("  printf(\"");
  while((c = getchar()) != EOF)
    {
      for (pos = 0;c == offstr[pos];c = getchar())
	{
	  pos++;
	  if (offstr[pos] == '\0')
	    {
	      printf("\");\n");
	      return(1);
	    }
	 }

      for (i = 0;i < pos;i++)
	putchar(offstr[i]);

      if (c == '\n')
	printf("\\n\");\n  printf(\"");
      else if (c == '\\')
	printf("\\\\");
      else if (c == '"')
	printf("\\\"");
      else
	putchar(c);
    }

  printf("\");\n");
  return(c != EOF);
}

/*****************************************************************/

SubstituteOffset()
{
  int c,pos;

  do
    c = getchar();
  while(isspace(c));

  if ((c == EOF)||(c != '('))
    exit(1);
  
  pos = 0;
  c = getchar();
  while((c != ')')&&(c != EOF)&&(c != '\n'))
    {
      buf[pos++] = c;
      c = getchar();
    }
  buf[pos] = '\0';

  if ((c != ')'))
    exit(1);

  printf("  printf(\" %%d, %%d \",OFFSET(%s));\n",buf);
}

/*****************************************************************/





