h15435
s 00000/00000/00419
d D 1.3 92/08/07 01:00:24 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00004/00002/00415
d D 1.2 92/01/20 16:45:51 labc-4lc 2 1
c fixed depth arguments for XCreatePixmapFromBitmapData calls
e
s 00417/00000/00000
d D 1.1 91/04/14 19:35:08 labc-3id 1 0
c date and time created 91/04/14 19:35:08 by labc-3id
e
u
U
f e 0
t
T
I 1
#include <stdio.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "standardx.h"
#include "lib.h"
#include "def.h"
#include "password.h"
#include "xx.h" /* This has to be last */
#include "bitmapx.h"
  
  void draw_bitmap_bits (disp)
disp_fields disp;
{
  XSetLineAttributes (disp->display, disp->regular_gc,
                      WINDOW_BORDER, LineSolid, CapButt, JoinMiter);
  
  XDrawLine (disp->display, disp->bitmap_win->win, disp->regular_gc, 
	     165,0,164,BITM_WINDOW_HEIGHT);
  
  XDrawLine (disp->display, disp->bitmap_win->win, disp->regular_gc, 
	     333,0,333,BITM_WINDOW_HEIGHT);
  
  XSetLineAttributes (disp->display, disp->regular_gc,
                      0, LineSolid, CapButt, JoinMiter);
  
  XDrawLine (disp->display, disp->bitmap_win->win, disp->regular_gc, 
	     165,21,333,21);
  
  XDrawLine (disp->display, disp->bitmap_win->win, disp->regular_gc, 
	     165,42,333,42);
  
  /*XDrawLine (disp->display, disp->bitmap_win->win, disp->regular_gc, 
    200,42,200,BITM_WINDOW_HEIGHT);
    
    XDrawLine (disp->display, disp->bitmap_win->win, disp->regular_gc, 
    294,42,294,BITM_WINDOW_HEIGHT); */
  strcpy (disp->line, "");
  strncpy (disp->line, disp->bm_file, MAX_FILE_SIZE);
  (disp->line)[MAX_FILE_SIZE] = '\0';
  text_write (disp->bitmap_win->win, disp->regfont, 188, 3, 
	      0,0, disp->line, disp->display, disp->regular_gc);
  strcpy (disp->line, "");
  strncpy (disp->line, disp->bm_m_file, MAX_FILE_SIZE);
  (disp->line)[MAX_FILE_SIZE] = '\0';
  text_write (disp->bitmap_win->win, disp->regfont, 188, 24,
	      0,0,disp->line, disp->display, disp->regular_gc);
  XCopyArea (disp->display, disp->disc_icon, disp->bitmap_win->win,
	     disp->regular_gc, 0,0, 20, 18, 166, 2);
  XCopyArea (disp->display, disp->disc_icon, disp->bitmap_win->win,
	     disp->regular_gc, 0,0, 20, 18, 166, 23);
  
}

void draw_bit (disp, flag, x, y)
     disp_fields disp;
     boolean flag;
     int x,y;
{
  if (flag)
    XFillRectangle (disp->display, disp->bitmap_win->win,
		    disp->regular_gc, x,y, SIZE , SIZE);
  else
    XDrawRectangle (disp->display, disp->bitmap_win->win,
		    disp->regular_gc, x,y, SIZE, SIZE);
}

void draw_pixmap_big (disp, mask_bitmap, bm_data)
     disp_fields	disp;
     boolean		mask_bitmap; /* TRUE for bitmap FALSE for mask */
     short		*bm_data;
{
  int x,y,i,j;
  int wordwidth; /* width of bitmap in full char ints */
  
  wordwidth = (int)ceil( ((double)WIDTH2) / (double)(sizeof(char) * 8) );
  
  if (mask_bitmap) {x = BM_X; y = BM_Y;}
  else {x = MSK_X; y = MSK_Y;}
  
  XClearArea (disp->display, disp->bitmap_win->win,
	      x,y, SIZE * WIDTH2, SIZE * HEIGHT, False);
  
  for (j = 0; j < HEIGHT ;j++,y += SIZE){
    if (mask_bitmap) x = BM_X;
    else x = MSK_X;
    for (i = 0; i < WIDTH2 ; i++, x += SIZE) {
      if (getBit(bm_data,i,j,wordwidth,HEIGHT))
	draw_bit (disp, TRUE,x,y); 
      else if (disp->grid_on) draw_bit (disp, FALSE,x,y);
    }
  }
}

