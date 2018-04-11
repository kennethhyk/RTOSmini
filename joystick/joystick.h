#define num_readings 5
#define num_joysticks 2

typedef enum joystick_identifier {
    SERVO = 0,
    ROOMBA
} JOYSTICK_NUM;

//pin configuration
int servo_pin_joystick_X = 0;
int servo_pin_joystick_Y = 1;
int roomba_pin_joystick_X = 2;
int roomba_pin_joystick_Y = 3;

// joystick initialization
int joystick_RAW_X[num_joysticks] = {0, 0};
int joystick_RAW_Y[num_joysticks] = {0, 0};
int joystick_X[num_joysticks] = {0, 0};
int joystick_Y[num_joysticks] = {0, 0};

int joystick_X_base[num_joysticks] = {501, 501};
int joystick_Y_base[num_joysticks] = {510, 510};
int deadband = 60;

// averaging initialization
int readings_X[num_joysticks][num_readings];

int readIndex_X[num_joysticks] = {0, 0};
int total_X[num_joysticks] = {0, 0};
int average_X[num_joysticks] = {0, 0};

int readings_Y[num_joysticks][num_readings];

int readIndex_Y[num_joysticks] = {0, 0};
int total_Y[num_joysticks] = {0, 0};
int average_Y[num_joysticks] = {0, 0};

// laser 
int laser_on = 0;
unsigned long last_start_time;
unsigned long cumulative_laser_time = 0;

// packet
typedef struct packet {
    uint16_t servo_x;
    uint16_t servo_y;
    uint16_t roomba_x;
    uint16_t roomba_y;
    uint16_t laser;
} packet;

void initReadings(){
    int i,j = 0;
    for (i; i < num_readings; i++){
        for (j; j < num_joysticks; j++){
            readings_X[j][i] = 0;
        }
    }

    i = 0; j = 0;
    for (i; i < num_readings; i++){
        for (j; j < num_joysticks; j++){
            readings_Y[j][i] = 0;
        }
    }
}