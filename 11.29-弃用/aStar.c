#include "aStar.h"
#include "flood.h"

extern int GoalGet, GoalX, GoalY, GucXStart, GucYStart;

// ����ʽ�����������پ��룩
int heuristic(int x1, int y1, int x2, int y2) {
	return abs(x1 - x2) + abs(y1 - y2);
}

void getNeighbors(int x, int y, MAZECOOR neighbors[], int* count) {
	*count = 0;

	printf("=== A*�ھӷ���: (%d,%d) ===\n", x, y);
	printf("��ͼ��: 0x%02X\n", GucMapBlock[x][y]);
	printf("������: %d\n", GucMapGet[x][y]);

	// �Ϸ���
	if ((GucMapBlock[x][y] & 0x01) && y + 1 < MAZETYPE) {
		neighbors[*count].cX = x;
		neighbors[*count].cY = y + 1;
		printf("  ��: (%d,%d) - ��ͨ��\n", x, y + 1);
		(*count)++;
	}
	else {
		printf("  11111\n");
	}

	// �ҷ���
	if ((GucMapBlock[x][y] & 0x02) && x + 1 < MAZETYPE) {
		neighbors[*count].cX = x + 1;
		neighbors[*count].cY = y;
		printf("  ��: (%d,%d) - ��ͨ��\n", x + 1, y);
		(*count)++;
	}
	else {
		printf("  ��: �赲��߽�\n");
	}

	// �·���
	if ((GucMapBlock[x][y] & 0x04) && y - 1 >= 0) {
		neighbors[*count].cX = x;
		neighbors[*count].cY = y - 1;
		printf("  ��: (%d,%d) - ��ͨ��\n", x, y - 1);
		(*count)++;
	}
	else {
		printf("  ��: �赲��߽�\n");
	}

	// ����
	if ((GucMapBlock[x][y] & 0x08) && x - 1 >= 0) {
		neighbors[*count].cX = x - 1;
		neighbors[*count].cY = y;
		printf("  ��: (%d,%d) - ��ͨ��\n", x - 1, y);
		(*count)++;
	}
	else {
		printf("right\n");
	}

	printf("���ھ���: %d\n", *count);
}

int aStarSearch(int startX, int startY, int goalX, int goalY, MAZECOOR* path, int* pathLength) {
    printf("=== A*������ʼ: (%d,%d) -> (%d,%d) ===\n", startX, startY, goalX, goalY);

    AStarNode nodes[MAZETYPE][MAZETYPE];
    MAZECOOR openList[MAZETYPE * MAZETYPE];
    int openListCount = 0;

    // ��ʼ���ڵ�
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

    // ������ʼ�ڵ�
    nodes[startX][startY].g = 0;
    nodes[startX][startY].h = heuristic(startX, startY, goalX, goalY);
    nodes[startX][startY].f = nodes[startX][startY].g + nodes[startX][startY].h;

    printf("��ʼ�ڵ�: f=%d, g=%d, h=%d\n", nodes[startX][startY].f, nodes[startX][startY].g, nodes[startX][startY].h);

    openList[openListCount].cX = startX;
    openList[openListCount].cY = startY;
    openListCount++;

    int iteration = 0;
    while (openListCount > 0 && iteration < 1000) {
        iteration++;

        // �Ľ��Ľڵ�ѡ�񣺵�fֵ��ͬʱ��ѡ��hֵ��С��
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

        printf("����%d: ��ǰ�ڵ�(%d,%d), f=%d, g=%d, h=%d\n",
            iteration, currentX, currentY, minF, nodes[currentX][currentY].g, nodes[currentX][currentY].h);

        // �������Ŀ��
        if (currentX == goalX && currentY == goalY) {
            printf("�ҵ�·��! ��ʼ����\n");

            // ���ݹ���·��
            *pathLength = 0;
            int tempX = currentX, tempY = currentY;
            MAZECOOR tempPath[MAX_PATH_LENGTH];
            int tempLength = 0;

            // ���ռ�·������ʱ����
            while (tempX != -1 && tempY != -1 && tempLength < MAX_PATH_LENGTH) {
                tempPath[tempLength].cX = tempX;
                tempPath[tempLength].cY = tempY;
                tempLength++;

                int parentX = nodes[tempX][tempY].parentX;
                int parentY = nodes[tempX][tempY].parentY;
                tempX = parentX;
                tempY = parentY;
            }

            // ��ת·��������㵽�յ㣩
            *pathLength = tempLength;
            for (int i = 0; i < tempLength; i++) {
                if (i >= MAX_PATH_LENGTH) break;
                path[i] = tempPath[tempLength - 1 - i];
                printf("·��[%d] = (%d,%d)\n", i, path[i].cX, path[i].cY);
            }

            printf("������ɣ�·������=%d\n", *pathLength);
            return 1;
        }

        // �ӿ����б��Ƴ�
        openList[currentIndex] = openList[openListCount - 1];
        openListCount--;
        nodes[currentX][currentY].closed = 1;

        // ����ھ�
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
                printf("  �ھ�(%d,%d): ����·�� g=%d->%d\n", neighborX, neighborY, nodes[neighborX][neighborY].g, tentativeG);

                nodes[neighborX][neighborY].parentX = currentX;
                nodes[neighborX][neighborY].parentY = currentY;
                nodes[neighborX][neighborY].g = tentativeG;
                nodes[neighborX][neighborY].h = heuristic(neighborX, neighborY, goalX, goalY);
                nodes[neighborX][neighborY].f = nodes[neighborX][neighborY].g + nodes[neighborX][neighborY].h;

                // ����Ƿ��ڿ����б���
                int inOpenList = 0;
                for (int j = 0; j < openListCount; j++) {
                    if (openList[j].cX == neighborX && openList[j].cY == neighborY) {
                        inOpenList = 1;
                        break;
                    }
                }

                if (!inOpenList) {
                    // ���ڿ����б��У�����
                    openList[openListCount].cX = neighborX;
                    openList[openListCount].cY = neighborY;
                    openListCount++;
                    printf("  �ھ�(%d,%d): ���뿪���б�\n", neighborX, neighborY);
                }
                else {
                    // �Ѿ��ڿ����б��У���Ҫ��������ͨ���´�ѭ����Ȼ������
                    printf("  �ھ�(%d,%d): ���ڿ����б���ֵ�Ѹ���\n", neighborX, neighborY);
                }
            }
        }
    }

    printf("A*����ʧ��: �����б�Ϊ�ջ��������\n");
    return 0;
}
// ����A*��������������
void aStarMethod(void) {
	int targetX, targetY;

	// ȷ��Ŀ���
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
		// �ҵ�·�����ƶ�����һ������
		int nextX = path[1].cX;
		int nextY = path[1].cY;

		// �����ƶ�����
		int moveDir;
		if (nextX == GmcMouse.cX) {
			if (nextY > GmcMouse.cY) moveDir = UP;
			else moveDir = DOWN;
		}
		else {
			if (nextX > GmcMouse.cX) moveDir = RIGHT;
			else moveDir = LEFT;
		}

		// ����ת��ǶȲ�ִ��
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
		// A*�Ҳ���·����fallback����ˮ�������ַ���
		floodFillMethod();
	}
}