//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
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

/**
* \brief Defines an interface to the multiboot header.
* \ingroup Boot
**/
class multiboot
{
public:
    /**
    * Header flags.
    **/
    enum {
        FLAG_MEM     = 0x0001,
        FLAG_DEVICE  = 0x0002,
        FLAG_CMDLINE = 0x0004,
        FLAG_MODS    = 0x0008,
        FLAG_AOUT    = 0x0010,
        FLAG_ELF     = 0x0020,
        FLAG_MMAP    = 0x0040,
        FLAG_CONFIG  = 0x0080,
        FLAG_LOADER  = 0x0100,
        FLAG_APM     = 0x0200,
        FLAG_VBE     = 0x0400
    };

    /**
    * Boot information passed in by multiboot loader.
    **/
    struct header
    {
        uint32_t flags; // enum above

        // memory here usually excludes occupid memory in mmapinfo
        uint32_t mem_lower; // kilobytes of lower memory
        uint32_t mem_upper; // kilobytes of upper memory

        uint32_t boot_device;

        uint32_t cmdline;

        uint32_t mods_count;
        uint32_t mods_addr;

        /* ELF information */
        uint32_t num;
        uint32_t size;
        uint32_t addr;
        uint32_t shndx;

        struct memmap {
            uint32_t length;
            uint32_t addr;
        } mmap PACKED;

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
        uint32_t size;      ///< size of the mmapinfo entry
        uint64_t base_addr; ///< base address of memory region
        uint64_t length;    ///< size of memory region
        uint32_t type;      ///< type == 1 for free regions, anything else means occupied
    } PACKED;

    multiboot() : header_(NULL) {}
    multiboot(header *h);

    inline uint32_t lower_mem()  const { return header_->mem_lower; }
    inline uint32_t upper_mem()  const { return header_->mem_upper; }
    inline uint32_t flags()      const { return header_->flags; }

    inline bool flags_set(uint32_t flag_mask) const { return (flags() & flag_mask) == flag_mask; }

    inline modinfo* mod(uint32_t i) const
    {
        ASSERT(sizeof(modinfo)==16);
        if (flags_set(FLAG_MODS)
            && header_->mods_count
            && i < header_->mods_count)
        {
            return (modinfo*)(header_->mods_addr + i * sizeof(modinfo));
        }
        return 0;
    }

    inline uint32_t mod_count() const
    {
        if (flags_set(FLAG_MODS))
            return header_->mods_count;
        return 0;
    }

    /**
    * Return highest address occupied by loaded modules.
    *
    * This method assumes that modules are sorted in order of their
    * load address, which might not be the case.
    **/
    inline address_t last_mod_end() const
    {
        modinfo *m = mod(header_->mods_count-1);
        return m ? m->mod_end : 0;
    }

    inline uint32_t elf_num_headers()     const { return header_->num; }
    inline uint32_t elf_header_size()     const { return header_->size; }
    inline uint32_t elf_header_addr()     const { return header_->addr; }
    inline uint32_t elf_strtab_index()    const { return header_->shndx; }
    inline elf32::section_header* symtab_start() const
    {
        return symtab;
    }
    inline address_t symtab_end() const
    {
        if (symtab)
        {
            return (address_t)symtab->sh_addr + symtab->sh_size;
        }
        return 0;
    }
    inline elf32::section_header* strtab_start() const
    {
        return strtab;
    }
    inline address_t strtab_end() const
    {
        if (strtab)
        {
            return (address_t)strtab->sh_addr + strtab->sh_size;
        }
        return 0;
    }

    inline bool is_elf() const { return flags_set(FLAG_ELF); }
    inline bool has_mem_info() const { return flags_set(FLAG_MEM); }

    inline bool has_mmap_info() const { return flags_set(FLAG_MMAP); }
    header::memmap* memory_map() const;

private:
    header*                header_;
    elf32::section_header* strtab;
    elf32::section_header* symtab;
};

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
