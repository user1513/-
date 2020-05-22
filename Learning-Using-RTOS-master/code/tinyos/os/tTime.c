/**
 * @brief tinyOS的延时实现
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

/**
 * @defgroup 任务管理 任务管理
 * @{
 */

#include "tinyOS.h"

/**
 * 使当前任务进入延时状态
 * @param delay 延时多少个ticks
 */
void tTaskDelay (uint32_t delay) {
    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = tTaskEnterCritical();

    // 设置延时值，插入延时队列
    tTimeTaskWait(currentTask, delay);

    // 将任务从就绪表中移除
    tTaskSchedUnRdy(currentTask);

    // 然后进行任务切换，切换至另一个任务，或者空闲任务
    // delayTikcs会在时钟中断中自动减1.当减至0时，会切换回来继续运行。
    tTaskSched();

    // 退出临界区
    tTaskExitCritical(status);
}

/** @} */

