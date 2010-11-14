#include "module_loader.h"
#include "default_console.h"
#include "memory.h"
#include "stl/algorithm"

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

    kconsole << "Program header" << endl
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

void elf32::section_header_t::dump()
{
    const char* flag_names[32] = {
        "SHF_WRITE",
        "SHF_ALLOC",
        "SHF_EXECINSTR",
        "bit 3",
        "SHF_MERGE",
        "SHF_STRINGS",
        "SHF_LINK_ORDER",
        "bit 7",
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

    kconsole << "Section header" << endl
    << "Name " << name << ", type " << type << ", flags " << flags << endl
    << "VAddr " << addr << ", FAddr " << offset << ", Size " << size << ", Align " << addralign << endl;
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
    address_t start = 0;

    // Load either program OR sections, prefer program (faster loading ideally).
    kconsole << "program headers: " << module.program_header_count() << endl
             << "section headers: " << module.section_header_count() << endl;

    std::for_each(module.program_headers_begin(), module.program_headers_end(), [this, &module, &start](elf32::program_header_t ph)
    {
        ph.dump();
        if (ph.type == PT_LOAD)
        {
            address_t section_base = *last_available_address;
            if (!start)
                start = section_base;
            kconsole << "Allocating this section at " << section_base << endl;
            *last_available_address += ph.memsz;
            //TODO: section alignment constraints! Page-aligning atm.
            *last_available_address = page_align_up(*last_available_address);
            kconsole << "Copying " << ph.filesz << " bytes" << endl;
            memutils::copy_memory(section_base, module.start() + ph.offset, ph.filesz);
            // Zero BSS
            if (ph.memsz > ph.filesz)
            {
                kconsole << "Clearing " << ph.memsz - ph.filesz << " bytes" << endl;
                memutils::fill_memory((void*)(section_base + ph.filesz), 0, ph.memsz - ph.filesz);
            }
        }
    });
    // Find minimum section offset adjustment
    size_t base_offset = 0;
    std::for_each(module.section_headers_begin(), module.section_headers_end(),
        [this, &module, &base_offset]
        (elf32::section_header_t sh)
        {
            sh.dump();
            if ((sh.offset > 0) && (!base_offset || sh.offset < base_offset))
            {
                kconsole << "found minimal base_offset = " << sh.offset << endl;
                base_offset = sh.offset;
            }
        }
    );

    elf32::section_header_t* text = module.section_header(".text");
    address_t entry = module.get_entry_point();
    ptrdiff_t offset = start - text->addr + text->offset - base_offset;

    kconsole << "base_offset = " << base_offset << endl;
    module.relocate_to(start, base_offset);

//     if (ret)
    {
        // Enter module into the list
        ++num_modules;
        // Symbol is a pointer to closure structure.
        UNUSED(closure_name);
//         loaded_module->initialised = true;
//         return *(void**)(module.find_symbol(closure_name));
    }
    return (void*)(entry + offset);
}
