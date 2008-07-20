#include "common.h"
#include "DefaultConsole.h"

void panic(const char *message, const char *file, uint32_t line)
{
	// We encountered a massive problem and have to stop.
	asm volatile("cli"); // Disable interrupts.

	kconsole.set_attr(RED, YELLOW);
	kconsole.print("PANIC(");
	kconsole.print(message);
	kconsole.print(") at ");
	kconsole.print(file);
	kconsole.print(":");
	kconsole.print_int(line);
	kconsole.newline();

	// Halt by going into an infinite loop.
	while(1) {}
}

void panic_assert(const char *desc, const char *file, uint32_t line)
{
	// An assertion failed, and we have to panic.
	asm volatile("cli"); // Disable interrupts.

	kconsole.set_attr(WHITE, RED);
	kconsole.print("ASSERTION-FAILED(");
	kconsole.print(desc);
	kconsole.print(") at ");
	kconsole.print(file);
	kconsole.print(":");
	kconsole.print_int(line);
	kconsole.newline();

	// Halt by going into an infinite loop.
	while(1) {}
}
