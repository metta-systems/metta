
kernel.bin: kernel.o loader.o
	ld -T linker.ld -o kernel.bin loader.o kernel.o

loader.o: loader.s
	nasm -f elf -o loader.o loader.s

kernel.o: kernel.c
	gcc -o kernel.o -c kernel.c -Wall -Wextra -Werror -nostdlib -nostartfiles -nodefaultlibs
