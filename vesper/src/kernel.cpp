#include "Kernel.h"
#include "DefaultConsole.h"
#include "kalloc.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "paging.h"
#include "task.h"

extern uint32_t mem_end_page; //in paging.cpp

void Kernel::run(multiboot_header *mbd)
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
	kconsole.locate(11, 22);
	kconsole.print_int(-21954321);
	kconsole.scroll_up();
	kconsole.set_color(YELLOW);
	kconsole.debug_showmem(mbd, 135);

	kconsole.set_color(WHITE);
	if (mbd->flags & MULTIBOOT_FLAG_MEM)
	{
		kconsole.print("Mem lower: ");
		kconsole.print_int(mbd->mem_lower);
		kconsole.newline();
		kconsole.print("Mem upper: ");
		kconsole.print_int(mbd->mem_upper);
		kconsole.newline();
		mem_end_page = (mbd->mem_lower + mbd->mem_upper + 1024) * 1024;
	}

	asm volatile ("int $0x3");
	asm volatile ("int $0x4");

	uint32_t a = kmalloc(8);
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
	kconsole.newline();

	ASSERT(b == d);

	Task::init();
	Timer::init();

	int ret = Task::self()->fork();

	kconsole.print("fork() returned ");
	kconsole.print_int(ret);
	kconsole.print(" and getpid() returned ");
	kconsole.print_int(Task::self()->getpid());
	kconsole.newline();

	while(1) { }
}

/* kate: indent-width 4; replace-tabs off; */
// vi: ts=4