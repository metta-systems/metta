//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "ELF.h"
#include "Types.h"

/**
 * Parses an ELF file, generating symbolic information and loading code/data segments.
 */
class ElfParser
{
	public:
		/**
		 *Creates a blank ELF parser, preparing for a call to loadKernel.
		 */
		ElfParser();

		/**
		 * Creates the ELF parser based on a file.
		 */
		ElfParser(char *fname);

		~ElfParser();

		/**
		 * Duplicates (this), performing a deep copy.
		 */
		ElfParser *clone();

  /**
  Writes all section information to the virtual memory image.
  **/
  void writeAllSections();

  /**
  Returns the address of the last byte to be loaded in.
  **/
  Address getLastAddress();

  /**
  Loads the symbol table for the kernel from the specified location.
  **/
  void loadKernel(Elf32SectionHeader *symtab, Elf32SectionHeader *strtab);

  /**
  Returns the symbol name for an address. Also returns the start address
  of that symbol in startAddr if startAddr != NULL.
  **/
  char *findSymbol(Address addr, Address *symbolStart=NULL);

  /**
  Returns the address of a symbol with name str.
  NOTE: This is much slower than it should be. This should be implemented
  using the hashtable sections in ELF.
  **/
  Address findSymbol(char *str);

  /**
  Returns the address of the symbol with offset o in the
  relocation symbol table.
  **/
  Address findDynamicSymbolLocation(Address o);

  /**
  Returns a NULL terminated string specifying the name of
  the symbol at offset o in the relocation symbol table.
  **/
  char *findDynamicSymbolName(Address o);

  /**
  Gets the address of the global offset table.
  **/
  Address getGlobalOffsetTable();

  /**
  Returns the entry point of the executable.
  **/
  Address getEntryPoint()
  {
	  return (Address)header->e_entry;
  }

  /**
  Returns true if the parser loaded correctly.
  **/
  bool isValid()
  {
	  return (filename!=0);
  }

	private:
		Elf32Header        *header;
		Elf32SectionHeader *symbolTable;
		Elf32SectionHeader *stringTable;
		Elf32SectionHeader *gotTable; // Global offset table.
		Elf32SectionHeader *relTable;
		Elf32SectionHeader *sectionHeaders;
	public:
		char               *filename;
};

