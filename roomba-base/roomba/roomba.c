#include "roomba.h"

void playSong(uint8_t song) {
	uart_putchar_2(141);
	uart_putchar_2(song);
}

void driveDirect(int vr, int vl) {
	uart_putchar_2(145);
	uart_putchar_2((vr>>8) & 0xff);
    uart_putchar_2((vr) & 0xff);
    uart_putchar_2((vl>>8) & 0xff);
    uart_putchar_2((vl) & 0xff);
}

void translateToMotion_roomba(int joystick_X,int joystick_Y){
    if(joystick_X == 9999 || joystick_Y == 9999) {
      //stop
      driveDirect(0, 0);
      return;
    }
    float power_X = 0;
    float power_Y = 0;
    power_X = abs(joystick_X-509)/515.00;
    power_Y = abs(joystick_Y-509)/515.00;
    int X = 0;
    int Y = 0;
    if(power_X > power_Y){
      if((joystick_X-509) > 0){
        X = 200 * power_X;
        driveDirect(X, X);
      }
      if((joystick_X-509) <= 0){
        X = -200 * power_X;
        driveDirect(X, X);
      }
    }
    if(power_Y >= power_X) {
      if((joystick_Y-509) > 0){
        X = 200 * power_Y;
        Y = -200 * power_Y;
        driveDirect(X, Y);
      }
      if((joystick_Y-509) <= 0){
        X = -200 * power_Y;
        Y = 200 * power_Y;
        driveDirect(X, Y);
      }
    }
}

void sense(){
	//query bumper and virtual wall sensors
 //    int garbage = 0;
	// for(int c = 0;c<3;c++){
	// 	garbage = UDR2;
	// }
    uart_putchar_2((uint8_t)142);
    uart_putchar_2((uint8_t)7);
    bumper = UDR2;
    if(UDR2 == 2 || UDR2 == 3){
        bumper = UDR2;
    }
    _delay_ms(15);
    uart_putchar_2((uint8_t)142);
    uart_putchar_2((uint8_t)13);
    virtual_wall = UDR2;
    if(virtual_wall == 2 || virtual_wall == 3){
        virtual_wall = 0;
    }
}
void cruise(){
	//left circle cruise
    if(choice == 0) {
        uart_putchar_2((uint8_t)137);
        uart_putchar_2((uint8_t)0);
        uart_putchar_2((uint8_t)200);
        uart_putchar_2((uint8_t)1);
        uart_putchar_2((uint8_t)244);
    }
    //right circle cruise
    if(choice == 1) {
        uart_putchar_2((uint8_t)137);
        uart_putchar_2((uint8_t)0);
        uart_putchar_2((uint8_t)200);
        uart_putchar_2((uint8_t)254);
        uart_putchar_2((uint8_t)12);
    }
    //forward cruise
    if(choice == 2) {
        driveDirect(200, 200);
    }
}
void escape(int X, int Y, int period){
    driveDirect(X, Y);
    if(period == 1){
        _delay_ms(200);
    }
    if(period == 2){
        _delay_ms(400);
    }
    if(period == 3){
        _delay_ms(2000);
    }
    if(period == 4){
        _delay_ms(1000);
    }
}

void spinMode(int joystick_Y){
    if(joystick_Y == 9999) {
      //stop
      driveDirect(0, 0);
      return;
    }
    float power_Y = 0;
    power_Y = abs(joystick_Y-509)/515.00;
    int X = 0;
    int Y = 0;
      if((joystick_Y-509) > 0){
        X = 200 * power_Y;
        Y = -200 * power_Y;
        driveDirect(X, Y);
      }
      if((joystick_Y-509) <= 0){
        X = -200 * power_Y;
        Y = 200 * power_Y;
        driveDirect(X, Y);
      }
}

void cruiseMode(){
    int roomba_x, roomba_y;
    char servo_x, servo_y;
    uint8_t laser, changeMode;
	srand((unsigned int) 28); //seed
    while(1){
        receivePacket(&roomba_x, &roomba_y, &servo_x, &servo_y, &laser, &changeMode);
        translate_to_servo_command(servo_x, servo_y);
        translate_to_laser(laser);
        if(roomba_x != 9999 || roomba_y != 9999){
            break;
        }
        sense();
        // printf("%x, %x\n", bumper, virtual_wall);
        if(bumper == 0 && virtual_wall == 0){
            //no collision
            cruise();
        } else {
        	if(virtual_wall == 1){
                escape(-200, -200, 1); //backward
                escape(200, -200, 3); //left
                virtual_wall = 0;
            }
            else if(bumper == 0x1){
                escape(-200, -200, 1); //backward
                escape(200, -200, 1); //left
                bumper = 0;
            }
            else if(bumper == 0x2){
                escape(-200, -200, 1); //backward
                escape(-200, 200, 1); //right
                bumper = 0;
            }
            else if(bumper == 0x3){
                escape(-200, -200, 1); //backward
                escape(200, -200, 3); //left
                bumper = 0;
            }
            choice = rand() % 3;
            _delay_ms(100);
        }
    }    
}