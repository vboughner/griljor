# **************************************************************************
# * The War of Griljor
# *
# * By  Van A. Boughner
# *     Mel Nicholson
# * And Albert C. Baker III
# *
# * Students of the University of California at Berkeley
# * October 1989
# **************************************************************************
#
# Makefile

CC = nice cc
CFLAGS = -w -g -DUSEPIXMAP -I/usr/X11/include -I.
LIBDIR =
MODE_SET = 4711
USLEEP = usleep.o

# Header files commonly in use.
# include the file hdrs after 'make hdrs' here:
HDRS    = \
	circle.h config.h connect.h def.h \
	dialog.h editobj.h event.h extern.h \
	externX11.h gamelist.h gametime.h hit.h \
	images.h language.h level.h lib.h \
	magic.h map.h menusX11.h missile.h \
	movement.h objects.h password.h person.h \
	playerX11.h plot_order.h putstuff.h queued.h \
	ray.h rulepack.h socket.h \
	windowsX11.h objinfo.h \
	volvox.h vsys.h vline.h

# Vline library sources
VHDRS = volvox.h vsys.h vline.h
VSRCS = vsys.c vline.c
VOBJS = vsys.o vline.o

# Source files commonly used by player and driver
SRCS    = help.c error.c interpret.c gamelist.c map.c bcopy.c \
	  objects.c lib.c socket.c person.c X11RBitmap.c objprops.c \
	  connect.c gametime.c ident.c flag.c appear.c objstore.c \
	  objheader.c objinfo.c notify.c message.c notice.c
OBJS    = help.o error.o interpret.o gamelist.o map.o bcopy.o \
	  objects.o lib.o socket.o person.o X11RBitmap.o objprops.o \
	  connect.o gametime.o ident.o flag.o appear.o objstore.o \
	  objheader.o objinfo.o notify.o message.o notice.o

# Source files for title screen (in player program)
TITLE_HDRS = animate.h credits.h faces.h free.h
TITLE_SRCS = animate.c credits.c faces.c free.c readbit.c scene.c title.c
TITLE_OBJS = animate.o credits.o faces.o free.o readbit.o scene.o title.o

# Unused ray code for player program
RAYSRCS = ray.c ray2.c rulepack.c
RAYOBJS = ray.o ray2.o rulepack.o

# Player movement sources (plus a whole lot more, all unique to player)
MOVE_SRCS = movement.c inventory.c handle.c missile.c play.c sight.c \
	    hit.c magic.c level.c multifire.c arc.c		     \
	    playerX11.c pickX11.c images.c mask.c bitok.c event.c \
            roomdraw.c player.c queued.c player_dud.c
MOVE_OBJS = movement.o inventory.o handle.o missile.o play.o sight.o \
	    hit.o magic.o level.o multifire.o arc.o		     \
	    playerX11.o pickX11.o images.o mask.o bitok.o event.o \
            roomdraw.o player.o queued.o player_dud.o

# Sources for window routines used by player and editor programs
WINDOW_SRCS = windowsX11.c inputX11.c outputX11.c menusX11.c
WINDOW_OBJS = windowsX11.o inputX11.o outputX11.o menusX11.o
WINDOW_LIBS = $(LIBDIR) -lX11

# Player program
PLAY     = griljor
PLAYSRCS = $(TITLE_SRCS) $(SRCS) $(MOVE_SRCS) $(WINDOW_SRCS) $(VSRCS)
PLAYOBJS = $(TITLE_OBJS) $(OBJS) $(MOVE_OBJS) $(WINDOW_OBJS) $(VOBJS) \
           ${USLEEP}

# Source files specifically for driver program
SERV_SRCS = driver.c accept.c password.c timecheck.c startfile.c \
            putstuff.c driver_dud.c
SERV_OBJS = driver.o accept.o password.o timecheck.o startfile.o \
            putstuff.o driver_dud.o

# Burt artificial intelligence code
BURT = burt
BURT_HDRS = burt/burt.h burt/extern.h
BURT_SRCS = burt/io.c burt/load.c burt/main.c burt/person.c burt/response.c
BURT_OBJS = burt/io.o burt/load.o burt/main.o burt/person.o burt/response.o

