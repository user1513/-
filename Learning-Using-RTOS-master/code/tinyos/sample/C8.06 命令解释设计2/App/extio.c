/**
 * @brief 外部IO设计
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#include "tinyOS.h"
#include "extio.h"
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO

static uint16_t extioMap[ExtIOPinEnd] = {
        GPIO_Pin_1,     // 对应ExtIOPin0，以下类堆
        GPIO_Pin_2,
        GPIO_Pin_3,
        GPIO_Pin_4
};

/**
 * 初始化外部IO
 */
void ExtIOInit (void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
 * 设置指定IO引脚状态
 * @param pin 需配置的IO
 * @param state 配置的状态值
 */
void ExtIOSetState (ExtIOPin pin, ExtIOState state) {
    tTaskCritical_t critical;
    uint16_t halPin = extioMap[pin];

    critical = tTaskEnterCritical();
    if (state == ExtIOHigh) {
        GPIO_SetBits(GPIOA, halPin);
    } else {
        GPIO_ResetBits(GPIOA, halPin);
    }
    tTaskExitCritical(critical);
}

/**
 * 获取指定引脚的状态
 * @param pin 需获取的引脚
 * @return 引脚的状态
 */
ExtIOState ExtIOGetState (ExtIOPin pin) {
    uint16_t halPin = extioMap[pin];

    uint8_t state = GPIO_ReadInputDataBit(GPIOA, halPin);
    return (state == Bit_SET) ? ExtIOHigh : ExtIOLow;
}

/**
 * 设置管脚状态
 * @param pin
 * @param isInput
 */
void ExtIOSetDir (ExtIOPin pin, uint8_t isInput) {
    tTaskCritical_t critical;
    GPIO_InitTypeDef GPIO_InitStructure;
    uint16_t halPin = extioMap[pin];

    critical = tTaskEnterCritical();

    GPIO_InitStructure.GPIO_Pin = halPin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = isInput ? GPIO_Mode_IPU : GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    tTaskExitCritical(critical);
}
