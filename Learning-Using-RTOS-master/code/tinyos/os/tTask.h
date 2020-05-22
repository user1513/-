/**
 * @brief tinyOS的任务管理
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

/**
 * @defgroup 任务管理 任务管理
 * @{
 */

#ifndef TTASK_H
#define TTASK_H

#define TINYOS_TASK_STATE_RDY                   0
#define TINYOS_TASK_STATE_DESTROYED             (1 << 0)
#define TINYOS_TASK_STATE_DELAYED               (1 << 1)
#define TINYOS_TASK_STATE_SUSPEND               (1 << 2)

#define TINYOS_TASK_WAIT_MASK                   (0xFF << 16)

// 前置声明
struct _tEvent;

// Cortex-M的堆栈单元类型：堆栈单元的大小为32位，所以使用uint32_t
typedef uint32_t tTaskStack;

// 任务结构：包含了一个任务的所有信息
typedef struct _tTask {
    // 任务所用堆栈的当前堆栈指针。每个任务都有他自己的堆栈，用于在运行过程中存储临时变量等一些环境参数
    // 在tinyOS运行该任务前，会从stack指向的位置处，会读取堆栈中的环境参数恢复到CPU寄存器中，然后开始运行
    // 在切换至其它任务时，会将当前CPU寄存器值保存到堆栈中，等待下一次运行该任务时再恢复。
    // stack保存了最后保存环境参数的地址位置，用于后续恢复
    uint32_t *stack;
    uint32_t *stackBase;                /**< 堆栈的起即地址 */

    uint32_t stackSize;                 /**< 堆栈的总容量 */
    tNode linkNode;                     /**< 连接结点 */

    uint32_t delayTicks;                /**< 任务延时计数器 */
    tNode delayNode;                    /**< 延时结点：通过delayNode就可以将tTask放置到延时队列中 */

    uint32_t prio;                      /**< 任务的优先级 */

    uint32_t state;                     /**< 任务当前状态 */
    uint32_t slice;                     /**< 当前剩余的时间片 */

    uint32_t suspendCount;              /**< 被挂起的次数 */

    void (*clean) (void *param);        /**< 任务被删除时调用的清理函数 */
    void *cleanParam;                   /**< 传递给清理函数的参数 */
    uint8_t requestDeleteFlag;          /**< 请求删除标志，非0表示请求删除 */


    struct _tEvent *waitEvent;          /**< 任务正在等待的事件类型 */
    void *eventMsg;                     /**< 等待事件的消息存储位置 */
    uint32_t waitEventResult;           /**< 等待事件的结 */
    uint32_t waitFlagsType;             /**< 等待的事件方式 */
    uint32_t eventFlags;                /**< 等待的事件标志 */
} tTask;

// 任务相关信息结构
typedef struct _tTaskInfo {
    uint32_t delayTicks;                /**< 任务延时计数器 */
    uint32_t prio;                      /**< 任务的优先级 */
    uint32_t state;                     /**< 任务当前状态 */
    uint32_t slice;                     /**< 当前剩余的时间片 */
    uint32_t suspendCount;              /**< 被挂起的次数 */
    uint32_t stackSize;                 /**< 堆栈的总容量 */
    uint32_t stackFree;                 /**< 堆栈空余量 */
} tTaskInfo;

void tTaskInit (tTask *task, void (*entry) (void *), void *param, uint32_t prio, uint32_t *stack, uint32_t size);
void tTaskSuspend (tTask *task);
void tTaskWakeUp (tTask *task);
void tTaskSetCleanCallFunc (tTask *task, void (*clean) (void *param), void *param);
void tTaskForceDelete (tTask *task);
void tTaskRequestDelete (tTask *task);
uint8_t tTaskIsRequestedDelete (void);
void tTaskDeleteSelf (void);
void tTaskGetInfo (tTask *task, tTaskInfo *info);

#endif /* TTASK_H */ 

/** @} */
