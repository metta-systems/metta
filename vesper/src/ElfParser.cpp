#include "Kernel.h"
#include "ElfParser.h"
#include "DefaultConsole.h"

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
	Address max = 0;
	Elf32Symbol *fallbackSymbol = 0;
	for (unsigned int i = 0; i < symbolTable->sh_size / symbolTable->sh_entsize; i++)
	{
		Elf32Symbol *symbol = (Elf32Symbol *)(symbolTable->sh_addr + i * symbolTable->sh_entsize);

		if ((addr >= symbol->st_value) &&
			(addr <  symbol->st_value + symbol->st_size) )
		{
			char *c = (char *)(symbol->st_name) + stringTable->sh_addr;

			if (symbolStart)
			{
				*symbolStart = symbol->st_value;
			}
			return c;
		}

		if (symbol->st_value > max && symbol->st_value <= addr)
		{
			max = symbol->st_value;
			fallbackSymbol = symbol;
		}
	}

	// Search for symbol with size failed, now take a wild guess.
	// Use a biggest symbol value less than addr (if found).
	if (fallbackSymbol)
	{
		char *c = (char *)(fallbackSymbol->st_name) + stringTable->sh_addr;

		if (symbolStart)
		{
			*symbolStart = fallbackSymbol->st_value;
		}
		return c;
	}

	if (symbolStart)
		*symbolStart = 0;
	return NULL;
}
