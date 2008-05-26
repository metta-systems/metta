ENTRY (_loader)

SECTIONS{
	.text 0x00100000 :
	{
		code = .; _code = .; __code = .;
		*(.text)
	}

	.rodata ALIGN (0x1000) :
	{
		*(.rodata)
	}

	.data ALIGN (0x1000) :
	{
		start_ctors = .;
		*(.ctor*)
		end_ctors = .;
		start_dtors = .;
		*(.dtor*)
		end_dtors = .;
		data = .; _data = .; __data = .;
		*(.data)
	}

    .bss ALIGN (0x1000) :
    {
        _sbss = .;
        *(COMMON)
        *(.bss)
        _ebss = .;
    }

    end = .; _end = .; __end = .;
}
