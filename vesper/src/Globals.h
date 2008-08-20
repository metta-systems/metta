#pragma once
#include "Kernel.h"

extern Kernel kernel;

extern "C" void kernel_entry(multiboot_header *mbd, unsigned int magic);
