//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "InterruptDescriptorTable.h"

#define CS_SEL 0x08

extern "C" void activate_idt(Address base); // in activate.s

IdtEntry idt_entries[256];

void InterruptDescriptorTable::init()
{
	limit = sizeof(idt_entries)-1;
	base = (Address)&idt_entries;

	// DPL is 3 to allow kernel isrs to work in user-mode.
	idt_entries[0].set(CS_SEL, isr0, IdtEntry::interrupt, 3);
	idt_entries[1].set(CS_SEL, isr1, IdtEntry::interrupt, 3);
	idt_entries[2].set(CS_SEL, isr2, IdtEntry::interrupt, 3);
	idt_entries[3].set(CS_SEL, isr3, IdtEntry::interrupt, 3);
	idt_entries[4].set(CS_SEL, isr4, IdtEntry::interrupt, 3);
	idt_entries[5].set(CS_SEL, isr5, IdtEntry::interrupt, 3);
	idt_entries[6].set(CS_SEL, isr6, IdtEntry::interrupt, 3);
	idt_entries[7].set(CS_SEL, isr7, IdtEntry::interrupt, 3);
	idt_entries[8].set(CS_SEL, isr8, IdtEntry::interrupt, 3);
	idt_entries[9].set(CS_SEL, isr9, IdtEntry::interrupt, 3);
	idt_entries[10].set(CS_SEL, isr10, IdtEntry::interrupt, 3);
	idt_entries[11].set(CS_SEL, isr11, IdtEntry::interrupt, 3);
	idt_entries[12].set(CS_SEL, isr12, IdtEntry::interrupt, 3);
	idt_entries[13].set(CS_SEL, isr13, IdtEntry::interrupt, 3);
	idt_entries[14].set(CS_SEL, isr14, IdtEntry::interrupt, 3);
	idt_entries[15].set(CS_SEL, isr15, IdtEntry::interrupt, 3);
	idt_entries[16].set(CS_SEL, isr16, IdtEntry::interrupt, 3);
	idt_entries[17].set(CS_SEL, isr17, IdtEntry::interrupt, 3);
	idt_entries[18].set(CS_SEL, isr18, IdtEntry::interrupt, 3);
	idt_entries[19].set(CS_SEL, isr19, IdtEntry::interrupt, 3);
	idt_entries[20].set(CS_SEL, isr20, IdtEntry::interrupt, 3);
	idt_entries[21].set(CS_SEL, isr21, IdtEntry::interrupt, 3);
	idt_entries[22].set(CS_SEL, isr22, IdtEntry::interrupt, 3);
	idt_entries[23].set(CS_SEL, isr23, IdtEntry::interrupt, 3);
	idt_entries[24].set(CS_SEL, isr24, IdtEntry::interrupt, 3);
	idt_entries[25].set(CS_SEL, isr25, IdtEntry::interrupt, 3);
	idt_entries[26].set(CS_SEL, isr26, IdtEntry::interrupt, 3);
	idt_entries[27].set(CS_SEL, isr27, IdtEntry::interrupt, 3);
	idt_entries[28].set(CS_SEL, isr28, IdtEntry::interrupt, 3);
	idt_entries[29].set(CS_SEL, isr29, IdtEntry::interrupt, 3);
	idt_entries[30].set(CS_SEL, isr30, IdtEntry::interrupt, 3);
	idt_entries[31].set(CS_SEL, isr31, IdtEntry::interrupt, 3);

	// Setup PIC.
	// Remap the irq table.
	// TODO: document reprogramming better.
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);

	// 32-47 are IRQs.
	// DPL is 3 so that interrupts can happen from user mode.
	idt_entries[32].set(CS_SEL, irq0, IdtEntry::interrupt, 3);
	idt_entries[33].set(CS_SEL, irq1, IdtEntry::interrupt, 3);
	idt_entries[34].set(CS_SEL, irq2, IdtEntry::interrupt, 3);
	idt_entries[35].set(CS_SEL, irq3, IdtEntry::interrupt, 3);
	idt_entries[36].set(CS_SEL, irq4, IdtEntry::interrupt, 3);
	idt_entries[37].set(CS_SEL, irq5, IdtEntry::interrupt, 3);
	idt_entries[38].set(CS_SEL, irq6, IdtEntry::interrupt, 3);
	idt_entries[39].set(CS_SEL, irq7, IdtEntry::interrupt, 3);
	idt_entries[40].set(CS_SEL, irq8, IdtEntry::interrupt, 3);
	idt_entries[41].set(CS_SEL, irq9, IdtEntry::interrupt, 3);
	idt_entries[42].set(CS_SEL, irq10, IdtEntry::interrupt, 3);
	idt_entries[43].set(CS_SEL, irq11, IdtEntry::interrupt, 3);
	idt_entries[44].set(CS_SEL, irq12, IdtEntry::interrupt, 3);
	idt_entries[45].set(CS_SEL, irq13, IdtEntry::interrupt, 3);
	idt_entries[46].set(CS_SEL, irq14, IdtEntry::interrupt, 3);
	idt_entries[47].set(CS_SEL, irq15, IdtEntry::interrupt, 3);

	activate_idt((Address)this);
}
