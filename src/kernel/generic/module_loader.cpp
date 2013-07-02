//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "module_loader.h"
#include "config.h"
#include "algorithm"
#include "default_console.h"
#include "memory.h"
#include "logger.h"
#include "debugger.h"
#include "panic.h"
#include "fourcc.h"
#include "infopage.h"
#include "bootinfo.h" // for print_module_map()
// #include "exceptions.h"
#include "symbol_table_finder.h"

/**
 * @class module_loader_t
 *
 * Load modules into last_load_address, allocate ST_ALLOC sections, copy only ST_LOAD sections
 * and relocate the resulting code.
 */

struct module_descriptor_t
{
    uint32_t magic;         // 'MDUL'
    char name[32];          // name of the module
    module_descriptor_t* previous;
    module_loader_t::module_entry entry;

    inline module_descriptor_t()
        : magic(four_cc<'M','D','U','L'>::value)
        , name({0})
        , previous(0)
        , entry()
    {}
} PACKED;

void elf32::program_header_t::dump()
{
    const char* flag_names[32] = {
        "PF_EXECUTE", "PF_WRITE", "PF_READ", "bit 3", "bit 4", "bit 5", "bit 6", "bit 7", "bit 8", "bit 9", "bit 10", "bit 11", "bit 12", 
        "bit 13", "bit 14", "bit 15", "bit 16", "bit 17", "bit 18", "bit 19", "bit 20", "bit 21", "bit 22", "bit 23", "bit 24", "bit 25", 
        "bit 26", "bit 27", "bit 28", "bit 29", "bit 30", "bit 31"
    };

    kconsole << "------------------------------------------------------------------------" << endl
             << "Program header" << endl
             << "Type " << type << ", flags " << flags << ", align " << align << endl
             << "VAddr " << vaddr << ", PAddr " << paddr << ", Memsize " << memsz << ", FAddr " << offset << ", FSize " << filesz  << endl;
    const char* comma = "";
    for (int x = 0; x < 32; ++x)
    {
        if (flags & (1 << x)) {
            kconsole << comma << flag_names[x];
            comma = ", ";
        }
    }
    kconsole << endl;
}

void elf32::section_header_t::dump(const char* shstrtab)
{
    const char* flag_names[32] = {
        "SHF_WRITE", "SHF_ALLOC", "SHF_EXECINSTR", "bit 3", "SHF_MERGE", "SHF_STRINGS", "SHF_INFO_LINK", "SHF_LINK_ORDER", 
        "SHF_OS_NONCONFORMING", "SHF_GROUP", "SHF_TLS", "bit 11", "bit 12", "bit 13", "bit 14", "bit 15", "bit 16", "bit 17", "bit 18", 
        "bit 19", "bit 20", "bit 21", "bit 22", "bit 23", "bit 24", "bit 25", "bit 26", "bit 27", "bit 28", "bit 29", "bit 30", "bit 31"
    };
    // 0 .group        00000008  00000000  00000000  00000034  2**2
    // CONTENTS, READONLY, EXCLUDE, GROUP, LINK_ONCE_DISCARD
    //word_t  link;          /**< Index of another section */
    //word_t  info;          /**< Additional section information */
    //word_t  entsize;       /**< Entry size if section holds table */

    kconsole << "------------------------------------------------------------------------" << endl
             << "Section header" << endl
             << "Name " << (shstrtab + name) << ", type " << type << ", flags " << flags << endl
             << "VAddr " << vaddr << ", FAddr " << offset << ", Size " << size << ", Align " << addralign << endl
             << "Link " << link << ", Info " << info << ", Entry Size " << entsize << endl;
    const char* comma = "";
    for (int x = 0; x < 32; ++x)
    {
        if (flags & (1 << x)) {
            kconsole << comma << flag_names[x];
            comma = ", ";
        }
    }
    kconsole << endl;
}

