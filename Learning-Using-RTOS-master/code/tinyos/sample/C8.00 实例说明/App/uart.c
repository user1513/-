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

// 接收缓冲处理
static tMbox rxMbox;
static void * rxMboxMsgBuffer[UART_RXBUFFER_SIZE];

// 发送相关设置
static tSem txSem;
static tMbox txMbox;
static void * txMboxMsgBuffer[UART_TXBUFFER_SIZE];     // 采用邮箱会浪费一定空间，但是简化

/**
 * 中断处理函数
 */
void USART1_IRQHandler (void) {
    ITStatus status;

    // 串口接收
    status = USART_GetITStatus(USART1, USART_IT_RXNE);
    if (status == SET) {
        uint32_t ch = (uint32_t)USART_ReceiveData(USART1);
        tMboxNotify(&rxMbox, (void *)ch, tMBOXSendNormal);

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }

    // 发送中断: 自动从邮箱中取数据发送
    status = USART_GetITStatus(USART1, USART_IT_TXE);
    if (status == SET) {
        void * ch;
        uint32_t err;

        // 有数据，从待发送邮箱中获取消息发送
        err = tMboxNoWaitGet(&txMbox, &ch);
        if (err == tErrorNoError) {
            tSemNotify(&txSem);        // 通知有新的空间可用

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

    tMboxInit(&rxMbox, rxMboxMsgBuffer, UART_RXBUFFER_SIZE);

    // 发送相关
    tSemInit(&txSem, UART_TXBUFFER_SIZE, UART_TXBUFFER_SIZE);
    tMboxInit(&txMbox, txMboxMsgBuffer, UART_TXBUFFER_SIZE);
}

/**
 * 等待接收数据包
 * @param packet 接收到的数据包指针存储地址
 * @return 0 无错误；1 有错误
 */
void UartRead (char * packet, uint32_t len) {
    while (len -- > 0) {
        void * ch;
        tMboxWait(&rxMbox, (void **)&ch, 0);
        *packet++ = (char)ch;
    }
}

/**
 * 写入数据包
 * @param packet 待写入的数据包
 */
void UartWrite (const char * packet, uint32_t len) {
    uint32_t status;
    const char * ch = packet;

    while (len-- > 0) {
        tSemWait(&txSem, 0);
        tMboxNotify(&txMbox, (void *)*ch++, tMBOXSendNormal);

        // 这里加循环反复调用，是考虑到中途可能发生其它中断延迟导致没有及时触发
        // 只有当硬件真正在空闲时，才手动触发一次
        status = USART_GetFlagStatus(USART1, USART_FLAG_TXE);
        if (status == SET) {
            USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
        }
    }
}
