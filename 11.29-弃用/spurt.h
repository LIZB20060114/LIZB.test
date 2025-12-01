#pragma once
#include "user.h"

void Spurt(void);
void executeUltraOptimizedStep(MAZECOOR* path, int totalLength, int* currentIndex);
int calculateMoveDirection(int fromX, int fromY, int toX, int toY);