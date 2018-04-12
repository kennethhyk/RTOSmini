#include "LED_Test.h"

void init_LED_idle()
{
	DDRB=(1<<DDB2);
	PORTB = 0x00;
}

void init_LED_B3()
{
	DDRB=(1<<DDB3);
	PORTB = 0x00;
}

void init_LED_B6()
{
	DDRB=(1<<DDB6);
	PORTB = 0x00;
}

void init_LED_B5()
{
	DDRB=(1<<DDB5);
	PORTB = 0x00;
}

void toggle_LED_idle(){
	PORTB ^= (1<<PORTB2);
}

void toggle_LED_B3()
{
	PORTB ^= (1<<PORTB3);
}

void toggle_LED_B5(){
	PORTB ^= (1<<PORTB5);
}

void toggle_LED_B6()
{
	PORTB ^= (1<<PORTB6);
}
