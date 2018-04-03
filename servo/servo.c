/**
 * \file servo.h
 * \brief Simple control of a servo using hardware PWM.
 *
 * Created: 19/08/2013 9:16:26 AM
 *  \Author: Daniel McIlvaney
 */

// MG995 middle:1350 us, +/- 870 ms

int i = 400;
int j = 340;

void init_servo(){
	DDRE |= (1<<PE5);  //PWM Pins as Out
	DDRE |= (1<<PE4);  //PWM Pins as Out

	//Set to Fast PWM mode 15
	TCCR3A |= (1<<WGM30) | (1<<WGM31);
	TCCR3B |= (1<<WGM32) | (1<<WGM33);

	TCCR3A |= (1<<COM3C1);
	TCCR3A |= (1<<COM3B1);

	TCCR3B |= (1<<CS31)|(1<<CS30);

	OCR3A=7000;  //20 ms period
	OCR3B = 375;
	OCR3C = 375;
}

void servo_set_pin_tilt_3(uint16_t pos) {
	if(pos <= 300) {
		pos = 325;
	} else if (pos >= 450) {
		pos = 450;
	}
    OCR3B = pos;
}

void servo_set_pin_pan_2(uint16_t pos) {
	if(pos <= 135) {
		pos = 135;
	} else if(pos >= 550) {
		pos = 550;
	}
	OCR3C = pos;
}

void servo1(char c){
	if(c=='D'){
		if((i+15)<550){
			i+=15;
			servo_set_pin_pan_2(i);
		}
	}else if(c=='U'){
		if((i-15)>250){
			i-=15;
			servo_set_pin_pan_2(i);
		}
	}
	return;
}

void servo2(char c){
	if(c=='L'){
		if((j+15)<550){
			j+=15;
			servo_set_pin_tilt_3(j);
		}
	}else if(c=='R'){
		if((j-15)>300){
			j-=15;
			servo_set_pin_tilt_3(j);
		}
	}
	return;
}
