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

static tTaskStack task1Env[TASK1_ENV_SIZE];     // 任务1的堆栈空间
static tTaskStack task2Env[TASK2_ENV_SIZE];     // 任务2的堆栈空间
static tTaskStack task3Env[TASK3_ENV_SIZE];     // 任务3的堆栈空间
static tTaskStack task4Env[TASK4_ENV_SIZE];     // 任务4的堆栈空间

int task1Flag;           // 用于指示任务运行状态的标志变量
int task2Flag;           // 用于指示任务运行状态的标志变量
int task3Flag;           // 用于指示任务运行状态的标志变量
int task4Flag;           // 用于指示任务运行状态的标志变量

tList list;             // 中断与任务共享的链表
tNode nodes[20];        // 待插入结点的链表

/**
 * 初始化链表
 */
void listInit (void) {
    int i = 0;

    tListInit(&list);
    for (i = 0; i < sizeof(nodes) / sizeof(tNode); i++) {
        tListAddLast(&list, &nodes[i]);
    }
}

/**
 * 遍历并打开整个链表
 */
void listPrint (void) {
    uint32_t count = 0;
    tNode *currentNode = 0;

    // 遍历整个链表，然后打印出各个结点对应的序号
    for (currentNode = tListFirst(&list); currentNode != (tNode *) 0; currentNode = tListNext(&list, currentNode)) {
        xprintf("Node %d\n", count);

        // 假设在此期间，突然发生了中断。由于中断操作了链表，导致再次返回任务时
        // 头结点已经从链表中移除。此时，再继续后面的循环发现，currentNode = tListNext(&list, currentNode) 为0
        // 导致没有办法再扫描链表中后面的其它结点
        if (count++ == 0) {
            interruptByIRQ(IRQ_PRIO_HIGH);
        }
    }
}

/**
 * 高优先级中断处理函数
 */
void IRQHighHandler (void) {
    // 在中断中，可能会访问链表，然后对链表进行一些修改操作
    // 比如删除第1个。而如果此时任务恰好在访问第1个结点，则会链表访问混乱，出现错误
    tListRemoveFirst(&list);
}


/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task1Entry (void *param) {
    tTaskCritical_t critical;
    listInit();

    critical = tTaskEnterCritical();
    listPrint();
    tTaskExitCritical(critical);
    
    for (;;) {
        task1Flag = 1;
        tTaskDelay(1);
        task1Flag = 0;
        tTaskDelay(1);
    }
}

/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task2Entry (void *param) {
    for (;;) {
        task2Flag = 1;
        tTaskDelay(1);
        task2Flag = 0;
        tTaskDelay(1);
    }
}

/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task3Entry (void *param) {
    for (;;) {
        task3Flag = 1;
        tTaskDelay(1);
        task3Flag = 0;
        tTaskDelay(1);
    }
}


/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task4Entry (void *param) {
    for (;;) {
        task4Flag = 1;
        tTaskDelay(1);
        task4Flag = 0;
        tTaskDelay(1);
    }
}

/**
 * App的初始化
 */
void tInitApp (void) {
    halInit();

    interruptEnable(IRQ_PRIO_HIGH, 1);

    tTaskInit(&task1, task1Entry, (void *) 0x0, TASK1_PRIO, task1Env, sizeof(task1Env));
    tTaskInit(&task2, task2Entry, (void *) 0x0, TASK2_PRIO, task2Env, sizeof(task2Env));
    tTaskInit(&task3, task3Entry, (void *) 0x0, TASK3_PRIO, task3Env, sizeof(task3Env));
    tTaskInit(&task4, task4Entry, (void *) 0x0, TASK4_PRIO, task4Env, sizeof(task4Env));
}

