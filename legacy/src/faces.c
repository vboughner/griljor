#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include "credits.h"
#include "free.h"
#include "faces.h"

Credits *CreateList(display, window, fg, bg, depth)
     Display *display;
     Window window;
     int fg, bg, depth;
{
  Credits *names;
  Credits *start;

  start = names = (Credits *)mymalloc(sizeof(Credits));
  names->name = van_name;
  names->contrib1 = van_con1;
  names->contrib2 = van_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      vanbit_bits,
					      vanbit_width,
					      vanbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      vanmask_bits,
					      vanmask_width,
					      vanmask_height,
					      fg, bg, depth);  
  names->width = vanbit_width;
  names->height = vanbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = mel_name;
  names->contrib1 = mel_con1;
  names->contrib2 = mel_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      melbit_bits,
					      melbit_width,
					      melbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      melmask_bits,
					      melmask_width,
					      melmask_height,
					      fg, bg, depth);  
  names->width = melbit_width;
  names->height = melbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = albert_name;
  names->contrib1 = albert_con1;
  names->contrib2 = albert_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      albertbit_bits,
					      albertbit_width,
					      albertbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      albertmask_bits,
					      albertmask_width,
					      albertmask_height,
					      fg, bg, depth);  
  names->width = albertbit_width;
  names->height = albertbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = trevor_name;
  names->contrib1 = trevor_con1;
  names->contrib2 = trevor_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      trevorbit_bits,
					      trevorbit_width,
					      trevorbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      trevormask_bits,
					      trevormask_width,
					      trevormask_height,
					      fg, bg, depth);  
  names->width = trevorbit_width;
  names->height = trevorbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = eric_name;
  names->contrib1 = eric_con1;
  names->contrib2 = eric_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      ericbit_bits,
					      ericbit_width,
					      ericbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      ericmask_bits,
					      ericmask_width,
					      ericmask_height,
					      fg, bg, depth);  
  names->width = ericbit_width;
  names->height = ericbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = spook_name;
  names->contrib1 = spook_con1;
  names->contrib2 = spook_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      spookbit_bits,
					      spookbit_width,
					      spookbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      spookmask_bits,
					      spookmask_width,
					      spookmask_height,
					      fg, bg, depth);  
  names->width = spookbit_width;
  names->height = spookbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = stefan_name;
  names->contrib1 = stefan_con1;
  names->contrib2 = stefan_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      stefanbit_bits,
					      stefanbit_width,
					      stefanbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      stefanmask_bits,
					      stefanmask_width,
					      stefanmask_height,
					      fg, bg, depth);  
  names->width = stefanbit_width;
  names->height = stefanbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = bh_name;
  names->contrib1 = bh_con1;
  names->contrib2 = bh_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      bhbit_bits,
					      bhbit_width,
					      bhbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      bhmask_bits,
					      bhmask_width,
					      bhmask_height,
					      fg, bg, depth);  
  names->width = bhbit_width;
  names->height = bhbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = gm_name;
  names->contrib1 = gm_con1;
  names->contrib2 = gm_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      gmbit_bits,
					      gmbit_width,
					      gmbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      gmmask_bits,
					      gmmask_width,
					      gmmask_height,
					      fg, bg, depth);  
  names->width = gmbit_width;
  names->height = gmbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = moronus_name;
  names->contrib1 = moronus_con1;
  names->contrib2 = moronus_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      moronusbit_bits,
					      moronusbit_width,
					      moronusbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      moronusmask_bits,
					      moronusmask_width,
					      moronusmask_height,
					      fg, bg, depth);  
  names->width = moronusbit_width;
  names->height = moronusbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = ollie_name;
  names->contrib1 = ollie_con1;
  names->contrib2 = ollie_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      olliebit_bits,
					      olliebit_width,
					      olliebit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      olliemask_bits,
					      olliemask_width,
					      olliemask_height,
					      fg, bg, depth);  
  names->width = olliebit_width;
  names->height = olliebit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = savaki_name;
  names->contrib1 = savaki_con1;
  names->contrib2 = savaki_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      savakibit_bits,
					      savakibit_width,
					      savakibit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      savakimask_bits,
					      savakimask_width,
					      savakimask_height,
					      fg, bg, depth);  
  names->width = savakibit_width;
  names->height = savakibit_height;

  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;
  names->name = stinglai_name;
  names->contrib1 = stinglai_con1;
  names->contrib2 = stinglai_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      stinglaibit_bits,
					      stinglaibit_width,
					      stinglaibit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      stinglaimask_bits,
					      stinglaimask_width,
					      stinglaimask_height,
					      fg, bg, depth);  
  names->width = stinglaibit_width;
  names->height = stinglaibit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = aaron_name;
  names->contrib1 = aaron_con1;
  names->contrib2 = aaron_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      aaronbit_bits,
					      aaronbit_width,
					      aaronbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      aaronmask_bits,
					      aaronmask_width,
					      aaronmask_height,
					      fg, bg, depth);  
  names->width = aaronbit_width;
  names->height = aaronbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = aragorn_name;
  names->contrib1 = aragorn_con1;
  names->contrib2 = aragorn_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      aragornbit_bits,
					      aragornbit_width,
					      aragornbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      aragornmask_bits,
					      aragornmask_width,
					      aragornmask_height,
					      fg, bg, depth);  
  names->width = aragornbit_width;
  names->height = aragornbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = crescendo_name;
  names->contrib1 = crescendo_con1;
  names->contrib2 = crescendo_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      crescendobit_bits,
					      crescendobit_width,
					      crescendobit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      crescendomask_bits,
					      crescendomask_width,
					      crescendomask_height,
					      fg, bg, depth);  
  names->width = crescendobit_width;
  names->height = crescendobit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = mahatma_name;
  names->contrib1 = mahatma_con1;
  names->contrib2 = mahatma_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      mahatmabit_bits,
					      mahatmabit_width,
					      mahatmabit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      mahatmamask_bits,
					      mahatmamask_width,
					      mahatmamask_height,
					      fg, bg, depth);  
  names->width = mahatmabit_width;
  names->height = mahatmabit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = crom_name;
  names->contrib1 = crom_con1;
  names->contrib2 = crom_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      crombit_bits,
					      crombit_width,
					      crombit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      crommask_bits,
					      crommask_width,
					      crommask_height,
					      fg, bg, depth);  
  names->width = crombit_width;
  names->height = crombit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = drustan_name;
  names->contrib1 = drustan_con1;
  names->contrib2 = drustan_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      drustanbit_bits,
					      drustanbit_width,
					      drustanbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      drustanmask_bits,
					      drustanmask_width,
					      drustanmask_height,
					      fg, bg, depth);  
  names->width = drustanbit_width;
  names->height = drustanbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = duel_name;
  names->contrib1 = duel_con1;
  names->contrib2 = duel_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      duelbit_bits,
					      duelbit_width,
					      duelbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      duelmask_bits,
					      duelmask_width,
					      duelmask_height,
					      fg, bg, depth);  
  names->width = duelbit_width;
  names->height = duelbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = mcelhoe_name;
  names->contrib1 = mcelhoe_con1;
  names->contrib2 = mcelhoe_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      mcelhoebit_bits,
					      mcelhoebit_width,
					      mcelhoebit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      mcelhoemask_bits,
					      mcelhoemask_width,
					      mcelhoemask_height,
					      fg, bg, depth);  
  names->width = mcelhoebit_width;
  names->height = mcelhoebit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = mike_name;
  names->contrib1 = mike_con1;
  names->contrib2 = mike_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      mikebit_bits,
					      mikebit_width,
					      mikebit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      mikemask_bits,
					      mikemask_width,
					      mikemask_height,
					      fg, bg, depth);  
  names->width = mikebit_width;
  names->height = mikebit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = adriana_name;
  names->contrib1 = adriana_con1;
  names->contrib2 = adriana_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      adrianabit_bits,
					      adrianabit_width,
					      adrianabit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      adrianamask_bits,
					      adrianamask_width,
					      adrianamask_height,
					      fg, bg, depth);  
  names->width = adrianabit_width;
  names->height = adrianabit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = avatar_name;
  names->contrib1 = avatar_con1;
  names->contrib2 = avatar_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      avatarbit_bits,
					      avatarbit_width,
					      avatarbit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      avatarmask_bits,
					      avatarmask_width,
					      avatarmask_height,
					      fg, bg, depth);  
  names->width = avatarbit_width;
  names->height = avatarbit_height;
  names->next = (Credits *)mymalloc(sizeof(Credits));
  names = names->next;

  names->name = mikey_name;
  names->contrib1 = mikey_con1;
  names->contrib2 = mikey_con2;
  names->face = MyXCreatePixmapFromBitmapData(display, window,
					      mikeybit_bits,
					      mikeybit_width,
					      mikeybit_height,
					      fg, bg, depth);  
  names->mask = MyXCreatePixmapFromBitmapData(display, window,
					      mikeymask_bits,
					      mikeymask_width,
					      mikeymask_height,
					      fg, bg, depth);  
  names->width = mikeybit_width;
  names->height = mikeybit_height;
  names->next = NULL;

  return start;
}



