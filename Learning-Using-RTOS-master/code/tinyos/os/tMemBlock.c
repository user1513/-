/**
 * @brief tinyOS的存储块的实现
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

/**
 * @defgroup 存储块 存储块
 * @{
 */

#include "tMemBlock.h"
#include "tinyOS.h"

#if !defined(TINYOS_ENABLE_MEMBLOCK) || TINYOS_ENABLE_MEMBLOCK == 1

/**
 * 初始化存储控制块
 * @param memBlock 等待初始化的存储控制块
 * @param memStart 存储区的起始地址
 * @param blockSize 每个块的大小
 * @param blockCnt 总的块数量
 * @return 唤醒的任务数量
 */
void tMemBlockInit (tMemBlock *memBlock, void *memStart, uint32_t blockSize, uint32_t blockCnt) {
    uint8_t *memBlockStart = (uint8_t *) memStart;
    uint8_t *memBlockEnd = memBlockStart + blockSize * blockCnt;

    // 每个存储块需要来放置链接指针，所以空间至少要比tNode大
    // 即便如此，实际用户可用的空间并没有少
    if (blockSize < sizeof(tSnode)) {
        return;
    }

    tEventInit(&memBlock->event, tEventTypeMemBlock);

    memBlock->memStart = memStart;
    memBlock->blockSize = blockSize;
    memBlock->maxCount = blockCnt;

    tSlistInit(&memBlock->blockList);
    while (memBlockStart < memBlockEnd) {
        tSnodeInit((tSnode *) memBlockStart);
        tSListAddLast(&memBlock->blockList, (tSnode *)memBlockStart);

        memBlockStart += blockSize;
    }
}

/**
 * 等待存储块
 * @param memBlock 等待的存储块
 * @param mem 存储块存储的地址
 * @param waitTicks 当没有存储块时，等待的ticks数，为0时表示永远等待
 * @return 等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
 */
uint32_t tMemBlockWait (tMemBlock *memBlock, void **mem, uint32_t waitTicks) {
    uint32_t status = tTaskEnterCritical();

    // 首先检查是否有空闲的存储块
    if (tSlistCount(&memBlock->blockList) > 0) {
        // 如果有的话，取出一个
        *mem = (uint8_t *) tSListRemoveFirst(&memBlock->blockList);
        tTaskExitCritical(status);
        return tErrorNoError;
    } else {
        // 然后将任务插入事件队列中
        tEventWait(&memBlock->event, currentTask, (void *) 0, tEventTypeMemBlock, waitTicks);
        tTaskExitCritical(status);

        // 最后再执行一次事件调度，以便于切换到其它任务
        tTaskSched();

        // 当切换回来时，从tTask中取出获得的消息
        *mem = currentTask->eventMsg;

        // 取出等待结果
        return currentTask->waitEventResult;
    }
}

/**
 * 获取存储块，如果没有存储块，则立即退回
 * @param memBlock 等待的存储块
 * @param mem 存储块存储的地址
 * @return 获取结果, tErrorResourceUnavaliable.tErrorNoError
 */
uint32_t tMemBlockNoWaitGet (tMemBlock *memBlock, void **mem) {
    uint32_t status = tTaskEnterCritical();

    // 首先检查是否有空闲的存储块
    if (tSlistCount(&memBlock->blockList) > 0) {
        // 如果有的话，取出一个
        *mem = (uint8_t *) tSListRemoveFirst(&memBlock->blockList);
        tTaskExitCritical(status);
        return tErrorNoError;
    } else {
        // 否则，返回资源不可用
        tTaskExitCritical(status);
        return tErrorResourceUnavaliable;
    }
}

/**
 * 通知存储块可用，唤醒等待队列中的一个任务，或者将存储块加入队列中
 * @param memBlock 通知存储块
 * @param memBlock 操作的信号量
 */
void tMemBlockNotify (tMemBlock *memBlock, void *mem) {
    uint32_t status = tTaskEnterCritical();

    // 检查是否有任务等待
    if (tEventWaitCount(&memBlock->event) > 0) {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        tTask *task = tEventWakeUp(&memBlock->event, (void *) mem, tErrorNoError);

        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio) {
            tTaskSched();
        }
    } else {
        // 如果没有任务等待的话，将存储块插入到队列中
        tSListAddLast(&memBlock->blockList, (tSnode *) mem);
    }

    tTaskExitCritical(status);
}

/**
 * 查询存储控制块的状态信息
 * @param memBlock 存储控制块
 * @param info 状态查询存储的位置
 */
void tMemBlockGetInfo (tMemBlock *memBlock, tMemBlockInfo *info) {
    uint32_t status = tTaskEnterCritical();

    // 拷贝需要的信息
    info->count = tSlistCount(&memBlock->blockList);
    info->maxCount = memBlock->maxCount;
    info->blockSize = memBlock->blockSize;
    info->taskCount = tEventWaitCount(&memBlock->event);

    tTaskExitCritical(status);
}

/**
 * 销毁存储控制块
 * @param memBlock 需要销毁的存储控制块
 * @return 因销毁该存储控制块而唤醒的任务数量
 */
uint32_t tMemBlockDestroy (tMemBlock *memBlock) {
    uint32_t status = tTaskEnterCritical();

    // 清空事件控制块中的任务
    uint32_t count = tEventRemoveAll(&memBlock->event, (void *) 0, tErrorDel);

    tTaskExitCritical(status);

    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0) {
        tTaskSched();
    }
    return count;
}

#endif // TINYOS_ENABLE_MEMBLOCK

/** @} */
