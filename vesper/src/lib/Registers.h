//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

extern "C"  address_t read_instruction_pointer();
extern "C"  address_t read_stack_pointer();
extern "C"  address_t read_base_pointer();

extern "C"  void write_stack_pointer(address_t ptr);
extern "C"  void write_base_pointer(address_t ptr);

extern "C"  address_t read_page_directory();
extern "C"  void write_page_directory(address_t pageDirPhysical);
extern "C"  void flush_page_directory(void);

extern "C"  void enable_paging(void);

inline void enable_interrupts(void)  { asm volatile ("sti"); }
inline void disable_interrupts(void) { asm volatile ("cli"); }

// defined in schedule/critical_section.cpp
extern "C" void critical_section();
extern "C" void end_critical_section();

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
