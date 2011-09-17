//
// The loader for hosted OS will simply set up some variables, allocate bootinfo structure and call kernel_startup.
//
// This loader doesn't follow the standard loader protocol, it simply calls kernel_entry()
//
#include "bootinfo.h"

extern "C" kernel_startup();

int main()
{
	// construct bootinfo_t - record location for subsequent calls..
	bootinfo_t bi(true);
	bi.ADDRESS = &bi; // or sth like that

	// GDT, IDT should be faked.

	// call the kernel function
	kernel_startup();

	// one interesting case with hosted system is that we need to run global destructors for proper tear-down as well...
	
	return 0;
}