h65247
s 00000/00000/00116
d D 1.2 92/08/07 01:00:00 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00116/00000/00000
d D 1.1 91/03/24 18:44:14 labc-3id 1 0
c date and time created 91/03/24 18:44:14 by labc-3id
e
u
U
f e 0
t
T
I 1
int EditItem();

ObjInfo tobj;

/*   Name                Proc  Data   Type     Offset */
DialogList MassEdit[] = 
{{"Object"		,TITLEBOX},
   { "",BLANKBOX},
   { "Done",QUITBOX },
   { "",BLANKBOX},
   { "",BLANKBOX},
   { "",BLANKBOX},
   { "Group"            ,MARKSHO, 0},
   { ""		,BLANKBOX},
   { ""		,BLANKBOX},
   { ""		,BLANKBOX},
   { ""			,BLANKBOX},
   { ""			,BLANKBOX},
   { ""			,BLANKBOX},
   { ""			,BLANKBOX},
   { ""			,BLANKBOX},
   { ""			,BLANKBOX},
   { ""			,BLANKBOX},
   { "Flag"	        ,QUITBOX, OFFSET(tobj,flag),EditItem,	BOLBOX},
   { "Target"	,QUITBOX, OFFSET(tobj,target),EditItem,		BOLBOX},
   { "Magic"         ,QUITBOX, OFFSET(tobj,magic),EditItem,	INTBOX},
   { "Ignoring"	,QUITBOX, OFFSET(tobj,ignoring),EditItem,BOLBOX},
   { "Piercing"	,QUITBOX, OFFSET(tobj,piercing),EditItem,	BOLBOX},
   { "Destroys"        ,QUITBOX, OFFSET(tobj,destroys),EditItem,BOLBOX},
   { "Boombit"        ,QUITBOX, OFFSET(tobj,boombit),EditItem,	INTBOX},
   { "", NEWCOL },
   { "Click on field to edit.  All changed are perminant.", TITLEBOX},
   
   { "General:"	,TITLEBOX},
   { "Masked"	,QUITBOX, OFFSET(tobj,masked),EditItem,		BOLBOX},
   { "Recorded"	,QUITBOX, OFFSET(tobj,recorded),EditItem,	BOLBOX},
   { "Transparent",QUITBOX, OFFSET(tobj,transparent),EditItem,	BOLBOX},
   { "Glows"	,QUITBOX, OFFSET(tobj,glows),EditItem,		BOLBOX},
   { "Flashlight",QUITBOX, OFFSET(tobj,flashlight),EditItem,	BOLBOX},
   { "Move"	,QUITBOX, OFFSET(tobj,move),EditItem,		BOLBOX},
   { "Movement"	,QUITBOX, OFFSET(tobj,movement),EditItem,	INTBOX},
   { "Override"	,QUITBOX, OFFSET(tobj,override),EditItem,	BOLBOX},
   { "Concealing",QUITBOX, OFFSET(tobj,concealing),EditItem,	BOLBOX},
   { "Permeable"	,QUITBOX, OFFSET(tobj,permeable),EditItem,BOLBOX},
   { "Exit"	,QUITBOX, OFFSET(tobj,exit),EditItem,		BOLBOX},
   { "Pushable"	,QUITBOX, OFFSET(tobj,pushable),EditItem,	BOLBOX},
   { ""			,BLANKBOX},
   { "Takeable"	,QUITBOX, OFFSET(tobj,takeable),EditItem,	BOLBOX},
   { "Weight"	,QUITBOX, OFFSET(tobj,weight),EditItem,		INTBOX},
   { ""			,BLANKBOX},
   { "Vulnerable",QUITBOX, OFFSET(tobj,vulnerable),EditItem,	BOLBOX},
   { "Destroyed"     ,QUITBOX, OFFSET(tobj,destroyed),EditItem,	LINKPIC},
   { "Restorable",QUITBOX, OFFSET(tobj,restorable),EditItem,	BOLBOX},
   { "Restored"       ,QUITBOX, OFFSET(tobj,restored),EditItem,	LINKPIC},

   { "New columb"	,NEWCOL},
   { ""			,BLANKBOX},
   { "Weapons:"		,TITLEBOX},
   { "Weapon"        ,QUITBOX, OFFSET(tobj,weapon),EditItem,	BOLBOX},
   { "Damage"	,QUITBOX, OFFSET(tobj,damage),EditItem,		INTBOX},
   { "Range"	,QUITBOX, OFFSET(tobj,range),EditItem,		INTBOX},
   { "Speed"	,QUITBOX, OFFSET(tobj,speed),EditItem,		INTBOX},
   { "Movingobj",QUITBOX, OFFSET(tobj,movingobj),EditItem,	LINKPIC},
   { "Stop"	,QUITBOX, OFFSET(tobj,stop),EditItem,		BOLBOX},
   { "Explodes"	,QUITBOX, OFFSET(tobj,explodes),EditItem,	INTBOX},
   { "Flamable"	,QUITBOX, OFFSET(tobj,flamable),EditItem,	INTBOX},
   { "Charges"	,QUITBOX, OFFSET(tobj,charges),EditItem,	INTBOX},
   { "Numbered"	,QUITBOX, OFFSET(tobj,numbered),EditItem,BOLBOX},
   { "Lost"	,QUITBOX, OFFSET(tobj,lost),EditItem,		BOLBOX},
   { "Thrown"	,QUITBOX, OFFSET(tobj,thrown),EditItem,		BOLBOX},
   { "Refire"	,QUITBOX, OFFSET(tobj,refire),EditItem,		INTBOX},
   { "Capacity"	,QUITBOX, OFFSET(tobj,capacity),EditItem,	INTBOX},
   { "Directional",QUITBOX, OFFSET(tobj,directional),EditItem,	BOLBOX},

   { ""			,BLANKBOX},
   { ""			,BLANKBOX},
   { "Armor:"           ,TITLEBOX},
   { "Armor"	,QUITBOX, OFFSET(tobj,armor),EditItem,		BOLBOX},
   { "Defense" ,QUITBOX, OFFSET(tobj,defense),EditItem,		INTBOX},
   { "Absorb"           ,QUITBOX, OFFSET(tobj,absorb),EditItem,	INTBOX},
   { "Wearable"         ,QUITBOX, OFFSET(tobj,wearable),EditItem,INTBOX},

   { "New columb"	,NEWCOL},
   { ""			,BLANKBOX},
   { "Defaults:"	,TITLEBOX},   
   { "Def1"	,QUITBOX, OFFSET(tobj,set[0]),EditItem,		BOLBOX},
   { "Def2"	,QUITBOX, OFFSET(tobj,set[1]),EditItem,		BOLBOX},
   { "Def3"	,QUITBOX, OFFSET(tobj,set[2]),EditItem,		BOLBOX},
   { "Def4"	,QUITBOX, OFFSET(tobj,set[3]),EditItem,		BOLBOX},
   { "Def5"	,QUITBOX, OFFSET(tobj,set[4]),EditItem,		BOLBOX},
   { "Def6"	,QUITBOX, OFFSET(tobj,set[5]),EditItem,		BOLBOX},
   { "Def7"	,QUITBOX, OFFSET(tobj,set[6]),EditItem,		BOLBOX},
   { ""			,BLANKBOX},
   { ""			,BLANKBOX},
   { ""			,BLANKBOX},
   { "Swings"	,QUITBOX, OFFSET(tobj,swings),EditItem,		BOLBOX},
   { "Alternate"     ,QUITBOX, OFFSET(tobj,alternate),EditItem,	LINKPIC},
   { "Id"	,QUITBOX, OFFSET(tobj,id),EditItem,		BOLBOX},
   { "Type"             ,QUITBOX, OFFSET(tobj,type),EditItem,	INTBOX},
   { "Opens"            ,QUITBOX, OFFSET(tobj,opens),EditItem,	INTBOX},

   { "New columb"	,NEWCOL},
   { ""			,BLANKBOX},
   { ""			,BLANKBOX},
   { "Data"	,QUITBOX, OFFSET(tobj,def[0]),EditItem,		INTBOX},
   { "Data"	,QUITBOX, OFFSET(tobj,def[1]),EditItem,		INTBOX},
   { "Data"	,QUITBOX, OFFSET(tobj,def[2]),EditItem,		INTBOX},
   { "Data"	,QUITBOX, OFFSET(tobj,def[3]),EditItem,		INTBOX},
   { "Data"	,QUITBOX, OFFSET(tobj,def[4]),EditItem,		INTBOX},
   { "Data"	,QUITBOX, OFFSET(tobj,def[5]),EditItem,		INTBOX},
   { "Data"	,QUITBOX, OFFSET(tobj,def[6]),EditItem,		INTBOX},
   { NULL }};




E 1
