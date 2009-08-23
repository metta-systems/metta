//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "page_allocator.h"

// Page frame allocator gives PHYSICAL memory addresses.

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
class stack_page_frame_allocator_t : public page_frame_allocator_impl_t
{
public:
    stack_page_frame_allocator_t(address_t mem_end, multiboot_t::header_t::memmap_t* mmap);

    virtual void alloc_frame(page* p, bool is_kernel, bool is_writeable);
    virtual void free_frame(page* p);
    virtual address_t alloc_frame();
    virtual void free_frame(address_t frame);

private:
    address_t *stack; // Stack of available page frames
    address_t *top;   // Top of stack, this is where we get new frames from.
};

stack_page_frame_allocator_t::stack_page_frame_allocator_t(address_t mem_end, multiboot_t::header_t::memmap_t* mmap)
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

void stack_page_frame_allocator_t::alloc_frame(page_t* p, bool is_kernel, bool is_writeable)
{
    assert(p);
    assert(!p->frame);

    p->set_present(true);
    p->set_writable(is_writeable);
    p->set_user(!is_kernel);
    p->set_frame(alloc_frame());
}

void stack_page_frame_allocator_t::free_frame(page* p)
{
    assert(p);

    free_frame(p->frame);
    p->setFrame(0);
}

address_t stack_page_frame_allocator_t::alloc_frame()
{
    return *top--;
}

void stack_page_frame_allocator_t::free_frame(address_t frame)
{
    *++top = frame;
}

