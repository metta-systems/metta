#pragma once

#include "Types.h"
#include "string.h"

extern class Kernel kernel;
extern class Multiboot multiboot;
extern class ElfParser kernelElfParser;
extern class MemoryManager memoryManager;

extern "C" void kernel_entry(class MultibootHeader *mh) /*NORETURN*/;

void *operator new(size_t size);
void *operator new(size_t size, uint32_t place);
void *operator new(size_t size, bool pageAlign, uint32_t *physAddr=NULL);
void *operator new[](size_t size);
void *operator new[](size_t size, bool pageAlign, uint32_t *physAddr=NULL);
void  operator delete(void *p);
void  operator delete[](void *p);
