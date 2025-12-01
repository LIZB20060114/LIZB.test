#pragma once
#include "user.h"

void realTimeAStarSearch(void);
void executeOptimizedStep(MAZECOOR* path, int pathLength, int startIndex);
int calculateUnexploredPriority(int x, int y);