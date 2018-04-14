#include "../uart/uart.h"
#include "util/delay.h"

// void sendPacket(packet * p){
sendPacket(int roomba_x, int roomba_y, char servo_x, char servo_y, uint8_t laser, uint8_t changeMode)
{
	//start packet
	uart_putchar('?');
	uart_putchar( roomba_x & 0xff );
	uart_putchar( (roomba_x>>8) & 0xff);
	uart_putchar( roomba_y & 0xff );
	uart_putchar( (roomba_y>>8) & 0xff);
	uart_putchar( servo_x & 0xff );
	uart_putchar( servo_y & 0xff );
	uart_putchar( laser & 0xff );
	uart_putchar( changeMode & 0xff );
	//end packet
	uart_putchar('.');
}

int buildInt(unsigned char H, unsigned char L){
	int i = 0;
	i = i | H<<8;
	i = i | L;
	return i;
}