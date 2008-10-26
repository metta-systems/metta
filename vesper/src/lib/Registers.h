//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "Types.h"

extern "C"  address_t read_instruction_pointer();
extern "C"  address_t read_stack_pointer();
extern "C"  address_t read_base_pointer();

extern "C"  void write_stack_pointer(address_t ptr);
extern "C"  void write_base_pointer(address_t ptr);

extern "C"  address_t read_page_directory();
extern "C"  void write_page_directory(address_t pageDirPhysical);
extern "C"  void flush_page_directory(void);

extern "C"  void enable_paging(void);
extern "C"  void enable_interrupts(void);
extern "C"  void disable_interrupts(void);

// defined in schedule/CriticalSection.cpp
extern "C" void critical_section();
extern "C" void end_critical_section();

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
