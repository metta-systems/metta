//
// C++ runtime support.
// Dummy implementation for booting.
//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
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

void *__dso_handle;

// also in namespace __cxxabiv1?
extern "C" void __cxa_atexit(void (*)(void *), void *, void *)
{
}

extern "C" void __cxa_pure_virtual()
{
}

// Support for STL bad_alloc
extern "C" void __bad_alloc_handler()
{
    //TODO: panic() here
}
