//
// Decoders for signed and unsigned LEB128 numbers.
// According to DWARF3 Format Specification.
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

class sleb128_t
{
    int32_t data;
public:
    sleb128_t() : data(0) {}
    sleb128_t(int32_t d) : data(d) {}
    void decode(address_t from, size_t& offset)
    {
        data = 0;
        int shift = 0;
        int size = 32; //number of bits in signed integer
        uint8_t byte;

        while(true)
        {
            byte = *reinterpret_cast<uint8_t*>(from + offset);
            data |= ((byte & 0x7F) << shift);
            shift += 7;
            ++offset;
            if (!(byte & 0x80))
                break;
        }
        /* sign bit of byte is second high order bit (0x40) */
        if ((shift < size) && (byte & 0x40))
            /* sign extend */
            data |= -(1 << shift);
    }
    static int32_t decode(address_t from, size_t& offset, int /*dummy*/)
    {
        sleb128_t d;
        d.decode(from, offset);
        return d;
    }
    operator int32_t() { return data; }
    sleb128_t operator =(int32_t d) { data = d; return *this; }
    bool operator<(sleb128_t other) const { return data < other.data; }
};

class uleb128_t
{
    uint32_t data;
public:
    uleb128_t() : data(0) {}
    uleb128_t(uint32_t d) : data(d) {}
    void decode(address_t from, size_t& offset)
    {
        data = 0;
        int shift = 0;
        while(true)
        {
            uint8_t byte = *reinterpret_cast<uint8_t*>(from + offset);
            data |= ((byte & 0x7F) << shift);
            shift += 7;
            ++offset;
            if (!(byte & 0x80))
                break;
        }
    }
    static uint32_t decode(address_t from, size_t& offset, int /*dummy*/)
    {
        uleb128_t d;
        d.decode(from, offset);
        return d;
    }
    operator uint32_t() { return data; }
    uleb128_t operator =(uint32_t d) { data = d; return *this; }
    bool operator<(uleb128_t other) const { return data < other.data; }
};
