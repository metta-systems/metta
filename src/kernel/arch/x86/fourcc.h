//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

template <uint8_t a, uint8_t b, uint8_t c, uint8_t d>
struct four_cc
{
    static const uint32_t value = (((((d << 8) | c) << 8) | b) << 8) | a;
};

// Bigendian version for little-endian machines, should use endian-magic to pull it off for real.
template <uint8_t h, uint8_t g, uint8_t f, uint8_t e, uint8_t d, uint8_t c, uint8_t b, uint8_t a>
struct Magic64BE
{
    static const uint64_t value = ((((((((((((((uint64_t)a << 8) | b) << 8) | c) << 8) | d) << 8) | e) << 8) | f) << 8) | g) << 8) | h;
};
