#include "Globals.h"
#include "DefaultConsole.h"
#include "kalloc.h"

/* Global objects FIXME: use singletons instead? */
Kernel kernel;

/* This entry point is called from loader */
void kernel_entry(multiboot_header *mbd, unsigned int magic)
{
	kconsole.clear();
	if(magic != MULTIBOOT_MAGIC)
	{
		kconsole.print("Multiboot-capable loader required.");
		//halt();
	}
	kernel.run(mbd);
}

// FIXME: Streamline memory allocation strategy (e.g. extra parameters to new())
//void *operator new (uint32_t size, bool align, void **phys)

//overload the operator "new"
// "new" is for 'big' objects that are page-aligned.
void *operator new (uint32_t size)
{
	uint32_t addr = kmalloc_a(size);
	kconsole.debug_log("operator new: %d @ %p", size, addr);
	return (void *)addr;
}

//overload the operator "new[]"
// "new[]" is for 'small' objects that are not page-aligned and usually in pools.
void *operator new[] (uint32_t size)
{
	uint32_t addr = kmalloc(size);
	kconsole.debug_log("operator new[]: %d @ %p", size, addr);
	return (void *)addr;
}

//overload the operator "delete"
void operator delete (void *p)
{
	kconsole.debug_log("operator delete: %p", p);
	kfree((uint32_t)p);
}

//overload the operator "delete[]"
void operator delete[] (void *p)
{
	kconsole.debug_log("operator delete[]: %p", p);
	kfree((uint32_t)p);
}
