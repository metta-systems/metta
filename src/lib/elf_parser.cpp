//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "elf_parser.h"
#include "default_console.h"
#include "memutils.h"
#include "memory.h"
#include "minmax.h"
#include "frame.h"
#include "config.h"

using namespace elf32;

elf_parser::elf_parser()
{
    header_             = NULL;
    section_headers     = NULL;
    symbol_table        = NULL;
    string_table        = NULL;
    got_table           = NULL;
    filename            = NULL;
}

#if ELF_LOADER_DEBUG
static void print_phent(int i, program_header *ph)
{
    kconsole <<
        "== section " << i << " ==" << endl <<
        "type   " << ph->type << endl <<
        "offset " << ph->offset << endl <<
        "vaddr  " << ph->vaddr << endl <<
        "paddr  " << ph->paddr << endl <<
        "filesz " << ph->filesz << endl <<
        "memsz  " << ph->memsz << endl <<
        "flags  [" << (ph->flags & PF_R ? "R" : "-") << (ph->flags & PF_W ? "W" : "-") << (ph->flags & PF_X ? "X" : "-")<< "]" << endl <<
        "align  " << ph->align << endl;
}
#endif

/*!
* Load ELF program image, allocate pages and frames from memory.
* Set up pagedir and copy from @p start to actual image start.
*/
bool elf_parser::load_image(address_t start, UNUSED_ARG size_t size/*, kickstart_n::memory_allocator_t* allocator*/)
{
    header* h = reinterpret_cast<header*>(start);

#define ERROR_RETURN_ON(x) \
if (x) { \
    kconsole << RED << #x <<endl; \
    return false; \
}

    ERROR_RETURN_ON(h->magic != ELF_MAGIC)
    ERROR_RETURN_ON(h->elfclass != ELF_CLASS_32)
    ERROR_RETURN_ON(h->data != ELF_DATA_2LSB)
    ERROR_RETURN_ON(h->type != ET_EXEC)
    ERROR_RETURN_ON(h->machine != EM_386)
    ERROR_RETURN_ON(h->version != EV_CURRENT)

    header_ = h;

    for (int i = 0; i < h->phnum; i++)
    {
        program_header* ph = reinterpret_cast<program_header*>(start + h->phoff + h->phentsize * i);

        if (ph->type != PT_LOAD)
            continue;

        size_t npages = page_align_up<size_t>(ph->memsz) / PAGE_SIZE;

#if ELF_LOADER_DEBUG
        print_phent(i, ph);
        kconsole << "Allocating " << npages << " pages (including bss)" << endl;
#endif

        size_t remain_to_copy = ph->filesz;
        size_t remain_to_zero = page_align_up<size_t>(ph->memsz) - ph->filesz;
        address_t vaddr = ph->vaddr;
        address_t copy_from = start + ph->offset;
        for (size_t p = 0; p < npages; p++)
        {
            frame_t* paddr = new(vaddr) frame_t;
            size_t page_offset = 0;
            if (remain_to_copy > 0)
            {
                size_t to_copy = min(remain_to_copy, PAGE_SIZE);
                memutils::copy_memory(paddr, (const void*)copy_from, to_copy);
                remain_to_copy -= to_copy;
                copy_from += to_copy;
                page_offset = to_copy;
            }
            // Zero bss part of the page, if any.
            if (remain_to_copy == 0 && remain_to_zero > 0 && page_offset < PAGE_SIZE)
            {
                size_t to_zero = min(remain_to_zero, PAGE_SIZE - page_offset);
                paddr += page_offset;
                memutils::fill_memory(paddr, 0, to_zero);
                remain_to_zero -= to_zero;
            }
            vaddr += PAGE_SIZE;
#if ELF_LOADER_DEBUG
            kconsole << "  " << (int)(p+1) << " ";
#endif
        }
#if ELF_LOADER_DEBUG
            kconsole << endl;
#endif
    }

    return true;
}

// TODO: use debugging info if present
char* elf_parser::find_symbol(address_t addr, address_t* symbol_start)
{
    address_t max = 0;
    elf32::symbol* fallback_symbol = 0;

    for (unsigned int i = 0; i < symbol_table->size / symbol_table->entsize; i++)
    {
        elf32::symbol* symbol = reinterpret_cast<elf32::symbol*>(symbol_table->addr + i * symbol_table->entsize);

        if ((addr >= symbol->value) && (addr <  symbol->value + symbol->size))
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

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
