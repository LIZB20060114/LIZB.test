#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include"user.h"
#pragma comment(lib,"simulation.lib")
void communicationInit(void);
int main()
{
    // freopen("log.txt", "w", stdout);
    communicationInit(); //通信初始化
    printf("仿真算法加载完成！\n");
    if (userMain() == 1)
    {
        printf("仿真正常结束！\n");
    }
    else
    {
        printf("仿真异常结束！\n");
    }
    return 0;
}
