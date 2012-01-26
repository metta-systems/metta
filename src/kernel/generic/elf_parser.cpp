//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "elf_parser.h"
#include "default_console.h"
#include "memutils.h"
// #include "memory.h"
// #include "minmax.h"
#include "config.h"
#include "panic.h"

#if ELF_LOADER_DEBUG
#define D(s) s
#else
#define D(s)
#endif

#if ELF_RELOC_DEBUG_V
#define V(s) s
#else
#define V(s)
#endif

using namespace elf32;

//======================================================================================================================
// elf_parser_t::program_iterator impl
//======================================================================================================================

elf_parser_t::program_iterator::program_iterator(program_header_t* entry, program_header_t* end, size_t entry_size)
    : ptr(entry)
    , end(end)
    , entry_size(entry_size)
{
}

program_header_t& elf_parser_t::program_iterator::operator *()
{
    return *ptr; // FIXME: dereferencing end() will fault
}

void elf_parser_t::program_iterator::operator ++()
{
    ptr = reinterpret_cast<program_header_t*>(reinterpret_cast<char*>(ptr) + entry_size);
    if (ptr > end)
        ptr = 0;
    return;
}

//======================================================================================================================
// elf_parser_t::section_iterator impl
//======================================================================================================================

elf_parser_t::section_iterator::section_iterator(section_header_t* entry, section_header_t* end, size_t entry_size)
    : ptr(entry)
    , end(end)
    , entry_size(entry_size)
{
}

section_header_t& elf_parser_t::section_iterator::operator *()
{
    return *ptr; // FIXME: dereferencing end() will fault
}

void elf_parser_t::section_iterator::operator ++(int)
{
	operator++();
}

void elf_parser_t::section_iterator::operator ++()
{
    ptr = reinterpret_cast<section_header_t*>(reinterpret_cast<char*>(ptr) + entry_size);
    if (ptr > end)
        ptr = 0;
    return;
}

//======================================================================================================================
// elf_parser_t impl
//======================================================================================================================

elf_parser_t::elf_parser_t()
    : header(NULL)
    , strtab(NULL)
    , symtab(NULL)
{
}

elf_parser_t::elf_parser_t(address_t image_base)
    : header(NULL)
    , strtab(NULL)
    , symtab(NULL)
{
    parse(image_base);
}

static inline address_t elf2loc(header_t* base, size_t offset)
{
    return reinterpret_cast<address_t>(base) + offset;
}

elf_parser_t::program_iterator elf_parser_t::program_headers_begin()
{
    if (!header->phnum)
        return program_headers_end();

    return program_iterator(program_header(0), program_header(header->phnum-1), header->phentsize);
}

elf_parser_t::program_iterator elf_parser_t::program_headers_end()
{
    return program_iterator(0, 0, 0);
}

program_header_t* elf_parser_t::program_header(int index) const
{
    if (index < 0 || index >= header->phnum)
        return 0;
    return reinterpret_cast<program_header_t*>(elf2loc(header, header->phoff + index * header->phentsize));
}

elf_parser_t::section_iterator elf_parser_t::section_headers_begin()
{
    if (!header->shnum)
        return section_headers_end();

    return section_iterator(section_header(0), section_header(header->shnum-1), header->shentsize);
}

elf_parser_t::section_iterator elf_parser_t::section_headers_end()
{
    return section_iterator(0, 0, 0);
}

section_header_t* elf_parser_t::section_header(int index) const
{
    if (index < 0 || index >= header->shnum)
        return 0;
    return reinterpret_cast<section_header_t*>(elf2loc(header, header->shoff + index * header->shentsize));
}

section_header_t* elf_parser_t::section_header_by_type(word_t type) const
{
    section_header_t* s;
    for (int i = 0; i < header->shnum; i++)
    {
        s = section_header(i);
        if (s->type == type)
            return s;
    }
    return 0;
}

section_header_t* elf_parser_t::section_shstring_table() const
{
    if (header->shstrndx == SHN_UNDEF)
        return 0;
    return section_header(header->shstrndx);
}

section_header_t* elf_parser_t::section_string_table() const
{
    if (strtab == 0)
        strtab = section_header(".strtab");
    return strtab;
}

size_t elf_parser_t::string_entries_count() const
{
    section_header_t* _strtab = section_string_table();
    if (!_strtab)
        return 0;
    return _strtab->size / _strtab->entsize;
}

