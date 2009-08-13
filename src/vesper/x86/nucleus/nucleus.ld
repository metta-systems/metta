/* Ensure everything goes in the right order. */
/* This is x86 linker script. */
ENTRY (entry)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    K_SPACE_START = 0xF0000000;
    PAGE_SIZE = 0x1000;

    . = K_SPACE_START;
    .text ALIGN (PAGE_SIZE) :
    {
        *(.kernel_infopage)
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

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note* .eh_frame .dtors) }
}

/*
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
*/