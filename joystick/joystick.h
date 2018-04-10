#define numReadings 5

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