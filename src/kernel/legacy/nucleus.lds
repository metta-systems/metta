/* This is x86 linker script. */
/* Ensure everything goes in the right order. */
ENTRY(init)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    K_SPACE_START             = 0xF0000000;

    K_HEAP_START              = 0xF1000000;
    K_HEAP_END                = 0xF2000000;

    K_STACK_START             = (0xF8000000-0x4);
    K_STACK_END               = 0xF0000000;

    PAGE_SIZE = 4K;

    . = K_SPACE_START;
    .text ALIGN(PAGE_SIZE) : AT(ADDR(.text) - K_SPACE_START)
    {
        *(.kernel_infopage)
        *(.text*)
        *(.rodata*)
        /* global static initializers */
        ctors_GLOBAL = .;
        *(SORT(.ctor*))
        QUAD(0)
        FILL(0x90)
    }
    text_end = .;

    .data ALIGN(PAGE_SIZE) : AT(ADDR(.data) - K_SPACE_START)
    {
        *(.data*)
        FILL(0x00)
    }
    data_end = .;

    .bss ALIGN (4) : AT(ADDR(.bss) - K_SPACE_START)
    {
        *(COMMON*)
        *(.bss*)
    }
    bss_end = .;

    . = ALIGN(PAGE_SIZE);
    image_end = .;

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note* .eh_frame .dtors) }
}

/*
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
*/
