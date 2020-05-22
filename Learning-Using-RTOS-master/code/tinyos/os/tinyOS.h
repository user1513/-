/**
 * @brief tinyOS的核心头文件。包含了所有关键数据类型的定义，还有核心的函数。
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

/**
 * @defgroup 内核核心 内核核心
 * @{
 */

#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>
#include "tLib.h"
#include "tConfig.h"
#include "tTask.h"
#include "tEvent.h"
#include "tSem.h"
#include "tMBox.h"
#include "tMemBlock.h"
#include "tFlagGroup.h"
#include "tMutex.h"
#include "tTimer.h"
#include "tHooks.h"

#define TICKS_PER_SEC                   (1000 / TINYOS_SYSTICK_MS)

// tinyOS的错误码
typedef enum _tError {
    tErrorNoError = 0,                                  /**< 没有错误 */
    tErrorTimeout,                                      /**< 等待超时 */
    tErrorResourceUnavaliable,                          /**< 资源不可用 */
    tErrorDel,                                          /**< 被删除 */
    tErrorResourceFull,                                 /**< 资源缓冲区满 */
    tErrorOwner,                                        /**< 不匹配的所有者 */
} tError;

//! 当前任务：记录当前是哪个任务正在运行
extern tTask *currentTask;

//! 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
extern tTask *nextTask;

typedef uint32_t tTaskCritical_t;

tTaskCritical_t tTaskEnterCritical (void);
void tTaskExitCritical (tTaskCritical_t status);

void tTaskRunFirst (void);

void tTaskSwitch (void);
tTask *tTaskHighestReady (void);

void tTaskSchedInit (void);
void tTaskSchedDisable (void);
void tTaskSchedEnable (void);
void tTaskSchedRdy (tTask *task);
void tTaskSchedUnRdy (tTask *task);
void tTaskSchedRemove (tTask *task);
void tTaskSched (void);

void tTimeTaskWait (tTask *task, uint32_t ticks);
void tTimeTaskWakeUp (tTask *task);
void tTimeTaskRemove (tTask *task);
void tTaskDelay (uint32_t delay);
void tTaskSystemTickHandler (void);

void tInitApp (void);
void tSetSysTickPeriod (uint32_t ms);
float tCpuUsageGet (void);
tTask * tIdleTask (void);

#endif /* TINYOS_H */

/** @} */
