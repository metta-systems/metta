//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "string.h"

bool string::equals(const char *in1, const char *in2)
{
	char *left = (char *)in1;
	char *right = (char *)in2;
	while(*left && *right && *left == *right)
		left++, right++;
	if (*left != *right)
		return false;
	return true;
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
