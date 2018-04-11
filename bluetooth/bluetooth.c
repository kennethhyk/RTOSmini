#include "bluetooth.h"
#include "../uart/uart.h"
#include "util/delay.h"

typedef enum packetEnum {
	PSTART = '?',
	PEND = '.'
} packetEnum;

void sendPacket(int x, int y){
	//start packet
	uart_putchar('?');
	//motors
	uart_putchar(x&0xff);
	uart_putchar((x>>8)&0xff);
	uart_putchar(y&0xff);
	uart_putchar((y>>8)&0xff);
	//end packet
	uart_putchar('.');

}

int buildInt(unsigned char H, unsigned char L){
	int i = 0;
	i = i | H<<8;
	i = i | L;
	return i;
}

void receivePacket(int *x1, int *y1){
	int start = 0;
	unsigned char x_H = 0;
	unsigned char x_L = 0;
	unsigned char y_H = 0;
	unsigned char y_L = 0;
	while(1){
		unsigned char newChar = uart_getchar();
		if(newChar == '?'){
			start = 0;
			start++;
		}
		else if(start == 1){
			x_L = newChar;
			start++;
		}
		else if(start == 2){
			x_H = newChar;
			start++;
		}
		else if(start == 3){
			y_L = newChar;
			start++;
		}
		else if(start == 4){
			y_H = newChar;
			start++;
		}
		else if(start == 5){
			*x1 = buildInt(x_H, x_L);
			*y1 = buildInt(y_H, y_L);
			break;
		}
	}
}