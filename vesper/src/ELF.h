//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/*
 *  ELF file format structures definitions
 *  According to Portable Formats Specification, version 1.1 (FIXME: update to 1.2)
 *
 *  Typed in by Stanislav Karchebny <berkus+os@madfire.net>, 2001
 *  This file is in Public Domain.
 */

#pragma once

#include "Types.h"
#include "Macros.h"

/*
 * ELF data types
 */
typedef uint32_t  Elf32_Addr;  /* 4 bytes/4 align/unsigned */
typedef uint16_t  Elf32_Half;  /* 2 bytes/2 align/unsigned */
typedef uint32_t  Elf32_Off;   /* 4 bytes/4 align/unsigned */
typedef  int32_t  Elf32_Sword; /* 4 bytes/4 align/signed   */
typedef uint32_t  Elf32_Word;  /* 4 bytes/4 align/unsigned */
typedef  uint8_t  Elf32_Byte;  /* 1 byte /1 align/unsigned */


/*
 * ELF structures: ELF file Header
 */
struct Elf32Header
{
	Elf32_Word e_magic;
	Elf32_Byte e_class;
	Elf32_Byte e_data;
	Elf32_Byte e_hdrversion;
	Elf32_Byte e_padding[9];
	Elf32_Half e_type;           /**< Identifies object file type */
	Elf32_Half e_machine;        /**< Specifies required architecture */
	Elf32_Word e_version;        /**< Identifies object file version */
	Elf32_Addr e_entry;          /**< Entry point virtual address */
	Elf32_Off  e_phoff;          /**< Program header table file offset */
	Elf32_Off  e_shoff;          /**< Section header table file offset */
	Elf32_Word e_flags;          /**< Processor-specific flags */
	Elf32_Half e_ehsize;         /**< ELF header size in bytes */
	Elf32_Half e_phentsize;      /**< Program header table entry size */
	Elf32_Half e_phnum;          /**< Program header table entry count */
	Elf32_Half e_shentsize;      /**< Section header table entry size */
	Elf32_Half e_shnum;          /**< Section header table entry count */
	Elf32_Half e_shstrndx;       /**< Section header string table index */
} PACKED;

/* Elf32Header.e_magic */
#define ELF_MAGIC  0x464c457f    /* ASCII "ELF",0x7F */

/* Elf32Header.e_class */
#define ELF_CLASS_NONE 0x00      /**< Invalid class  */
#define ELF_CLASS_32   0x01      /**< 32 bit objects */
#define ELF_CLASS_64   0x02      /**< 64 bit objects */

/* Elf32Header.e_data */
#define ELF_DATA_NONE  0x00      /**< Invalid data encoding   */
#define ELF_DATA_2LSB  0x01      /**< LSB (Intel) encoding    */
#define ELF_DATA_2MSB  0x02      /**< MSB (Motorola) encoding */

/* Elf32Header.e_type */
#define ET_NONE    0x0000        /* No type     */
#define ET_REL     0x0001        /* Relocatable */
#define ET_EXEC    0x0002        /* Executable  */
#define ET_DYN     0x0003        /* Shared      */
#define ET_CORE    0x0004        /* Core        */
#define ET_LOPROC  0xff00
#define ET_HIPROC  0xffff

/* Elf32Header.e_machine */
#define EM_NONE  0x0000          /* No machine     */
#define EM_M32   0x0001          /* AT&T WE32100   */
#define EM_SPARC 0x0002          /* SPARC          */
#define EM_386   0x0003          /* x86            */
#define EM_68K   0x0004          /* Motorola 68000 */
#define EM_88K   0x0005          /* Motorola 88000 */
#define EM_860   0x0007          /* Intel 80860    */
#define EM_MIPS  0x0008          /* MIPS RS3000    */

/* Elf32Header.e_version */
#define EV_NONE        0         /* Invalid version */
#define EV_CURRENT     1         /* Current version */


