//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "bootinfo.h"
#include "new.h"
#include "algorithm"
#include "default_console.h"
#include "memory_v1_interface.h"

//======================================================================================================================
// internal structures
//======================================================================================================================

/*!
 * boot info page layout
 * -------------------- START of page
 * 4 bytes magic (0xbeefdea1)
 * 4 bytes offset of first free byte
 * then a list of bootrec_t subtypes
 * (free space)
 * -------------------- END of page
 *
 * bootrec subtypes:
 * size is the size of the entire record, including bootrec_t header (so, cur + size will equal start of next record).
 *
 */

enum bootrec_tag_e
{
    bootrec_module = 1,      // loadable module info
    bootrec_memory_map,      // memory map info
    bootrec_virtual_mapping, // initial virtual-to-physical mapping
    bootrec_command_line,    // command line info
    bootrec_device_tree,
    end
};

class bootrec_t
{
public:
    uint16_t tag;
    uint16_t size;
};

class bootrec_module_t : public bootrec_t
{
public:
    uint32_t number; // original module number
    uint64_t start;
    uint64_t end;
    char*    name;
};

// Each memmap entry is separate.
class bootrec_mmap_entry_t : public bootrec_t
{
public:
    uint64_t start;
    uint64_t length;
    uint32_t type;
};

// Initial virtual mappings.
class bootrec_vmap_entry_t : public bootrec_t
{
public:
    memory_v1_mapping mapping;  
};

class bootrec_cmdline_t : public bootrec_t
{
public:
    char* cmdline;
};

union bootrec_info_t
{
    bootrec_t*            rec;
    bootrec_module_t*     module;
    bootrec_mmap_entry_t* memmap;
    bootrec_vmap_entry_t* vmemmap;
    bootrec_cmdline_t*    cmdline;
    char*                 generic;
};

//======================================================================================================================
// mmap_iterator
//======================================================================================================================

bootinfo_t::mmap_iterator::mmap_iterator(void* entry, void* end)
    : end(end)
{
    set(entry);
}

void bootinfo_t::mmap_iterator::set(void* entry)
{
    ptr = entry;
    if (ptr)
    {
        bootrec_mmap_entry_t* e = reinterpret_cast<bootrec_mmap_entry_t*>(entry);
        start = e->start;
        size = e->length;
        type = e->type;
    }
}

multiboot_t::mmap_entry_t* bootinfo_t::mmap_iterator::operator *()
{
    return static_cast<multiboot_t::mmap_entry_t*>(ptr);
/*    multiboot_t::mmap_entry_t entry;
    entry.set_region(start, size, (multiboot_t::mmap_entry_t::entry_type_e)type);
    return entry;*/
}

void bootinfo_t::mmap_iterator::operator ++(int)
{
	operator++();
}

void bootinfo_t::mmap_iterator::operator ++()
{
    bootrec_info_t info;
    info.generic = reinterpret_cast<char*>(ptr);
    info.generic += info.rec->size; // move ahead one entry
    while (info.generic < end)
    {
        if (info.rec->tag == bootrec_memory_map)
        {
            set(info.generic);
            return;
        }
        info.generic += info.rec->size;
    }
    set(0);
    end = 0; // iterator has exhausted!
    return;
}

//======================================================================================================================
// vmap_iterator
//======================================================================================================================

bootinfo_t::vmap_iterator::vmap_iterator(void* entry, void* end)
    : end(end)
{
    set(entry);
}

void bootinfo_t::vmap_iterator::set(void* entry)
{
    ptr = entry;
}

memory_v1_mapping* bootinfo_t::vmap_iterator::operator *()
{
    if (!ptr)
        return 0;

    return &(reinterpret_cast<bootrec_vmap_entry_t*>(ptr)->mapping);
}

void bootinfo_t::vmap_iterator::operator ++(int)
{
	operator++();
}

void bootinfo_t::vmap_iterator::operator ++()
{
    bootrec_info_t info;
    info.generic = reinterpret_cast<char*>(ptr);
    info.generic += info.rec->size; // move ahead one entry
    while (info.generic < end)
    {
        if (info.rec->tag == bootrec_virtual_mapping)
        {
            set(info.generic);
            return;
        }
        info.generic += info.rec->size;
    }
    set(0);
    end = 0; // iterator has exhausted!
    return;
}

//======================================================================================================================
// module_iterator
//======================================================================================================================

// TODO: IMPLEMENT!

//======================================================================================================================
// bootinfo_t
//======================================================================================================================

/*!
 * FIXME: Fairly arbitrary location chosen to not mess around with memory maps atm.
 */
static const int MODULE_LOAD_START = 4*MiB;

