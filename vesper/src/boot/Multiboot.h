//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "Types.h"
#include "ELF.h"

// #define MULTIBOOT_MAGIC   0x2BADB002

#define MULTIBOOT_FLAG_MEM     0x0001
#define MULTIBOOT_FLAG_DEVICE  0x0002
#define MULTIBOOT_FLAG_CMDLINE 0x0004
#define MULTIBOOT_FLAG_MODS    0x0008
#define MULTIBOOT_FLAG_AOUT    0x0010
#define MULTIBOOT_FLAG_ELF     0x0020
#define MULTIBOOT_FLAG_MMAP    0x0040
#define MULTIBOOT_FLAG_CONFIG  0x0080
#define MULTIBOOT_FLAG_LOADER  0x0100
#define MULTIBOOT_FLAG_APM     0x0200
#define MULTIBOOT_FLAG_VBE     0x0400

/**
 * Boot information passed in by multiboot loader.
 */
struct multiboot_header
{
	uint32_t flags;

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

/**
 * Defines an interface to the multiboot header
 */
class multiboot
{
public:
	multiboot() : header(NULL) {}
	multiboot(multiboot_header *h);

	INLINE uint32_t lower_mem()   { return header->mem_lower; }
	INLINE uint32_t upper_mem()   { return header->mem_upper; }
	INLINE uint32_t flags()       { return header->flags; }

	INLINE address_t mod_start()
	{
		if (header->mods_count)
		{
			return *((uint32_t*)(header->mods_addr));
		}
		else
		{
			return 0;
		}
	}
	INLINE address_t mod_end()
	{
		if (header->mods_count)
		{
			return *(uint32_t*)((header->mods_addr)+4);
		}
		else
		{
			return 0;
		}
	}

	INLINE uint32_t elf_num_headers()      { return header->num; }
	INLINE uint32_t elf_header_size()      { return header->size; }
	INLINE uint32_t elf_header_addr()      { return header->addr; }
	INLINE uint32_t elf_strtab_index()     { return header->shndx; }
	INLINE address_t strtab_end()
	{
		if (strtab)
		{
			return (address_t)strtab->sh_addr + strtab->sh_size;
		}
		else
		{
			return 0;
		}
	}
	INLINE address_t symtab_end()
	{
		if (symtab)
		{
			return (address_t)symtab->sh_addr + symtab->sh_size;
		}
		else
		{
			return 0;
		}
	}
	INLINE elf32_section_header* symtab_start()
	{
		if (symtab)
		{
			return (elf32_section_header*)symtab;
		}
		else
		{
			return 0;
		}
	}
    INLINE elf32_section_header* strtab_start()
	{
		if (strtab)
		{
            return (elf32_section_header*)strtab;
		}
		else
		{
			return 0;
		}
	}

	INLINE bool is_elf() { return header->flags & MULTIBOOT_FLAG_ELF; }
	INLINE bool has_mem_info() { return header->flags & MULTIBOOT_FLAG_MEM; }

private:
	multiboot_header*     header;
    elf32_section_header* strtab;
    elf32_section_header* symtab;
};

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
