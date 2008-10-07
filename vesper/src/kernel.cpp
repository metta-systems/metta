#include "Kernel.h"
#include "Globals.h"
#include "Registers.h"
#include "Multiboot.h"
#include "ElfParser.h"
#include "MemoryManager.h"
#include "DefaultConsole.h"
#include "gdt.h"
#include "idt.h"
#include "Timer.h"
#include "Task.h"

// [ ] TODO create own stack after we enabled paging
void Kernel::run()
{
	if (!multiboot.isElf())
		PANIC("ELF information is missing in kernel!");

	// Make sure we aren't overwriting anything by writing at placementAddress.
	relocatePlacementAddress();

	kernelElfParser.loadKernel(multiboot.symtabStart(), multiboot.strtabStart());

	GlobalDescriptorTable::init();
	InterruptDescriptorTable::init();

	// TODO: add page fault handler init before enabling paging.

	memoryManager.init(multiboot.upperMem() * 1024);
	memoryManager.remapStack();
	kconsole.print("Returned from remapStack()\n");

	Task::init();
	Timer::init();

	// Load initrd and pass control to init component

	while(1) { }
}

void Kernel::relocatePlacementAddress()
{
	Address newPlacementAddress = memoryManager.getPlacementAddress();
	if (multiboot.isElf() && multiboot.symtabEnd() > newPlacementAddress)
	{
		newPlacementAddress = multiboot.symtabEnd();
	}
	if (multiboot.isElf() && multiboot.strtabEnd() > newPlacementAddress)
	{
		newPlacementAddress = multiboot.strtabEnd();
	}
	if (multiboot.modStart() > newPlacementAddress)
	{
		newPlacementAddress = multiboot.modEnd();
	}
	memoryManager.setPlacementAddress(newPlacementAddress);
}

void Kernel::dumpMemory(Address start, size_t size)
{
	char *ptr = (char *)start;
	int run;

	// Silly limitation, probably.
	if (size > 256)
		size = 256;

	kconsole.newline();

	while (size > 0)
	{
		kconsole.print_hex((unsigned int)ptr);
		kconsole.print("  ");
		run = size < 16 ? size : 16;
		for(int i = 0; i < run; i++)
		{
			kconsole.print_byte(*(ptr+i));
			kconsole.print_char(' ');
			if (i == 7)
				kconsole.print_char(' ');
		}
		if (run < 16)// pad
		{
			if(run < 8)
				kconsole.print_char(' ');
			for(int i = 0; i < 16-run; i++)
				kconsole.print("   ");
		}
		kconsole.print_char(' ');
		for(int i = 0; i < run; i++)
		{
			char c = *(ptr+i);
			if (c == kconsole.EOL)
				c = ' ';
			kconsole.print_char(c);
		}
		kconsole.newline();
		ptr += run;
		size -= run;
	}
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
	kconsole.print("*** Backtrace *** Tracing %d stack frames:\n", n);
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