//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

class atomic_t
{
public:
	static address_t exchange(address_t *lock, address_t new_val);
};
// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
