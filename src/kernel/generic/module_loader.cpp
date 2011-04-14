#include "config.h"
#include "stl/algorithm"
#include "module_loader.h"
#include "default_console.h"
#include "memory.h"
#include "debugger.h"

#if ELF_RELOC_DEBUG_V
#define V(s) s
#else
#define V(s)
#endif

/*!
 * Load modules into last_load_address, allocate ST_ALLOC sections, copy only ST_LOAD sections
 * and relocate the resulting code.
 */

struct module_descriptor_t
{
    char name[32];         // name of the module
    bool initialised;      // true if module has already initialised itself?
    address_t entry_point; // address of initialisation entry point
    address_t code_start;  // start of module code
    size_t    code_size;
    address_t data_start;
    size_t    data_size;
    address_t bss_start;
    size_t    bss_size;
    address_t debug_start; // start of debugging info tables
};

static const int MAX_MODULES = 32;
static module_descriptor_t modules[MAX_MODULES];
static int num_modules = 0;

void elf32::program_header_t::dump()
{
    const char* flag_names[32] = {
        "PF_EXECUTE",
        "PF_WRITE",
        "PF_READ",
        "bit 3",
        "bit 4",
        "bit 5",
        "bit 6",
        "bit 7",
        "bit 8",
        "bit 9",
        "bit 10",
        "bit 11",
        "bit 12",
        "bit 13",
        "bit 14",
        "bit 15",
        "bit 16",
        "bit 17",
        "bit 18",
        "bit 19",
        "bit 20",
        "bit 21",
        "bit 22",
        "bit 23",
        "bit 24",
        "bit 25",
        "bit 26",
        "bit 27",
        "bit 28",
        "bit 29",
        "bit 30",
        "bit 31"
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
        "SHF_WRITE",
        "SHF_ALLOC",
        "SHF_EXECINSTR",
        "bit 3",
        "SHF_MERGE",
        "SHF_STRINGS",
        "SHF_INFO_LINK",
        "SHF_LINK_ORDER",
        "SHF_OS_NONCONFORMING",
        "SHF_GROUP",
        "SHF_TLS",
        "bit 11",
        "bit 12",
        "bit 13",
        "bit 14",
        "bit 15",
        "bit 16",
        "bit 17",
        "bit 18",
        "bit 19",
        "bit 20",
        "bit 21",
        "bit 22",
        "bit 23",
        "bit 24",
        "bit 25",
        "bit 26",
        "bit 27",
        "bit 28",
        "bit 29",
        "bit 30",
        "bit 31"
    };
    // 0 .group        00000008  00000000  00000000  00000034  2**2
    // CONTENTS, READONLY, EXCLUDE, GROUP, LINK_ONCE_DISCARD
    //word_t  link;          /*!< Index of another section */
    //word_t  info;          /*!< Additional section information */
    //word_t  entsize;       /*!< Entry size if section holds table */

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

/*!
 * Load data from ELF file into a predefined location, relocate it and return entry point address or
 * a closure location (if closure_name is not null).
 *
 * TODO: move this code to elf_parser_t, as it's a natural part of ELF loading.
 */
void* module_loader_t::load_module(const char* name, elf_parser_t& module, const char* closure_name)
{
    if (num_modules >= MAX_MODULES)
    {
        kconsole << "Cannot load more modules, increase MAX_MODULES in module_loader.cpp" << endl;
        return 0;
    }
    module_descriptor_t* loaded_module = &modules[num_modules];
    memutils::copy_string(loaded_module->name, name, sizeof(loaded_module->name));
    loaded_module->initialised = false;

//    size_t size = 0;
    address_t start = ~0;

    // Load either program OR sections, prefer program (faster loading ideally).
    V(kconsole << "program headers: " << module.program_header_count() << endl
             << "section headers: " << module.section_header_count() << endl);

    address_t section_base = *last_available_address;

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
        start = 0;
        size_t section_offset = 0;
#ifdef CLANG_HAS_LAMBDAS
        std::for_each(module.section_headers_begin(), module.section_headers_end(),
            [this, &module, &start, &size, &section_base, &section_offset]
            (elf32::section_header_t& sh)
        {
#else
        for (auto shi = module.section_headers_begin(); shi != module.section_headers_end(); ++shi)
        {
				elf32::section_header_t& sh = *shi;
#endif
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
            }
#ifdef CLANG_HAS_LAMBDAS
        );
#endif

        // Allocate space for this ELF file sections.
        *last_available_address += section_offset;
        // Page-align after last section, so that next module could be loaded onto separate page.
        *last_available_address = page_align_up(*last_available_address);

#ifdef CLANG_HAS_LAMBDAS
        std::for_each(module.section_headers_begin(), module.section_headers_end(),
            [this, &module, &start, &size, &section_base]
            (elf32::section_header_t& sh)
        {
#else
        for (auto shi = module.section_headers_begin(); shi != module.section_headers_end(); ++shi)
        {
				auto sh = *shi;
#endif
                if (sh.flags & SHF_ALLOC)
                {
                    if (sh.type == SHT_NOBITS)
                    {
                        V(kconsole << "Clearing " << sh.size << " bytes at " << sh.vaddr << endl);
                        memutils::fill_memory((void*)sh.vaddr, 0, sh.size);
                    }
                    else
                    {
                        V(kconsole << "Copying " << sh.size << " bytes from " << (module.start() + sh.offset) << " to " << sh.vaddr << endl);
                        memutils::copy_memory(sh.vaddr, module.start() + sh.offset, sh.size);
                    }
                }
            }
#ifdef CLANG_HAS_LAMBDAS
        );
#endif
    }
    else
        PANIC("Do not know how to load ELF file!");

    // Update symbols values.
    elf32::section_header_t* symbol_table = module.section_symbol_table();
    if (symbol_table)
    {
        for (unsigned int i = 0; i < module.symbol_entries_count(); i++)
        {
            elf32::symbol_t* symbol = reinterpret_cast<elf32::symbol_t*>(module.start() + symbol_table->offset + i * symbol_table->entsize);
            if (ELF32_ST_TYPE(symbol->info) < STT_SECTION)
            {
                V(kconsole << "symbol '" << (module.string_table() + symbol->name) << "' old value " << symbol->value);
                symbol->value += module.section_header(symbol->shndx)->vaddr;
                V(kconsole << ", new value " << symbol->value << endl);
            }
        }
    }

    // Relocate loaded data.
    module.relocate_to(section_base);

    ++num_modules;

    if (!closure_name)
    {
        address_t entry = module.get_entry_point();
		V(kconsole << "entry " << entry << ", section_base " << section_base << ", start " << start << ", next mod start " << *last_available_address << endl);
        return (void*)(entry + section_base - start);
    }
    else
    {
        // Symbol is a pointer to closure structure.
        return *(void**)(module.find_symbol(closure_name));
    }
}
