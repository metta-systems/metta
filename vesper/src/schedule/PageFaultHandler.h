//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "InterruptServiceRoutine.h"

class PageFaultHandler : public InterruptServiceRoutine
{
public:
	PageFaultHandler() : InterruptServiceRoutine() {}
	virtual ~PageFaultHandler() {}

	virtual void run(Registers *r);
};
