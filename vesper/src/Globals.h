#pragma once
#include "Kernel.h"
#include "Multiboot.h"
#include "ElfParser.h"
#include "MemoryManager.h"

extern Kernel kernel;
extern Multiboot multiboot;
extern ElfParser kernelElfParser;
extern MemoryManager memoryManager;

extern "C" void kernel_entry(MultibootHeader *mh);
