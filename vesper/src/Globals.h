#pragma once
#include "Kernel.h"
#include "Multiboot.h"
#include "ElfParser.h"

extern Kernel kernel;
extern Multiboot multiboot;
extern ElfParser kernelElfParser;

extern "C" void kernel_entry(MultibootHeader *mh);
