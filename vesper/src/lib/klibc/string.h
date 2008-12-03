//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "Macros.h"

/** Encapsulate string manipulation methods. */
class string
{
public:
    string() : len(0), str(0) {}

    INLINE int length() { return len; }
    int cmp(const string& other);
    bool equals(const string& other);

    static int length(const char *str);
    static bool equals(const char *in1, const char *in2);

private:
    int len;
    char *str;
};

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
