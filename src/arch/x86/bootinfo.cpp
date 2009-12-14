#include "bootinfo.h"

bool bootinfo_t::append_mmap_entry(multiboot_t::mmap_entry_t* entry)
{
    const size_t entry_size = sizeof(multiboot_t::mmap_entry_t);
    if (size() + entry_size > PAGE_SIZE - optional_fields_size())
        return false;

    multiboot_t::header_t* header = multiboot_header();
    uint32_t end = uint32_t(this) + size();

    memutils::copy_memory(reinterpret_cast<void*>(end), entry, entry_size);
    reinterpret_cast<multiboot_t::mmap_entry_t*>(end)->set_entry_size(entry_size); // ignore any extra fields
    header->mmap.set_size(header->mmap.size() + entry_size);
    increase_size(entry_size);

    return true;
}
