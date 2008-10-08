#include "common.h"
#include "Task.h"
#include "Timer.h"
#include "DefaultConsole.h"
#include "InterruptServiceRoutine.h"
#include "idt.h"
#include "Globals.h"

void Timer::init()
{
	static Timer timer;
}

class TimerCallback : public InterruptServiceRoutine
{
	uint32_t tick;

public:
	TimerCallback() : tick(0) {}
	virtual ~TimerCallback() {}

	virtual void run(Registers *)
	{
		tick++;
		Task::yield();
	}
} timerCallback;

Timer::Timer()
{
	uint32_t frequency = 50;

	// Firstly, register our timer callback.
	interruptsTable.setIrqHandler(0, &timerCallback);

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

	kconsole.debug_log("Constructed timer.");
}