bootinfo_t::bootinfo_t(bool create_new)
{
    if (create_new)
    {
        magic = BI_MAGIC;
        free = reinterpret_cast<char*>(this + 1);
        last_available_module_address = MODULE_LOAD_START;
    }
}

module_loader_t bootinfo_t::get_module_loader()
{
    return module_loader_t(this, &last_available_module_address);
}

address_t bootinfo_t::used_modules_memory(size_t* size)
{
    *size = last_available_module_address - MODULE_LOAD_START;
    return MODULE_LOAD_START;
}

bool bootinfo_t::get_module(uint32_t number, address_t& start, address_t& end, const char*& name)
{
    bootrec_info_t info;
    info.generic = reinterpret_cast<char*>(this + 1);
    while (info.generic < free)
    {
        if (info.rec->tag == bootrec_module)
        {
            if (info.module->number == number)
            {
                start = info.module->start;
                end = info.module->end;
                name = info.module->name;
                return true;
            }
        }
        info.generic += info.rec->size;
    }
    return false;
}

bool bootinfo_t::get_cmdline(const char*& cmdline)
{
    bootrec_info_t info;
    info.generic = reinterpret_cast<char*>(this + 1);
    while (info.generic < free)
    {
        if (info.rec->tag == bootrec_command_line)
        {
            cmdline = info.cmdline->cmdline;
            return true;
        }
        info.generic += info.rec->size;
    }
    return false;
}

bootinfo_t::mmap_iterator bootinfo_t::mmap_begin()
{
    bootrec_info_t info;
    info.generic = reinterpret_cast<char*>(this + 1);
    while (info.generic < free)
    {
        if (info.rec->tag == bootrec_memory_map)
        {
            return mmap_iterator(info.memmap, free);
        }
        info.generic += info.rec->size;
    }
    return mmap_end();
}

bootinfo_t::mmap_iterator bootinfo_t::mmap_end()
{
    return mmap_iterator(0, 0);
}

bootinfo_t::vmap_iterator bootinfo_t::vmap_begin()
{
    bootrec_info_t info;
    info.generic = reinterpret_cast<char*>(this + 1);
    while (info.generic < free)
    {
        if (info.rec->tag == bootrec_virtual_mapping)
        {
            return vmap_iterator(info.vmemmap, free);
        }
        info.generic += info.rec->size;
    }
    return vmap_end();
}

bootinfo_t::vmap_iterator bootinfo_t::vmap_end()
{
    return vmap_iterator(0, 0);
}

bool bootinfo_t::append_module(uint32_t number, multiboot_t::modinfo_t* mod)
{
    if (!mod)
        return false;

    size_t size = sizeof(bootrec_module_t) + memutils::string_length(mod->str) + 1;

    if (will_overflow(size))
        return false;

    bootrec_module_t* bootmod = new(free) bootrec_module_t;
    bootmod->tag = bootrec_module;
    bootmod->size = size;

    bootmod->number = number;
    bootmod->start = mod->mod_start;
    bootmod->end = mod->mod_end;
    char* name = free + sizeof(bootrec_module_t);
    bootmod->name = name;

    memutils::copy_string(name, mod->str);

    free += size;
    return true;
}

bool bootinfo_t::append_mmap(multiboot_t::mmap_entry_t* entry)
{
    if (!entry)
        return false;

    size_t size = sizeof(bootrec_mmap_entry_t);

    if (will_overflow(size))
        return false;

    bootrec_mmap_entry_t* bootmmap = new(free) bootrec_mmap_entry_t;
    bootmmap->tag = bootrec_memory_map;
    bootmmap->size = size;

    bootmmap->start = entry->address();
    bootmmap->length = entry->size();
    bootmmap->type = entry->type();

    free += size;
    return true;
}

bool bootinfo_t::append_vmap(address_t vstart, address_t pstart, size_t size)
{
    size_t entry_size = sizeof(bootrec_vmap_entry_t);
    
    if (will_overflow(entry_size))
        return false;
        
    bootrec_vmap_entry_t* vmap = new(free) bootrec_vmap_entry_t;
    vmap->tag = bootrec_virtual_mapping;
    vmap->size = entry_size;

    vmap->mapping.virt = vstart;
    vmap->mapping.phys = pstart;
    vmap->mapping.nframes = size >> FRAME_WIDTH;
    vmap->mapping.frame_width = FRAME_WIDTH;
    vmap->mapping.mode = 0;

    free += entry_size;
    return true;    
}

bool bootinfo_t::append_cmdline(const char* cmdline)
{
    if (!cmdline)
        return false;

    size_t size = sizeof(bootrec_cmdline_t) + memutils::string_length(cmdline) + 1;

    if (will_overflow(size))
        return false;

    bootrec_cmdline_t* bootcmd = new(free) bootrec_cmdline_t;
    bootcmd->tag = bootrec_command_line;
    bootcmd->size = size;

    char* name = free + sizeof(bootrec_cmdline_t);
    bootcmd->cmdline = name;

    memutils::copy_string(name, cmdline);

    free += size;
    return true;
}

