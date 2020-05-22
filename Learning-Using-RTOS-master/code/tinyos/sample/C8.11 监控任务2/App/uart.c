/**
 * @brief 串口驱动
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#include "tinyOS.h"
#include "uart.h"
#include "stm32f10x_usart.h"

static uint8_t rxBuffer[UART_RXBUFFER_SIZE];
static uint32_t rxWritePos;
static uint32_t rxReadPos;
static tSem rxReceivedSem;

static uint8_t txBuffer[UART_TXBUFFER_SIZE];
static uint32_t txWritePos;
static uint32_t txReadPos;
static tSem txFreeSem;

/**
 * 中断处理函数
 */
void USART1_IRQHandler (void) {
    ITStatus status;

    // 串口接收
    status = USART_GetITStatus(USART1, USART_IT_RXNE);
    if (status == SET) {
        tSemInfo semInfo;

        uint16_t ch = (uint32_t)USART_ReceiveData(USART1);

        // 仅当有空闲空间时才写入，否则丢弃
        tSemGetInfo(&rxReceivedSem, &semInfo);
        if (semInfo.count < semInfo.maxCount) {
            tTaskCritical_t critical = tTaskEnterCritical();
            rxBuffer[rxWritePos++] = (uint8_t)ch;
            if (rxWritePos >= UART_RXBUFFER_SIZE) {
                rxWritePos = 0;
            }
            tTaskExitCritical(critical);

            tSemNotify(&rxReceivedSem);
        }

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }

    // 发送中断: 自动从邮箱中取数据发送
    status = USART_GetITStatus(USART1, USART_IT_TXE);
    if (status == SET) {
        tSemInfo semInfo;

        // 如果发送缓冲有数据，取一个发送
        tSemGetInfo(&txFreeSem, &semInfo);
        if (semInfo.count < semInfo.maxCount) {
            tTaskCritical_t critical;
            uint8_t ch;

            // 从发送缓冲区中取数据
            critical = tTaskEnterCritical();
            ch = txBuffer[txReadPos++];
            if (txReadPos >= UART_TXBUFFER_SIZE) {
                txReadPos = 0;
            }
            tTaskExitCritical(critical);

            tSemNotify(&txFreeSem);

            USART_SendData(USART1, (uint16_t)ch);
        } else {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }

        USART_ClearITPendingBit(USART1, USART_IT_TXE);  
    }
}

/**
 * USART硬件初始化
 */
static void UartHalInit (void) {
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = UART_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    NVIC_EnableIRQ(USART1_IRQn);
}

/**
 * 初始化串口
 */
void UartInit (void) {
    UartHalInit();

    tSemInit(&rxReceivedSem, 0, UART_RXBUFFER_SIZE);
    rxReadPos = 0;
    rxWritePos = 0;

    tSemInit(&txFreeSem, UART_TXBUFFER_SIZE, UART_TXBUFFER_SIZE);
    txReadPos = 0;
    txWritePos = 0;
 }

/**
 * 等待接收数据包
 * @param packet 接收到的数据包指针存储地址
 * @return 0 无错误；1 有错误
 */
void UartRead (char * packet, uint32_t len) {
    tTaskCritical_t critical;

    while (len -- > 0) {
        tSemWait(&rxReceivedSem, 0);

        // 从接收缓冲区中读取
        critical = tTaskEnterCritical();
        *packet++ = rxBuffer[rxReadPos++];
        if (rxReadPos >= UART_RXBUFFER_SIZE) {
            rxReadPos = 0;
        }
        tTaskExitCritical(critical);
    }
}

/**
 * 写入数据包
 * @param packet 待写入的数据包
 */
void UartWrite (const char * packet, uint32_t len) {
    tTaskCritical_t critical;
    uint32_t status;

    while (len-- > 0) {
        // 等待空闲空间
        tSemWait(&txFreeSem, 0);

        // 写入发送缓冲区
        critical = tTaskEnterCritical();
        txBuffer[txWritePos++] = *packet++;
        if (txWritePos >= UART_TXBUFFER_SIZE) {
            txWritePos = 0;
        }
        tTaskExitCritical(critical);

        // 这里加循环反复调用，是考虑到中途可能发生其它中断延迟导致没有及时触发
        // 只有当硬件真正在空闲时，才手动触发一次
        status = USART_GetFlagStatus(USART1, USART_FLAG_TXE);
        if (status == SET) {
            USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
        }
    }
}
