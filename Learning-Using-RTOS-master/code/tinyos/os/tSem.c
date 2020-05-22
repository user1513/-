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

#include "tSem.h"
#include "tinyOS.h"

#if !defined(TINYOS_ENABLE_SEM) || TINYOS_ENABLE_SEM == 1

/**
 * 初始化信号量
 * @param startCount 初始的计数
 * @param maxCount 最大计数，如果为0，则不限数量
 */
void tSemInit (tSem *sem, uint32_t startCount, uint32_t maxCount) {
    tEventInit(&sem->event, tEventTypeSem);

    sem->maxCount = maxCount;
    if (maxCount == 0) {
        sem->count = startCount;
    } else {
        sem->count = (startCount > maxCount) ? maxCount : startCount;
    }
}

/**
 * 等待信号量
 * @param sem 等待的信号量
 * @param waitTicks 当信号量计数为0时，等待的ticks数，为0时表示永远等待
 * @return 等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
 */
uint32_t tSemWait (tSem *sem, uint32_t waitTicks) {
    uint32_t status = tTaskEnterCritical();

    // 首先检查信号量计数是否大于0
    if (sem->count > 0) {
        // 如果大于0的话，消耗掉一个，然后正常退出
        --sem->count;
        tTaskExitCritical(status);
        return tErrorNoError;
    } else {
        // 然后将任务插入事件队列中
        tEventWait(&sem->event, currentTask, (void *) 0, tEventTypeSem, waitTicks);
        tTaskExitCritical(status);

        // 最后再执行一次事件调度，以便于切换到其它任务
        tTaskSched();

        // 当由于等待超时或者计数可用时，执行会返回到这里，然后取出等待结构
        return currentTask->waitEventResult;
    }
}

/**
 * 获取信号量，如果信号量计数不可用，则立即退回
 * @param sem 等待的信号量
 * @return 获取结果, tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
 */
uint32_t tSemNoWaitGet (tSem *sem) {
    uint32_t status = tTaskEnterCritical();

    // 首先检查信号量计数是否大于0
    if (sem->count > 0) {
        // 如果大于0的话，消耗掉一个，然后正常退出
        --sem->count;
        tTaskExitCritical(status);
        return tErrorNoError;
    } else {
        // 否则，返回资源不可用
        tTaskExitCritical(status);
        return tErrorResourceUnavaliable;
    }
}

/**
 * 通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
 * @param sem 操作的信号量
 */
void tSemNotify (tSem *sem) {
    uint32_t status = tTaskEnterCritical();

    // 检查是否有任务等待
    if (tEventWaitCount(&sem->event) > 0) {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        tTask *task = tEventWakeUp(&sem->event, (void *) 0, tErrorNoError);

        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio) {
            tTaskSched();
        }
    } else {
        // 如果没有任务等待的话，增加计数
        ++sem->count;

        // 如果这个计数超过了最大允许的计数，则递减
        if ((sem->maxCount != 0) && (sem->count > sem->maxCount)) {
            sem->count = sem->maxCount;
        }
    }

    tTaskExitCritical(status);
}

/**
 * 查询信号量的状态信息
 * @param sem 查询的信号量
 * @param info 状态查询存储的位置
 */
void tSemGetInfo (tSem *sem, tSemInfo *info) {
    uint32_t status = tTaskEnterCritical();

    // 拷贝需要的信息
    info->count = sem->count;
    info->maxCount = sem->maxCount;
    info->taskCount = tEventWaitCount(&sem->event);

    tTaskExitCritical(status);
}

/**
 * 销毁信号量
 * @param sem 需要销毁的信号量
 * @return 因销毁该信号量而唤醒的任务数量
 */
uint32_t tSemDestroy (tSem *sem) {
    uint32_t status = tTaskEnterCritical();

    // 清空事件控制块中的任务
    uint32_t count = tEventRemoveAll(&sem->event, (void *) 0, tErrorDel);
    sem->count = 0;
    tTaskExitCritical(status);

    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0) {
        tTaskSched();
    }
    return count;
}

#endif

/** @} */
