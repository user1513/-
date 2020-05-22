/**
 * @brief tinyOS的邮箱实现
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

/**
 * @defgroup 邮箱 邮箱
 * @{
 */

#include "tMBox.h"
#include "tinyOS.h"

#if !defined(TINYOS_ENABLE_MBOX) || TINYOS_ENABLE_MBOX == 1

/**
 * 初始化邮箱
 * @param mbox 等待初始化的邮箱
 * @param msgBuffer 消息存储缓冲区
 * @param maxCount 最大计数
 */
void tMboxInit (tMbox *mbox, void **msgBuffer, uint32_t maxCount) {
    tEventInit(&mbox->event, tEventTypeMbox);

    mbox->msgBuffer = msgBuffer;
    mbox->maxCount = maxCount;
    mbox->read = 0;
    mbox->write = 0;
    mbox->count = 0;
}

/**
 * 等待邮箱, 获取一则消息
 * @param mbox 等待的邮箱
 * @param msg 消息存储缓存区
 * @param waitTicks 最大等待的ticks数，为0表示无限等待
 * @return 等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
 */
uint32_t tMboxWait (tMbox *mbox, void **msg, uint32_t waitTicks) {
    uint32_t status = tTaskEnterCritical();

    // 首先检查消息计数是否大于0
    if (mbox->count > 0) {
        // 如果大于0的话，取出一个
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];

        // 同时读取索引前移，如果超出边界则回绕
        if (mbox->read >= mbox->maxCount) {
            mbox->read = 0;
        }
        tTaskExitCritical(status);
        return tErrorNoError;
    } else {
        // 然后将任务插入事件队列中
        tEventWait(&mbox->event, currentTask, (void *) 0, tEventTypeMbox, waitTicks);
        tTaskExitCritical(status);

        // 最后再执行一次事件调度，以便于切换到其它任务
        tTaskSched();

        // 当切换回来时，从tTask中取出获得的消息
        *msg = currentTask->eventMsg;

        // 取出等待结果
        return currentTask->waitEventResult;
    }
}

/**
 * 获取一则消息，如果没有消息，则立即退回
 * @param mbox 获取消息的邮箱
 * @param msg 消息存储缓存区
 * @return 获取结果, tErrorResourceUnavaliable.tErrorNoError
 */
uint32_t tMboxNoWaitGet (tMbox *mbox, void **msg) {
    uint32_t status = tTaskEnterCritical();

    // 首先检查消息计数是否大于0
    if (mbox->count > 0) {
        // 如果大于0的话，取出一个
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];

        // 同时读取索引前移，如果超出边界则回绕
        if (mbox->read >= mbox->maxCount) {
            mbox->read = 0;
        }
        tTaskExitCritical(status);
        return tErrorNoError;
    } else {
        // 否则，返回资源不可用
        tTaskExitCritical(status);
        return tErrorResourceUnavaliable;
    }
}

/**
 * 通知消息可用，唤醒等待队列中的一个任务，或者将消息插入到邮箱中
 * @param mbox 操作的信号量
 * @param msg 发送的消息
 * @param notifyOption 发送的选项
 * @return tErrorNoError，或者当邮箱满时，返回tErrorResourceFull
 */
uint32_t tMboxNotify (tMbox *mbox, void *msg, uint32_t notifyOption) {
    uint32_t status = tTaskEnterCritical();

    // 检查是否有任务等待
    if (tEventWaitCount(&mbox->event) > 0) {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        tTask *task = tEventWakeUp(&mbox->event, (void *) msg, tErrorNoError);

        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio) {
            tTaskSched();
        }
    } else {
        // 如果没有任务等待的话，将消息插入到缓冲区中
        if (mbox->count >= mbox->maxCount) {
            tTaskExitCritical(status);
            return tErrorResourceFull;
        }

        // 可以选择将消息插入到头，这样后面任务获取的消息的时候，优先获取该消息
        if (notifyOption & tMBOXSendFront) {
            if (mbox->read <= 0) {
                mbox->read = mbox->maxCount - 1;
            } else {
                --mbox->read;
            }
            mbox->msgBuffer[mbox->read] = msg;
        } else {
            mbox->msgBuffer[mbox->write++] = msg;
            if (mbox->write >= mbox->maxCount) {
                mbox->write = 0;
            }
        }

        // 增加消息计数
        mbox->count++;
    }

    tTaskExitCritical(status);
    return tErrorNoError;
}

/**
 * 清空邮箱中所有消息
 * @param mbox 等待清空的邮箱
 */
void tMboxFlush (tMbox *mbox) {
    uint32_t status = tTaskEnterCritical();

    // 如果队列中有任务等待，说明邮箱已经是空的了，不需要再清空
    if (tEventWaitCount(&mbox->event) == 0) {
        mbox->read = 0;
        mbox->write = 0;
        mbox->count = 0;
    }

    tTaskExitCritical(status);
}

/**
 * 销毁邮箱
 * @param mbox 需要销毁的邮箱
 * @return 因销毁该信号量而唤醒的任务数量
 */
uint32_t tMboxDestroy (tMbox *mbox) {
    uint32_t status = tTaskEnterCritical();

    // 清空事件控制块中的任务
    uint32_t count = tEventRemoveAll(&mbox->event, (void *) 0, tErrorDel);

    tTaskExitCritical(status);

    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0) {
        tTaskSched();
    }
    return count;
}

/**
 * 查询状态信息
 * @param mbox 查询的邮箱
 * @param info 状态查询存储的位置
 */
void tMboxGetInfo (tMbox *mbox, tMboxInfo *info) {
    uint32_t status = tTaskEnterCritical();

    // 拷贝需要的信息
    info->count = mbox->count;
    info->maxCount = mbox->maxCount;
    info->taskCount = tEventWaitCount(&mbox->event);

    tTaskExitCritical(status);
}

#endif // TINYOS_ENABLE_MBOX

/** @} */


