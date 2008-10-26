//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "Types.h"

extern "C"  address_t readInstructionPointer();
extern "C"  address_t readStackPointer();
extern "C"  address_t readBasePointer();

extern "C"  void writeStackPointer(address_t ptr);
extern "C"  void writeBasePointer(address_t ptr);

extern "C"  address_t readPageDirectory();
extern "C"  void writePageDirectory(address_t pageDirPhysical);
extern "C"  void flushPageDirectory(void);

extern "C"  void enablePaging(void);
extern "C"  void enableInterrupts(void);
extern "C"  void disableInterrupts(void);

// defined in schedule/CriticalSection.cpp
extern "C" void criticalSection();
extern "C" void endCriticalSection();
// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
