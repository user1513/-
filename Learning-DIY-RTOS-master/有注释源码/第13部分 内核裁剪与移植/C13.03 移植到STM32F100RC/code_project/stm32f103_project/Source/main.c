/*************************************** Copyright (c)******************************************************
** File name            :   main.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   主文件，包含应用代码
**
**--------------------------------------------------------------------------------------------------------
** Created by           :   01课堂 lishutong
** Created date         :   2016-06-01
** Version              :   1.0
** Descriptions         :   The original version
**
**--------------------------------------------------------------------------------------------------------
** Copyright            :   版权所有，禁止用于商业用途
** Author Blog          :   http://ilishutong.com
**********************************************************************************************************/
#include "tinyOS.h"

// 当前任务：记录当前是哪个任务正在运行
tTask * currentTask;

// 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
tTask * nextTask;

// 空闲任务
tTask * idleTask;

// 任务优先级的标记位置结构
tBitmap taskPrioBitmap;

// 所有任务的指针数组：简单起见，只使用两个任务
tList taskTable[TINYOS_PRO_COUNT];

// 调度锁计数器
uint8_t schedLockCount;

// 时钟节拍计数
uint32_t tickCount;

// 延时队列
tList tTaskDelayedList;

// 空闲任务计数与最大计数
uint32_t idleCount;
uint32_t idleMaxCount;

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
static void initCpuUsageStat (void);
static void checkCpuUsage (void);
static void cpuUsageSyncWithSysTick (void);
#endif

/**********************************************************************************************************
** Function name        :   tTaskHighestReady
** Descriptions         :   获取当前最高优先级且可运行的任务
** parameters           :   无
** Returned value       :   优先级最高的且可运行的任务
***********************************************************************************************************/
tTask * tTaskHighestReady (void) 
{
    uint32_t highestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
    tNode * node = tListFirst(&taskTable[highestPrio]);
    return (tTask *)tNodeParent(node, tTask, linkNode);
}

/**********************************************************************************************************
** Function name        :   初始化调度器
** Descriptions         :   无
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedInit (void)
{
    int i = 0;

    schedLockCount = 0;
    tBitmapInit(&taskPrioBitmap);
    for (i = 0; i < TINYOS_PRO_COUNT; i++)
    {
        tListInit(&taskTable[i]);
    }
}

/**********************************************************************************************************
** Function name        :   tTaskSchedDisable
** Descriptions         :   禁止任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedDisable (void) 
{
    uint32_t status = tTaskEnterCritical();

    if (schedLockCount < 255) 
    {
        schedLockCount++;
    }

    tTaskExitCritical(status);
}

/**********************************************************************************************************
** Function name        :   tTaskSchedEnable
** Descriptions         :   允许任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedEnable (void) 
{
    uint32_t status = tTaskEnterCritical();

    if (schedLockCount > 0) 
    {
        if (--schedLockCount == 0) 
        {
            tTaskSched(); 
        }
    }

    tTaskExitCritical(status);
}

/**********************************************************************************************************
** Function name        :   tTaskSchedRdy
** Descriptions         :   将任务设置为就绪状态
** input parameters     :   task    等待设置为就绪状态的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedRdy (tTask * task)
{
    tListAddLast(&taskTable[task->prio], &(task->linkNode));
    tBitmapSet(&taskPrioBitmap, task->prio);
}

/************************************************************************************************************ Function name        :   tSchedulerUnRdyTask
** Descriptions         :   tTaskSchedUnRdy
** Descriptions         :   将任务从就绪列表中移除
** input parameters     :   task    ÒªÒÆ³ýµÄÈÎÎñ¿é
** output parameters    :   None
** Returned value       :   None
***********************************************************************************************************/
void tTaskSchedUnRdy (tTask * task)
{
    tListRemove(&taskTable[task->prio], &(task->linkNode));

    // 队列中可能存在多个任务。只有当没有任务时，才清除位图标记
    if (tListCount(&taskTable[task->prio]) == 0) 
    {
        tBitmapClear(&taskPrioBitmap, task->prio);
    }
}

