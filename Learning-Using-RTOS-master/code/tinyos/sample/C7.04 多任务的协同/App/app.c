/**
 * @brief tOS应用示例
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#include "tinyOS.h"
#include "app.h"
#include "hal.h"

static tTask task1;                     // 任务1结构
static tTask task2;                     // 任务2结构
static tTask task3;                     // 任务3结构
static tTask task4;                     // 任务4结构
static tTask task5;                     // 任务4结构
static tTask task6;                     // 任务4结构

static tTaskStack task1Env[TASK1_ENV_SIZE];     // 任务1的堆栈空间
static tTaskStack task2Env[TASK2_ENV_SIZE];     // 任务2的堆栈空间
static tTaskStack task3Env[TASK3_ENV_SIZE];     // 任务3的堆栈空间
static tTaskStack task4Env[TASK4_ENV_SIZE];     // 任务4的堆栈空间
static tTaskStack task5Env[TASK5_ENV_SIZE];     // 任务4的堆栈空间
static tTaskStack task6Env[TASK6_ENV_SIZE];     // 任务4的堆栈空间

int task1Flag;           // 用于指示任务运行状态的标志变量
int task2Flag;           // 用于指示任务运行状态的标志变量
int task3Flag;           // 用于指示任务运行状态的标志变量
int task4Flag;           // 用于指示任务运行状态的标志变量
int task5Flag;           // 用于指示任务运行状态的标志变量
int task6Flag;           // 用于指示任务运行状态的标志变量

tSem task1Sem;
tSem task3Sem;
tSem task4Sem;
tSem task5Sem;

typedef struct {
	int returnAck;
	int taskId;
}Task6Msg;

#define TASK6_MSG_NR 20

Task6Msg task6Msg[TASK6_MSG_NR];
tMemBlock task6MsgMemBlock;

tMbox task6Mbox;
void * task6MsgBuffer[TASK6_MSG_NR];

tMbox task6AckMbox;
void * task6AckMsgBuffer[TASK6_MSG_NR];

/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task1Entry (void *param) {
    for (;;) {
        task1Flag = 1;
        tTaskDelay(1);
        task1Flag = 0;
        tTaskDelay(1);
			
				tSemNotify(&task1Sem);
			
    }
}

/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task2Entry (void *param) {
		Task6Msg * msg;
	
    for (;;) {
        tSemWait(&task1Sem, 0);
			
				xprintf("Task2 Running...\n");
			
				tSemNotify(&task3Sem);
				tSemNotify(&task4Sem);
				tSemNotify(&task5Sem);
			
				tMemBlockWait(&task6MsgMemBlock, (void **)&msg, 0);
				msg->returnAck = 1;
				msg->taskId = 2;
				tMboxNotify(&task6Mbox, msg, tMBOXSendNormal);
			
				tMboxWait(&task6AckMbox, (void **)&msg, 0);
				xprintf("Task Get ACK!\n");
				tMemBlockNotify(&task6MsgMemBlock, msg);
    }
}

/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task3Entry (void *param) {
		Task6Msg * msg;
	
    for (;;) {
        tSemWait(&task3Sem, 0);
			
				xprintf("Task3 Running...\n");
			
				tMemBlockWait(&task6MsgMemBlock, (void *)&msg, 0);
				msg->returnAck = 0;
				msg->taskId = 3;
				tMboxNotify(&task6Mbox, msg, tMBOXSendNormal);
    }
}


/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task4Entry (void *param) {
		Task6Msg * msg;

		for (;;) {
        tSemWait(&task4Sem, 0);
			
				xprintf("Task4 Running...\n");

				tMemBlockWait(&task6MsgMemBlock, (void *)&msg, 0);
				msg->returnAck = 0;
				msg->taskId = 4;
				tMboxNotify(&task6Mbox, msg, tMBOXSendNormal);
    }
}

/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task5Entry (void *param) {
    for (;;) {
        tSemWait(&task3Sem, 0);
			
				xprintf("Task3 Running...\n");

    }
}

/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task6Entry (void *param) {
		Task6Msg * msg;
	
    for (;;) {
        tMboxWait(&task6Mbox, (void **)&msg, 0);
			
				xprintf("From task %d\n", msg->taskId);
			
				if (msg->returnAck) {
					msg->taskId = 6;
					msg->returnAck = 0;
					tMboxNotify(&task6AckMbox, msg, tMBOXSendNormal);
				} else {
					tMemBlockNotify(&task6MsgMemBlock, msg);
				}	
    }
}

/**
 * App的初始化
 */
void tInitApp (void) {
    halInit();

		tSemInit(&task1Sem, 0, 0);
			tSemInit(&task3Sem, 0, 0);
		tSemInit(&task4Sem, 0, 0);
		tSemInit(&task5Sem, 0, 0);

		tMemBlockInit(&task6MsgMemBlock, task6Msg, sizeof(Task6Msg), TASK6_MSG_NR);
		tMboxInit(&task6Mbox, task6MsgBuffer, TASK6_MSG_NR);
			tMboxInit(&task6AckMbox, task6AckMsgBuffer, TASK6_MSG_NR);

    tTaskInit(&task1, task1Entry, (void *) 0x0, TASK1_PRIO, task1Env, sizeof(task1Env));
    tTaskInit(&task2, task2Entry, (void *) 0x0, TASK2_PRIO, task2Env, sizeof(task2Env));
    tTaskInit(&task3, task3Entry, (void *) 0x0, TASK3_PRIO, task3Env, sizeof(task3Env));
    tTaskInit(&task4, task4Entry, (void *) 0x0, TASK4_PRIO, task4Env, sizeof(task4Env));
    tTaskInit(&task5, task5Entry, (void *) 0x0, TASK5_PRIO, task5Env, sizeof(task5Env));
    tTaskInit(&task6, task6Entry, (void *) 0x0, TASK6_PRIO, task6Env, sizeof(task6Env));
}

