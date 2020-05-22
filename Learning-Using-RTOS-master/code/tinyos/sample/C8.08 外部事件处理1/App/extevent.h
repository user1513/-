/**
 * @brief 按键事件处理设计
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#ifndef EXTEVENT_H
#define EXTEVENT_H

#include "extio.h"
#include "button.h"

#define EXTEVENT_TASK_PRIO           0
#define EXTEVENT_TASK_ENV_SIZE       512

void ExtEventInit (void);
void ExtEventButtonPress (ButtonId id);

#endif //MONITOR_H
