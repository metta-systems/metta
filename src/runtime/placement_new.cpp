//
// Minimal operator new/delete implementation.
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//INLINED!
// #include "new.h"

// void* operator new(size_t, void* place) throw() { return place; }
// void* operator new[](size_t, void* place) throw() { return place; }
// void operator delete(void*, void*) throw() {}
// void operator delete[](void*, void*) throw() {}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
