/**
 * @brief 按键驱动
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#ifndef BUTTON_H
#define BUTTON_H

#include "appConfig.h"

#define BUTTON_MSG_NR                   10          // 按键消息缓存数量
#define BUTTON_SCAN_INTERVAL            20          // 安键扫描的时间间隔, ms为单位

typedef enum {
    ButtonNone,
    ButtonStartStop,
    Button1,
    Button2,
    Button3,
}ButtonId;

typedef enum {
    ButtonDown,
    ButtonUp,
}ButtonState;

typedef void (*ButtonPressCb) (ButtonId id);

void ButtonInit (ButtonPressCb cb);
uint32_t ButtonWaitPress (ButtonId * id);
uint32_t ButtonGetPressed (ButtonId * id);
uint32_t ButtonNoWaitGetPressed (ButtonId * id);

#endif //BUTTON_H
