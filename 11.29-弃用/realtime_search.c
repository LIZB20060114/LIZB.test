#include "realtime_search.h"
#include "aStar.h"
#include "flood.h"
#include <stdio.h>

void validateCurrentPositionMap(void) {
    printf("=== 当前位置地图验证: (%d,%d) ===\n", GmcMouse.cX, GmcMouse.cY);
    printf("GucMapGet[%d][%d] = %d\n", GmcMouse.cX, GmcMouse.cY, GucMapGet[GmcMouse.cX][GmcMouse.cY]);
    printf("GucMapBlock[%d][%d] = 0x%02X\n", GmcMouse.cX, GmcMouse.cY, GucMapBlock[GmcMouse.cX][GmcMouse.cY]);

    if (GucMapGet[GmcMouse.cX][GmcMouse.cY] == 0) {
        printf("!!! 严重错误: 当前位置未标记为已搜索 !!!\n");
    }

    if (GucMapBlock[GmcMouse.cX][GmcMouse.cY] == 0x00) {
        printf("!!! 严重错误: 当前位置地图块为0x00但实际可通行 !!!\n");
        // 紧急修复
        printf("紧急修复当前位置地图块...\n");
        GucMapBlock[GmcMouse.cX][GmcMouse.cY] = 0x0F; // 设置为四面可通行
    }
}

// 检查两个格子之间是否有直接墙壁阻挡
int isBlockedByWall(int fromX, int fromY, int toX, int toY) {
    if (fromX == toX) {
        // 垂直方向
        if (toY > fromY) {
            // 向上：检查当前格子的上墙和目标格子的下墙
            return !(GucMapBlock[fromX][fromY] & 0x01) || !(GucMapBlock[toX][toY] & 0x04);
        }
        else {
            // 向下：检查当前格子的下墙和目标格子的上墙
            return !(GucMapBlock[fromX][fromY] & 0x04) || !(GucMapBlock[toX][toY] & 0x01);
        }
    }
    else if (fromY == toY) {
        // 水平方向
        if (toX > fromX) {
            // 向右：检查当前格子的右墙和目标格子的左墙
            return !(GucMapBlock[fromX][fromY] & 0x02) || !(GucMapBlock[toX][toY] & 0x08);
        }
        else {
            // 向左：检查当前格子的左墙和目标格子的右墙
            return !(GucMapBlock[fromX][fromY] & 0x08) || !(GucMapBlock[toX][toY] & 0x02);
        }
    }

    return 1; // 斜角方向认为有阻挡
}

void realTimeAStarSearch(void) {
    int currentX = GmcMouse.cX;
    int currentY = GmcMouse.cY;

    MAZECOOR nearestUnexplored = findNearestUnexplored();

    if (nearestUnexplored.cX != -1) {
        int manhattanDist = abs(nearestUnexplored.cX - currentX) +
            abs(nearestUnexplored.cY - currentY);

        printf("RT-A*: 目标(%d,%d), 当前(%d,%d), 直线距离=%d\n",
            nearestUnexplored.cX, nearestUnexplored.cY, currentX, currentY, manhattanDist);

        MAZECOOR searchPath[MAX_PATH_LENGTH];
        int pathLength = 0;

        if (aStarSearch(currentX, currentY, nearestUnexplored.cX, nearestUnexplored.cY,
            searchPath, &pathLength)) {
            printf("RT-A*: 路径长度=%d\n", pathLength);

            // 关键：检查路径质量
            if (pathLength <= manhattanDist + 3) {
                // 路径合理，执行
                printf("RT-A*: 路径质量良好，执行移动\n");
                executeOptimizedStep(searchPath, pathLength, 1);
                return;
            }
            else {
                // 路径绕路过多，直接使用洪水填充
                printf("RT-A*: 路径绕路过多(%d vs %d)，使用洪水填充\n", pathLength, manhattanDist);
                floodFillMethod();
                return;
            }
        }
    }

    printf("RT-A*: 使用洪水填充备用\n");
    floodFillMethod();
}


