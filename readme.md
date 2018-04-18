# RTOSmini

This project is an implementation of a real time OS, written in C, eventually used
to run dumb tesla using a wireless controller!

![dumbtesla](https://github.com/therafatm/RTOSmini/dumb_tesla.jpg "DUMB TESLA!")
![controller](https://github.com/therafatm/RTOSmini/controller.jpg "Controller!")

# What is it?

Dumb tesla is a modded iRobot roomba. It has a pan and tilt kit (two servo motors) with a laser
attached on top, and can move around autonomously, which is where it gets its name from. It is also
connected via bluetooth to a remote controller, which can control it too! The controller is also 
application code running on the same OS. When it was alive, dumb tesla could move around, aim, and shoot things with its laser, and could play the ocassional 8bit song. 

# THATS SO COOL! Pls tell me more!

Thanks! Both stations (roomba and controller) run on the Arduino MEGA2560 development board, and the only
code running on is the OS and the app code, i.e. no arduino libraries. All interfacing with hardware is 
done using the AVR instruction sets, for e.g. reading analog values from ports using the built in ADC, reading bumpers on the roomba, generating PWM via timers, etc. This project literally runs on the bare metal! 

The controller has two cheap PS2 joysticks, which control dumb tesla's movements, and movement/firing of the laser. By default, DT runs on autonomous mode, with controller commands overriding the semi-autonomous features, however semi-autonomous-ness can be completely turned off with the press of the left joystick.

# You wrote an OS? Holy .....

Thaaaats right. It even does context-switching manually; literally saves all the registers in memory, and loads it back. And the board has like 8k ram! The OS design is also pretty gnarly! This picture here kinda shows you how it works, but feel free to dig into `kernel.c` to find out more about the demons that live inside!

![OS Structure](https://github.com/therafatm/RTOSmini/OS_flow.png "OS!")

