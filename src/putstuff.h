
#define MAXSTRLEN 80
#define MAXLINES  1000
#define DEFAULT_HOW_OFTEN 30

typedef int boolean;

typedef struct line {
  char command;
  int objnum;
  int quantity;
  int where;
} Line;

typedef struct DS {
  int NumLines;
  int HowOften;		/* number of seconds between placements */
  int LastPlacement;	/* time (second count) of last placement */
  Line file[MAXLINES];
} DropShipment;


boolean PutStuffInGame();
DropShipment *ReadStuffFile();
