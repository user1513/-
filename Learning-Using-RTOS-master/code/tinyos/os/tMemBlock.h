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

#ifndef TMEMBLOCK_H
#define TMEMBLOCK_H

#include "tConfig.h"

#if !defined(TINYOS_ENABLE_MEMBLOCK) || TINYOS_ENABLE_MEMBLOCK == 1

#include "tEvent.h"

typedef struct _tMemBlock {
    tEvent event;                   /**< 事件控制块 */

    void *memStart;                 /**< 存储块的首地址 */
    uint32_t blockSize;             /**< 每个存储块的大小 */
    uint32_t maxCount;              /**< 总的存储块的个数 */

    tSlist blockList;                /**< 存储块列表 */
} tMemBlock;

typedef struct _tMemBlockInfo {
    uint32_t count;                 /**< 当前存储块的计数 */
    uint32_t maxCount;              /**< 允许的最大计数 */
    uint32_t blockSize;             /**< 每个存储块的大小 */
    uint32_t taskCount;             /**< 当前等待的任务计数 */
} tMemBlockInfo;

void tMemBlockInit (tMemBlock *memBlock, void *memStart, uint32_t blockSize, uint32_t blockCnt);
uint32_t tMemBlockWait (tMemBlock *memBlock, void **mem, uint32_t waitTicks);
uint32_t tMemBlockNoWaitGet (tMemBlock *memBlock, void **mem);
void tMemBlockNotify (tMemBlock *memBlock, void *mem);
void tMemBlockGetInfo (tMemBlock *memBlock, tMemBlockInfo *info);
uint32_t tMemBlockDestroy (tMemBlock *memBlock);

#endif // TINYOS_ENABLE_MEMBLOCK

#endif /* TMEMBLOCK_H */

/** @} */