# Driver program
DRIV	 = grildriver
DRIVSRCS = $(SRCS) $(SERV_SRCS) $(BURT_SRCS) $(VSRCS)
DRIVOBJS = $(OBJS) $(SERV_OBJS) $(BURT_OBJS) $(VOBJS)

# Sources for the map editing program
EDITMAP = editmap
EDIT_SRCS = editmap.c editorX11.c editor_dud.c \
            ranroom.c ranstuff.c mapdiagX11.c
EDIT_OBJS = editmap.o editorX11.o editor_dud.o \
            ranroom.o ranstuff.o mapdiagX11.o
EDIT_SHARE_SRCS = $(WINDOW_SRCS) $(VSRCS) objects.c error.c map.c lib.c \
	    objstore.c objheader.c objprops.c bcopy.c dialog.c objinfo.c \
	    X11RBitmap.c
EDIT_SHARE_OBJS = $(WINDOW_OBJS) $(VOBJS) objects.o error.o map.o lib.o \
	    objstore.o objheader.o objprops.o bcopy.o dialog.o objinfo.o \
	    X11RBitmap.o

# Sources for object editor
OBTOR = obtor
OBT_HDRS = obtor.h dialog.h editobj.h help.h objects.h
OBT_SRCS = arrow.c dragobj.c infodiag.c obtmask.c wininfo.c bitmap.c \
           drawgrab.c loadbit.c obtor_dud.c winspace.c cutwin.c editobj.c \
           massinfo.c pixmapedit.c dialog.c globals.c moveobj.c trywin.c \
           obtorX11.c adjobj.c
OBT_OBJS = arrow.o dragobj.o infodiag.o obtmask.o wininfo.o bitmap.o \
           drawgrab.o loadbit.o obtor_dud.o winspace.o cutwin.o editobj.o \
           massinfo.o pixmapedit.o dialog.o globals.o moveobj.o trywin.o \
           obtorX11.o adjobj.o
OBT_SHARE_SRCS = ${VSRCS} ${WINDOW_SRCS} lib.c map.c error.c objects.c \
           X11RBitmap.c objstore.c objheader.c objprops.c bcopy.c objinfo.c
OBT_SHARE_OBJS = ${VOBJS} ${WINDOW_OBJS} lib.o map.o error.o objects.o \
           X11RBitmap.o objstore.o objheader.o objprops.o bcopy.o objinfo.o

# Player password file editor sources
EDITPASS = editpass
PASS_SRCS = errorx.c xx.c processorx.c menux.c attributex.c importx.c \
            scrollx.c sliderx.c bitmapx.c timex.c X11WBitmap.c \
            editpass.c pass_dud.c bcopy.c
PASS_OBJS = errorx.o xx.o processorx.o menux.o attributex.o importx.o \
            scrollx.o sliderx.o bitmapx.o timex.o X11WBitmap.o \
            editpass.o pass_dud.o bcopy.o
PASS_SHARE_SRCS = error.c lib.c password.c X11RBitmap.c extern.c
PASS_SHARE_OBJS = error.o lib.o password.o X11RBitmap.c extern.o


all: 	${PLAY} ${DRIV} ${OBTOR} ${EDITMAP} ${EDITPASS}

two:	${PLAY} ${DRIV}

${PLAY}:  ${PLAYOBJS}
	${CC}  ${CFLAGS} ${PLAYOBJS} -o ${PLAY} ${WINDOW_LIBS} \
	-lm
	@ chmod 711 ${PLAY}

${DRIV}:  ${DRIVOBJS}
	${CC}  ${CFLAGS} ${DRIVOBJS} -o ${DRIV} \
	-lm
	@ chmod ${MODE_SET} ${DRIV}

${EDITMAP}:  ${EDIT_OBJS} ${EDIT_SHARE_OBJS}
	${CC}  ${CFLAGS} ${EDIT_OBJS} ${EDIT_SHARE_OBJS} \
	-o ${EDITMAP} ${WINDOW_LIBS} -lm
	@ chmod 711 ${EDITMAP}

