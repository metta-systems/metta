//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "leb128.h"
#include <vector>
#include <string>

struct lnp_filename_t
{
    std::string filename;
    uleb128_t directory_index;
    uleb128_t file_timestamp;
    uleb128_t file_bytes;

    bool decode(address_t from, size_t& offset);
    void dump();
};

// Line Number Program header.
class lnp_header_t
{
public:
    uint32_t unit_length;
    uint16_t version;
    uint32_t header_length;
    uint8_t  minimum_instruction_length;
    uint8_t  default_is_stmt;
    int8_t   line_base;
    uint8_t  line_range;
    uint8_t  opcode_base;
    std::vector<uleb128_t> standard_opcode_lengths;
    std::vector<std::string> include_directories;
    std::vector<lnp_filename_t> file_names;

    address_t line_program_start;

    void decode(address_t from, size_t& offset);
    void dump();
};

class lineprogram_regs_t
{
public:
    address_t address;
//     size_t    length;
    int file;
    int line;
    int column;
    bool is_stmt;
    bool basic_block;
    bool end_sequence;
    bool prologue_end;
    bool epilogue_begin;
    int isa;
    bool reset_fields, reset_to_initial;
    lnp_header_t& header;

    lineprogram_regs_t(lnp_header_t& h)
        : address(0), file(1), line(1), column(0)
        , is_stmt(h.default_is_stmt), basic_block(false), end_sequence(false), prologue_end(false), epilogue_begin(false)
        , isa(0), reset_fields(false), reset_to_initial(false), header(h)
    {
    }

    lineprogram_regs_t& operator=(const lineprogram_regs_t& other)
    {
        if (this != &other)
        {
            address = other.address;
            file = other.file;
            line = other.line;
            column = other.column;
            is_stmt = other.is_stmt;
            basic_block = other.basic_block;
            end_sequence = other.end_sequence;
            prologue_end = other.prologue_end;
            epilogue_begin = other.epilogue_begin;
            isa = other.isa;
            reset_fields = other.reset_fields;
            reset_to_initial = other.reset_to_initial;
            header = other.header;
        }
        return *this;
    }

    void reset()
    {
        address = 0;
        file = 1;
        line = 1;
        column = 0;
        is_stmt = header.default_is_stmt;
        basic_block = false;
        end_sequence = false;
        prologue_end = false;
        epilogue_begin = false;
        isa = 0;
        reset_to_initial = false;
    }

    bool execute(address_t from, size_t& offset);
    void dump();

    inline address_t address_increment(uint8_t opcode)
    {
        return (opcode / header.line_range) * header.minimum_instruction_length;
    }

    inline int line_increment(uint8_t opcode)
    {
        return header.line_base + (opcode % header.line_range);
    }
};

enum {
    DW_LNS_extended_op = 0x00,
    // Standard opcodes
    DW_LNS_copy = 0x01,
    DW_LNS_advance_pc = 0x02,
    DW_LNS_advance_line = 0x03,
    DW_LNS_set_file = 0x04,
    DW_LNS_set_column = 0x05,
    DW_LNS_negate_stmt = 0x06,
    DW_LNS_set_basic_block = 0x07,
    DW_LNS_const_add_pc = 0x08,
    DW_LNS_fixed_advance_pc = 0x09,
    DW_LNS_set_prologue_end = 0x0a, // dwarf3
    DW_LNS_set_epilogue_begin = 0x0b, // dwarf3
    DW_LNS_set_isa = 0x0c, // dwarf3
    // Extended opcodes
    DW_LNE_end_sequence = 0x01,
    DW_LNE_set_address  = 0x02,
    DW_LNE_define_file  = 0x03
};

class dwarf_debug_lines_t
{
    address_t start;
    size_t    size;
    lnp_header_t header;
    lineprogram_regs_t current_regs;
    std::vector<lineprogram_regs_t> state_matrix;

public:
    dwarf_debug_lines_t(address_t st, size_t sz);

    // Populate state matrix from a given line program.
    bool execute(size_t& offset);

    std::string file_name(address_t address, address_t low_pc, address_t high_pc);
    int line_number(address_t address, address_t low_pc, address_t high_pc);
};
