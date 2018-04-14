/**
 * @file   uart.c
 * @brief  Ported from justin tanners implementaion of
 *         UART Driver targetted for the AT90USB1287
 *
 */

#include "uart.h"
#define F_CPU 16000000UL

#ifndef BAUD
#define BAUD 19200
#endif

#include <util/setbaud.h>

static volatile uint8_t uart_buffer[UART_BUFFER_SIZE];
static volatile uint8_t uart_buffer_index;

static volatile uint8_t uart_buffer_2[UART_BUFFER_SIZE];
static volatile uint8_t uart_buffer_index_2;
/**
 * Initalize UART
 *
 */
void uart_init()
{
	UBRR1H = UBRRH_VALUE;
    UBRR1L = UBRRL_VALUE;

    // UCSR0A = _BV(U2X0);
    #if USE_2X
        UCSR1A |= _BV(U2X1);
    #else
        UCSR1A &= ~(_BV(U2X1));
    #endif

	// UCSR1B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
    UCSR1B = _BV(RXEN1) | _BV(TXEN1);
	UCSR1C = _BV(UCSZ11) | _BV(UCSZ10);
}

void uart_init_0()
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

    // UCSR0A = _BV(U2X0);
    #if USE_2X
        UCSR0A |= _BV(U2X0);
    #else
        UCSR0A &= ~(_BV(U2X0));
    #endif

    // UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}

void uart_init_2()
{
    UBRR2H = UBRRH_VALUE;
    UBRR2L = UBRRL_VALUE;

    // UCSR0A = _BV(U2X0);
    #if USE_2X
        UCSR2A |= _BV(U2X2);
    #else
        UCSR2A &= ~(_BV(U2X2));
    #endif

    UCSR2B = _BV(RXEN2) | _BV(TXEN2) | _BV(RXCIE2);
    // UCSR2B = _BV(RXEN0) | _BV(TXEN0);
    UCSR2C = _BV(UCSZ21) | _BV(UCSZ20);
}

/**
 * Transmit one byte
 * NOTE: This function uses busy waiting
 *
 * @param byte data to trasmit
 */
void uart_putchar(unsigned char byte)
{
    /* wait for empty transmit buffer */
    if (byte == '\n') {
        uart_putchar('\r');
    }
    while (!( UCSR1A & (1 << UDRE1)));

    /* Put data into buffer, sends the data */
    UDR1 = byte;
}

void uart_putchar_0(unsigned char byte)
{   
    /* wait for empty transmit buffer */
    if (byte == '\n') {
        uart_putchar_0('\r');
    }
    while (!( UCSR0A & (1 << UDRE0)));

    /* Put data into buffer, sends the data */
    UDR0 = byte;
}

void uart_putchar_2(unsigned char byte)
{   
    /* wait for empty transmit buffer */
    if (byte == '\n') {
        uart_putchar_2('\r');
    }
    while (!( UCSR2A & (1 << UDRE2)));

    /* Put data into buffer, sends the data */
    UDR2 = byte;
}

uint8_t uart_getchar()
{
    loop_until_bit_is_set(UCSR1A, RXC1);
    return UDR1;
}

uint8_t uart_getchar_0()
{
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

/**
 * Receive a single byte from the receive buffer
 *
 * @param index
 *
 * @return
 */
uint8_t uart_get_byte(int index)
{
    if (index < UART_BUFFER_SIZE)
    {
        return uart_buffer[index];
    }
    return 0;
} 

uint8_t uart_get_byte_2(int index)
{
    if (index < UART_BUFFER_SIZE)
    {
        return uart_buffer_2[index];
    }
    return 0;
}
/**
 * Get the number of bytes received on UART
 *
 * @return number of bytes received on UART
 */
uint8_t uart_bytes_received(void)
{
    return uart_buffer_index;
}

uint8_t uart_bytes_received_2(void)
{
    return uart_buffer_index_2;
}

/**
 * Prepares UART to receive another payload
 *
 */
void uart_reset_receive(void)
{
    uart_buffer_index = 0;
}

void uart_reset_receive_2(void)
{
    uart_buffer_index_2 = 0;
}

ISR(USART2_RX_vect)
{
    while(!(UCSR2A & (1<<RXC2)));
    uart_buffer_2[uart_buffer_index_2] = UDR2;
    uart_buffer_index_2 = (uart_buffer_index_2 + 1) % UART_BUFFER_SIZE;
}