/************************************************************************************************************ Function name        :   tSchedulerUnRdyTask
** Descriptions         :   tTaskSchedRemove
** Descriptions         :   将任务从就绪列表中移除
** input parameters     :   task    等待移除的任务
** output parameters    :   None
** Returned value       :   None
***********************************************************************************************************/
void tTaskSchedRemove (tTask * task)
{
    tListRemove(&taskTable[task->prio], &(task->linkNode));
	
    if (tListCount(&taskTable[task->prio]) == 0) 
    {
        tBitmapClear(&taskPrioBitmap, task->prio);
    }
}

/**********************************************************************************************************
** Function name        :   tTaskSched
** Descriptions         :   任务调度接口。tinyOS通过它来选择下一个具体的任务，然后切换至该任务运行。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSched (void) 
{   
    tTask * tempTask;

    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = tTaskEnterCritical();

    // 如何调度器已经被上锁，则不进行调度，直接退bm
    if (schedLockCount > 0) 
    {
        tTaskExitCritical(status);
        return;
    }

    // 找到优先级最高的任务。这个任务的优先级可能比当前低低
    // 但是当前任务是因为延时才需要切换，所以必须切换过去，也就是说不能再通过判断优先级来决定是否切换
    // 只要判断不是当前任务，就立即切换过去
    tempTask = tTaskHighestReady();
    if (tempTask != currentTask) 
    {
        nextTask = tempTask;

#if TINYOS_ENABLE_HOOKS == 1
        tHooksTaskSwitch(currentTask, nextTask);
#endif
        tTaskSwitch();   
    }

    // 退出临界区
    tTaskExitCritical(status); 
}

/**********************************************************************************************************
** Function name        :   tTaskDelayedInit
** Descriptions         :   初始化任务延时机制
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskDelayedInit (void) 
{
    tListInit(&tTaskDelayedList);
}

/**********************************************************************************************************
** Function name        :   tTimeTaskWait
** Descriptions         :   将任务加入延时队列中
** input parameters     :   task    需要延时的任务
**                          ticks   延时的ticks
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTaskWait (tTask * task, uint32_t ticks)
{
    task->delayTicks = ticks;
    tListAddLast(&tTaskDelayedList, &(task->delayNode)); 
    task->state |= TINYOS_TASK_STATE_DELAYED;
}

/**********************************************************************************************************
** Function name        :   tTimeTaskWakeUp
** Descriptions         :   将延时的任务从延时队列中唤醒
** input parameters     :   task  需要唤醒的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTaskWakeUp (tTask * task)
{
    tListRemove(&tTaskDelayedList, &(task->delayNode));
    task->state &= ~TINYOS_TASK_STATE_DELAYED;
}

/**********************************************************************************************************
** Function name        :   tTimeTaskRemove
** Descriptions         :   将延时的任务从延时队列中移除
** input parameters     :   task  需要移除的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTaskRemove (tTask * task)
{
    tListRemove(&tTaskDelayedList, &(task->delayNode));
}


/**********************************************************************************************************
** Function name        :   tTimeTickInit
** Descriptions         :   初始化时钟节拍计数
** input parameters     :   无
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTickInit (void)
{
    tickCount = 0;
}

/**********************************************************************************************************
** Function name        :   tTaskSystemTickHandler
** Descriptions         :   系统时钟节拍处理。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSystemTickHandler (void) 
{
    tNode * node;
    
    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = tTaskEnterCritical();
    
    // 检查所有任务的delayTicks数，如果不0的话，减1。
    for (node = tTaskDelayedList.headNode.nextNode; node != &(tTaskDelayedList.headNode); node = node->nextNode)
    {
        tTask * task = tNodeParent(node, tTask, delayNode);
        if (--task->delayTicks == 0) 
        {
            // 如果任务还处于等待事件的状态，则将其从事件等待队列中唤醒
            if (task->waitEvent) 
            {
                // 此时，消息为空，等待结果为超时
                tEventRemoveTask(task, (void *)0, tErrorTimeout);
            }

            // 将任务从延时队列中移除
            tTimeTaskWakeUp(task);

            // 将任务恢复到就绪状态
            tTaskSchedRdy(task);            
        }
    }

    // 检查下当前任务的时间片是否已经到了
    if (--currentTask->slice == 0) 
    {
        // 如果当前任务中还有其它任务的话，那么切换到下一个任务
        // 方法是将当前任务从队列的头部移除，插入到尾部
        // 这样后面执行tTaskSched()时就会从头部取出新的任务取出新的任务作为当前任务运行
        if (tListCount(&taskTable[currentTask->prio]) > 0) 
        {            
            tListRemoveFirst(&taskTable[currentTask->prio]);
            tListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));
       
            // 重置计数器
            currentTask->slice = TINYOS_SLICE_MAX;
        }
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

static float cpuUsage;                      // cpu使用率统计
static uint32_t enableCpuUsageStat;         // 是否使能cpu统计

/**********************************************************************************************************
** Function name        :   initCpuUsageStat
** Descriptions         :   初始化cpu统计
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void initCpuUsageStat (void)
{
    idleCount = 0;
    idleMaxCount = 0;
    cpuUsage = 0;
    enableCpuUsageStat = 0;
}

/**********************************************************************************************************
** Function name        :   checkCpuUsage
** Descriptions         :   检查cpu使用率
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void checkCpuUsage (void)
{
    // 与空闲任务的cpu统计同步
    if (enableCpuUsageStat == 0)
    {
        enableCpuUsageStat = 1;
        tickCount = 0;
        return;
    }

    if (tickCount == TICKS_PER_SEC)
    {
        // 统计最初1s内的最大计数值
        idleMaxCount = idleCount;
        idleCount = 0;

        // 计数完毕，开启调度器，允许切换到其它任务
        tTaskSchedEnable();
    }
    else if (tickCount % TICKS_PER_SEC == 0)
    {
        // 之后每隔1s统计一次，同时计算cpu利用率
        cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
        idleCount = 0;
    }
}

/**********************************************************************************************************
** Function name        :   cpuUsageSyncWithSysTick
** Descriptions         :   为检查cpu使用率与系统时钟节拍同步
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void cpuUsageSyncWithSysTick (void)
{
    // 等待与时钟节拍同步
    while (enableCpuUsageStat == 0)
    {
        ;;
    }
}

/**********************************************************************************************************
** Function name        :   cpuUsageSyncWithSysTick
** Descriptions         :   为检查cpu使用率与系统时钟节拍同步
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
float tCpuUsageGet (void)
{
    float usage = 0;

    uint32_t status = tTaskEnterCritical();
    usage = cpuUsage;
    tTaskExitCritical(status);

    return usage;
}

#endif

// 用于空闲任务的任务结构和堆栈空间
tTask tTaskIdle;
tTaskStack idleTaskEnv[TINYOS_IDLETASK_STACK_SIZE];

void idleTaskEntry (void * param) {
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

    for (;;)
    {
        uint32_t status = tTaskEnterCritical();
        idleCount++;
        tTaskExitCritical(status);

#if TINYOS_ENABLE_HOOKS == 1
        tHooksCpuIdle();
#endif
    }
}

int main () 
{
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
    tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINYOS_PRO_COUNT - 1, idleTaskEnv, TINYOS_IDLETASK_STACK_SIZE);
    
    // 这里，不再指定先运行哪个任务，而是自动查找最高优先级的任务运行
    nextTask = tTaskHighestReady();

    // 切换到nextTask， 这个函数永远不会返回
    tTaskRunFirst();
    return 0;
}
