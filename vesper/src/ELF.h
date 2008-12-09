//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
//  ELF file format structure definitions.
//  According to Portable Formats Specification, version 1.1
//  (FIXME: update to 1.2)
//
#pragma once

#include "types.h"
#include "macros.h"

namespace elf32 {

/*
 * ELF data types
 */
typedef uint32_t  addr_t;  /* 4 bytes/4 align/unsigned */
typedef uint16_t  half_t;  /* 2 bytes/2 align/unsigned */
typedef uint32_t  off_t;   /* 4 bytes/4 align/unsigned */
typedef  int32_t  sword_t; /* 4 bytes/4 align/signed   */
typedef uint32_t  word_t;  /* 4 bytes/4 align/unsigned */
typedef  uint8_t  byte_t;  /* 1 byte /1 align/unsigned */


/*
 * ELF structures: ELF file Header
 */
struct header
{
    word_t e_magic;
    byte_t e_class;
    byte_t e_data;
    byte_t e_hdrversion;
    byte_t e_padding[9];
    half_t e_type;           /**< Identifies object file type */
    half_t e_machine;        /**< Specifies required architecture */
    word_t e_version;        /**< Identifies object file version */
    addr_t e_entry;          /**< Entry point virtual address */
    off_t  e_phoff;          /**< Program header table file offset */
    off_t  e_shoff;          /**< Section header table file offset */
    word_t e_flags;          /**< Processor-specific flags */
    half_t e_ehsize;         /**< ELF header size in bytes */
    half_t e_phentsize;      /**< Program header table entry size */
    half_t e_phnum;          /**< Program header table entry count */
    half_t e_shentsize;      /**< Section header table entry size */
    half_t e_shnum;          /**< Section header table entry count */
    half_t e_shstrndx;       /**< Section header string table index */
} PACKED;

/* header.e_magic */
#define ELF_MAGIC  0x464c457f    /* ASCII "ELF",0x7F */

/* header.e_class */
#define ELF_CLASS_NONE 0x00      /**< Invalid class  */
#define ELF_CLASS_32   0x01      /**< 32 bit objects */
#define ELF_CLASS_64   0x02      /**< 64 bit objects */

/* header.e_data */
#define ELF_DATA_NONE  0x00      /**< Invalid data encoding   */
#define ELF_DATA_2LSB  0x01      /**< LSB (Intel) encoding    */
#define ELF_DATA_2MSB  0x02      /**< MSB (Motorola) encoding */

/* header.e_type */
#define ET_NONE    0x0000        /* No type     */
#define ET_REL     0x0001        /* Relocatable */
#define ET_EXEC    0x0002        /* Executable  */
#define ET_DYN     0x0003        /* Shared      */
#define ET_CORE    0x0004        /* Core        */
#define ET_LOPROC  0xff00        /* Processor-specific */
#define ET_HIPROC  0xffff

/* header.e_machine */
#define EM_NONE  0x0000          /* No machine     */
#define EM_M32   0x0001          /* AT&T WE32100   */
#define EM_SPARC 0x0002          /* SPARC          */
#define EM_386   0x0003          /* x86            */
#define EM_68K   0x0004          /* Motorola 68000 */
#define EM_88K   0x0005          /* Motorola 88000 */
#define EM_860   0x0007          /* Intel 80860    */
#define EM_MIPS  0x0008          /* MIPS RS3000    */

/* header.e_version */
#define EV_NONE        0         /* Invalid version */
#define EV_CURRENT     1         /* Current version */


/*
 * ELF structures: Section header
 */
struct section_header
{
    word_t sh_name;          /**< Section name, index in string table */
    word_t sh_type;          /**< Type of section */
    word_t sh_flags;         /**< Miscellaneous section attributes */
    addr_t sh_addr;          /**< Section virtual addr at execution */
    off_t  sh_offset;        /**< Section file offset */
    word_t sh_size;          /**< Size of section in bytes */
    word_t sh_link;          /**< Index of another section */
    word_t sh_info;          /**< Additional section information */
    word_t sh_addralign;     /**< Section alignment */
    word_t sh_entsize;       /**< Entry size if section holds table */
} PACKED;

/* predefined section table indices */
#define SHN_UNDEF     0x0000
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC    0xff00
#define SHN_HIPROC    0xff1f
#define SHN_ABS       0xfff1
#define SHN_COMMON    0xfff2
#define SHN_HIRESERVE 0xffff

/* section_header.sh_type */
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


/* section_header.sh_flags */
#define SHF_WRITE     0x00000001
#define SHF_ALLOC     0x00000002
#define SHF_EXECINSTR 0x00000004
#define SHF_MASKPROC  0xf0000000


/*
 * ELF structures: Symbol Table
 */
struct symbol
{
    word_t st_name;          /**< Symbol name, index into string table */
    addr_t st_value;         /**< Symbol value */
    word_t st_size;          /**< Size occupied by this symbol */
    byte_t st_info;          /**< Symbol type and binding */
    byte_t st_other;
    half_t st_shndx;         /**< Section index this symbol belongs to */
} PACKED;

/* Symbol Table index: first/undefined entry */
#define STN_UNDEF 0x0000

/* symbol.st_info manipulation macros */
#define ELF32_ST_BIND(i)    ((i) >> 4)
#define ELF32_ST_TYPE(i)    ((i) & 0xf)
#define ELF32_ST_INFO(b,t)  ((b) << 4 + ((t) & 0xf))

/* ELF32_ST_BIND(symbol.st_info) values */
#define STB_LOCAL  0x0
#define STB_GLOBAL 0x1
#define STB_WEAK   0x2
#define STB_LOPROC 0xd
#define STB_HIPROC 0xf

/* ELF32_ST_TYPE(symbol.st_info) values */
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
struct dyn
{
    sword_t d_tag;
    union
    {
        word_t d_val;
        addr_t d_ptr;
    } d_un;
};

/* dyn.d_tag */
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
struct rel
{
    addr_t r_offset;
    word_t r_info;
};

struct rela
{
    addr_t  r_offset;
    word_t  r_info;
    sword_t r_addend;
};

/* rel|a.r_info manipulation macros */
#define ELF32_R_SYM(i)     ((i) >> 8)
#define ELF32_R_TYPE(i)    ((i) & 0xff)
#define ELF32_R_INFO(s,t)  ((s) << 8 + (t) & 0xff)

/* ELF32_R_TYPE(rel|a.r_info) values */
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
struct program_header
{
    word_t p_type;           /**< Program section type */
    off_t  p_offset;         /**< File offset */
    addr_t p_vaddr;          /**< Execution virtual address */
    addr_t p_paddr;          /**< Execution physical address */
    word_t p_filesz;         /**< Size in file */
    word_t p_memsz;          /**< Size in memory */
    word_t p_flags;          /**< Section flags */
    word_t p_align;          /**< Section alignment */
};

/* program_header.p_type */
#define PT_NULL     0
#define PT_LOAD     1
#define PT_DYNAMIC  2
#define PT_INTERP   3
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6
#define PT_LOPROC   0x70000000
#define PT_HIPROC   0x7fffffff

/* program_header.p_flags */
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

} // namespace elf32

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
