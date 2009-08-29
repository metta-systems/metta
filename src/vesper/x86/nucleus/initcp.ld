/* Ensure everything goes in the right order. */
/* This is x86 linker script. */
ENTRY (entry)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    TEXT_BASE = 0x1000;
    PAGE_SIZE = 0x1000;

    . = TEXT_BASE;
    .text ALIGN (PAGE_SIZE) : AT(ADDR(.text) - TEXT_BASE)
    {
        *(.text)
        FILL(0x90)
    }
    text_end = .;

    .data ALIGN (PAGE_SIZE) : AT(ADDR(.data) - TEXT_BASE)
    {
        *(.data)
        *(.rodata*)
        FILL(0x00)
    }
    data_end = .;

    .bss ALIGN (PAGE_SIZE) :  AT(ADDR(.bss) - TEXT_BASE)
    {
        *(COMMON*)
        *(.bss*)
    }
    bss_end = .;

    placement_address = ALIGN(PAGE_SIZE);

    /* Strip unnecessary stuff */
    /* TODO: keep static ctors for initcp */
    /DISCARD/ : { *(.comment .note* .eh_frame .ctors .dtors) }
}

/*
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
*/