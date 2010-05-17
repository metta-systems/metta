//
// ELF file format structure definitions.
// According to Portable Formats Specification, version 1.2
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "macros.h"

namespace elf32
{


/*!
 * ELF data types
 */
typedef uint32_t  addr_t;  /*!< 4 bytes/4 align/unsigned */
typedef uint16_t  half_t;  /*!< 2 bytes/2 align/unsigned */
typedef uint32_t  off_t;   /*!< 4 bytes/4 align/unsigned */
typedef  int32_t  sword_t; /*!< 4 bytes/4 align/signed   */
typedef uint32_t  word_t;  /*!< 4 bytes/4 align/unsigned */
typedef  uint8_t  byte_t;  /*!< 1 byte /1 align/unsigned */


/*!
 * ELF file Header
 */
struct header_t
{
    word_t  magic;
    byte_t  elfclass;       /*!< File class */
    byte_t  data;           /*!< Data encoding */
    byte_t  hdrversion;     /*!< File version */
    byte_t  padding[9];
    half_t  type;           /*!< Identifies object file type */
    half_t  machine;        /*!< Specifies required architecture */
    word_t  version;        /*!< Identifies object file version */
    addr_t  entry;          /*!< Entry point virtual address */
    off_t   phoff;          /*!< Program header table file offset */
    off_t   shoff;          /*!< Section header table file offset */
    word_t  flags;          /*!< Processor-specific flags */
    half_t  ehsize;         /*!< ELF header size in bytes */
    half_t  phentsize;      /*!< Program header table entry size */
    half_t  phnum;          /*!< Program header table entry count */
    half_t  shentsize;      /*!< Section header table entry size */
    half_t  shnum;          /*!< Section header table entry count */
    half_t  shstrndx;       /*!< Section header string table index */
} PACKED;

/* header.magic */
#define ELF_MAGIC  0x464c457f    /*!< ASCII "ELF",0x7F */

/* header.elfclass */
#define ELF_CLASS_NONE 0x00      /*!< Invalid class  */
#define ELF_CLASS_32   0x01      /*!< 32 bit objects */
#define ELF_CLASS_64   0x02      /*!< 64 bit objects */

/* header.data */
#define ELF_DATA_NONE  0x00      /*!< Invalid data encoding   */
#define ELF_DATA_2LSB  0x01      /*!< LSB (Intel) encoding    */
#define ELF_DATA_2MSB  0x02      /*!< MSB (Motorola) encoding */

/* header.type */
#define ET_NONE    0x0000        /*!< No type     */
#define ET_REL     0x0001        /*!< Relocatable */
#define ET_EXEC    0x0002        /*!< Executable  */
#define ET_DYN     0x0003        /*!< Shared      */
#define ET_CORE    0x0004        /*!< Core        */
#define ET_LOPROC  0xff00        /*!< Processor-specific start */
#define ET_HIPROC  0xffff        /*!< Processor-specific end */

/* header.machine */
#define EM_NONE  0x0000          /*!< No machine     */
#define EM_M32   0x0001          /*!< AT&T WE32100   */
#define EM_SPARC 0x0002          /*!< SPARC          */
#define EM_386   0x0003          /*!< x86            */
#define EM_68K   0x0004          /*!< Motorola 68000 */
#define EM_88K   0x0005          /*!< Motorola 88000 */
#define EM_860   0x0007          /*!< Intel 80860    */
#define EM_MIPS  0x0008          /*!< MIPS RS3000 big-endian    */
#define EM_MIPS_RS4_BE 0x000a    /*!< MIPS RS4000 big-endian */

/* header.version */
#define EV_NONE        0         /*!< Invalid version */
#define EV_CURRENT     1         /*!< Current version */


/*!
 * Section header entry.
 */
struct section_header_t
{
    word_t  name;          /*!< Section name, index in string table */
    word_t  type;          /*!< Type of section */
    word_t  flags;         /*!< Miscellaneous section attributes */
    addr_t  addr;          /*!< Section virtual addr at execution */
    off_t   offset;        /*!< Section file offset */
    word_t  size;          /*!< Size of section in bytes */
    word_t  link;          /*!< Index of another section */
    word_t  info;          /*!< Additional section information */
    word_t  addralign;     /*!< Section alignment */
    word_t  entsize;       /*!< Entry size if section holds table */
} PACKED;

/* predefined section table indices */
#define SHN_UNDEF     0x0000
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC    0xff00
#define SHN_HIPROC    0xff1f
#define SHN_ABS       0xfff1
#define SHN_COMMON    0xfff2
#define SHN_HIRESERVE 0xffff

/* section_header.type */
#define SHT_NULL      0x00000000
#define SHT_PROGBITS  0x00000001 /*!< The data is contained in program file */
#define SHT_SYMTAB    0x00000002 /*!< Symbol table */
#define SHT_STRTAB    0x00000003 /*!< String table */
#define SHT_RELA      0x00000004
#define SHT_HASH      0x00000005 /*!< Symbol hash table */
#define SHT_DYNAMIC   0x00000006 /*!< Dynamic linking information */
#define SHT_NOTE      0x00000007
#define SHT_NOBITS    0x00000008 /*!< The data is not contained in program file */
#define SHT_REL       0x00000009
#define SHT_SHLIB     0x0000000a /*!< Reserved with unspecified semantics */
#define SHT_DYNSYM    0x0000000b
#define SHT_LOPROC    0x70000000
#define SHT_HIPROC    0x7fffffff
#define SHT_LOUSER    0x80000000
#define SHT_HIUSER    0xffffffff
/* GNU extensions */
#define SHT_INIT      0x0000000e
#define SHT_FINI      0x0000000f
#define SHT_PREINIT   0x00000010

/* section_header.flags */
#define SHF_WRITE     0x00000001
#define SHF_ALLOC     0x00000002
#define SHF_EXECINSTR 0x00000004
#define SHF_MASKPROC  0xf0000000


/*!
 * Symbol Table entry.
 */
struct symbol_t
{
    word_t  name;          /*!< Symbol name, index into string table */
    addr_t  value;         /*!< Symbol value */
    word_t  size;          /*!< Size occupied by this symbol */
    byte_t  info;          /*!< Symbol type and binding */
    byte_t  other;         /*!< This member currently holds 0 and has no defined meaning. */
    half_t  shndx;         /*!< Section index this symbol belongs to */
} PACKED;

/* Symbol Table index: first/undefined entry */
#define STN_UNDEF 0x0000

/* symbol.info manipulation macros */
#define ELF32_ST_BIND(i)    ((i) >> 4)
#define ELF32_ST_TYPE(i)    ((i) & 0xf)
#define ELF32_ST_INFO(b,t)  ((b) << 4 + ((t) & 0xf))

/* ELF32_ST_BIND(symbol.info) values */
#define STB_LOCAL  0x0
#define STB_GLOBAL 0x1
#define STB_WEAK   0x2
#define STB_LOPROC 0xd
#define STB_HIPROC 0xf

/* ELF32_ST_TYPE(symbol.info) values */
#define STT_NOTYPE  0x0
#define STT_OBJECT  0x1
#define STT_FUNC    0x2
#define STT_SECTION 0x3
#define STT_FILE    0x4
#define STT_LOPROC  0xd
#define STT_HIPROC  0xf


/*!
 * Relocation Entries
 */
struct rel_t
{
    addr_t  offset;
    word_t  info;
};

struct rela_t
{
    addr_t   offset;
    word_t   info;
    sword_t  addend;
};

/* rel|a.info manipulation macros */
#define ELF32_R_SYM(i)     ((i) >> 8)
#define ELF32_R_TYPE(i)    ((i) & 0xff)
#define ELF32_R_INFO(s,t)  ((s) << 8 + (t) & 0xff)

/* ELF32_R_TYPE(rel|a.info) values */
#define R_386_NONE      0x00
#define R_386_32        0x01
#define R_386_PC32      0x02
/* Following relocation types are reserved for i386 architecture */
#define R_386_GOT32     0x03
#define R_386_PLT32     0x04
#define R_386_COPY      0x05
#define R_386_GLOB_DAT  0x06
#define R_386_JMP_SLOT  0x07
#define R_386_RELATIVE  0x08
#define R_386_GOTOFF    0x09
#define R_386_GOTPC     0x0a


/*!
 * Program Header entry.
 */
struct program_header_t
{
    word_t  type;           /*!< Program section type */
    off_t   offset;         /*!< File offset */
    addr_t  vaddr;          /*!< Execution virtual address */
    addr_t  paddr;          /*!< Execution physical address */
    word_t  filesz;         /*!< Size in file */
    word_t  memsz;          /*!< Size in memory */
    word_t  flags;          /*!< Section flags */
    word_t  align;          /*!< Section alignment */
};

/* program_header.type */
#define PT_NULL     0
#define PT_LOAD     1
#define PT_DYNAMIC  2
#define PT_INTERP   3
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6
#define PT_LOPROC   0x70000000
#define PT_HIPROC   0x7fffffff

/* program_header.flags */
#define PF_X        0x1
#define PF_W        0x2
#define PF_R        0x4
#define PF_MASKPROC 0xf0000000


/*!
 * Dynamic linking info
 */
struct dyn_t
{
    sword_t tag;
    union
    {
        word_t val;
        addr_t ptr;
    } u;
};
//extern dyn _DYNAMIC[];
//extern addr_t _GLOBAL_OFFSET_TABLE_[];

/* dyn.tag */
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


/*! Symbol Hash table hashing function */
inline uint32_t elf_hash(const char* name)
{
    uint32_t h = 0, g;
    while (*name)
    {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        if (g)
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

} // namespace elf32

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
