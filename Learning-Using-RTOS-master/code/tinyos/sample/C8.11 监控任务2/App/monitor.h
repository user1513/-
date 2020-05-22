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
#define MONITOR_MAX_CMD             10
#define REPORT_BUFFER_SIZE          128
#define MONITOR_DEFAULT_TIMEOUT     1000

// 监控对像
typedef enum {
    MonitorCPUUsage,
}MonitorTarget;

// 监控命令
typedef struct _MonitorCmd {
    tNode linkNode;

    uint8_t isAdd;
    MonitorTarget target;

    union MonitorOption{
        struct {
            uint8_t warning;
            float warnPercent;
        }cpu;
    }options;
}MonitorCmd;

void MonitorInit (void);
void MonitorOn (void);
void MonitorOff (void);
uint8_t MonitorIsOn (void);

MonitorCmd * MonitorAllocCmd (void);
void MonitorFreeCmd (MonitorCmd * cmd);
void MonitorSendCmd (MonitorCmd * cmd);
void MonitorSetPeriod (uint32_t ms);

#endif //PROJECT_MONITOR_H