address_t bootinfo_t::find_usable_physical_memory_top()
{
    address_t top = 0;
    std::for_each(mmap_begin(), mmap_end(), [&top](const multiboot_t::mmap_entry_t* e)
    {
// temp defines
#define RAM_FREE 1
#define ACPI_RECLAIMABLE 3
        if ((e->type() == RAM_FREE) || (e->type() == ACPI_RECLAIMABLE))
        {
            if (e->start() + e->size() > top)
                top = e->start() + e->size();
        }
	});
	return top;
}
/*
// Build a list of memory ranges from the current mmap.
build_free_memory_ranges(range_list_t& range)
{
    // Add free RAM, subtract used RAM.
    std::for_each(mmap_begin(), mmap_end(), [&first_range, bytes](const multiboot_t::mmap_entry_t* e)
    {
        if (e->type() == multiboot_t::mmap_entry_t::non_free)
        {
            range.subtract(e->address(), e->size());
        }
        else
        {
            range.add(e->address(), e->size());
        }
    });
}
*/
/*!
 * Find usable memory range of given size above the low memory (which means 1Mb mark on x86).
 */
address_t bootinfo_t::find_highmem_range_of_at_least(size_t bytes)
{
	const address_t LOWER_BOUND = 1*MB;
	address_t first_range = ~0;
    std::for_each(mmap_begin(), mmap_end(), [&first_range, bytes](const multiboot_t::mmap_entry_t* e)
    {
			if (e->is_free() && (e->start() > LOWER_BOUND) && (first_range > e->start()) && (e->size() >= bytes))
				first_range = e->start();
			else if ((e->type() == multiboot_t::mmap_entry_t::bootinfo) && (first_range <= e->end()))
				first_range = e->end() + 1;

            kconsole << "memory at " << e->address() << " is " << e->size() << " bytes of type " << e->type() << endl;
    });
	kconsole << __FUNCTION__ << "(" << bytes << ") found first free range at " << first_range << endl;
	return first_range;
}

/*!
 * Find an entry containing given address range (start, start + size).
 *
 * Four variants:
 * - start addresses match,
 * - end addresses match,
 * - both start and end match, which means we need to REMOVE entry,
 * - new entry is entirely within the old entry, which means 3-way split,
 * - other possibilities (overlapping ranges) are not supported.
 *
 * @returns n_way == -1 if no entry is found.
 * @returns n_way == 0 if entry needs to be removed.
 * ?? @returns n_way == 1 if entry is split in two parts, beginning is used
 * @returns n_way == 2 if entry is split in two parts, end is used
 * @returns n_way == 3 if entry is split in three parts
 * FIXME: magic numbers.
 */
multiboot_t::mmap_entry_t* bootinfo_t::find_matching_entry(address_t start, size_t size, int& n_way)
{
    multiboot_t::mmap_entry_t* ret = 0;
    n_way = -1;
    std::for_each(mmap_begin(), mmap_end(), [&ret,&n_way,start,size](const multiboot_t::mmap_entry_t* e)
    {
		if (start >= e->start() && e->size() >= size + (start - e->start()))
		{
		    if (start == e->start() && size == e->size())
                n_way = 0;
            else if (start == e->start())
                n_way = 1;
            else if (start + size == e->end() + 1)
                n_way = 2;
            else
                n_way = 3;

            ret = const_cast<multiboot_t::mmap_entry_t*>(e);
		}
	});
    return ret;
}

/*!
 * Use memory (start, start + size) from a given range, this will cause a split of found range into 2 or 3 regions, 
 * one or two of which will remain free, and one will be marked occupied. Will perform various consistency checks 
 * and add new regions to bootinfo_t memory map.
 *
 * @returns false if memory could not be used.
 * @returns true if memory map is updated successfully.
 */
// TODO: also add used memory to VMAP?
// FIXME: use_memory should not augment original entries, should just put used regions on top as an overlay
// this would help frames mod initialisation to build proper physical frame regions.
bool bootinfo_t::use_memory(address_t start, size_t size)
{
	multiboot_t::mmap_entry_t temp_entry;
	multiboot_t::mmap_entry_t* orig_entry;
    int n_way;

    orig_entry = find_matching_entry(start, size, n_way);
    if (!orig_entry || !orig_entry->is_free())
        return false;

    temp_entry.set_region(start, size, multiboot_t::mmap_entry_t::non_free);
    if (!append_mmap(&temp_entry))
        return false;

	return true;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