void print_module_map()
{
    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;
    address_t from = bi->module_load_end();

    kconsole << "********* Loaded modules *********" << endl;
    module_descriptor_t* module = reinterpret_cast<module_descriptor_t*>(from - sizeof(module_descriptor_t));
    while (module)
    {
        if (module->magic != four_cc<'M','D','U','L'>::value)
        {
            kconsole << "*** No valid signature found." << endl;
            break;
        }

        kconsole << "*** " << module->name << " @ " << module->entry.load_base << ".." << module->entry.load_base + module->entry.loaded_size << ", size " << int(module->entry.loaded_size) << " bytes. Entry " << module->entry.entry_point << ", symtab " << module->entry.symtab_start << ", strtab " << module->entry.strtab_start << endl;
        module = module->previous;
    }
    kconsole << "**********************************" << endl;
}

static bool module_already_loaded(address_t from, cstring_t name, module_descriptor_t*& out_mod)
{
    module_descriptor_t* module = reinterpret_cast<module_descriptor_t*>(from - sizeof(module_descriptor_t));
    while (module)
    {
        if (module->magic != four_cc<'M','D','U','L'>::value)
        {
            // kconsole << "*** No valid signature found." << endl;
            break;
        }

        if (name == module->name)
        {
            out_mod = module;
            return true;
        }
        module = module->previous;
    }
    return false;    
}

// List names of all loaded modules so far.
module_loader_t::strvec
module_loader_t::loaded_module_names()
{
    strvec out(strvec_alloc(PVS(heap)));

    module_descriptor_t* module = reinterpret_cast<module_descriptor_t*>(*d_last_available_address - sizeof(module_descriptor_t));
    while (module)
    {
        if (module->magic != four_cc<'M','D','U','L'>::value)
        {
            // kconsole << "*** No valid signature found." << endl;
            break;
        }
        out.push_back(module->name);
        module = module->previous;
    }
    return out;
}

static bool starts_with(const cstring_t& str, const char* prefix)
{
    uint32_t i = 0;
    while (prefix[i] && i < str.length() && str[i] == prefix[i])
        ++i;
    if (i < str.length() && str[i] == prefix[i])
       return true;
    return false;
}

static bool ends_with(const cstring_t& str, const cstring_t& suffix)
{
    if (str.length() < suffix.length())
        return false;

    uint32_t i = 1;
    while ((i <= suffix.length()) && (str[str.length()-i] == suffix[suffix.length()-i]))
    {
        ++i;
    }

    // Compared to end..
    if (i == suffix.length()+1)
       return true;

    return false;
}

module_symbols_t::symmap
symbol_table_finder_t::all_symbols(const char* suffix)
{
    logger::function_scope fs("symbol_table_finder_t::all_symbols");

    module_symbols_t::symmap symbols(module_symbols_t::symmap_alloc(PVS(heap)));

    size_t n_entries = symbol_table->size / symbol_table->entsize;
    logger::trace() << "All symbols: " << int(n_entries);
    logger::trace() << "Symbol table @ " << base + symbol_table->offset;
    logger::trace() << "String table @ " << base + string_table->offset;

    // OS_TRY{
        for (size_t i = 0; i < n_entries; i++)
        {
            elf32::symbol_t* symbol = reinterpret_cast<elf32::symbol_t*>(base + symbol_table->offset + i * symbol_table->entsize);
            const char* c = reinterpret_cast<const char*>(base + string_table->offset + symbol->name);

            if (ends_with(c, suffix))
            {
                logger::trace() << "all_symbols adding symbol " << c;
                symbols.insert(std::make_pair(c, symbol));
            }
        }
    // }
    // OS_CATCH("heap_v1.no_memory")
    // {
    //     PANIC("Out of memory!");
    // }
    // OS_ENDTRY;

    return symbols;
}

module_symbols_t
module_loader_t::symtab_for(const char* name, const char* suffix)
{
    logger::function_scope fs("module_loader_t::symtab_for");

    logger::debug() << "'" << name << "'";
    module_descriptor_t* out_mod;

    if (!module_already_loaded(*d_last_available_address, name, out_mod))
    {
        logger::warning() << "The module " << name << " is not yet loaded, cannot look up symbols." << endl;
        return module_symbols_t::symmap(module_symbols_t::symmap_alloc(PVS(heap)));
    }

    symbol_table_finder_t finder(out_mod->entry);
    module_symbols_t t(finder.all_symbols(suffix));
    return t;
}

