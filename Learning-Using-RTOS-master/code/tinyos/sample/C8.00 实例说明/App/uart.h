/**
 * @brief 串口驱动
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#ifndef UART_H
#define UART_H

#define UART_BAUDRATE           115200
#define UART_RXBUFFER_SIZE      64
#define UART_TXBUFFER_SIZE      64

void UartInit (void);
void UartRead (char * packet, uint32_t len);
void UartWrite (const char * packet, uint32_t len);

#endif //UART_H
