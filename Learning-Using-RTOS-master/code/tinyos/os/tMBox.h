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

#ifndef TMBOX_H
#define TMBOX_H

#include "tConfig.h"

#if !defined(TINYOS_ENABLE_MBOX) || TINYOS_ENABLE_MBOX == 1

#include "tEvent.h"

#define tMBOXSendNormal             0x00        /**< 正常发送发送至缓冲区 */
#define tMBOXSendFront              0x01        /**< 消息发送至缓冲区头部 */

// 邮箱类型
typedef struct _tMbox {
    tEvent event;                               /**< 事件控制块 */

    uint32_t count;                             /**< 当前的消息数量 */
    uint32_t read;                              /**< 读取消息的索引 */
    uint32_t write;                             /**< 写消息的索引 */
    uint32_t maxCount;                          /**< 最大允许容纳的消息数量 */

    void **msgBuffer;                           /**< 消息存储缓冲区 */
} tMbox;

// 邮箱状态类型
typedef struct _tMboxInfo {
    uint32_t count;                             /**< 当前的消息数量 */
    uint32_t maxCount;                          /**< 最大允许容纳的消息数量 */
    uint32_t taskCount;                         /**< 当前等待的任务计数 */
} tMboxInfo;

void tMboxInit (tMbox *mbox, void **msgBuffer, uint32_t maxCount);
uint32_t tMboxWait (tMbox *mbox, void **msg, uint32_t waitTicks);
uint32_t tMboxNoWaitGet (tMbox *mbox, void **msg);
uint32_t tMboxNotify (tMbox *mbox, void *msg, uint32_t notifyOption);
void tMboxFlush (tMbox *mbox);
uint32_t tMboxDestroy (tMbox *mbox);
void tMboxGetInfo (tMbox *mbox, tMboxInfo *info);

#endif // TINYOS_ENABLE_MBOX

#endif /* TMBOX_H */ 

/** @} */
