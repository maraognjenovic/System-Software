#! /bin/sh

./linker -hex -place=ivt@0x0000 -o program.hex test/interrupts.o test/main.o

