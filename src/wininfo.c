

#include "obtor.h"


/*****************************************************************/

HideWindowInfo()
{
  int i;

  window_info_up = FALSE;
  for (i = NORM_WINDOW;i < MAX_BUFFERS;i++)  /* not to the cut buffer */
    {
      AdjArrowWindows(i);
      XUnmapWindow(display,infowin[i]);
      XCopyArea(display,objwin[i],objwin[i],mainGC,
		0,BITMAP_HEIGHT+GRAB_SPACING,
		OBJWIN_W(i),OBJWIN_H(i)-BITMAP_HEIGHT-GRAB_SPACING,
		0,0);

      XClearArea(display,objwin[i],0,OBJWIN_H(i)-BITMAP_HEIGHT-GRAB_SPACING,
		 0,0,True);
    }
}

/*****************************************************************/

ShowWindowInfo()
{
  int i;

  window_info_up = TRUE;
  for (i = NORM_WINDOW;i < MAX_BUFFERS;i++)  /* not to the cut-buffer */
    {
      AdjArrowWindows(i);
      XCopyArea(display,objwin[i],objwin[i],mainGC,
		0,0,
		OBJWIN_W(i),OBJWIN_H(i)-BITMAP_HEIGHT-GRAB_SPACING,
		0,BITMAP_HEIGHT+GRAB_SPACING);
      XMapWindow(display,infowin[i]);
    }
}

/*****************************************************************/

redraw_info_window(winnum)
int winnum;
{
  char message[1024],data[100];

  if (!window_info_up)
    return;

  XClearWindow(display,infowin[winnum]);

  if (file_changed[winnum])
    strcpy(data,"  (modified)");
  else 
    strcpy(data,"");

  sprintf(message,"#%d:%s%s ",winnum,objfile_name[winnum],data);
  text_write(infowin[winnum],regfont,OBTINFO_PAD,OBTINFO_PAD,0,0,message);

  if (transmark == winnum)
    strcpy(data,"  *Destination*");
  else
    strcpy(data,"");

  if (all_shown[winnum])
    sprintf(message,"%d Objects%s ",num_objects[winnum],data);
  else
    sprintf(message,"%d Objects (%d-%d)%s ",
	    num_objects[winnum],arrowbottom[winnum],arrowtop[winnum],data);
  text_write(infowin[winnum],regfont,OBTINFO_PAD,OBTINFO_PAD,0,1,message);
}
