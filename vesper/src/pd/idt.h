//
// Interrupt descriptor tables wrapper class.
//
#pragma once

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
			uint32_t offset_low		: 16;
			uint32_t sel			: 16;
			uint32_t res0			:  8;
			uint32_t type			:  3;
			uint32_t datasize		:  1;
			uint32_t res1			:  1;
			uint32_t dpl			:  2;
			uint32_t present		:  1;
			uint32_t offset_high	: 16;
		} d PACKED;
	} x;
};


/**
 * Set a descriptor entry.
 * @c segsel is code segment selector to run the handler in.
 * @c address is the offset of the handler in X86_KCS.
 * @c type selects Interrupt Gate or Trap Gate respectively.
 * @c dpl sets the numerical maximum CPL of allowed calling code.
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

class InterruptServiceRoutine;

class InterruptDescriptorTable
{
	public:
		INLINE InterruptDescriptorTable()
		{
			for (int i = 0; i < 256; i++)
				interruptRoutines[i] = 0;
		}

		void init();

		// Generic interrupt service routines.
		INLINE void setIsrHandler(int isrNum, InterruptServiceRoutine *isr)
		{
			interruptRoutines[isrNum] = isr;
		}

		// Hardware interrupt requests routines. (FIXME: archdep)
		INLINE void setIrqHandler(int irq, InterruptServiceRoutine *isr)
		{
			interruptRoutines[irq+32] = isr;
		}

		INLINE InterruptServiceRoutine* getIsr(int isrNum)
		{
			return interruptRoutines[isrNum];
		}

	private:
		// Two fields are part of the IDT
		uint16_t limit;
		uint32_t base;

		InterruptServiceRoutine *interruptRoutines[256];
} PACKED;

// These extern directives let us access the addresses of our ASM ISR handlers.
extern "C"
{
extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0 ();
extern void irq1 ();
extern void irq2 ();
extern void irq3 ();
extern void irq4 ();
extern void irq5 ();
extern void irq6 ();
extern void irq7 ();
extern void irq8 ();
extern void irq9 ();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
}