/*
 * ELF structures: Section header
 */
struct Elf32SectionHeader
{
	Elf32_Word sh_name;          /**< Section name, index in string table */
	Elf32_Word sh_type;          /**< Type of section */
	Elf32_Word sh_flags;         /**< Miscellaneous section attributes */
	Elf32_Addr sh_addr;          /**< Section virtual addr at execution */
	Elf32_Off  sh_offset;        /**< Section file offset */
	Elf32_Word sh_size;          /**< Size of section in bytes */
	Elf32_Word sh_link;          /**< Index of another section */
	Elf32_Word sh_info;          /**< Additional section information */
	Elf32_Word sh_addralign;     /**< Section alignment */
	Elf32_Word sh_entsize;       /**< Entry size if section holds table */
} PACKED;

/* predefined section table indices */
#define SHN_UNDEF     0x0000
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC    0xff00
#define SHN_HIPROC    0xff1f
#define SHN_ABS       0xfff1
#define SHN_COMMON    0xfff2
#define SHN_HIRESERVE 0xffff

/* Elf32SectionHeader.sh_type */
#define SHT_NULL      0x00000000
#define SHT_PROGBITS  0x00000001 /* The data is contained in program file */
#define SHT_SYMTAB    0x00000002 /* Symbol table */
#define SHT_STRTAB    0x00000003 /* String table */
#define SHT_RELA      0x00000004
#define SHT_HASH      0x00000005 /* Symbol hash table */
#define SHT_DYNAMIC   0x00000006 /* Dynamic linking information */
#define SHT_NOTE      0x00000007
#define SHT_NOBITS    0x00000008 /* The data is not contained in program file */
#define SHT_REL       0x00000009
#define SHT_SHLIB     0x0000000a /* Reserved with unspecified semantics */
#define SHT_DYNSYM    0x0000000b
#define SHT_LOPROC    0x70000000
#define SHT_HIPROC    0x7fffffff
#define SHT_LOUSER    0x80000000
#define SHT_HIUSER    0xffffffff
/* GNU extensions */
#define SHT_INIT      0x0000000e
#define SHT_FINI      0x0000000f
#define SHT_PREINIT   0x00000010


/* Elf32SectionHeader.sh_flags */
#define SHF_WRITE     0x00000001
#define SHF_ALLOC     0x00000002
#define SHF_EXECINSTR 0x00000004
#define SHF_MASKPROC  0xf0000000


/*
 * ELF structures: Symbol Table
 */
struct Elf32Symbol
{
	Elf32_Word st_name;          /**< Symbol name, index into string table */
	Elf32_Addr st_value;         /**< Symbol value */
	Elf32_Word st_size;          /**< Size occupied by this symbol */
	Elf32_Byte st_info;          /**< Symbol type and binding */
	Elf32_Byte st_other;
	Elf32_Half st_shndx;         /**< Section index this symbol belongs to */
} PACKED;

/* Symbol Table index: first/undefined entry */
#define STN_UNDEF 0x0000

/* Elf32Symbol.st_info manipulation macros */
#define ELF32_ST_BIND(i)    ((i) >> 4)
#define ELF32_ST_TYPE(i)    ((i) & 0xf)
#define ELF32_ST_INFO(b,t)  ((b) << 4 + ((t) & 0xf))

/* ELF32_ST_BIND(Elf32Symbol.st_info) values */
#define STB_LOCAL  0x0
#define STB_GLOBAL 0x1
#define STB_WEAK   0x2
#define STB_LOPROC 0xd
#define STB_HIPROC 0xf

/* ELF32_ST_TYPE(Elf32Symbol.st_info) values */
#define STT_NOTYPE  0x0
#define STT_OBJECT  0x1
#define STT_FUNC    0x2
#define STT_SECTION 0x3
#define STT_FILE    0x4
#define STT_LOPROC  0xd
#define STT_HIPROC  0xf


/*
 * ELF structures: Dynamic linking info
 */
