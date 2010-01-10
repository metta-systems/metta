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

        while(true)
        {
            uint8_t byte = *reinterpret_cast<uint8_t*>(from + offset);
            data |= ((byte & 0x7F) << shift);
            shift += 7;
            /* sign bit of byte is second high order bit (0x40) */
            if (!(byte & 0x80))
                break;
            offset++;
        }
        if ((shift < size) && (sign bit of byte is set))
            /* sign extend */
            result |= -(1 << shift);
    }
    operator int32_t() { return data; }
    sleb128_t operator =(int32_t d) { data = d; return *this; }
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
            if (!(byte & 0x80))
                break;
            offset++;
            shift += 7;
        }
    }
    operator uint32_t() { return data; }
    uleb128_t operator =(uint32_t d) { data = d; return *this; }
};
