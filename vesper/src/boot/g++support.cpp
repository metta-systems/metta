#include "DefaultConsole.h"

/* Dummy implementation for now */
namespace __cxxabiv1
{
	/* guard variables */

	/* The ABI requires a 64-bit type.  */
	__extension__ typedef int __guard __attribute__((mode (__DI__)));

	extern "C" int __cxa_guard_acquire (__guard *);
	extern "C" void __cxa_guard_release (__guard *);
	extern "C" void __cxa_guard_abort (__guard *);

	extern "C" int __cxa_guard_acquire (__guard *g)
	{
		return !*(char *)(g);
	}

	extern "C" void __cxa_guard_release (__guard *g)
	{
		*(char *)g = 1;
	}

	extern "C" void __cxa_guard_abort (__guard *)
	{
	}
}

#include "kalloc.h"

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
