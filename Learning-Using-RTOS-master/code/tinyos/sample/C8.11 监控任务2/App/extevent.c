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
    ExtEventExtIO4 = 1 << 1,
    ExtEventExtIO5 = 1 << 2,
    ExtEventExtIO6 = 1 << 3,
    ExtEventExtIO7 = 1 << 4,
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

/**
 * 通知引脚中断
 */
void ExtEventExtIOInt (ExtIntIOPin pin) {
    switch (pin) {
        case ExtIntIOPin4:
            tFlagGroupNotify(&flagGroup, 1, ExtEventExtIO4);
            break;
        case ExtIntIOPin5:
            tFlagGroupNotify(&flagGroup, 1, ExtEventExtIO5);
            break;
        case ExtIntIOPin6:
            tFlagGroupNotify(&flagGroup, 1, ExtEventExtIO6);
            break;
        case ExtIntIOPin7:
            tFlagGroupNotify(&flagGroup, 1, ExtEventExtIO7);
            break;
    }
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

static void extioEvent (uint32_t resultFlag) {
    if (resultFlag & ExtEventExtIO4) {
        showMsg("Pin4 Interrupt\r\n");
    }

    if (resultFlag & ExtEventExtIO5) {
        showMsg("Pin5 Interrupt\r\n");
    }

    if (resultFlag & ExtEventExtIO6) {
        showMsg("Pin6 Interrupt\r\n");
    }

    if (resultFlag & ExtEventExtIO7) {
        showMsg("Pin7 Interrupt\r\n");
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

        tFlagGroupWait(&flagGroup, TFLAGGROUP_SET_ANY | TFLAGGROUP_CONSUME, ExtEventFlagAll, &resultFlag, 0);
        if (resultFlag & ExtEventButton) {
            ButtonNoWaitGetPressed(&buttonId);
            buttonEvent(buttonId);
        } else if (resultFlag & (ExtEventExtIO4 | ExtEventExtIO5 | ExtEventExtIO6 | ExtEventExtIO7)) {
            extioEvent(resultFlag);
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
