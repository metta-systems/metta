//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "multiboot.h"
#include "default_console.h"

extern "C" void entry(multiboot::header *mbh);

void entry(multiboot::header *mbh)
{
    (void)mbh;
	kconsole << YELLOW << "initcp reached";//, mbh " << (unsigned)mbh << endl;
	while(1) {}
}

// Sprint 2 is overall success: bootloader sets up structures and jumps to
// initcomp. Next step is implementing ELF parser that will be able to call
// given initcp module's entrypoint.
// (otoh, a binary only initcp and kernel could save some bytes)

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
