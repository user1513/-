/**
 * @brief tOS应用示例
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#include "tinyOS.h"
#include "app.h"
#include "hal.h"

#include "button.h"
#include "uart.h"
#include "extio.h"
#include "WaveGen.h"
#include "cli.h"
#include "extevent.h"
#include "monitor.h"

#include "string.h"

#ifdef DEBUG

#define STACK_CHECK_TASK_ENV_SIZE   128

static tTaskStack stackCheckTaskEnv[STACK_CHECK_TASK_ENV_SIZE];
static tTask stackCheckTask;

static void showTaskInfo (const char * taskName, tTaskInfo * taskInfo) {
    static char msgBuffer[128];

    sprintf(msgBuffer, "Task %s prio %d, stackSize %d, stackFree %d, stackUsed %d\r\n",
            taskName, taskInfo->prio,
            taskInfo->stackSize, taskInfo->stackFree, taskInfo->stackSize - taskInfo->stackFree);
    UartWrite(msgBuffer, strlen(msgBuffer));
}

void TaskStackCheckEntry (void * arg) {
    static const char * lr = "\r\n";
    tTaskInfo taskInfo;

    for (;;) {
        UartWrite(lr, strlen(lr));

        tTaskGetInfo(CLITask(), &taskInfo);
        showTaskInfo("cli", &taskInfo);

        tTaskGetInfo(ExtEventTask(), &taskInfo);
        showTaskInfo("ext", &taskInfo);

        if (MonitorIsOn()) {
            tTaskGetInfo(MonitorTask(), &taskInfo);
            showTaskInfo("mon", &taskInfo);
        }

        tTaskGetInfo(tTimerTask(), &taskInfo);
        showTaskInfo("tmr", &taskInfo);

        tTaskGetInfo(tIdleTask(), &taskInfo);
        showTaskInfo("idle", &taskInfo);

        tTaskGetInfo(currentTask, &taskInfo);
        showTaskInfo("stack", &taskInfo);

        tTaskDelay(1000 / TINYOS_SYSTICK_MS);
    }
}

void TaskMonitorInit (void) {
    tTaskInit(&stackCheckTask, TaskStackCheckEntry, (void *) 0x0,
              TINYOS_PRO_COUNT - 2, stackCheckTaskEnv, sizeof(stackCheckTaskEnv));
}

#endif // DEBUG

/**
 * App的初始化
 */
void tInitApp (void) {
    halInit();

    ButtonInit(ExtEventButtonPress);
    UartInit();
    ExtIOInit(ExtEventExtIOInt);
    WaveGenInit();

    CLIInit();
    ExtEventInit();

    TaskMonitorInit();
}

