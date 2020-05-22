/**
 * @brief 监控任务设计
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#include <string.h>
#include "tinyOS.h"
#include "extevent.h"
#include "button.h"
#include "uart.h"
#include "extio.h"
#include "WaveGen.h"

typedef enum {
    ExtEventButton = 1 << 0,
    ExtEventFlagAll = 0x1F,
}ExtEventType;

// 任务相关
static tTaskStack exteventTaskEnv[EXTEVENT_TASK_ENV_SIZE];
static tTask exteventTask;

static tFlagGroup flagGroup;

/**
 * @param msg
 */
static void showMsg (const char * msg) {
    UartWrite(msg, strlen(msg));
}

/**
 * 通知按键按下
 */
void ExtEventButtonPress (ButtonId id) {
    tFlagGroupNotify(&flagGroup, 1, ExtEventButton);
}

static void buttonEvent (ButtonId id) {
    switch (id) {
        case ButtonStartStop:
            WaveStopOutput();
            ExtIOResetInput();

            showMsg("Start/Stop Button Press\r\n");
            break;
        case Button1:
            showMsg("Button1 Press\r\n");
            break;
        case Button2:
            showMsg("Button2 Press\r\n");
            break;
        case Button3:
            showMsg("Button3 Press\r\n");
            break;
        default:
            break;
    }
}

/**
 * 监控任务
 * @param param
 */
void exteventTaskEntry (void * param) {
    ButtonId buttonId;

    for (;;) {
        uint32_t resultFlag;

        tFlagGroupWait(&flagGroup, TFLAGGROUP_SET_ANY | TFLAGGROUP_CONSUME, ExtEventButton, &resultFlag, 0);
        if (resultFlag & ExtEventButton) {
            ButtonNoWaitGetPressed(&buttonId);
            buttonEvent(buttonId);
        }
    }
}

/**
 * 监控初始化
 */
void ExtEventInit (void) {
    tFlagGroupInit(&flagGroup, 0);
    tTaskInit(&exteventTask, exteventTaskEntry, (void *) 0x0, EXTEVENT_TASK_PRIO, exteventTaskEnv, sizeof(exteventTaskEnv));
}
