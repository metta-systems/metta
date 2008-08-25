#include "Kernel.h"
#include "Globals.h"
#include "Registers.h"
#include "DefaultConsole.h"
#include "kalloc.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "paging.h"
#include "task.h"
#include "Globals.h"

extern uint32_t mem_end_page; //in paging.cpp

void backtraceTest2()
{
	kernel.printStacktrace(5);
	kernel.printBacktrace(0,0);
}

void backtraceTest()
{
	backtraceTest2();
}

void Kernel::run()
{
	if (!multiboot.isElf())
		PANIC("ELF information is missing in kernel!");

	kernelElfParser.loadKernel(multiboot.symtabStart(), multiboot.strtabStart());

	backtraceTest();
	while(1) {}

	kconsole.locate(5, 0);
	kconsole.set_color(LIGHTRED);
	kconsole.print("Reloading GDT...\n");
	GlobalDescriptorTable::init();

	kconsole.print("Loading IDT...\n");
	InterruptDescriptorTable::init();

	kconsole.set_color(WHITE);
	if (multiboot.hasMemInfo())
	{
		kconsole.print("Mem lower: %d\n", multiboot.lowerMem());
		kconsole.print("Mem upper: %d\n", multiboot.upperMem());
		mem_end_page = (multiboot.lowerMem() + multiboot.upperMem() + 1024) * 1024;
	}

	// malloc check
	uint32_t a = kmalloc(8);
	Paging::self();
	uint32_t b = kmalloc(8);
	uint32_t c = kmalloc(8);
	kfree(c);
	kfree(b);
	uint32_t d = kmalloc(12);
	ASSERT(b == d);
	kfree(a);
	kfree(d);

	Task::init();
	Timer::init();

	// Load initrd and pass control to init component

	while(1) { }
}

Address Kernel::backtrace(Address basePointer, Address& returnAddress)
{
	// We take a stack base pointer (in basePointer), return what it's pointing at
	// and put the Address just above it in the stack in returnAddress.
	Address nextBase = *((Address*)basePointer);
	returnAddress    = *((Address*)(basePointer+sizeof(Address)));
	return nextBase;
}

Address Kernel::backtrace(int n)
{
	Address basePointer = readBasePointer();
	Address ebp = basePointer;
	Address eip = 1;
	int i = 0;
	while (ebp && eip /*&& eip < 0x87000000*/)
	{
		ebp = backtrace(ebp, eip);
		if (i == n)
		{
			return eip;
		}
		i++;
	}
	return 0;
}

void Kernel::printBacktrace(Address basePointer, int n)
{
	Address eip = 1; // Don't initialise to 0, will kill the loop immediately.
	if (basePointer == NULL)
	{
		basePointer = readBasePointer();
	}
	Address ebp = basePointer;
	kconsole.set_color(GREEN);
	kconsole.print("*** Backtrace ***\nTracing %d stack frames:\n", n);
	int i = 0;
	while (ebp && eip &&
		( (n && i<n) || !n) &&
		eip < 0x87000000)
	{
		ebp = backtrace(ebp, eip);
		unsigned int offset;
		char *symbol = kernelElfParser.findSymbol(eip, &offset);
		offset = eip - offset;
		kconsole.print("| %08x <%s+0x%x>\n", eip, symbol ? symbol : "UNRESOLVED", offset);
		i++;
	}
}

void Kernel::printStacktrace(unsigned int n)
{
	Address esp = readStackPointer();
	Address espBase = esp;
	kconsole.set_color(GREEN);
	kconsole.print("<ESP=%08x>\n", esp);
	for (unsigned int i = 0; i < n; i++)
	{
		kconsole.print("<ESP+%4d> %08x\n", esp - espBase, *(Address*)esp);
		esp += sizeof(Address);
	}
}

/* kate: indent-width 4; replace-tabs off; */
// vi: ts=4