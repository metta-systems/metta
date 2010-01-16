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
            printf("encoded byte %x, decoded data %x\n", byte, data);
            shift += 7;
            offset++;
            if (!(byte & 0x80))
                break;
        }
        /* sign bit of byte is second high order bit (0x40) */
        if ((shift < size) && (byte & 0x40))
            /* sign extend */
            data |= -(1 << shift);
        printf("done decoding sleb128 number, final value %d\n", data);
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
            shift += 7;
            offset++;
            if (!(byte & 0x80))
                break;
        }
    }
    operator uint32_t() { return data; }
    uleb128_t operator =(uint32_t d) { data = d; return *this; }
};
