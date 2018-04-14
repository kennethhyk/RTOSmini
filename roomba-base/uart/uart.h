/**
 * @file   uart.c
 * @brief  Ported from justin tanners implementaion of
 *         UART Driver targetted for the AT90USB1287
 *
 */

#ifndef __UART_H__
#define __UART_H__

#include <avr/interrupt.h>
#include <stdint.h>
#define F_CPU 16000000UL

#define UART_BUFFER_SIZE    32

void uart_init();
void uart_init_0();
void uart_init_2();
void uart_putchar(unsigned char byte);
void uart_putchar_0(unsigned char byte);
void uart_putchar_2(unsigned char byte);
uint8_t uart_get_byte(int index);
uint8_t uart_get_byte_2(int index);
uint8_t uart_bytes_received(void);
uint8_t uart_bytes_received_2(void);
uint8_t uart_getchar();
uint8_t uart_getchar_0();
void uart_reset_receive(void);
void uart_reset_receive_2(void);
FILE uart_output = FDEV_SETUP_STREAM(uart_putchar_0, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar_0, _FDEV_SETUP_READ);
#endif
