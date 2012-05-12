//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/*!
 * The loader for hosted OS will simply set up some variables, allocate bootinfo structure and call kernel_startup.
 *
 * This loader doesn't follow the standard loader protocol.
 */
#include "bootinfo.h"

extern "C" kernel_startup();

int main()
{
	// construct bootinfo_t - record location for subsequent calls..
	char* bootinfo_area = malloc(4*KB);
	new(bootinfo_area) bootinfo_t(true);

	// GDT, IDT should be faked.

	// call the kernel function
	kernel_startup();

	// one interesting case with hosted system is that we need to run global destructors for proper tear-down as well...
	delete bootinfo_area;
	
	return 0;
}