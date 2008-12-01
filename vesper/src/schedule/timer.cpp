//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "common.h"
#include "Task.h"
#include "Timer.h"
#include "DefaultConsole.h"
#include "InterruptServiceRoutine.h"
#include "InterruptDescriptorTable.h"
#include "Globals.h"

namespace metta {
namespace kernel {

void timer::init()
{
	static timer timer_instance;
}

class timer_callback : public interrupt_service_routine
{
	uint32_t tick;

public:
    timer_callback() : tick(0) {}
    virtual ~timer_callback() {}

	virtual void run(Registers *)
	{
		tick++;
		task::yield();
	}
} timer_callback_;

timer::timer()
{
	uint32_t frequency = 50;

	// Firstly, register our timer callback.
	interrupts_table.set_irq_handler(0, &timer_callback_);

	// The value we send to the PIT is the value to divide it's input clock
	// (1193180 Hz) by, to get our required frequency. Important to note is
	// that the divisor must be small enough to fit into 16-bits.
	uint32_t divisor = 1193180 / frequency;

	// Send the command byte.
	outb(0x43, 0x36);

	// Divisor has to be sent byte-wise, so split here into upper/lower bytes.
	uint8_t l = divisor & 0xFF;
	uint8_t h = (divisor>>8) & 0xFF;

	// Send the frequency divisor.
	outb(0x40, l);
	outb(0x40, h);

// 	kconsole.debug_log("Constructed timer.");
}

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
