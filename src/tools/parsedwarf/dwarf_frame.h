//
// Use .debug_frame or .eh_frame to unwind stack.
//
// Entries in a .debug_frame section are aligned on an addressing unit boundary and come in two forms:
// A Common Information Entry (CIE) and a Frame Description Entry (FDE).
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

/* Resides in: .debug_frame, .eh_frame */
class cie_t
{
    uint32_t  length;
    uint32_t  CIE_id;
    uint8_t   version;
    char*     augmentation;
    uleb128_t code_alignment_factor;
    sleb128_t data_alignment_factor;
    uleb128_t return_address_register;
    uint8_t*  initial_instructions;

    void decode(address_t from, size_t& offset);
};

/* Resides in: .debug_frame, .eh_frame */
class fde_t
{
    uint32_t length;
    uint32_t CIE_pointer;
    uint32_t initial_location;
    uint32_t address_range;
    uint8_t* instructions;
};