// (Brendan@osdev.forum)
// 
// The minimum you need is 4 bytes.
// 
// Assume the free page stack is empty, and you've got a "physical address of the next free page on the stack" pointer which is NULL. When someone frees a page they tell you the current linear address of the page and the physical address of the page; and you store the current "physical address of the next free page on the stack" at this linear address, then set the new "physical address of the next free page on the stack" pointer to the physical address of the free page. Then the page is removed from the linear address space. Now you're using 4 bytes for the pointer, plus 4 bytes in the free page (which doesn't really matter because the page is free and wouldn't be used for anything else anyway). You can keep doing this and you still only end up needing 4 bytes (plus the first 4 bytes of each free page, which doesn't really matter).
// 
// When someone allocates a page you tell them the physical address of the first page on the stack, they map it into the address space and tell you where, then you get the physical address of the next page on the stack from this linear address.
// 
// The only real problem with this approach is that you end up with TLB misses.
// 
// There are variations on this that are intended to reduce the TLB misses. One idea is to have a stack of "index pages", where the top "index page" is mapped into the address space and contains the physical addresses of up to 512 or 1024 more free pages. In this case it costs you 4100 or 4104 bytes per free page stack (but in theory you get a lot less TLB misses).
// 
// I use the first method. Partly because it's very likely that a newly allocated page will be accessed soon, and the TLB miss will happen anyway, so there isn't too much point trying to avoid it; and there are other benefits...
// 
// For pages that are above 0x0000000100000000 you'd need to use 8 bytes (64-bit physical addresses). However, it's a good idea to use 2 free page stacks (one for pages below 0x0000000100000000 and one for pages above 0x0000000100000000), because sometimes you need to allocate a page that has a 32-bit physical address (e.g. for a "page directory pointer table" when you're using PAE, or for some 32-bit PCI devices, or for some 64-bit PCI devices that are behind a dodgy "32-bit only" PCI to PCI bridge).
// 
// However, why stop at just 2 free page stacks? For each free page stack you'll probably need a re-entrancy lock, and you don't want all the CPUs (potentially) fighting for the same lock at the same time. If you increase the number of free page stacks for no reason at all, then you'd have more locks and less lock contention, and because of that you'll get better performance on "many CPU" systems (with almost no extra overhead).
// 
// However, if you're going to have lots of free page stacks, why not use them for different things? It's easy to implement page colouring by having one free page stack per page/cache colour, and that will improve performance by improving cache efficiency. It's also possible to have a different group of free page stacks for each NUMA domain (to improve performance by minimizing "distant" RAM accesses), or have different groups of free page stacks for other purposes.
// 
// The previous version of my OS used 1024 free page stacks (but the new version will probably use many more). At 8 or 16 bytes per free page stack I could probably have millions of them without any problem; but at 4104 bytes per free page stack it starts to get expensive... ;)
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// Is there any easy way to read/write physical address?
// Postby torshie on August 16th, 2009, 1:34 pm
// 
// My kernel needs to read/write a given physical address. My kernel runs in 64-bit mode, and does not use identity map. In order to read/write a given physical address, I have to do the following steps:
// 1, modify current page map to map the page my kernel needs to read/write into linear address space. (Normally, the first page in my kernel's linear address space doesn't exist, so I map the page into the first page in linear address space)
// 2, read/write
// 3. remove the first page in linear address space.
// So I need to modify the page map twice in order to read/write a given physical address, which is too inefficient. Could reading/writing physical address be done more efficiently and easily?
// 
// Re: Is there any easy way to read/write physical address?
// Postby Brendan on August 16th, 2009, 2:17 pm
// Hi,
// 
// torshie wrote:My kernel needs to read/write a given physical address. My kernel runs in 64-bit mode, and does not use identity map. In order to read/write a given physical address, I have to do the following steps:
// 1, modify current page map to map the page my kernel needs to read/write into linear address space. (Normally, the first page in my kernel's linear address space doesn't exist, so I map the page into the first page in linear address space)
// 2, read/write
// 3. remove the first page in linear address space.
// So I need to modify the page map twice in order to read/write a given physical address, which is too inefficient. Could reading/writing physical address be done more efficiently and easily?
// 
// 
// For accessing memory mapped I/O areas, you map the areas into the address space and leave them mapped for next time. The BIOS won't help you, so there's no real reason to temporarily map that into an address space. That only really leaves paging structures (page tables, page directories, page directory pointer tables, etc).
// 
// If you do the "self reference" thing (e.g. use a PML4 entry to point to the PML4) then you can access all the paging structures (page tables, page directories, page directory pointer tables, etc) for the current address space.
// 
// Now we're only left with how to access the paging structures that belong to a different address space; so my question is, how often do you need to do this and why?
// 
// 
// 
// Re: Is there any easy way to read/write physical address?
// Postby torshie on August 16th, 2009, 3:40 pm
// Brendan wrote:Hi,
// 
// For accessing memory mapped I/O areas, you map the areas into the address space and leave them mapped for next time. The BIOS won't help you, so there's no real reason to temporarily map that into an address space. That only really leaves paging structures (page tables, page directories, page directory pointer tables, etc).
// 
// If you do the "self reference" thing (e.g. use a PML4 entry to point to the PML4) then you can access all the paging structures (page tables, page directories, page directory pointer tables, etc) for the current address space.
// 
// Now we're only left with how to access the paging structures that belong to a different address space; so my question is, how often do you need to do this and why?
// 
// 
// Cheers,
// 
// Brendan
// 
// 
// Hi,
// The "self reference" trick seems to be OK for PML4, but if applied to other levels of paging structures will make the linear address space non-continuous. How to you solve this problem?
// 
// Cheers,
// torshie
// 
// 
// Re: Is there any easy way to read/write physical address?
// Postby Brendan on August 16th, 2009, 5:19 pm
// Hi,
// 
// torshie wrote:The "self reference" trick seems to be OK for PML4, but if applied to other levels of paging structures will make the linear address space non-continuous. How to you solve this problem?
// 
// 
// Consider something like a page directory. You could put the physical address of the page directory into any page directory entry you like (and create a 1 GiB mapping of all the page directory entries and page tables in that are effected by that page directory), and you can place this 1 GiB mapping anywhere in the address space (on any 1 GiB boundary).
// 
// However, I assume that you're asking this because you're still thinking of doing temporary mappings. There's no real need for temporary mappings though. Think of it like this...
// 
// If you put the physical address of the PML4 into a PLM4 entry (and do this for every address space), then you have a permanent "master map of everything"; which costs no RAM at all. The only real cost is linear space - for example, by using the highest PLM4 entries for the "self reference" you'd end up with a 512 GiB "master map of everything" at the top of the address space (from 0xFFFFFF8000000000 to 0xFFFFFFFFFFFFFFFF). Note: 512 GiB might sound huge, but it's really a small percentage of the entire usable address space (512 GiB out of 262144 GiB).
// 
// Also note that this "self reference" thing is recursive. The same 4 KiB of physical RAM that's used for the PLM4 actually becomes a page directory pointer table, and a page directory, and a page table, and a page. This creates an address space like this:
// 
// CODE: SELECT ALL
// 0xFFFFFFFFFFFFF000 to 0xFFFFFFFFFFFFFFFF - mapping of all page map level 4 entries in the address space
// 0xFFFFFFFFFFE00000 to 0xFFFFFFFFFFFFFFFF - mapping of all page directory pointer table entries in the address space
// 0xFFFFFFFFC0000000 to 0xFFFFFFFFFFFFFFFF - mapping of all page directory entries in the address space
// 0xFFFFFF8000000000 to 0xFFFFFFFFFFFFFFFF - mapping of all page table entries in the address space
// 0xFFFF800000000000 to 0xFFFFFF7FFFFFFFFF - normal "kernel space"
// 0x0000800000000000 to 0xFFFF7FFFFFFFFFFF - unusable (not canonical)
// 0x0000000000000000 to 0x00007FFFFFFFFFFF - "User space"
// 
// 
// One single (permanent) PLM4 entry, and you can access everything (for the current address space) you could ever possibly want to access.
// 
// The only real problem is accessing the paging structures for other address spaces. However, there's many variations of the same trick. You can take any type of paging structure (PLM4, page directory pointer table, page directory or page table) from any address space, and map it as anything else in any other address space; as long as it's at the same or lower level (e.g. you can map a page directory into an address space as if it's a page table, but you can't map a page table into an address space as if it's a page directory).
// 
// However, what you need depends on your OS (more specifically, it depends on how your OS supports "shared memory"). For my OS I don't support shared memory at all. This means I need to be able to change the PLM4 entries in any address space from any other address space, but nothing else. To allow this I have an array of PLM4s (e.g. for N address spaces I've got N pages containing PLM4 data mapped into kernel space).
// 
// 
// 
// Re: Is there any easy way to read/write physical address?
// Postby torshie on August 17th, 2009, 3:08 pm
// Hi Brendan,
// I finally get your "self reference" trick. I just didn't get the fact that I could forge different linear addresses to read/write all four levels of page map structures with only one PML4 "self reference" entry. I thought I need a PML4 entry to read/write PML4 structure, a PDP entry to read/write PDP structure, etc. Of course, I was completely wrong.
// This is really a greeeeeeeeeaaaaaaaaaaaaat trick.
// Thank you very much.
// 
// 
// 
// Re: Is there any easy way to read/write physical address?
// Postby Brendan on August 17th, 2009, 5:18 pm
// Hi,
// 
// torshie wrote:I finally get your "self reference" trick.
// 
// 
// Hmm - it's not really "my" trick. I first found out about it about 15 years ago, but I wouldn't be surprised if Intel planned the paging structures to allow this sort of thing back when they designed the 80386.
// 
// I should also provide some warnings though.
// 
// The first thing to be careful of is TLB invalidations. Basically, if you change a page directory entry, page directory pointer table entry or PML4 entry, then you need to invalidate anything in the area you've changed (like you normally would/should) but you *also* need to invalidate the area in the "master map of everything". Of course in most of these cases it's usually faster to flush the entire TLB anyway. For example, if you change a page directory entry then you'd need to do "INVLPG" up to 512 times in the address space plus once in the mapping (or flush the entire TLB), and if you change a page directory pointer table entry then you'd need to do "INVLPG" up to 262144 times in the address space plus 512 times in the mapping (or flush the entire TLB).
// 
// The next thing to consider is the "accessed" flags. For something like a page directory entry there's one "accessed" flag in RAM, plus up to 2 copies of that "accessed" flag in TLB entries (a normal TLB entry plus a TLB entry for the mapping); and it's the operating systems responsibility to ensure that the "accessed" flags don't become out-of-sync, or to ensure that out-of-sync "accessed" flags don't cause problems for the OS (mainly for the code to decide if a page should/shouldn't be sent to swap space). If you only look at the "accessed" flag in page table entries for 4 KiB pages (and in page directory entries for 2 MiB pages) then you should be able to ignore this problem; but if you check the "accessed" flag in page directory entries, page directory pointer table entries or PML4 entries for any reason, then you may need to be careful.
// 
// Finally, for a 2 MiB page the page directory entry has bit 7 set (to indicate that it is a "large" page) and bit 12 contains the highest PAT bit; but when the CPU interprets this as a page table entry (in the "master map of everything") then bit 7 becomes the highest PAT bit and bit 12 becomes part of the physical address of a page. This makes a mess. For example, imagine you've programmed the PAT like this:
// 
// CODE: SELECT ALL
// PAT value   Cache Type
// 0           Write-back (default, for compatibility)
// 1           Write-through (default, for compatibility)
// 2           Uncached (default, for compatibility)
// 3           Uncached (default, for compatibility)
// 4           Write-combining (modified by the OS and used by device drivers)
// 5           Write-through (default, for compatibility)
// 6           Uncached (default, for compatibility)
// 7           Uncached (default, for compatibility)
// 
// 
// Also imagine that you map some normal RAM into an address space using a 2 MiB page, and you set the PAT so that the 2 MiB page uses "write-back" caching (as you would for all normal RAM). If the 2 MiB page is at physical address 0x123000000, then you create a page directory entry that contains 0x800000001230108F (or, "not executable, physical address = 0x0000000012300000, large page, PAT = 0 = write-back, read/write, user, present"), and when the CPU interprets this as a page table entry (in the "master map of everything") the CPU reads it as "not executable, physical address = 0x0000000012301000, PAT = 4 = write-combining, read/write, user, present". Assuming that your kernel only uses the "master map of everything" to access paging structures, then it shouldn't access this messed up page anyway, and it should be OK (but if your kernel accidentally does write to the messed up page then good luck trying to debug what happened).



// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
