#include<stdio.h>
#include"user.h"
#include"flood.h"
#include"aStar.h"
#include"spurt.h"
#include"realtime_search.h"
int GoalGet = 0;
int GoalX, GoalY;
int GucXStart = 0;
int GucYStart = 0;
int userMain(void)
{
	int temp;
	int canMoveNum;
	static int lastMouseX = -1, lastMouseY = -1;

	mouseInit();

	while (1)
	{
		printf("状态: %d, 位置: (%d,%d), 方向: %d, 目标获取: %d\n",
			GucMouseTask, GmcMouse.cX, GmcMouse.cY, GucMouseDir, GoalGet);

		if (GmcMouse.cX >= 6 && GmcMouse.cX <= 9 && GmcMouse.cY >= 6 && GmcMouse.cY <= 9) {
			printf("== 在终点附近 ==\n");
		}
		// 若进入未搜索地区，则更新墙壁情况
		if (GucMapGet[GmcMouse.cX][GmcMouse.cY] == 0)
		{
			updateMap();
		}
		if (lastMouseX != -1 && (GmcMouse.cX != lastMouseX || GmcMouse.cY != lastMouseY)) {
			printf("!!! 主循环位置变化: (%d,%d) -> (%d,%d) !!!\n",
				lastMouseX, lastMouseY, GmcMouse.cX, GmcMouse.cY);
		}
		lastMouseX = GmcMouse.cX;
		lastMouseY = GmcMouse.cY;
		// 根据运行状态，分类处理
		switch (GucMouseTask)
		{

		case WAIT:
			
			GucMouseTask = START;
			break;

		case START:

			if (GucMapBlock[GmcMouse.cX][GmcMouse.cY] & 0x08)
			{
				GucXStart = MAZETYPE - 1;
				GmcMouse.cX = MAZETYPE - 1;

				if (GmcMouse.cY < MAZETYPE - 1)
				{
					GucMapBlock[MAZETYPE - 1][GmcMouse.cY + 1] = GucMapBlock[0][GmcMouse.cY + 1];
					GucMapBlock[0][GmcMouse.cY + 1] = 0xf0;
				}
				temp = GmcMouse.cY;
				do
				{
					// 转换墙壁信息
					GucMapBlock[MAZETYPE - 1][temp] = GucMapBlock[0][temp];
					GucMapBlock[MAZETYPE - 2][temp] = 0xD0;
					GucMapBlock[0][temp] = 0xf0;
					GucMapBlock[1][temp] = 0xf0;

					// 转换已搜索区域信息
					GucMapGet[MAZETYPE - 1][temp] = 1;
					GucMapGet[0][temp] = 0;

				} while (temp--);
				GucMapBlock[MAZETYPE - 2][GmcMouse.cY] = 0xf0;

				GucMouseTask = MAZESEARCH;
			}
			else if (GucMapBlock[GmcMouse.cX][GmcMouse.cY] & 0x02)
			{
				GucMouseTask = MAZESEARCH;
			}
			else MoveOneBlock();
			break;

		case MAZESEARCH:
			if (((GmcMouse.cX == 7) || (GmcMouse.cX == 8)) &&
				((GmcMouse.cY == 7) || (GmcMouse.cY == 8)) && !GoalGet) {
				GoalGet = 1;
				GoalX = GmcMouse.cX;
				GoalY = GmcMouse.cY;
				printf("*** 目标发现: (%d,%d) ***\n", GoalX, GoalY);
			}

			if (GoalGet) {
				printf("*** 开始冲刺阶段 ***\n");
				objectGoTo(GucXStart, GucYStart);
				TurnBack();
				GucMouseTask = SPURT;
				break;
			}

			int canMoveNum = crosswayCheck(GmcMouse.cX, GmcMouse.cY);

			if (canMoveNum > 0) {
				realTimeAStarSearch();
			}
			else {
				smartBacktrack();
			}
			break;

		case SPURT:
			Spurt();
			break;

		case END:

			mouseEnd();
			return 1; // 用户算法执行完毕，主动结束仿真
			break;
		}

	}

	mouseEnd();
	return 0;	// 用户算法执行完毕，主动结束仿真

}

