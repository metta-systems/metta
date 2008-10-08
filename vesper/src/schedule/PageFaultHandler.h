#pragma once

#include "InterruptServiceRoutine.h"

class PageFaultHandler : public InterruptServiceRoutine
{
public:
	PageFaultHandler() : InterruptServiceRoutine() {}
	virtual ~PageFaultHandler() {}

	virtual void run(Registers *r);
};
