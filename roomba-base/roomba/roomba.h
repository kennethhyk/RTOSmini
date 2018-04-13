#include <util/delay.h>

extern uint8_t bumper = 0;
extern uint8_t virtual_wall = 0;
extern int choice = 0;

void translateToMotion_roomba(int joystick_X,int joystick_Y);
void sense();
void cruise();
void escape(int X, int Y, int period);
void cruiseMode();