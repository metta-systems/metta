//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "raiifile.h"
#include "../tools/parsedwarf/leb128.h"

using namespace raii_wrapper;

int main(int, char**)
{
    file f("slebtest.txt", fstream::in);
    size_t fsize = f.size();
    char* buffer = new char [fsize];
    f.read(buffer, fsize);

    address_t start = reinterpret_cast<address_t>(buffer);
    size_t offset = 0;
    sleb128_t sleb;

    while (offset < fsize)
    {
        sleb.decode(start, offset);
    }
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
