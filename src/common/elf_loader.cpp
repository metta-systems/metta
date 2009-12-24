#include "elf_loader.h"

using namespace elf32;

// TODO: use debugging info if present
cstring_t elf_loader_t::find_symbol(address_t addr, address_t* symbol_start)
{
//     section_header_t* debug_table = section_header(".debug_frame");

    address_t max = 0;
    symbol_t* fallback_symbol = 0;

    for (unsigned int i = 0; i < symbol_entries_count(); i++)
    {
        symbol_t* symbol = reinterpret_cast<symbol_t*>(symbol_table->addr + i * symbol_table->entsize);//FIXME: base addr missing?

        if ((addr >= symbol->value) && (addr < symbol->value + symbol->size))
        {
            char* c = reinterpret_cast<char*>(symbol->name) + string_table->addr;

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
        char* c = reinterpret_cast<char*>(fallback_symbol->name) + string_table->addr;

        if (symbol_start)
            *symbol_start = fallback_symbol->value;
        return c;
    }

    if (symbol_start)
        *symbol_start = 0;
    return NULL;
}
