CFLAGS = -Wall -Wextra -Werror -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -fno-stack-protector -fno-leading-underscore -fno-exceptions -DCONFIG_CPU_IA32_P4
CXXFLAGS = $(CFLAGS) -fno-rtti
BUILDDIR = build

SRCS = loader.s g++support.cpp ctype.c string.c vsprintf.c common.cpp kernel.cpp kalloc.cpp DefaultConsole.cpp gdt.cpp activate.s isr.cpp idt.cpp interrupt.s timer.cpp paging.cpp process.s task.cpp
LINK = $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(patsubst %.s, %.o, $(SRCS))))
OBJS = $(addprefix $(BUILDDIR)/, $(LINK))

.PHONY: bootable builddir

bootable: builddir floppy.img
	@echo All done, start 'bochs' to boot.

floppy.img: kernel.bin update_image.sh
	@./update_image.sh

kernel.bin: $(OBJS) linker.ld
	@echo [Lnk] $@
	@ld -T linker.ld -o $@ $(LINK)

builddir:
	@mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: %.s
	@echo [Asm]  $<
	@nasm -f elf -o $@ $<

# Use g++ to compile C code.
$(BUILDDIR)/%.o: %.c
	@echo [C++]  $<
	@g++ -MD $(CXXFLAGS) -o $@ -c $<

$(BUILDDIR)/%.o: %.cpp
	@echo [C++]  $<
	@g++ -MD $(CXXFLAGS) -o $@ -c $<

clean:
	rm kernel.bin $(OBJS)

-include $(BUILDDIR)/*.d
