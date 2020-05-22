/**
 * @brief 监控任务设计
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

#ifndef MONITOR_H
#define MONITOR_H

#include "tinyOS.h"

#define MONITOR_TASK_PRIO           0
#define MONITOR_TASK_ENV_SIZE       512

void MonitorInit (void);
void MonitorOn (void);
void MonitorOff (void);
uint8_t MonitorIsOn (void);

#endif //PROJECT_MONITOR_H