section_header_t* elf_parser_t::section_symbol_table() const
{
    if (symtab == 0)
        symtab = section_header_by_type(SHT_SYMTAB);
    return symtab;
}

size_t elf_parser_t::symbol_entries_count() const
{
    section_header_t* _symtab = section_symbol_table();
    if (!_symtab)
        return 0;
    return _symtab->size / _symtab->entsize;
}

section_header_t* elf_parser_t::section_header(cstring_t name) const
{
    section_header_t* shstrtab = section_shstring_table();
    if (!shstrtab)
        return 0;

    section_header_t* s;
    for (int i = 0; i < header->shnum; i++)
    {
        s = section_header(i);
        if (name == strtab_pointer(shstrtab, s->name))
            return s;
    }
    return 0;
}

const char* elf_parser_t::strtab_pointer(section_header_t* _strtab, elf32::word_t name_offset) const
{
    if (!_strtab)
        return 0;
    return reinterpret_cast<char*>(header) + _strtab->offset + name_offset;
}

bool elf_parser_t::is_valid() const
{
//     kconsole << RED << "elf file parsing @" << start << " failed: " << s <<endl;
#define ERROR_RETURN_ON(x, s) \
if (x) { \
    return false; \
}

    ERROR_RETURN_ON(!header, "no header");
    ERROR_RETURN_ON(header->magic != ELF_MAGIC, "bad magic")
    ERROR_RETURN_ON(header->elfclass != ELF_CLASS_32, "wrong class")
    ERROR_RETURN_ON(header->data != ELF_DATA_2LSB, "wrong endianness")
    ERROR_RETURN_ON(header->machine != EM_386, "wrong architecture")
    ERROR_RETURN_ON(header->version != EV_CURRENT, "wrong version")

    return true;
#undef ERROR_RETURN_ON
}

/*!
 * Parse ELF program image, finding corresponding sections.
 */
bool elf_parser_t::parse(address_t start)
{
    header = reinterpret_cast<header_t*>(start);

    if (!is_valid())
        return false;

    return true;
}

bool elf_parser_t::is_relocatable() const
{
    return header && (header->type == ET_REL
        || section_header_by_type(SHT_REL) != 0);
// we don't support RELA sections yet
//         || section_header_by_type(SHT_RELA) != 0);
}

bool elf_parser_t::relocate_to(address_t load_address)
{
    section_header_t* shstrtab = section_shstring_table();
    if (!shstrtab)
        return false;

	kconsole << "Relocating module to " << load_address << endl;

    // Traverse all sections, find relocation sections and apply them.
    section_header_t* rel_section;
    for (int i = 0; i < header->shnum; i++)
    {
        rel_section = section_header(i);
        if (rel_section->type == SHT_REL)
        {
            section_header_t* target_sect = section_header(rel_section->info);
            if (!(target_sect->flags & SHF_ALLOC))
                continue;
            V(kconsole << "Found rel section " << strtab_pointer(shstrtab, rel_section->name) << " @" << rel_section->offset << endl);
            elf32::rel_t* rels = reinterpret_cast<elf32::rel_t*>(elf2loc(header, rel_section->offset));
            ASSERT(sizeof(rels[0]) == rel_section->entsize); // Standard says entsize should tell the actual entry size
            size_t nrels = rel_section->size / sizeof(rels[0]);
            section_header_t* _symtab = section_header(rel_section->link);
            symbol_t* symbols = _symtab ? reinterpret_cast<symbol_t*>(elf2loc(header, _symtab->offset)) : 0;
            if (symbols)
            {
                for (size_t j = 0; j < nrels; j++)
                {
                    elf32::rel_t& rel = rels[j];
                    symbol_t& sym = symbols[ELF32_R_SYM(rel.info)];

                    // TODO: check return value
                    apply_relocation(rel, sym, target_sect, load_address);
                }
            }
        }
    }

    return true;
}

