//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

class x86_cpu_t
{
public:
    /*!
     * Write a byte out to the specified port.
     */
    static inline void outb(uint16_t port, uint8_t value)
    {
        //("a" puts value in eax, "dN" puts port in edx or uses 1-byte constant.)
        asm volatile ("outb %0, %1" :: "a" (value), "dN" (port));
    }

    /*!
     * Write a word out to the specified port.
     */
    static inline void outw(uint16_t port, uint16_t value)
    {
        asm volatile ("outw %0, %1" :: "a" (value), "dN" (port));
    }

    /*!
     * Read a byte in from the specified port.
     */
    static inline uint8_t inb(uint16_t port)
    {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
    }

    /*!
     * Read a word in from the specified port.
     */
    static inline uint16_t inw(uint16_t port)
    {
        uint16_t ret;
        asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
    }

    /*!
     * Read internal CPU 64-bit clock (timestamp counter).
     */
    static inline uint64_t rdtsc()
    {
        uint64_t ret;
        asm volatile("rdtsc" : "=A"(ret));
        return ret;
    }

    /*!
     * Enable external interrupts.
     */
    static inline void enable_interrupts()
    {
        asm volatile ("sti");
    }

    /*!
     * Disable external interrupts.
     */
    static inline void disable_interrupts()
    {
        asm volatile ("cli");
    }
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
