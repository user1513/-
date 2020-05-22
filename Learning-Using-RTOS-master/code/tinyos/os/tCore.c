/**
 * @brief tinyOS 内核核心支持代码
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

/**
 * @defgroup 内核核心 内核核心
 * @{
 */

#include "tinyOS.h"
#include "stm32f10x.h"

tTask *currentTask;                             /**< 当前任务：记录当前是哪个任务正在运行 */
tTask *nextTask;                                /**< 下一个将即运行的任务： */
tTask *idleTask;                                /**< 空闲任务 */

tBitmap taskPrioBitmap;                         /**< 任务优先级的标记位置结构 */
tList taskTable[TINYOS_PRO_COUNT];              /**< 所有任务的指针数组 */

uint8_t schedLockCount;                         /**< 调度锁计数器 */
uint32_t tickCount;                             /**< 时钟节拍计数 */

tList tTaskDelayedList;                         /**< 延时队列 */

uint32_t idleCount;                             /**< 空闲任务计数 */
uint32_t idleMaxCount;                          /**< 空闲任务最大计数 */

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
static void initCpuUsageStat (void);
static void checkCpuUsage (void);
static void cpuUsageSyncWithSysTick (void);
#endif

/**
 * @brief 获取当前最高优先级且可运行的任务
 * @return 优先级最高的且可运行的任务
 */
tTask *tTaskHighestReady (void) {
    uint32_t highestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
    tNode *node = tListFirst(&taskTable[highestPrio]);
    return (tTask *) tNodeParent(node, tTask, linkNode);
}

/**
 * @brief 初始化调度器
 */
void tTaskSchedInit (void) {
    int i = 0;

    schedLockCount = 0;
    tBitmapInit(&taskPrioBitmap);
    for (i = 0; i < TINYOS_PRO_COUNT; i++) {
        tListInit(&taskTable[i]);
    }
}

/**
 * @brief 禁止任务调度
 */
void tTaskSchedDisable (void) {
    uint32_t status = tTaskEnterCritical();

    if (schedLockCount < 255) {
        schedLockCount++;
    }

    tTaskExitCritical(status);
}

/**
 * @brief 允许任务调度
 */
void tTaskSchedEnable (void) {
    uint32_t status = tTaskEnterCritical();

    if (schedLockCount > 0) {
        if (--schedLockCount == 0) {
            tTaskSched();
        }
    }

    tTaskExitCritical(status);
}

/**
 * @brief 将任务设置为就绪状态
 * @param task 等待设置为就绪状态的任务
 */
void tTaskSchedRdy (tTask *task) {
    tListAddLast(&taskTable[task->prio], &(task->linkNode));
    tBitmapSet(&taskPrioBitmap, task->prio);
}

/**
 * @brief 将任务从就绪列表中移除
 * @param task 等待取消就绪的任务
 */
void tTaskSchedUnRdy (tTask *task) {
    tListRemove(&taskTable[task->prio], &(task->linkNode));

    // 队列中可能存在多个任务。只有当没有任务时，才清除位图标记
    if (tListCount(&taskTable[task->prio]) == 0) {
        tBitmapClear(&taskPrioBitmap, task->prio);
    }
}

/**
 * @brief 将任务从就绪列表中移除
 * @param task    等待移除的任务
 */
void tTaskSchedRemove (tTask *task) {
    tListRemove(&taskTable[task->prio], &(task->linkNode));

    if (tListCount(&taskTable[task->prio]) == 0) {
        tBitmapClear(&taskPrioBitmap, task->prio);
    }
}

/**
 * @brief 任务调度接口。通过它来选择下一个具体的任务，然后切换至该任务运行。
 */
