#pragma once
#include "multiboot.h"

class Kernel
{
	public:
		void run(multiboot_header *mh);
};
