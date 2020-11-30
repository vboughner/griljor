h03911
s 00027/00027/00166
d D 1.22 94/06/08 14:28:52 vbo 22 21
c changed paths
e
s 00028/00028/00165
d D 1.21 92/08/27 17:18:09 vbo 21 20
c renamed standard obj and map filenames - changed lib paths
e
s 00029/00029/00164
d D 1.20 92/08/07 20:52:09 vbo 20 19
c fixes for sun port merged in
e
s 00029/00029/00164
d D 1.19 92/02/20 22:06:17 vanb 19 18
c 
e
s 00017/00002/00176
d D 1.18 92/01/20 16:25:30 labc-4lc 18 17
c added references to bitmaps used in editpass
e
s 00001/00000/00177
d D 1.17 91/12/08 03:33:42 labc-4lc 17 16
c implemented usage messages for objects
e
s 00002/00001/00175
d D 1.16 91/12/07 17:37:12 labc-4lc 16 15
c made error message window bigger and scrollable
e
s 00004/00000/00172
d D 1.15 91/11/29 16:08:58 labc-4lc 15 14
c finished making entry of hidden text properties in obtor
e
s 00000/00000/00172
d D 1.14 91/10/16 20:06:57 labc-4lc 14 13
c changed made for DecStations
e
s 00021/00021/00151
d D 1.13 91/09/08 19:14:49 labc-4lc 13 12
c 
e
s 00009/00002/00163
d D 1.12 91/09/03 21:38:45 labb-3li 12 11
c Trevor's misc changes - fixed missile heap
e
s 00003/00003/00162
d D 1.11 91/08/30 17:12:40 labb-3li 11 10
c 
e
s 00001/00001/00164
d D 1.10 91/08/30 16:46:24 labb-3li 10 9
c 
e
s 00021/00021/00144
d D 1.9 91/08/30 16:34:47 labb-3li 9 8
c 
e
s 00003/00003/00162
d D 1.8 91/08/28 01:55:06 vanb 8 7
c 
e
s 00022/00022/00143
d D 1.7 91/08/26 00:33:01 vanb 7 6
c fixed up procedure defs and other compatibilty problems
e
s 00003/00003/00162
d D 1.6 91/08/03 16:27:53 labc-3id 6 5
c 
e
s 00001/00001/00164
d D 1.5 91/07/04 15:36:32 labc-3id 5 4
c changes made to help effect new object set
e
s 00001/00001/00164
d D 1.4 91/05/20 23:50:39 labc-3id 4 3
c changed default object set name
e
s 00001/00001/00164
d D 1.3 91/04/17 16:56:08 labc-3id 3 2
c 
e
s 00001/00001/00164
d D 1.2 91/04/14 23:07:06 labc-3id 2 1
c 
e
s 00165/00000/00000
d D 1.1 91/02/16 13:00:17 labc-3id 1 0
c date and time created 91/02/16 13:00:17 by labc-3id
e
u
U
f e 0
t
T
I 1
/***************************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * October 1989
 **************************************************************************/

/* Configuration definitions */

#define PROGRAM_NAME	"Griljor"
D 21
#define	PROGRAM_VERSION	"0.9"
E 21
I 21
#define	PROGRAM_VERSION	"2.0"
E 21
D 7
#define DRIVER_RUN_FILE	"/net/wolf/c60c-3/sp91/labc-3id/games/grildriver"
E 7
I 7
D 9
#define DRIVER_RUN_FILE	"/users/vanb/games/grildriver"
E 9
I 9
D 13
#define DRIVER_RUN_FILE	"/net/wolf/c60b-3/fa91/labb-3li/games/grildriver"
E 13
I 13
D 19
#define DRIVER_RUN_FILE	"/net/water/c60c-4/fa91/labc-4lc/games/grildriver"
E 19
I 19
D 20
#define DRIVER_RUN_FILE	"/users/vanb/games/grildriver"
E 20
I 20
D 22
#define DRIVER_RUN_FILE	"/home/cloud_91/vbo/games/grildriver"
E 22
I 22
#define DRIVER_RUN_FILE	"/net/rootbeer.Eng/export/home/vbo/games/grildriver"
E 22
E 20
E 19
E 13
E 9
E 7