int calculateUnexploredPriority(int x, int y) {
    int currentX = GmcMouse.cX;
    int currentY = GmcMouse.cY;

    // 1. 距离因素（距离越近优先级越高）
    int distance = abs(x - currentX) + abs(y - currentY);
    int distanceScore = (MAZETYPE * 2 - distance) * 10;

    // 2. 可达性因素（有已探索邻居优先级更高）
    int accessibilityScore = 0;
    if (x > 0 && GucMapGet[x - 1][y] == 1 && (GucMapBlock[x - 1][y] & 0x02))
        accessibilityScore += 8;
    if (x < MAZETYPE - 1 && GucMapGet[x + 1][y] == 1 && (GucMapBlock[x + 1][y] & 0x08))
        accessibilityScore += 8;
    if (y > 0 && GucMapGet[x][y - 1] == 1 && (GucMapBlock[x][y - 1] & 0x01))
        accessibilityScore += 8;
    if (y < MAZETYPE - 1 && GucMapGet[x][y + 1] == 1 && (GucMapBlock[x][y + 1] & 0x04))
        accessibilityScore += 8;

    // 3. 中心区域优先级
    int centerBonus = 0;
    if ((x >= 6 && x <= 9) && (y >= 6 && y <= 9)) {
        centerBonus = 25;
    }

    // 4. 边界惩罚
    int borderPenalty = 0;
    if (x == 0 || x == MAZETYPE - 1 || y == 0 || y == MAZETYPE - 1) {
        borderPenalty = -10;
    }

    // 5. 新增：墙壁阻挡惩罚
    int wallPenalty = 0;
    if (distance == 1) {
        // 相邻格子，检查是否被墙壁直接隔开
        if (x == currentX) {
            // 垂直相邻
            if (y > currentY && !(GucMapBlock[currentX][currentY] & 0x01)) {
                wallPenalty = -50; // 上方有墙阻挡
            }
            else if (y < currentY && !(GucMapBlock[currentX][currentY] & 0x04)) {
                wallPenalty = -50; // 下方有墙阻挡
            }
        }
        else if (y == currentY) {
            // 水平相邻
            if (x > currentX && !(GucMapBlock[currentX][currentY] & 0x02)) {
                wallPenalty = -50; // 右方有墙阻挡
            }
            else if (x < currentX && !(GucMapBlock[currentX][currentY] & 0x08)) {
                wallPenalty = -50; // 左方有墙阻挡
            }
        }
    }

    int totalScore = distanceScore + accessibilityScore + centerBonus + borderPenalty + wallPenalty;

    printf("坐标(%d,%d): 距离=%d, 可达性=%d, 墙壁惩罚=%d, 总分=%d\n",
        x, y, distanceScore, accessibilityScore, wallPenalty, totalScore);

    return totalScore;
}
void executeOptimizedStep(MAZECOOR* path, int pathLength, int startIndex) {
    if (startIndex >= pathLength) return;

    // 使用当前实时位置，而不是路径中的起点
    int currentX = GmcMouse.cX;
    int currentY = GmcMouse.cY;
    int currentDir = GucMouseDir;

    int nextX = path[startIndex].cX;
    int nextY = path[startIndex].cY;

    printf("执行移动: 实际位置(%d,%d)->目标(%d,%d)\n", currentX, currentY, nextX, nextY);

    // 如果当前位置与路径起点不一致，需要调整
    if (currentX != path[0].cX || currentY != path[0].cY) {
        printf("!!! 路径起点不匹配: 期望(%d,%d), 实际(%d,%d) !!!\n",
            path[0].cX, path[0].cY, currentX, currentY);
        // 可以在这里重新计算路径或调整逻辑
    }

    // 计算移动方向
    int moveDir;
    if (nextX == currentX) {
        moveDir = (nextY > currentY) ? UP : DOWN;
    }
    else {
        moveDir = (nextX > currentX) ? RIGHT : LEFT;
    }

    printf("需要方向: %d, 当前方向: %d\n", moveDir, currentDir);

    // 执行转向
    int turnAngle = (moveDir - currentDir + 4) % 4;
    printf("转向角度: %d\n", turnAngle);

    switch (turnAngle) {
    case 1: StepTurnRight(); break;
    case 2: TurnBack(); break;
    case 3: StepTurnLeft(); break;
    default: break;
    }

    // 执行移动
    MoveOneBlock();

    printf("移动完成: 新位置(%d,%d)\n", GmcMouse.cX, GmcMouse.cY);
}