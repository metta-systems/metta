#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "paging.h"
#include "DefaultConsole.h"

extern "C" void kmain(void *mbd, unsigned int magic);

void kmain(void *mbd, unsigned int magic)
{
	kconsole.locate(5, 0);
	kconsole.set_color(LIGHTRED);
	kconsole.print("Reloading GDT...\n");
	GlobalDescriptorTable::init();

	kconsole.print("Loading IDT...\n");
	InterruptDescriptorTable::init();

	kconsole.locate(7, 20);
	kconsole.set_color(LIGHTGRAY);
	kconsole.print("Hello,\n");
	kconsole.newline();
	kconsole.print_byte(0xAB);
	kconsole.print_char('_');
	kconsole.print("World!\n\n");
	kconsole.print_hex(magic);
	kconsole.locate(11, 22);
	kconsole.print_int(-21954321);
	kconsole.scroll_up();
	kconsole.set_color(YELLOW);
	kconsole.debug_showmem(mbd, 135);

	asm volatile ("int $0x3");
	asm volatile ("int $0x4");

// 	Timer::init();
// 	kconsole.wait_ack();
// 	asm volatile ("sti");

	Paging::self();
	kconsole.print("Enabling paging...\n");
	uint32_t *ptr = (uint32_t*)0xA0000000;
    uint32_t do_page_fault = *ptr;
    UNUSED(do_page_fault);
}
