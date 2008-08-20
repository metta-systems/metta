#pragma once
#include "Kernel.h"
#include "ElfParser.h"

extern Kernel kernel;
extern ElfParser kernelElfParser;

extern "C" void kernel_entry(multiboot_header *mbd, unsigned int magic);
