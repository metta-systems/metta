CXXFLAGS = -Wall -Wextra -Werror -nostdlib -nostartfiles -nodefaultlibs -fno-leading-underscore -fno-rtti -fno-exceptions -DCONFIG_CPU_IA32_P4
BUILDDIR = build

SRCS = loader.s kernel.cpp DefaultConsole.cpp g++support.cpp gdt.cpp activate.s isr.cpp idt.cpp interrupt.s common.cpp timer.cpp kalloc.cpp paging.cpp process.s
OBJS = $(addprefix $(BUILDDIR)/, $(patsubst %.cpp, %.o, $(patsubst %.s, %.o, $(SRCS))))

.PHONY: bootable builddir

bootable: floppy.img
	@echo All done, start 'bochs' to boot.

floppy.img: kernel.bin update_image.sh
	@./update_image.sh

kernel.bin: $(OBJS) linker.ld
	@echo [Lnk] $@
	@ld -T linker.ld -o $@ $(OBJS)

$(BUILDDIR)/%.o: %.s
	@mkdir -p $(BUILDDIR)
	@echo [Asm]  $<
	@nasm -f elf -o $@ $<

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(BUILDDIR)
	@echo [C++]  $<
	@g++ -MD $(CXXFLAGS) -o $@ -c $<

clean:
	rm kernel.bin $(OBJS)

-include $(BUILDDIR)/*.d