I 15
/* obtor editor callup strings */
#define XTERM_COMMAND	"xterm -T 'Obtor Text Edit' -e %s"
#define DEFAULT_EDITOR	"emacs %s"

E 15
/* default filenames for maps and object definition files */
#define DEFAULT_MAP_PATH \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/map/default.map"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/map/default.map"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/map/default.map"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/map/default.map"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/map/default.map"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/map/default.map"
E 20
E 19
E 13
E 9
E 7
D 4
#define DEFAULT_OBJ_PATH "default.obj"
E 4
I 4
D 5
#define DEFAULT_OBJ_PATH "standard.obj"
E 5
I 5
#define DEFAULT_OBJ_PATH "main.obj"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/map/standard.map"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/map/standard.map"
E 22
#define DEFAULT_OBJ_PATH "standard.obj"
E 21
E 5
E 4
#define MAP_LIST_FILE	"maplist"

/* define the library directories for maps and objects and other things */
#define OBJ_LIB_DIR \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/obj"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/obj"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/obj"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/obj"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/obj"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/obj"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/obj"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/obj"
E 22
E 21
E 20
E 19
E 13
E 9
E 7
#define MAP_LIB_DIR \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/map"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/map"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/map"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/map"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/map"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/map"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/map"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/map"
E 22
E 21
E 20
E 19
E 13
E 9
E 7
#define HELP_LIB_DIR \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib"
E 22
E 21
E 20
E 19
E 13
E 9
E 7
#define MAINT_LIB_DIR \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib"
E 22
E 21
E 20
E 19
E 13
E 9
E 7
#define MONSTER_LIB_DIR \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib"
E 22
E 21
E 20
E 19
E 13
E 9
E 7


/* locations of help files for text in help windows */
#define EDITOR_HELP_FILE \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/editor.help"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/editor.help"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/editor.help"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/editor.help"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/editor.help"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/editor.help"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/editor.help"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/editor.help"
E 22
E 21
E 20
E 19
E 13
E 9
E 7
#define PLAYER_HELP_FILE \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/player.help"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/player.help"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/player.help"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/player.help"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/player.help"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/player.help"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/player.help"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/player.help"
E 22
E 21
E 20
E 19
E 13
E 9
E 7
#define GAME_HELP_FILE	 \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/game.help"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/game.help"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/game.help"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/game.help"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/game.help"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/game.help"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/game.help"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/game.help"
E 22
E 21
E 20
E 19
E 13
E 9
E 7
#define OBTOR_HELP_FILE  \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/obtor.help"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/obtor.help"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/obtor.help"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/obtor.help"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/obtor.help"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/obtor.help"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/obtor.help"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/obtor.help"
E 22
E 21
E 20
E 19
E 13
E 9
E 7
#define HOST_HELP_FILE	\
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/host.help"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/host.help"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/host.help"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/host.help"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/host.help"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/host.help"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/host.help"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/host.help"
E 22
E 21
E 20
E 19
E 13
E 9
E 7

/* location of current news file for game startup */
#define NEWS_FILE \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/news"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/news"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/news"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/news"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/news"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/news"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/news"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/news"
E 22
E 21
E 20
E 19
E 13
E 9
E 7

/* location of the time resitrictions file, if it exists */
#define TIME_FILE \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/times"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/times"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/times"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/times"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/times"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/times"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/times"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/times"
E 22
E 21
E 20
E 19
E 13
E 9
E 7

/* default map file for editor and driver */
#define DFLT_MAP_FILE	DEFAULT_MAP_PATH

