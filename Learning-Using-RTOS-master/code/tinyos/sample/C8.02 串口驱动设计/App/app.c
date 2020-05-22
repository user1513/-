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

static tTaskStack task1Env[TASK1_ENV_SIZE];     // 任务1的堆栈空间
static tTask task1;

void task1Entry (void * param) {
    char str[64];
    uint32_t count = 0;

    for (;;) {
//        sprintf(str, "Button press: %d\n", count++);
//        UartWritePacket(str);

        char msg;

        UartRead(&msg, 1);
        UartWrite(&msg, 1);
    }
}

/**
 * App的初始化
 */
void tInitApp (void) {
    halInit();

    ButtonInit();
    UartInit();
    tTaskInit(&task1, task1Entry, (void *) 0x0, TASK1_PRIO, task1Env, sizeof(task1Env));
}

