#include <string.h>
#include <stdio.h>
#include "tinyOS.h"
#include "monitor.h"
#include "Uart.h"
#include "extio.h"

static uint8_t monitorIsOn;

__align(8) static tTaskStack monitorTaskEnv[MONITOR_TASK_ENV_SIZE];
static tTask monitorTask;

static tMbox cmdMbox;
static void * msgBuffer[MONITOR_MAX_CMD];

static tMemBlock cmdMemBlock;
static MonitorCmd cmdMem[MONITOR_MAX_CMD];

static tList monitorCmdList;

static char reportBuffer[REPORT_BUFFER_SIZE];

MonitorCmd * MonitorAllocCmd (void) {
    MonitorCmd * cmd = 0;
    
    tMemBlockWait(&cmdMemBlock, (void **)&cmd, 0);
    memset(cmd, 0, sizeof(MonitorCmd));
    tNodeInit(&cmd->linkNode);
    return cmd;
}

void MonitorSendCmd (MonitorCmd * cmd) {
    tMboxNotify(&cmdMbox, cmd, tMBOXSendNormal);
}

void MonitorFreeCmd (MonitorCmd * cmd) {
    tMemBlockNotify(&cmdMemBlock, cmd);
}

static MonitorCmd * findCmd (MonitorCmd * cmd) {
    tNode * currNode;
    
    for (currNode = tListFirst(&monitorCmdList); currNode != NULL; currNode = tListNext(&monitorCmdList, currNode)) {
        MonitorCmd * currentCmd = (MonitorCmd *)tNodeParent(currNode, MonitorCmd, linkNode);
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
    
    return 0;
}

static void processReceivedCmd (MonitorCmd * cmd) {
    MonitorCmd * existCmd = findCmd(cmd);
    
    if (cmd->isAddr) {
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
    sprintf(reportBuffer, "ExtIO Pin %d:%d\r\n", pin, (state == ExtIOHigh) ? 1 :0);
    showReportMsg(reportBuffer);
}

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
            default:
                break;
        }
    }
}


void monitorTaskEntry (void * param) {
    for (;;) {
        uint32_t err;
        
        MonitorCmd * cmd;
        err = tMboxWait(&cmdMbox, (void **)&cmd, MONITOR_PERIOD);
        if (err == tErrorNoError) {
            processReceivedCmd(cmd);
        } else {
            reportTarget();
        }
        tTaskDelay(1);
    }
}

void MonitorInit (void) {
    monitorIsOn = 0;
}

void MonitorOn (void) {
    if (monitorIsOn) {
        return;
    }

    monitorIsOn = 1;
    
    tListInit(&monitorCmdList);
    tMboxInit(&cmdMbox, msgBuffer, MONITOR_MAX_CMD);
    tMemBlockInit(&cmdMemBlock, cmdMem, sizeof(MonitorCmd), MONITOR_MAX_CMD);
    
    tTaskInit(&monitorTask, monitorTaskEntry, 0, MONITOR_TASK_PRIO, monitorTaskEnv, MONITOR_TASK_ENV_SIZE);
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

tTask * MonitorTask (void) {
    return &monitorTask;
}