/* default password file drivers will look into */
#define DFLT_PASS_FILE \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/players"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/players"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/players"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/players"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/players"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/players"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/players"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/players"
E 22
E 21
E 20
E 19
E 13
E 9
E 7

/* default game list file that players will search to find games to join */
#define DFLT_GAME_FILE \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/games"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/games"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/games"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/games"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/games"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/games"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/games"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/games"
E 22
E 21
E 20
E 19
E 13
E 9
E 7

/* default monster variable file, for specifying monster info */
#define DFLT_VAR_FILE \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/variables"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/variables"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/variables"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/variables"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/variables"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/variables"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/variables"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/variables"
E 22
E 21
E 20
E 19
E 13
E 9
E 7

/* default bitmap and mask that players will assume when joining game */
#define DFLT_BITMAP \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/man.bit"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/man.bit"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/man.bit"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/man.bit"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/man.bit"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/man.bit"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/man.bit"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/man.bit"
E 22
E 21
E 20
E 19
E 13
E 9
E 7
#define DFLT_MASK \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/man.mask"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/man.mask"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/man.mask"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/man.mask"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/man.mask"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/man.mask"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/man.mask"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/man.mask"
E 22
E 21
E 20
E 19
E 13
E 9
E 7

I 18
/* location of bitmaps used by the editpass program */
#define	SLIDER_FILE_NAME \
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/bit/slider.bit"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/bit/slider.bit"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/bit/slider.bit"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/bit/slider.bit"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/bit/slider.bit"
E 22
E 21
E 20
E 19
#define	ICON_FILE_NAME \
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/bit/icon.bit"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/bit/icon.bit"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/bit/icon.bit"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/bit/icon.bit"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/bit/icon.bit"
E 22
E 21
E 20
E 19
#define DISC_FILE_NAME \
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/bit/disc_icon.bit"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/bit/disc_icon.bit"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/bit/disc_icon.bit"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/bit/disc_icon.bit"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/bit/disc_icon.bit"
E 22
E 21
E 20
E 19
#define GREY_FILE_NAME \
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/bit/grey.bit"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/bit/grey.bit"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/bit/grey.bit"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/bit/grey.bit"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/bit/grey.bit"
E 22
E 21
E 20
E 19
#define PICT_FILE_NAME \
D 19
       "/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/bit/pict.bit"
E 19
I 19
D 20
       "/users/vanb/games/lib/griljor/bit/pict.bit"
E 20
I 20
D 21
       "/home/cloud_91/vbo/games/lib/griljor/bit/pict.bit"
E 21
I 21
D 22
       "/home/cloud_91/vbo/games/lib/griljor2.0/bit/pict.bit"
E 22
I 22
       "/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/bit/pict.bit"
E 22
E 21
E 20
E 19
#define TITLE_FILE_NAME \
D 19
       "/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/bit/title.bit"
E 19
I 19
D 20
       "/users/vanb/games/lib/griljor/bit/title.bit"
E 20
I 20
D 21
       "/home/cloud_91/vbo/games/lib/griljor/bit/title.bit"
E 21
I 21
D 22
       "/home/cloud_91/vbo/games/lib/griljor2.0/bit/title.bit"
E 22
I 22
       "/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/bit/title.bit"
E 22
E 21
E 20
E 19


E 18
/* active flag and location of logfile for messages to game master */
#define GM_NAME		"GM"	/* mail name for GM from inside game */
#define	GM_ACTIVE	1	/* is 1 if messages are logged */
#define GM_FILE \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/messages"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/messages"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/messages"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/messages"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/messages"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/messages"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/messages"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/messages"
E 22
E 21
E 20
E 19
E 13
E 9
E 7
#define	GM_PERSONALITY \
D 7
	"/net/wolf/c60c-3/sp91/labc-3id/games/lib/griljor/gm.text"
E 7
I 7
D 9
	"/users/vanb/games/lib/griljor/gm.text"
