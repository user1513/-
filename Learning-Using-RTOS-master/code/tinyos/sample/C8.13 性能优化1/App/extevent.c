#include "tinyOS.h"
#include "extevent.h"
#include "Button.h"
#include "WaveGen.h"
#include "extio.h"
#include "Uart.h"
#include <string.h>

typedef enum {
    ExtEventButton = 1 << 0,
    ExtEventExtIO4 = 1 << 1,
    ExtEventExtIO5 = 1 << 2,
    ExtEventExtIO6 = 1 << 3,
    ExtEventExtIO7 = 1 << 4,
    ExtEventAll = 0xFF,
}ExtEventType;

static tTaskStack exteventTaskEnv[EXTEVENT_TASK_ENV_SIZE];
static tTask exteventTask;

static tFlagGroup flagGroup;

static void showMsg (const char * msg) {
    UartWrite(msg, strlen(msg));
}

void ExtEventButtonPress (ButtonId id) {
    tFlagGroupNotify(&flagGroup, 1, ExtEventButton);
}

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
            showMsg("Button2 Press\r\n");
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

void exteventTaskEntry (void * param) {
    ButtonId buttonId;
    
    for (;;) {
        uint32_t resultFlag;
        
        tFlagGroupWait(&flagGroup, TFLAGGROUP_SET_ANY | TFLAGGROUP_CONSUME, ExtEventAll, &resultFlag, 0);
        if (resultFlag & ExtEventButton) {
            ButtonWaitPress(&buttonId);
            
            buttonEvent(buttonId);
        } else if (resultFlag & (ExtEventExtIO4 | ExtEventExtIO5 | ExtEventExtIO6 | ExtEventExtIO7)) {
            extioEvent(resultFlag);
        }
    }
}

void ExtEventInit (void) {
    tFlagGroupInit(&flagGroup, 0);
    tTaskInit(&exteventTask, exteventTaskEntry, 0, EXTEVENT_TASK_PRIO, exteventTaskEnv, EXTEVENT_TASK_ENV_SIZE);
}

tTask * ExtEvnetTask (void) {
    return &exteventTask;
}

