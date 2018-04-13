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