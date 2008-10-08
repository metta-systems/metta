#pragma once

#include "Types.h"

/**
 * These are the set of registers that appear when an interrupt is recieved
 * in kernel mode. The useresp and ss values are missing.
 */
typedef struct registers
{
	uint32_t ds;                  // Data segment selector
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
	uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
	uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} Registers;

class InterruptServiceRoutine
{
public:
	InterruptServiceRoutine() {}
	virtual ~InterruptServiceRoutine() {}

	virtual void run(Registers *) {}
};
