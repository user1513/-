/*************************************** Copyright (c)******************************************************
** File name            :   tinyOS.h
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的核心头文件。包含了所有关键数据类型的定义，还有核心的函数。
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
#ifndef TINYOS_H
#define TINYOS_H

// 标准头文件，里面包含了常用的类型定义，如uint32_t
#include <stdint.h>

// tinyOS的内核库文件
#include "tLib.h"

// TinyOS的配置文件
#include "tConfig.h"

// 任务管理头文件
#include "tTask.h"

// 事件控制头文件
#include "tEvent.h"

// 信号量头文件
#include "tSem.h"

// 邮箱头文件
#include "tMBox.h"

// 存储块头文件
#include "tMemBlock.h"

// 事件标志组头文件
#include "tFlagGroup.h"

// 互斥体头文件
#include "tMutex.h"

// 软定时器
#include "tTimer.h"

// Hooks扩展
#include "tHooks.h"

#define TICKS_PER_SEC                   (1000 / TINYOS_SYSTICK_MS)

// tinyOS的错误码
typedef enum _tError {
    tErrorNoError = 0,                              // 没有错误
    tErrorTimeout,                                  // 等待超时
    tErrorResourceUnavaliable,                 		// 资源不可用
    tErrorDel,										// 被删除
    tErrorResourceFull,								// 资源缓冲区满
    tErrorOwner,                                    // 不匹配的所有者
}tError;

// 当前任务：记录当前是哪个任务正在运行
extern tTask * currentTask;

// 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
extern tTask * nextTask;

/**********************************************************************************************************
** Function name        :   tTaskEnterCritical
** Descriptions         :   进入临界区
** parameters           :   无
** Returned value       :   进入之前的临界区状态值
***********************************************************************************************************/
uint32_t tTaskEnterCritical (void);

/**********************************************************************************************************
** Function name        :   tTaskExitCritical
** Descriptions         :   退出临界区,恢复之前的临界区状态
** parameters           :   status 进入临界区之前的CPU
** Returned value       :   进入临界区之前的临界区状态值
***********************************************************************************************************/
void tTaskExitCritical (uint32_t status);

/**********************************************************************************************************
** Function name        :   tTaskRunFirst
** Descriptions         :   在启动tinyOS时，调用该函数，将切换至第一个任务运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskRunFirst (void); 

/**********************************************************************************************************
** Function name        :   tTaskSwitch
** Descriptions         :   进行一次任务切换，tinyOS会预先配置好currentTask和nextTask, 然后调用该函数，切换至
**                          nextTask运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSwitch (void);

/**********************************************************************************************************
** Function name        :   tTaskHighestReady
** Descriptions         :   获取当前最高优先级且可运行的任务
** parameters           :   无
** Returned value       :   优先级最高的且可运行的任务
***********************************************************************************************************/
tTask * tTaskHighestReady (void);

/**********************************************************************************************************
** Function name        :   初始化调度器
** Descriptions         :   无
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedInit (void);

/**********************************************************************************************************
** Function name        :   tTaskSchedDisable
** Descriptions         :   禁止任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedDisable (void);

/**********************************************************************************************************
** Function name        :   tTaskSchedEnable
** Descriptions         :   允许任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedEnable (void);

/**********************************************************************************************************
** Function name        :   tTaskSchedRdy
** Descriptions         :   将任务设置为就绪状态
** input parameters     :   task    等待设置为就绪状态的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedRdy (tTask * task);

/************************************************************************************************************ Function name        :   tSchedulerUnRdyTask
** Descriptions         :   tTaskSchedUnRdy
** Descriptions         :   将任务从就绪列表中移除
** input parameters     :   task    ÒªÒÆ³ýµÄÈÎÎñ¿é
** output parameters    :   None
** Returned value       :   None
***********************************************************************************************************/
void tTaskSchedUnRdy (tTask * task);

/************************************************************************************************************ Function name        :   tSchedulerUnRdyTask
** Descriptions         :   tTaskSchedRemove
** Descriptions         :   将任务从就绪列表中移除
** input parameters     :   task    等待移除的任务
** output parameters    :   None
** Returned value       :   None
***********************************************************************************************************/
void tTaskSchedRemove (tTask * task);

/**********************************************************************************************************
** Function name        :   tTaskSched
** Descriptions         :   任务调度接口。tinyOS通过它来选择下一个具体的任务，然后切换至该任务运行。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSched (void);

/**********************************************************************************************************
** Function name        :   tTimeTaskWait
** Descriptions         :   将任务加入延时队列中
** input parameters     :   task    需要延时的任务
**                          ticks   延时的ticks
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTaskWait (tTask * task, uint32_t ticks);

/**********************************************************************************************************
** Function name        :   tTimeTaskWakeUp
** Descriptions         :   将延时的任务从延时队列中唤醒
** input parameters     :   task  需要唤醒的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTaskWakeUp (tTask * task);

/**********************************************************************************************************
** Function name        :   tTimeTaskRemove
** Descriptions         :   将延时的任务从延时队列中移除
** input parameters     :   task  需要移除的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTaskRemove (tTask * task);

/**********************************************************************************************************
** Function name        :   tTaskDelay
** Descriptions         :   使当前任务进入延时状态。
** parameters           :   delay 延时多少个ticks
** Returned value       :   无
***********************************************************************************************************/
void tTaskDelay (uint32_t delay);

/**********************************************************************************************************
** Function name        :   tTaskSystemTickHandler
** Descriptions         :   系统时钟节拍处理。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSystemTickHandler (void);

/**********************************************************************************************************
** Function name        :   tInitApp
** Descriptions         :   初始化应用接口
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tInitApp (void);

/*********************************************************************************************************
** 系统时钟节拍定时器System Tick配置
** 在我们目前的环境（模拟器）中，系统时钟节拍为12MHz
** 请务必按照本教程推荐配置，否则systemTick的值就会有变化，需要查看数据手册才了解
**********************************************************************************************************/
void tSetSysTickPeriod(uint32_t ms);

/**********************************************************************************************************
** Function name        :   cpuUsageSyncWithSysTick
** Descriptions         :   为检查cpu使用率与系统时钟节拍同步
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
float tCpuUsageGet (void);

#endif /* TINYOS_H */