${OBTOR}:  ${OBT_OBJS} ${OBT_SHARE_OBJS}
	${CC}  ${CFLAGS} ${OBT_OBJS} ${OBT_SHARE_OBJS} \
	-o ${OBTOR} ${WINDOW_LIBS} -lm
	@ chmod 711 ${OBTOR}

${EDITPASS}: ${PASS_SHARE_OBJS} ${PASS_OBJS}
	${CC} ${CFLAGS} ${PASS_SHARE_OBJS} ${PASS_OBJS} \
	-o ${EDITPASS} ${WINDOW_LIBS} -lm
	@ chmod 711 ${EDITPASS}

# make burt files
${BURT_OBJS}:	burt/burt.h
	cd burt; make burt; cd ..


# Make a super tag file that refers to all procedures anywhere
tags:   ${SRCS} ${HDRS} ${VHDRS} ${VSRCS} ${MOVE_SRCS} ${TITLE_SRCS} \
	${SERV_SRCS} ${WINDOW_SRCS} ${BURT_SRCS} \
	${EDIT_SRCS} ${OBT_SRCS} ${PASS_SRCS}
	ctags ${SRCS} ${HDRS} ${VHDRS} ${VSRCS} ${MOVE_SRCS} ${TITLE_SRCS} \
	${SERV_SRCS} ${WINDOW_SRCS} ${BURT_SRCS} \
	${EDIT_SRCS} ${OBT_SRCS} ${PASS_SRCS}


# search the SCCS directory for the headers in the file, include in the
# make file up above, under $HDRS
hdrs:   
	@echo Searching for headers...
	@ls SCCS/s.*.h | paste -s -d"   \n" - |  \
	sed -e s=SCCS/s.==g -e s/$$/' \\'/g | tee hdrs


# figure out dependencies
depend:	${SRCS} ${HDRS} ${VHDRS} ${VSRCS} ${MOVE_SRCS} ${TITLE_SRCS} \
	${SERV_SRCS} ${WINDOW_SRCS} ${BURT_SRCS} \
	${EDIT_SRCS} ${OBT_SRCS} ${PASS_SRCS}
	gcc -MM ${SRCS} ${HDRS} ${VHDRS} ${VSRCS} ${MOVE_SRCS} ${TITLE_SRCS} \
	${SERV_SRCS} ${WINDOW_SRCS} ${BURT_SRCS} \
	${EDIT_SRCS} ${OBT_SRCS} ${PASS_SRCS} | tee depend


# Clean up just a few things, leaving the object files behind
clean:
	rm -f ${PLAY} ${DRIV} ${OBTOR} ${EDITMAP} ${EDITPASS} core


# Individual file dependencies (place "make depend" results below this line)
help.o : help.c config.h def.h extern.h 
error.o : error.c config.h def.h extern.h 
interpret.o : interpret.c config.h def.h extern.h 
gamelist.o : gamelist.c config.h def.h extern.h map.h missile.h socket.h \
  gamelist.h lib.h 
map.o : map.c config.h def.h map.h objects.h 
bcopy.o : bcopy.c 
objects.o : objects.c config.h def.h objects.h lib.h objinfo.h 
lib.o : lib.c config.h def.h lib.h 
socket.o : socket.c config.h def.h extern.h objects.h map.h missile.h \
  socket.h person.h gametime.h level.h password.h 
person.o : person.c config.h def.h extern.h objects.h map.h missile.h \
  socket.h person.h level.h 
X11RBitmap.o : X11RBitmap.c 
objprops.o : objprops.c def.h config.h objects.h map.h dialog.h objinfo.h 
connect.o : connect.c config.h connect.h 
gametime.o : gametime.c config.h def.h 
ident.o : ident.c config.h def.h map.h objects.h missile.h socket.h \
  person.h 
flag.o : flag.c config.h def.h extern.h objects.h map.h missile.h socket.h \
  person.h lib.h 
appear.o : appear.c config.h def.h extern.h objects.h map.h 
objstore.o : objstore.c def.h config.h objects.h dialog.h vline.h vsys.h \
  volvox.h objheader.h objinfo.h 
objheader.o : objheader.c def.h config.h objects.h dialog.h vline.h vsys.h \
  volvox.h 
