/**
 * \file servo.h
 * \brief Simple control of a servo using hardware PWM.
 *
 * Created: 19/08/2013 9:16:26 AM
 *  \Author: Daniel McIlvaney
 */

// MG995 middle:1350 us, +/- 870 ms

// uint16_t MIN_X = 200;
// uint16_t MAX_X = 600;
// uint16_t MIN_Y = 490;
// uint16_t MAX_Y = 650;
uint16_t MIN_X = 300;
uint16_t MAX_X = 500;
uint16_t MIN_Y = 300;
uint16_t MAX_Y = 500;

uint16_t i = 400;
uint16_t j = 400; // to account for pan motor height
uint16_t pan_offset = 1;
uint16_t tilt_offset = 1;

int laser_on = 0;
unsigned long last_start_time;
unsigned long cumulative_laser_time = 0;

void init_servo(){
	DDRE |= (1<<PE5);  //PWM Pins as Out
	DDRE |= (1<<PE4);  //PWM Pins as Out

	//Set to Fast PWM mode 15
	TCCR3A |= (1<<WGM30) | (1<<WGM31);
	TCCR3B |= (1<<WGM32) | (1<<WGM33);

	TCCR3A |= (1<<COM3C1);
	TCCR3A |= (1<<COM3B1);

	TCCR3B |= (1<<CS31)|(1<<CS30);

	OCR3A = 5000;  //20 ms period
	OCR3B = 400;  // base positions pan
	OCR3C = 400;  // base position tilt
}

void set_laser()
{
	// use PB5
	int pin = 6;
	PINB |= ~(1 << pin);
}

void clear_laser()
{
	// use PB5
	int pin = 6;
	PINB &= (~(1) << pin);
}

void translate_to_laser(uint8_t l) {
	if(l == 1){
		// set_laser();
		PORTB = 0xFF;
	} 
	else if(l == 0){
		// clear_laser();
		PORTB = 0x00;
	}
}

void servo_set_pin_tilt_3(uint16_t pos) {
	if(pos < MIN_Y) {
		pos = MIN_Y;
	}
	if(pos > MAX_Y) {
		pos = MAX_Y;
	}

	// printf("Writing %d to servo 3\n", pos);
	OCR3B = pos;
}

void servo_set_pin_pan_2(uint16_t pos) {
	if(pos < MIN_X) {
		pos = MIN_X;
	}

	if(pos > MAX_X) {
		pos = MAX_X;
	}

	// printf("Writing %d to servo 2\n", pos);
	OCR3C = pos;
}

void translate_to_servo_command(char x, char y)
{
	uint16_t angle_x = 0;
	uint16_t angle_y = 0;
	uint16_t laser = 0;
	if(x != '&'){
		if (x == 'R')
		{
			// right
			if ((i - pan_offset) > MIN_X)
			{
				i -= pan_offset;
				servo_set_pin_pan_2(i);
			}
		}

		else if (x == 'L')
		{
			// left
			if ((i + pan_offset) < MAX_X)
			{
				i += pan_offset;
				servo_set_pin_pan_2(i);
			}
		}
	}
	if(y != '&'){
		if (y == 'D')
		{
			// down 
			if ((j - tilt_offset) > MIN_Y)
			{
				j -= tilt_offset;
				servo_set_pin_tilt_3(j);
			}
		}

		if (y == 'U')
		{
			// up
			if ((j + tilt_offset) < MAX_Y)
			{
				j += tilt_offset;
				servo_set_pin_tilt_3(j);
			}
		}
	}
}