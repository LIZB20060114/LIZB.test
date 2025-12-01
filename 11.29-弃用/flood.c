#include"flood.h"

extern int GoalGet, GoalX, GoalY, GucXStart, GucYStart;


void floodFillMethod(void)
{
    int targetX, targetY;

    // 如果已经发现目标，就以目标为导向；否则以中心区域为导向
    if (GoalGet) {
        targetX = GoalX;
        targetY = GoalY;
    }
    else {
        // 以迷宫中心(7,7)或(8,8)等作为临时目标
        targetX = 7;
        targetY = 7;
    }

    // 计算到目标的最短路径
    mapStepEdit(targetX, targetY);

    int currentStep = GucMapStep[GmcMouse.cX][GmcMouse.cY];
    int bestDir = -1;
    int minStep = 255;

    // 检查四个方向，选择步数最小且未搜索的方向
    if ((GucMapBlock[GmcMouse.cX][GmcMouse.cY] & 0x01) &&
        (GucMapGet[GmcMouse.cX][GmcMouse.cY + 1] == 0) &&
        (GucMapStep[GmcMouse.cX][GmcMouse.cY + 1] < minStep)) {
        minStep = GucMapStep[GmcMouse.cX][GmcMouse.cY + 1];
        bestDir = UP;
    }

    if ((GucMapBlock[GmcMouse.cX][GmcMouse.cY] & 0x02) &&
        (GucMapGet[GmcMouse.cX + 1][GmcMouse.cY] == 0) &&
        (GucMapStep[GmcMouse.cX + 1][GmcMouse.cY] < minStep)) {
        minStep = GucMapStep[GmcMouse.cX + 1][GmcMouse.cY];
        bestDir = RIGHT;
    }

    if ((GucMapBlock[GmcMouse.cX][GmcMouse.cY] & 0x04) &&
        (GucMapGet[GmcMouse.cX][GmcMouse.cY - 1] == 0) &&
        (GucMapStep[GmcMouse.cX][GmcMouse.cY - 1] < minStep)) {
        minStep = GucMapStep[GmcMouse.cX][GmcMouse.cY - 1];
        bestDir = DOWN;
    }

    if ((GucMapBlock[GmcMouse.cX][GmcMouse.cY] & 0x08) &&
        (GucMapGet[GmcMouse.cX - 1][GmcMouse.cY] == 0) &&
        (GucMapStep[GmcMouse.cX - 1][GmcMouse.cY] < minStep)) {
        minStep = GucMapStep[GmcMouse.cX - 1][GmcMouse.cY];
        bestDir = LEFT;
    }

    // 如果找到合适的方向，转向并移动
    if (bestDir != -1) {
        // 计算需要转向的角度
        int turnAngle = (bestDir - GucMouseDir + 4) % 4;

        switch (turnAngle) {
        case 1:  // 右转90度
            StepTurnRight();
            break;
        case 2:  // 掉头
            TurnBack();
            break;
        case 3:  // 左转90度
            StepTurnLeft();
            break;
        case 0:  // 不需要转向
        default:
            break;
        }

        MoveOneBlock();

        // 记录岔路口（如果有多于1个选择）
        if (crosswayCheck(GmcMouse.cX, GmcMouse.cY) > 1) {
            GmcCrossway[crosswayNum].cX = GmcMouse.cX;
            GmcCrossway[crosswayNum].cY = GmcMouse.cY;
            crosswayNum++;
        }
    }
    else {
        // 没有找到合适方向， fallback 到右手法则
        rightMethod();
    }
}

void smartBacktrack(void)
{
    // 寻找最近的已搜索格子，该格子有未搜索的邻居
    MAZECOOR gateway = findUnexploredGateway();

    if (gateway.cX != -1 && gateway.cY != -1) {
        // 导航到这个有未搜索邻居的格子
        objectGoTo(gateway.cX, gateway.cY);
    }
    else {
        // 所有区域都已搜索，返回起点准备冲刺
        objectGoTo(GucXStart, GucYStart);
        TurnBack();
        GucMouseTask = SPURT;
    }
}

MAZECOOR findUnexploredGateway(void)
{
    MAZECOOR bestGateway = { -1, -1 };
    int minDistance = MAZETYPE * MAZETYPE;

    // 遍历所有已搜索的格子
    for (int i = 0; i < MAZETYPE; i++) {
        for (int j = 0; j < MAZETYPE; j++) {
            if (GucMapGet[i][j] == 1) { // 已搜索的格子
                // 检查这个格子是否有未搜索的邻居
                int hasUnexploredNeighbor = 0;

                // 检查上方向
                if ((GucMapBlock[i][j] & 0x01) && (j + 1 < MAZETYPE) && (GucMapGet[i][j + 1] == 0)) {
                    hasUnexploredNeighbor = 1;
                }
                // 检查右方向
                else if ((GucMapBlock[i][j] & 0x02) && (i + 1 < MAZETYPE) && (GucMapGet[i + 1][j] == 0)) {
                    hasUnexploredNeighbor = 1;
                }
                // 检查下方向
                else if ((GucMapBlock[i][j] & 0x04) && (j - 1 >= 0) && (GucMapGet[i][j - 1] == 0)) {
                    hasUnexploredNeighbor = 1;
                }
                // 检查左方向
                else if ((GucMapBlock[i][j] & 0x08) && (i - 1 >= 0) && (GucMapGet[i - 1][j] == 0)) {
                    hasUnexploredNeighbor = 1;
                }

                if (hasUnexploredNeighbor) {
                    int distance = abs(i - GmcMouse.cX) + abs(j - GmcMouse.cY);
                    if (distance < minDistance) {
                        minDistance = distance;
                        bestGateway.cX = i;
                        bestGateway.cY = j;
                    }
                }
            }
        }
    }

    return bestGateway;
}

// 检查搜索是否完成
int isSearchComplete(void)
{
    for (int i = 0; i < MAZETYPE; i++) {
        for (int j = 0; j < MAZETYPE; j++) {
            if (GucMapGet[i][j] == 0) {
                return 0; // 还有未搜索的格子
            }
        }
    }
    return 1; // 全部搜索完成
}