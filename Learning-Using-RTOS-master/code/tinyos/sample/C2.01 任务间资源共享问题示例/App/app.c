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

static uint32_t shareCount = 0;                  // 全局共享计数器

/**
 * 初始化共享计数器
 * @param count 初始的计数器
 */
void shareCountInit (uint32_t count) {
    shareCount = count;
}

/**
 * 增加共享计数器值
 * @param count 增加的数量
 */
static uint32_t realCount = 0;
void shareCountIncrease (uint32_t count, int interrupt) {
    realCount += count;

    if (interrupt == 0) {
        shareCount += count;
    } else {
        // 以下展开，以便模拟汇编指令被打断的情况
        uint32_t var = shareCount;
        var = var + count;
        interruptByOtherTask();     // 模拟被其它任务打断
        shareCount = var;
    }
}

/**
 * 获取共享计数器值
 * @return 当前计数值
 */
uint32_t shareCountGet (void) {
    return shareCount;
}


/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task1Entry (void *param) {
    uint32_t before;

#if 1
    shareCountIncrease(1, 1);
    xprintf("Task1 S1:%d/%d\n", shareCountGet(), realCount);
#else
    before = shareCountGet();
    shareCountIncrease(1, 0);
    interruptByOtherTask();     // 如果在这期间被打断
    xprintf("Task1 S2:%d/%d\n", before + 1, realCount);
#endif
    for (;;) {
        tTaskDelay(1);
    }
}

/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task2Entry (void *param) {
    uint32_t before;

#if 1
    // task2的修改会被覆盖
    shareCountIncrease(1, 0);
    xprintf("Task2 S1:%d/%d\n", shareCountGet(), realCount);
#else
    before = shareCountGet();
    shareCountIncrease(1, 0);
    xprintf("Task2 S2:%d/%d\n", before + 1, realCount);
#endif
    for (;;) {
        tTaskDelay(10);
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

    shareCountInit(0);

    tTaskInit(&task1, task1Entry, (void *) 0x0, TASK1_PRIO, task1Env, sizeof(task1Env));
    tTaskInit(&task2, task2Entry, (void *) 0x0, TASK2_PRIO, task2Env, sizeof(task2Env));
    tTaskInit(&task3, task3Entry, (void *) 0x0, TASK3_PRIO, task3Env, sizeof(task3Env));
    tTaskInit(&task4, task4Entry, (void *) 0x0, TASK4_PRIO, task4Env, sizeof(task4Env));
}