objinfo.o : objinfo.c vline.h vsys.h volvox.h objinfo.h objheader.h 
notify.o : notify.c config.h def.h extern.h objects.h map.h missile.h \
  socket.h person.h gametime.h level.h password.h 
message.o : message.c vline.h vsys.h volvox.h message.h 
notice.o : notice.c config.h def.h extern.h objects.h map.h missile.h \
  socket.h person.h gametime.h level.h password.h 
player.o : player.c config.h def.h extern.c lib.h map.h objects.h missile.h \
  socket.h person.h gamelist.h gametime.h
queued.o : queued.c config.h def.h queued.h windowsX11.h objects.h map.h \
  lib.h bit/mouse missile.h roomdraw.h 
animate.o : animate.c credits.h animate.h free.h bitmaps/tombbit \
  bitmaps/tombmask bitmaps/skullbit bitmaps/skullmask 
credits.o : credits.c free.h credits.h 
faces.o : faces.c credits.h free.h faces.h facebits/vanbit facebits/vanmask \
  facebits/melbit facebits/melmask facebits/albertbit facebits/albertmask \
  facebits/trevorbit facebits/trevormask facebits/ericbit facebits/ericmask \
  facebits/spookbit facebits/spookmask facebits/stefanbit facebits/stefanmask \
  facebits/bhbit facebits/bhmask facebits/gmbit facebits/gmmask \
  facebits/moronusbit facebits/moronusmask facebits/olliebit \
  facebits/olliemask facebits/savakibit facebits/savakimask \
  facebits/stinglaibit facebits/stinglaimask facebits/aaronbit \
  facebits/aaronmask facebits/aragornbit facebits/aragornmask \
  facebits/crescendobit facebits/crescendomask facebits/mahatmabit \
  facebits/mahatmamask facebits/crombit facebits/crommask facebits/drustanbit \
  facebits/drustanmask facebits/duelbit facebits/duelmask facebits/mcelhoebit \
  facebits/mcelhoemask facebits/mikebit facebits/mikemask facebits/mikeybit \
  facebits/mikeymask facebits/adrianabit facebits/adrianamask \
  facebits/avatarbit facebits/avatarmask 
free.o : free.c 
readbit.o : readbit.c free.h bitmaps/g bitmaps/r1 bitmaps/i bitmaps/l \
  bitmaps/j bitmaps/o1 bitmaps/o2 bitmaps/o3 bitmaps/o4 bitmaps/o5 bitmaps/o6 \
  bitmaps/o7 bitmaps/o8 bitmaps/r2 bitmaps/beach0 bitmaps/beach1 \
  bitmaps/beach2 bitmaps/forest bitmaps/sand bitmaps/llpath bitmaps/lrpath \
  bitmaps/ulpath bitmaps/urpath bitmaps/vpath bitmaps/hpath bitmaps/river0 \
  bitmaps/river1 bitmaps/river2 bitmaps/river3 bitmaps/river4 bitmaps/river5 \
  bitmaps/river6 bitmaps/river7 bitmaps/river8 bitmaps/river9 bitmaps/river10 \
  bitmaps/river11 bitmaps/river12 bitmaps/river13 bitmaps/river14 \
  bitmaps/river15 bitmaps/river16 bitmaps/river17 bitmaps/river18 \
  bitmaps/river20 bitmaps/mask0 bitmaps/mask1 bitmaps/mask2 bitmaps/mask3 \
  bitmaps/mask4 bitmaps/mask5 bitmaps/mask6 bitmaps/mask7 bitmaps/mask8 \
  bitmaps/mask9 bitmaps/mask10 bitmaps/mask11 bitmaps/mask12 bitmaps/mask13 \
  bitmaps/mask14 bitmaps/mask15 bitmaps/mask20 
scene.o : scene.c 
title.o : title.c bitmaps/spookbit bitmaps/spookmask free.h credits.h \
  animate.h 
driver.o : driver.c config.h def.h extern.c objects.h map.h missile.h \
  socket.h gamelist.h person.h lib.h gametime.h putstuff.h 
accept.o : accept.c config.h def.h objects.h map.h missile.h socket.h \
  person.h password.h level.h 
