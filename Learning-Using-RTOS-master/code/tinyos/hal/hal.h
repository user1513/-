/**
 * @brief tOS 目标相关配置代码
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#ifndef PROJECT_TARGET_H
#define PROJECT_TARGET_H

#include "tinyOS.h"
#include <stdio.h>

enum IRQType {
    IRQ_PRIO_HIGH,                  // 高优先级中断
    IRQ_PRIO_MIDDLE,                // 中优先级中断
    IRQ_PRIO_LOW,                   // 低优先级中断
};

#if TINYOS_ENABLE_MUTEX == 1
    extern tMutex xprintfMutex;

    #define xprintf(fmt, ...) {     \
            tMutexWait(&xprintfMutex, 0);   \
            printf(fmt, ##__VA_ARGS__);       \
            tMutexNotify(&xprintfMutex);    \
    }
#else
    #define xprintf(fmt, ...) { printf(fmt, ##__VA_ARGS__); }
#endif


void halInit (void);

void targetEnterSleep (void);

void interruptByOtherTask (void);

void interruptEnable (enum IRQType irq, int enable);

void interruptByIRQ (enum IRQType irq);

void xprintfMem (uint8_t * mem, uint32_t size);

__weak void IRQHighHandler (void);
__weak void IRQMiddleHandler (void);
__weak void IRQLowHandler (void);

#endif //PROJECT_TARGET_H
