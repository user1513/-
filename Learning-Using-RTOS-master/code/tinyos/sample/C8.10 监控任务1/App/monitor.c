/**
 * @brief 监控任务设计
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#include <string.h>
#include <stdio.h>
#include "monitor.h"
#include "uart.h"
#include "extio.h"

static uint8_t monitorIsOn;

static tTaskStack monitorTaskEnv[MONITOR_TASK_ENV_SIZE];
static tTask monitorTask;

static void showReportMsg (const char * msg) {
    UartWrite(msg, strlen(msg));
}

/**
 * 监控任务
 * @param param
 */
void monitorTaskEntry (void * param) {
    showReportMsg("Monitor Running\r\n");
    
    for (;;) {
        tTaskDelay(1);
    }
}

/**
 * 监控初始化
 */
void MonitorInit (void) {
    monitorIsOn = 0;          // 未开启
}

void MonitorOn (void) {
    if (monitorIsOn) {
        return;
    }

    monitorIsOn = 1;
    tTaskInit(&monitorTask, monitorTaskEntry, (void *) 0x0, MONITOR_TASK_PRIO, monitorTaskEnv, sizeof(monitorTaskEnv));
}

void MonitorOff (void) {
    if (monitorIsOn == 0) {
        return;
    }

    tTaskForceDelete(&monitorTask);
    monitorIsOn = 0;
}

uint8_t MonitorIsOn (void) {
    return monitorIsOn;
}