E 9
I 9
D 13
	"/net/wolf/c60b-3/fa91/labb-3li/games/lib/griljor/gm.text"
E 13
I 13
D 19
	"/net/water/c60c-4/fa91/labc-4lc/games/lib/griljor/gm.text"
E 19
I 19
D 20
	"/users/vanb/games/lib/griljor/gm.text"
E 20
I 20
D 21
	"/home/cloud_91/vbo/games/lib/griljor/gm.text"
E 21
I 21
D 22
	"/home/cloud_91/vbo/games/lib/griljor2.0/gm.text"
E 22
I 22
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/lib/gm.text"
E 22
E 21
E 20
E 19
E 13
E 9
E 7

/* path names of a few good fonts on the system */
D 8
#define BIG_FONT_PATH	"12x24"
#define REG_FONT_PATH	"9x15"
#define TINY_FONT_PATH	"5x8"
E 8
I 8
D 11
#define BIG_FONT_PATH	"12x21bas"   /* "12x24" */
#define REG_FONT_PATH	"9x15"       /* "9x15"  */
#define TINY_FONT_PATH	"6x10"       /* "5x8"   */
E 11
I 11
D 19
#define BIG_FONT_PATH	"12x24"  /* "12x21bas" */   /* "12x24" */
E 19
I 19
D 20
#define BIG_FONT_PATH	"12x21bas"  /* "12x21bas" */   /* "12x24" */
E 20
I 20
#define BIG_FONT_PATH	"12x24"  /* "12x21bas" */   /* "12x24" */
E 20
E 19
#define REG_FONT_PATH	"9x15"                      /* "9x15"  */
D 19
#define TINY_FONT_PATH	"5x8"    /* "6x10"     */   /* "5x8"   */
E 19
I 19
D 20
#define TINY_FONT_PATH	"6x10"    /* "6x10"     */   /* "5x8"   */
E 20
I 20
#define TINY_FONT_PATH	"5x8"    /* "6x10"     */   /* "5x8"   */
E 20
E 19
E 11
E 8


/* player and game size limitations */
D 12
#define MAX_PLAYER_SIZE	50	/* max # of persons player process may run */
#define	MAX_GAME_SIZE	500	/* max # of persons in game */
E 12
I 12
#define MAX_PLAYER_SIZE	5	/* max # of persons player process may run */
#define	MAX_GAME_SIZE	20	/* max # of persons in game */
E 12

/* game speed (all items are measured in clicks) */
#define	CLICKS_PER_SEC	1000	/* clicks that pass per second of real time
				   this is set to 1000 for one click per ms */
#define	CLICKS_TO_QUIT	4000	/* time wait when quitting */
#define CLICKS_TO_DIE	5000	/* time wait when dying */
#define USUAL_EXPLOSION_CLICKS	800	/* normal explosion duration */
#define HIT_LINGER_CLICKS	200	/* duration hit register seen */
#define FIRING_WAIT_CLICKS	850	/* usual wait after firing missile */
#define	CLICKS_PER_MOVE	500	/* usual wait between player moves, this is
				   the one to change to adjust player speed */
#define	GAME_OVER_WAIT		12000	/* usual wait for end of game */
#define	GAME_OVER_LINGER	5000	/* how long the driver will linger
					   beyond the GAME_OVER_WAIT time */
#define	NO_IO_CUTOUT_CLICKS	1800000	/* (ms) time till quit when is no io */

/* Factor by which missiles are faster than people */
#define MISSILE_SPEED_FACTOR	2.2

/* socket constants (port numbers should stay same for playing across net) */
#define BOSS_PORT	1137	/* port number we try to get */
#define BOSS_REC_PORT	3323	/* port number for driver receiving players */
#define BOSS_PASSWORD	(game_password)	/* packet password for current game */
#define BOSS_DEBUG	DEBUG	/* TRUE when you want to trace use of boss */
#define DRIVER_SLEEP	1	/* seconds the driver will pause for when
				   checking for new players, until he again
				   checks his current game mail socket */
