.PHONY: bootable

bootable: floppy.img sizecalc.sh
	@./sizecalc.sh

floppy.img: floppy.s kernel.bin grub/stage1 grub/stage2
	nasm -f bin -o floppy.img floppy.s

kernel.bin: kernel.o loader.o
	ld -T linker.ld -o kernel.bin loader.o kernel.o

loader.o: loader.s
	nasm -f elf -o loader.o loader.s

kernel.o: kernel.c
	gcc -o kernel.o -c kernel.c -Wall -Wextra -Werror -nostdlib -nostartfiles -nodefaultlibs -fno-leading-underscore

clean:
	rm floppy.img kernel.bin *.o
