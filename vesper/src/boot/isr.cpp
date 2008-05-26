#include "DefaultConsole.h"
#include "isr.h"

extern "C" void isr_handler(registers_t regs);

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
	kconsole.set_color(GREEN);
	kconsole.print("Received interrupt: ");
	kconsole.print_int(regs.int_no);
	kconsole.newline();
}
