/**
 * @brief tinyOS的定时器实现
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

/**
 * @defgroup 软定时器 软定时器
 * @{
 */

#ifndef TTIMER_H
#define TTIMER_H

#include "tConfig.h"

#if !defined(TINYOS_ENABLE_TIMER) || TINYOS_ENABLE_TIMER == 1

#include "tEvent.h"

// 定时器状态
typedef enum _tTimerState {
    tTimerCreated,          /**< 定时器已经创建 */
    tTimerStarted,          /**< 定时器已经启动 */
    tTimerRunning,          /**< 定时器正在执行回调函数 */
    tTimerStopped,          /**< 定时器已经停止 */
    tTimerDestroyed         /**< 定时器已经销毁 */
} tTimerState;

// 软定时器结构
typedef struct _tTimer {
    tNode linkNode;                     /**< 链表结点 */

    uint32_t startDelayTicks;           /**< 初次启动延后的ticks数 */
    uint32_t durationTicks;             /**< 周期定时时的周期tick数 */
    uint32_t delayTicks;                /**< 当前定时递减计数值 */

    void (*timerFunc) (void *arg);      /**< 定时回调函数 */
    void *arg;                          /**< 传递给回调函数的参数 */

    uint32_t config;                    /**< 定时器配置参数 */
    tTimerState state;                  /**< 定时器状态 */
} tTimer;

// 软定时器状态信息
typedef struct _tTimerInfo {
    uint32_t startDelayTicks;           /**< 初次启动延后的ticks数 */
    uint32_t durationTicks;             /**< 周期定时时的周期tick数 */

    void (*timerFunc) (void *arg);      /**< 定时回调函数 */
    void *arg;                          /**< 传递给回调函数的参数 */

    uint32_t config;                    /**< 定时器配置参数 */
    tTimerState state;                  /**< 定时器状态 */
} tTimerInfo;

//!< 软硬定时器
#define TIMER_CONFIG_TYPE_HARD          (1 << 0)
#define TIMER_CONFIG_TYPE_SOFT          (0 << 0)

void tTimerInit (tTimer *timer, uint32_t delayTicks, uint32_t durationTicks,
                 void (*timerFunc) (void *arg), void *arg, uint32_t config);
void tTimerStart (tTimer *timer);

void tTimerStop (tTimer *timer);

void tTimerDestroy (tTimer *timer);
void tTimerGetInfo (tTimer *timer, tTimerInfo *info);
void tTimerModuleTickNotify (void);
void tTimerModuleInit (void);
void tTimerInitTask (void);
tTask * tTimerTask (void);

#endif // TINYOS_ENABLE_TIMER


#endif /* TTIMER_H */
/** @} */
