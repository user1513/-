/**
 * @brief tinyOS的互斥信号量的实现
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

/**
 * @defgroup 互斥信号量 互斥信号量
 * @{
 */

#ifndef TMUTEX_H
#define TMUTEX_H

#include "tConfig.h"

#if !defined(TINYOS_ENABLE_MUTEX) || TINYOS_ENABLE_MUTEX == 1

#include "tEvent.h"

// 互斥信号量类型
typedef struct _tMutex {
    tEvent event;                   /**< 事件控制块 */

    uint32_t lockedCount;           /**< 已被锁定的次数 */
    tTask *owner;                   /**< 拥有者 */
    uint32_t ownerOriginalPrio;     /**< 拥有者原始的优先级 */
} tMutex;

// 互斥信号量查询结构
typedef struct _tMutexInfo {
    uint32_t taskCount;             /**< 等待的任务数量 */

    uint32_t ownerPrio;             /**< 拥有者任务的优先级 */
    uint32_t inheritedPrio;         /**< 继承优先级 */

    tTask *owner;                   /**< 当前信号量的拥有者 */
    uint32_t lockedCount;           /**< 锁定次数 */
} tMutexInfo;

void tMutexInit (tMutex *mutex);
uint32_t tMutexWait (tMutex *mutex, uint32_t waitTicks);
uint32_t tMutexNoWaitGet (tMutex *mutex);
uint32_t tMutexNotify (tMutex *mutex);
uint32_t tMutexDestroy (tMutex *mutex);
void tMutexGetInfo (tMutex *mutex, tMutexInfo *info);

#endif // TINYOS_ENABLE_MUTEX

#endif /* TMUTEX_H */

/** @} */