void transfer_data_to_pixmaps (disp, person)
     disp_fields	disp;
     PersonLink	*person;
{
  disp->person_bitmap = XCreatePixmapFromBitmapData (disp->display,
						     disp->bitmap_win->win, person->person->bitmap,
D 2
						     WIDTH, HEIGHT, disp->fgcolor, disp->bgcolor, 1);
E 2
I 2
						     WIDTH, HEIGHT, disp->fgcolor, disp->bgcolor,
						     DefaultDepth(disp->display, DefaultScreen(disp->display)));
E 2
  disp->person_mask = XCreatePixmapFromBitmapData (disp->display,
						   disp->bitmap_win->win, person->person->mask,
D 2
						   WIDTH, HEIGHT, disp->fgcolor, disp->bgcolor, 1);
E 2
I 2
						   WIDTH, HEIGHT, disp->fgcolor, disp->bgcolor,
						   DefaultDepth(disp->display, DefaultScreen(disp->display)));
E 2
}

void display_bitmaps (disp)
     disp_fields	disp;
{
  unsigned long valuemask;
  XGCValues	values1,values2;
  int		i,j,x,y;
  
  /* Draw the bitmaps on the screen seperately */
  XCopyArea (disp->display, disp->person_bitmap, disp->bitmap_win->win,
	     disp->regular_gc, 0,0, WIDTH, HEIGHT, PIX_B_X, PIX_B_Y);
  XCopyArea (disp->display, disp->person_mask, disp->bitmap_win->win,
	     disp->regular_gc, 0,0, WIDTH, HEIGHT, PIX_M_X, PIX_M_Y);
  
  /* Draw the background */
  for (i = 0, y = PIX_BM_Y; i < 3; i++, y += HEIGHT)
    for (j = 0, x = PIX_BM_X; j < 3; j++, x += WIDTH)
      XCopyArea (disp->display, disp->grey, disp->bitmap_win->win,
		 disp->regular_gc, 0,0, WIDTH, HEIGHT, x,y);
  
  /* First change the gc's to draw the bitmap*/
  valuemask = GCFunction;
  if (disp->fgcolor > 0) {
    values1.function = GXor;
    values2.function = GXandInverted;
  }
  else {
    values1.function = GXand;
    values2.function = GXorInverted;
  }
  
  XChangeGC (disp->display, disp->regular_gc, valuemask, &values1);
  XChangeGC (disp->display, disp->regular_erase_gc, valuemask, &values2);
  
  /* Draw a 3x3 block of bitmaps & their masks */
  for (i = 0, y = PIX_BM_Y; i < 3; i++, y += HEIGHT)
    for (j = 0, x = PIX_BM_X; j < 3; j++, x += WIDTH)
      XCopyArea (disp->display, disp->person_mask, disp->bitmap_win->win,
		 disp->regular_erase_gc, 0,0, WIDTH, HEIGHT, x,y);
  
  for (i = 0, y = PIX_BM_Y; i < 3; i++, y += HEIGHT)
    for (j = 0, x = PIX_BM_X; j < 3; j++, x += WIDTH)
      XCopyArea (disp->display, disp->person_bitmap, disp->bitmap_win->win,
		 disp->regular_gc, 0,0, WIDTH, HEIGHT, x,y);
  
  /* Change it back ! */
  values1.function = GXcopy;
  values2.function = GXcopyInverted;
  
  XChangeGC (disp->display, disp->regular_gc, valuemask, &values1);
  XChangeGC (disp->display, disp->regular_erase_gc, valuemask, &values2);
}


