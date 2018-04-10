#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define numReadings 5
// float time_msec(struct timeval t)
// {
//     return (t.tv_sec) * 1000.0f + (t.tv_usec) / 1000.0f;
// }

//pin configuration
int pin_joystick_X = 0;
int pin_joystick_Y = 1;
int pin_joystick_Btn = 36;
int pin_laser = 11;
int pin_horizontal = 9;
int pin_vertical = 10;
int pin_idle = 8;

//initialization
int joystick_RAW_X = 0;
int joystick_RAW_Y = 0;
int joystick_X;
int last_joystick_X;
int joystick_Y;
int last_joystick_Y;

int joystick_X_base = 496;
int joystick_Y_base = 520;

int joystick_Btn = 0;
int joystick_centered = 1;
int deadband = 35;

int readings_X[numReadings];
int readIndex_X = 0;
int total_X = 0;
int average_X = 0;
int readings_Y[numReadings];
int readIndex_Y = 0;
int total_Y = 0;
int average_Y = 0;

int laser_toggle = 0;
int laser_toggle_changeable = 0;
float laser_toggle_timer = 0;

uint16_t i = 375;
uint16_t j = 450;

uint16_t min_x = 200;
uint16_t max_x = 600;

uint16_t min_y = 500;
uint16_t max_y = 600;

void init_joystick()
{
	/* Set PORTC to receive analog inputs */
	DDRC = (DDRC & 0x00);

	/* Set ADC configurations */
	// Set ADC prescalar to 128 - 125KHz sample rate @ 16MHz
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	// Set ADC reference to AVCC
	ADMUX |= (1 << REFS0);

	// Left adjust ADC result to allow easy 8 bit reading
	ADMUX |= (1 << ADLAR);

	// Enable ADC
	ADCSRA |= (1 << ADEN);

	//Start ADC conversion
	ADCSRA |= (1 << ADSC);
}

/**
 * Ported and adjusted from
 * https://garretlab.web.fc2.com/en/arduino/inside/arduino/wiring_analog.c/analogRead.html
 */
uint16_t analog_read(uint16_t c)
{
	/* We're using Single Ended input for our ADC readings, this requires some
	 * work to correctly set the mux values between the ADMUX and ADCSRB registers. 
	 * ADMUX contains the four LSB of the multiplexer, while the fifth bit is kept
	 * within the ADCSRB register. Given the specifications, we want to keep the 
	 * three least significant bits as is, and check to see if the fourth bit is set, if it
	 * is, we need to set the mux5 pin. 
	 */

	/* Set the three LSB of the Mux value. */
	/* Caution modifying this line, we want MUX4 to be set to zero, always */
	ADMUX = (ADMUX & 0xF0) | (0x07 & c);
	/* We set the MUX5 value based on the fourth bit of the channel, see page 292 of the 
	 * ATmega2560 data sheet for detailed information */
	ADCSRB = (ADCSRB & 0xF7) | (c & (1 << MUX5));

	/* We now set the Start Conversion bit to trigger a fresh sample. */
	ADCSRA |= (1 << ADSC);
	/* We wait on the ADC to complete the operation, when it completes, the hardware
    will set the ADSC bit to 0. */
	while ((ADCSRA & (1 << ADSC)));
	/* We setup the ADC to shift input to left, so we simply return the High register. */

	uint16_t lowADC = ADCL;
	uint16_t highADC = ADCH;
	return (lowADC >> 6) | (highADC << 2);
}

void readJoyStick()
{
	joystick_RAW_X = analog_read(pin_joystick_X);
	joystick_RAW_Y = analog_read(pin_joystick_Y);
	
	// joystick_Btn = bit_get(PINC, pin_joystick_Btn);
	// struct timeval t0;
	// struct timeval t1;

	int a = joystick_RAW_X - joystick_X_base;
	int b = joystick_RAW_Y - joystick_Y_base;
	// printf("X: %d\nY: %d\n", joystick_RAW_X, joystick_RAW_Y);

	if ((square(a) + square(b)) > square(deadband))
	{
		joystick_centered = 0;
		total_X = total_X - readings_X[readIndex_X];
		readings_X[readIndex_X] = joystick_RAW_X;
		total_X = total_X + readings_X[readIndex_X];
		readIndex_X = readIndex_X + 1;
		if (readIndex_X >= numReadings)
		{
			readIndex_X = 0;
		}
		joystick_X = total_X / numReadings;
		// joystick_X = joystick_RAW_X;

		total_Y = total_Y - readings_Y[readIndex_Y];
		readings_Y[readIndex_Y] = joystick_RAW_Y;
		total_Y = total_Y + readings_Y[readIndex_Y];
		readIndex_Y = readIndex_Y + 1;
		if (readIndex_Y >= numReadings)
		{
			readIndex_Y = 0;
		}
		joystick_Y = total_Y / numReadings;
		// joystick_Y = joystick_RAW_Y;
		// printf("X: %d\nY: %d\n", joystick_X, joystick_Y);
		translateToMotion();
	}
	else
	{
		joystick_centered = 1;
	}
	// if (joystick_Btn == 0)
	// {
		// if (laser_toggle_changeable)
		// {
		// 	if (laser_toggle == 0) laser_toggle = 1;
		// 	else if (laser_toggle == 1) laser_toggle = 0;
		// 	laser_toggle_timer = time_msec(t0);
		// 	laser_toggle_changeable = 0;
		// }
		// else
		// {
		// 	float current_time = time_msec(t0);
		// 	if (current_time - laser_toggle_timer) >= 300)
		// 		laser_toggle_changeable = 1;
		// }
	// }
}

bool withinDeadBand(int value, int base){
	if (value > base + 15) return false;
	if (value < base - 15) return false;
	return true;
}

void translateToMotion()
{
	uint16_t angle_x = 0;
	uint16_t angle_y = 0;
	uint16_t laser = 0;

	printf("JX: %d, JY: %d\n", joystick_X, joystick_Y);

	if (joystick_X < joystick_X_base && !withinDeadBand(joystick_X, joystick_X_base))
	{
		// turn left
		// decrease value unless min threshold is reached
		if((i-10)>min_x){
			i-=10;
			// printf("ugh1\n");
			servo_set_pin_pan_2(i);
		}	
	}

	else if (joystick_X > joystick_X_base && !withinDeadBand(joystick_X, joystick_X_base))
	{
		if((i+10)<max_x){
			i+=10;
			// printf("ugh2\n");
			servo_set_pin_pan_2(i);
		}
	}
	
	if (joystick_Y < joystick_Y_base && !withinDeadBand(joystick_Y, joystick_Y_base))
	{
		if((j-10)>min_y){
			j-=10;
			// printf("here1\n");
			servo_set_pin_tilt_3(j);
		}
	}

	else if (joystick_Y > joystick_Y_base && !withinDeadBand(joystick_Y, joystick_Y_base))
	{
		if((j+10)<max_y){
			j+=10;
			// printf("here2\n");
			servo_set_pin_tilt_3(j);
		}
	} 

}