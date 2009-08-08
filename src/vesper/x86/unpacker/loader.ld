/* Ensure everything goes in the right order. */
/* This is x86 linker script. */
ENTRY (loader)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    KERNEL_BASE = 0x100000;
    PAGE_SIZE = 0x1000;

    . = 0x100000;
    .text ALIGN (PAGE_SIZE) :
    {
        *(.loader.text)
        *(.text)
        FILL(0x90)
    }
    text_end = .;

    .data ALIGN (PAGE_SIZE) :
    {
        start_ctors = .;
        *(.ctor*)
        end_ctors = .;
        *(.data)
        *(.rodata*)
        FILL(0x00)
    }
    data_end = .;

    .bss ALIGN (PAGE_SIZE) :
    {
        *(COMMON*)
        *(.bss*)
    }
    bss_end = .;

    placement_address = ALIGN(PAGE_SIZE);

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note* .eh_frame .dtors) }
}

/*
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
*/