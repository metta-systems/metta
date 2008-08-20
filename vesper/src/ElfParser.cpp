#include "ElfParser.h"

ElfParser::ElfParser()
{
	header             = NULL;
	sectionHeaders     = NULL;
	symbolTable        = NULL;
	stringTable        = NULL;
	gotTable           = NULL;
	filename           = NULL;
}

ElfParser::~ElfParser()
{
	delete header;
	delete sectionHeaders;
	delete [] filename;
}

void ElfParser::loadKernel(Elf32SectionHeader *symtab, Elf32SectionHeader *strtab)
{
	symbolTable = symtab;
	stringTable = strtab;
}

char *ElfParser::findSymbol(Address addr, Address *symbolStart)
{
	Elf32Symbol *symbol = (Elf32Symbol *)symbolTable->sh_addr;

	for(unsigned int i = 0; i < symbolTable->sh_size / sizeof(Elf32Symbol); i++)
	{
		if ( (addr >= symbol->st_value) &&
			(addr <  symbol->st_value + symbol->st_size) )
		{
			char *c = (char *)(symbol->st_name)+stringTable->sh_addr;

			if (symbolStart)
			{
				*symbolStart = symbol->st_value;
			}
			return c;
		}
		symbol ++;
	}
	return NULL;
}
