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

static void print_phent(int i, program_header *ph)
{
    kconsole << "== section " << i << " ==" << endl <<
                "type   " << ph->type << endl <<
                "offset " << ph->offset << endl <<
                "vaddr  " << ph->vaddr << endl <<
                "paddr  " << ph->paddr << endl <<
                "filesz " << ph->filesz << endl <<
                "memsz  " << ph->memsz << endl <<
                "flags  [" << (ph->flags & PF_R ? "R" : "-") << (ph->flags & PF_W ? "W" : "-") << (ph->flags & PF_X ? "X" : "-")<< "]" << endl <<
                "align  " << ph->align << endl;
}

/*!
 * Load ELF program image, allocate pages and frames from memory.
 * Set up pagedir and copy from @p start to actual image start.
 */
bool elf_parser::load_image(address_t start, size_t size)
{
    header* h = reinterpret_cast<header*>(start);

    if (h->magic != ELF_MAGIC)
        return false;
    if (h->elfclass != ELF_CLASS_32)
        return false;
    if (h->data != ELF_DATA_2LSB)
        return false;
    if (h->type != ET_EXEC)
        return false;
    if (h->machine != EM_386)
        return false;
    if (h->version != EV_CURRENT)
        return false;

    header_ = h; // FIXME this address is out of pagedir mapping

    for (int i = 0; i < h->phnum; i++)
    {
        program_header* ph = reinterpret_cast<program_header*>(start + h->phoff + h->phentsize * i);

        if (ph->type != PT_LOAD)
            continue;

        size_t npages = page_align_up<size_t>(ph->memsz) / PAGE_SIZE;

        print_phent(i, ph);
        kconsole << "Allocating " << npages << " pages (including bss)" << endl;

        size_t remain_to_copy = ph->filesz;
        size_t remain_to_zero = page_align_up<size_t>(ph->memsz) - ph->filesz;
        address_t vaddr = ph->vaddr;
        address_t copy_from = start + ph->offset;
        for (size_t p = 0; p < npages; p++)
        {
            address_t paddr = pmm_alloc_page(vaddr);
            size_t page_offset = 0;
            if (remain_to_copy > 0)
            {
                size_t to_copy = min(remain_to_copy, PAGE_SIZE);
                memutils::copy_memory((void*)paddr, (const void*)copy_from, to_copy);
                remain_to_copy -= to_copy;
                copy_from += to_copy;
                page_offset = to_copy;
            }
            //! Zero bss part of the page, if any.
            if (remain_to_copy == 0 && remain_to_zero > 0 && page_offset < PAGE_SIZE)
            {
                size_t to_zero = min(remain_to_zero, PAGE_SIZE - page_offset);
                paddr += page_offset;
                memutils::fill_memory((void*)paddr, 0, to_zero);
                remain_to_zero -= to_zero;
            }
            vaddr += PAGE_SIZE;
            kconsole << "  " << (p+1) << " " << endl;
        }
    }

    (void)size;
    return true;
}

// @todo use debugging info if present
char* elf_parser::find_symbol(address_t addr, address_t *symbol_start)
{
    address_t max = 0;
    elf32::symbol *fallback_symbol = 0;
    for (unsigned int i = 0; i < symbol_table->size /
            symbol_table->entsize; i++)
    {
        elf32::symbol *symbol = (elf32::symbol *)(symbol_table->addr
                                + i * symbol_table->entsize);

        if ((addr >= symbol->value) &&
            (addr <  symbol->value + symbol->size) )
        {
            char *c = (char *)(symbol->name) + string_table->addr;

            if (symbol_start)
            {
                *symbol_start = symbol->value;
            }
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
        char *c = (char *)(fallback_symbol->name) + string_table->addr;

        if (symbol_start)
        {
            *symbol_start = fallback_symbol->value;
        }
        return c;
    }

    if (symbol_start)
        *symbol_start = 0;
    return NULL;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :