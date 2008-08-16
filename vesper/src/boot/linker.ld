/* Ensure everything goes in the right order. */
ENTRY (_loader)
SEARCH_DIR (./build/default)

SECTIONS
{
	.text 0x100000 :
	{
		code = .; _code = .; __code = .;
		default/loader.o(.text)
		*(.text)
		FILL(0x90)
	}

	.data ALIGN (0x1000) :
	{
		start_ctors = .;
		*(.ctors)  /* or *(SORT(.ctors)) */
		end_ctors = .;

		data = .; _data = .; __data = .;
		*(.data)
		*(.rodata)
		FILL(0x00)
	}
	edata = .; _edata = .; __edata = .;

	.bss ALIGN (0x1000) :
	{
		_sbss = .;
		*(COMMON)
		*(.bss)
		_ebss = .;
		. = ALIGN(0x1000);
	}

	end = .; _end = .; __end = .;
}
