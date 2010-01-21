#pragma once

#include "types.h"
#include "leb128.h"

class lineprogram_regs_t
{
public:
    address_t address;
    int file;
    int line;
    int column;
    bool is_stmt;
    bool basic_block;
    bool end_sequence;
    bool prologue_end;
    bool epilogue_begin;
    int isa;

    lineprogram_regs_t(bool stmt)
        : address(0), file(1), line(1), column(0)
        , is_stmt(stmt), basic_block(false), end_sequence(false), prologue_end(false), epilogue_begin(false)
        , isa(0)
    {
    }

    // Execute opcode of the line program and update regs.
    void execute(address_t from, size_t& offset);
};

enum {
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
    DW_LNS_set_isa = 0x0c // dwarf3
};

struct lnp_filename_t
{
    std::string filename;
    uleb128_t directory_index;
    uleb128_t file_timestamp;
    uleb128_t file_bytes;
};

// Line Number Program header.
class lnp_header_t
{
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
}

class dwarf_debug_lines_t
{
    lnp_header_t* header;
    lineprogram_regs_t current_regs;
    std::vector<lineprogram_regs_t> state_matrix;

public:
    dwarf_debug_lines_t();

    std::string file_name(int index);
    int line_number(address_t address);
};
