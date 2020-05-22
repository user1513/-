/**
 * @brief 按键驱动
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#include "tinyOS.h"
#include "button.h"
#include "stm32f10x.h"

// 按键存储消息缓冲区
static tMbox buttonMbox;
static void * buttonMsgBuffer[BUTTON_MSG_NR];

static tTimer buttonTimer;              // 按键扫描定时器

/**
 * 初始化按键硬件
 */
static void ButtonHalInit (void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * 获取哪一个按键按下
 */
static ButtonId ButtonGetWhichPress (void) {
    uint8_t state;
    ButtonId buttonId = ButtonNone;

    state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
    if (state == Bit_SET) {
        buttonId = Button1;
    }

    state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_2);
    if (state == Bit_SET) {
        buttonId = Button2;
    }

    state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3);
    if (state == Bit_SET) {
        buttonId = Button3;
    }

    state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
    if (state == Bit_SET) {
        buttonId = ButtonStartStop;
    }

    return buttonId;
}

/**
 * 按键定时器扫描处理, 按键处理译码
 * @param arg
 */
static void timerFunc (void * arg) {
    static ButtonId prePressedButton = ButtonNone;
    static enum {
        NO_DOWN,
        RECHECK_DOWN,
        CONFIRMED_DOWN,
        RECHECK_UP
    } scanState = NO_DOWN;

    // 读取当前硬件状态
    ButtonId currPressedButton = ButtonGetWhichPress();

    switch (scanState) {
        case NO_DOWN:
            // 有按键按下，进入第1次确认检查状态
            if (currPressedButton != ButtonNone) {
                scanState = RECHECK_DOWN;
                DEBUG_PRINT("NO_DOWN -> RECHECK_DOWN:%d\n", currPressedButton);
            }
            break;
        case RECHECK_DOWN:
            if (currPressedButton == ButtonNone) {
                scanState = NO_DOWN;
                DEBUG_PRINT("RECHECK_DOWN -> NO_DOWN:%d\n", currPressedButton);
            } else if (prePressedButton == currPressedButton) {
                // 发消息通知，根据按键类型，决定如何处理. 可以考虑从某个配置表中查，这样可配置多个按键事件的通知行为
                uint32_t notifyOption = (currPressedButton == ButtonStartStop)
                                        ? tMBOXSendFront : tMBOXSendNormal;
                tMboxNotify(&buttonMbox, (void *)currPressedButton, notifyOption);

                scanState = CONFIRMED_DOWN;
                DEBUG_PRINT("RECHECK_DOWN -> CONFIRMED_DOWN:%d\n", currPressedButton);
            }
            break;
        case CONFIRMED_DOWN:
            // 进入这个状态后，要做的是检查按键是否重复按下或者等待按键释.这里简单起见，等待按键释放
            if (currPressedButton == ButtonNone) {
                // 发现按键释放？进一步确认。但这里要return，否则prePressedButton会在后面设置None
                scanState = RECHECK_UP;
                DEBUG_PRINT("CONFIRMED_DOWN -> RECHECK_UP:%d\n", currPressedButton);
                return;
            } else if (currPressedButton != prePressedButton) {
                // 发现不一样的按键按下，重新确认
                scanState = RECHECK_DOWN;
                DEBUG_PRINT("CONFIRMED_DOWN -> RECHECK_DOWN:%d\n", currPressedButton);
            }
            break;
        case RECHECK_UP:
            if (currPressedButton == ButtonNone) {
                // 确认没有按键按下，已经释放
                scanState = NO_DOWN;
                DEBUG_PRINT("RECHECK_UP -> NO_DOWN:%d\n", currPressedButton);
            } else if (currPressedButton != prePressedButton) {
                // 发现不一样的按键按下，重新确认是否按下
                scanState = RECHECK_DOWN;
                DEBUG_PRINT("RECHECK_UP -> scanState:%d\n", currPressedButton);
            } else if (currPressedButton == prePressedButton) {
                // 相同的键，噢，前一次可能是误触发，再次重新检查
                scanState = CONFIRMED_DOWN;
                DEBUG_PRINT("RECHECK_UP -> CONFIRMED_DOWN:%d\n", currPressedButton);
            }
            break;
    }

    // 记录当前结果
    prePressedButton = currPressedButton;
}

/**
 * 按键初始化
 */
void ButtonInit (void) {
    ButtonHalInit();

    tMboxInit(&buttonMbox, buttonMsgBuffer, BUTTON_MSG_NR);

    tTimerInit(&buttonTimer, 0, BUTTON_SCAN_INTERVAL / TINYOS_SYSTICK_MS, timerFunc, 0, TIMER_CONFIG_TYPE_SOFT);
    tTimerStart(&buttonTimer);
}

/**
 * 等待任意按键按下
 * @param id 按键是否按下的状态
 * @return 非0，有错误发生；0无错误
 */
uint32_t ButtonWaitPress (ButtonId * id) {
    uint32_t err = 0;

    err = tMboxWait(&buttonMbox, (void **)id, 0);
    return err;
}