module_symbols_t::symmap
module_symbols_t::starting_with(const char* prefix)
{
    symmap out(symmap_alloc(PVS(heap)));
    for (auto e : symtab)
    {
        if (starts_with(e.first, prefix))
        {
            out.insert(std::make_pair(e.first, e.second));
        }
    }
    return out;
}

module_symbols_t::symmap
module_symbols_t::ending_with(const char* suffix)
{
    logger::function_scope fs("module_symbols_t::ending_with");
    logger::debug() << "'" << suffix << "'";
    symmap out(symmap_alloc(PVS(heap)));
    for (auto e : symtab)
    {
        logger::trace() << "Ending with: checking symbol " << e.first << " against " << suffix;
        if (ends_with(e.first, suffix))
        {
            logger::trace() << "Symbol " << e.first << " ends with " << suffix;
            out.insert(std::make_pair(e.first, e.second));
        }
    }
    return out;
}

/**
 * Load data from ELF file into a predefined location, relocate it and return entry point address or
 * a closure location (if closure_name is not null).
 *
 * Maintain a list of module descriptors as a pointer to last descriptor. (single-linked list)
 * Last module descriptor is always at *d_last_available_address minus sizeof(module_descriptor_t).
 */
void* module_loader_t::load_module(const char* name, elf_parser_t& module, const char* closure_name)
{
    module_descriptor_t* out_mod;

    if (module_already_loaded(*d_last_available_address, name, out_mod))
    {
        logger::debug() << "This module has already been loaded, trying to look up closure.";
        if (!closure_name)
        {
            PANIC("UNSUPPORTED");
        }

        symbol_table_finder_t finder(out_mod->entry.load_base, reinterpret_cast<elf32::section_header_t*>(out_mod->entry.symtab_start), reinterpret_cast<elf32::section_header_t*>(out_mod->entry.strtab_start));

        address_t symbol = finder.find_symbol(closure_name);
        address_t entry = reinterpret_cast<address_t>(*(void**)(symbol));
        logger::debug() << "Returning closure symbol " << symbol << ", pointer " << entry;
        return reinterpret_cast<void*>(symbol);
    }

    module_descriptor_t* previous_loaded_module = reinterpret_cast<module_descriptor_t*>(*d_last_available_address - sizeof(module_descriptor_t));
    module_descriptor_t this_loaded_module;

    size_t size = 0;
    address_t start = ~0;
    elf32::section_header_t* symbol_table = 0;

    // Load either program OR sections, prefer program (faster loading ideally).
    logger::debug() << "program headers: " << module.program_header_count() << endl
             << "section headers: " << module.section_header_count();

    address_t section_base = page_align_up(*d_last_available_address);
    if (*d_first_used_address == 0)
        *d_first_used_address = section_base;

    logger::debug() << __FUNCTION__ << ": loading module at " << section_base;

    this_loaded_module.entry.load_base = section_base;

/*    if (module.program_header_count() > 0)
    {
        // Load using program headers (usually simpler and faster).
        std::for_each(module.program_headers_begin(), module.program_headers_end(),
            [this, &module, &start, &size]
            (elf32::program_header_t& ph)
            {
                if (ph.type == PT_LOAD)
                {
                    if (ph.vaddr < start)
                        start = ph.vaddr;
                    size += ph.vaddr + ph.memsz;
                }
            }
        );
        size -= start;

        // Allocate space for this ELF file sections.
        *last_available_address += size;
        //TODO: section alignment constraints! Page-aligning atm.
        *last_available_address = page_align_up(*last_available_address);

        std::for_each(module.program_headers_begin(), module.program_headers_end(),
            [this, &module, &start, &size, &section_base]
            (elf32::program_header_t& ph)
            {
                if (ph.type == PT_LOAD)
                {
                    ph.vaddr = ph.vaddr + section_base - start;
                    ph.dump();
                    address_t section_addr = ph.vaddr + section_base - start;
                    kconsole << "Allocating this section at " << section_addr << endl;
                    kconsole << "Copying " << ph.filesz << " bytes" << endl;
                    memutils::copy_memory(section_addr, module.start() + ph.offset, ph.filesz);
                    // Zero BSS
                    if (ph.memsz > ph.filesz)
                    {
                        kconsole << "Clearing " << ph.memsz - ph.filesz << " bytes" << endl;
                        memutils::fill_memory((void*)(section_addr + ph.filesz), 0, ph.memsz - ph.filesz);
                    }
                }
            }
        );
    }
    else*/ if (module.section_header_count() > 0)
    {
        logger::debug() << "Loading module \"" << name << "\" with " << int(module.section_header_count()) << " section headers.";

        // Calculate section offsets and sizes.
        start = 0;
        size_t section_offset = 0;
        std::for_each(module.section_headers_begin(), module.section_headers_end(),
            [this, &module, &start, &size, &section_base, &section_offset]
            (elf32::section_header_t& sh)
        {
            if (sh.flags & SHF_ALLOC)
            {
                if (sh.vaddr == 0)
                    sh.vaddr = section_base + section_offset;
                else
                {
                    // Sometimes relocatable section vaddr is non-zero and I'm utterly confused as to what this
                    // could mean, didn't find any reasonable explanation on the internets, might need to look
                    // into gcc/binutils ELF generation code.
                    // For now we just move section_offset by this value, pretending that we reserved this space.
                    section_offset += sh.vaddr;
                    sh.vaddr = section_base + section_offset;
                }
                // Align section to its alignment constraint
                if (sh.addralign > 1)
                {
                    size_t align = align_bytes(sh.vaddr, sh.addralign);
                    sh.vaddr += align;
                    section_offset += align;
                }
                section_offset += sh.size;
            }
        });

        // Allocate space for this ELF file sections.
        *d_last_available_address += section_offset;

        // Actually "load" the sections.
        std::for_each(module.section_headers_begin(), module.section_headers_end(),
            [this, name, &module]
            (elf32::section_header_t& sh)
        {
            if (sh.flags & SHF_ALLOC)
            {
                if (sh.type == SHT_NOBITS)
                {
                    logger::trace() << "Clearing " << int(sh.size) << " bytes at " << sh.vaddr;
                    memutils::clear_memory((void*)sh.vaddr, sh.size);
                }
                else
                {
                    logger::trace() << "Copying " << int(sh.size) << " bytes from " << (module.start() + sh.offset) << " to " << sh.vaddr;
                    memutils::copy_memory(sh.vaddr, module.start() + sh.offset, sh.size);
                }
                // Adjust module end address.
                if (sh.vaddr + sh.size > *d_last_available_address)
                    *d_last_available_address = sh.vaddr + sh.size;
            }
        });

        // "Load" support sections - symtab and strtab.
        symbol_table = module.section_symbol_table();
        elf32::section_header_t* string_table = module.section_string_table();
        if (symbol_table || string_table)
        {
            // Metadata should be separated from the module code. Only kernel module loader has access to this information.
            *d_last_available_address = page_align_up(*d_last_available_address);
        }
        if (symbol_table && (symbol_table->size > 0))
        {
            this_loaded_module.entry.symtab_start = *d_last_available_address;
            memutils::copy_memory(*d_last_available_address, address_t(symbol_table), sizeof(*symbol_table));
            elf32::section_header_t* new_symtab = reinterpret_cast<elf32::section_header_t*>(*d_last_available_address);
            *d_last_available_address += sizeof(*symbol_table);
            memutils::copy_memory(*d_last_available_address, module.start() + symbol_table->offset, symbol_table->size);
            logger::debug() << "### symbol table copied to " << *d_last_available_address;
            // TODO: new_symtab uses offset field as an absolute address in memory where the section starts.
            // for now simply patch a new offset into the old section header!!
            new_symtab->offset = *d_last_available_address - this_loaded_module.entry.load_base;

            // FIXME - we patch the source symbol table here because of the clumsy way we do loading
            symbol_table->offset = *d_last_available_address - module.start();
            // It will now point to our new just loaded symbol table.
            // END FIXME

            *d_last_available_address += symbol_table->size;
            // Switch to new symbol table.
            symbol_table = new_symtab;
        }
        if (string_table)
        {
            this_loaded_module.entry.strtab_start = *d_last_available_address;
            memutils::copy_memory(*d_last_available_address, address_t(string_table), sizeof(*string_table));
            elf32::section_header_t* new_strtab = reinterpret_cast<elf32::section_header_t*>(*d_last_available_address);
            *d_last_available_address += sizeof(*string_table);
            memutils::copy_memory(*d_last_available_address, module.start() + string_table->offset, string_table->size);
            logger::debug() << "### string table copied to " << *d_last_available_address;
            new_strtab->offset = *d_last_available_address - this_loaded_module.entry.load_base;

            // FIXME - we patch the source string table here because of the clumsy way we do loading
            string_table->offset = *d_last_available_address - module.start();
            // It will now point to our new just loaded string table.
            // END FIXME

            *d_last_available_address += string_table->size;
        }
    }
    else
        PANIC("Do not know how to load ELF file!");

    // Update symbols values.
    // This code patches our loaded copy of the symbol table, not the original!
    if (symbol_table)
    {
        for (size_t i = 0; i < module.symbol_entries_count(); i++)
        {
            elf32::symbol_t* symbol = reinterpret_cast<elf32::symbol_t*>(this_loaded_module.entry.load_base + symbol_table->offset + i * symbol_table->entsize);
            if (ELF32_ST_TYPE(symbol->info) < STT_SECTION)
            {
                // cstring_t symname(module.string_table() + symbol->name);
                // if (closure_name && (symname == closure_name)) {
                //     kconsole << "Entry symbol " << (module.string_table() + symbol->name) << " at " << symbol->value << " (before)" << endl; 
                // }

                logger::trace() << "symbol '" << (module.string_table() + symbol->name) << "' old value " << symbol->value << ", new value " << symbol->value + module.section_header(symbol->shndx)->vaddr;
                symbol->value += module.section_header(symbol->shndx)->vaddr;

                // if (closure_name && (symname == closure_name)) {
                //     kconsole << "Entry symbol " << (module.string_table() + symbol->name) << " at " << symbol->value << " (after)" << endl;
                // }
            }
        }
    }

    // Relocate loaded data.
    module.relocate_to(section_base);//, symbol_table);
    
    // Prepare new module descriptor and put it into right memory location.
    address_t prev_address = *d_last_available_address;
    *d_last_available_address = page_align_up(*d_last_available_address);
    size_t free_space = *d_last_available_address - prev_address;
    if (free_space < sizeof(this_loaded_module))
    {
        *d_last_available_address = page_align_up((*d_last_available_address)+1);
    }
    free_space = *d_last_available_address - prev_address;
    ASSERT(free_space >= sizeof(this_loaded_module));
    memutils::copy_string(this_loaded_module.name, name, sizeof(this_loaded_module.name));
    this_loaded_module.previous = (previous_loaded_module->magic == four_cc<'M','D','U','L'>::value) ? previous_loaded_module : 0;
    this_loaded_module.entry.entry_point = module.get_entry_point() + section_base - start; // make absolute address of entry point

    this_loaded_module.entry.loaded_size = *d_last_available_address - this_loaded_module.entry.load_base; // full size including metadata and descriptor
    // we also need another size - code, data and bss without metadata and descriptor, for separating them in the userspace context.

    memutils::copy_memory(*d_last_available_address - sizeof(this_loaded_module), address_t(&this_loaded_module), sizeof(this_loaded_module));
    logger::debug() << "### writing module descriptor to " << *d_last_available_address - sizeof(this_loaded_module);

    // D(print_module_map());

    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;
    bi->use_memory(this_loaded_module.entry.load_base, this_loaded_module.entry.loaded_size, multiboot_t::mmap_entry_t::loaded_module);

    if (!closure_name)
    {
        address_t entry = this_loaded_module.entry.entry_point;
		logger::debug() << "Entry " << entry << ", section_base " << section_base << ", start " << start << ", next mod start " << *d_last_available_address;
        return (void*)(entry);
    }
    else
    {
        // Symbol is a pointer to closure structure.
        symbol_table_finder_t finder(this_loaded_module.entry);

        address_t symbol = finder.find_symbol(closure_name);
        address_t entry = reinterpret_cast<address_t>(*(void**)(symbol));
        logger::debug() << "Returning closure symbol " << symbol << ", pointer " << entry;
        return reinterpret_cast<void*>(symbol);//entry);
    }
}