Credits *getElement(nameptr, num)
     Credits **nameptr;
     int num;
{
  Credits *thisOne;
  Credits *lastOne;

  if(num == 0) {
    thisOne = *nameptr;
    *nameptr = (*nameptr)->next;
    return thisOne;
  }
  else if(!((*nameptr)->next)) {
    thisOne = *nameptr;
    *nameptr = NULL;
    return thisOne;
  }
  else {
    lastOne = *nameptr;
    thisOne = (*nameptr)->next;
    while(--num) {
      lastOne = lastOne->next;
      thisOne = thisOne->next;
    }
    lastOne->next = thisOne->next;
    thisOne->next = NULL;
    return thisOne;
  }
}
    
      

void randomOrder(names, g1, g2, g3)
     Credits **names;
     int g1, g2, g3;
{
  int i;
  Credits **gp1;
  Credits **gp2;
  Credits **gp3;

  gp1 = (Credits **)malloc(g1*sizeof(Credits *));
  gp2 = (Credits **)malloc(g2*sizeof(Credits *));
  gp3 = (Credits **)malloc(g3*sizeof(Credits *));

  if(!gp1 || !gp2 || !gp3) {
    printf("Malloc failed in randomOrder\n");
    exit(1);
  }

  for(i=0; i<g1; i++) {
    *(gp1 + i) = getElement(names, randNum(g1-i));
  }
  for(i=0; i<g2; i++) {
    *(gp2 + i) = getElement(names, randNum(g2-i));
  }
  for(i=0; i<g3; i++) {
    *(gp3 + i) = getElement(names, randNum(g3-i));
  }


  for(i=0; i<g1; i++) {
    if(i+1 < g1)
      (*(gp1 + i))->next = *(gp1 + i + 1);
    else
      (*(gp1 + i))->next = *gp2;
  }
  for(i=0; i<g2; i++) {
    if(i+1 < g2)
      (*(gp2 + i))->next = *(gp2 + i + 1);
    else
      (*(gp2 + i))->next = *gp3;
  }
  for(i=0; i<g3; i++) {
    if(i+1 < g3)
      (*(gp3 + i))->next = *(gp3 + i + 1);
    else
      (*(gp3 + i))->next = NULL;
  }
  *names = *gp1;

  free(gp1);
  free(gp2);
  free(gp3);
}
  


Credits *setUpCredits(display, window, fg, bg, depth)
     Display *display;
     Window window;
     int fg, bg, depth;
{
  int i;
  Credits *names;

  names = CreateList(display, window, fg, bg, depth);
  randomOrder(&names, GROUP1_SIZE, GROUP2_SIZE, GROUP3_SIZE);

  return names;
}
