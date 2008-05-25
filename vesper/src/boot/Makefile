CXXFLAGS = -Wall -Wextra -Werror -nostdlib -nostartfiles -nodefaultlibs -fno-leading-underscore -fno-rtti -fno-exceptions

.PHONY: bootable

bootable: floppy.img sizecalc.sh
	@./sizecalc.sh

#floppy.img: boot.asm kernel.bin
#	nasm -f bin -o floppy.img -DKERNEL_IMAGE=\"kernel.bin\" boot.asm

floppy.img: floppy.s kernel.bin grub/stage1 grub/stage2
	nasm -f bin -o floppy.img floppy.s

kernel.bin: kernel.o loader.o DefaultConsole.o
	ld -T linker.ld -o kernel.bin loader.o kernel.o DefaultConsole.o

loader.o: loader.s
	nasm -f elf -o loader.o loader.s

kernel.o: kernel.cpp
	g++ -o kernel.o -c kernel.cpp $(CXXFLAGS)

DefaultConsole.o: DefaultConsole.cpp
	g++ -o DefaultConsole.o -c DefaultConsole.cpp $(CXXFLAGS)

clean:
	rm floppy.img kernel.bin *.o
