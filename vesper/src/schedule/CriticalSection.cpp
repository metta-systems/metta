//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "Registers.h"

// Some operations require interrupts disabled. We create a stack here -
// when the index reaches zero, we enable interrupts. when it becomes nonzero,
// we disable interrupts. That way, if two nested start/stop calls take place,
// the inner will not disrupt the outer. TODO: for SMP make this a semaphore!
volatile uint32_t interruptsEnabledStack = 0;

void criticalSection()
{
	if (interruptsEnabledStack == 0)
	{
		disableInterrupts();
	}
	interruptsEnabledStack++;
}

void endCriticalSection() // TODO: check for unbalanced crit/endCrit calls.
{
	interruptsEnabledStack--;
	if (interruptsEnabledStack == 0)
	{
		enableInterrupts();
	}
}
