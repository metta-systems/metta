#include "dwarf_lines.h"
#include "local_panic.h"

void lineprogram_regs_t::execute(address_t from, size_t& offset)
{
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

    for (int i = 1; i < opcode_base; i++)
    {
        uleb128_t size;
        size.decode(from, offset);
        standard_opcode_lengths.push_back(size);
    }

    char* p = reinterpret_cast<char*>(from + offset);
    while (*p)
    {
        char* str = p;
        while (*p++)
            ++offset;
        include_directories.push_back(std::string(str));
        ++p;
        ++offset;
    }
    ++offset;

    char* s = reinterpret_cast<char*>(from + offset);
    while (*s)
    {
        lnp_filename_t fname;

        char* str = s;
        while (*s++)
            ++offset;
        ++offset;

        fname.filename = std::string(str);
        fname.directory_index.decode(from, offset);
        fname.file_timestamp.decode(from, offset);
        fname.file_bytes.decode(from, offset);

        file_names.push_back(fname);
        s = reinterpret_cast<char*>(from + offset);
    }
    ++offset;
}

