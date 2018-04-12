#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <math.h>
#include "joystick.h"
#include "../servo/servo.c"
#include "../os.h"

void set_laser()
{
	// use PB5
	int pin = 6;
	PINB |= ~(1 << pin);
}

void clear_laser()
{
	// use PB5
	int pin = 5;
	PINB &= (~(1) << pin);
}

uint16_t i = 500;
uint16_t j = 500; // to account for pan motor height
uint16_t pan_offset = 10;
uint16_t tilt_offset = 5;

void init_joystick()
{
	initReadings();
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

bool read_joystick(uint16_t pin_x, uint16_t pin_y, JOYSTICK_NUM joystick_id)
{
	joystick_RAW_X[joystick_id] = analog_read(pin_y);
	joystick_RAW_Y[joystick_id] = analog_read(pin_x);

	int a = joystick_RAW_X[joystick_id] - joystick_X_base[joystick_id];
	int b = joystick_RAW_Y[joystick_id] - joystick_Y_base[joystick_id];

	if ((square(a) + square(b)) > square(deadband))
	{
		total_X[joystick_id] = total_X[joystick_id] - readings_X[joystick_id][readIndex_X[joystick_id]];
		readings_X[joystick_id][readIndex_X[joystick_id]] = joystick_RAW_X[joystick_id];
		total_X[joystick_id] = total_X[joystick_id] + readings_X[joystick_id][readIndex_X[joystick_id]];
		readIndex_X[joystick_id] = readIndex_X[joystick_id] + 1;
		if (readIndex_X[joystick_id] >= num_readings)
		{
			readIndex_X[joystick_id] = 0;
		}

		joystick_X[joystick_id] = total_X[joystick_id] / num_readings;

		total_Y[joystick_id] = total_Y[joystick_id] - readings_Y[joystick_id][readIndex_Y[joystick_id]];
		readings_Y[joystick_id][readIndex_Y[joystick_id]] = joystick_RAW_Y[joystick_id];
		total_Y[joystick_id] = total_Y[joystick_id] + readings_Y[joystick_id][readIndex_Y[joystick_id]];
		readIndex_Y[joystick_id] = readIndex_Y[joystick_id] + 1;
		if (readIndex_Y[joystick_id] >= num_readings)
		{
			readIndex_Y[joystick_id] = 0;
		}

		joystick_Y[joystick_id] = total_Y[joystick_id] / num_readings;
		return true;
	}

	return false;
}

void drive_laser()
{
	// calculate cumulative_laser_time
	// 30 * 1000 / 10 ms interrupts
	if (cumulative_laser_time > 40)
	{
		// turn laser off
		clear_laser();
		laser_on = 0;
	}
	// joystick pressed
	// due to PUPDR, PINC == 0 means laser is on
	else if (PINC == 0 || laser_on == 1)
	{

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
			if (PINC != 0)
			{
				laser_on = 0;
				clear_laser();
				// calculate cumulative_laser_time
				unsigned long laser_on_time = Now() - last_start_time;
				last_start_time = 0;
				printf("laser turned on for %ld ticks\n", laser_on_time);
			}
		}
	}
}

// void drive_servo()
// {
// 	uint16_t pin_x = 0;
// 	uint16_t pin_y = 1;
// 	JOYSTICK_NUM joystick_id = SERVO;
// 	while (1)
// 	{
// 		if (read_joystick(pin_x, pin_y, joystick_id) == true)
// 		{
// 			translate_to_servo_command(joystick_id);
// 		};
// 		drive_laser();
// 		Task_Next();
// 	}
// }

void read_photoressistor()
{
	uint16_t pin = 3;
	int d = analog_read(pin);
}

bool within_deadband(int value, int base)
{
	if (value > base + 25)
		return false;
	if (value < base - 25)
		return false;
	return true;
}