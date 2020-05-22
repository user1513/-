/**
 * @brief tinyOS的Hooks扩展接口
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

/**
 * @defgroup Hooks扩展 Hooks扩展
 * @{
 */

#ifndef THOOKS_H
#define THOOKS_H

#include "tinyOS.h"

/**
 * cpu空闲时的hooks
 */
void tHooksCpuIdle (void);

/**
 * 时钟节拍Hooks
 */
void tHooksSysTick (void);

/**
 * 任务切换hooks
 * @param from 从哪个任务开始切换
 * @param to 切换至哪个任务
 */
void tHooksTaskSwitch (tTask *from, tTask *to);

/**
 * 任务初始化的Hooks
 * @param task 等待初始化的任务
 */
void tHooksTaskInit (tTask *task);

#endif

/** @} */

