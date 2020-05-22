/**
 * @brief tinyOS 与处理器相关配置
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#include "tinyOS.h"
#include "stm32f10x.h"

/**
 * 系统时钟节拍定时器System Tick配置
 * 在我们目前的环境（模拟器）中，系统时钟节拍为12MHz
 * 请务必按照本教程推荐配置，否则systemTick的值就会有变化，需要查看数据手册才了解
 */
void tSetSysTickPeriod (uint32_t ms) {
    SystemCoreClockUpdate();
    SysTick->LOAD = (uint64_t)ms * SystemCoreClock / 1000 - 1;
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}

/**
 * SystemTick的中断处理函数
 */
void SysTick_Handler () {
    tTaskSystemTickHandler();
}

