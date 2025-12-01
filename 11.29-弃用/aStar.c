#include "aStar.h"
#include "flood.h"

extern int GoalGet, GoalX, GoalY, GucXStart, GucYStart;

// 启发式函数（曼哈顿距离）
int heuristic(int x1, int y1, int x2, int y2) {
	return abs(x1 - x2) + abs(y1 - y2);
}

void getNeighbors(int x, int y, MAZECOOR neighbors[], int* count) {
	*count = 0;

	printf("=== A*邻居分析: (%d,%d) ===\n", x, y);
	printf("地图块: 0x%02X\n", GucMapBlock[x][y]);
	printf("已搜索: %d\n", GucMapGet[x][y]);

	// 上方向
	if ((GucMapBlock[x][y] & 0x01) && y + 1 < MAZETYPE) {
		neighbors[*count].cX = x;
		neighbors[*count].cY = y + 1;
		printf("  上: (%d,%d) - 可通行\n", x, y + 1);
		(*count)++;
	}
	else {
		printf("  上: 阻挡或边界\n");
	}

	// 右方向
	if ((GucMapBlock[x][y] & 0x02) && x + 1 < MAZETYPE) {
		neighbors[*count].cX = x + 1;
		neighbors[*count].cY = y;
		printf("  右: (%d,%d) - 可通行\n", x + 1, y);
		(*count)++;
	}
	else {
		printf("  右: 阻挡或边界\n");
	}

	// 下方向
	if ((GucMapBlock[x][y] & 0x04) && y - 1 >= 0) {
		neighbors[*count].cX = x;
		neighbors[*count].cY = y - 1;
		printf("  下: (%d,%d) - 可通行\n", x, y - 1);
		(*count)++;
	}
	else {
		printf("  下: 阻挡或边界\n");
	}

	// 左方向
	if ((GucMapBlock[x][y] & 0x08) && x - 1 >= 0) {
		neighbors[*count].cX = x - 1;
		neighbors[*count].cY = y;
		printf("  左: (%d,%d) - 可通行\n", x - 1, y);
		(*count)++;
	}
	else {
		printf("  左: 阻挡或边界\n");
	}

	printf("总邻居数: %d\n", *count);
}

