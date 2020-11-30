#include "def.h"
#include "ray.h"

RuleSquare rulepack[ROOM_WIDTH][ROOM_HEIGHT] = {

	{{1, 1}, {3, 3}, {5, 5}, {7, 7}, {9, 9}, {11, 11}, {13, 13}, {15, 15}, {17, 17}, {19, 19}, {21, 21}, {23, 23}, {25, 25}, {27, 27}, {29, 29}, {31, 31}, {33, 33}, {35, 35}, {37, 37}, {39, 39}},
	{{1, 0}, {1, 1}, {2, 1}, {3, 2}, {3, 3}, {4, 3}, {5, 4}, {5, 5}, {6, 5}, {7, 6}, {7, 7}, {8, 7}, {9, 8}, {9, 9}, {10, 9}, {11, 10}, {11, 11}, {12, 11}, {13, 12}, {13, 13}},
	{{1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {3, 2}, {3, 2}, {3, 3}, {4, 3}, {4, 3}, {5, 4}, {5, 4}, {5, 5}, {6, 5}, {6, 5}, {7, 6}, {7, 6}, {7, 7}, {8, 7}, {8, 7}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {3, 2}, {3, 2}, {3, 2}, {3, 3}, {4, 3}, {4, 3}, {4, 3}, {5, 4}, {5, 4}, {5, 4}, {5, 5}, {6, 5}, {6, 5}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {3, 2}, {3, 2}, {3, 2}, {3, 2}, {3, 3}, {4, 3}, {4, 3}, {4, 3}, {4, 3}, {5, 4}, {5, 4}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {3, 2}, {3, 2}, {3, 2}, {3, 2}, {3, 2}, {3, 3}, {4, 3}, {4, 3}, {4, 3}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {3, 2}, {3, 2}, {3, 2}, {3, 2}, {3, 2}, {3, 2}, {3, 3}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {3, 2}, {3, 2}, {3, 2}, {3, 2}, {3, 2}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {3, 2}, {3, 2}, {3, 2}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {3, 2}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}, {2, 1}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}, {2, 1}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 1}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 1}, {2, 1}},
	{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}}
};


struct raypack notraypack[ROOM_WIDTH + 2][ROOM_HEIGHT + 2];

struct raypack **raypack;

void SetupRaypack(void)
{ struct raypack **temp;
  int i;

  AINIT(temp, ROOM_HEIGHT + 2);
  raypack = &temp[1];
  for (i=-1; i<ROOM_HEIGHT + 1; i++) {
    raypack[i] = &notraypack[i][1];
  }
}