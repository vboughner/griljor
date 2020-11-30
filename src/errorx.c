#include <stdio.h>
#include "standardx.h"
  
  /*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  /* Procedure handle_error Written by Eric van Bezooijen (from proj1)
     Purpose      : When called, it looks at the error-condition code which
     is passed to it, and :
     
     1] Prints out only the text string passed to it if the
     error-code is NULL.
     2] Prints out "warning" and the text string passed to it
     if the error-code is WARNING.
     3] Prints out "Fatal error" and the text string passed to
     it if the error-code is FATAL.  The program also exits.
     4] If the program is passed a error-condition that is
     not defined, it will call itself with a FATAL error.
     
     Takes        : An error condition (NULL, WARNING, FATAL), and a text
     string to be displayed.
     Returns      : Nothing. */
  /*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  
  void handle_error (error_condition, ptr_text)
int                error_condition;
char               *ptr_text;
{
  switch (error_condition)
    {
    case NULL: /* Just print the given string */
      (void ) printf ("%s \n", ptr_text);
      break;
      
    case WARNING: /* Give a warning */
      (void) printf ("Warning : %s \n", ptr_text);
      break;
      
    case FATAL: /* Give a warning, then exit */
      (void ) printf ("Fatal Error: %s \n", ptr_text);
      exit (-1);
      
    case IGNORED: /* Tell the user that it will be ignored */
      (void ) printf ("%s Ignored. \n", ptr_text);
      break;
      
    default: /* If incorrect input is given */
      handle_error (FATAL, "Error in error procedure call.");
      break;
    }
}
