//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "cpu.h"
#include "default_console.h"

class ia32_pic_t
{
	enum {
		PIC_MASTER_COMMAND = 0x20,
		PIC_SLAVE_COMMAND = 0xA0,
		PIC_MASTER_DATA = 0x21,
		PIC_SLAVE_DATA = 0xA1,
		PIC_ICW1_INIT = 0x10,
		PIC_ICW1_ICW4 = 0x01,
		PIC_INITIALIZE = (PIC_ICW1_INIT|PIC_ICW1_ICW4),
		ICW4_8086 = 0x01,
		PIC_EOI = 0x20
	};

public:
	// Remap the irq tables
	static inline void reprogram_pic(int master_vec, int slave_vec)
	{
	    uint8_t master_mask, slave_mask;

	    master_mask = x86_cpu_t::inb(PIC_MASTER_DATA);
	    slave_mask = x86_cpu_t::inb(PIC_SLAVE_DATA);

	    x86_cpu_t::outb(PIC_MASTER_COMMAND, PIC_INITIALIZE); // ICW1: start initialization in cascade mode
	    x86_cpu_t::outb(PIC_SLAVE_COMMAND, PIC_INITIALIZE);

	    x86_cpu_t::outb(PIC_MASTER_DATA, master_vec); // ICW2: vector offset
	    x86_cpu_t::outb(PIC_SLAVE_DATA, slave_vec);   // ICW2: vector offset

	    x86_cpu_t::outb(PIC_MASTER_DATA, 4); // ICW3: tell Master PIC that there's a Slave PIC at IRQ2 (0000 0100)
	    x86_cpu_t::outb(PIC_SLAVE_DATA, 2);  // ICW3: tell Slave PIC its cascade identity (0000 0010)

	    x86_cpu_t::outb(PIC_MASTER_DATA, ICW4_8086); // ICW4: 8086/88 (MCS-80/85) mode
	    x86_cpu_t::outb(PIC_SLAVE_DATA, ICW4_8086);

	    x86_cpu_t::outb(PIC_MASTER_DATA, master_mask);
	    x86_cpu_t::outb(PIC_SLAVE_DATA, slave_mask);
	    kconsole << "PIC remapped to use vectors " << master_vec << ", " << slave_vec << endl;
	    print_masks();
	}

	static inline void print_masks()
	{
	    uint8_t master_mask, slave_mask;

	    master_mask = x86_cpu_t::inb(PIC_MASTER_DATA);
	    slave_mask = x86_cpu_t::inb(PIC_SLAVE_DATA);

		kconsole << "PIC interrupt masks " << master_mask << ", " << slave_mask << endl;
	}

	static inline void enable_irq(int irq_line)
	{
	    uint16_t port;
	    int offset;
	    if (irq_line < 8)
	    {
	        port = PIC_MASTER_DATA;
	        offset = irq_line;
	    }
	    else
	    {
	    	enable_irq(2); // afaik need to enable cascade IRQ2 on master, too?
	        port = PIC_SLAVE_DATA;
	        offset = irq_line - 8;
	    }

	    uint8_t value = x86_cpu_t::inb(port) & ~(1 << offset);
	    x86_cpu_t::outb(port, value);
	    kconsole << "IRQ" << irq_line << " enabled." << endl;
	}

	static inline void disable_irq(int irq_line)
	{
	    uint16_t port;
	    int offset;
	    if (irq_line < 8)
	    {
	        port = PIC_MASTER_DATA;
	        offset = irq_line;
	    }
	    else
	    {
	        port = PIC_SLAVE_DATA;
	        offset = irq_line - 8;
	    }

	    uint8_t value = x86_cpu_t::inb(port) | (1 << offset);
	    x86_cpu_t::outb(port, value);
	    kconsole << "IRQ" << irq_line << " disabled." << endl;
	}

    // Send an EOI (end of interrupt) signal to the PICs.
	static inline void eoi(int irq_line)
	{
		// IRQ8 and above should be acknowledged to the slave controller, too.
	    if (irq_line >= 8)
	    {
	        // If this interrupt involved the slave.
	        // Send reset signal to slave.
	        x86_cpu_t::outb(PIC_SLAVE_COMMAND, PIC_EOI);
	    }
	    // Send reset signal to master.
	    x86_cpu_t::outb(PIC_MASTER_COMMAND, PIC_EOI);
	}
};
