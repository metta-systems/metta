//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "macros.h"
#include "string.h"

#ifndef __HAVE_ARCH_STRNCASECMP
int strncasecmp(const char *s1, const char *s2, size_t n)
{
	int c1, c2;

	do {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while ((--n > 0) && c1 == c2 && c1 != 0);
	return c1 - c2;
}
EXPORT_SYMBOL(strncasecmp);
#endif

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
