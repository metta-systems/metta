/* This is x86 linker script. */
/* Ensure everything goes in the right order. */
ENTRY(entry)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    K_SPACE_START = 0xF0000000;
    PAGE_SIZE = 4K;

    .text K_SPACE_START ALIGN(PAGE_SIZE) : AT(ADDR(.text) - K_SPACE_START)
    {
        *(.kernel_infopage)
        *(.text)
        *(.rodata*)
    }
    text_end = .;

    .data . ALIGN(PAGE_SIZE) : AT(ADDR(.data) - K_SPACE_START)
    {
        start_ctors = .;
        *(SORT(.ctor*))
        end_ctors = .;
        *(.data)
        FILL(0x00)
    }
    data_end = .;

    .bss . ALIGN (PAGE_SIZE) : AT(ADDR(.bss) - K_SPACE_START)
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