void tTaskSched (void) {
    tTask *tempTask;

    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = tTaskEnterCritical();

    // 如何调度器已经被上锁，则不进行调度，直接退bm
    if (schedLockCount > 0) {
        tTaskExitCritical(status);
        return;
    }

    // 找到优先级最高的任务。这个任务的优先级可能比当前低低
    // 但是当前任务是因为延时才需要切换，所以必须切换过去，也就是说不能再通过判断优先级来决定是否切换
    // 只要判断不是当前任务，就立即切换过去
    tempTask = tTaskHighestReady();
    if (tempTask != currentTask) {
        nextTask = tempTask;

#if TINYOS_ENABLE_HOOKS == 1
        tHooksTaskSwitch(currentTask, nextTask);
#endif
        tTaskSwitch();
    }

    // 退出临界区
    tTaskExitCritical(status);
}

/**
 * @brief 初始化任务延时机制
 */
void tTaskDelayedInit (void) {
    tListInit(&tTaskDelayedList);
}

/**
 * @brief 将任务加入延时队列中
 * @param task    需要延时的任务
 * @param ticks   延时的ticks
 */
void tTimeTaskWait (tTask *task, uint32_t ticks) {
    task->delayTicks = ticks;
    tListAddLast(&tTaskDelayedList, &(task->delayNode));
    task->state |= TINYOS_TASK_STATE_DELAYED;
}

/**
 * @brief 将延时的任务从延时队列中唤醒
 * @param task  需要唤醒的任务
 */
void tTimeTaskWakeUp (tTask *task) {
    // 2017-6-28 添加清0操作
    task->delayTicks = 0;

    tListRemove(&tTaskDelayedList, &(task->delayNode));
    task->state &= ~TINYOS_TASK_STATE_DELAYED;
}

/**
 * @brief 将延时的任务从延时队列中移除
 * @param task  需要移除的任务
 */
void tTimeTaskRemove (tTask *task) {
    tListRemove(&tTaskDelayedList, &(task->delayNode));
}

/**
 * @brief 初始化时钟节拍计数
 */
void tTimeTickInit (void) {
    tickCount = 0;
}

/**
 * @brief 系统时钟节拍处理
 */
void tTaskSystemTickHandler (void) {
    tNode *node;
    uint32_t count;

    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = tTaskEnterCritical();

    // 检查所有任务的delayTicks数，如果不0的话，减1。
    // 2017-9-2：修改访问方式，不涉及细节
    for (node = tListFirst(&tTaskDelayedList), count = tListCount(&tTaskDelayedList); count > 0; count--) {
        tTask *task = tNodeParent(node, tTask, delayNode);
        if (--task->delayTicks == 0) {
            // 如果任务还处于等待事件的状态，则将其从事件等待队列中唤醒
            if (task->waitEvent) {
                // 此时，消息为空，等待结果为超时
                tEventRemoveTask(task, (void *) 0, tErrorTimeout);
            }

            // 预先获取下一结点，否则下面的移除会造成问题
            node = tListNext(&tTaskDelayedList, node);

            // 将任务从延时队列中移除
            tTimeTaskWakeUp(task);

            // 将任务恢复到就绪状态
            tTaskSchedRdy(task);
        }
    }

    // 检查下当前任务的时间片是否已经到了
    if (--currentTask->slice == 0) {
        // 如果当前任务中还有其它任务的话，那么切换到下一个任务
        // 方法是将当前任务从队列的头部移除，插入到尾部
        // 这样后面执行tTaskSched()时就会从头部取出新的任务取出新的任务作为当前任务运行
        if (tListCount(&taskTable[currentTask->prio]) > 1) {
            tListRemoveFirst(&taskTable[currentTask->prio]);
            tListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));

        }
        // 2017-6-28 移出，调整tListCount > 1, 重置计数器
        currentTask->slice = TINYOS_SLICE_MAX;
    }

    // 节拍计数增加
    tickCount++;

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
    // 检查cpu使用率
    checkCpuUsage();
#endif

    // 退出临界区
    tTaskExitCritical(status);

#if TINYOS_ENABLE_TIMER == 1
    // 通知定时器模块节拍事件
    tTimerModuleTickNotify();
#endif

#if TINYOS_ENABLE_HOOKS == 1
    tHooksSysTick();
