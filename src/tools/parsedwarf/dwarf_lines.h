#pragma once

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
};

class dwarf_debug_lines_t
{
};
