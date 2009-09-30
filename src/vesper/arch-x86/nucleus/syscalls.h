//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
inline void kernel_syscall(int code)
{
    asm volatile("mov %0, %%eax\n"
                 "sysenter", "a"(code));
}

inline void kernel_interface_syscall()
{
    kernel_syscall(1);
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
