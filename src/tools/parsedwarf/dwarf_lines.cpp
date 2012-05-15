//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "dwarf_lines.h"
#include "local_panic.h"
#include "dwarf_debug.h"

/**
 * @returns true if program line is complete and can be added to matrix, false otherwise.
 */
bool lineprogram_regs_t::execute(address_t from, size_t& offset)
{
    if (reset_to_initial)
        reset();

    if (reset_fields)
    {
        basic_block = false;
        prologue_end = false;
        epilogue_begin = false;
        reset_fields = false;
    }

    uint8_t opcode = *reinterpret_cast<uint8_t*>(from + offset);
    ++offset;

    // Decode opcode into standard, extended or special and execute updating current information.
    if (opcode < header.opcode_base) // std opcode
    {
        bool append_line = false;
        switch (opcode)
        {
            case DW_LNS_copy:
                reset_fields = true;
                append_line = true;
                DPRINT("DW_LNS_copy\n");
                break;
            case DW_LNS_advance_pc:
            {
                address_t operand = uleb128_t::decode(from, offset, -1);
                address += operand * header.minimum_instruction_length;
                DPRINT("DW_LNS_advance_pc(%u) => %08x\n", operand, address);
                break;
            }
            case DW_LNS_advance_line:
            {
                int operand = sleb128_t::decode(from, offset, -1);
                line += operand;
                DPRINT("DW_LNS_advance_line(%d) => %d\n", operand, line);
                break;
            }
            case DW_LNS_set_file:
            {
                file = uleb128_t::decode(from, offset, -1);
                DPRINT("DW_LNS_set_file(%d)\n", file);
                break;
            }
            case DW_LNS_set_column:
            {
                column = uleb128_t::decode(from, offset, -1);
                DPRINT("DW_LNS_set_column(%d)\n", column);
                break;
            }
            case DW_LNS_negate_stmt:
                is_stmt = !is_stmt;
                DPRINT("DW_LNS_negate_stmt => %d\n", is_stmt);
                break;
            case DW_LNS_set_basic_block:
                basic_block = true;
                DPRINT("DW_LNS_set_basic_block(true)\n");
                break;
            case DW_LNS_const_add_pc:
            {
                address_t operand = address_increment(255) * header.minimum_instruction_length;
                address += operand;
                DPRINT("DW_LNS_const_add_pc (add %u) => %08x\n", operand, address);
                break;
            }
            case DW_LNS_fixed_advance_pc:
            {
                uint16_t operand = *reinterpret_cast<uint16_t*>(from + offset);
                offset += sizeof(uint16_t);
                address += operand;
                DPRINT("DW_LNS_fixed_advance_pc(%u) => %08x\n", operand, address);
                break;
            }
            case DW_LNS_set_prologue_end:
                prologue_end = true;
                DPRINT("DW_LNS_set_prologue_end(true)\n");
                break;
            case DW_LNS_set_epilogue_begin:
                epilogue_begin = true;
                DPRINT("DW_LNS_set_epilogue_begin(true)\n");
                break;
            case DW_LNS_set_isa:
            {
                isa = uleb128_t::decode(from, offset, -1);
                DPRINT("DW_LNS_set_isa(%d)\n", isa);
                break;
            }
            case DW_LNS_extended_op:
            {
                uleb128_t ext_area_length;
                ext_area_length.decode(from, offset);
                size_t prev_offset = offset;
                uint8_t sub_opcode = *reinterpret_cast<uint8_t*>(from + offset);
                ++offset;
                switch (sub_opcode)
                {
                    case DW_LNE_end_sequence:
                        end_sequence = true;
                        reset_to_initial = true;
                        append_line = true;
                        DPRINT("DW_LNE_end_sequence\n");
                        break;
                    case DW_LNE_set_address:
                    {
                        address = *reinterpret_cast<uint32_t*>(from + offset);//TODO: address is relocatable
                        offset += sizeof(uint32_t);
                        DPRINT("DW_LNE_set_address(%08x)\n", address);
                        break;
                    }
                    case DW_LNE_define_file:
                    {
                        lnp_filename_t fname;
                        fname.decode(from, offset);
                        header.file_names.push_back(fname);
                        DPRINT("DW_LNE_define_file\n");
                        fname.dump();
                        break;
                    }
                    default:
                        DPRINT("UNKNOWN EXTENDED OPCODE 0x%x\n", sub_opcode);
                        return false;
                }
                if (offset != prev_offset + ext_area_length)
                {
                    DPRINT("STREAM OUT OF SYNC! offset 0x%x should be 0x%x\n", offset, prev_offset + ext_area_length);
                    return false;
                }
                break;
            }
            default:
                DPRINT("UNKNOWN OPCODE 0x%x\n", opcode);
                // TODO: skip unknown opcode by reading as many leb128 dummy parameters as specified in standard_opcode_lengths table
                return false;
        }
        return append_line; // standard opcodes do not trigger adding a new matrix line by default
    }

    // special opcode
    opcode -= header.opcode_base;
    address += address_increment(opcode);
    line += line_increment(opcode);
    DPRINT("special opcode %x (byte %x): address increment %u, line increment %i\n", opcode, opcode + header.opcode_base, address_increment(opcode), line_increment(opcode));
    reset_fields = true;
    return true; // special opcodes trigger adding a new matrix line
}

void lineprogram_regs_t::dump()
{
    DPRINT("*lineprog 0x%08x %d %d:%d stmt:%s bblock:%s endseq:%s prologue:%s epilogue:%s isa:%d\n",
           address, file, line, column, is_stmt?"yes":"no", basic_block?"yes":"no", end_sequence?"yes":"no", prologue_end?"yes":"no", epilogue_begin?"yes":"no", isa);
}

