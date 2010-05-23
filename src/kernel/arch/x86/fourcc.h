#pragma once

#include "types.h"

template <uint8_t a, uint8_t b, uint8_t c, uint8_t d>
struct FourCC
{
    static const uint32_t value = (((((d << 8) | c) << 8) | b) << 8) | a;
};
