#pragma once
#ifndef __INCLUDED_MULTIBOOT_H
#define __INCLUDED_MULTIBOOT_H

#include "Types.h"
#include "ELF.h"

// #define MULTIBOOT_MAGIC   0x2BADB002

#define MULTIBOOT_FLAG_MEM     0x001
#define MULTIBOOT_FLAG_DEVICE  0x002
#define MULTIBOOT_FLAG_CMDLINE 0x004
#define MULTIBOOT_FLAG_MODS    0x008
#define MULTIBOOT_FLAG_AOUT    0x010
#define MULTIBOOT_FLAG_ELF     0x020
#define MULTIBOOT_FLAG_MMAP    0x040
#define MULTIBOOT_FLAG_CONFIG  0x080
#define MULTIBOOT_FLAG_LOADER  0x100
#define MULTIBOOT_FLAG_APM     0x200
#define MULTIBOOT_FLAG_VBE     0x400

/**
 * Boot information passed in by multiboot loader.
 */
struct MultibootHeader
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
class Multiboot
{
	public:
		Multiboot() : header(NULL) {}
		Multiboot(MultibootHeader *h);

		~Multiboot();

		inline uint32_t lowerMem()      { return header->mem_lower; }
		inline uint32_t upperMem()      { return header->mem_upper; }
		inline uint32_t headerFlags()   { return header->flags; }

		inline uint32_t modStart()
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
		inline uint32_t modEnd()
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

		inline uint32_t elfNumHeaders()      { return header->num; }
		inline uint32_t elfHeaderSize()      { return header->size; }
		inline uint32_t elfHeaderAddr()      { return header->addr; }
		inline uint32_t elfStrtabIndex()     { return header->shndx; }
		inline Address strtabEnd()
		{
			if (strtab)
			{
				return (Address)strtab->sh_addr + strtab->sh_size;
			}
			else
			{
				return 0;
			}
		}
		inline Address symtabEnd()
		{
			if (symtab)
			{
				return (Address)symtab->sh_addr + symtab->sh_size;
			}
			else
			{
				return 0;
			}
		}
		inline Elf32SectionHeader *symtabStart()
		{
			if (symtab)
			{
				return (Elf32SectionHeader *)symtab;
			}
			else
			{
				return 0;
			}
		}
		inline Elf32SectionHeader *strtabStart()
		{
			if (strtab)
			{
				return (Elf32SectionHeader *)strtab;
			}
			else
			{
				return 0;
			}
		}

		bool isElf() { return header->flags & MULTIBOOT_FLAG_ELF; }
		bool hasMemInfo() { return header->flags & MULTIBOOT_FLAG_MEM; }

	private:
		MultibootHeader *header;
		Elf32SectionHeader *strtab;
		Elf32SectionHeader *symtab;
};

#endif
