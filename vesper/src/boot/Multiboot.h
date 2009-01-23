//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "elf.h"
#include "common.h" // panic_assert()

namespace metta {
namespace kernel {

// #define MULTIBOOT_MAGIC   0x2BADB002

enum {
    MULTIBOOT_FLAG_MEM     = 0x0001,
    MULTIBOOT_FLAG_DEVICE  = 0x0002,
    MULTIBOOT_FLAG_CMDLINE = 0x0004,
    MULTIBOOT_FLAG_MODS    = 0x0008,
    MULTIBOOT_FLAG_AOUT    = 0x0010,
    MULTIBOOT_FLAG_ELF     = 0x0020,
    MULTIBOOT_FLAG_MMAP    = 0x0040,
    MULTIBOOT_FLAG_CONFIG  = 0x0080,
    MULTIBOOT_FLAG_LOADER  = 0x0100,
    MULTIBOOT_FLAG_APM     = 0x0200,
    MULTIBOOT_FLAG_VBE     = 0x0400
};

/**
* \brief Defines an interface to the multiboot header.
* \ingroup Boot
**/
class multiboot
{
public:
    /**
    * Boot information passed in by multiboot loader.
    **/
    struct header
    {
        uint32_t flags; // enum above

        uint32_t mem_lower;
        uint32_t mem_upper;

        uint32_t boot_device;

        uint32_t cmdline;

        uint32_t mods_count;
        uint32_t mods_addr;

        /* ELF information */
        uint32_t num;
        uint32_t size;
        uint32_t addr;
        uint32_t shndx;

        uint32_t mmap_length;
        uint32_t mmap_addr;

        uint32_t drives_length;
        uint32_t drives_addr;

        uint32_t config_table;

        uint32_t boot_loader_name;

        uint32_t apm_table;

        uint32_t vbe_control_info;
        uint32_t vbe_mode_info;
        uint32_t vbe_mode;
        uint32_t vbe_interface_seg;
        uint32_t vbe_interface_off;
        uint32_t vbe_interface_len;
    } PACKED;

    struct modinfo
    {
        uint32_t mod_start;
        uint32_t mod_end;
        const char* str;
        uint32_t reserved;
    } PACKED;

    struct mmapinfo
    {
        uint32_t size;
        uint64_t base_addr;
        uint64_t length;
        uint32_t type;
    } PACKED;

    multiboot() : header_(NULL) {}
    multiboot(header *h);

    inline uint32_t lower_mem()   { return header_->mem_lower; }
    inline uint32_t upper_mem()   { return header_->mem_upper; }
    inline uint32_t flags()       { return header_->flags; }

    inline modinfo* mod(uint32_t i)
    {
        ASSERT(sizeof(modinfo)==16);
        if (header_->flags & MULTIBOOT_FLAG_MODS
            && header_->mods_count
            && i < header_->mods_count)
        {
            return (modinfo*)(header_->mods_addr + i * sizeof(modinfo));
        }
        return 0;
    }

    inline uint32_t mod_count() const
    {
        if (header_->flags & MULTIBOOT_FLAG_MODS)
            return header_->mods_count;
        return 0;
    }

    /**
    * Return highest address occupied by loaded modules.
    *
    * This method assumes that modules are sorted in order of their
    * load address, which might not be the case.
    **/
    inline address_t last_mod_end()
    {
        modinfo *m = mod(header_->mods_count-1);
        return m ? m->mod_end : 0;
    }

    inline uint32_t elf_num_headers()      { return header_->num; }
    inline uint32_t elf_header_size()      { return header_->size; }
    inline uint32_t elf_header_addr()      { return header_->addr; }
    inline uint32_t elf_strtab_index()     { return header_->shndx; }
    inline elf32::section_header* symtab_start()
    {
        return symtab;
    }
    inline address_t symtab_end()
    {
        if (symtab)
        {
            return (address_t)symtab->sh_addr + symtab->sh_size;
        }
        return 0;
    }
    inline elf32::section_header* strtab_start()
    {
        return strtab;
    }
    inline address_t strtab_end()
    {
        if (strtab)
        {
            return (address_t)strtab->sh_addr + strtab->sh_size;
        }
        return 0;
    }

    inline bool is_elf() { return header_->flags & MULTIBOOT_FLAG_ELF; }
    inline bool has_mem_info() { return header_->flags & MULTIBOOT_FLAG_MEM; }

    inline bool has_mmap_info() { return header_->flags & MULTIBOOT_FLAG_MMAP; }
    void print_mmap_info();

private:
    header*                header_;
    elf32::section_header* strtab;
    elf32::section_header* symtab;
};

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
