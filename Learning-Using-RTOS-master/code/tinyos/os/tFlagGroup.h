/**
 * @brief tinyOS的事件标志实现
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

/**
 * @defgroup 事件标志组 事件标志组
 * @{
 */

#ifndef TFLAGGROUP_H
#define TFLAGGROUP_H

#include "tConfig.h"

#if !defined(TINYOS_ENABLE_FLAGGROUP) || TINYOS_ENABLE_FLAGGROUP == 1

#include "tEvent.h"

// 事件标志组结构
typedef struct _tFlagGroup {
    tEvent event;                           /**< 事件控制块 */
    uint32_t flags;                         /**< 当前事件标志 */
} tFlagGroup;

// 事件标志组查询信息
typedef struct _tFlagGroupInfo {
    uint32_t flags;                         /**< 当前的事件标志 */
    uint32_t taskCount;                     /**< 当前等待的任务计数 */
} tFlagGroupInfo;

#define TFLAGGROUP_CLEAR            (0x0 << 0)
#define TFLAGGROUP_SET              (0x1 << 0)
#define TFLAGGROUP_ANY              (0x0 << 1)
#define TFLAGGROUP_ALL              (0x1 << 1)

#define TFLAGGROUP_SET_ALL          (TFLAGGROUP_SET | TFLAGGROUP_ALL)
#define TFLAGGROUP_SET_ANY          (TFLAGGROUP_SET | TFLAGGROUP_ANY)
#define TFLAGGROUP_CLEAR_ALL        (TFLAGGROUP_CLEAR | TFLAGGROUP_ALL)
#define TFLAGGROUP_CLEAR_ANY        (TFLAGGROUP_CLEAR | TFLAGGROUP_ANY)

#define TFLAGGROUP_CONSUME        (0x1 << 7)

void tFlagGroupInit (tFlagGroup *flagGroup, uint32_t flags);
uint32_t tFlagGroupWait (tFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag,
                         uint32_t *resultFlag, uint32_t waitTicks);
uint32_t tFlagGroupNoWaitGet (tFlagGroup *flagGroup, uint32_t waitType, uint32_t requstFlag, uint32_t *resultFlag);
void tFlagGroupNotify (tFlagGroup *flagGroup, uint8_t isSet, uint32_t flags);
void tFlagGroupGetInfo (tFlagGroup *flagGroup, tFlagGroupInfo *info);
uint32_t tFlagGroupDestroy (tFlagGroup *flagGroup);

#endif // TINYOS_ENABLE_FLAGGROUP


#endif /* TFLAGGROUP_H */

/** @} */