password.o : password.c config.h def.h lib.h password.h 
timecheck.o : timecheck.c config.h 
startfile.o : startfile.c config.h lib.h 
putstuff.o : putstuff.c config.h def.h lib.h putstuff.h 
mstr_main.o : mstr_main.c config.h def.h extern.c map.h objects.h missile.h \
  socket.h person.h 
language.o : language.c config.h def.h extern.h lib.h map.h objects.h \
  missile.h socket.h person.h gamelist.h gametime.h 
monster.o : monster.c bit/gm bit/gmask bit/dodger 
queued.o : queued.c config.h def.h queued.h windowsX11.h objects.h map.h \
  lib.h bit/mouse missile.h roomdraw.h 
windowsX11.o : windowsX11.c def.h config.h windowsX11.h objects.h map.h \
  lib.h bit/mouse bit/arrow bit/amask 
inputX11.o : inputX11.c def.h config.h windowsX11.h objects.h map.h lib.h \
  bit/mouse 
outputX11.o : outputX11.c def.h config.h windowsX11.h objects.h map.h lib.h \
  bit/mouse plot_order.h 
menusX11.o : menusX11.c windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse menusX11.h 
editmap.o : editmap.c config.h def.h extern.c objects.h map.h externX11.h 
objects.o : objects.c config.h def.h objects.h lib.h objinfo.h 
error.o : error.c config.h def.h extern.h 
map.o : map.c config.h def.h map.h objects.h 
lib.o : lib.c config.h def.h lib.h 
editorX11.o : editorX11.c windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse def.h bit/link bit/editmapicon bit/scrollup bit/scrolldown \
  bit/pageup bit/pagedown 
editor_dud.o : editor_dud.c 
X11RBitmap.o : X11RBitmap.c 
ranroom.o : ranroom.c /usr/include/stdio.h ranroom.h config.h map.h \
  objects.h /usr/include/math.h 
ranstuff.o : ranstuff.c /usr/include/stdio.h config.h map.h objects.h \
  /usr/include/math.h 
objstore.o : objstore.c def.h config.h objects.h dialog.h vline.h vsys.h \
  volvox.h objheader.h objinfo.h 
objheader.o : objheader.c def.h config.h objects.h dialog.h vline.h vsys.h \
  volvox.h 
objprops.o : objprops.c def.h config.h objects.h map.h dialog.h objinfo.h 
bcopy.o : bcopy.c 
mapdiagX11.o : mapdiagX11.c windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse def.h dialog.h 
dialog.o : dialog.c windowsX11.h config.h objects.h map.h lib.h bit/mouse \
  def.h dialog.h 
objinfo.o : objinfo.c vline.h vsys.h volvox.h objinfo.h objheader.h 
movement.o : movement.c config.h def.h extern.h gametime.h objects.h map.h \
  missile.h socket.h person.h movement.h level.h queued.h 
inventory.o : inventory.c config.h def.h extern.h objects.h map.h missile.h \
  socket.h person.h 
handle.o : handle.c config.h def.h extern.h map.h objects.h missile.h \
  socket.h person.h 
missile.o : missile.c config.h def.h extern.h gametime.h lib.h map.h \
  objects.h missile.h socket.h person.h movement.h queued.h message.h
play.o : play.c config.h def.h extern.h objects.h map.h missile.h socket.h \
  person.h level.h gamelist.h gametime.h lib.h message.h
sight.o : sight.c config.h def.h extern.h objects.h map.h missile.h \
  socket.h person.h movement.h 
hit.o : hit.c config.h def.h extern.h gametime.h queued.h hit.h 
magic.o : magic.c config.h def.h extern.h objects.h map.h missile.h \
  socket.h person.h gametime.h lib.h level.h magic.h 
level.o : level.c config.h def.h extern.h lib.h map.h objects.h missile.h \
  socket.h person.h gametime.h level.h /usr/include/math.h 
multifire.o : multifire.c config.h def.h extern.h objects.h map.h missile.h \
  socket.h person.h gametime.h movement.h 