// 更新迷宫墙壁信息
void updateMap(void)
{
	printf("=== updateMap 开始: 位置(%d,%d), 方向%d ===\n", GmcMouse.cX, GmcMouse.cY, GucMouseDir);
	printf("传感器读数: 左=%d, 前=%d, 右=%d\n", leftHasWall, frontHasWall, rightHasWall);

	uchar ucMap = 0;
	uchar temp, temp1;

	// 标记为已搜索
	GucMapGet[GmcMouse.cX][GmcMouse.cY] = 1;
	printf("设置 GucMapGet[%d][%d] = 1\n", GmcMouse.cX, GmcMouse.cY);

	// 设置基础方向 - 这里可能有逻辑问题！
	ucMap |= MOUSEWAY_B;
	printf("设置后方可通行 (MOUSEWAY_B)\n");

	if (leftHasWall)
	{
		ucMap &= ~MOUSEWAY_L;
		printf("左侧有墙，清除 MOUSEWAY_L\n");
	}
	else
	{
		ucMap |= MOUSEWAY_L;
		printf("左侧无墙，设置 MOUSEWAY_L\n");
	}
	if (frontHasWall)
	{
		ucMap &= ~MOUSEWAY_F;
		printf("前方有墙，清除 MOUSEWAY_F\n");
	}
	else
	{
		ucMap |= MOUSEWAY_F;
		printf("前方无墙，设置 MOUSEWAY_F\n");
	}
	if (rightHasWall)
	{
		ucMap &= ~MOUSEWAY_R;
		printf("右侧有墙，清除 MOUSEWAY_R\n");
	}
	else
	{
		ucMap |= MOUSEWAY_R;
		printf("右侧无墙，设置 MOUSEWAY_R\n");
	}

	printf("计算的地图块值: 0x%02X\n", ucMap);

	// 保存到地图块
	GucMapBlock[GmcMouse.cX][GmcMouse.cY] = ucMap;
	printf("设置 GucMapBlock[%d][%d] = 0x%02X\n", GmcMouse.cX, GmcMouse.cY, ucMap);

	// 解析地图块值，显示具体含义
	printf("地图块解析: 上=%d, 右=%d, 下=%d, 左=%d\n",
		(ucMap & 0x01) ? 1 : 0,  // 上
		(ucMap & 0x02) ? 1 : 0,  // 右  
		(ucMap & 0x04) ? 1 : 0,  // 下
		(ucMap & 0x08) ? 1 : 0); // 左

	// 检查是否有逻辑问题：如果当前方向前方有墙但地图显示可通行，说明有问题
	int currentFrontWall = 0;
	switch (GucMouseDir) {
	case UP: currentFrontWall = (ucMap & 0x01) ? 0 : 1; break;
	case RIGHT: currentFrontWall = (ucMap & 0x02) ? 0 : 1; break;
	case DOWN: currentFrontWall = (ucMap & 0x04) ? 0 : 1; break;
	case LEFT: currentFrontWall = (ucMap & 0x08) ? 0 : 1; break;
	}

	if (frontHasWall != currentFrontWall) {
		printf("!!! 逻辑不一致: 传感器前方=%d, 地图前方=%d !!!\n", frontHasWall, currentFrontWall);
	}

	// 更新相邻格子的墙壁信息
	if ((GucMapBlock[GmcMouse.cX][GmcMouse.cY] & 0x0f) == 0x00)
	{
		printf("检测到死路(0x00)，更新相邻格子墙壁信息\n");

		GucMapBlock[GmcMouse.cX][GmcMouse.cY] = ucMap;

		if (GmcMouse.cX > 0)
		{
			if ((GucMapBlock[GmcMouse.cX][GmcMouse.cY] & 0x08) == 0x00)
			{
				printf("更新左侧格子(%d,%d)的右墙\n", GmcMouse.cX - 1, GmcMouse.cY);
				GucMapBlock[GmcMouse.cX - 1][GmcMouse.cY] = ((GucMapBlock[GmcMouse.cX - 1][GmcMouse.cY]) & 0xdf);
			}
		}

		if (GmcMouse.cX < MAZETYPE - 1)
		{
			if ((GucMapBlock[GmcMouse.cX][GmcMouse.cY] & 0x02) == 0x00)
			{
				printf("更新右侧格子(%d,%d)的左墙\n", GmcMouse.cX + 1, GmcMouse.cY);
				GucMapBlock[GmcMouse.cX + 1][GmcMouse.cY] = ((GucMapBlock[GmcMouse.cX + 1][GmcMouse.cY]) & 0x7f);
			}
		}

		if (GmcMouse.cY > 0)
		{
			if ((GucMapBlock[GmcMouse.cX][GmcMouse.cY] & 0x04) == 0x00)
			{
				printf("更新下方格子(%d,%d)的上墙\n", GmcMouse.cX, GmcMouse.cY - 1);
				GucMapBlock[GmcMouse.cX][GmcMouse.cY - 1] = ((GucMapBlock[GmcMouse.cX][GmcMouse.cY - 1]) & 0xef);
			}
		}

		if (GmcMouse.cY < MAZETYPE - 1)
		{
			if ((GucMapBlock[GmcMouse.cX][GmcMouse.cY] & 0x01) == 0x00)
			{
				printf("更新上方格子(%d,%d)的下墙\n", GmcMouse.cX, GmcMouse.cY + 1);
				GucMapBlock[GmcMouse.cX][GmcMouse.cY + 1] = ((GucMapBlock[GmcMouse.cX][GmcMouse.cY + 1]) & 0xbf);
			}
		}
	}

	printf("=== updateMap 结束 ===\n\n");
}
// 右手法则选动作
void rightMethod(void)
{
	if ((GucMapBlock[GmcMouse.cX][GmcMouse.cY] & MOUSEWAY_R) &&
		(mazeIsSearched(MOUSERIGHT) == 0))
	{
		StepTurnRight();
		MoveOneBlock();
		return;
	}
	if ((GucMapBlock[GmcMouse.cX][GmcMouse.cY] & MOUSEWAY_F) &&
		(mazeIsSearched(MOUSEFRONT) == 0))
	{
		MoveOneBlock();
		return;
	}
	if ((GucMapBlock[GmcMouse.cX][GmcMouse.cY] & MOUSEWAY_L) &&
		(mazeIsSearched(MOUSELEFT) == 0))
	{
		StepTurnLeft();
		MoveOneBlock();
		return;
	}
	else
	{
		TurnBack();
		return;
	}
}
// 检查相对方向的迷宫格，是否搜索过
int mazeIsSearched(int  ucDirTemp)
{
	int cX = 0, cY = 0;

	switch (ucDirTemp) {

	case MOUSEFRONT:
		ucDirTemp = GucMouseDir;
		break;

	case MOUSELEFT:
		ucDirTemp = (GucMouseDir + 3) % 4;
		break;

	case MOUSERIGHT:
		ucDirTemp = (GucMouseDir + 1) % 4;
		break;

	default:
		break;
	}

	switch (ucDirTemp) {

	case 0:
		cX = GmcMouse.cX;
		cY = GmcMouse.cY + 1;
		break;

	case 1:
		cX = GmcMouse.cX + 1;
		cY = GmcMouse.cY;
		break;

	case 2:
		cX = GmcMouse.cX;
		cY = GmcMouse.cY - 1;
		break;

	case 3:
		cX = GmcMouse.cX - 1;
		cY = GmcMouse.cY;
		break;

	default:
		break;
	}

	return(GucMapGet[cX][cY]);
}
// 计算可前进方向数目
int crosswayCheck(char cX, char cY)
{
	int moveDirNum = 0;

	if ((GucMapBlock[cX][cY] & 0x01) &&
		(GucMapGet[cX][cY + 1] == 0)) {
		moveDirNum++;
	}
	if ((GucMapBlock[cX][cY] & 0x02) &&
		(GucMapGet[cX + 1][cY] == 0)) {
		moveDirNum++;
	}
	if ((GucMapBlock[cX][cY] & 0x04) &&
		(GucMapGet[cX][cY - 1] == 0)) {
		moveDirNum++;
	}
	if ((GucMapBlock[cX][cY] & 0x08) &&
		(GucMapGet[cX - 1][cY] == 0)) {
		moveDirNum++;
	}

	//printf("crosswayCheackNum(%d,%d) = %d\n", GmcMouse.cX, GmcMouse.cY,moveDirNum);

	return moveDirNum;
}
// 计算洪水数据
void mapStepEdit(char  cX, char  cY)
{
	uchar n = 0;
	uchar ucStep = 0;
	uchar ucStat = 0;
	uchar i, j;

	MAZECOOR GmcStack[MAZETYPE * MAZETYPE] = { 0 };

	GmcStack[n].cX = cX;
	GmcStack[n].cY = cY;
	n++;


	for (i = 0; i < MAZETYPE; i++)
	{
		for (j = 0; j < MAZETYPE; j++)
		{
			GucMapStep[i][j] = 255;
		}
	}

	while (n)
	{
		GucMapStep[cX][cY] = ucStep++;

		ucStat = 0;
		if ((GucMapBlock[cX][cY] & 0x01) && (GucMapStep[cX][cY + 1] > (ucStep)))
		{
			ucStat++;
		}
		if ((GucMapBlock[cX][cY] & 0x02) && (GucMapStep[cX + 1][cY] > (ucStep)))
		{
			ucStat++;
		}
		if ((GucMapBlock[cX][cY] & 0x04) && (GucMapStep[cX][cY - 1] > (ucStep)))
		{
			ucStat++;
		}
		if ((GucMapBlock[cX][cY] & 0x08) && (GucMapStep[cX - 1][cY] > (ucStep)))
		{
			ucStat++;
		}

		if (ucStat == 0)
		{
			n--;
			cX = GmcStack[n].cX;
			cY = GmcStack[n].cY;
			ucStep = GucMapStep[cX][cY];
		}
		else
		{
			if (ucStat > 1)
			{
				GmcStack[n].cX = cX;
				GmcStack[n].cY = cY;
				n++;
			}

			if ((GucMapBlock[cX][cY] & 0x01) && (GucMapStep[cX][cY + 1] > (ucStep)))
			{
				cY++;
				continue;
			}
			if ((GucMapBlock[cX][cY] & 0x02) && (GucMapStep[cX + 1][cY] > (ucStep)))
			{
				cX++;
				continue;
			}
			if ((GucMapBlock[cX][cY] & 0x04) && (GucMapStep[cX][cY - 1] > (ucStep)))
			{
				cY--;
				continue;
			}
			if ((GucMapBlock[cX][cY] & 0x08) && (GucMapStep[cX - 1][cY] > (ucStep)))
			{
				cX--;
				continue;
			}
		}
	}
}
// 根据最短路径引导至目标点（本次新增）
void objectGoTo(char  cXdst, char  cYdst)
{
	int ucStep = 1;
	char cNBlock = 0, cDirTemp = -1;
	char cX, cY;

	cX = GmcMouse.cX;
	cY = GmcMouse.cY;
	mapStepEdit(cXdst, cYdst);

	while ((cX != cXdst) || (cY != cYdst))
	{
		ucStep = GucMapStep[cX][cY];
		cDirTemp = -1;

		if ((GucMapBlock[cX][cY] & 0x01) && (GucMapStep[cX][cY + 1] == ucStep - 1))
		{
			cDirTemp = UP;
			if (cDirTemp == GucMouseDir)
			{
				cNBlock++;
				cY++;
				continue;
			}
		}
		if ((GucMapBlock[cX][cY] & 0x02) && (GucMapStep[cX + 1][cY] == ucStep - 1))
		{
			cDirTemp = RIGHT;
			if (cDirTemp == GucMouseDir)
			{
				cNBlock++;
				cX++;
				continue;
			}
		}
		if ((GucMapBlock[cX][cY] & 0x04) && (GucMapStep[cX][cY - 1] == ucStep - 1))
		{
			cDirTemp = DOWN;
			if (cDirTemp == GucMouseDir)
			{
				cNBlock++;
				cY--;
				continue;
			}
		}
		if ((GucMapBlock[cX][cY] & 0x08) && (GucMapStep[cX - 1][cY] == ucStep - 1))
		{
			cDirTemp = LEFT;
			if (cDirTemp == GucMouseDir)
			{
				cNBlock++;
				cX--;
				continue;
			}
		}

		cDirTemp = (cDirTemp + 4 - GucMouseDir) % 4;

		if (cNBlock)
		{
			for (int i = 0; i < cNBlock; i++)
			{
				MoveOneBlock();
			}

		}
		cNBlock = 0;

		switch (cDirTemp)
		{

		case 1:
			StepTurnRight();
			break;

		case 2:
			TurnBack();
			break;

		case 3:
			StepTurnLeft();
			break;

		default:
			break;
		}

		cX = GmcMouse.cX;
		cY = GmcMouse.cY;
	}
	if (cNBlock)
	{
		for (int i = 0; i < cNBlock; i++)
		{
			MoveOneBlock();
		}
	}
}
MAZECOOR findNearestUnexplored(void) {
	MAZECOOR bestTarget = { -1, -1 };
	int bestScore = -9999;

	for (int i = 0; i < MAZETYPE; i++) {
		for (int j = 0; j < MAZETYPE; j++) {
			if (GucMapGet[i][j] == 0) {
				int score = calculateUnexploredPriority(i, j);

				// 关键改进：考虑实际可达性，不仅仅是直线距离
				int manhattanDist = abs(i - GmcMouse.cX) + abs(j - GmcMouse.cY);

				// 惩罚被墙壁隔开的目标
				if (isBlockedByWall(GmcMouse.cX, GmcMouse.cY, i, j)) {
					score -= 50; // 大幅降低被墙壁隔开的目标的优先级
				}

				// 距离过远惩罚
				if (manhattanDist > 8) {
					score -= 20;
				}

				if (score > bestScore) {
					bestScore = score;
					bestTarget.cX = i;
					bestTarget.cY = j;
				}
			}
		}
	}

	if (bestTarget.cX != -1) {
		int distance = abs(bestTarget.cX - GmcMouse.cX) + abs(bestTarget.cY - GmcMouse.cY);
		printf("选择目标: (%d,%d), 分数=%d, 直线距离=%d\n",
			bestTarget.cX, bestTarget.cY, bestScore, distance);
	}

	return bestTarget;
}