#include "DefaultConsole.h"
#include "isr.h"
#include "common.h"

extern "C" void isr_handler(registers_t regs);
extern "C" void irq_handler(registers_t regs);

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler)
{
	interrupt_handlers[n] = handler;
}

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
	kconsole.set_color(GREEN);
	kconsole.print("Received interrupt: %d\n", regs.int_no);

	isr_t handler = interrupt_handlers[regs.int_no];
	if (handler)
	{
		handler(regs);
	}
}

// This gets called from our ASM hardware interrupt handler stub.
void irq_handler(registers_t regs)
{
	isr_t handler = interrupt_handlers[regs.int_no];
	if (handler)
	{
		handler(regs);
	}

	// Send an EOI (end of interrupt) signal to the PICs.
	// If this interrupt involved the slave.
	if (regs.int_no >= IRQ8)
	{
		// Send reset signal to slave.
		outb(0xA0, 0x20);
	}
	// Send reset signal to master. (As well as slave, if necessary).
	outb(0x20, 0x20);
}
