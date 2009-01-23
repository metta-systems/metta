//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "interrupt_descriptor_table.h"
#include "common.h"

// These extern directives let us access the addresses of our ASM ISR handlers.
extern "C"
{
    void isr0 ();
    void isr1 ();
    void isr2 ();
    void isr3 ();
    void isr4 ();
    void isr5 ();
    void isr6 ();
    void isr7 ();
    void isr8 ();
    void isr9 ();
    void isr10();
    void isr11();
    void isr12();
    void isr13();
    void isr14();
    void isr15();
    void isr16();
    void isr17();
    void isr18();
    void isr19();
    void isr20();
    void isr21();
    void isr22();
    void isr23();
    void isr24();
    void isr25();
    void isr26();
    void isr27();
    void isr28();
    void isr29();
    void isr30();
    void isr31();

    void irq0 ();
    void irq1 ();
    void irq2 ();
    void irq3 ();
    void irq4 ();
    void irq5 ();
    void irq6 ();
    void irq7 ();
    void irq8 ();
    void irq9 ();
    void irq10();
    void irq11();
    void irq12();
    void irq13();
    void irq14();
    void irq15();

    void activate_idt(address_t base); // in activate.s
}

namespace metta {
namespace kernel {

#define CS_SEL 0x08

void interrupt_descriptor_table::init()
{
    limit = sizeof(idt_entries)-1;
    base = (address_t)&idt_entries;

    // DPL is 3 to allow kernel isrs to work in user-mode.
    idt_entries[0].set(CS_SEL, isr0, idt_entry::interrupt, 3);
    idt_entries[1].set(CS_SEL, isr1, idt_entry::interrupt, 3);
    idt_entries[2].set(CS_SEL, isr2, idt_entry::interrupt, 3);
    idt_entries[3].set(CS_SEL, isr3, idt_entry::interrupt, 3);
    idt_entries[4].set(CS_SEL, isr4, idt_entry::interrupt, 3);
    idt_entries[5].set(CS_SEL, isr5, idt_entry::interrupt, 3);
    idt_entries[6].set(CS_SEL, isr6, idt_entry::interrupt, 3);
    idt_entries[7].set(CS_SEL, isr7, idt_entry::interrupt, 3);
    idt_entries[8].set(CS_SEL, isr8, idt_entry::interrupt, 3);
    idt_entries[9].set(CS_SEL, isr9, idt_entry::interrupt, 3);
    idt_entries[10].set(CS_SEL, isr10, idt_entry::interrupt, 3);
    idt_entries[11].set(CS_SEL, isr11, idt_entry::interrupt, 3);
    idt_entries[12].set(CS_SEL, isr12, idt_entry::interrupt, 3);
    idt_entries[13].set(CS_SEL, isr13, idt_entry::interrupt, 3);
    idt_entries[14].set(CS_SEL, isr14, idt_entry::interrupt, 3);
    idt_entries[15].set(CS_SEL, isr15, idt_entry::interrupt, 3);
    idt_entries[16].set(CS_SEL, isr16, idt_entry::interrupt, 3);
    idt_entries[17].set(CS_SEL, isr17, idt_entry::interrupt, 3);
    idt_entries[18].set(CS_SEL, isr18, idt_entry::interrupt, 3);
    idt_entries[19].set(CS_SEL, isr19, idt_entry::interrupt, 3);
    idt_entries[20].set(CS_SEL, isr20, idt_entry::interrupt, 3);
    idt_entries[21].set(CS_SEL, isr21, idt_entry::interrupt, 3);
    idt_entries[22].set(CS_SEL, isr22, idt_entry::interrupt, 3);
    idt_entries[23].set(CS_SEL, isr23, idt_entry::interrupt, 3);
    idt_entries[24].set(CS_SEL, isr24, idt_entry::interrupt, 3);
    idt_entries[25].set(CS_SEL, isr25, idt_entry::interrupt, 3);
    idt_entries[26].set(CS_SEL, isr26, idt_entry::interrupt, 3);
    idt_entries[27].set(CS_SEL, isr27, idt_entry::interrupt, 3);
    idt_entries[28].set(CS_SEL, isr28, idt_entry::interrupt, 3);
    idt_entries[29].set(CS_SEL, isr29, idt_entry::interrupt, 3);
    idt_entries[30].set(CS_SEL, isr30, idt_entry::interrupt, 3);
    idt_entries[31].set(CS_SEL, isr31, idt_entry::interrupt, 3);

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
    idt_entries[32].set(CS_SEL, irq0, idt_entry::interrupt, 3);
    idt_entries[33].set(CS_SEL, irq1, idt_entry::interrupt, 3);
    idt_entries[34].set(CS_SEL, irq2, idt_entry::interrupt, 3);
    idt_entries[35].set(CS_SEL, irq3, idt_entry::interrupt, 3);
    idt_entries[36].set(CS_SEL, irq4, idt_entry::interrupt, 3);
    idt_entries[37].set(CS_SEL, irq5, idt_entry::interrupt, 3);
    idt_entries[38].set(CS_SEL, irq6, idt_entry::interrupt, 3);
    idt_entries[39].set(CS_SEL, irq7, idt_entry::interrupt, 3);
    idt_entries[40].set(CS_SEL, irq8, idt_entry::interrupt, 3);
    idt_entries[41].set(CS_SEL, irq9, idt_entry::interrupt, 3);
    idt_entries[42].set(CS_SEL, irq10, idt_entry::interrupt, 3);
    idt_entries[43].set(CS_SEL, irq11, idt_entry::interrupt, 3);
    idt_entries[44].set(CS_SEL, irq12, idt_entry::interrupt, 3);
    idt_entries[45].set(CS_SEL, irq13, idt_entry::interrupt, 3);
    idt_entries[46].set(CS_SEL, irq14, idt_entry::interrupt, 3);
    idt_entries[47].set(CS_SEL, irq15, idt_entry::interrupt, 3);

// asm ("    lidt [eax]        ; Load the IDT pointer.");

    activate_idt((address_t)this);
}

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
