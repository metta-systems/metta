#include "idt.h"
#include "string.h"

extern "C" void activate_idt(uint32_t base);

IdtEntry idt_entries[256];

/* Singleton? */
void InterruptDescriptorTable::init()
{
	static InterruptDescriptorTable idt;
}

InterruptDescriptorTable::InterruptDescriptorTable()
{
	limit = sizeof(idt_entries)-1;
	base = (uint32_t)&idt_entries;

	memset(&idt_entries, 0, sizeof(idt_entries));

	// Change dpl to 3 when we get to user-mode-enabled part.
	// This will allow kernel isrs to work in user-mode.
	idt_entries[0].set(0x08, isr0, IdtEntry::interrupt, 0);
	idt_entries[1].set(0x08, isr1, IdtEntry::interrupt, 0);
	idt_entries[2].set(0x08, isr2, IdtEntry::interrupt, 0);
	idt_entries[3].set(0x08, isr3, IdtEntry::interrupt, 0);
	idt_entries[4].set(0x08, isr4, IdtEntry::interrupt, 0);
	idt_entries[5].set(0x08, isr5, IdtEntry::interrupt, 0);
	idt_entries[6].set(0x08, isr6, IdtEntry::interrupt, 0);
	idt_entries[7].set(0x08, isr7, IdtEntry::interrupt, 0);
	idt_entries[8].set(0x08, isr8, IdtEntry::interrupt, 0);
	idt_entries[9].set(0x08, isr9, IdtEntry::interrupt, 0);
	idt_entries[10].set(0x08, isr10, IdtEntry::interrupt, 0);
	idt_entries[11].set(0x08, isr11, IdtEntry::interrupt, 0);
	idt_entries[12].set(0x08, isr12, IdtEntry::interrupt, 0);
	idt_entries[13].set(0x08, isr13, IdtEntry::interrupt, 0);
	idt_entries[14].set(0x08, isr14, IdtEntry::interrupt, 0);
	idt_entries[15].set(0x08, isr15, IdtEntry::interrupt, 0);
	idt_entries[16].set(0x08, isr16, IdtEntry::interrupt, 0);
	idt_entries[17].set(0x08, isr17, IdtEntry::interrupt, 0);
	idt_entries[18].set(0x08, isr18, IdtEntry::interrupt, 0);
	idt_entries[19].set(0x08, isr19, IdtEntry::interrupt, 0);
	idt_entries[20].set(0x08, isr20, IdtEntry::interrupt, 0);
	idt_entries[21].set(0x08, isr21, IdtEntry::interrupt, 0);
	idt_entries[22].set(0x08, isr22, IdtEntry::interrupt, 0);
	idt_entries[23].set(0x08, isr23, IdtEntry::interrupt, 0);
	idt_entries[24].set(0x08, isr24, IdtEntry::interrupt, 0);
	idt_entries[25].set(0x08, isr25, IdtEntry::interrupt, 0);
	idt_entries[26].set(0x08, isr26, IdtEntry::interrupt, 0);
	idt_entries[27].set(0x08, isr27, IdtEntry::interrupt, 0);
	idt_entries[28].set(0x08, isr28, IdtEntry::interrupt, 0);
	idt_entries[29].set(0x08, isr29, IdtEntry::interrupt, 0);
	idt_entries[30].set(0x08, isr30, IdtEntry::interrupt, 0);
	idt_entries[31].set(0x08, isr31, IdtEntry::interrupt, 0);

	// Setup PIC.
	// Remap the irq table.
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

	idt_entries[32].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[33].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[34].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[35].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[36].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[37].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[38].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[39].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[40].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[41].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[42].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[43].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[44].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[45].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[46].set(0x08, irq0, IdtEntry::interrupt, 0);
	idt_entries[47].set(0x08, irq15, IdtEntry::interrupt, 0);

	activate_idt((uint32_t)this);
}
