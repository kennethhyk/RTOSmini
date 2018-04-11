make clean
avr-gcc -c -O2 -DF_CPU=16000000 -mmcu=atmega2560 -Wa,--gstabs -o cswitch.o cswitch.S
avr-gcc -DF_CPU=16000000 -mmcu=atmega2560 -Wall -Wextra -Os -S cswitch.o -c kernel.c -o kernel.o
avr-gcc -DF_CPU=16000000 -mmcu=atmega2560 -Wall -Wextra -Os -o kernel.elf kernel.c cswitch.s
avr-size --mcu=atmega2560 -C --format=avr kernel.elf
avr-objcopy -j .text -j .data -O ihex kernel.elf kernel.hex
avrdude -v -p m2560 -c wiring -P /dev/cu.usbmodem1421 -b 115200 -D -U flash:w:kernel.hex:i