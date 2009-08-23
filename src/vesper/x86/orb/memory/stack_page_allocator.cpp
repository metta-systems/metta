//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "page_allocator.h"

// Page allocator gives PHYSICAL memory addresses.

// unpacker will use page allocatorto grab some pages for kernel and init component.
// some init structures will be passed to init component so that it can
// finish setting up page allocator for in-kernel use (?)

// unsigned int total_count;
// INDEX_PAGE *root, *top;
//
// 4Kb page:
// struct INDEX_PAGE {
//     void *next_index;
//     unsigned int count;
//     void *free_pages[1022];
// }
//
// void *alloc_page(void) {
//     void *freePage;
//     int entry;
//
//     entry = currentIndex->count;
//     currentIndex->count--;
//     if( entry == 0) {
//         freePage = currentIndex;
//         currentIndex = currentIndex->next_index;
//     } else {
//         freePage = currentIndex->free_pages[entry];
//     }
//     return freePage;
// }
//
// The stack of index pages has the same problem - you have to map the current index page into linear memory before you can access it. Because of the way it works you can get 1022 free pages easily, but when an index page becomes a free page you need map the next index page into linear memory.

// Stack-based page allocator implementation.
class stack_page_allocator : public page_allocator_impl
{
public:
    stack_page_allocator(address_t mem_end, multiboot::header::memmap *mmap);

    virtual void alloc_frame(page* p, bool is_kernel, bool is_writeable);
    virtual void free_frame(page* p);
    virtual address_t alloc_frame();
    virtual void free_frame(address_t frame);

private:
    address_t *stack; // Stack of available page frames
    address_t *top;   // Top of stack, this is where we get new frames from.
};

stack_page_allocator::stack_page_allocator(address_t mem_end, multiboot::header::memmap *mmap)
{
    // Make enough frames to reach 0x00000000 .. memEnd.
    // make sure memEnd is on a page boundary.
    uint32_t mem_end_page = page_align_down<address_t>(mem_end);

    // A mmap will give us more precise info about maximum available memory
    if (mmap)
    {
        address_t highest = mem_end_page;

        multiboot::mmapinfo* mmi = (multiboot::mmapinfo*)(mmap->addr);
        multiboot::mmapinfo* end = (multiboot::mmapinfo*)(mmap->addr + mmap->length);
        while (mmi < end)
        {
            address_t end = page_align_up<address_t>(mmi->base_addr + mmi->length);
            if (end > highest)
                highest = end;
            mmi = (multiboot::mmapinfo*)(((char *)mmi) + mmi->size + 4);
        }

        mem_end_page = highest;
    }

    n_frames = mem_end_page / PAGE_SIZE;

    // use mmap if provided to mark unavailable areas
    if (mmap)
    {
        kconsole.print("MMAP is provided @ %p:\n", mmap->addr);
        multiboot::mmapinfo* mmi = (multiboot::mmapinfo*)(mmap->addr);
        multiboot::mmapinfo* end = (multiboot::mmapinfo*)(mmap->addr + mmap->length);
        while (mmi < end)
        {
            kconsole.print("[entry @ %p, %d bytes]  %llx, %llu bytes (type %u = %s)\n", mmi, mmi->size, mmi->base_addr, mmi->length, mmi->type, mmi->type == 1 ? "Free" : "Occupied");
            if (mmi->type != 1) // occupied space
            {
                address_t start = page_align_down<address_t>(mmi->base_addr);
                address_t end = page_align_up<address_t>(mmi->base_addr + mmi->length);
                // mark frames bitmap as occupied
                kconsole.print(" > marking as occupied from %08x to %08x\n", start, end);
                for (i = start; i < end; i += PAGE_SIZE)
                {
                    frames->set(i / PAGE_SIZE);
                }
            }
            mmi = (multiboot::mmapinfo*)(((char *)mmi) + mmi->size + 4);
        }
    }
}

void stack_page_allocator::alloc_frame(page* p, bool is_kernel, bool is_writeable)
{
    assert(p);
    assert(!p->frame);

    p->setPresent(true);
    p->setWritable(is_writeable);
    p->setUser(!is_kernel);
    p->setFrame(alloc_frame());
}

void stack_page_allocator::free_frame(page* p)
{
    assert(p);

    free_frame(p->frame);
    p->setFrame(0);
}

address_t stack_page_allocator::alloc_frame()
{
    return *top--;
}

void stack_page_allocator::free_frame(address_t frame)
{
    *++top = frame;
}

// Recursive PDE/PTE explained by Brendan @ http://forum.osdev.org/viewtopic.php?f=15&t=19387
//
// The area from 0xFFFFF000 to 0xFFFFFFFF becomes a mapping of all page directory entries, and the area from 0xFFC00000 to 0xFFFFFFFF becomes a mapping of all page table entries (if the corresponding page table is present). These areas overlap because the page directory is being used as a page table - the area from 0xFFFFF000 to 0xFFFFFFFF contains the page directory entries and the page table entries for the last page table (which is the same thing).
//
// To access the page directory entry for any virtual address you'd be able to do "mov eax,[0xFFFFF000 + (virtualAddress >> 20) * 4]", and (if the page table is present) to access the page table entry for any virtual address you'd be able to do "mov eax,[0xFFC00000 + (virtualAddress >> 12) * 4]". Most C programmers would use these areas as arrays and let the compiler do some of the work, so they can access any page directory entry as "PDE = pageDirectoryMapping[virtualAddress >> 20]" and any page table entry as "PTE = pageTableMapping[virtualAddress >> 12]".
//
// Now imagine that someone wants to allocate a new page at 0x12345000. You need to check if a page table exists for this area (and if the page table doesn't exist, allocate a new page of RAM to use as a page table, create a page directory entry for it, and INVLPG the mapping for the new page table), then check if the page already exists (and if the page doesn't exist, allocate a new page of RAM, create a page table entry for the new page, and INVLPG the new page).
//
// The same sort of lookups are needed for lots of things - freeing a page, checking accessed/dirty bits, changing page attributes (read, write, execute, etc), page fault handling, etc. Basically, having fast access to page directory entries and page table entries is important, because it speeds up everything your virtual memory management code does.
//
// Without making a page directory entry refer to the page directory, you need to find some other way to access page directory entries and page table entries, and there is no better way (all the other ways are slower, more complicated and waste more RAM).


// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
