//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "cstring.h"

string_ascii_trait::code_point string_ascii_trait::get_code_point(const char* data) const
{
    return code_point(*data);
}

size_t string_ascii_trait::get_sequence_length(const char) const
{
    return 1;
}


string_utf8_trait::code_point string_utf8_trait::get_code_point(const char* data) const
{
    size_t length = get_sequence_length(*data);
    switch (length)
    {
        case 1:
            return code_point(*data);
        case 2:
            return ((*data << 6) & 0x7FF) + (*(data + 1) & 0x3F);
        case 3:
            return ((*data << 12) & 0xFFFF) + ((*(data + 1) << 6) & 0xFFF) + (*(data + 2) & 0x3F);
        case 4:
            return ((*data << 18) & 0x1FFFFF) + ((*(data + 1) << 12) & 0x3FFFF) + ((*(data + 2) << 6) & 0xFFF) + ((*data + 3) & 0x3F);
    }
    return 0;
}

size_t string_utf8_trait::get_sequence_length(const char data) const
{
    if (!(data & 0x80))
        return 1;
    else if ((data >> 5) == 0x6)
        return 2;
    else if ((data >> 4) == 0xE)
        return 3;
    else if ((data >> 3) == 0x1E)
        return 4;
    return 0;//assert here?
}
