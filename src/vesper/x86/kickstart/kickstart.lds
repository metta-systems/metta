/* This is x86 linker script. */
/* Ensure everything goes in the right order. */
ENTRY (loader)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    KICKSTART_BASE = 0x100000;
    PAGE_SIZE = 0x1000;

    . = KICKSTART_BASE;
    .text ALIGN (4) :
    {
        *(.multiboot_info)
        *(.text)
        *(.rodata*)
        FILL(0x90)
    }
    text_end = .;

    .data ALIGN (4) :
    {
        start_ctors = .;
        *(.ctor*)
        end_ctors = .;
        *(.data)
        FILL(0x00)
    }
    data_end = .;

    .bss ALIGN (4) :
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