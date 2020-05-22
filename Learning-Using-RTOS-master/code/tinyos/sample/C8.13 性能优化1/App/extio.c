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

static ExtIntIOCb extIntIOCb;

static uint16_t extioMap[ExtIOPinEnd] = {
        GPIO_Pin_1,     // 对应ExtIOPin0，以下类堆
        GPIO_Pin_2,
        GPIO_Pin_3,
        GPIO_Pin_4
};

/**
 * 初始化外部IO
 */
void ExtIOInit (ExtIntIOCb cb) {
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置GPIO中断管脚
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource7);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);

    EXTI_InitStructure.EXTI_Line = EXTI_Line5 | EXTI_Line6 | EXTI_Line7 | EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    
    extIntIOCb = cb;
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

void ExtIOResetInput (void) {
    ExtIOSetDir(ExtIOPin0, 1);
    ExtIOSetDir(ExtIOPin1, 1);
    ExtIOSetDir(ExtIOPin2, 1);
    ExtIOSetDir(ExtIOPin3, 1);
}

void EXTI9_5_IRQHandler (void) {
    if (EXTI_GetITStatus(EXTI_Line5) != RESET) {
        if (extIntIOCb) {
            extIntIOCb(ExtIntIOPin4);
        }
        
        EXTI_ClearITPendingBit(EXTI_Line5);
    }

    if (EXTI_GetITStatus(EXTI_Line6) != RESET) {
        if (extIntIOCb) {
            extIntIOCb(ExtIntIOPin5);
        }
        
        EXTI_ClearITPendingBit(EXTI_Line6);
    }

    if (EXTI_GetITStatus(EXTI_Line7) != RESET) {
        if (extIntIOCb) {
            extIntIOCb(ExtIntIOPin6);
        }
        
        EXTI_ClearITPendingBit(EXTI_Line7);
    }

    if (EXTI_GetITStatus(EXTI_Line8) != RESET) {
        if (extIntIOCb) {
            extIntIOCb(ExtIntIOPin7);
        }
        
        EXTI_ClearITPendingBit(EXTI_Line8);
    }
}

