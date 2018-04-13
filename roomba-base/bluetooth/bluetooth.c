#include "../uart/uart.h"
#include "util/delay.h"

typedef enum packetEnum {
	PSTART = '?',
	PEND = '.'
} packetEnum;

// void sendPacket(packet * p){
// 	//start packet
// 	uart_putchar('?');
	
// 	uart_putchar( p->roomba_x & 0xff );
// 	uart_putchar( (p->roomba_x>>8) & 0xff);

// 	uart_putchar( p->roomba_y & 0xff );
// 	uart_putchar( (p->roomba_y>>8) & 0xff);

// 	uart_putchar( p->servo_x & 0xff );
// 	uart_putchar( p->servo_y & 0xff );

// 	uart_putchar( p->laser & 0xff );

// 	//end packet
// 	uart_putchar('.');

// }

int buildInt(unsigned char H, unsigned char L){
	int i = 0;
	i = i | H<<8;
	i = i | L;
	return i;
}

void receivePacket(int * roomba_x, int * roomba_y, char * servo_x, char * servo_y, uint8_t * laser, uint8_t * changeMode){
	int start = 0;

	unsigned char x_H = 0;
	unsigned char x_L = 0;

	while(1){
		unsigned char newChar = uart_getchar();
		if(newChar == '?'){
			start = 0;
		}

		else if(start == 1){
			x_L = newChar;
		}

		else if(start == 2){
			x_H = newChar;
			*roomba_x = buildInt(x_H, x_L);
		}

		else if(start == 3){
			x_L = newChar;
		}

		else if(start == 4){
			x_H = newChar;
			*roomba_y = buildInt(x_H, x_L);
		}

		else if (start == 5) {
			*servo_x = newChar;
		}

		else if(start == 6){
			*servo_y = newChar;
		}

		else if(start == 7){
			*laser = newChar;
		}

		else if(start == 8){
			*changeMode = newChar;
		}

		else
		{
			break;
		}
	
		start++;
	}
}