rm cswitch.o active.elf active.hex active.o
avr-gcc -c -O2 -DF_CPU=16000000 -mmcu=atmega2560 -Wa,--gstabs -o cswitch.o cswitch.S
avr-gcc -DF_CPU=16000000 -mmcu=atmega2560 -Wall -Wextra -Os -S cswitch.o -c active.c -o active.o
avr-gcc -DF_CPU=16000000 -mmcu=atmega2560 -Wall -Wextra -Os -o active.elf active.c cswitch.s
avr-objcopy -j .text -j .data -O ihex active.elf active.hex
avrdude -v -p m2560 -c wiring -P /dev/cu.usbmodem1421 -b 115200 -D -U flash:w:active.hex:i