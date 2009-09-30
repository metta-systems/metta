#include "bootinfo.h"

bool bootinfo_t::append_mmap_entry(multiboot_t::mmap_entry_t* entry)
{
    const size_t entry_size = sizeof(multiboot_t::mmap_entry_t);
    if (size() + entry_size > PAGE_SIZE - optional_fields_size())
        return false;

    multiboot_t::header_t* header = multiboot_header();
    uint32_t end = boot_info + size();

    memutils::copy_memory(reinterpret_cast<void*>(end), entry, entry_size);
    reinterpret_cast<multiboot_t::mmap_entry_t*>(end)->set_entry_size(entry_size); // ignore any extra fields
    header->mmap.set_size(header->mmap.size() + entry_size);
    increase_size(entry_size);

    return true;
}

kickstart_n::memory_allocator_t* bootinfo_t::memmgr()
{
    if (flags() & 0x1)
        return *reinterpret_cast<kickstart_n::memory_allocator_t**>(boot_info + PAGE_SIZE - sizeof(address_t));
    return 0;
}

bool bootinfo_t::set_memmgr(kickstart_n::memory_allocator_t* mgr)
{
    if (size() > PAGE_SIZE - sizeof(address_t))
        return false;

    *reinterpret_cast<kickstart_n::memory_allocator_t**>(boot_info + PAGE_SIZE - sizeof(address_t)) = mgr;

    flags() |= 0x1;
    return true;
}
