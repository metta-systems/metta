//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "idt.h"
#include "cpu.h"
#include "segs.h"

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
}

// Remap the irq table so that IRQ interrupts start at MASTER_VEC
static inline void reprogram_pic() // FIXME: move to ia32_pic_t
{
#define PICM 0x20
#define PICS 0xA0
#define ICW1 0x11
#define PICMI 0x21
#define PICSI 0xA1
#define MASTER_VEC 0x20
#define SLAVE_VEC  0x28
#define ICW4 0x01

    x86_cpu_t::outb(PICM, ICW1);
    x86_cpu_t::outb(PICS, ICW1);
    x86_cpu_t::outb(PICMI, MASTER_VEC);
    x86_cpu_t::outb(PICSI, SLAVE_VEC);
    x86_cpu_t::outb(PICMI, 4);
    x86_cpu_t::outb(PICSI, 2);
    x86_cpu_t::outb(PICMI, ICW4);
    x86_cpu_t::outb(PICSI, ICW4);
    x86_cpu_t::outb(PICMI, 0xff);//0?
    x86_cpu_t::outb(PICSI, 0xff);//0?
}

interrupt_descriptor_table_t& interrupt_descriptor_table_t::instance()
{
    static interrupt_descriptor_table_t interrupts_table;
    return interrupts_table;
}

void interrupt_descriptor_table_t::install()
{
    limit = sizeof(idt_entries)-1;
    base = (address_t)&idt_entries;

    // DPL is 3 to allow kernel isrs to work in user-mode.
    idt_entries[0].set(KERNEL_CS, isr0, idt_entry_t::interrupt, 3);
    idt_entries[1].set(KERNEL_CS, isr1, idt_entry_t::interrupt, 3);
    idt_entries[2].set(KERNEL_CS, isr2, idt_entry_t::interrupt, 3);
    idt_entries[3].set(KERNEL_CS, isr3, idt_entry_t::trap, 3);
    idt_entries[4].set(KERNEL_CS, isr4, idt_entry_t::trap, 3);
    idt_entries[5].set(KERNEL_CS, isr5, idt_entry_t::interrupt, 3);
    idt_entries[6].set(KERNEL_CS, isr6, idt_entry_t::interrupt, 3);
    idt_entries[7].set(KERNEL_CS, isr7, idt_entry_t::interrupt, 3);
    idt_entries[8].set(KERNEL_CS, isr8, idt_entry_t::interrupt, 3);
    idt_entries[9].set(KERNEL_CS, isr9, idt_entry_t::interrupt, 3);
    idt_entries[10].set(KERNEL_CS, isr10, idt_entry_t::interrupt, 3);
    idt_entries[11].set(KERNEL_CS, isr11, idt_entry_t::interrupt, 3);
    idt_entries[12].set(KERNEL_CS, isr12, idt_entry_t::interrupt, 3);
    idt_entries[13].set(KERNEL_CS, isr13, idt_entry_t::interrupt, 3);
    idt_entries[14].set(KERNEL_CS, isr14, idt_entry_t::interrupt, 3);
    idt_entries[15].set(KERNEL_CS, isr15, idt_entry_t::interrupt, 3);
    idt_entries[16].set(KERNEL_CS, isr16, idt_entry_t::interrupt, 3);
    idt_entries[17].set(KERNEL_CS, isr17, idt_entry_t::interrupt, 3);
    idt_entries[18].set(KERNEL_CS, isr18, idt_entry_t::interrupt, 3);
    idt_entries[19].set(KERNEL_CS, isr19, idt_entry_t::interrupt, 3);
    // Nothing useful defined on intel below this line
    idt_entries[20].set(KERNEL_CS, isr20, idt_entry_t::interrupt, 3);
    idt_entries[21].set(KERNEL_CS, isr21, idt_entry_t::interrupt, 3);
    idt_entries[22].set(KERNEL_CS, isr22, idt_entry_t::interrupt, 3);
    idt_entries[23].set(KERNEL_CS, isr23, idt_entry_t::interrupt, 3);
    idt_entries[24].set(KERNEL_CS, isr24, idt_entry_t::interrupt, 3);
    idt_entries[25].set(KERNEL_CS, isr25, idt_entry_t::interrupt, 3);
    idt_entries[26].set(KERNEL_CS, isr26, idt_entry_t::interrupt, 3);
    idt_entries[27].set(KERNEL_CS, isr27, idt_entry_t::interrupt, 3);
    idt_entries[28].set(KERNEL_CS, isr28, idt_entry_t::interrupt, 3);
    idt_entries[29].set(KERNEL_CS, isr29, idt_entry_t::interrupt, 3);
    idt_entries[30].set(KERNEL_CS, isr30, idt_entry_t::interrupt, 3);
    idt_entries[31].set(KERNEL_CS, isr31, idt_entry_t::interrupt, 3);

    reprogram_pic();

    // 32-47 are IRQs.
    // DPL is 3 so that interrupts can happen from user mode.
    idt_entries[32].set(KERNEL_CS, irq0, idt_entry_t::interrupt, 3);
    idt_entries[33].set(KERNEL_CS, irq1, idt_entry_t::interrupt, 3);
    idt_entries[34].set(KERNEL_CS, irq2, idt_entry_t::interrupt, 3);
    idt_entries[35].set(KERNEL_CS, irq3, idt_entry_t::interrupt, 3);
    idt_entries[36].set(KERNEL_CS, irq4, idt_entry_t::interrupt, 3);
    idt_entries[37].set(KERNEL_CS, irq5, idt_entry_t::interrupt, 3);
    idt_entries[38].set(KERNEL_CS, irq6, idt_entry_t::interrupt, 3);
    idt_entries[39].set(KERNEL_CS, irq7, idt_entry_t::interrupt, 3);
    idt_entries[40].set(KERNEL_CS, irq8, idt_entry_t::interrupt, 3);
    idt_entries[41].set(KERNEL_CS, irq9, idt_entry_t::interrupt, 3);
    idt_entries[42].set(KERNEL_CS, irq10, idt_entry_t::interrupt, 3);
    idt_entries[43].set(KERNEL_CS, irq11, idt_entry_t::interrupt, 3);
    idt_entries[44].set(KERNEL_CS, irq12, idt_entry_t::interrupt, 3);
    idt_entries[45].set(KERNEL_CS, irq13, idt_entry_t::interrupt, 3);
    idt_entries[46].set(KERNEL_CS, irq14, idt_entry_t::interrupt, 3);
    idt_entries[47].set(KERNEL_CS, irq15, idt_entry_t::interrupt, 3);

    asm volatile("lidtl %0\n" :: "m"(*this));
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