void redraw_bitmap_win (disp)
     disp_fields disp;
{
  int i;
  PersonLink	*person;
  
  XClearWindow (disp->display, disp->bitmap_win->win);
  if (!disp->current_edit_no) return;
  person = disp->file;
  if (!disp->show_bitmaps) return;
  for (i = 1; i < disp->current_edit_no; i++, person = person->next);
  if (!person->person->bitmap_saved) return;
  draw_bitmap_bits (disp);
  transfer_data_to_pixmaps (disp, person);
  draw_pixmap_big (disp, TRUE, person->person->bitmap);
  draw_pixmap_big (disp, FALSE, person->person->mask);
  display_bitmaps (disp);
}

boolean load_bitmap_mask (disp, flag)
     disp_fields	disp;
     boolean		flag; /* TRUE == bitmaps , FALSE == masks */
{
  unsigned int dummy1,dummy2;
  int dummy3,dummy4;
  
  if (flag) { /* Load bitmap */
    
    if (!(strcmp (disp->bm_file, "None")))
      {
	strcpy (disp->message, "No Bitmap File Specified");
	handle_message (disp, TRUE);
	return FALSE;
      }
    else
      {
	if (XReadBitmapFile (disp->display,
			     disp->win,
			     disp->bm_file,
			     &dummy1,
			     &dummy2,
			     &(disp->memory_bitmap),
			     &dummy3,
			     &dummy4) != BitmapSuccess)
	  {
	    strcpy (disp->message, "Error In Reading File");
	    handle_message (disp, TRUE);
	    return FALSE;
	  }
	else
	  {
	    if ( (dummy1 != WIDTH2) || (dummy2 != HEIGHT))
	      {
		strcpy (disp->message, "Error.  Wrong Size Bitmap.");
		handle_message (disp, TRUE);
		disp->memory_bm = FALSE;
		return FALSE;
	      }
	    strcpy (disp->message, "Bitmap Loaded Into Memory");
	    handle_message (disp, TRUE);
	    disp->memory_bm = TRUE;
	    return TRUE;
	  }
      }
  }
  else { /* Load Mask */
    if (!(strcmp (disp->bm_m_file, "None")))
      {
	strcpy (disp->message, "No Mask File Specified");
	handle_message (disp, TRUE);
	return FALSE;
      }
    else
      {
	if (XReadBitmapFile (disp->display,
			     disp->win,
			     disp->bm_file,
			     &dummy1,
			     &dummy2,
			     &(disp->memory_mask),
			     &dummy3,
			     &dummy4) != BitmapSuccess)
	  {
	    strcpy (disp->message, "Error In Reading File");
	    handle_message (disp, TRUE);
	    return FALSE;
	  }
	else
	  {
	    if ( (dummy1 != WIDTH2) || (dummy2 != HEIGHT))
	      {
		strcpy (disp->message, "Error.  Wrong Size Mask.");
		handle_message (disp, TRUE);
		disp->memory_bm = FALSE;
		return FALSE;
	      }
	    strcpy (disp->message, "Mask Loaded Into Memory");
	    handle_message (disp, TRUE);
	    disp->memory_m = TRUE;
	    return TRUE;
	    
	  }
      }
  }
}

boolean transfer_memory_to_pixmaps (disp, flag)
     disp_fields	disp;
     boolean		flag; /* TRUE == bitmap, FALSE == mask */
{
  PersonLink *person;
  
  if (!disp->current_edit_no)
    {
      strcpy (disp->message, "No Person Specified!");
      handle_message (disp, TRUE);
      return FALSE;
    }
  if (flag) {
    if (!disp->memory_bm) 
      {
	strcpy (disp->message, "No Bitmap Stored In Memory!");
	handle_message (disp, TRUE);
	return FALSE;
      }
    else
      {
	person = 
	  (PersonLink *) return_user_link (disp, disp->current_edit_no);
	TransferPixmapToData (disp->memory_bitmap, person->person->bitmap,disp);
	strcpy (disp->message, "Character Has New Bitmap");
	handle_message (disp, TRUE);
	if (flag)
	  {
	    if (person->person->mask)
	      person->person->bitmap_saved = TRUE;
	  }
	else
	  {
	    if (person->person->bitmap)
	      person->person->bitmap_saved = TRUE;
	  }
	return TRUE;
      }
  }
  else {
    if (!disp->memory_m) 
      {
	strcpy (disp->message, "No Mask Stored In Memory!");
	handle_message (disp, TRUE);
	return FALSE;
      }
    else
      {
	person = (PersonLink *)
	  return_user_link (disp, disp->current_edit_no);
	TransferPixmapToData (disp->memory_mask, person->person->mask,disp);
	strcpy (disp->message, "Character Has New Mask");
	handle_message (disp, TRUE);
	return TRUE;
      }
  }
}