int aStarSearch(int startX, int startY, int goalX, int goalY, MAZECOOR* path, int* pathLength) {
    printf("=== A*搜索开始: (%d,%d) -> (%d,%d) ===\n", startX, startY, goalX, goalY);

    AStarNode nodes[MAZETYPE][MAZETYPE];
    MAZECOOR openList[MAZETYPE * MAZETYPE];
    int openListCount = 0;

    // 初始化节点
    for (int i = 0; i < MAZETYPE; i++) {
        for (int j = 0; j < MAZETYPE; j++) {
            nodes[i][j].cX = i;
            nodes[i][j].cY = j;
            nodes[i][j].f = 9999;
            nodes[i][j].g = 9999;
            nodes[i][j].h = 0;
            nodes[i][j].parentX = -1;
            nodes[i][j].parentY = -1;
            nodes[i][j].closed = 0;
        }
    }

    // 设置起始节点
    nodes[startX][startY].g = 0;
    nodes[startX][startY].h = heuristic(startX, startY, goalX, goalY);
    nodes[startX][startY].f = nodes[startX][startY].g + nodes[startX][startY].h;

    printf("起始节点: f=%d, g=%d, h=%d\n", nodes[startX][startY].f, nodes[startX][startY].g, nodes[startX][startY].h);

    openList[openListCount].cX = startX;
    openList[openListCount].cY = startY;
    openListCount++;

    int iteration = 0;
    while (openListCount > 0 && iteration < 1000) {
        iteration++;

        // 改进的节点选择：当f值相同时，选择h值较小的
        int currentIndex = 0;
        int minF = nodes[openList[0].cX][openList[0].cY].f;
        int minH = nodes[openList[0].cX][openList[0].cY].h;

        for (int i = 1; i < openListCount; i++) {
            int currentF = nodes[openList[i].cX][openList[i].cY].f;
            int currentH = nodes[openList[i].cX][openList[i].cY].h;

            if (currentF < minF || (currentF == minF && currentH < minH)) {
                minF = currentF;
                minH = currentH;
                currentIndex = i;
            }
        }

        int currentX = openList[currentIndex].cX;
        int currentY = openList[currentIndex].cY;

        printf("迭代%d: 当前节点(%d,%d), f=%d, g=%d, h=%d\n",
            iteration, currentX, currentY, minF, nodes[currentX][currentY].g, nodes[currentX][currentY].h);

        // 如果到达目标
        if (currentX == goalX && currentY == goalY) {
            printf("找到路径! 开始回溯\n");

            // 回溯构建路径
            *pathLength = 0;
            int tempX = currentX, tempY = currentY;
            MAZECOOR tempPath[MAX_PATH_LENGTH];
            int tempLength = 0;

            // 先收集路径到临时数组
            while (tempX != -1 && tempY != -1 && tempLength < MAX_PATH_LENGTH) {
                tempPath[tempLength].cX = tempX;
                tempPath[tempLength].cY = tempY;
                tempLength++;

                int parentX = nodes[tempX][tempY].parentX;
                int parentY = nodes[tempX][tempY].parentY;
                tempX = parentX;
                tempY = parentY;
            }

            // 反转路径（从起点到终点）
            *pathLength = tempLength;
            for (int i = 0; i < tempLength; i++) {
                if (i >= MAX_PATH_LENGTH) break;
                path[i] = tempPath[tempLength - 1 - i];
                printf("路径[%d] = (%d,%d)\n", i, path[i].cX, path[i].cY);
            }

            printf("回溯完成，路径长度=%d\n", *pathLength);
            return 1;
        }

        // 从开放列表移除
        openList[currentIndex] = openList[openListCount - 1];
        openListCount--;
        nodes[currentX][currentY].closed = 1;

        // 检查邻居
        MAZECOOR neighbors[4];
        int neighborCount;
        getNeighbors(currentX, currentY, neighbors, &neighborCount);

        for (int i = 0; i < neighborCount; i++) {
            int neighborX = neighbors[i].cX;
            int neighborY = neighbors[i].cY;

            if (nodes[neighborX][neighborY].closed) {
                continue;
            }

            int tentativeG = nodes[currentX][currentY].g + 1;

            if (tentativeG < nodes[neighborX][neighborY].g) {
                printf("  邻居(%d,%d): 更新路径 g=%d->%d\n", neighborX, neighborY, nodes[neighborX][neighborY].g, tentativeG);

                nodes[neighborX][neighborY].parentX = currentX;
                nodes[neighborX][neighborY].parentY = currentY;
                nodes[neighborX][neighborY].g = tentativeG;
                nodes[neighborX][neighborY].h = heuristic(neighborX, neighborY, goalX, goalY);
                nodes[neighborX][neighborY].f = nodes[neighborX][neighborY].g + nodes[neighborX][neighborY].h;

                // 检查是否在开放列表中
                int inOpenList = 0;
                for (int j = 0; j < openListCount; j++) {
                    if (openList[j].cX == neighborX && openList[j].cY == neighborY) {
                        inOpenList = 1;
                        break;
                    }
                }

                if (!inOpenList) {
                    // 不在开放列表中，加入
                    openList[openListCount].cX = neighborX;
                    openList[openListCount].cY = neighborY;
                    openListCount++;
                    printf("  邻居(%d,%d): 加入开放列表\n", neighborX, neighborY);
                }
                else {
                    // 已经在开放列表中，需要重新排序（通过下次循环自然处理）
                    printf("  邻居(%d,%d): 已在开放列表，值已更新\n", neighborX, neighborY);
                }
            }
        }
    }

    printf("A*搜索失败: 开放列表为空或迭代超限\n");
    return 0;
}
// 基于A*的智能搜索方法
void aStarMethod(void) {
	int targetX, targetY;

	// 确定目标点
	if (GoalGet) {
		targetX = GoalX;
		targetY = GoalY;
	}
	else {
		MAZECOOR nearestUnexplored = findUnexploredGateway();
		if (nearestUnexplored.cX != -1) {
			targetX = nearestUnexplored.cX;
			targetY = nearestUnexplored.cY;
		}
		else {
			targetX = 7;
			targetY = 7;
		}
	}

	MAZECOOR path[MAX_PATH_LENGTH];
	int pathLength = 0;

	if (aStarSearch(GmcMouse.cX, GmcMouse.cY, targetX, targetY, path, &pathLength) && pathLength > 1) {
		// 找到路径，移动到下一个格子
		int nextX = path[1].cX;
		int nextY = path[1].cY;

		// 计算移动方向
		int moveDir;
		if (nextX == GmcMouse.cX) {
			if (nextY > GmcMouse.cY) moveDir = UP;
			else moveDir = DOWN;
		}
		else {
			if (nextX > GmcMouse.cX) moveDir = RIGHT;
			else moveDir = LEFT;
		}

		// 计算转向角度并执行
		int turnAngle = (moveDir - GucMouseDir + 4) % 4;
		switch (turnAngle) {
		case 1: StepTurnRight(); break;
		case 2: TurnBack(); break;
		case 3: StepTurnLeft(); break;
		default: break;
		}

		MoveOneBlock();

	}
	else {
		// A*找不到路径，fallback到洪水填充或右手法则
		floodFillMethod();
	}
}