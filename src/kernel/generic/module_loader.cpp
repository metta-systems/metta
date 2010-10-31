#include "module_loader.h"
#include "default_console.h"
#include "stl/algorithm"

/*!
 * FIXME: Fairly arbitrary location chosen to not mess around with memory maps atm.
 */
static const int MODULE_LOAD_START = 16*MiB;

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

/*!
 * Load modules into last_load_address, allocate ST_ALLOC sections, copy only ST_LOAD sections
 * and relocate the resulting code.
 */
module_loader_t::module_loader_t()
    : modules_start(MODULE_LOAD_START)
    , last_available_address(MODULE_LOAD_START)
{
}

void elf32::program_header_t::dump()
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

    kconsole << "Program header" << endl
    << "Type " << type << ", flags " << flags << ", align " << align << endl
    << "VAddr " << vaddr << ", PAddr " << paddr << ", Memsize " << memsz << ", FAddr " << offset << ", FSize " << filesz  << endl;
    for (int x = 0; x < 32; ++x)
    {
        const char* comma = "";
        if (flags & (1 << x)) { kconsole << comma << flag_names[x]; comma = ", "; }
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
    for (int x = 0; x < 32; ++x)
    {
        const char* comma = "";
        if (flags & (1 << x)) { kconsole << comma << flag_names[x]; comma = ", "; }
    }
    kconsole << endl;
}

void* module_loader_t::load_module(const char* name, elf_loader_t& module, const char* closure_name)
{
    // we miss Program Headers currently, because we know the code loaded from initramfs contains only Section Headers
    // FIXME: add more checks in the future
    if (num_modules >= MAX_MODULES)
    {
        kconsole << "Cannot load more modules, increase MAX_MODULES in module_loader.cpp" << endl;
        return false;
    }
    module_descriptor_t* loaded_module = &modules[num_modules];
    memutils::copy_string(loaded_module->name, name, sizeof(loaded_module->name));
    loaded_module->initialised = false;

    kconsole << "program headers: " << module.program_header_count() << endl
             << "section headers: " << module.section_header_count() << endl;

    std::for_each(module.program_headers_begin(), module.program_headers_end(), [&module](elf32::program_header_t ph)
    {
        ph.dump();
    });

    std::for_each(module.section_headers_begin(), module.section_headers_end(), [this, &module](elf32::section_header_t section)
    {
        kconsole << "Section " << module.strtab_pointer(module.section_shstring_table(), section.name) << endl;
//         section.dump();
        if (section.is_allocatable())
        {
            address_t section_base = last_available_address;
            kconsole << "Allocating this section at " << section_base;
            last_available_address += section.size;
            //TODO: section alignment constraints!

            if (section.size > 0)
            {
                memutils::copy_memory(section_base, module.start() + section.offset, section.size);
//                 set_memory(section_base + section.filesz, 0, section.mem_size - section.file_sz);
            }
            else
            {
//                 set_memory(section_base, 0, section.mem_size);
            }
        }
    });

//     bool ret = module.relocate_to(new_start);
//     bool ret = module.relocate_between(old_start, module_start);
//     if (ret)
    {
        // Zero BSS
//         section_header_t* bss = section_header(".bss");
//         memutils::fill_memory(reinterpret_cast<void*>(load_address + bss->offset), 0, bss->size);

        // Enter module into the list
        ++num_modules;
        // Symbol is a pointer to closure structure.
        UNUSED(closure_name);
//         return *(void**)(module.find_symbol(closure_name));
    }
    return 0;
}
