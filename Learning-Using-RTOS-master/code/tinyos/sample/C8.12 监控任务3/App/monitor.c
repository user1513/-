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

__align(8) static tTaskStack monitorTaskEnv[MONITOR_TASK_ENV_SIZE];
static tTask monitorTask;

static tMbox cmdMbox;
static void * msgBuffer[MONITOR_MAX_CMD];

static tMemBlock cmdMemBlock;
static MonitorCmd cmdMem[MONITOR_MAX_CMD];

static tList monitorCmdList;

static uint32_t monitorPeriod;          // 监控周期
static char reportBuffer[REPORT_BUFFER_SIZE];

/**
 * 分配命令块
 * @return
 */
MonitorCmd * MonitorAllocCmd (void) {
    MonitorCmd * cmd = 0;

    tMemBlockWait(&cmdMemBlock, (void **)&cmd, 0);
    memset(cmd, 0, sizeof(MonitorCmd));
    tNodeInit(&cmd->linkNode);
    return cmd;
}

/**
 * 释放命令块
 * @param cmd
 */
void MonitorFreeCmd (MonitorCmd * cmd) {
    tMemBlockNotify(&cmdMemBlock, cmd);
}

/**
 * 添加监控命令
 * @param cmd
 */
void MonitorSendCmd (MonitorCmd * cmd) {
    tMboxNotify(&cmdMbox, cmd, tMBOXSendNormal);
}

/**
 * 设置监控周期
 * @param ms
 */
void MonitorSetPeriod (uint32_t ms) {
    monitorPeriod = ms;
}

/**
 * 找到配置相同的命令
 * @param cmd
 * @return
 */
static MonitorCmd * findCmd (MonitorCmd * cmd) {
    tNode * currNode;

    for (currNode = tListFirst(&monitorCmdList); currNode != (tNode *)0; currNode = tListNext(&monitorCmdList, currNode)) {
        MonitorCmd * currentCmd = (MonitorCmd *)tNodeParent(currNode, MonitorCmd, linkNode);

        if (currentCmd->target == cmd->target) {
            switch (currentCmd->target) {
                case MonitorCPUUsage:
                    return currentCmd;
                case MonitorExtIOPin:
                    if (currentCmd->options.pin.pinNum == cmd->options.pin.pinNum) {
                        return currentCmd;
                    }
                    break;
                default:
                    break;
            }
        }
    }
    return 0;
}

/**
 * 添加命令
 */
static void procReceivedCmd (MonitorCmd * cmd) {
    MonitorCmd * existCmd = findCmd(cmd);

    if (cmd->isAdd) {
        if (existCmd) {
            tListRemove(&monitorCmdList, &existCmd->linkNode);
            MonitorFreeCmd(existCmd);
        }

        tListAddLast(&monitorCmdList, &cmd->linkNode);
    } else {
        if (existCmd) {
            tListRemove(&monitorCmdList, &existCmd->linkNode);
            MonitorFreeCmd(existCmd);
        }

        MonitorFreeCmd(cmd);
    }
}

static void showReportMsg (const char * msg) {
    UartWrite(msg, strlen(msg));
}

static void reportCPUUsage (MonitorCmd * cmd) {
    float cpuUsage = tCpuUsageGet();

    sprintf(reportBuffer, "CPU usage:%lf\r\n", cpuUsage);
    showReportMsg(reportBuffer);

    if (cmd->options.cpu.warning) {
        if (cmd->options.cpu.warnPercent <= cpuUsage) {
            showReportMsg("Warning: CPU usage too high\r\n");
        }
    }
}

static void reportExtIOPin (MonitorCmd * cmd) {
    ExtIOState state;

    ExtIOPin pin = (ExtIOPin)cmd->options.pin.pinNum;
    if (pin >= ExtIOPinEnd) {
        return;
    }

    state = ExtIOGetState(pin);
    sprintf(reportBuffer, "ExtIO Pin %d:%d\r\n", pin, (state == ExtIOHigh) ? 1 : 0);
    showReportMsg(reportBuffer);
}

/**
 * 报告错误
 * @param cmd
 */
static void reportTarget (void) {
    tNode * currNode;

    for (currNode = tListFirst(&monitorCmdList); currNode != NULL; currNode = tListNext(&monitorCmdList, currNode)) {
        MonitorCmd * currentCmd = (MonitorCmd *)tNodeParent(currNode, MonitorCmd, linkNode);

        switch (currentCmd->target) {
            case MonitorCPUUsage:
                reportCPUUsage(currentCmd);
                break;
            case MonitorExtIOPin:
                reportExtIOPin(currentCmd);
                break;
        }
    }
}

/**
 * 监控任务
 * @param param
 */
void monitorTaskEntry (void * param) {
    for (;;) {
        uint32_t err;
        MonitorCmd * cmd;

        err = tMboxWait(&cmdMbox, (void **)&cmd, monitorPeriod / TINYOS_SYSTICK_MS);
        if (err == tErrorNoError) {
            procReceivedCmd(cmd);
        } else {
            reportTarget();
        }
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

    monitorPeriod = MONITOR_DEFAULT_TIMEOUT;
    tListInit(&monitorCmdList);

    tMboxInit(&cmdMbox, msgBuffer, MONITOR_MAX_CMD);
    tMemBlockInit(&cmdMemBlock, cmdMem, sizeof(MonitorCmd), MONITOR_MAX_CMD);

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