#define BITMAP_WIDTH 32
#define BITMAP_HEIGHT 32
#define BITMAP_ARRAY_SIZE 128

/* This procedure was given to me by Trevor Pering, from the Griljor
   Object Editor */

TransferPixmapToData(pixmap,array,disp)
     Pixmap pixmap;
     char array[];
     disp_fields disp;
{
  char *data;
  char temp[L_tmpnam];
  int width,height,x,y,i;
  
  tmpnam(temp);
  
  
  XFillRectangle(disp->display,pixmap,disp->inversion_gc,0,0,BITMAP_WIDTH,BITMAP_HEIGHT);
  XWriteBitmapFile(disp->display,temp,pixmap,BITMAP_WIDTH,BITMAP_HEIGHT,-1,-1);
  XFillRectangle(disp->display,pixmap,disp->inversion_gc,0,0,BITMAP_WIDTH,BITMAP_HEIGHT);
  
  XXReadBitmapFile(temp, &width, &height, &data, &x, &y);
  
  if (data != NULL)
    {
      for (i = 0;i < BITMAP_ARRAY_SIZE;i++)
        array[i] = data[i];
      free(data);
    }
  unlink(temp);
}
/* I also got this from Trevor Pering */

edit_pixmap(disp, pixmap)
     disp_fields disp;
     Pixmap *pixmap;
{
  unsigned int width,height;
  int x,y;
  char temp[L_tmpnam],out[L_tmpnam+10];
  Pixmap new;
  
  XGCValues	values;
  unsigned long value_mask;
  GC xorGC;
  
  values.foreground = disp->fgcolor;
  values.background = disp->bgcolor; 
  values.function = GXxor;
  
  xorGC = XCreateGC (disp->display, disp->win,
		     (GCForeground | GCBackground | GCFunction),
		     &values);
  tmpnam(temp);
  
  if (*pixmap)
    {
      if (disp->fgcolor == 0)
	XFillRectangle(disp->display,*pixmap,xorGC,0,0,BITMAP_WIDTH,BITMAP_HEIGHT);
      XWriteBitmapFile(disp->display,temp,*pixmap,BITMAP_WIDTH,BITMAP_HEIGHT,-1,-1);
      if (disp->fgcolor == 0)
	XFillRectangle(disp->display,*pixmap,xorGC,0,0,BITMAP_WIDTH,BITMAP_HEIGHT);
    }
  
  sprintf(out,"bitmap %s %dx%d",temp,BITMAP_WIDTH,BITMAP_HEIGHT);
  system(out);
  
  XReadBitmapFile(disp->display,RootWindow(disp->display,disp->screen),
		  temp,&width,&height,&new,&x,&y);
  unlink(temp);
  
  if ((width != BITMAP_WIDTH)||(height != BITMAP_HEIGHT))
    {
      strcpy (disp->message, "Wrong Size Bitmap!");
      handle_message (disp, TRUE);
      return(FALSE);
    }
  
  if (disp->fgcolor == 0)
    XFillRectangle(disp->display,new,xorGC,0,0,BITMAP_WIDTH,BITMAP_HEIGHT);
  
  
  if (*pixmap)
    XFreePixmap(disp->display,*pixmap);
  *pixmap = new;
  
  XFreeGC (disp->display, xorGC);
  
  return(True);
}


E 1
