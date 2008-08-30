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
