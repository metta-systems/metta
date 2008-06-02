#include "kalloc.h"
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

	uint32_t a = kmalloc(8);
	kconsole.print("Enabling paging...\n");
	Paging::self();
	uint32_t b = kmalloc(8);
	uint32_t c = kmalloc(8);
	kconsole.print("a: ");
	kconsole.print_hex(a);
	kconsole.print(", b: ");
	kconsole.print_hex(b);
	kconsole.print("\nc: ");
	kconsole.print_hex(c);

	kfree(c);
	kfree(b);
	uint32_t d = kmalloc(12);
	kconsole.print(", d: ");
	kconsole.print_hex(d);

	ASSERT(b == d);

	uint32_t *ptr = (uint32_t*)0xA0000000;
    uint32_t do_page_fault = *ptr;
    UNUSED(do_page_fault);
}
