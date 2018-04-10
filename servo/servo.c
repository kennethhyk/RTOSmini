/**
 * \file servo.h
 * \brief Simple control of a servo using hardware PWM.
 *
 * Created: 19/08/2013 9:16:26 AM
 *  \Author: Daniel McIlvaney
 */

// MG995 middle:1350 us, +/- 870 ms

uint16_t MIN_X = 200;
uint16_t MAX_X = 600;
uint16_t MIN_Y = 490;
uint16_t MAX_Y = 650;

void init_servo(){
	DDRE |= (1<<PE5);  //PWM Pins as Out
	DDRE |= (1<<PE4);  //PWM Pins as Out

	//Set to Fast PWM mode 15
	TCCR3A |= (1<<WGM30) | (1<<WGM31);
	TCCR3B |= (1<<WGM32) | (1<<WGM33);
	// TCCR3C |= (1<<WGM32) | (1<<WGM33);`

	TCCR3A |= (1<<COM3C1);
	TCCR3A |= (1<<COM3B1);

	TCCR3B |= (1<<CS31)|(1<<CS30);

	OCR3A = 5000;  //20 ms period
	OCR3B = 375;  // base positions pan
	OCR3C = MIN_Y + 100;  // base position tilt
}


void servo_set_pin_tilt_3(uint16_t pos) {
	if(pos < MIN_Y) {
		pos = MIN_Y;
	}
	if(pos > MAX_Y) {
		pos = MAX_Y;
	}

	printf("Writing %d to servo 3\n", pos);
	OCR3C = pos;
}

void servo_set_pin_pan_2(uint16_t pos) {
	if(pos < MIN_X) {
		pos = MIN_X;
	}

	if(pos > MAX_X) {
		pos = MAX_X;
	}

	printf("Writing %d to servo 2\n", pos);
	OCR3B = pos;
}