#define STARTUP_WAIT	60000	/* game clicks a new player will send his
				   MY_INFO packet to other new players when
				   they join the game near time he does */

/* miscellaneous things */
D 2
#define DEBUG		0	/* 1 if we should be in debug mode */
E 2
I 2
D 3
#define DEBUG		1	/* 1 if we should be in debug mode */
E 3
I 3
D 7
#define DEBUG		0	/* 1 if we should be in debug mode */
E 7
I 7
D 10
#define DEBUG		1	/* 1 if we should be in debug mode */
E 10
I 10
#define DEBUG		0	/* 1 if we should be in debug mode */
E 10
E 7
E 3
E 2
#define SPECIAL_IDS	10	/* number of reserved person id numbers */
#define MAX_MISSILE_RANGE	50	/* maximum distance missiles may go */
#define MAX_MULTI_MISSILES	25	/* max number of missiles in packet */
D 6
#define MAX_MESSAGES	20	/* number of recent msgs stored for player */
E 6
I 6
D 16
#define MAX_MESSAGES	40	/* number of recent msgs stored for player */
E 16
I 16
D 18
#define MAX_MESSAGES	30	/* number of recent msgs stored for player */
#define MAX_ERROR_MESSAGES	16	/* number of error msgs stored */
E 18
I 18
#define MAX_MESSAGES	28	/* number of recent msgs stored for player */
#define MAX_ERROR_MESSAGES	12	/* number of error msgs stored */
E 18
I 17
#define PLAYER_MESSAGE_COLUMNS 70	/* width of message windows */
E 17
E 16
E 6
/* see "level.h" for configuration of experience levels, hit points, etc. */

/* map definition particulars (changing these makes old maps unusable) */
#define MAP_NAME_LENGTH		80	/* string size reserved */
#define ROOM_NAME_LENGTH	80	/* ditto */
#define ROOM_WIDTH		20	/* object slots (squares) in width */
#define ROOM_HEIGHT		20	/* object slots (squares) in height */
#define ROOM_DEPTH		2	/* number of objs per square */
#define	NUM_OF_TEAMS		4	/* number of teams game handles */
#define	TEAM_NAME_LENGTH	80	/* string space for team names */

D 6
/* object particulars (changing these makes old object def files unusable) */
E 6
I 6
/* object particulars (not a good idea to change these) */
E 6
#define OBJ_NAME_LENGTH		80	/* string size reserved */
#define BITMAP_WIDTH		32	/* pixels in width */
#define BITMAP_HEIGHT		32	/* pixels in height */
#define BITMAP_ARRAY_SIZE	128	/* number of chars required */
#define MAX_OBJECTS		256	/* maximum types of objects allowed */

D 6
/* inventory limitations (changing sizes might cause display to go funny) */
E 6
I 6
/* inventory limitations (changing might cause player window to look funny) */
E 6
#define INV_WIDTH	7	/* width of inventory box */
#define INV_HEIGHT	5	/* height of same */
#define INV_SIZE	(INV_WIDTH * INV_HEIGHT)
#define ID_MATCH	10	/* the base number for id matching */

/* other misc things (overall these needn't/shouldn't be changed) */
#define PATH_LENGTH	180	/* length of strings for disk access paths */
#define NAME_LENGTH	25	/* one more than maximum player name length */
#define RANK_LENGTH	50	/* string size for rank names */
#define MSG_LENGTH	80	/* longest message sendable player to player */
#define LOGIN_LENGTH	15	/* length of storage for login name */
#define HOST_LENGTH	40	/* length of storage for host name */
#define PASSWORD_LENGTH	 9	/* one more than maximum password length */
#define GAME_NAME_LENGTH 9	/* one more than maximum game name length */
#define UNKNOWN		"#??#"	/* string signifying an unknown variable */
I 12







E 12
E 1