#endif

    // 这个过程中可能有任务延时完毕(delayTicks = 0)，进行一次调度。
    tTaskSched();
}

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1

static float cpuUsage;                      /**< cpu使用率统计 */
static uint32_t enableCpuUsageStat;         /**< 是否使能cpu统计 */

/**
 * @brief 初始化cpu统计
 */
static void initCpuUsageStat (void) {
    idleCount = 0;
    idleMaxCount = 0;
    cpuUsage = 0;
    enableCpuUsageStat = 0;
}

/**
 * @brief 检查cpu使用率
 */
static void checkCpuUsage (void) {
    // 与空闲任务的cpu统计同步
    if (enableCpuUsageStat == 0) {
        enableCpuUsageStat = 1;
        tickCount = 0;
        return;
    }

    if (tickCount == TICKS_PER_SEC) {
        // 统计最初1s内的最大计数值
        idleMaxCount = idleCount;
        idleCount = 0;

        // 计数完毕，开启调度器，允许切换到其它任务
        tTaskSchedEnable();
    } else if (tickCount % TICKS_PER_SEC == 0) {
        // 之后每隔1s统计一次，同时计算cpu利用率
        cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
        idleCount = 0;
    }
}

/**
 * @brief 为检查cpu使用率与系统时钟节拍同步
 */
static void cpuUsageSyncWithSysTick (void) {
    while (enableCpuUsageStat == 0) {
    }
}

/**
 * @brief 获取cpu使用率,已经计算为百分比
 */
float tCpuUsageGet (void) {
    float usage = 0;

    uint32_t status = tTaskEnterCritical();
    usage = cpuUsage;
    tTaskExitCritical(status);

    return usage;
}

#endif

tTask tTaskIdle;                                            /**< 空闲任务结构 */
tTaskStack idleTaskEnv[TINYOS_IDLETASK_STACK_SIZE];         /**< 空闲任务堆栈 */

/**
 * @brief 空闲任务
 * @param param 空闲任务的初始参数
 */
void idleTaskEntry (void *param) {
    // 禁止调度，防止后面在创建任务时切换到其它任务中去
    tTaskSchedDisable();

    // 初始化App相关配置
    tInitApp();

#if TINYOS_ENABLE_TIMER == 1
    // 初始化定时器任务
    tTimerInitTask();
#endif

    // 启动系统时钟节拍
    tSetSysTickPeriod(TINYOS_SYSTICK_MS);

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
    // 等待与时钟同步
    cpuUsageSyncWithSysTick();
#else
    // 开启调度器，允许切换到其它任务
    tTaskSchedEnable();
#endif

    for (;;) {
        uint32_t status = tTaskEnterCritical();
        idleCount++;
        tTaskExitCritical(status);

#if TINYOS_ENABLE_HOOKS == 1
        tHooksCpuIdle();
#endif
    }
}

/**
 * 获取空闲任务结构
 * @return 空闲任务结构
 */
tTask * tIdleTask (void) {
    return &tTaskIdle;
}


/**
 * @brief 系统入口，完成所有功能的创建，空闲任务的创建等功能
 * @return 0, 无用
 */
int main () {
    // 优先初始化tinyOS的核心功能
    tTaskSchedInit();

    // 初始化延时队列
    tTaskDelayedInit();

#if TINYOS_ENABLE_TIMER == 1
    // 初始化定时器模块
    tTimerModuleInit();
#endif

    // 初始化时钟节拍
    tTimeTickInit();

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
    // 初始化cpu统计
    initCpuUsageStat();
#endif

    // 创建空闲任务
    tTaskInit(&tTaskIdle, idleTaskEntry, (void *) 0, TINYOS_PRO_COUNT - 1, idleTaskEnv, sizeof(idleTaskEnv));

    // 这里，不再指定先运行哪个任务，而是自动查找最高优先级的任务运行
    nextTask = tTaskHighestReady();
    currentTask = (tTask *) 0;

    // 切换到nextTask， 这个函数永远不会返回
    tTaskRunFirst();
    return 0;
}

/** @} */
