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

//overload the operator "new"
void *operator new (uint32_t size)
{
    return (void *)kmalloc(size);
}

//overload the operator "new[]"
void *operator new[] (uint32_t size)
{
    return (void *)kmalloc(size);
}

//overload the operator "delete"
void operator delete (void *p)
{
    kfree((uint32_t)p);
}

//overload the operator "delete[]"
void operator delete[] (void *p)
{
    kfree((uint32_t)p);
}