bool elf_parser_t::apply_relocation(elf32::rel_t& rel, symbol_t& sym, section_header_t* target_sect, address_t load_address)
{
    V(section_header_t* shstrtab = section_shstring_table());

    uint32_t result;
    address_t P = (target_sect ? target_sect->vaddr : load_address) + rel.offset;
    uint32_t  A = *reinterpret_cast<uint32_t*>(P);
    address_t S = 0;

    if (ELF32_ST_TYPE(sym.info) == 0 && sym.shndx == 0)
        PANIC("Invalid relocatable image: undefined symbols!");

    if (ELF32_ST_TYPE(sym.info) == STT_SECTION)
    {
        S = section_header(sym.shndx)->vaddr;
        V(kconsole << "S is section '" << strtab_pointer(shstrtab, section_header(sym.shndx)->name) << "'" << endl);
    }
    else
    {
        S = sym.value;
        V(kconsole << "S is symbol '" << strtab_pointer(section_string_table(), sym.name) << "' of type " << ELF32_ST_TYPE(sym.info) << " for section " << sym.shndx << " '" << strtab_pointer(shstrtab, section_header(sym.shndx)->name) << "'" << endl);
        V(section_header(sym.shndx)->dump(shstring_table()));
    }

    switch (ELF32_R_TYPE(rel.info))
    {
        case R_386_NONE:
            result = A;
            break;
        case R_386_32:
            result = S + A;
            V(kconsole << "R_386_32: S " << S << " + A " << A << " = " << result << endl);
            break;
        case R_386_PC32:
            result = S + A - P;
            V(kconsole << "R_386_PC32: S " << S << " + A " << A << " - P " << P << " = " << result << endl);
            break;
        default:
            kconsole << "Unknown relocation type " << ELF32_R_TYPE(rel.info) << ", skipped, expect crashes!" << endl;
            break;
    }
    V(kconsole << P << " = " << A << " -> " << result << endl);
    *reinterpret_cast<uint32_t*>(P) = result;

    return true;
}

// TODO: use debugging info if present
cstring_t elf_parser_t::find_symbol(address_t addr, address_t* symbol_start)
{
//     section_header_t* debug_table = section_header(".debug_frame");

    address_t max = 0;
    symbol_t* fallback_symbol = 0;
    section_header_t* symbol_table = section_symbol_table();
    if (!symbol_table)
        return NULL;

    for (unsigned int i = 0; i < symbol_entries_count(); i++)
    {
        symbol_t* symbol = reinterpret_cast<symbol_t*>(symbol_table->vaddr + i * symbol_table->entsize);

        if ((addr >= symbol->value) && (addr < symbol->value + symbol->size))
        {
            const char* c = strtab_pointer(section_string_table(), symbol->name);

            if (symbol_start)
                *symbol_start = symbol->value;
            return c;
        }

        if (symbol->value > max && symbol->value <= addr)
        {
            max = symbol->value;
            fallback_symbol = symbol;
        }
    }

    // Search for symbol with size failed, now take a wild guess.
    // Use a biggest symbol value less than addr (if found).
    if (fallback_symbol)
    {
        const char* c = strtab_pointer(section_string_table(), fallback_symbol->name);

        if (symbol_start)
            *symbol_start = fallback_symbol->value;
        return c;
    }

    if (symbol_start)
        *symbol_start = 0;
    return NULL;
}

// Find symbol str in symbol table and return its absolute address.
address_t elf_parser_t::find_symbol(cstring_t str)
{
    section_header_t* symbol_table = section_symbol_table();
    if (!symbol_table)
        return 0;

    V(kconsole << symbol_entries_count() << " symbols to consider." << endl);
    V(kconsole << "Symbol table @ " << start() + symbol_table->offset << endl);
    V(kconsole << "BSS          @ " << start() + section_header(".bss")->offset << endl);

    for (unsigned int i = 0; i < symbol_entries_count(); i++)
    {
        // FIXME: we use symbol_table->offset here and ->vaddr in the function above, unify both these to ->vaddr
        // This would require copying elf file's symbol and string table somewhere in area allocated by module_loader.
        symbol_t* symbol = reinterpret_cast<symbol_t*>(start() + symbol_table->offset + i * symbol_table->entsize);

        const char* c = strtab_pointer(section_string_table(), symbol->name);
        V(kconsole << "Looking at symbol " << c << " @ " << symbol << endl);
        if (str == c)
        {
            if (ELF32_ST_TYPE(symbol->info) == STT_SECTION)
            {
                return section_header(symbol->shndx)->vaddr; //offset + start();
            }
            else
            {
                return /*section_header(symbol->shndx)->offset +*/ symbol->value /*+ start()*/;
            }
        }
    }

    return 0;
}

//TODO:
// symbol_table_t
// iterator for searching the symbols by name
// non-linear lookups
// support elf .so hash tables?

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
