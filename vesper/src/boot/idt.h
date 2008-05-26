#ifndef __INCLUDED_IDT_H
#define __INCLUDED_IDT_H

#include "common.h"

class IdtEntry
{
public:
	enum type_e
	{
		interrupt = 6,
		trap = 7
	};

	void set(uint16_t segsel, void (*address)(), type_e type, int dpl);

private:
	union {
		uint32_t raw[2];

		struct {
			uint32_t offset_low	: 16;
			uint32_t sel		: 16;
			uint32_t res0		:  8;
			uint32_t type		:  3;
			uint32_t datasize		:  1;
			uint32_t res1		:  1;
			uint32_t dpl		:  2;
			uint32_t present		:  1;
			uint32_t offset_high	: 16;
		} d;
	} x;
};


/* IdtEntry::set
 * sets an descriptor entry
 * - address is the offset of the handler in X86_KCS
 * - type selects Interrupt Gate or Trap Gate respectively
 * - dpl sets the numerical maximum CPL of allowed calling code
 */

INLINE void IdtEntry::set(uint16_t segsel, void (*address)(), type_e type, int dpl)
{
    x.d.offset_low  = ((uint32_t) address      ) & 0xFFFF;
    x.d.offset_high = ((uint32_t) address >> 16) & 0xFFFF;
    x.d.sel = segsel;
    x.d.dpl = dpl;
    x.d.type = type;

    /* set constant values */
    x.d.present = 1;	/* present	*/
    x.d.datasize = 1;	/* size is 32	*/

    /* clear reserved fields */
    x.d.res0 = x.d.res1 = 0;
};

class InterruptDescriptorTable
{
	public:
		static void init();

	private:
		InterruptDescriptorTable();
		uint16_t limit;
		uint32_t base;
} __attribute__((packed));

// These extern directives let us access the addresses of our ASM ISR handlers.
extern "C" void isr0 ();
extern "C" void isr1 ();
extern "C" void isr2 ();
extern "C" void isr3 ();
extern "C" void isr4 ();
extern "C" void isr5 ();
extern "C" void isr6 ();
extern "C" void isr7 ();
extern "C" void isr8 ();
extern "C" void isr9 ();
extern "C" void isr10();
extern "C" void isr11();
extern "C" void isr12();
extern "C" void isr13();
extern "C" void isr14();
extern "C" void isr15();
extern "C" void isr16();
extern "C" void isr17();
extern "C" void isr18();
extern "C" void isr19();
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();
extern "C" void irq0 ();
extern "C" void irq1 ();
extern "C" void irq2 ();
extern "C" void irq3 ();
extern "C" void irq4 ();
extern "C" void irq5 ();
extern "C" void irq6 ();
extern "C" void irq7 ();
extern "C" void irq8 ();
extern "C" void irq9 ();
extern "C" void irq10();
extern "C" void irq11();
extern "C" void irq12();
extern "C" void irq13();
extern "C" void irq14();
extern "C" void irq15();

#endif /* !__INCLUDED_IDT_H */
