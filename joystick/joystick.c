#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <math.h>
#include "joystick.h"
#include "../servo/servo.c"
#include "../os.h"

#define get_bit(p, m) ((p) & (m))

// float get_current_time_msec()
// {
// 	struct timeval current_time;
//     return (current_time.tv_sec) * 1000.0f + (current_time.tv_usec) / 1000.0f;
// }

void set_laser(){
  // use PB5 
  int pin = 5;
  PINB |= ~(1 << pin);
//   printf("SET PIN: %d\n", PINB);
}

void clear_laser(){
  // use PB5
  int pin = 5;
  PINB &= (~(1) << pin);
//   printf("CLEARED PIN: %d\n", PINB);
}

uint16_t i = 375;
uint16_t j = 490; // to account for pan motor height
uint16_t pan_offset = 10;
uint16_t tilt_offset = 5;

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

	// joystick button as input
	DDRC = 0x00;

	// set laser as output
	DDRB = 0xFF;
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
	while ((ADCSRA & (1 << ADSC)))
		;
	/* We setup the ADC to shift input to left, so we simply return the High register. */

	uint16_t lowADC = ADCL;
	uint16_t highADC = ADCH;
	return (lowADC >> 6) | (highADC << 2);
}

void read_joystick()
{
	while(1)
	{
		joystick_RAW_X = analog_read(pin_joystick_X);
		joystick_RAW_Y = analog_read(pin_joystick_Y);

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
			translate_to_servo_command();
		}

		// joystick pressed
		// due to PUPDR, PINC == 0 means laser is on
		if (PINC == 0 || laser_on == 1)
		{	
			// calculate cumulative_laser_time
			// 30 * 1000 / 10 ms interrupts
			if (cumulative_laser_time > 3000){
				// turn laser off
				Task_Next();
			}

			// if laser off
			if (laser_on == 0)
			{
				// turn on laser toggle
				laser_on = 1;
				// write to laser -- fill in code
				set_laser();
				// set last start time
				last_start_time = Now();
			} 
			else 
			{
				// switch off laser when button is released
				if (PINC != 0){
					laser_on = 0;
					clear_laser();
					last_start_time = 0;
					// calculate cumulative_laser_time
					unsigned long laser_on_time = Now() - last_start_time;
					// printf("laser turned on for %ld ticks\n", laser_on_time);
				}
			}
		}

		read_photoressistor();
		_delay_ms(1000);
		Task_Next();
	}
}

void read_photoressistor(){
	uint16_t pin = 3;
    int d = analog_read(pin);
    printf("photoressistor value: %d\n", d);
}

bool within_deadband(int value, int base)
{
	if (value > base + 15)
		return false;
	if (value < base - 15)
		return false;
	return true;
}

void translate_to_servo_command()
{
	uint16_t angle_x = 0;
	uint16_t angle_y = 0;
	uint16_t laser = 0;

	printf("JX: %d, JY: %d\n", joystick_X, joystick_Y);

	if (joystick_X < joystick_X_base && !within_deadband(joystick_X, joystick_X_base))
	{
		// turn left
		// decrease value unless min threshold is reached
		if ((i - pan_offset) > MIN_X)
		{
			i -= pan_offset;
			// printf("ugh1\n");
			servo_set_pin_pan_2(i);
		}
	}

	else if (joystick_X > joystick_X_base && !within_deadband(joystick_X, joystick_X_base))
	{
		if ((i + pan_offset) < MAX_X)
		{
			i += pan_offset;
			// printf("ugh2\n");
			servo_set_pin_pan_2(i);
		}
	}

	if (joystick_Y < joystick_Y_base && !within_deadband(joystick_Y, joystick_Y_base))
	{
		if ((j - tilt_offset) > MIN_Y)
		{
			j -= tilt_offset;
			// printf("here1\n");
			servo_set_pin_tilt_3(j);
		}
	}

	else if (joystick_Y > joystick_Y_base && !within_deadband(joystick_Y, joystick_Y_base))
	{
		if ((j + tilt_offset) < MAX_Y)
		{
			j += tilt_offset;
			// printf("here2\n");
			servo_set_pin_tilt_3(j);
		}
	}
}