CXXFLAGS = -Wall -Wextra -Werror -nostdlib -nostartfiles -nodefaultlibs -fno-leading-underscore -fno-rtti -fno-exceptions
#BUILDDIR = build

SRCS = loader.s kernel.cpp DefaultConsole.cpp g++support.cpp gdt.cpp activate.s
OBJS = $(patsubst %.cpp, %.o, $(patsubst %.s, %.o, $(SRCS)))

.PHONY: bootable

bootable: floppy.img sizecalc.sh
	@./sizecalc.sh

#floppy.img: boot.asm kernel.bin
#	nasm -f bin -o floppy.img -DKERNEL_IMAGE=\"kernel.bin\" boot.asm

floppy.img: floppy.s kernel.bin grub/stage1 grub/stage2
	nasm -f bin -o floppy.img floppy.s

kernel.bin: $(OBJS) linker.ld
	ld -T linker.ld -o $@ $(OBJS)

%.o: %.s
	nasm -f elf -o $@ $<

%.o: %.cpp
	g++ $(CXXFLAGS) -o $@ -c $<

clean:
	rm floppy.img kernel.bin $(OBJS)
