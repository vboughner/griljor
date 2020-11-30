#define NEW_X_RAN 5 
#define NEW_Y_RAN 2 

typedef struct _sqr
{
  int x1,x2,y1,y2,gx,gy;
  double nw,ne,sw,se;
} SquareDef;

typedef struct _prop
{
  double dismult,diffmult,ranadd;
} PropDef;

typedef struct _stat
{
  int (*treemark)[20],*terrmark;
  int range;
} StatDef;
