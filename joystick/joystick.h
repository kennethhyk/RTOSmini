#define numReadings 5

//pin configuration
int pin_joystick_X = 0;
int pin_joystick_Y = 1;
int pin_joystick_Btn = 6;
int pin_laser = 36;
int pin_horizontal = 9;
int pin_vertical = 10;
int pin_idle = 8;

// joystick initialization
int joystick_RAW_X = 0;
int joystick_RAW_Y = 0;
int joystick_X;
int last_joystick_X;
int joystick_Y;
int last_joystick_Y;

int joystick_X_base = 496;
int joystick_Y_base = 494;

int joystick_Btn = 0;
int joystick_centered = 1;
int deadband = 30;

// averaging initialization
int readings_X[numReadings];
int readIndex_X = 0;
int total_X = 0;
int average_X = 0;
int readings_Y[numReadings];
int readIndex_Y = 0;
int total_Y = 0;
int average_Y = 0;

// laser 
int laser_on = 0;

unsigned long last_start_time;
unsigned long cumulative_laser_time = 0;