#pragma once


//============================================
// 【第1部分】电脑鼠决策函数声明
//============================================
int userMain(void); // 函数名称不能修改
typedef unsigned char  uchar; // 数据类型
void updateMap(void);
void rightMethod(void);
int mazeIsSearched(int  ucDirTemp);
int crosswayCheck(char cX, char cY);
void mapStepEdit(char  cX, char  cY);
void objectGoTo(char  cXdst, char  cYdst);

//============================================
// 【第2部分】电脑鼠执行函数声明
//============================================
// 存储更新的墙壁信息(0-无墙，1-有墙)
extern int leftHasWall;
extern int frontHasWall;
extern int rightHasWall;
// 电脑鼠起止操作
void mouseInit(void);
void mouseEnd(void);
// 电脑鼠执行动作
void StepTurnRight(void);
void StepTurnLeft(void);
void TurnBack(void);
void MoveOneBlock(void);

// 电脑鼠地图记忆

// 迷宫类型
#define MAZETYPE		16	
#define MAX_PATH_LENGTH (MAZETYPE * MAZETYPE)

// 运行状态
#define  WAIT           0
#define  START          1
#define  MAZESEARCH     2
#define  SPURT          3
#define  END			4

// 绝对方向
#define UP				0
#define RIGHT			1
#define DOWN			2
#define LEFT			3

// 相对方向
#define MOUSELEFT		0
#define MOUSEFRONT		1
#define MOUSERIGHT		2

// 用绝对方向来表示相对方向上的资料表位置
#define MOUSEWAY_F		(1 <<   GucMouseDir)
#define MOUSEWAY_R		(1 << ((GucMouseDir + 1) % 4))
#define MOUSEWAY_B		(1 << ((GucMouseDir + 2) % 4))
#define MOUSEWAY_L		(1 << ((GucMouseDir + 3) % 4))

struct mazecoor	// 结构体
{
	int cX;
	int cY;
};
typedef struct mazecoor MAZECOOR;

extern MAZECOOR GmcMouse;							// 当前坐标
extern int GucMouseDir;								// 绝对方向
extern int GucMouseTask;							// 当前状态
extern uchar GucMapBlock[MAZETYPE][MAZETYPE];		// 墙壁信息
extern int GucMapGet[MAZETYPE][MAZETYPE];			// 已搜索迷宫

extern int GucMapStep[MAZETYPE][MAZETYPE];			// 洪水信息
extern MAZECOOR GmcCrossway[MAZETYPE * MAZETYPE];	// 岔路口信息
extern int crosswayNum;								// 岔路口索引


//====================================
void Spurt();
MAZECOOR findNearestUnexplored(void);