CXXFLAGS = -Wall -Wextra -Werror -nostdlib -nostartfiles -nodefaultlibs -fno-leading-underscore -fno-rtti -fno-exceptions
BUILDDIR = build

SRCS = loader.s kernel.cpp DefaultConsole.cpp g++support.cpp gdt.cpp activate.s isr.cpp idt.cpp interrupt.s common.cpp timer.cpp
OBJS = $(addprefix $(BUILDDIR)/, $(patsubst %.cpp, %.o, $(patsubst %.s, %.o, $(SRCS))))

.PHONY: bootable builddir

bootable: floppy.img sizecalc.sh
	@./sizecalc.sh

builddir:
	mkdir -p $(BUILDDIR)

#floppy.img: boot.asm kernel.bin
#	nasm -f bin -o floppy.img -DKERNEL_IMAGE=\"kernel.bin\" boot.asm

floppy.img: floppy.s kernel.bin grub/stage1 grub/stage2
	nasm -f bin -o floppy.img floppy.s

kernel.bin: builddir $(OBJS) linker.ld
	ld -T linker.ld -o $@ $(OBJS)

$(BUILDDIR)/%.o: %.s
	nasm -f elf -o $@ $<

$(BUILDDIR)/%.o: %.cpp
	g++ $(CXXFLAGS) -o $@ -c $<

clean:
	rm floppy.img kernel.bin $(OBJS)