arc.o : arc.c config.h def.h 
playerX11.o : playerX11.c windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse playerX11.h def.h extern.h missile.h socket.h person.h gametime.h \
  gamelist.h movement.h images.h level.h queued.h circle.h roomdraw.h \
  bit/target bit/tmask bit/circle bit/cmask bit/pen bit/pmask bit/griljoricon \
  bit/movemark bit/hit bit/hmask bit/friend bit/fmask bit/box bit/bmask \
  bit/logo bit/mainback bit/quit bit/qmask bit/stone bit/smask bit/invis \
  bit/imask bit/loser bit/lmask bit/winner bit/wmask message.h
pickX11.o : pickX11.c windowsX11.h config.h objects.h map.h lib.h bit/mouse \
  playerX11.h menusX11.h def.h extern.h missile.h socket.h person.h \
  gamelist.h bit/logo bit/border 
images.o : images.c lib.h 
mask.o : mask.c config.h def.h lib.h 
bitok.o : bitok.c config.h def.h 
event.o : event.c config.h def.h extern.h objects.h map.h missile.h \
  socket.h person.h level.h gametime.h event.h 
roomdraw.o : roomdraw.c windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse playerX11.h def.h extern.h missile.h socket.h person.h gametime.h \
  gamelist.h movement.h images.h level.h queued.h roomdraw.h 
arrow.o : arrow.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
dragobj.o : dragobj.c  obtor.h windowsX11.h \
  config.h objects.h map.h lib.h bit/mouse def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
infodiag.o : infodiag.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
obtmask.o : obtmask.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
wininfo.o : wininfo.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
bitmap.o : bitmap.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h bit/backg.b bit/border.b bit/up.b \
  bit/down.b bit/icon.b bit/nothing.b bit/intro1.b bit/intro2.b bit/intro3.b \
  bit/foo.b bit/info.b 
drawgrab.o : drawgrab.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
loadbit.o : loadbit.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
obtor_dud.o : obtor_dud.c 
winspace.o : winspace.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
cutwin.o : cutwin.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
editobj.o : editobj.c externX11.h obtor.h windowsX11.h config.h objects.h \
  map.h lib.h bit/mouse  def.h vline.h vsys.h \
  volvox.h editobj.h dialog.h objheader.h objinfo.h extern.c 
massinfo.o : massinfo.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
pixmapedit.o : pixmapedit.c obtor.h windowsX11.h config.h objects.h map.h \
  lib.h bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
dialog.o : dialog.c windowsX11.h config.h objects.h map.h lib.h bit/mouse \
  def.h dialog.h 
globals.o : globals.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
moveobj.o : moveobj.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
trywin.o : trywin.c  obtor.h windowsX11.h \
  config.h objects.h map.h lib.h bit/mouse def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h 
obtorX11.o : obtorX11.c obtor.h windowsX11.h config.h objects.h map.h lib.h \
  bit/mouse  def.h vline.h vsys.h volvox.h \
  editobj.h dialog.h objheader.h objinfo.h help.h 
adjobj.o : adjobj.c /usr/include/stdio.h obtor.h windowsX11.h config.h \
  objects.h map.h lib.h bit/mouse  def.h vline.h \
  vsys.h volvox.h editobj.h dialog.h objheader.h objinfo.h 
errorx.o : errorx.c standardx.h 
xx.o : xx.c standardx.h lib.h def.h config.h password.h xx.h 
processorx.o : processorx.c lib.h def.h config.h password.h standardx.h \
  processorx.h xx.h 
menux.o : menux.c standardx.h lib.h def.h config.h password.h xx.h menux.h 
attributex.o : attributex.c standardx.h lib.h def.h config.h password.h \
  xx.h attributex.h bitmapx.h 
importx.o : importx.c importx.h 
scrollx.o : scrollx.c standardx.h lib.h def.h config.h password.h scrollx.h 
sliderx.o : sliderx.c standardx.h lib.h def.h config.h password.h sliderx.h 
bitmapx.o : bitmapx.c standardx.h lib.h def.h config.h password.h xx.h \
  bitmapx.h 
timex.o : timex.c 
X11WBitmap.o : X11WBitmap.c 
editpass.o : editpass.c config.h def.h password.h lib.h 
pass_dud.o : pass_dud.c 
bcopy.o : bcopy.c 
vsys.o : vsys.c vsys.h volvox.h 
vline.o : vline.c vline.h vsys.h volvox.h 