struct Elf32Dyn
{
	Elf32_Sword d_tag;
	union
	{
		Elf32_Word d_val;
		Elf32_Addr d_ptr;
	} d_un;
};

/* Elf32Dyn.d_tag */
#define DT_NULL     0x00000000
#define DT_NEEDED   0x00000001
#define DT_PLTRELSZ 0x00000002
#define DT_PLTGOT   0x00000003
#define DT_HASH     0x00000004
#define DT_STRTAB   0x00000005
#define DT_SYMTAB   0x00000006
#define DT_RELA     0x00000007
#define DT_RELASZ   0x00000008
#define DT_RELAENT  0x00000009
#define DT_STRSZ    0x0000000a
#define DT_SYMENT   0x0000000b
#define DT_INIT     0x0000000c
#define DT_FINI     0x0000000d
#define DT_SONAME   0x0000000e
#define DT_RPATH    0x0000000f
#define DT_SYMBOLIC 0x00000010
#define DT_REL      0x00000011
#define DT_RELSZ    0x00000012
#define DT_RELENT   0x00000013
#define DT_PLTREL   0x00000014
#define DT_DEBUG    0x00000015
#define DT_TEXTREL  0x00000016
#define DT_JMPREL   0x00000017
#define DT_BIND_NOW 0x00000018
#define DT_LOPROC   0x70000000
#define DT_HIPROC   0x7fffffff

/*
 * ELF structures: Relocation Entries
 */
struct Elf32_Rel
{
	Elf32_Addr r_offset;
	Elf32_Word r_info;
};

struct Elf32_Rela
{
	Elf32_Addr  r_offset;
	Elf32_Word  r_info;
	Elf32_Sword r_addend;
};

/* Elf32_Rel|a.r_info manipulation macros */
#define ELF32_R_SYM(i)     ((i) >> 8)
#define ELF32_R_TYPE(i)    ((i) & 0xff)
#define ELF32_R_INFO(s,t)  ((s) << 8 + (t) & 0xff)

/* ELF32_R_TYPE(Elf32_Rel|a.r_info) values */
#define R_386_NONE      0x00
#define R_386_32        0x01
#define R_386_PC32      0x02
#define R_386_GOT32     0x03
#define R_386_PLT32     0x04
#define R_386_COPY      0x05
#define R_386_GLOB_DAT  0x06
#define R_386_JMP_SLOT  0x07
#define R_386_RELATIVE  0x08
#define R_386_GOTOFF    0x09
#define R_386_GOTPC     0x0a


/*
 * ELF structures: Program Header
 */
struct Elf32ProgramHeader
{
	Elf32_Word p_type;           /**< Program section type */
	Elf32_Off  p_offset;         /**< File offset */
	Elf32_Addr p_vaddr;          /**< Execution virtual address */
	Elf32_Addr p_paddr;          /**< Execution physical address */
	Elf32_Word p_filesz;         /**< Size in file */
	Elf32_Word p_memsz;          /**< Size in memory */
	Elf32_Word p_flags;          /**< Section flags */
	Elf32_Word p_align;          /**< Section alignment */
};

/* Elf32ProgramHeader.p_type */
#define PT_NULL     0
#define PT_LOAD     1
#define PT_DYNAMIC  2
#define PT_INTERP   3
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6
#define PT_LOPROC   0x70000000
#define PT_HIPROC   0x7fffffff

/* Elf32ProgramHeader.p_flags */
#define PF_X        0x1
#define PF_W        0x2
#define PF_R        0x4
#define PF_MASKPROC 0xf0000000


/* Symbol Hash table hashing function */
inline unsigned long
elf_hash(const unsigned char *name)
{
	unsigned long h = 0, g;
	while (*name)
	{
		h = (h << 4) + *name++;
		if ((g = h & 0xf0000000))
			h ^= g >> 24;
		h &= ~g;
	}
	return h;
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
