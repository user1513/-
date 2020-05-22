/**
 * @brief tinyOS的计数信号量的实现
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

/**
 * @defgroup 信号量 计数信号量
 * @{
 */

#ifndef TSEM_H
#define TSEM_H

#include "tConfig.h"

#if !defined(TINYOS_ENABLE_SEM) || TINYOS_ENABLE_SEM == 1

#include "tEvent.h"

// 信号量类型
typedef struct _tSem {
    tEvent event;               /**< 事件控制块 */
    uint32_t count;             /**< 当前的计数 */
    uint32_t maxCount;          /**< 最大计数 */
} tSem;

// 信号量的信息类型
typedef struct _tSemInfo {
    uint32_t count;             /**< 当前信号量的计数 */
    uint32_t maxCount;          /**< 信号量允许的最大计 */
    uint32_t taskCount;         /**< 当前等待的任务计数 */
} tSemInfo;

void tSemInit (tSem *sem, uint32_t startCount, uint32_t maxCount);
uint32_t tSemWait (tSem *sem, uint32_t waitTicks);
uint32_t tSemNoWaitGet (tSem *sem);
void tSemNotify (tSem *sem);
void tSemGetInfo (tSem *sem, tSemInfo *info);
uint32_t tSemDestroy (tSem *sem);

#endif // TINYOS_ENABLE_SEM

#endif /* TSEM_H */

/** @} */
