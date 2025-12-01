#pragma once

#include"user.h"

struct AStarNode {
    int cX, cY;
    int f, g, h;
    int parentX, parentY;
    int closed;
};
typedef struct AStarNode AStarNode;

int aStarSearch(int startX, int startY, int goalX, int goalY, MAZECOOR* path, int* pathLength);
int heuristic(int x1, int y1, int x2, int y2);
void getNeighbors(int x, int y, MAZECOOR neighbors[], int* count);
void aStarMethod(void);
