#pragma once
#include"user.h"
#include<math.h>

void floodFillMethod(void);
void smartBacktrack(void);
int isSearchComplete(void);
MAZECOOR findUnexploredGateway(void);
// 找到最近的未搜索格子
MAZECOOR findNearestUnexplored(void);