void lnp_header_t::decode(address_t from, size_t& offset)
{
    unit_length = *reinterpret_cast<uint32_t*>(from + offset);
    if (unit_length == 0xffffffff)
        PANIC("DWARF64 is not supported!");
    offset += sizeof(uint32_t);
    version = *reinterpret_cast<uint16_t*>(from + offset);
    offset += sizeof(uint16_t);
    header_length = *reinterpret_cast<uint32_t*>(from + offset);
    offset += sizeof(uint32_t);
    line_program_start = from + offset + header_length;

    minimum_instruction_length = *reinterpret_cast<uint8_t*>(from + offset);
    offset += sizeof(uint8_t);
    default_is_stmt = *reinterpret_cast<uint8_t*>(from + offset);
    offset += sizeof(uint8_t);
    line_base = *reinterpret_cast<int8_t*>(from + offset);
    offset += sizeof(int8_t);
    line_range = *reinterpret_cast<uint8_t*>(from + offset);
    offset += sizeof(uint8_t);
    opcode_base = *reinterpret_cast<uint8_t*>(from + offset);
    offset += sizeof(uint8_t);

    standard_opcode_lengths.clear();
    for (int i = 1; i < opcode_base; ++i)
    {
        uleb128_t size;
        size.decode(from, offset);
        standard_opcode_lengths.push_back(size);
    }

    include_directories.clear();
    char* p = reinterpret_cast<char*>(from + offset);
    while (*p)
    {
        char* str = p;
        while (*p)
        {
            ++offset;
            ++p;
        }
        include_directories.push_back(std::string(str, p - str));
        ++p;
        ++offset;
    }
    ++offset;

    file_names.clear();
    lnp_filename_t fname;
    while (fname.decode(from, offset))
        file_names.push_back(fname);
}

void lnp_header_t::dump()
{
#if DWARF_DEBUG
    printf("* line program header:\n"
           "  unit_length: %d\n"
           "  version: 0x%x\n"
           "  header_length: %d, line program start: 0x%x\n"
           "  minimum_instruction_length: %d\n"
           "  default_is_stmt: %s\n"
           "  line_base: %d\n"
           "  line_range: %u\n"
           "  opcode_base: %u\n",
    unit_length, version,
    header_length, line_program_start,
    minimum_instruction_length,
    default_is_stmt ? "yes" : "no",
    line_base, line_range,
    opcode_base);

    for (size_t i = 0; i < standard_opcode_lengths.size(); ++i)
        printf("%x ", (uint32_t)standard_opcode_lengths[i]);
    printf("\n");
    for (size_t i = 0; i < include_directories.size(); ++i)
        printf("%03d include %s\n", i+1, include_directories[i].c_str());
    for (size_t i = 0; i < file_names.size(); ++i)
    {
        printf("%03d ", i+1);
        file_names[i].dump();
    }
#endif
}

bool lnp_filename_t::decode(address_t from, size_t& offset)
{
    char* s = reinterpret_cast<char*>(from + offset);
    if (!*s)
    {
        ++offset;
        return false;
    }

    char* str = s;
    while (*s++)
        ++offset;
    ++offset;

    filename = std::string(str, s - str);
    directory_index.decode(from, offset);
    file_timestamp.decode(from, offset);
    file_bytes.decode(from, offset);

    return true;
}

void lnp_filename_t::dump()
{
    DPRINT("* filename: %s dirindex %d file_timestamp %d file_bytes %d\n", filename.c_str(), (uint32_t)directory_index, (uint32_t)file_timestamp, (uint32_t)file_bytes);
}

dwarf_debug_lines_t::dwarf_debug_lines_t(address_t st, size_t sz)
    : start(st)
    , size(sz)
    , current_regs(header)
{
}

bool dwarf_debug_lines_t::execute(size_t& offset)
{
    size_t start_offset = offset;

    state_matrix.clear();
    header.decode(start, offset);
    current_regs.reset();

    if (start + offset != header.line_program_start)
    {
        DPRINT("LINE PROGRAM HEADER OUT OF SYNC! offset 0x%x should be 0x%x\n", start + offset, header.line_program_start);
        return false;
    }

    header.dump();

    while (offset - start_offset < header.unit_length + 4)
    {
        if (current_regs.execute(start, offset))
        {
            current_regs.dump();
            state_matrix.push_back(current_regs);
        }
    }

    return true;
}

std::string dwarf_debug_lines_t::file_name(address_t address, address_t low_pc, address_t high_pc)
{
    for (size_t i = 0; i < state_matrix.size()-1; ++i)
    {
        address_t pc_l = state_matrix[i].address;
        address_t pc_h = state_matrix[i+1].address;
        DPRINT("%08x <= %08x <= %08x <= %08x <= %08x\n", low_pc, pc_l, address, pc_h, high_pc);
        if (pc_l >= low_pc && pc_l <= address
         && pc_h >= address && pc_h <= high_pc)
            return header.file_names[state_matrix[i].file-1].filename;
    }

    return std::string();
}

int dwarf_debug_lines_t::line_number(address_t address, address_t low_pc, address_t high_pc)
{
    for (size_t i = 0; i < state_matrix.size()-1; ++i)
    {
        address_t pc_l = state_matrix[i].address;
        address_t pc_h = state_matrix[i+1].address;
        DPRINT("%08x <= %08x <= %08x <= %08x <= %08x\n", low_pc, pc_l, address, pc_h, high_pc);
        if (pc_l >= low_pc && pc_l <= address
         && pc_h >= address && pc_h <= high_pc)
            return state_matrix[i].line;
    }

    return 0;
}
